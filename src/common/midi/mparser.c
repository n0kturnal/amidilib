
#include "timing/miditim.h"
#include "mformats.h"
#include "config.h"
#include "list/list.h"
#include "midi.h"
#include "mparser.h"

#include "string.h"

#ifdef MIDI_PARSER_DEBUG
#include "amidilib.h"
#endif

#include <alloca.h>

/* for saving last running status */
static U8 g_runningStatus;

//////////////////////////////////////////////////////
U16 am_getTimeDivision (const void *pMidiPtr){
sMThd *pMidiInfo=(sMThd *)pMidiPtr;

/* check midi header */
if(((pMidiInfo->id)==(ID_MTHD)&&(pMidiInfo->headLenght==6L))){
  return (pMidiInfo->division);
}
    /* (X)Midi has timing data inside midi eventlist */
 return (0);
}



/* at this point pCurSequence should have the info about the type of file that resides in memory,
because we have to know if we have to dump event data to one eventlist or several ones */

/* all the events found in the track will be dumped to the sSequenceState_t structure  */


void *processMidiTrackData(void *startPtr, const U32 fileTypeFlag,const U32 numTracks, sSequence_t **pCurSequence, S16 *iError )
{
U32 trackCounter=0;
U32 endAddr=0L;
U32 ulChunkSize=0;

sMThd *pMidiHeader=(sMThd *)startPtr;
sChunkHeader *pHeader=0;
sTrack_t **ppTrack=0;
void *end=0;

amTrace((const U8*)"Number of tracks to process: %d\n\n",numTracks);

pHeader=(sChunkHeader *)startPtr;
startPtr=(U8*)startPtr + sizeof(sChunkHeader);
ulChunkSize=pHeader->headLenght;
endAddr=(U32)startPtr+ulChunkSize;

switch(fileTypeFlag){

    case T_MIDI0:{
      /* we have only one track data to process */
      /* add all of them to given track */
      (*pCurSequence)->seqType=ST_SINGLE;

      sTrack_t *pTempTrack=(*pCurSequence)->arTracks[0];
      pTempTrack->currentState.playState = getGlobalConfig()->initialTrackState&(~(TM_MUTE));
      pTempTrack->currentState.currentTempo=DEFAULT_MPQN;
      pTempTrack->currentState.currentBPM=DEFAULT_BPM;

      ppTrack=&pTempTrack;
      end=(void *)endAddr;

      startPtr=processMIDItrackEvents(*pCurSequence,&startPtr,(const void *)end,ppTrack, iError );

      if(iError<0) {
        return NULL;
      }


    }
    break;
     case T_MIDI1:{
      (*pCurSequence)->seqType=ST_MULTI;

      while(((pHeader!=0)&&(pHeader->id==ID_MTRK)&&(trackCounter<numTracks))){
      /* we have got track data :)) */
      /* add all of them to given track */
      sTrack_t *pTempTrack=(*pCurSequence)->arTracks[trackCounter];

      pTempTrack->currentState.playState = getGlobalConfig()->initialTrackState&(~(TM_MUTE));
      pTempTrack->currentState.currentTempo=DEFAULT_MPQN;
      pTempTrack->currentState.currentBPM=DEFAULT_BPM;

      ppTrack=&pTempTrack;
      end=(void *)endAddr;

      startPtr=processMIDItrackEvents(*pCurSequence,&startPtr,(const void *)end,ppTrack, iError );

      if(*iError<0) {
        return NULL;
      }

      /* increase track counter */
      ++trackCounter;

      //prevent reading chunk after processing the last track
      if(trackCounter<numTracks){
        /* get next data chunk info */
        pHeader=(sChunkHeader *)startPtr;
        ulChunkSize=pHeader->headLenght;

        /* omit Track header */
        startPtr=(U8*)startPtr+sizeof(sChunkHeader);
        endAddr=(U32)startPtr+ulChunkSize;
      }else{
        //just to play safe
        pHeader=0;
        ulChunkSize=0;
        startPtr=0;
        endAddr=0;
      }
    }
    }break;
    case T_MIDI2:{
    /* handle MIDI 2, multitrack type */
    /* create several track lists according to numTracks */
    (*pCurSequence)->seqType=ST_MULTI_SUB;

    /* tracks inited, now insert track data */
    while(((pHeader!=0)&&(pHeader->id==ID_MTRK)&&(trackCounter<numTracks))){
      /* we have got track data :)) */
      /* add all of them to given track */
      sTrack_t *pTempTrack=(*pCurSequence)->arTracks[trackCounter];

      pTempTrack->currentState.playState = getGlobalConfig()->initialTrackState&(~(TM_MUTE));
      pTempTrack->currentState.currentTempo=DEFAULT_MPQN;
      pTempTrack->currentState.currentBPM=DEFAULT_BPM;

      ppTrack=&pTempTrack;
      end=(void *)endAddr;

      startPtr=processMIDItrackEvents(*pCurSequence,&startPtr,(const void *)end,ppTrack, iError );

      if(*iError<0) {
        return NULL;
      }

      /* increase track counter */
      ++trackCounter;

      if(trackCounter<numTracks){
        /* get next data chunk info */
        pHeader=(sChunkHeader *)startPtr;
        ulChunkSize=pHeader->headLenght;

        /* omit Track header */
        startPtr=(U8*)startPtr+sizeof(sChunkHeader);
        endAddr=(U32)startPtr+ulChunkSize;
      }else{
        //just to play safe
        pHeader=0;
        ulChunkSize=0;
        startPtr=0;
        endAddr=0;
      }
    }
    }break;
     case T_XMIDI:
     case T_RMID:
     case T_SMF:
     case T_XMF:

     default:
    /*TODO: unimplemented
      except T_MUS, it is converted to Midi 0 format
    */
     return NULL;
  };

 amTrace((const U8*)"Finished processing...\n");
 return 0;
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

/* combine bytes function for pitch bend */
U16 combineBytes(U8 bFirst, U8 bSecond){
    U16 val;
    val = (U16)bSecond;
    val<<=7;
    val|=(U16)bFirst;
 return(val);
}

/* handles the events in tracks and returns pointer to the next midi track */
void *processMIDItrackEvents(sSequence_t *pSeq, void**startPtr, const void *endAddr, sTrack_t **pCurTrack, S16 *iError ){
U8 usSwitch=0;
U16 recallStatus=0;
U32 delta=0L;
BOOL bEOF=FALSE;

    /* execute as long we are on the end of file or EOT meta occured,
      50% midi track headers is broken, so the web says ;)) */

    U8 *pCmd=((U8 *)(*startPtr));
    U8 ubSize=0;

    while ( ((pCmd!=endAddr)&&(bEOF!=TRUE)&&(*iError>=0)) ){

      /*read delta time, pCmd should point to the command data */
      delta=readVLQ(pCmd,&ubSize);

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
        *iError=am_noteOff(pSeq,&pCmd,&recallStatus, delta, pCurTrack );
      break;
      case EV_NOTE_ON:
        *iError=am_noteOn(pSeq,&pCmd,&recallStatus, delta, pCurTrack );
      break;
      case EV_NOTE_AFTERTOUCH:
        *iError=am_noteAft(pSeq,&pCmd,&recallStatus, delta, pCurTrack );
      break;
      case EV_CONTROLLER:
        *iError=am_Controller(pSeq,&pCmd,&recallStatus, delta, pCurTrack );
      break;
      case EV_PROGRAM_CHANGE:
        *iError=am_PC(pSeq,&pCmd,&recallStatus, delta, pCurTrack );
      break;
      case EV_CHANNEL_AFTERTOUCH:
        *iError=am_ChannelAft(pSeq,&pCmd,&recallStatus, delta, pCurTrack );
      break;
      case EV_PITCH_BEND:
        *iError=am_PitchBend(pSeq,&pCmd,&recallStatus, delta, pCurTrack );
      break;
      case EV_META:
        *iError=am_Meta(pSeq,&pCmd, delta, pCurTrack,&bEOF);
      break;
      case EV_SOX:                          	/* SySEX midi exclusive */
        recallStatus=0; 	                /* cancel out midi running status */
        *iError=(S16)am_Sysex(pSeq,&pCmd,delta, pCurTrack);
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
      default:{
    amTrace((const U8*)"Event: Unknown type: %d\n",(*pCmd));
    /* unknown event, do nothing or maybe throw error? */
      }break;
    }
} /*end of decode events loop */

  /* return the next track data */
  return(pCmd);
}


