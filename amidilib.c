/**  Copyright 2007-2010 Pawel Goralski
    e-mail: pawel.goralski@nokturnal.pl
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <mint/ostruct.h>
#include "include/amidilib.h"
#include "include/mfp.h"
#include "include/list/list.h"
#include <limits.h>

extern volatile sEventItem *g_pEventPtr;
extern volatile U32 iCurrentDelta;

/* current version */
typedef struct AMIDI_version {
	U16 major;
	U16 minor;
	U16 patch;
} AMIDI_version;

static AMIDI_version version =
	{ AMIDI_MAJOR_VERSION, AMIDI_MINOR_VERSION, AMIDI_PATCHLEVEL };

/* for saving last running status */
static U8 g_runningStatus;

static U8 outputFilename[] = "amidi.log";
static BOOL CON_LOG;
static sSequence_t *gpCurrentSequence;

/* variables for debug output to file */
#ifdef DEBUG_FILE_OUTPUT
static FILE *ofp;
#endif

/* static table with MIDI controller names */
extern const U8 *g_arMIDIcontrollers[];

/* static table with MIDI notes to ASA ISO */
extern const char *g_arMIDI2key[];

S16 am_getHeaderInfo(void *pMidiPtr){
 sMThd midiInfo;

 memcpy(&midiInfo, pMidiPtr, sizeof(sMThd));

    /* check midi header */
    if(((midiInfo.id)==(ID_MTHD)&&(midiInfo.headLenght==6L))){

        switch(midiInfo.format){
	 case 0:
	  /* Midi Format 0 detected */
	  return (0);
	 break;

	case 1:
	 /* Midi Format 1 detected */
	  return(1);
	 break;

        case 2:
	/* Midi Format 2 detected */
	  return(2);
	break;

	default:
	/*Error: Unsupported MIDI format */
	  return(-2);
	break;
    }

   }
   else if ((midiInfo.id==ID_FORM)||(midiInfo.id==ID_CAT)){
    /* possible XMIDI*/
    return(3);
   }
    
return(-1);
}

S16 am_handleMIDIfile(void *pMidiPtr, S16 type, U32 lenght, sSequence_t *pSequence){
    S16 iNumTracks=0;
    U16 iTimeDivision=0;
    U32 ulAddr;
   
    void *startPtr=pMidiPtr;
    void *endPtr=0L;

    /* calculate end pointer */
    ulAddr=(U32)startPtr+lenght*sizeof(U8);
    endPtr=(void *)ulAddr;

    pSequence->pSequenceName=NULL;	 	/* name of the sequence empty string */
    pSequence->ubNumTracks=0;		 	/*  */
    pSequence->currentState.ubActiveTrack=0; 	/* first one from the array */
    pSequence->uiTimeDivision=DEFAULT_PPQ;	/* PPQN */
    
    /* init sequence table */
    for(U16 iLoop=0;iLoop<AMIDI_MAX_TRACKS;iLoop++){
      /* we will allocate needed track tables when appropriate */
      pSequence->arTracks[iLoop]=NULL;
    }
    
    switch(type){
    /* TODO: refactor this mess */
        case 0:{
            /* handle MIDI type 0 */
            iNumTracks=am_getNbOfTracks(pMidiPtr,type);

            if(iNumTracks!=1){
	      return(-1);
	    } /* invalid number of tracks, there can be only one! */
            else{
                 /* prepare our structure */
		 pSequence->ubNumTracks=iNumTracks;	/* one by default */
		 
		 /* OK! valid number of tracks */
                 /* get time division for timing */
                 iTimeDivision = am_getTimeDivision(pMidiPtr);
                 /* process track data, offset the start pointer a little to get directly to track data and decode MIDI events */
                 startPtr=(void *)((U32)startPtr+sizeof(sMThd));
       
		 /* Store time division for sequence, TODO: SMPTE handling */
		  pSequence->uiTimeDivision=am_decodeTimeDivisionInfo(iTimeDivision);	/* PPQN */
                  /* create one track list only */
		  pSequence->arTracks[0] = (sTrack_t *)malloc(sizeof(sTrack_t));
		  
		   while (((startPtr!=endPtr)&&(startPtr!=NULL))){
		  /* Pointer to midi data, 
		     type of midi to preprocess, 
		     number of tracks, 
		     pointer to the structure in which track data will be dumped (or not).  
		  */
		  startPtr=processMidiTrackData(startPtr,T_MIDI0,1, pSequence);
                 }
            }
         return(0);
        }
        break;

        case 1:{
         /* handle MIDI type 1 */
	 /* several tracks, one sequence */
	 /* prepare our structure */
	  pSequence->ubNumTracks=1;	/* one by default */
	  iNumTracks=am_getNbOfTracks(pMidiPtr,type);
	  iTimeDivision = am_getTimeDivision(pMidiPtr);
          startPtr=(void *)((U32)startPtr+sizeof(sMThd));
                	
	  /* Store time division for sequence, TODO: SMPTE handling */
	  pSequence->uiTimeDivision=am_decodeTimeDivisionInfo(iTimeDivision);	/* PPQN */
	  /* create one track list only */
	  pSequence->arTracks[0] = (sTrack_t *)malloc(sizeof(sTrack_t));
	  /*assert(pCurSequence->arTracks[0]>0);*/

	  (pSequence->arTracks[0])->pInstrumentName=NULL;
	  (pSequence->arTracks[0])->currTrackState.currentPos=0;
	  (pSequence->arTracks[0])->currTrackState.ubVolume=128;                
	  //(pSequence->arTracks[0])->currTrackState.ubPlayModeState=0;	/* IDLE state */
	  (pSequence->arTracks[0])->currTrackState.ulTimeStep=128;	/* sequence current track tempo */
	  
	  /* init event list */
	  initEventList(&((pSequence->arTracks[0])->trkEventList));
           
          while (((startPtr!=endPtr)&&(startPtr!=NULL))){
	    startPtr=processMidiTrackData(startPtr,T_MIDI1, iNumTracks, pSequence);
          }
	  return(0);
        }
        break;

        case 2:{
		/* handle MIDI type 2 */
		/* several tracks not tied to each others tracks */

		iNumTracks=am_getNbOfTracks(pMidiPtr,type);
		iTimeDivision = am_getTimeDivision(pMidiPtr);
		startPtr=(void *)((U32)startPtr+sizeof(sMThd));
		               
		/* TODO: fill in proper value based on timedivision and PPQ/SMPTE */
		(pSequence->arTracks[0])->currTrackState.ulTimeStep=128;	

		/* Store time division for sequence, TODO: SMPTE handling */
		pSequence->uiTimeDivision=am_decodeTimeDivisionInfo(iTimeDivision);	/* PPQN */

                while (((startPtr!=endPtr)&&(startPtr!=NULL))){
                  startPtr=processMidiTrackData(startPtr,T_MIDI2,iNumTracks,pSequence);
                }
             return(0);
            }
        break;
	case 3:{
         /* handle XMIDI */
         iNumTracks=am_getNbOfTracks(pMidiPtr,type);
         iTimeDivision = am_getTimeDivision(pMidiPtr);

         /* processing (X)MIDI file */
	/* TODO: handle + process */

         return(0);
        }

        break;
	default:;
	/* unsupported file type */
 }
 return(-1);
}

