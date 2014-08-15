// PERMEATE
// Copyright (C) 2008 Jarek Paduch, Bilal Khan, Jamie Levy
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
// Email: jarek.paduch@gmail.com, grouptheory@gmail.com, jamie.levy@gmail.com
// The above header may not be removed or modified without contacting the authors.

#include <linux/netfilter.h>
#include <libipq.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// constants
#define BUFSIZE 16384
#define PHASE1  10
#define IDBUFFERSIZE 1000
#define MASK 128
#define DEBUG 1
#define SLOTS 16

#define ifaceName "eth0"
extern long myip;
char *remoteip;

//global var to see how many chars getNextBits() sees before it sees "0"
int charCount=0;

// prototypes
void juggler(struct ipq_handle* h, unsigned char* packet, ipq_packet_msg_t* m,int slot);
void antijuggler(struct ipq_handle* h, unsigned char* packet, ipq_packet_msg_t* m,int slot);
void init_juggler(char* fname_tosend);
void init_antijuggler(char* fname_torecv);
void saveIDs(int id0);
void exit_juggler();
void exit_antijuggler();
void interpret();
void getLocalMac();
void calculateRemoteMac();
int test48();
char getNextBitToSend();
char sendFrameFooter();
void logMac();

//Juggler global state
struct juggler_state {  
  int ct;
  int mod2;
  unsigned char* packet0;
  ipq_packet_msg_t* m0;
  int slot0;
  unsigned char* packet1;
  ipq_packet_msg_t* m1;
  int slot1;
  char filename[255];
  FILE* fp;
  int bitCount;
  char currentChar;
  char mystring[1024];
  char remoteMac[1024];
  char *mystringPtr;
  int sendFrame;
  int frameBitCount;
} state;

//Antijuggler global state
struct antijuggler_state {  
  int ct;
  unsigned char* packet0;
  ipq_packet_msg_t* m0;
  int slot0;
  int savedIDs[IDBUFFERSIZE];
  int insertIndex;
  char filename[255];
  FILE* fp;
  int logfd;
  int bitCount;
  char localMac[1024];
  char rcvdMac[1024];
} antistate;

struct packet_processing_state {  
    struct ipq_handle *h;
    char available[SLOTS];
    unsigned char** packet;
    ipq_packet_msg_t** m;
} ppstate;


void init_packet_processing() {
    int i, status;
    
    // open the socket and initialize
    
    ppstate.h=ipq_create_handle(0,PF_INET);
    if(!ppstate.h) die(ppstate.h);
 
    //IPQ_COPY_PACKET will copy the entire message, not just the header 
    status = ipq_set_mode(ppstate.h,IPQ_COPY_PACKET,BUFSIZE);
    if(status < 0) die(ppstate.h);
  
    ppstate.packet = (unsigned char**) malloc(SLOTS*sizeof(unsigned char*));
    ppstate.m = (ipq_packet_msg_t**)malloc(SLOTS*sizeof(ipq_packet_msg_t*));
   
    for (i=0;i<SLOTS;i++) {
        ppstate.available[i]=1;
        ppstate.packet[i]=(unsigned char*)malloc(BUFSIZE*sizeof(unsigned char));
        ppstate.m[i]=0L;
    }
}

int take_available_slot() {
    // go through the slots
    int i;
    for (i=0;i<SLOTS;i++) {
        // find one that is available
        if (ppstate.available[i]==1) {
            // mark it as no longer available
            ppstate.available[i]=0;
            return i;
        }
    }
    // the sentinel that means no slots available.  This should never occur
    return -1;
}

void return_slot(int i) {
    // mark the slot as available now
    ppstate.available[i]=1;
}

// main packet-processor
void packet_processing() {
  int status, slot;

  init_packet_processing();
  
  // forever
  while(1) {

    slot = take_available_slot();
    if (slot<0) {
        printf("FATAL: packet_processing() ran out of slots!\n");
        exit(-1);
    }
    
    // read a packet
    status=ipq_read(ppstate.h,ppstate.packet[slot],BUFSIZE,0);

    if(status < 0) 
      fprintf(stderr,"Received error on rcv: %s\n",ipq_errstr());

    switch (ipq_message_type(ppstate.packet[slot])) {
    case NLMSG_ERROR:
      fprintf(stderr, "Received error msg: %d\n",ipq_get_msgerr(ppstate.packet[slot]));
      break;
    case IPQM_PACKET:
      ppstate.m[slot]=ipq_get_packet(ppstate.packet[slot]);

      // dispatch
      if(isMe(getSrcIP(ppstate.m[slot]->payload))) 
	juggler(ppstate.h, ppstate.packet[slot], ppstate.m[slot], slot);
      else 
	antijuggler(ppstate.h, ppstate.packet[slot], ppstate.m[slot], slot);
      break;
    default:
      fprintf(stderr, "Unknown message type!\n");
      break;
    }
  }
}

