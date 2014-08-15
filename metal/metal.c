/*
 *     This program is free software; you can redistribute it and/or
 *     modify it under the terms of the GNU General Public License
 *     as published by the Free Software Foundation; either version
 *     2 of the License, or (at your option) any later version.
 *
 *  Author: Jamie Levy
 *  jamie.levy@gmail.com
 *  
 *  METAL
 *  	metal.c
 *  Modified: 5/22/07
 *
 */


#include "metal.h"
#define NO_OP 0		/* This was used for expirements */

/*
 *  settimer()
 *  Function allows one to set the library timer to a 
 *		different value than the default
 *
 *  Precondition: int val, value of the library timer
 *  Postcondition: library timer is set to the 
 *		new timer value
 */
void settimer(int val){		
  TIMERVAL = val;
}


/*
 *  crash()
 *	Function prints out an error message and kills 
 *		running program
 *
 *  Precondition: char mess[], error message to display
 *  Postcondition: Error message is displayed and 
 *		program exits
 */
void crash(char mess[])
{
  perror(mess);
  exit(1);
}

/*
 *  init()
 *	Initialization function.  All signal handlers 
 *		and variables are initialized here for the 
 *		first time.
 *
 *	Precondition: None
 *	Postcondition: All variables are initialized
 */

void init(){
 /* set pagesize */
  pagesize = getpagesize();

  /* get and set page mask */
  myhandler.mask = 0xffffffff ^ ( pagesize - 1 );

  /* set random seed for key */
  srand( time( NULL ));

  /* initialize mutex */
  pthread_mutex_init(&mutex, NULL);

  /* set up signal handlers */
  handlersetup();

  /* set first flag to mark that we have initialized everything */
  first = 1;

  /* allocate memory for memory manager and encryption handler */
  memSlots =  mmap( NULL, 5 * pagesize,
              PROT_READ|PROT_WRITE,
              MAP_PRIVATE|MAP_ANONYMOUS,0,0);
  encrHandler = mmap( NULL, 3 * pagesize,
              PROT_READ|PROT_WRITE,
              MAP_PRIVATE|MAP_ANONYMOUS,0,0);

   /* initialize keys and address sizes */
   encrHandler[0].K = (rand() % 104729)+1;    /* eventually we want 1 key per page */
   myhandler.K = encrHandler[0].K;
   myhandler.totalNumAddrs = 0;
   myhandler.encrNumAddrs = 0;
   myhandler.encr_size = 5 * pagesize / sizeof( encrHandler[0] );
   myhandler.total_size = 3 * pagesize /sizeof( memSlots[0] );

}

/*
 *  handlersetup()
 *	Sets up signal handlers for SEGV and ALRM
 *
 *  Precondition: None
 *  Postcondition: Signal handlers are set up
 */
void handlersetup( ){
  /*
   * set up a signal handlers for SEGV and ALRM
   */
  sigset_t    newset;
  struct sigaction act, alact;
  act.sa_sigaction = &segvhandler;
  alact.sa_handler = &catchalarm;
  
  sigfillset(&newset);		/* let's do full mask */
  sigdelset(&newset, SIGINT);	/* and delete INT in case we get in trouble... */
  sigdelset(&newset, SIGUSR1);
  
  act.sa_flags = SA_SIGINFO;
  act.sa_restorer = NULL;
  
  alact.sa_mask = newset;
  act.sa_mask = newset;
  alact.sa_flags = 0;
  alact.sa_flags |= SA_INTERRUPT;
  
  if (sigaction(SIGSEGV, &act, NULL) != 0)
    crash("sigaction");
  
  if( sigaction( SIGALRM, &alact, NULL) != 0 )
    crash("sigaction 2");
  
}

/*
 *  extend1()
 *	Memory extension program for memory management handler.
 *		Called when more memory is needed to store page information
 *
 *	Precondtion: oldptr: Pointer to current memory management handler
 *		newSize: newsize of memory management handler.
 *	Postcondition: memory is extended for the memory management handler
 *		to the newSize variable given and the new pointer is returned
 */
struct memory *extend1( struct memory *oldptr, size_t newSize ){
    if( oldptr ){
        struct memory *newptr = mmap( NULL, newSize, 
                                PROT_READ|PROT_WRITE,
                                MAP_PRIVATE|MAP_ANONYMOUS,0,0),
                      *temp;

		/* copy over old contents */
        memcpy( newptr, oldptr, myhandler.total_size);

		/* set remaining memory to all zeroes */
        memset(&(((char *)newptr)[myhandler.total_size]),0, newSize - myhandler.total_size);

		/* switch out the pointers */
        temp = newptr;
        newptr = oldptr;
        oldptr = temp;
        temp = NULL;

		/* delete the old address space and update size */
        munmap( newptr, myhandler.total_size * sizeof( memSlots[0]) );
        myhandler.total_size = newSize/sizeof( memSlots[0]);

		/* return the new pointer to the call */
        return oldptr;
    }else{
		/* else return NULL if pointer was already NULL */
        return NULL;
    }
}