S16 am_getNbOfTracks(void *pMidiPtr, S16 type){
 sMThd midiInfo;
 IFF_Chunk xmidiInfo;
 U32 ulNextChunk=0;
 U32 ulChunkOffset=0;
 U8 *Ptr=NULL;

    switch(type){
     case T_MIDI0:
     case T_MIDI1:
     case T_MIDI2:{
         memcpy(&midiInfo, pMidiPtr, sizeof(sMThd));

        /* check midi header */
        if(((midiInfo.id)==(ID_MTHD)&&(midiInfo.headLenght==6L))){
	  return (midiInfo.nTracks);
        }
     }
     break;

     case T_XMIDI:{
        /*TODO: ! not implemented */
     }
     case T_RMID:{
     return -1;/*TODO: ! not implemented */
     }break;
     case T_SMF:{
      return -1;/*TODO: ! not implemented */
     }break;
     case T_XMF:{
      return -1;/*TODO: ! not implemented */
     } break;
     
     case T_SNG:{
      return -1;/*TODO: ! not implemented */
    }break;
     case T_MUS:
       return -1;/*TODO: ! not implemented */
     
     break;
     default:
       return -1;/*TODO: ! not implemented */
    }
return -1;
}

U16 am_getTimeDivision (void *pMidiPtr){
sMThd midiInfo;
memcpy(&midiInfo, pMidiPtr, sizeof(sMThd));

/* check midi header */
if(((midiInfo.id)==(ID_MTHD)&&(midiInfo.headLenght==6L))){
  return (midiInfo.division);
}
    /* (X)Midi has timing data inside midi eventlist */

 return (0);
}


S16 am_getTrackInfo(void *pMidiPtr, U16 usiTrackNb, sMIDItrackInfo *pTrackInfo){
 return(0);
}

void *am_getTrackPtr(void *pMidiPtr,S16 iTrackNum){
 return NULL;
}

U8 am_calcRolandChecksum(U8 *buf_start, U8 *buf_end){
U8 total = 0 ;
U8 mask  = 0x7F ;

while ( buf_start <= buf_end ){
  total += *buf_start ;
  buf_start++ ;
}

 return (0x80 - (total & mask)) & mask ;
}

static U8 g_arMidiBuffer[MIDI_BUFFER_SIZE];

/* Midi buffers system info */
static _IOREC g_sOldMidiBufferInfo;
static _IOREC *g_psMidiBufferInfo;

S16 am_init(){
 S32 iCounter=0;

#ifdef DEBUG_CONSOLE_OUTPUT
  CON_LOG=TRUE;
#else
  CON_LOG=FALSE;
#endif
 
 am_setSuperOn();

 /* if file output enabled open debug file */
#ifdef DEBUG_FILE_OUTPUT
 ofp = fopen((const char *)outputFilename, "w");

 if (ofp == NULL) {
   fprintf(stderr,"Can't init debug file output to %s!\n",outputFilename);
 }
 #endif
 
 /* init sequence */
 /* nothing loaded, nothing played */
 
 gpCurrentSequence=NULL;
 
 /* clear our new buffer */
 for(iCounter=0;iCounter<(MIDI_BUFFER_SIZE-1);iCounter++){
   g_arMidiBuffer[iCounter]=0x00;
  } 
  g_psMidiBufferInfo=(_IOREC*)Iorec(XB_DEV_MIDI);
		
 /* copy old MIDI buffer info */
 g_sOldMidiBufferInfo.ibuf=(*g_psMidiBufferInfo).ibuf;
 g_sOldMidiBufferInfo.ibufsiz=(*g_psMidiBufferInfo).ibufsiz;
 g_sOldMidiBufferInfo.ibufhd=(*g_psMidiBufferInfo).ibufhd;
 g_sOldMidiBufferInfo.ibuftl=(*g_psMidiBufferInfo).ibuftl;
 g_sOldMidiBufferInfo.ibuflow=(*g_psMidiBufferInfo).ibuflow;
 g_sOldMidiBufferInfo.ibufhi=(*g_psMidiBufferInfo).ibufhi;

 /* set up new MIDI buffer */
 (*g_psMidiBufferInfo).ibuf = (char *)g_arMidiBuffer;
 (*g_psMidiBufferInfo).ibufsiz = MIDI_BUFFER_SIZE;
 (*g_psMidiBufferInfo).ibufhd=0;	/* first byte index to write */
 (*g_psMidiBufferInfo).ibuftl=0;	/* first byte to read(remove) */
 (*g_psMidiBufferInfo).ibuflow=(U16)MIDI_LWM;
 (*g_psMidiBufferInfo).ibufhi=(U16)MIDI_HWM;
 am_setSuperOff();
 return(1);
}

void am_deinit(){
  am_setSuperOn();
  /* restore standard MIDI buffer */
  (*g_psMidiBufferInfo).ibuf=g_sOldMidiBufferInfo.ibuf;
  (*g_psMidiBufferInfo).ibufsiz=g_sOldMidiBufferInfo.ibufsiz;
  (*g_psMidiBufferInfo).ibufhd=g_sOldMidiBufferInfo.ibufhd;
  (*g_psMidiBufferInfo).ibuftl=g_sOldMidiBufferInfo.ibuftl;
  (*g_psMidiBufferInfo).ibuflow=g_sOldMidiBufferInfo.ibuflow;
  (*g_psMidiBufferInfo).ibufhi=g_sOldMidiBufferInfo.ibufhi;
  am_setSuperOff();
  
  /* close debug file output */
  #ifdef DEBUG_FILE_OUTPUT
  fclose(ofp);
  #endif
 /* end sequence */
}

void am_dumpMidiBuffer(){
 
  if(g_arMidiBuffer[0]!=0){
  amTrace((const U8*)"MIDI buffer dump:\n %s",g_arMidiBuffer);
 }
}


/* at this point pCurSequence should have the info about the type of file that resides in memory,
because we have to know if we have to dump event data to one eventlist or several ones */

/* all the events found in the track will be dumped to the sSequenceState_t structure  */

void *processMidiTrackData(void *startPtr, U32 fileTypeFlag,U32 numTracks, sSequence_t *pCurSequence){   
sChunkHeader header;
U32 trackCounter=0;
U32 endAddr=0L;
U32 ulChunkSize=0;

amTrace((const U8*)"Number of tracks to process: %ld\n\n",numTracks);


memcpy(&header, startPtr, sizeof(sChunkHeader));
startPtr=(U8*)startPtr + sizeof(sChunkHeader);
    
ulChunkSize=header.headLenght;
endAddr=(U32)startPtr+header.headLenght;

  switch(fileTypeFlag){
  
    case T_MIDI0:{
	  /* we have only one track data to process */
	  /* add all of them to given track */ 
	  sTrack_t *pTempTrack=pCurSequence->arTracks[0];
	  pCurSequence->arTracks[0]->currTrackState.ulTrackTempo=DEFAULT_TEMPO;
	
	  sTrack_t **ppTrack=&pTempTrack;
	  const void *pTemp=(const void *)endAddr;
	  const void **end=&pTemp;
	  startPtr=processMIDItrackEvents(&startPtr,end,ppTrack );
    }
    break;
     case T_MIDI1:{
      while(( (header.id==ID_MTRK)&&(trackCounter<numTracks))){
	  /* we have got track data :)) */
	  /* add all of them to given track */ 
	  sTrack_t *pTempTrack=pCurSequence->arTracks[0];
	  pCurSequence->arTracks[0]->currTrackState.ulTrackTempo=DEFAULT_TEMPO;
	
	  sTrack_t **ppTrack=&pTempTrack;
	  const void *pTemp=(const void *)endAddr;
	  const void **end=&pTemp;
	  startPtr=processMIDItrackEvents(&startPtr,end,ppTrack );
	
	  /* get next data chunk info */
	  memcpy(&header, startPtr,sizeof(sChunkHeader));
	  ulChunkSize=header.headLenght;
	
	  /* omit Track header */
	  startPtr=(U8*)startPtr+sizeof(sChunkHeader);
	  endAddr=(U32)startPtr+header.headLenght;

	  /* increase track counter */
	  trackCounter++;
	}
	
    }break;
    case T_MIDI2:{
	/* handle MIDI 2, multitrack type */
	/* create several track lists according to numTracks */
	/*  TODO: not finished !*/
	/*assert((pCurSequence->arTracks[trackCounter])->pEventListPtr>0);*/

	/* init event list */
	/*initEventList((pCurSequence->arTracks[trackCounter])->pEventListPtr);*/

	/* tracks inited, now insert track data */
	trackCounter=0;	/* reset track counter first */

	while(( (header.id==ID_MTRK)&&(trackCounter<numTracks))){
	  /* we have got track data :)) */
	  /* add all of them to given track */ 
	  sTrack_t *pTempTrack=pCurSequence->arTracks[trackCounter];
	  pCurSequence->arTracks[trackCounter]->currTrackState.ulTrackTempo=DEFAULT_TEMPO;
	
	  sTrack_t **ppTrack=&pTempTrack;
	  const void *pTemp=(const void *)endAddr;
	  const void **end=&pTemp;
	
	  startPtr=processMIDItrackEvents(&startPtr,end,ppTrack);
	
	  /* get next data chunk info */
	  memcpy(&header, startPtr,sizeof(sChunkHeader));
	  ulChunkSize=header.headLenght;
	
	  /* omit Track header */
	  startPtr=(U8*)startPtr+sizeof(sChunkHeader);
	  endAddr=(U32)startPtr+header.headLenght;

	  /* increase track counter */
	  trackCounter++;
	}
    }break;
     case T_XMIDI:{
        /*TODO: ! not implemented */
	return NULL;
     }
     case T_RMID:{
     return NULL;/*TODO: ! not implemented */
     }break;
     case T_SMF:{
      return NULL;/*TODO: ! not implemented */
     }break;
     case T_XMF:{
      return NULL;/*TODO: ! not implemented */
     } break;
     
     case T_SNG:{
      return NULL;;/*TODO: ! not implemented */
    }break;
     case T_MUS:
       return NULL;;/*TODO: ! not implemented */
     break;
    default:{
      return NULL;
    }
  };
  
  
  
 return NULL;
}