S16 am_noteOff(sSequence_t *pSeq,U8 **pPtr,U16 *recallRS,U32 delta, sTrack_t **pCurTrack){
sEventBlock_t tempEvent;
sNoteOff_EventBlock_t *pEvntBlock=NULL;
sNoteOff_t *pNoteOff=0;
tempEvent.dataPtr=0;

#ifdef MIDI_PARSER_DEBUG
assert(sizeof(sNoteOff_t)==2);
#endif

if((*recallRS)==0){
  /* save last running status */
  g_runningStatus=*(*pPtr);

  tempEvent.uiDeltaTime=delta;
  tempEvent.type=T_NOTEOFF;

#ifdef IKBD_MIDI_SEND_DIRECT
  getEventFuncCopyInfo(T_NOTEOFF,&tempEvent.copyEventCb);
  tempEvent.dataPtr=alloca(tempEvent.copyEventCb.size);
#else
  getEventFuncInfo(T_NOTEOFF,&tempEvent.sendEventCb);
  tempEvent.dataPtr=alloca(tempEvent.sendEventCb.size);
#endif

  pEvntBlock=(sNoteOff_EventBlock_t *)tempEvent.dataPtr;
  pEvntBlock->ubChannelNb=g_runningStatus&0x0F;

  /* now we can recall former running status next time */
  (*recallRS)=1;

  (*pPtr)++;
  pNoteOff=(sNoteOff_t *)(*pPtr);

  /* get parameters */
  pEvntBlock->eventData.noteNb=pNoteOff->noteNb;
  pEvntBlock->eventData.velocity=pNoteOff->velocity;

  (*pPtr)=(*pPtr)+sizeof(sNoteOff_t);

}else {
  /* recall last cmd status */
  /* and get parameters as usual */

  /* get last note info */
  tempEvent.uiDeltaTime=delta;
  tempEvent.type=T_NOTEOFF;

#ifdef IKBD_MIDI_SEND_DIRECT
  getEventFuncCopyInfo(T_NOTEOFF,&tempEvent.copyEventCb);
  tempEvent.dataPtr=alloca(tempEvent.copyEventCb.size);
#else
  getEventFuncInfo(T_NOTEOFF,&tempEvent.sendEventCb);
  tempEvent.dataPtr=alloca(tempEvent.sendEventCb.size);
#endif

  pEvntBlock=(sNoteOff_EventBlock_t *)tempEvent.dataPtr;

  /* save channel */
  pEvntBlock->ubChannelNb=g_runningStatus&0x0F;

  pNoteOff=(sNoteOff_t *)(*pPtr);

  /* get parameters */
  pEvntBlock->eventData.noteNb=pNoteOff->noteNb;
  pEvntBlock->eventData.velocity=pNoteOff->velocity;

  (*pPtr)=(*pPtr)+sizeof(sNoteOff_t);
}

#ifdef MIDI_PARSER_DEBUG
  amTrace((const U8*)"delta: %lu\t",delta);
  amTrace((const U8*)"event: Note off ");
  amTrace((const U8*)"ch: %d\t",(pEvntBlock->ubChannelNb)+1);
  amTrace((const U8*)"note: %d(%s)\t",pNoteOff->noteNb,getMidiNoteName(pNoteOff->noteNb));
  amTrace((const U8*)"vel: %d\n",pNoteOff->velocity);
#endif

  /* add event to list */

#ifdef EVENT_LINEAR_BUFFER
  return addEvent(pSeq, &(*pCurTrack)->pTrkEventList, &tempEvent );
#else
   return addEvent(&(*pCurTrack)->pTrkEventList, &tempEvent );
#endif
}