void juggler(struct ipq_handle* h, unsigned char* packet, ipq_packet_msg_t* m, int slot) {
  if (state.ct < PHASE1) { // BOOT PHASE
    int status = ipq_set_verdict(h,m->packet_id,NF_ACCEPT,0,NULL);
    if (status < 0) die(h);
    return_slot(slot);
  }
  else { // REAL STUFF
    
    if (state.mod2==0) {
      state.packet0=packet;
      state.m0=m;
      state.slot0=slot;
    }
    else {
      state.packet1=packet;
      state.m1=m;
      state.slot1=slot;
    }
//after getting 2 outgoing packets do the following
    if (++state.mod2 > 1) {

  	char bit;
  	state.mod2 = 0;
 //Get the next bit to send	
	bit = getNextBitToSend();

 	 if (bit == 0) {
		int status = ipq_set_verdict(h,state.m0->packet_id,NF_ACCEPT,0,NULL);
		if (status < 0) die(h);
      			return_slot(state.slot0);
		status = ipq_set_verdict(h,state.m1->packet_id,NF_ACCEPT,0,NULL);
		if (status < 0) die(h);
       		return_slot(state.slot1);
 	 }
 	 else { 
		int status = ipq_set_verdict(h,state.m1->packet_id,NF_ACCEPT,0,NULL);
		if (status < 0) die(h);
      	  	return_slot(state.slot1);
		status = ipq_set_verdict(h,state.m0->packet_id,NF_ACCEPT,0,NULL);
		if (status < 0) die(h);
        	return_slot(state.slot0);
 	 }
      }
    }
  state.ct++;
}

void antijuggler(struct ipq_handle* h, unsigned char* packet, ipq_packet_msg_t* m, int slot) 
{
  if (antistate.ct < PHASE1) { // BOOT PHASE
    int status = ipq_set_verdict(h,m->packet_id,NF_ACCEPT,0,NULL);
    if (status < 0) die(h);
    return_slot(slot);
  }
  else 
  { // REAL STUFF
      antistate.packet0=packet;
      antistate.m0=m;
      antistate.slot0=slot;
      short id0=getID(antistate.m0->payload);
      saveIDs(id0);
   
      //pass the verdict
      int status = ipq_set_verdict(h,antistate.m0->packet_id,NF_ACCEPT,0,NULL);
      if (status < 0) die(h);
      return_slot(antistate.slot0);
      int eoframe = 0;
      int i;
      int tmpI;

      // check for end of frame (00000000 11111111 00000000)by looking at the last 48 packet IDs 
      eoframe = test48();
      
      //if we find our frame mark do the following 
      if(eoframe)
      {
	//extract the MAC address by examing packet IDs saved in savedIDs array
	interpret();
	//set the insert index back to 0
	antistate.insertIndex = 0;
	//check to see if the MAC we received is null or not
	if((tmpI=memcmp(antistate.rcvdMac,"",1)) != 0)
	{
		//check to see if the local mac address is the same as the received mac address.
		//If they are different do the following
		if((tmpI=memcmp(antistate.localMac,antistate.rcvdMac,17))!=0){
			//print local MAC
			printf("My MAC is: ");
			for(i=0;i<17;i++)
				printf("%c",antistate.localMac[i]);
			
			printf("\n");
			//print what the other host thinks my mac is
			printf("Received MAC is: ");
			for(i=0;i<17;i++)
				printf("%c",antistate.rcvdMac[i]);
			
			printf("\nMACS don't match up\n");
			//log the bad mac in "/var/log/badmacs"
			logMac();
		}
		//if MACs are the same do nothing
		else
			printf("MACS match up\n");
	}
	else
	{
		printf("rcvdMac is empty\n");
	}
      }
  }
  antistate.ct++;
}
void logMac()
{
	int j;
	if((j=write(antistate.logfd,&antistate.rcvdMac,17))!=17)
	{
		printf("Error writing\n");
		exit(-1);
	}
}

void saveIDs(int id0) {
  antistate.savedIDs[antistate.insertIndex]=id0;
  ++antistate.insertIndex;
}