U8 am_isMidiChannelEvent(U8 byteEvent){

    if(( ((byteEvent&0xF0)>=0x80) && ((byteEvent&0xF0)<=0xE0)))
    {return 1;}
    else return 0;
}

U8 am_isMidiRTorSysex(U8 byteEvent){

    if( ((byteEvent>=(U8)0xF0)&&(byteEvent<=(U8)0xFF)) ){   
      /* it is! */
        return (1);
    }
    else /*no, it's not! */
        return (0);
}

/* handles the events in tracks and returns pointer to the next midi track */

void *processMIDItrackEvents(void**startPtr, const void **endAddr, sTrack_t **pCurTrack ){
U8 *pCmd=((U8 *)(*startPtr));
U8 ubSize;
U8 usSwitch=0;
U16 recallStatus=0;
U32 delta=0L;
U32 deltaAll=0L;
BOOL bEOF=FALSE;
/*U8 arDeltaInfo[16];*/

    /* execute as long we are on the end of file or EOT meta occured, 
      50% midi track headers is broken, so the web says ;)) */
    while ( ((pCmd!=(*endAddr))) ){
    
      /*read delta time, pCmd should point to the command data */
      delta=readVLQ(pCmd,&ubSize);
     /* deltaAll=deltaAll+delta;
      sprintf((char *)arDeltaInfo,"Event: delta %u \n",(unsigned int)deltaAll);*/
     
      pCmd=(U8 *)((U32)pCmd+ubSize*sizeof(U8));

      /* handling of running status */
      /* if byte is not from 0x08-0x0E range then recall last running status AND set recallStatus = 1 */
      /* else set recallStatus = 0 and do nothing special */
      ubSize=(*pCmd);
      if( (!(am_isMidiChannelEvent(ubSize))&&(recallStatus==1)&&(!(am_isMidiRTorSysex(ubSize))))){
	/*recall last cmd byte */
	usSwitch=g_runningStatus;
        usSwitch=((usSwitch>>4)&0x0F);
      }else{
	/* check if the new cmd is the system one*/
	recallStatus=0;

      if((am_isMidiRTorSysex(ubSize))){
	usSwitch=ubSize;
      }else{
	usSwitch=ubSize;
        usSwitch=((usSwitch>>4)&0x0F);
      }
   }

    /* decode event and write it to our custom structure */
    switch(usSwitch){
      case EV_NOTE_OFF:
	am_noteOff(&pCmd,&recallStatus, delta, pCurTrack );
      break;
      case EV_NOTE_ON:
	am_noteOn(&pCmd,&recallStatus, delta, pCurTrack );
      break;
      case EV_NOTE_AFTERTOUCH:
	am_noteAft(&pCmd,&recallStatus, delta, pCurTrack );
      break;
      case EV_CONTROLLER:
	am_Controller(&pCmd,&recallStatus, delta, pCurTrack );
      break;
      case EV_PROGRAM_CHANGE:
	am_PC(&pCmd,&recallStatus, delta, pCurTrack );
      break;
      case EV_CHANNEL_AFTERTOUCH:
	am_ChannelAft(&pCmd,&recallStatus, delta, pCurTrack );
      break;
      case EV_PITCH_BEND:
	am_PitchBend(&pCmd,&recallStatus, delta, pCurTrack );
      break;
      case EV_META:
	bEOF=am_Meta(&pCmd, delta, pCurTrack );
      break;
      case EV_SOX:                          	/* SySEX midi exclusive */
	recallStatus=0; 	                /* cancel out midi running status */
	am_Sysex(&pCmd,delta, pCurTrack);
      break;
      case SC_MTCQF:
	recallStatus=0;                        /* Midi time code quarter frame, 1 byte */
	amTrace((const U8*)"Event: System common MIDI time code qt frame\n");
	pCmd++;
	pCmd++;
      break;
      case SC_SONG_POS_PTR:
	amTrace((const U8*)"Event: System common Song position pointer\n");
	recallStatus=0;                      /* Song position pointer, 2 data bytes */
	pCmd++;
	pCmd++;
	pCmd++;
      break;
      case SC_SONG_SELECT:              /* Song select 0-127, 1 data byte*/
	amTrace((const U8*)"Event: System common Song select\n");
	recallStatus=0;
	pCmd++;
	pCmd++;
      break;
      case SC_UNDEF1:                   /* undefined */
      case SC_UNDEF2:                  /* undefined */
	amTrace((const U8*)"Event: System common not defined.\n");
	recallStatus=0;
	pCmd++;
      break;
      case SC_TUNE_REQUEST:             /* tune request, no data bytes */
	amTrace((const U8*)"Event: System tune request.\n");
	recallStatus=0;
	pCmd++;
      break;
      default:
	amTrace((const U8*)"Event: Unknown type: %d\n",(*pCmd));
	/* unknown event, do nothing or maybe throw error? */
    }
} /*end of decode events loop */
/* return the next track data */
return(pCmd);
}