//
S16 am_noteOn(sSequence_t *pSeq,U8 **pPtr,U16 *recallRS,U32 delta, sTrack_t **pCurTrack){
sEventBlock_t tempEvent;

 U8 channel=0;
 U8 note=0;
 U8 velocity=0;
 sNoteOn_EventBlock_t *pEvntBlock=NULL;
 tempEvent.dataPtr=0;

 if((*recallRS)==0){
    /* save last running status */
  g_runningStatus=*(*pPtr);

  tempEvent.uiDeltaTime=delta;
  tempEvent.type=T_NOTEON;

#ifdef IKBD_MIDI_SEND_DIRECT
  getEventFuncCopyInfo(T_NOTEON,&tempEvent.copyEventCb);
  tempEvent.dataPtr=alloca(tempEvent.copyEventCb.size);
#else
  getEventFuncInfo(T_NOTEON,&tempEvent.sendEventCb);
  tempEvent.dataPtr=alloca(tempEvent.sendEventCb.size);
#endif

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
 }else{
    /* get last note info */
    channel=(g_runningStatus&0x0F)+1;

    tempEvent.uiDeltaTime=delta;
    tempEvent.type=T_NOTEON;
#ifdef IKBD_MIDI_SEND_DIRECT
    getEventFuncCopyInfo(T_NOTEON,&tempEvent.copyEventCb);
    tempEvent.dataPtr=alloca(tempEvent.copyEventCb.size);
#else
    getEventFuncInfo(T_NOTEON,&tempEvent.sendEventCb);
    tempEvent.dataPtr=alloca(tempEvent.sendEventCb.size);
#endif
    pEvntBlock=(sNoteOn_EventBlock_t *)tempEvent.dataPtr;
    pEvntBlock->ubChannelNb=g_runningStatus&0x0F;

    note=*(*pPtr);
    pEvntBlock->eventData.noteNb=*(*pPtr);
    /* get parameters */
    (*pPtr)++;
    velocity=*(*pPtr);
    pEvntBlock->eventData.velocity=*(*pPtr);

    (*pPtr)++;
 }

 #ifdef MIDI_PARSER_DEBUG
    amTrace((const U8*)"delta: %lu\t",delta);
    amTrace((const U8*)"event: Note on ");
    amTrace((const U8*)"ch: %d\t",channel);
    amTrace((const U8*)"note: %d(%s)\t",note,getMidiNoteName(note));
    amTrace((const U8*)"vel: %d \n",velocity);
 #endif

    /* add event to list */
#ifdef EVENT_LINEAR_BUFFER
  return addEvent(pSeq, &(*pCurTrack)->pTrkEventList, &tempEvent );
#else
   return addEvent(&(*pCurTrack)->pTrkEventList, &tempEvent );
#endif

}

S16 am_noteAft(sSequence_t *pSeq,U8 **pPtr,U16 *recallRS,U32 delta, sTrack_t **pCurTrack){
sEventBlock_t tempEvent;
U8 noteNb=0;
U8 pressure=0;
sNoteAft_EventBlock_t *pEvntBlock=NULL;

 if((*recallRS)==0){
    /* save last running status */
    g_runningStatus=*(*pPtr);

    tempEvent.uiDeltaTime=delta;
    tempEvent.type=T_NOTEAFT;

#ifdef IKBD_MIDI_SEND_DIRECT
    getEventFuncCopyInfo(T_NOTEAFT,&tempEvent.copyEventCb);
    tempEvent.dataPtr=alloca(tempEvent.copyEventCb.size);
#else
    getEventFuncInfo(T_NOTEAFT,&tempEvent.sendEventCb);
    tempEvent.dataPtr=alloca(tempEvent.sendEventCb.size);
#endif
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

 }else{
        /* get parameters */
    tempEvent.uiDeltaTime=delta;
    tempEvent.type=T_NOTEAFT;
#ifdef IKBD_MIDI_SEND_DIRECT
    getEventFuncCopyInfo(T_NOTEAFT,&tempEvent.copyEventCb);
    tempEvent.dataPtr=alloca(tempEvent.copyEventCb.size);
#else
    getEventFuncInfo(T_NOTEAFT,&tempEvent.sendEventCb);
    tempEvent.dataPtr=alloca(tempEvent.sendEventCb.size);
#endif

    pEvntBlock=(sNoteAft_EventBlock_t *)tempEvent.dataPtr;
    pEvntBlock->ubChannelNb=g_runningStatus&0x0F;

    noteNb=*(*pPtr);
    pEvntBlock->eventData.noteNb=*(*pPtr);
        (*pPtr)++;
        pressure=*(*pPtr);
    pEvntBlock->eventData.pressure=*(*pPtr);
        (*pPtr)++;
    }

#ifdef MIDI_PARSER_DEBUG
     amTrace((const U8*)"delta: %u\tevent: Note Aftertouch note: %d, pressure: %d\n",(unsigned long)delta, noteNb,pressure);
#endif
    /* add event to list */
#ifdef EVENT_LINEAR_BUFFER
  return addEvent(pSeq, &(*pCurTrack)->pTrkEventList, &tempEvent );
#else
   return addEvent(&(*pCurTrack)->pTrkEventList, &tempEvent );
#endif
}

S16 am_Controller(sSequence_t *pSeq,U8 **pPtr,U16 *recallRS,U32 delta, sTrack_t **pCurTrack){
sEventBlock_t tempEvent;

  U8 channelNb=0;
    U8 controllerNb=0;
    U8 value=0;
    sController_EventBlock_t *pEvntBlock=NULL;
    tempEvent.dataPtr=0;

    if((*recallRS)==0){
        /* save last running status */
        g_runningStatus=*(*pPtr);
        /* now we can recall former running status next time */
        (*recallRS)=1;

    tempEvent.uiDeltaTime=delta;
    tempEvent.type=T_CONTROL;

#ifdef IKBD_MIDI_SEND_DIRECT
    getEventFuncCopyInfo(T_CONTROL,&tempEvent.copyEventCb);
    tempEvent.dataPtr=alloca(tempEvent.copyEventCb.size);
#else
    getEventFuncInfo(T_CONTROL,&tempEvent.sendEventCb);
    tempEvent.dataPtr=alloca(tempEvent.sendEventCb.size);
#endif


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
    }else{
        channelNb=g_runningStatus&0x0F;
    tempEvent.uiDeltaTime=delta;
    tempEvent.type=T_CONTROL;

#ifdef IKBD_MIDI_SEND_DIRECT
    getEventFuncCopyInfo(T_CONTROL,&tempEvent.copyEventCb);
    tempEvent.dataPtr=alloca(tempEvent.copyEventCb.size);
#else
    getEventFuncInfo(T_CONTROL,&tempEvent.sendEventCb);
    tempEvent.dataPtr=alloca(tempEvent.sendEventCb.size);
#endif

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
    }
#ifdef MIDI_PARSER_DEBUG
    amTrace((const U8*)"delta: %lu\tevent: Controller ch: %d, nb:%d name: %s\tvalue: %d\n",delta, channelNb+1, controllerNb,getMidiControllerName(controllerNb), value);
#endif

    /* add event to list */
#ifdef EVENT_LINEAR_BUFFER
  return addEvent(pSeq, &(*pCurTrack)->pTrkEventList, &tempEvent );
#else
   return addEvent(&(*pCurTrack)->pTrkEventList, &tempEvent );
#endif
}