/*
 *  extend2()
 *	Memory extension program for encryption/decryption handler.
 *		Called when more memory is needed to store page information
 *
 *	Precondtion: oldptr: Pointer to current encryption/decryption handler
 *		newSize: newsize of encryption/decryption handler.
 *	Postcondition: memory is extended for the encryption/decryption handler
 *		to the newSize variable given and the new pointer is returned
 */
struct decr_handler *extend2( struct decr_handler *oldptr, size_t newSize ){
    if( oldptr ){
        struct decr_handler *newptr = mmap( NULL, newSize,
                                PROT_READ|PROT_WRITE,
                                MAP_PRIVATE|MAP_ANONYMOUS,0,0),
                            *temp;
		
		/* copy over old contents */
        memcpy( newptr, oldptr, myhandler.encr_size);

		/* set remaining memory to all zeroes */
        memset(&(((char *)newptr)[myhandler.encr_size]),0, newSize - myhandler.encr_size);

		/* switch out the pointers */
        temp = newptr;
        newptr = oldptr;
        oldptr = temp;
        temp = NULL;

		/* delete the old address space and update size */
        munmap( newptr, myhandler.encr_size * sizeof( encrHandler[0] ));
        myhandler.encr_size = newSize / sizeof( encrHandler[0]);

		/* return the new pointer to the call */
        return oldptr;
    }else{
		/* else return NULL if pointer was already NULL */
        return NULL;
    }
}

/*  setMemSlots()
 *  Function to initialize the current memory slot
 *
 *	Precondition: size: size of the memory allocated
 *		*all:  allocated address
 *		index:  location for page information in memory manager
 *	Postcondition: page information is saved in the memory manager
 */
void setMemSlots(int size, void *all, int index){
    memSlots[index].del = 0;
    memSlots[index].size = size;
    memSlots[index].addrs = all;
}

/*
 *  lock()
 *  Function for critical section accesses to 
 *		memory allocation
 */
static void lock() {
  if (pthread_mutex_trylock(&mutex)) {
    if (mutexpid==getpid()) {
      locknr++;
      return;
    } else {
      pthread_mutex_lock(&mutex);
    }
  } 
  mutexpid=getpid();
  locknr=1;
}

/*
 *  unlock()
 *  Function for critical section accesses to 
 *		memory allocation
 */
static void unlock() {
  locknr--;
  if (!locknr) {
    mutexpid=0;
    pthread_mutex_unlock(&mutex);
  }
}

/*****************************************************
 *****************************************************
 *************	Memory allocation functions **********
 *************	malloc()
 *************	valloc()
 *************	calloc()
 *************	realloc()
 *****************************************************
 *****************************************************/

/*
 *  malloc()
 *
 *	Precondition: size: size of buffer to allocate
 *	Postcondition: pointer to allocated buffer is
 *		returned
 */
extern C_LINKAGE void *malloc(size_t size) {
  if( !first )
	/* initialize everything if this is the very first malloc() call */
    init();
  else
    alarm(0);
  
  lock();

  /* Allocate the buffer using mmap() */
  caddr_t allocation = (caddr_t) mmap(NULL,
				      size,
				      PROT_READ|PROT_WRITE,
				      MAP_PRIVATE|MAP_ANONYMOUS,
				      0,0);
  
  /* Update memory manager if we have room for the new information */
  if( myhandler.totalNumAddrs < myhandler.total_size){
    setMemSlots(size, allocation, myhandler.totalNumAddrs);
    myhandler.totalNumAddrs++;
  }else{
	/* Else use first fit algorithm to check for holes in memory
	 * If one is found, we save the information in this location 
	 */
    int i;
    for(i = 0; i < myhandler.total_size; i++){
      if(memSlots[i].del == 1 ){
        setMemSlots(size, allocation, myhandler.totalNumAddrs);
	    break;
      }
    }
    
	/* Extend memory if we are out of room and place the 
	 * new information on the end
	 */
    if( i >= myhandler.total_size){ 
      memSlots = extend1(memSlots, myhandler.total_size * sizeof(memSlots[0]) + pagesize);
      setMemSlots(size, allocation, myhandler.totalNumAddrs);
    }
  }
  
  /* If there is room for this page's information on the encryption 
   * handler, then mprotect it with PROT_NONE
   */
  if( !NO_OP && myhandler.encrNumAddrs < myhandler.encr_size ){
    if ( mprotect(allocation, size, PROT_NONE) < 0 )
      crash("mprotect - malloc");
  }else{
	/* Else extend encryption handler memory and then do mprotect */
	if( myhandler.encrNumAddrs < myhandler.encr_size )
		encrHandler = extend2( encrHandler, myhandler.encr_size * sizeof(encrHandler[0]) + pagesize);
	if ( mprotect(allocation, size, PROT_NONE) < 0 )
      crash("mprotect - malloc");
  }
  
  unlock();
  
  /* return buffer */
  return allocation;
}