void am_noteOff(U8 **pPtr,U16 *recallRS,U32 delta, sTrack_t **pCurTrack){
U8 channel=0;
U8 note=0;
U8 velocity=0;
sEventBlock_t tempEvent;
sNoteOff_EventBlock_t *pEvntBlock=NULL;

if((*recallRS)==0){
  /* save last running status */
  g_runningStatus=*(*pPtr);
	
  tempEvent.uiDeltaTime=delta;
  tempEvent.type=T_NOTEOFF;
  tempEvent.infoBlock=getEventFuncInfo(T_NOTEOFF);
  tempEvent.dataPtr=malloc(tempEvent.infoBlock.size);
  /*assert(tempEvent.dataPtr>0);*/
  pEvntBlock=(sNoteOff_EventBlock_t *)tempEvent.dataPtr;
  pEvntBlock->ubChannelNb=g_runningStatus&0x0F;

  /* now we can recall former running status next time */
  (*recallRS)=1;

  (*pPtr)++;
  channel=(g_runningStatus&0x0F)+1;
  note=*(*pPtr);
  pEvntBlock->eventData.noteNb=*(*pPtr);
			
  /* get parameters */
  (*pPtr)++;
  velocity=*(*pPtr);
  pEvntBlock->eventData.velocity=*(*pPtr);
			
  (*pPtr)++;

  /* add event to list */
  addEvent( &((*pCurTrack)->trkEventList), &tempEvent );
  free(tempEvent.dataPtr);

}
else {
  /* recall last cmd status */
  /* and get parameters as usual */

  /* get last note info */
  tempEvent.uiDeltaTime=delta;
  tempEvent.type=T_NOTEOFF;
  tempEvent.infoBlock=getEventFuncInfo(T_NOTEOFF);
  tempEvent.dataPtr=malloc(tempEvent.infoBlock.size);
  /*assert(tempEvent.dataPtr>0);*/
  pEvntBlock=(sNoteOff_EventBlock_t *)tempEvent.dataPtr;
  /* save channel */
  pEvntBlock->ubChannelNb=g_runningStatus&0x0F;

  channel=(g_runningStatus&0x0F)+1;
  note=*(*pPtr);
  pEvntBlock->eventData.noteNb=*(*pPtr);

  /* get parameters */
  (*pPtr)++;
  velocity=*(*pPtr);
  pEvntBlock->eventData.velocity=*(*pPtr);
			
  (*pPtr)++;

  /* add event to list */
  addEvent(&((*pCurTrack)->trkEventList), &tempEvent );
  free(tempEvent.dataPtr);
}
  amTrace((const U8*)"delta: %u\t",(unsigned int)delta);
  amTrace((const U8*)"event: Note off ");
  amTrace((const U8*)"ch: %d\t",(g_runningStatus&0x0F)+1);
  amTrace((const U8*)"note: %d(%s)\t",*(*pPtr),am_getMIDInoteName(*(*pPtr)));
  amTrace((const U8*)"vel: %d\n",*(*pPtr));
}

void am_noteOn(U8 **pPtr,U16 *recallRS,U32 delta, sTrack_t **pCurTrack){
 U8 channel=0;
 U8 note=0;
 U8 velocity=0;
 sEventBlock_t tempEvent;
 sNoteOn_EventBlock_t *pEvntBlock=NULL;

 if((*recallRS)==0){
    /* save last running status */
    g_runningStatus=*(*pPtr);

    tempEvent.uiDeltaTime=delta;
    tempEvent.type=T_NOTEON;
    tempEvent.infoBlock=getEventFuncInfo(T_NOTEON);
    tempEvent.dataPtr=malloc(tempEvent.infoBlock.size);
    /*assert(tempEvent.dataPtr>0);*/
    pEvntBlock=(sNoteOn_EventBlock_t *)tempEvent.dataPtr;

    /* now we can recall former running status next time */
    (*recallRS)=1;

    (*pPtr)++;
    channel=(g_runningStatus&0x0F)+1;
	pEvntBlock->ubChannelNb=g_runningStatus&0x0F;
    note=*(*pPtr);
	pEvntBlock->eventData.noteNb=*(*pPtr);

    /* get parameters */
    (*pPtr)++;
    velocity=*(*pPtr);
	pEvntBlock->eventData.velocity=*(*pPtr);
    
	(*pPtr)++;

	/* add event to list */
		addEvent(&((*pCurTrack)->trkEventList), &tempEvent );
		free(tempEvent.dataPtr);

 }
 else{
    /* get last note info */
    channel=(g_runningStatus&0x0F)+1;

	tempEvent.uiDeltaTime=delta;
	tempEvent.type=T_NOTEON;
	tempEvent.infoBlock=getEventFuncInfo(T_NOTEON);
	tempEvent.dataPtr=malloc(tempEvent.infoBlock.size);
	/*assert(tempEvent.dataPtr>0);*/
	pEvntBlock=(sNoteOn_EventBlock_t *)tempEvent.dataPtr;

	pEvntBlock->ubChannelNb=g_runningStatus&0x0F;

    note=*(*pPtr);
	pEvntBlock->eventData.noteNb=*(*pPtr);
    /* get parameters */
    (*pPtr)++;
    velocity=*(*pPtr);
	pEvntBlock->eventData.velocity=*(*pPtr);
	
    (*pPtr)++;

	/* add event to list */
		addEvent(&((*pCurTrack)->trkEventList), &tempEvent );
		free(tempEvent.dataPtr);
 }
 /* print and handle */
    amTrace((const U8*)"delta: %u\t",(unsigned int)delta);
    amTrace((const U8*)"event: Note on ");
    amTrace((const U8*)"ch: %d\t",channel);
    amTrace((const U8*)"note: %d(%s)\t",note,am_getMIDInoteName(note));
    amTrace((const U8*)"vel: %d \n",velocity);
    
}

void am_noteAft(U8 **pPtr,U16 *recallRS,U32 delta, sTrack_t **pCurTrack){
U8 noteNb=0;
U8 pressure=0;
sEventBlock_t tempEvent;
sNoteAft_EventBlock_t *pEvntBlock=NULL;

 if((*recallRS)==0){
    /* save last running status */
    g_runningStatus=*(*pPtr);
	
	tempEvent.uiDeltaTime=delta;
	tempEvent.type=T_NOTEAFT;
	tempEvent.infoBlock=getEventFuncInfo(T_NOTEAFT);
	tempEvent.dataPtr=malloc(tempEvent.infoBlock.size);
	/*assert(tempEvent.dataPtr>0);*/
	pEvntBlock=(sNoteAft_EventBlock_t *)tempEvent.dataPtr;

	pEvntBlock->ubChannelNb=g_runningStatus&0x0F;

    /* now we can recall former running status next time */
    (*recallRS)=1;

    (*pPtr)++;
    /* get parameters */
    noteNb=*(*pPtr);
	pEvntBlock->eventData.noteNb=*(*pPtr);
    (*pPtr)++;
    pressure=*(*pPtr);
	pEvntBlock->eventData.pressure=*(*pPtr);
    (*pPtr)++;
	/* add event to list */
	addEvent(&((*pCurTrack)->trkEventList), &tempEvent );
	free(tempEvent.dataPtr);
 }
 else{
        /* get parameters */
	tempEvent.uiDeltaTime=delta;
	tempEvent.type=T_NOTEAFT;
	tempEvent.infoBlock=getEventFuncInfo(T_NOTEAFT);
	tempEvent.dataPtr=malloc(tempEvent.infoBlock.size);
	/*assert(tempEvent.dataPtr>0);*/
	pEvntBlock=(sNoteAft_EventBlock_t *)tempEvent.dataPtr;
	pEvntBlock->ubChannelNb=g_runningStatus&0x0F;

        noteNb=*(*pPtr);
	pEvntBlock->eventData.noteNb=*(*pPtr);
        (*pPtr)++;
        pressure=*(*pPtr);
	pEvntBlock->eventData.pressure=*(*pPtr);
        (*pPtr)++;
		
	/* add event to list */
	addEvent(&((*pCurTrack)->trkEventList), &tempEvent );
	free(tempEvent.dataPtr);

    }
     amTrace((const U8*)"delta: %u\tevent: Note Aftertouch note: %d, pressure: %d\n",(unsigned int)delta, noteNb,pressure);
     

}

