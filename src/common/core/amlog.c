
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include <mint/osbind.h>

#ifdef ENABLE_GEMDOS_IO
#include "fmio.h"
#include <mint/ostruct.h>
#endif

#include "amlog.h"

// debugging OUTPUT settings
#define OUTPUT_BUFFER_SIZE 1024
#define DEBUG_LOG "LOG.TXT"

#if defined(DEBUG_FILE_OUTPUT)

#ifdef ENABLE_GEMDOS_IO
static int16 fh=GDOS_OK;
#else
static FILE *ofp=0;
#endif

#endif


//todo: console output to RAWCON(2(CON)->5(RAWCON))
// 1 - serial,
// 6 - st compatible serial (Modem 1, works with hatari),
// 7 - scc (modem 2, doesn't work under hatari)

static const  uint8 SERIAL_OUTPUT_CHANNEL=7;

#if (defined(DEBUG)||defined(DEBUG_BUILD)||defined(DEBUG_SERIAL_OUTPUT)||defined(DEBUG_FILE_OUTPUT))
static char buffer[OUTPUT_BUFFER_SIZE];
#endif

#if (defined(DEBUG_BUILD)||defined(DEBUG_SERIAL_OUTPUT)||defined(DEBUG_FILE_OUTPUT))
void logd(const char *mes,...){

#if (defined(DEBUG_SERIAL_OUTPUT)||defined(DEBUG_CONSOLE_OUTPUT)||defined(DEBUG_FILE_OUTPUT))
    int bBytesNotSent=1;
    int bytesSent=0;
    int len=0;
#endif

    va_list va;
    va_start(va,mes);
    vsprintf(buffer,(const char *)mes,va);
    va_end(va);

#if (defined(DEBUG_CONSOLE_OUTPUT)||defined(DEBUG_SERIAL_OUTPUT)||defined(DEBUG_FILE_OUTPUT))
    len=strlen(buffer);
#endif

#ifdef DEBUG_CONSOLE_OUTPUT
    if(len) printf(buffer);
#endif

#ifdef DEBUG_SERIAL_OUTPUT
    while(bBytesNotSent){
      if( Bcostat(SERIAL_OUTPUT_CHANNEL) == -1 ){
        Bconout(SERIAL_OUTPUT_CHANNEL,(short int)buffer[bytesSent] );
        ++bytesSent;
      }

     if((len)==bytesSent)bBytesNotSent=0;
    }
#endif

#ifdef DEBUG_FILE_OUTPUT

#ifdef ENABLE_GEMDOS_IO

if(len){
    if(Fwrite(fh,len,buffer)<0){
        printf("[GEMDOS] Error: %s\n",getGemdosError(fh));
    }
}

#else
    fprintf(ofp,buffer);
    fflush(ofp);
#endif

#endif
 return;
}
#endif

#if defined(DEBUG_SERIAL_OUTPUT_ONLY)

void serialLog(const char *mes,...){
  int bSerialNotSent=1;
  int bytesSent=0;

  va_list va;
  va_start(va,mes);
  vsprintf(buffer,(const char *)mes,va);
  va_end(va);

  int len=strlen(buffer);

  while(bSerialNotSent){
      if( Bcostat(SERIAL_OUTPUT_CHANNEL) == -1 ){
        Bconout(SERIAL_OUTPUT_CHANNEL,(short int) buffer[bytesSent] );
        ++bytesSent;
      }

     if((len)==bytesSent) bSerialNotSent=0;
  }
}

#endif

void initDebug(const char *pFilename){

#ifdef DEBUG_FILE_OUTPUT

#ifdef ENABLE_GEMDOS_IO
    fh=Fcreate(pFilename,0);

    if(fh<0){
        printf("Can't create debug file: %s\n",DEBUG_LOG);
        printf("[GEMDOS] Error: %s\n",getGemdosError(fh));
    }else{
        printf("Init debug [%d]\n",fh);
    }

#else
    ofp=NULL;
    ofp=fopen(pFilename,"w+b");

    if(ofp==NULL){
        printf("Can't init file output: %s\n",DEBUG_LOG);
    }
#endif

#endif

#ifdef DEBUG_SERIAL_OUTPUT
    Bconmap(SERIAL_OUTPUT_CHANNEL);
#endif

 return;
}

void deinitDebug(){

#if defined(DEBUG_FILE_OUTPUT)

#ifdef ENABLE_GEMDOS_IO
    printf("Deinit debug: [%d] \n", fh);

    int16 err=Fclose(fh);

    if(err!=GDOS_OK){
      printf("[GEMDOS] Error closing file handle : [%d] %s\n", fh, getGemdosError(err));
    }

#else
    fflush(ofp);
    fclose(ofp);
#endif

#endif

 return;
}