/* 
 *  valloc()
 *
 *	Precondition: size: size of buffer to allocate
 *	Postcondition: pointer to allocated buffer is
 *		returned
 */
extern C_LINKAGE void *valloc(size_t size){
  lock();
  void *allocation = malloc( size );
  unlock();
  
  return allocation;
}

/* 
 *  realloc()
 *
 *	Precondition: size: size of buffer to allocate
 *		newSize: new size of allocated buffer
 *	Postcondition: pointer to reallocated buffer is
 *		returned
 */
extern C_LINKAGE void *realloc(void *oldBuff, size_t newSize){
  ualarm(0,0);
  alarm(0);
  /*  I don't think this part is needed... */
  sigset_t    newmask, oldmask;
  sigemptyset( &newmask );
  sigaddset( &newmask, SIGALRM );
  if ( sigprocmask( SIG_BLOCK, &newmask, &oldmask ) < 0 )
    crash( "SIG_BLOCK error" );
  
  
  void *newBuff = malloc( newSize );
  
  if (mprotect(newBuff, newSize, PROT_READ | PROT_WRITE) != 0)
    crash("mprotect REALLOC");
  
  lock();
  
  int i, size;
  if( oldBuff ){
	/* Look for buffer address in memory handler 
	 * and get size 
	 */
    for( i = 0; i < myhandler.total_size; i++ ){
      if( oldBuff == memSlots[i].addrs ){
		size = memSlots[i].size;
		break;
      }
    }
    if( i >= myhandler.total_size )
      crash("address not from malloc!");
    
    if( newSize < size )
      size = newSize;
    
    if( size > 0 )
      memcpy( newBuff, oldBuff, size );
    
    
    free( oldBuff );
    
    if( size < newSize )
      memset(&(((char *)newBuff)[size]), 0, newSize - size);    
    
  }   
  
  unlock();
  
  if( sigprocmask( SIG_SETMASK, &oldmask, NULL ) < 0 )
    crash( "SIG_SETMASK error" );
  
  
  return newBuff;
}


/* 
 *  calloc()
 *
 *	Precondition: size: size of buffer to allocate
 *	Postcondition: pointer to allocated buffer is
 *		returned
 */
extern C_LINKAGE void *calloc(size_t nelem, size_t elsize) {
  size_t  size; 
  caddr_t	allocation;
  
  size  = nelem * elsize;
  
  lock();
  allocation = malloc( size );
  
  if (mprotect(allocation, size, PROT_READ | PROT_WRITE) != 0)
    crash("mprotect CALLOC");
  
  memset( allocation, 0, size );
  
  unlock();
  
  return allocation;
}

/*****************************************************
 *****************************************************
 ***************	End Allocation Functions  ********
 *****************************************************
 *****************************************************/


/*****************************************************
 *****************************************************
 ************ Memory Deallocation functions **********
 ************	cfree()
 ************	free()
 *****************************************************
 *****************************************************/

/*
 *	cfree()
 *	Frees a pointer
 *
 *	Precondition: freeptr: pointer to be freed
 *	Postcondtion: freeptr is freed by calling free()
 */
extern C_LINKAGE void cfree( void *freeptr ){
  free( freeptr );
}

/*
 *	cfree()
 *	Frees a pointer
 *
 *	Precondition: freeptr: pointer to be freed
 *	Postcondtion: freeptr is freed by calling 
 *		munmap()
 */