void am_Controller(U8 **pPtr,U16 *recallRS,U32 delta, sTrack_t **pCurTrack){
    U8 channelNb=0;
    U8 controllerNb=0;
    U8 value=0;
    sEventBlock_t tempEvent;
    sController_EventBlock_t *pEvntBlock=NULL;

    if((*recallRS)==0){
        /* save last running status */
        g_runningStatus=*(*pPtr);
        /* now we can recall former running status next time */
        (*recallRS)=1;

	tempEvent.uiDeltaTime=delta;
	tempEvent.type=T_CONTROL;
	tempEvent.infoBlock=getEventFuncInfo(T_CONTROL);
	tempEvent.dataPtr=malloc(tempEvent.infoBlock.size);
	/*assert(tempEvent.dataPtr>0);*/
	pEvntBlock=(sController_EventBlock_t *)tempEvent.dataPtr;

        channelNb=g_runningStatus&0x0F;
		pEvntBlock->ubChannelNb=g_runningStatus&0x0F;
        (*pPtr)++;
        /* get controller nb */
        controllerNb=(*(*pPtr));
	pEvntBlock->eventData.controllerNb=(*(*pPtr));
        (*pPtr)++;
        value=*((*pPtr));
	pEvntBlock->eventData.value=*((*pPtr));

        (*pPtr)++;
	
	/* add event to list */
	addEvent(&((*pCurTrack)->trkEventList), &tempEvent );
	free(tempEvent.dataPtr);
    }
    else{
        
        channelNb=g_runningStatus&0x0F;
		
	tempEvent.uiDeltaTime=delta;
	tempEvent.type=T_CONTROL;
	tempEvent.infoBlock=getEventFuncInfo(T_CONTROL);
	tempEvent.dataPtr=malloc(tempEvent.infoBlock.size);
	/*assert(tempEvent.dataPtr>0);*/
	pEvntBlock=(sController_EventBlock_t *)tempEvent.dataPtr;

	pEvntBlock->ubChannelNb=g_runningStatus&0x0F;

        /* get program controller nb */
        controllerNb=(*(*pPtr));
	pEvntBlock->eventData.controllerNb=(*(*pPtr));

        (*pPtr)++;
        value=*((*pPtr));
		pEvntBlock->eventData.value=*((*pPtr));
        (*pPtr)++;

		/* add event to list */
		addEvent(&((*pCurTrack)->trkEventList), &tempEvent  );
		free(tempEvent.dataPtr);

    }

    amTrace((const U8*)"delta: %u\tevent: Controller ch: %d, nb:%d name: %s\tvalue: %d\n",(unsigned int)delta, channelNb+1, controllerNb,getMIDIcontrollerName(controllerNb), value);
    

}

void am_PC(U8 **pPtr,U16 *recallRS,U32 delta, sTrack_t **pCurTrack)
{
	U8 channel=0;
	U8 PN=0;
	sEventBlock_t tempEvent;
	sPrgChng_EventBlock_t *pEvntBlock=NULL;

     if((*recallRS)==0){
        /* save last running status */
        g_runningStatus=*(*pPtr);
		
	/* now we can recall former running status next time */
        (*recallRS)=1;

	channel=(g_runningStatus&0x0F)+1;

	tempEvent.uiDeltaTime=delta;
	tempEvent.type=T_PRG_CH;
	tempEvent.infoBlock=getEventFuncInfo(T_PRG_CH);
	tempEvent.dataPtr=malloc(tempEvent.infoBlock.size);
	/*assert(tempEvent.dataPtr>0);*/
	pEvntBlock=(sPrgChng_EventBlock_t *)tempEvent.dataPtr;
	pEvntBlock->ubChannelNb=g_runningStatus&0x0F;
        (*pPtr)++;
        /* get parameters */
        PN=*(*pPtr);
	pEvntBlock->eventData.programNb=*(*pPtr);
	(*pPtr)++;
	/* add event to list */
	addEvent(&((*pCurTrack)->trkEventList), &tempEvent );
	free(tempEvent.dataPtr);
    }
    else{
         /* get last PC status */
          channel=(g_runningStatus&0x0F)+1;
	  tempEvent.uiDeltaTime=delta;
	  tempEvent.type=T_PRG_CH;
	  tempEvent.infoBlock=getEventFuncInfo(T_PRG_CH);
	  tempEvent.dataPtr=malloc(tempEvent.infoBlock.size);
	  /*assert(tempEvent.dataPtr>0);*/
	  pEvntBlock=(sPrgChng_EventBlock_t *)tempEvent.dataPtr;
	  pEvntBlock->ubChannelNb=g_runningStatus&0x0F;

          PN=*(*pPtr);
	  pEvntBlock->eventData.programNb=*(*pPtr);
         
	 /* get parameters */
	  (*pPtr)++;
	
	 /* add event to list */
	addEvent(&((*pCurTrack)->trkEventList), &tempEvent );
	free(tempEvent.dataPtr);
    }
    amTrace((const U8*)"delta: %u\t",(unsigned int)delta);
    amTrace((const U8*)"event: Program change ");
    amTrace((const U8*)"ch: %d\t",channel);
    amTrace((const U8*)"program nb: %d\n",PN);
 }

void am_ChannelAft(U8 **pPtr,U16 *recallRS,U32 delta, sTrack_t **pCurTrack)
{
U8 channel=0;
U8 param=0;
sEventBlock_t tempEvent;
sChannelAft_EventBlock_t *pEvntBlock=NULL;

    if((*recallRS)==0){
        /* save last running status */
        g_runningStatus=*(*pPtr);
        /* now we can recall former running status next time */
        (*recallRS)=1;

	tempEvent.uiDeltaTime=delta;
	tempEvent.type=T_CHAN_AFT;
	tempEvent.infoBlock=getEventFuncInfo(T_CHAN_AFT);
	tempEvent.dataPtr=malloc(tempEvent.infoBlock.size);
	/*assert(tempEvent.dataPtr>0);*/
	pEvntBlock=(sChannelAft_EventBlock_t *)tempEvent.dataPtr;
	pEvntBlock->ubChannelNb=(g_runningStatus&0x0F);
        
	/* get parameters */
        (*pPtr)++;
        param=*(*pPtr);
		pEvntBlock->eventData.pressure=*(*pPtr);
        (*pPtr)++;

	/* add event to list */
	addEvent(&((*pCurTrack)->trkEventList), &tempEvent );
	free(tempEvent.dataPtr);

    }
    else{
	tempEvent.uiDeltaTime=delta;
	tempEvent.type=T_CHAN_AFT;
	tempEvent.infoBlock=getEventFuncInfo(T_CHAN_AFT);
	tempEvent.dataPtr=malloc(tempEvent.infoBlock.size);
	/*		assert(tempEvent.dataPtr>0);*/
	pEvntBlock=(sChannelAft_EventBlock_t *)tempEvent.dataPtr;
	pEvntBlock->ubChannelNb=(g_runningStatus&0x0F);

        /* get parameters */
        param=*(*pPtr);
	pEvntBlock->eventData.pressure=*(*pPtr);
        (*pPtr)++;

	/* add event to list */
	addEvent(&((*pCurTrack)->trkEventList), &tempEvent );
	free(tempEvent.dataPtr);
    }
    amTrace((const U8*)"delta: %u\tevent: Channel aftertouch pressure: %d\n",(unsigned int)delta, param);
    

}