int test48()
{
	int tmpE;
	int tmpB;
	int count=0;
	int section=8;
	int i;

	if(antistate.insertIndex >= 48)
	{
		tmpB=antistate.insertIndex-48;
		tmpE=antistate.insertIndex-1;

//check for eight 0-bits
		for(i=0;i<section;i++)
		{
			if((antistate.savedIDs[tmpB]) < antistate.savedIDs[tmpB+1])
			count++;

			tmpB=tmpB+2;
		}
		if(count == 8){
			count=0;
		}
		else
			return 0;

//check for eight 1-bits			
		for(i=0;i<section;i++)
		{
			if((antistate.savedIDs[tmpB]) >  antistate.savedIDs[tmpB+1])
				count++;
			
		 	tmpB=tmpB+2;
		}	
		if(count == 8){
			count=0;
		}
		else return 0;
//check for eight 0-bits	
		for(i=0;i<section;i++)
		{
			if((antistate.savedIDs[tmpB]) < antistate.savedIDs[tmpB+1])
				count++;

			tmpB=tmpB+2;
		}	
		if(count == 8)
			return 1;
		else
			return 0;
	}
	else return 0;
}
void interpret()
{
	int i=0;
	int j=0;
	char bit;
	char tmpMAC[1024];
	char tmpChar;
	int stillGood=1;
	int count = 0;
//interpret MAC address after we have received at least 320 packets.
//MAC=17characters,it takes 8 bits to represent each character, and it takes 2 IP ids to represent each
//bit. 17*8*2 + (48)=320... where 48 is the number of IP IDs needed to represent frame mark	
	if(antistate.insertIndex >= 320)
	{
		while((i<272) && stillGood)
		{
			tmpChar<<=1;
			if((antistate.savedIDs[i]) > (antistate.savedIDs[i+1]))	bit=1;
			else 							bit=0;	
			tmpChar = tmpChar | bit;
			count++;
			if(count > 7)
			{
				if(is_legal(tmpChar)){
						tmpMAC[j]=tmpChar;
						j++;
				}
				else{
					stillGood=0;
				}	
			        tmpChar=0;
				count=0;
			}
			i=i+2;
		}
		if(DEBUG)printf("Value of 'i' should be 272 but its really %d\n",i);
		if(!stillGood){
			memcpy(antistate.rcvdMac,"",1);
			if(DEBUG)printf("MAC corrupted...Leaving Interpret now\n");
		}
		else{
	//if my logic is correct we should copy j-1 bytes from tmpMAC to antistate.rcvdMac.
			 memcpy(antistate.rcvdMac,tmpMAC,j-1);
			 if(DEBUG)printf("DONE READING REMOTE MAC...Leaving Interpret() now\n");
		}
	}
	else
	{
		if(DEBUG)printf("InsertIndex== %d....Not big enough...Exiting INTERPRET NOW\n",antistate.insertIndex);
	//if the insert indext is not big enough, set it to NULL
		 memcpy(antistate.rcvdMac,"",1);
	}
}

void calculateRemoteMac() {
  char cmd[255];
  FILE *fp;
  int i=1;
  if((fp = fopen("/tmp/mac","r"))< 0)
  {
	perror("open()");
	exit(-1);

  }

  sprintf(cmd,"arp %s | grep -v Address | sed -e 's/[^ ]* *[^ ]* *\\([^ ]*\\).*/\\1/' > /tmp/mac", remoteip);
  system(cmd);
  fgets(state.remoteMac,LINE_MAX,fp);
  while(i)
  {
	if(state.remoteMac[i]==0xA){
		state.remoteMac[i]=0;
		i=0;
	}
	else
		i++;
  }
//  return state.remoteMac;
}

char getNextBitToSend() {
  int bit,n;
 // return 0;
	
  if (state.bitCount > 7) {
    charCount++;
    if(DEBUG)printf("Number of chars juggler sent so far is %d\n",charCount);
    state.mystringPtr++;
//i don't think we need the line below but i'm going to leave it here for now because its here 
    state.currentChar = *(state.mystringPtr);
    if (*(state.mystringPtr) == 0) {
	if(DEBUG)printf("CurrentChar==0 it's time to START SENDING FRAME FOOTER\n");
      // recalculate what mystring should be
        calculateRemoteMac();
	charCount=0;

//sendFrame == 1 therefore we'll return bit=1 for the next 16 bits(that's our frame); 
	state.sendFrame=1;
        state.frameBitCount=0;
       if((n=sprintf(state.mystring,"%s",state.remoteMac))<0)
		perror("sprintf error");
	 // reset back to beginning when we get to the \0 terminator
      state.mystringPtr = state.mystring;
      state.currentChar = *(state.mystring);
    }
    
    state.bitCount    = 0;

   }
  if(!state.sendFrame)
  {
     if (state.currentChar & MASK)
   		 bit=1;
     else
                 bit=0;

 	 state.bitCount++;
 	 state.currentChar<<=1;
         return bit;	
   }
  else
   {
	bit= sendFrameFooter();
	return bit;
   }
}
 