S16 am_PC(sSequence_t *pSeq,U8 **pPtr,U16 *recallRS,U32 delta, sTrack_t **pCurTrack){
sEventBlock_t tempEvent;

  U8 channel=0;
    U8 PN=0;
    sPrgChng_EventBlock_t *pEvntBlock=NULL;
     tempEvent.dataPtr=0;

     if((*recallRS)==0){
        /* save last running status */
        g_runningStatus=*(*pPtr);

    /* now we can recall former running status next time */
        (*recallRS)=1;

    channel=(g_runningStatus&0x0F)+1;

    tempEvent.uiDeltaTime=delta;
    tempEvent.type=T_PRG_CH;

#ifdef IKBD_MIDI_SEND_DIRECT
    getEventFuncCopyInfo(T_PRG_CH,&tempEvent.copyEventCb);
    tempEvent.dataPtr=alloca(tempEvent.copyEventCb.size);
#else
    getEventFuncInfo(T_PRG_CH,&tempEvent.sendEventCb);
    tempEvent.dataPtr=alloca(tempEvent.sendEventCb.size);
#endif

    pEvntBlock=(sPrgChng_EventBlock_t *)tempEvent.dataPtr;
    pEvntBlock->ubChannelNb=g_runningStatus&0x0F;
        (*pPtr)++;
        /* get parameters */
        PN=*(*pPtr);
    pEvntBlock->eventData.programNb=*(*pPtr);
    (*pPtr)++;
    }else{
      /* get last PC status */
      channel=(g_runningStatus&0x0F)+1;
      tempEvent.uiDeltaTime=delta;
      tempEvent.type=T_PRG_CH;

#ifdef IKBD_MIDI_SEND_DIRECT
      getEventFuncCopyInfo(T_PRG_CH,&tempEvent.copyEventCb);
      tempEvent.dataPtr=alloca(tempEvent.copyEventCb.size);
#else
      getEventFuncInfo(T_PRG_CH,&tempEvent.sendEventCb);
      tempEvent.dataPtr=alloca(tempEvent.sendEventCb.size);
#endif

      pEvntBlock=(sPrgChng_EventBlock_t *)tempEvent.dataPtr;
      pEvntBlock->ubChannelNb=g_runningStatus&0x0F;

      PN=*(*pPtr);
      pEvntBlock->eventData.programNb=*(*pPtr);

     /* get parameters */
      (*pPtr)++;
    }
#ifdef MIDI_PARSER_DEBUG
    amTrace((const U8*)"delta: %lu\t",delta);
    amTrace((const U8*)"event: Program change ");
    amTrace((const U8*)"ch: %d\t",channel);
    amTrace((const U8*)"program nb: %d\n",PN);
#endif

  /* add event to list */
#ifdef EVENT_LINEAR_BUFFER
  return addEvent(pSeq, &(*pCurTrack)->pTrkEventList, &tempEvent );
#else
   return addEvent(&(*pCurTrack)->pTrkEventList, &tempEvent );
#endif
 }

S16 am_ChannelAft(sSequence_t *pSeq,U8 **pPtr,U16 *recallRS,U32 delta, sTrack_t **pCurTrack)
{
sEventBlock_t tempEvent;

U8 channel=0;
U8 param=0;
sChannelAft_EventBlock_t *pEvntBlock=NULL;
tempEvent.dataPtr=0;

if((*recallRS)==0){
        /* save last running status */
        g_runningStatus=*(*pPtr);
        /* now we can recall former running status next time */
        (*recallRS)=1;

    tempEvent.uiDeltaTime=delta;
    tempEvent.type=T_CHAN_AFT;

#ifdef IKBD_MIDI_SEND_DIRECT
    getEventFuncCopyInfo(T_CHAN_AFT,&tempEvent.copyEventCb);
    tempEvent.dataPtr=alloca(tempEvent.copyEventCb.size);
#else
    getEventFuncInfo(T_CHAN_AFT,&tempEvent.sendEventCb);
    tempEvent.dataPtr=alloca(tempEvent.sendEventCb.size);
#endif

    pEvntBlock=(sChannelAft_EventBlock_t *)tempEvent.dataPtr;
    pEvntBlock->ubChannelNb=(g_runningStatus&0x0F);

    /* get parameters */
        (*pPtr)++;
        param=*(*pPtr);
    pEvntBlock->eventData.pressure=*(*pPtr);
        (*pPtr)++;
    }else{
    tempEvent.uiDeltaTime=delta;
    tempEvent.type=T_CHAN_AFT;

#ifdef IKBD_MIDI_SEND_DIRECT
    getEventFuncCopyInfo(T_CHAN_AFT,&tempEvent.copyEventCb);
    tempEvent.dataPtr=alloca(tempEvent.copyEventCb.size);
#else
    getEventFuncInfo(T_CHAN_AFT,&tempEvent.sendEventCb);
    tempEvent.dataPtr=alloca(tempEvent.sendEventCb.size);
#endif

    pEvntBlock=(sChannelAft_EventBlock_t *)tempEvent.dataPtr;
    pEvntBlock->ubChannelNb=(g_runningStatus&0x0F);

        /* get parameters */
    param=*(*pPtr);
    pEvntBlock->eventData.pressure=*(*pPtr);
        (*pPtr)++;
    }
#ifdef MIDI_PARSER_DEBUG
    amTrace((const U8*)"delta: %lu\tevent: Channel aftertouch pressure: %d\n",delta, param);
#endif
    /* add event to list */
#ifdef EVENT_LINEAR_BUFFER
  return addEvent(pSeq, &(*pCurTrack)->pTrkEventList, &tempEvent );
#else
   return addEvent(&(*pCurTrack)->pTrkEventList, &tempEvent );
#endif
}