void am_PitchBend(U8 **pPtr,U16 *recallRS,U32 delta, sTrack_t **pCurTrack){
U8 MSB=0;
U8 LSB=0;
sEventBlock_t tempEvent;
sPitchBend_EventBlock_t *pEvntBlock=NULL;

    if((*recallRS)==0)
    {
        /* save last running status */
        g_runningStatus=*(*pPtr);

        /* now we can recall former running status next time */
        (*recallRS)=1;

	tempEvent.uiDeltaTime=delta;
	tempEvent.type=T_PITCH_BEND;
	tempEvent.infoBlock=getEventFuncInfo(T_PITCH_BEND);
	tempEvent.dataPtr=malloc(tempEvent.infoBlock.size);
	/*assert(tempEvent.dataPtr>0);*/
	pEvntBlock=(sPitchBend_EventBlock_t *)tempEvent.dataPtr;
	pEvntBlock->ubChannelNb=(g_runningStatus&0x0F);

        (*pPtr)++;
        LSB=*(*pPtr);
	pEvntBlock->eventData.LSB=*(*pPtr);
        /* get parameters */
        (*pPtr)++;
        MSB=*(*pPtr);
	pEvntBlock->eventData.MSB=*(*pPtr);
        (*pPtr)++;
		
	/* add event to list */
	addEvent(&((*pCurTrack)->trkEventList), &tempEvent );
	free(tempEvent.dataPtr);
    }
    else{
        g_runningStatus;    /* get last PC status */

	tempEvent.uiDeltaTime=delta;
	tempEvent.type=T_PITCH_BEND;
	tempEvent.infoBlock=getEventFuncInfo(T_PITCH_BEND);
	tempEvent.dataPtr=malloc(tempEvent.infoBlock.size);
	/*assert(tempEvent.dataPtr>0);*/
	pEvntBlock=(sPitchBend_EventBlock_t *)tempEvent.dataPtr;
	pEvntBlock->ubChannelNb=(g_runningStatus&0x0F);

        /* get parameters */
        LSB=*(*pPtr);
	pEvntBlock->eventData.LSB=*(*pPtr);
        (*pPtr)++;
        MSB=*(*pPtr);
	pEvntBlock->eventData.MSB=*(*pPtr);
        (*pPtr)++;
		
	/* add event to list */
	addEvent(&((*pCurTrack)->trkEventList), &tempEvent );
	free(tempEvent.dataPtr);
    }
 amTrace((const U8*)"delta: %u\tevent: Pitch bend LSB: %d, MSB:%d\n",(unsigned int)delta,LSB,MSB);
 
}

void am_Sysex(U8 **pPtr,U32 delta, sTrack_t **pCurTrack){
  U32 ulCount=0L;
  sEventBlock_t tempEvent;
 
  amTrace((const U8*)"SOX: ");
  
    while( (*(*pPtr))!=EV_EOX){
     amTrace((const U8*)"%x ",*(*pPtr));
     
     (*pPtr)++;
      /*count Sysex msg data bytes */
      ulCount++;
    }
    amTrace((const U8*)" EOX, size: %ld\n",ulCount);
    
}