char sendFrameFooter()
{
	char tmpBit;
	if(state.frameBitCount < 8)
		tmpBit=0;
	if(state.frameBitCount >=8 && state.frameBitCount < 16)
		tmpBit=1;
	if(state.frameBitCount >=16 && state.frameBitCount < 24)
		tmpBit=0;
	if(state.frameBitCount == 23)
	{
		state.sendFrame=0;
	}
	state.frameBitCount++;
	return tmpBit;

}

void getLocalMac()
{
	//Prints out my MAC address
  char cmd[255];
  FILE *fp;
  int i=0;
  int tmp =1;
  if((fp = fopen("/tmp/mymac","r"))< 0)
  {
	perror("open()");
	exit(-1);

  }
  sprintf(cmd,"/sbin/ifconfig %s | grep eth0 | sed -e 's/[^ ]* *[^ ]* *[^ ]* *[^ ]* *//' > /tmp/mymac",ifaceName);
  system(cmd);
  fgets(antistate.localMac,LINE_MAX,fp);
  while(tmp)
  {
	if(antistate.localMac[i]==0x20){
		antistate.localMac[i]=0;
		tmp=0;
		printf("Num of chars before finding 0 is %d\n",i);
	}
	else
		i++;
  }
}
void init_juggler(char* fname) {
  state.packet0 = 0L;
  state.packet1 = 0L;
  state.m0 = 0L;
  state.m1 = 0L;
  state.ct=0;
  state.mod2=0;
  strcpy(state.filename, fname);
  state.fp=fopen(state.filename,"r");
  if (state.fp == 0L) {
    printf("Juggler could not open file for reading/sending\n");
    exit(-1);
  }
  calculateRemoteMac(); 
//we don't need this anymore
//  memcpy(state.mystring,state.remoteMac,17);
  state.mystringPtr = state.mystring;
  if(DEBUG){
	int i;
	printf("INIT_JUGGLER: value of state.mystring is: \n");
	for(i=0;i<17;i++)
		printf("%c",state.mystring[i]);

	printf("\n");
  }
  state.currentChar = *(state.mystring);
  state.bitCount    = 0;
  state.sendFrame   = 0;
  state.frameBitCount=0;
}

void init_antijuggler(char* fname) {
  int i;
  antistate.packet0 = 0L;
  antistate.m0 = 0L;
  antistate.ct=0;
  antistate.insertIndex=0;
  for (i=0;i<IDBUFFERSIZE;i++) {
    antistate.savedIDs[i]=-1;
  }
  //we don't need this anymore
  //memcpy(antistate.filename, fname,strlen(fname));
  antistate.fp=fopen(antistate.filename,"w");
  if (antistate.fp == 0L) {
    printf("Antijuggler could not open file for receiving/writing\n");
    exit(-1);
  }
  antistate.logfd = open("/var/log/badmacs",O_CREAT|O_WRONLY);
  if(antistate.logfd == 0L){
	  printf("Antijuggler could not open badmacs for writing\n");
	  exit(-1);
  }
  antistate.bitCount= 0;
  getLocalMac();
}

void exit_juggler() {
  fclose(state.fp);
}

void exit_antijuggler() {
  fclose(antistate.fp);
}


int main(int argc, char *argv[])
{
  printf("PERMEATE Copyright (C) 2008 Jarek Paduch, Bilal Khan, Jamie Levy\n");
  printf("   This program comes with ABSOLUTELY NO WARRANTY.\n");
  printf("   This is free software, and you are welcome to redistribute it\n");
  printf("   under the conditions of the GNU General Public License.\n\n");

  if(argc < 2)
  {
     printf("Usage: bb <remote IP>\n");
     exit(-1);
  }


  // initialize
  remoteip = argv[1]; 
  init_juggler();
  init_antijuggler();
  // main
  packet_processing();

  // shutdown 
  exit_juggler();
  exit_antijuggler();

  exit(-1);
}