extern C_LINKAGE void free( void *freeptr ){
  ualarm(0,0);
  alarm(0);
  
  /* Don't think this is needed... */
  sigset_t    newmask, oldmask;
  sigemptyset( &newmask );
  sigaddset( &newmask, SIGALRM );
  if ( sigprocmask( SIG_BLOCK, &newmask, &oldmask ) < 0 )
    crash( "SIG_BLOCK error" );
  
  lock();
  if( freeptr ){
    if( myhandler.totalNumAddrs == 0 )
      crash("free before first malloc!");
    int i, j;
    ptr = freeptr;
    ptr2 = freeptr;
    
	/* search for pointer in memory handler */
    for( i = 0; i < myhandler.total_size; i++ ){
      if( memSlots[i].addrs == freeptr ){
		/* if found set delete to true and 
		 * erase page info from encryption handler 
		 */
		memSlots[i].del = 1;
		zero(ptr);

		/* If the buffer is more than one page, 
		 * we have to erase each page from the 
		 * encryption handler
		 */
		for( j = 0; j < memSlots[i].size; j += pagesize ){
		   if( (ptr + pagesize) <= (ptr2 + memSlots[i].size) ){
				ptr += pagesize;
				zero(ptr);
		   }
		}
	
		/* Erase the page */
		munmap(freeptr, memSlots[i].size);
		break;
      }
    }
    if( i == myhandler.total_size )
      crash("address not from malloc!");
    
  }
  unlock();
  
  if( sigprocmask( SIG_SETMASK, &oldmask, NULL ) < 0 )
    crash( "SIG_SETMASK error" );
  
  ualarm(TIMERVAL, 0);
    return;
  }

/*****************************************************
 *****************************************************
 ***************  End Deallocation Functions  ********
 *****************************************************
 *****************************************************/

/*
 *	zero()
 *	Function to clear deleted pages from the 
 *	decryption/encryption handler
 *
 *	Precondition: zeroptr: address to be 
 *		cleared from the decryption/encryption handler
 *	Postcondition: zeroptr is cleared from the 
 *		decryption/encryption handler
 */
void zero(char *zeroptr){
  int i;
  for(i = myhandler.encrNumAddrs; i > 0; i-- ){
    if( encrHandler[i].addrs == zeroptr ){
      encrHandler[i].del = 0;
      break;
    }
  }
}

/* 
 *	decr()
 *	Function to decrypt a page
 *
 *	Precondition: *thing: address to be decrypted
 *		size: size of the buffer to be decrypted
 *	Postcontion:  the page is decrypted
 */
void decr(void *thing, int size){
  int i;
  char *temp;
  temp = thing;
  
  if (mprotect(thing, size, PROT_READ | PROT_WRITE) != 0)
    crash("mprotect DECR");
  
  for( i = 0; i < size; i++ ){
    if (!NO_OP) temp[i] = myhandler.K ^ temp[i];
  }
  
  temp = NULL;
  
}

/* 
 *	encr()
 *	Function to encrypt a page
 *
 *	Precondition: *thing: address to be encrypted
 *		size: size of the buffer to be encrypted
 *	Postcontion:  the page is encrypted
 */
void encr(void *thing, int size){
  int i;
  char *temp;
  temp = thing;
  
  for( i = 0; i < size; i++ ){
    if (!NO_OP) temp[i] = myhandler.K ^ temp[i];
  }

  if (mprotect(thing, size, PROT_NONE) !=  0)
    crash("mprotect ENCR");
  
  temp = NULL;
  
}

/*
 *	catchalarm()
 *	SIGALRM signal handler
 *
 */
void catchalarm( int sig ){
  ualarm(0, 0);
  alarm(0);		/* just in case */
			/* though both may not be needed */
  if( myhandler.encrNumAddrs == 0 ){
    return;
  }
  
  
  for ( ; myhandler.encrNumAddrs > 0; myhandler.encrNumAddrs-- ){
    if( encrHandler[myhandler.encrNumAddrs].del != 0 ){ 
      encr( encrHandler[myhandler.encrNumAddrs].addrs, pagesize );
    }
    encrHandler[myhandler.encrNumAddrs].addrs = 0;
    encrHandler[myhandler.encrNumAddrs].del = 0;
    encrHandler[myhandler.encrNumAddrs].decr = 0;
  }
  
  return;
}


/*
 *	segvhandler()
 *	SIGSEGV signal handler
 *
 */

void segvhandler(int signum, siginfo_t *info, void *extra){

  if( signum == SIGSEGV){
	alarm(0);				/* just in case */
	ualarm(0, 0);
  
	ptr = (void *)(((int)info->si_addr) & myhandler.mask);
  
	if( myhandler.encrNumAddrs < myhandler.encr_size ){
		myhandler.encrNumAddrs++;
		encrHandler[myhandler.encrNumAddrs].addrs = ptr; 
		encrHandler[myhandler.encrNumAddrs].del = 1;
		encrHandler[myhandler.encrNumAddrs].decr = 1;
	}

	decr(encrHandler[myhandler.encrNumAddrs].addrs, pagesize);
  
  
	alarm(0);
	ualarm(0,0);
	ualarm(TIMERVAL, TIMERVAL);
  }

  return;
  
}