BOOL am_Meta(U8 **pPtr,U32 delta, sTrack_t **pCurTrack){
 /* TODO: maybe move these variables to static/global area and/or replace them with register vars for speed ?*/
 U8 ubLenght,ubVal,ubSize=0;

 U8 param1=0,param2=0;
 U32 addr;
 U8 textBuffer[128]={0};
 sSMPTEoffset SMPTEinfo;
 sTimeSignature timeSign;
 sEventBlock_t tempEvent;

 /*get meta event type */
 (*pPtr)++;
 ubVal=*(*pPtr);

 switch(ubVal){
    case MT_SEQ_NB:
        amTrace((const U8*)"delta: %u\tMeta event: Sequence nb: ",(unsigned int)delta);
	
        (*pPtr)++;
        ubLenght=(*(*pPtr));
        amTrace((const U8*)"%d\n", ubLenght);
	
        (*pPtr)++;
        addr=((U32)(*pPtr))+ubLenght*sizeof(U8);
        *pPtr=(U8*)addr;
	return FALSE;
    break;
    case MT_TEXT:
        amTrace((const U8*)"delta: %u\tMeta event: Text:",(unsigned int)delta);
	
        (*pPtr)++;
        ubLenght=readVLQ((*pPtr),&ubSize);
        amTrace((const U8*)"meta size: %d ",ubLenght);
        
	/* set to the start of the string */
        (*pPtr)++;
        memcpy(textBuffer, (*pPtr),ubLenght*sizeof(U8) );
        (*pPtr)=((*pPtr)+ubLenght);
        amTrace((const U8*)"%s \n",textBuffer);
	
	return FALSE;
    break;
    case MT_COPYRIGHT:
        amTrace((const U8*)"delta: %u\tMeta event: Copyright: ",(unsigned int)delta);
	
        (*pPtr)++;
        ubLenght=readVLQ((*pPtr),&ubSize);
        amTrace((const U8*)"meta size: %d ",ubLenght);
	
        /* set to the start of the string */
        (*pPtr)++;
        memcpy(textBuffer, (*pPtr),ubLenght*sizeof(U8) );
        (*pPtr)=((*pPtr)+ubLenght);
        amTrace((const U8*)"%s \n",textBuffer);
	
	return FALSE;
    break;
    case MT_SEQNAME:
        amTrace((const U8*)"delta: %u\tMeta event: Sequence name: ",(unsigned int)delta);
	
        (*pPtr)++;
        ubLenght=readVLQ((*pPtr),&ubSize);
        /* set to the start of the string */
        (*pPtr)++;
        memcpy(textBuffer, (*pPtr),ubLenght*sizeof(U8) );
        (*pPtr)=((*pPtr)+ubLenght);
        amTrace((const U8*)"meta size: %d ",ubLenght);
	
        amTrace((const U8*)"%s \n",textBuffer);
	
	return FALSE;
    break;
    case MT_INSTRNAME:
        amTrace((const U8*)"delta: %u\tMeta event: Instrument name: ",(unsigned int)delta);
	
        (*pPtr)++;
        ubLenght=readVLQ((*pPtr),&ubSize);
        amTrace((const U8*)"meta size: %d",ubLenght);
	
        /* set to the start of the string */
        (*pPtr)++;
        memcpy(textBuffer, (*pPtr),ubLenght*sizeof(U8) );
        (*pPtr)=((*pPtr)+ubLenght);
        amTrace((const U8*)"%s \n",textBuffer);
	
	return FALSE;
    break;
    case MT_LYRICS:
        amTrace((const U8*)"delta: %u\tMeta event: Lyrics: ",(unsigned int)delta);
	
        (*pPtr)++;
        ubLenght=readVLQ((*pPtr),&ubSize);
        amTrace((const U8*)"meta size: %d ",ubLenght);
	
        /* set to the start of the string */
        (*pPtr)++;
        memcpy(textBuffer, (*pPtr),ubLenght*sizeof(U8) );
        (*pPtr)=((*pPtr)+ubLenght);
        amTrace((const U8*)"%s \n",textBuffer);
	
	return FALSE;
    break;

    case MT_MARKER:
        amTrace((const U8*)"delta: %u\tMeta event: Marker: ",(unsigned int)delta);
	
        (*pPtr)++;
        ubLenght=readVLQ((*pPtr),&ubSize);
        /* set to the start of the string */
        (*pPtr)++;
        memcpy(textBuffer, (*pPtr),ubLenght*sizeof(U8) );
        (*pPtr)=((*pPtr)+ubLenght);
        amTrace((const U8*)"meta size: %d ",ubLenght);
	
        amTrace((const U8*)"%s \n",textBuffer);
	
	return FALSE;
    break;
    case MT_CUEPOINT:
        amTrace((const U8*)"delta: %u\tMeta event: Cuepoint\n",(unsigned int)delta);
	
        (*pPtr)++;
        ubLenght=readVLQ((*pPtr),&ubSize);
        /* set to the start of the string */
        (*pPtr)++;
        memcpy(textBuffer, (*pPtr),ubLenght*sizeof(U8) );
        (*pPtr)=((*pPtr)+ubLenght);
        amTrace((const U8*)"meta size: %d ",ubLenght);
	
        amTrace((const U8*)"%s \n",textBuffer);
	
	return FALSE;
    break;

    case MT_PROGRAM_NAME:
        /* program(patch) name */
        amTrace((const U8*)"delta: %u\tMeta event: Program (patch) name: ",(unsigned int)delta);
	
        (*pPtr)++;
        ubLenght=readVLQ((*pPtr),&ubSize);

        /* set to the start of the string */
        (*pPtr)++;
        memcpy(textBuffer, (*pPtr),ubLenght*sizeof(U8) );
        (*pPtr)=((*pPtr)+ubLenght);
        amTrace((const U8*)"meta size: %d ",ubLenght);
	
        amTrace((const U8*)"%s \n",textBuffer);
	
	return FALSE;
    break;
    case MT_DEVICE_NAME:
        /* device (port) name */
        amTrace((const U8*)"delta: %u\tMeta event: Device (port) name: ",(unsigned int)delta);
	
        (*pPtr)++;
        ubLenght=readVLQ((*pPtr),&ubSize);

        /* set to the start of the string */
        (*pPtr)++;
        memcpy(textBuffer, (*pPtr),ubLenght*sizeof(U8) );
        (*pPtr)=((*pPtr)+ubLenght);
        amTrace((const U8*)"meta size: %d ",ubLenght);
	
        amTrace((const U8*)"%s \n",textBuffer);
	
	return FALSE;
    break;
    case MT_CH_PREFIX:
        amTrace((const U8*)"delta: %u\tMeta event: Channel prefix\n",(unsigned int)delta);
	
        (*pPtr)++;
        ubLenght=(*(*pPtr));
        (*pPtr)++;
        addr=((U32)(*pPtr))+ubLenght*sizeof(U8);
        *pPtr=(U8*)addr;
	return FALSE;
    break;
    case MT_MIDI_CH: /* obsolete! just ignore */
        (*pPtr)++;
        /*get size */
        ubLenght=(*(*pPtr));
        (*pPtr)++;

        /*get port nb*/
        amTrace((const U8*)"delta: %u\tMeta event: Midi channel nb: %d\n",(unsigned int)delta,*(*pPtr));
	
        (*pPtr)++;
	return FALSE;
    break;
    case MT_MIDI_PORT: /* obsolete! just ignore */
        (*pPtr)++;
        /*get size */
        ubLenght=(*(*pPtr));
        (*pPtr)++;

        /*get port nb*/
        amTrace((const U8*)"delta: %u\tMeta event: Midi port nb: %d\n",(unsigned int)delta,*(*pPtr));
	
        (*pPtr)++;
	return FALSE;
    break;
    case MT_EOT:
        amTrace((const U8*)"delta: %u\tMeta event: End of track\n\n",(unsigned int)delta);
	
        (*pPtr)++;
        ubLenght=(*(*pPtr));
        (*pPtr)++;
        addr=((U32)(*pPtr))+ubLenght*sizeof(U8);
        *pPtr=(U8*)addr;
	return TRUE;
    break;
    case MT_SET_TEMPO:{
        /* sets tempo in track, should be in the first track, if not 120 BPM is assumed */
	U8 ulVal[3]={0};   /* for retrieving set tempo info */
	amTrace((const U8*)"delta: %u\tMeta event: Set tempo: ",(unsigned int)delta);
	
	(*pPtr)++;
        ubLenght=(*(*pPtr));
         (*pPtr)++;
        /* get those 3 bytes */
        memcpy(ulVal, (*pPtr),ubLenght*sizeof(U8) );

        addr=((U32)(*pPtr))+ubLenght*sizeof(U8);
        *pPtr=(U8*)addr;
        /*amTrace((const U8*)"0x%x%x%x ms per quarter-note\n", ulVal[0],ulVal[1],ulVal[2]);
	*/
	
	U32 val1=ulVal[0],val2=ulVal[1],val3=ulVal[2]; 
	val1=(val1<<16)&0x00FF0000L;
	val2=(val2<<8)&0x0000FF00L;
	val3=(val3)&0x000000FFL;

	/* range: 0-8355711 ms, 24 bit value */
	val1=val1|val2|val3;
	(*pCurTrack)->currTrackState.ulTrackTempo=val1;
	amTrace((const U8*)"%u ms per quarter-note\n", (unsigned int)val1);
	
	return FALSE;
    }
    break;
    case MT_SMPTE_OFFSET:
        amTrace((const U8*)"delta: %u\tMeta event: SMPTE offset:\n",(unsigned int)delta);
	
        (*pPtr)++;
        ubLenght=(*(*pPtr));
        (*pPtr)++;
        memcpy (&SMPTEinfo,(*pPtr),sizeof(sSMPTEoffset));

        addr=((U32)(*pPtr))+ubLenght*sizeof(U8);
        *pPtr=(U8*)addr;
    /* print out info */
        amTrace((const U8*)"hr: %d\n",SMPTEinfo.hr);
        amTrace((const U8*)"mn: %d\n",SMPTEinfo.mn);
        amTrace((const U8*)"se: %d\n",SMPTEinfo.fr);
        amTrace((const U8*)"fr: %d\n",SMPTEinfo.fr);
        amTrace((const U8*)"ff: %d\n",SMPTEinfo.ff);
	return FALSE;
    break;
    case MT_TIME_SIG:
        amTrace((const U8*)"delta: %u\tMeta event: Time signature: ",(unsigned int)delta);
	
        (*pPtr)++;
        ubLenght=(*(*pPtr));
        (*pPtr)++;

        memcpy (&timeSign,(*pPtr),sizeof(sTimeSignature));
        addr=((U32)(*pPtr))+ubLenght*sizeof(U8);
        *pPtr=(U8*)addr;
        /* print out info */
        amTrace((const U8*)"nn: %d\tdd: %d\tcc: %d\tbb: %d\n",timeSign.nn,timeSign.dd,timeSign.cc,timeSign.bb);
        
	return FALSE;
    break;
    case MT_KEY_SIG:
        amTrace((const U8*)"delta: %u\tMeta event: Key signature: ",(unsigned int)delta);
	
        (*pPtr)++;
        ubLenght=(*(*pPtr));
        (*pPtr)++;
        param1=(*(*pPtr));
        (*pPtr)++;
        param2=(*(*pPtr));

        if (param2==0) {amTrace((const U8*)"scale: major ");}
        else if(param2==1) {amTrace((const U8*)"scale: minor ");}
         else { amTrace((const U8*)"error: wrong key signature scale. "); }

        if(param1==0)
            {amTrace((const U8*)"Key of C\n");}
        else if (((S8)param1==-1))
            {amTrace((const U8*)"1 flat\n");}
        else if(((S8)param1)==1)
            {amTrace((const U8*)"1 sharp\n");}
        else if ((param1>1&&param1<=7))
            {amTrace((const U8*)" %d sharps\n",param1);}
        else if (( ((S8)param1)<-1&& ((S8)param1)>=-7))
            {amTrace((const U8*)" %ld flats\n",(U32)param1);}
        else {amTrace((const U8*)" error: wrong key signature. %d\n",param1);}
        (*pPtr)++;
	return FALSE;
    break;
    case MT_SEQ_SPEC:
        amTrace((const U8*)"delta: %u\tMeta event: Sequencer specific data.\n",(unsigned int)delta);
        (*pPtr)++;
        ubLenght=(*(*pPtr));
        (*pPtr)++;
        addr=((U32)(*pPtr))+ubLenght*sizeof(U8);
        *pPtr=(U8*)addr;
	return FALSE;
    break;
    default:
       amTrace((const U8*)"delta: %u\tUnknown meta event.\n",(unsigned int)delta);
        (*pPtr)++;
        ubLenght=(*(*pPtr));
        /* we should put here assertion failed or something with "send this file to author" message */
        /* file also could be broken */
	U8 tempArray[128]={0};
	
        amTrace((const U8*)"id: %d, size: %d\n" /*parameters: %ld \n"*/,ubVal,*(*pPtr));
        (*pPtr)=(*pPtr)+ubLenght;
	return FALSE;
    break;
 }

}