S16 am_PitchBend(sSequence_t *pSeq, U8 **pPtr,U16 *recallRS,U32 delta, sTrack_t **pCurTrack){
sEventBlock_t tempEvent;

sPitchBend_EventBlock_t *pEvntBlock=NULL;
sPitchBend_t *pPitchBend=0;

#ifdef MIDI_PARSER_DEBUG
  assert(sizeof(sPitchBend_t)==2);
#endif

tempEvent.dataPtr=0;

    if((*recallRS)==0){
        /* save last running status */
        g_runningStatus=*(*pPtr);

        /* now we can recall former running status next time */
        (*recallRS)=1;

    tempEvent.uiDeltaTime=delta;
    tempEvent.type=T_PITCH_BEND;

#ifdef IKBD_MIDI_SEND_DIRECT
    getEventFuncCopyInfo(T_PITCH_BEND,&tempEvent.copyEventCb);
    tempEvent.dataPtr=alloca(tempEvent.copyEventCb.size);
#else
    getEventFuncInfo(T_PITCH_BEND,&tempEvent.sendEventCb);
    tempEvent.dataPtr=alloca(tempEvent.sendEventCb.size);
#endif

    pEvntBlock=(sPitchBend_EventBlock_t *)tempEvent.dataPtr;
    pEvntBlock->ubChannelNb=(g_runningStatus&0x0F);

    (*pPtr)++;
    pPitchBend=(sPitchBend_t *)(*pPtr);

    /* get parameters */
    pEvntBlock->eventData.LSB=pPitchBend->LSB;
    pEvntBlock->eventData.MSB=pPitchBend->MSB;
        (*pPtr)=(*pPtr)+sizeof(sPitchBend_t);
    }else{
    tempEvent.uiDeltaTime=delta;
    tempEvent.type=T_PITCH_BEND;

#ifdef IKBD_MIDI_SEND_DIRECT
    getEventFuncCopyInfo(T_PITCH_BEND,&tempEvent.copyEventCb);
    tempEvent.dataPtr=alloca(tempEvent.copyEventCb.size);
#else
    getEventFuncInfo(T_PITCH_BEND,&tempEvent.sendEventCb);
    tempEvent.dataPtr=alloca(tempEvent.sendEventCb.size);
#endif

    pEvntBlock=(sPitchBend_EventBlock_t *)tempEvent.dataPtr;
    pEvntBlock->ubChannelNb=(g_runningStatus&0x0F);

        /* get parameters */
        pPitchBend=(sPitchBend_t *)(*pPtr);

    /* get parameters */
    pEvntBlock->eventData.LSB=pPitchBend->LSB;
    pEvntBlock->eventData.MSB=pPitchBend->MSB;
        (*pPtr)=(*pPtr)+sizeof(sPitchBend_t);
    }
 #ifdef MIDI_PARSER_DEBUG
 amTrace((const U8*)"delta: %u\tevent: Pitch bend LSB: %d, MSB:%d\n",(unsigned long)delta,pPitchBend->LSB,pPitchBend->MSB);
 #endif

 /* add event to list */
#ifdef EVENT_LINEAR_BUFFER
  return addEvent(pSeq, &(*pCurTrack)->pTrkEventList, &tempEvent );
#else
   return addEvent(&(*pCurTrack)->pTrkEventList, &tempEvent );
#endif
}

S16 am_Sysex(sSequence_t *pSeq, U8 **pPtr,U32 delta, sTrack_t **pCurTrack){
  sEventBlock_t tempEvent;
  sSysEX_EventBlock_t *pEvntBlock=0;
  U8 *pTmpPtr=0;

  U32 ulCount=0L;
  tempEvent.dataPtr=0;

  tempEvent.uiDeltaTime=delta;
  tempEvent.type=T_SYSEX;

#ifdef IKBD_MIDI_SEND_DIRECT
  getEventFuncCopyInfo(T_SYSEX,&tempEvent.copyEventCb);
  tempEvent.dataPtr=alloca(tempEvent.copyEventCb.size);
#else
  getEventFuncInfo(T_SYSEX,&tempEvent.sendEventCb);
  tempEvent.dataPtr=alloca(tempEvent.sendEventCb.size);
#endif

  pEvntBlock=(sSysEX_EventBlock_t *)tempEvent.dataPtr;

#ifdef MIDI_PARSER_DEBUG
  amTrace((const U8*)"SOX: ");
#endif
   pTmpPtr=(*pPtr); //save start

   while( (*(*pPtr))!=EV_EOX){
#ifdef MIDI_PARSER_DEBUG
    amTrace((const U8*)"0x%x, ",*(*pPtr));
#endif
     (*pPtr)++;
      /*count Sysex msg data bytes */
      ulCount++;
    }
    pEvntBlock->bufferSize=ulCount; //size of data
    pEvntBlock->pBuffer=amMallocEx(ulCount*sizeof(U8),PREFER_TT);

    //copy ulCount of data
    amMemCpy(pEvntBlock->pBuffer,pTmpPtr,ulCount*sizeof(U8));

#ifdef MIDI_PARSER_DEBUG
    amTrace((const U8*)" EOX, size: %lu\n",ulCount);
#endif

 /* add event to list */
#ifdef EVENT_LINEAR_BUFFER
  return addEvent(pSeq, &(*pCurTrack)->pTrkEventList, &tempEvent );
#else
   return addEvent(&(*pCurTrack)->pTrkEventList, &tempEvent );
#endif
}

