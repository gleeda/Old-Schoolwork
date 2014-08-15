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

#ifndef METAL_H
#define METAL_H

#include <sys/mman.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <memory.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/param.h>

#ifdef  malloc
#undef  malloc
#endif

#ifdef  calloc
#undef  calloc
#endif

#ifdef  realloc
#undef  realloc
#endif

#ifdef  valloc
#undef  valloc
#endif

#ifdef  free
#undef  free
#endif

#ifdef  __cplusplus
#define C_LINKAGE       "C"
#else
#define C_LINKAGE
#endif

static int TIMERVAL = 5000;		/* default value of timer */

char 		            	*ptr, *ptr2;
int 	    	        	pagesize;
static pthread_mutex_t  	mutex ;
static pid_t 	        	mutexpid = 0;
static int 	            	locknr = 0;
static int	            	first = 0;


/*
 *  Our defined structs
 */

struct decr_handler{        /* decr/encr handler type */
    void                    *addrs;     /* Page addresses in decrypted state */
    int                     del;        /* In case page is deleted */
    unsigned int            K;          /* Key */
    int                     decr;       /* In decr state Not needed */
};

struct{                     /* struct for number of addreses and stuff */
    int                     totalNumAddrs;      /* total memory addresses */
    int                     encrNumAddrs;       /* total of encrypted pages */
    int                     total_size;         /* Maximum size in pages of memory */
    int                     encr_size;          /* Maximum size in pages of encrypted pages */
    unsigned int            mask;               /* Page Mask */
    unsigned int            K;                  /* Key */
}myhandler;



struct memory{              /* Memory manager struct */
    void                    *addrs;     /* Addresses of all allocated pages */
    int                     size;       /* size of each page */
    int                     del;        /* whether or not page is deleted */
};

struct memory               *memSlots;      /* Memory manager handler */
struct decr_handler         *encrHandler;   /* encrypter handler */

struct memory               *extend1(struct memory *oldptr, size_t newSize);
struct decr_handler         *extend2(struct decr_handler *oldptr, size_t newSize);

void                        setMemSlots( int size, void *all, int index );

void                        settimer(int val);

/*
 *
 *  Function to output errors and end program  
 *
 */

void 		crash(char mess[]);

/*
 *
 *  Encryption and decryption functions
 *
 */
void 		encr(void *thing, int size);
void 		decr(void *thing, int size);

/*
 *
 *  SEGV and ALRM handlers
 *
 */
void 		segvhandler(int signum, siginfo_t *info, void *extra);
void 		catchalarm( int sig );


/*
 *
 *  Overwritten malloc and calloc functions
 *
 */
extern C_LINKAGE void       *malloc(size_t size);
extern C_LINKAGE void       *calloc(size_t nelem, size_t elsize);
extern C_LINKAGE void       *valloc(size_t size);
extern C_LINKAGE void       *realloc(void *oldBuff, size_t newSize);
extern C_LINKAGE void       free( void *freeptr );
extern C_LINKAGE void       cfree(void *freeptr );

static void                     lock();
static void                     unlock();

/*
 *
 * Initialization functions 
 *
 */
void                            init();
void                            handlersetup();

#endif