/* reads Variable Lenght Quantity */
U32 readVLQ(U8 *pChar,U8 *ubSize){
U32 value=0;
U8 c=0;
(*ubSize)=0;
value = (*pChar);
    
if ( (value & 0x80) ){
       value &= 0x7F;

/* get next byte */
pChar++;
(*ubSize)++;

       do{
         value = (value << 7);
         c = (*pChar);
         value = value + (c&0x7F);

         pChar++;
          (*ubSize)++;
       } while (c & 0x80);
    }
    else{
     (*ubSize)++;
    }

return(value);
}

/* combine bytes function for pitch bend */
U16 combineBytes(U8 bFirst, U8 bSecond){
    U16 val;

    val = (U16)bSecond;
    val<<=7;
    val|=(U16)bFirst;
 return(val);
}

/* returns name of MIDI controller */
const U8 *getMIDIcontrollerName(U8 iNb)
{

 return(g_arMIDIcontrollers[iNb]);
}

void getDeviceInfoResponse(U8 channel){
  static U8 getInfoSysEx[]={0xF0,ID_ROLAND,GS_DEVICE_ID,GS_MODEL_ID,0x7E,0x7F,0x06,0x01,0x00,0xF7}; 
  //U8 getInfoSysEx[]={0xF0,0x41,0x10,0x42,0x7E,0x7F,0x06,0x01,0x00,0xF7};
  BOOL bFlag=FALSE;
  
  U32 data=0;
  
  /* calculate checksum */
  getInfoSysEx[5]=am_calcRolandChecksum(&getInfoSysEx[2],&getInfoSysEx[4]);
  getInfoSysEx[5]=channel;
  getInfoSysEx[8]=am_calcRolandChecksum(&getInfoSysEx[5],&getInfoSysEx[7]);  
    
    MIDI_SEND_DATA(10,(void *)getInfoSysEx); 
   // am_dumpMidiBuffer(); 

    /* get reply */
    while(MIDI_DATA_READY) {
      data = GET_MIDI_DATA;
      
      if(data!=0){
	if(bFlag==FALSE){
	 amTrace((const U8*)"Received device info on ch: %d\t",channel);
 	 bFlag=TRUE;
	}
	
	amTrace((const U8*)"%x\t",(unsigned int)data);
      }
    
    
}
}
/* gets info about connected devices via MIDI interface */
const S8 *getConnectedDeviceInfo(void){
  /*  request on all channels */
  amTrace((const U8*)"Quering connected MIDI device...\n");
  
  for(U8 channel=0;channel<0x7f;channel++){
    getDeviceInfoResponse(channel);
   }
 
   //am_dumpMidiBuffer();
 return NULL;
}

/* function for calculating tempo */
/* called each time tempo is changed returned value is assigned to TimeStep value in sequence */
/* TODO: rewrite FPU version in asm in 060 and maybe 030 version */

/* BPM - beats per minute (tempo of music) */
/* UPS - update interval (updates per second) */
/* music resolution are in PPQ */

U32  am_calculateTimeStep(U16 qpm, U16 ppq, U16 ups){
    U32 ppu;
    U32 temp;
    temp=(U32)qpm*(U32)ppq;
    
    if(temp<0x10000){
        ppu=((temp*0x10000)/60)/(U32)ups;
    }
    else{
        ppu=((temp/60)*0x10000)/(U32)ups;
    }
return ppu;
}

/* function for calculating tempo (float version) */
/* called each time tempo is changed returned value is assigned to TimeStep value in sequence */
/* BPM - beats per minute (tempo of music) */
/* UPS - update interval (updates per second) */
/* music resolution are in PPQ */

float  am_calculateTimeStepFlt(U16 qpm, U16 ppq, U16 ups){
    float ppu=0;
    float temp=0;
    ppu=(float)qpm*(float)ppq;
    temp=(temp/ups)/60.0f;
   
 return ppu;
}

/* support functions:
    BPM = 60,000,000/MicroTempo
    MicrosPerPPQN = MicroTempo/TimeBase
    MicrosPerMIDIClock = MicroTempo/24
    PPQNPerMIDIClock = TimeBase/24
    MicrosPerSubFrame = 1000000 * Frames * SubFrames
    SubFramesPerQuarterNote = MicroTempo/(Frames * SubFrames)
    SubFramesPerPPQN = SubFramesPerQuarterNote/TimeBase
    MicrosPerPPQN = SubFramesPerPPQN * Frames * SubFrames
*/

U16 am_decodeTimeDivisionInfo(U16 timeDivision){
  U8 subframe=0;
  
  if(timeDivision&0x8000){
    
    /* SMPTE */
    timeDivision&=0x7FFF;
    subframe=timeDivision>>7;
    amTrace((const U8*)"Timing (SMPTE): %x, %d\n", subframe,(timeDivision&0x00FF));
    return 0;		//todo:
  }
   else{
    /* PPQN */
    amTrace((const U8*)"Timing (PPQN): %d (0x%x)\n", timeDivision,timeDivision);
    return timeDivision;
   }
}

void am_allNotesOff(U16 numChannels){

  for(U16 iCounter=0;iCounter<numChannels;iCounter++){
  all_notes_off(iCounter);
 }
}        

/* utility for measuring function time execution */
double am_diffclock(clock_t end,clock_t begin){
 double diffticks=end-begin;
 double diffms=(diffticks)/(CLOCKS_PER_SEC/1000.0f);
return diffms;
}


const U8 *am_getMIDInoteName(U8 ubNoteNb){
if((ubNoteNb>=0&&ubNoteNb<=127)) /* 0-127 range check */
 return((const U8*)g_arMIDI2key[ubNoteNb]);
else 
  return NULL;
}


void hMidiEvent(void){

if(counter!=0){
  counter--;
  amTrace("hMidiEvent() counter!=0 Decreasing counter.\n");
}else{
  amTrace("hMidiEvent() counter==0 send current event and update g_pEventPtr.\n");
    /* 	
	counter is 0 so we have to:
	send current event
	set g_pEventPtr to next event in the list
    */
    //sending event
    printEventBlock(counter, (sEventBlockPtr_t)&((*g_pEventPtr).eventBlock));
     
    // get next event
    g_pEventPtr=g_pEventPtr->pNext;	
    amTrace("hMidiEvent() setting pointer to new event: %p\n",g_pEventPtr);
    
    if(((g_pEventPtr!=NULL)&&(g_pEventPtr->eventBlock.type!=MT_EOT))){
     //set up counter
     counter=g_pEventPtr->eventBlock.uiDeltaTime;
     amTrace("hMidiEvent() setting counter to new delta value: %d\n",counter);
    
    }else{ 
      amTrace("hMidiEvent() counter=UINT_MAX.\n");
      counter=UINT_MAX;
    }
 }

}

/* sends all midi events with given delta */
/* returns next delta time or 0, if end of track */
/* waits for space keypress after sending all of the data */

/* MIDI replay draft */
/*

int iCurrentDelta=0;
sEventItem *pTemp


*/


void playSequence(const sEventList **listPtr){
  
  if((*listPtr)->pEventList!=NULL){
    g_pEventPtr=(*listPtr)->pEventList;
    iCurrentDelta=0;
    //getTempo 
    installMIDIreplay(MFP_DIV10,59);
  }
}

void am_log(const U8 *mes,...){
static char buffer[256];

va_list va;
va_start(va, mes);
vsprintf(buffer,(const char *)mes,va);
va_end(va);   

if(CON_LOG==TRUE) fprintf(stdout,buffer);

#ifdef DEBUG_FILE_OUTPUT
    fprintf(ofp,buffer);
#endif

return;
}



///////////////////