S16 am_Meta(sSequence_t *pSeq, U8 **pPtr,U32 delta, sTrack_t **pCurTrack, BOOL *bEOT){
sEventBlock_t tempEvent;

 U32 addr;
 U8 ubLenght,ubVal,ubSize=0;
 U8 textBuffer[64]={0};
 *bEOT=FALSE;

 tempEvent.dataPtr=0;

 /*get meta event type */
 (*pPtr)++;
 ubVal=*(*pPtr);

 switch(ubVal){
    case MT_SEQ_NB:{
#ifdef MIDI_PARSER_DEBUG
        amTrace((const U8*)"delta: %u\tMeta event: Sequence nb: ",(unsigned long)delta);
#endif
        (*pPtr)++;
        ubLenght=(*(*pPtr));
#ifdef MIDI_PARSER_DEBUG
        amTrace((const U8*)"%d\n", ubLenght);
#endif
        (*pPtr)++;
        addr=((U32)(*pPtr))+ubLenght*sizeof(U8);
        *pPtr=(U8*)addr;
    return 0;
    }break;
    case MT_TEXT:{
#ifdef MIDI_PARSER_DEBUG
        amTrace((const U8*)"delta: %u\tMeta event: Text:",(unsigned long)delta);
#endif
        (*pPtr)++;
        ubLenght=readVLQ((*pPtr),&ubSize);
#ifdef MIDI_PARSER_DEBUG
        amTrace((const U8*)"meta size: %d ",ubLenght);
#endif
    /* set to the start of the string */
        (*pPtr)++;
        amMemCpy(textBuffer, (*pPtr),ubLenght*sizeof(U8) );
        (*pPtr)=((*pPtr)+ubLenght);
#ifdef MIDI_PARSER_DEBUG
        amTrace((const U8*)"%s \n",textBuffer);
#endif
    return 0;
    }break;
    case MT_COPYRIGHT:{
#ifdef MIDI_PARSER_DEBUG
        amTrace((const U8*)"delta: %u\tMeta event: Copyright: ",(unsigned long)delta);
#endif
        (*pPtr)++;
        ubLenght=readVLQ((*pPtr),&ubSize);
#ifdef MIDI_PARSER_DEBUG
        amTrace((const U8*)"meta size: %d ",ubLenght);
#endif
        /* set to the start of the string */
        (*pPtr)++;
        amMemCpy(textBuffer, (*pPtr),ubLenght*sizeof(U8) );
        (*pPtr)=((*pPtr)+ubLenght);
#ifdef MIDI_PARSER_DEBUG
        amTrace((const U8*)"%s \n",textBuffer);
#endif
    return 0;
    }break;
    case MT_SEQNAME:{
#ifdef MIDI_PARSER_DEBUG
        amTrace((const U8*)"delta: %u\tMeta event: Sequence name: ",(unsigned long)delta);
#endif
        (*pPtr)++;
        ubLenght=readVLQ((*pPtr),&ubSize);
        /* set to the start of the string */
        (*pPtr)++;

        (*pCurTrack)->pTrackName=amMallocEx(128*sizeof(U8),PREFER_TT);

    if((*pCurTrack)->pTrackName!=NULL){
      amMemSet((*pCurTrack)->pTrackName,0,128*sizeof(U8));
      strncpy((char *)(*pCurTrack)->pTrackName,(char *)(*pPtr),ubLenght);
      (*pCurTrack)->pTrackName[ubLenght]='\0';
      //amMemCpy((*pCurTrack)->pTrackName, (*pPtr),ubLenght*sizeof(U8));
    }

        (*pPtr)=((*pPtr)+ubLenght);
#ifdef MIDI_PARSER_DEBUG
        amTrace((const U8*)"meta size: %d ",ubLenght);
        amTrace((const U8*)"%s \n",(*pCurTrack)->pTrackName);
#endif
    return 0;
    }break;
    case MT_INSTRNAME:{
#ifdef MIDI_PARSER_DEBUG
        amTrace((const U8*)"delta: %u\tMeta event: Instrument name: ",(unsigned long)delta);
#endif
        (*pPtr)++;
        ubLenght=readVLQ((*pPtr),&ubSize);
#ifdef MIDI_PARSER_DEBUG
        amTrace((const U8*)"meta size: %d",ubLenght);
#endif
        /* set to the start of the string */
        (*pPtr)++;
        amMemCpy(textBuffer, (*pPtr),ubLenght*sizeof(U8) );
        (*pPtr)=((*pPtr)+ubLenght);

#ifdef MIDI_PARSER_DEBUG
        amTrace((const U8*)"%s \n",textBuffer);
#endif
    return 0;
    }break;
    case MT_LYRICS:{
#ifdef MIDI_PARSER_DEBUG
        amTrace((const U8*)"delta: %u\tMeta event: Lyrics: ",(unsigned long)delta);
#endif
        (*pPtr)++;
        ubLenght=readVLQ((*pPtr),&ubSize);
#ifdef MIDI_PARSER_DEBUG
        amTrace((const U8*)"meta size: %d ",ubLenght);
#endif
        /* set to the start of the string */
        (*pPtr)++;
        amMemCpy(textBuffer, (*pPtr),ubLenght*sizeof(U8));
        (*pPtr)=((*pPtr)+ubLenght);
#ifdef MIDI_PARSER_DEBUG
        amTrace((const U8*)"%s \n",textBuffer);
#endif
    return 0;
    }break;

    case MT_MARKER:{
#ifdef MIDI_PARSER_DEBUG
        amTrace((const U8*)"delta: %u\tMeta event: Marker: ",(unsigned long)delta);
#endif
    sMarker_EventBlock_t *pEvntBlock=0;

        (*pPtr)++;
        ubLenght=readVLQ((*pPtr),&ubSize);

    /* set to the start of the string */
        (*pPtr)++;
        tempEvent.uiDeltaTime=delta;
    tempEvent.type=T_META_MARKER;

#ifdef IKBD_MIDI_SEND_DIRECT
    getEventFuncCopyInfo(T_META_MARKER,&tempEvent.copyEventCb);
    tempEvent.dataPtr=alloca(tempEvent.copyEventCb.size);
#else
    getEventFuncInfo(T_META_MARKER,&tempEvent.sendEventCb);
    tempEvent.dataPtr=alloca(tempEvent.sendEventCb.size);
#endif

    pEvntBlock=(sMarker_EventBlock_t *)tempEvent.dataPtr;
    pEvntBlock->pMarkerName=amMallocEx(ubLenght+1,PREFER_TT);
    amMemSet(pEvntBlock->pMarkerName,0,((ubLenght+1)*sizeof(U8)));
    amMemCpy(pEvntBlock->pMarkerName,(*pPtr),ubLenght*sizeof(U8));
    pEvntBlock->pMarkerName[ubLenght]='\0';
    (*pPtr)=((*pPtr)+ubLenght);

    /* add event to list */
#ifdef EVENT_LINEAR_BUFFER
   addEvent(pSeq, &(*pCurTrack)->pTrkEventList, &tempEvent );
#else
   addEvent(&(*pCurTrack)->pTrkEventList, &tempEvent );
#endif

#ifdef MIDI_PARSER_DEBUG
        amTrace((const U8*)"meta size: %d ",ubLenght);
        amTrace((const U8*)"%s \n",textBuffer);
#endif
    return 0;
    }
    break;
    case MT_CUEPOINT:{
#ifdef MIDI_PARSER_DEBUG
        amTrace((const U8*)"delta: %u\tMeta event: Cuepoint\n",(unsigned long)delta);
#endif
    sCuePoint_EventBlock_t *pEvntBlock=0;

        (*pPtr)++;
        ubLenght=readVLQ((*pPtr),&ubSize);
        /* set to the start of the string */
        (*pPtr)++;

    tempEvent.uiDeltaTime=delta;
    tempEvent.type=T_META_CUEPOINT;

#ifdef IKBD_MIDI_SEND_DIRECT
    getEventFuncCopyInfo(T_META_CUEPOINT,&tempEvent.copyEventCb);
    tempEvent.dataPtr=alloca(tempEvent.copyEventCb.size);
#else
    getEventFuncInfo(T_META_CUEPOINT,&tempEvent.sendEventCb);
    tempEvent.dataPtr=alloca(tempEvent.sendEventCb.size);
#endif

    pEvntBlock=(sCuePoint_EventBlock_t *)tempEvent.dataPtr;
    pEvntBlock->pCuePointName=0;

    pEvntBlock->pCuePointName=amMallocEx(ubLenght+1,PREFER_TT);
    amMemSet(pEvntBlock->pCuePointName,0,((ubLenght+1)*sizeof(U8)));
    amMemCpy(pEvntBlock->pCuePointName,(*pPtr),ubLenght*sizeof(U8));
    pEvntBlock->pCuePointName[ubLenght]='\0';

    (*pPtr)=((*pPtr)+ubLenght);

    /* add event to list */
#ifdef EVENT_LINEAR_BUFFER
   addEvent(pSeq, &(*pCurTrack)->pTrkEventList, &tempEvent );
#else
   addEvent(&(*pCurTrack)->pTrkEventList, &tempEvent );
#endif

#ifdef MIDI_PARSER_DEBUG
        amTrace((const U8*)"meta size: %d ",ubLenght);
        amTrace((const U8*)"%s \n",pEvntBlock->pCuePointName);
#endif
    return 0;
    }break;

    case MT_PROGRAM_NAME:{
        /* program(patch) name */
#ifdef MIDI_PARSER_DEBUG
        amTrace((const U8*)"delta: %u\tMeta event: Program (patch) name: ",(unsigned long)delta);
#endif
        (*pPtr)++;
        ubLenght=readVLQ((*pPtr),&ubSize);

        /* set to the start of the string */
        (*pPtr)++;
        amMemCpy(textBuffer, (*pPtr),ubLenght*sizeof(U8) );
        (*pPtr)=((*pPtr)+ubLenght);

#ifdef MIDI_PARSER_DEBUG
        amTrace((const U8*)"meta size: %d ",ubLenght);
        amTrace((const U8*)"%s \n",textBuffer);
#endif
    return 0;
    }break;
    case MT_DEVICE_NAME:{
        /* device (port) name */
#ifdef MIDI_PARSER_DEBUG
    amTrace((const U8*)"delta: %u\tMeta event: Device (port) name: ",(unsigned long)delta);
#endif
        (*pPtr)++;
        ubLenght=readVLQ((*pPtr),&ubSize);

        /* set to the start of the string */
        (*pPtr)++;
        amMemCpy(textBuffer, (*pPtr),ubLenght*sizeof(U8) );
        (*pPtr)=((*pPtr)+ubLenght);
#ifdef MIDI_PARSER_DEBUG
        amTrace((const U8*)"meta size: %d ",ubLenght);
        amTrace((const U8*)"%s \n",textBuffer);
#endif
    return 0;
    }break;
    case MT_CH_PREFIX:{
#ifdef MIDI_PARSER_DEBUG
        amTrace((const U8*)"delta: %u\tMeta event: Channel prefix\n",(unsigned long)delta);
#endif
        (*pPtr)++;
        ubLenght=(*(*pPtr));
        (*pPtr)++;
        addr=((U32)(*pPtr))+ubLenght*sizeof(U8);
        *pPtr=(U8*)addr;
    return 0;
    }break;
    case MT_MIDI_CH:{ /* obsolete! just ignore */
        (*pPtr)++;
        /*get size */
        ubLenght=(*(*pPtr));
        (*pPtr)++;

        /*get port nb*/
#ifdef MIDI_PARSER_DEBUG
        amTrace((const U8*)"delta: %u\tMeta event: Midi channel nb: %d\n",(unsigned long)delta,*(*pPtr));
#endif
        (*pPtr)++;
    return FALSE;
    }break;
    case MT_MIDI_PORT:{ /* obsolete! just ignore */
        (*pPtr)++;
        /*get size */
        ubLenght=(*(*pPtr));
        (*pPtr)++;

        /*get port nb*/
#ifdef MIDI_PARSER_DEBUG
        amTrace((const U8*)"delta: %u\tMeta event: Midi port nb: %d\n",(unsigned long)delta,*(*pPtr));
#endif
        (*pPtr)++;
    return 0;
    }break;
    case MT_EOT:{
#ifdef MIDI_PARSER_DEBUG
        amTrace((const U8*)"delta: %u\tMeta event: End of track\n\n",(unsigned long)delta);
#endif
    sEot_EventBlock_t *pEvntBlock=0;
        (*pPtr)++;
        ubLenght=(*(*pPtr));
        (*pPtr)++;
        addr=((U32)(*pPtr))+ubLenght*sizeof(U8);
        *pPtr=(U8*)addr;

    tempEvent.uiDeltaTime=delta;
    tempEvent.type=T_META_EOT;

#ifdef IKBD_MIDI_SEND_DIRECT
    getEventFuncCopyInfo(T_META_EOT,&tempEvent.copyEventCb);
    tempEvent.dataPtr=alloca(tempEvent.copyEventCb.size);
#else
    getEventFuncInfo(T_META_EOT,&tempEvent.sendEventCb);
    tempEvent.dataPtr=alloca(tempEvent.sendEventCb.size);
#endif

    pEvntBlock=(sEot_EventBlock_t *)tempEvent.dataPtr;
    pEvntBlock->dummy=0L;		//dummy value

    *bEOT=TRUE;
    /* add event to list */
#ifdef EVENT_LINEAR_BUFFER
  return addEvent(pSeq, &(*pCurTrack)->pTrkEventList, &tempEvent );
#else
   return addEvent(&(*pCurTrack)->pTrkEventList, &tempEvent );
#endif
    }break;
    case MT_SET_TEMPO:{
        /* sets tempo in track, should be in the first track, if not 120 BPM is assumed */
    U32 val1,val2,val3;
    U8 ulVal[3]={0};   /* for retrieving set tempo info */
    sTempo_EventBlock_t *pEvntBlock=NULL;
    #ifdef MIDI_PARSER_DEBUG
    amTrace((const U8*)"delta: %u\tMeta event: Set tempo: ",(unsigned long)delta);
    #endif

    (*pPtr)++;
        ubLenght=(*(*pPtr));
         (*pPtr)++;
        /* get those 3 bytes */
        amMemCpy(ulVal, (*pPtr),ubLenght*sizeof(U8) );
        addr=((U32)(*pPtr))+ubLenght*sizeof(U8);
        *pPtr=(U8*)addr;

    val1=ulVal[0],val2=ulVal[1],val3=ulVal[2];
    val1=(val1<<16)&0x00FF0000L;
    val2=(val2<<8)&0x0000FF00L;
    val3=(val3)&0x000000FFL;

    /* range: 0-8355711 ms, 24 bit value */
    val1=val1|val2|val3;

    tempEvent.uiDeltaTime=delta;
    tempEvent.type=T_META_SET_TEMPO;

#ifdef IKBD_MIDI_SEND_DIRECT
    getEventFuncCopyInfo(T_META_SET_TEMPO,&tempEvent.copyEventCb);
    tempEvent.dataPtr=alloca(tempEvent.copyEventCb.size);
#else
    getEventFuncInfo(T_META_SET_TEMPO,&tempEvent.sendEventCb);
    tempEvent.dataPtr=alloca(tempEvent.sendEventCb.size);
#endif

    pEvntBlock=(sTempo_EventBlock_t *)tempEvent.dataPtr;
    pEvntBlock->eventData.tempoVal=	val1;

#ifdef MIDI_PARSER_DEBUG
    amTrace((const U8*)"%lu ms per quarter-note\n", val1);
#endif
    /* add event to list */
#ifdef EVENT_LINEAR_BUFFER
  return addEvent(pSeq, &(*pCurTrack)->pTrkEventList, &tempEvent );
#else
   return addEvent(&(*pCurTrack)->pTrkEventList, &tempEvent );
#endif
    }
    break;
    case MT_SMPTE_OFFSET:{
       sSMPTEoffset SMPTEinfo;
#ifdef MIDI_PARSER_DEBUG
      amTrace((const U8*)"delta: %lu\tMeta event: SMPTE offset:\n",delta);
#endif
        (*pPtr)++;
        ubLenght=(*(*pPtr));
        (*pPtr)++;
        amMemCpy (&SMPTEinfo,(*pPtr),sizeof(sSMPTEoffset));

        addr=((U32)(*pPtr))+ubLenght*sizeof(U8);
        *pPtr=(U8*)addr;
/* print out info */
#ifdef MIDI_PARSER_DEBUG
        amTrace((const U8*)"hr: %d\n",SMPTEinfo.hr);
        amTrace((const U8*)"mn: %d\n",SMPTEinfo.mn);
        amTrace((const U8*)"se: %d\n",SMPTEinfo.fr);
        amTrace((const U8*)"fr: %d\n",SMPTEinfo.fr);
        amTrace((const U8*)"ff: %d\n",SMPTEinfo.ff);
#endif
      return 0;
    }break;
    case MT_TIME_SIG:{
     sTimeSignature timeSign;
     sTimeSignature_EventBlock_t *pEvntBlock=NULL;

#ifdef MIDI_PARSER_DEBUG
        amTrace((const U8*)"delta: %lu\tMeta event: Time signature: ",delta);
#endif
        (*pPtr)++;
        ubLenght=(*(*pPtr));
        (*pPtr)++;

        amMemCpy(&timeSign,(*pPtr),sizeof(sTimeSignature));
        addr=((U32)(*pPtr))+ubLenght*sizeof(U8);
        *pPtr=(U8*)addr;

    tempEvent.uiDeltaTime=delta;
    tempEvent.type = T_META_SET_SIGNATURE;

#ifdef IKBD_MIDI_SEND_DIRECT
    getEventFuncCopyInfo(T_META_SET_SIGNATURE,&tempEvent.copyEventCb);
    tempEvent.dataPtr=alloca(tempEvent.copyEventCb.size);
#else
    getEventFuncInfo(T_META_SET_SIGNATURE,&tempEvent.sendEventCb);
    tempEvent.dataPtr=alloca(tempEvent.sendEventCb.size);
#endif

    pEvntBlock=(sTimeSignature_EventBlock_t *)tempEvent.dataPtr;
    pEvntBlock->timeSignature=timeSign;

        /* print out info */
#ifdef MIDI_PARSER_DEBUG
        amTrace((const U8*)"Time signature change nn: %d\tdd: %d\tcc: %d\tbb: %d\r\n",timeSign.nn,timeSign.dd,timeSign.cc,timeSign.bb);
#endif
    /* add event to list */
#ifdef EVENT_LINEAR_BUFFER
  return addEvent(pSeq, &(*pCurTrack)->pTrkEventList, &tempEvent );
#else
   return addEvent(&(*pCurTrack)->pTrkEventList, &tempEvent );
#endif

    }break;
    case MT_KEY_SIG:{
        U8 param1=0,param2=0;

#ifdef MIDI_PARSER_DEBUG
        amTrace((const U8*)"delta: %lu\tMeta event: Key signature: ",delta);
#endif
        (*pPtr)++;
        ubLenght=(*(*pPtr));
        (*pPtr)++;
        param1=(*(*pPtr));
        (*pPtr)++;
        param2=(*(*pPtr));
#ifdef MIDI_PARSER_DEBUG
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
#endif

        (*pPtr)++;
    return 0;
    }break;
    case MT_SEQ_SPEC:{
    #ifdef MIDI_PARSER_DEBUG
      amTrace((const U8*)"delta: %lu\tMeta event: Sequencer specific data.\n",delta);
    #endif
        (*pPtr)++;
        ubLenght=(*(*pPtr));
        (*pPtr)++;
        addr=((U32)(*pPtr))+ubLenght*sizeof(U8);
        *pPtr=(U8*)addr;
    return 0;
    }break;
    default:{
    #ifdef MIDI_PARSER_DEBUG
       amTrace((const U8*)"delta: %lu\tUnknown meta event.\n",delta);
    #endif
        (*pPtr)++;
        ubLenght=(*(*pPtr));
        /* we should put here assertion failed or something with "send this file to author" message */
        /* file also could be broken */

    #ifdef MIDI_PARSER_DEBUG
        amTrace((const U8*)"id: %d, size: %d\n" /*parameters: %ld \n"*/,ubVal,*(*pPtr));
    #endif
        (*pPtr)=(*pPtr)+ubLenght;
    return 0;
    }break;
 }

}

