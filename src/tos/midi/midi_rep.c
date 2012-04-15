
//tos version

#include "include/midi_rep.h"

#include "include/timing/mfp.h"

#include "include/amidilib.h"
#include "include/amidiseq.h"
#include "include/midi_send.h"
#include "include/list/list.h"

#include <math.h>

extern volatile sSequence_t *pCurrentSequence;
extern volatile U16 startPlaying;
extern volatile U8 tbData;
extern volatile U8 tbMode;


void initSeq(sSequence_t *seq){
if(seq!=0){
  U8 activeTrack=seq->ubActiveTrack;
  U32 mode=0,data=0;
  pCurrentSequence=seq;
  
  U32 dd,nn,cc;
  dd=seq->arTracks[activeTrack]->currentState.timeSignature.dd;
  nn=seq->arTracks[activeTrack]->currentState.timeSignature.nn;
  cc=seq->arTracks[activeTrack]->currentState.timeSignature.cc;
  dd=(U16)pow(2.0f,(float)dd);
  
  //convert tempo from microseconds to seconds
  float freq=((float)(seq->arTracks[activeTrack]->currentState.currentTempo/(dd/4)*(cc/24))/1000000.0f);
  float fTempoSecs=seq->arTracks[activeTrack]->currentState.currentTempo/1000000.0f;
      
  //calculate 1 tick duration
  fTempoSecs=fTempoSecs/(pCurrentSequence->timeDivision);
      
  //calculate hz
  fTempoSecs=(1.0f/(float)fTempoSecs);	
  
  pCurrentSequence->pulseCounter=0;  
  pCurrentSequence->divider=0;  //not used atm
  
#ifdef DEBUG_BUILD
  amTrace("freq: %f %u\n",fTempoSecs,(U32)freq);
#endif  
  getMFPTimerSettings((U32)fTempoSecs,&mode,&data);

#ifdef DEBUG_BUILD
  amTrace("calculated mode: %d, data: %d\n",mode,data);
#endif 
  startPlaying=1;	
  installReplayRout(mode, data);
  
}
 return;
}

extern volatile sSequence_t *pCurrentSequence;	//here is stored current sequence
extern volatile U8 MIDIsendBuffer[]; //buffer from which we will send all data from the events
				     //once per 
extern volatile U16 MIDIbufferReady; //flag indicating buffer ready for sending data


//this will be called from an interrupt in each delta increment
void sequenceUpdate(){
 static sTrack_t *pTrk=0;
 static sEventList *pCurrent=0;
 static evntFuncPtr myFunc=0; 
 static sSequenceState_t *seqState=0;
 static const U8 MIDI_START=0xFA;
 static const U8 MIDI_STOP=0xFC;
 
 BOOL bNoteOffSent=FALSE;		//flag for sending note off events only once when pausing or stopping the sequence
 static U32 silenceThrCounter=0;	//for tracking silence lenght on all tracks
 
 if(pCurrentSequence){ 
  
  U8 activeTrack=pCurrentSequence->ubActiveTrack;
  seqState=&(pCurrentSequence->arTracks[activeTrack]->currentState);
  
  switch(seqState->playState){
    
    case PS_PLAYING:{
    
    bNoteOffSent=FALSE; //for handling PS_STOPPED and PS_PAUSED states
    
    //for each track (0-> (numTracks-1) ) 
    for (U32 i=0;i<pCurrentSequence->ubNumTracks;i++){
      pTrk=pCurrentSequence->arTracks[i];

      if(pTrk!=0){
	pCurrent=pTrk->currentState.pCurrent;
		
	//sometimes eventlist will be empty, because there will be nothing interesting
	//to send in parsed file, usually in the first track of MIDI 1 file (for example) 
	if(pCurrent!=NULL){ 
	
	//if (internal counter == current event delta)
	if((pCurrent->eventBlock.uiDeltaTime)==(pTrk->currentState.deltaCounter)){
#ifdef DEBUG_BUILD 
	printEventBlock(&(pCurrent->eventBlock));
#endif		
	if(!(pTrk->currentState.bMute)){
	  //output data only when track is not mute
	  myFunc= pCurrent->eventBlock.infoBlock.func;
	  //TODO: slap raw data to buffer
	  
	}else{
	 ;  //TODO: silence whole channel
	 //all_notes_off(U8 channel)
	}
		    
	pCurrent=pCurrent->pNext;
	pTrk->currentState.pCurrent=pCurrent;
	
	//check if next event isn't NULL, if yes do nothing
	// else check if event delta==0 if yes keep sending events, otherwise there is nothing to do
	while(((pCurrent!=0)&&(pCurrent->eventBlock.uiDeltaTime==0))){
	  //send data with delta==0
#ifdef DEBUG_BUILD 
	printEventBlock(&(pCurrent->eventBlock));
#endif		
	  if(!(pTrk->currentState.bMute)){
	    //the same as above
	    //myFunc= pCurrent->eventBlock.infoBlock.func;
	    //TODO: slap raw data to buffer
	 
	  }else{
	   ; //TODO: silence whole channel
	   //all_notes_off(U8 channel)
	  }
	//next
	  pCurrent=pCurrent->pNext;
	  pTrk->currentState.pCurrent=pCurrent;
	}
	// done reset internal track counter
	// pMidiTune->arTracks[i]->currentState.pCurrent should point to event with NULL or
	// event with delta higher than 0
	pTrk->currentState.deltaCounter=0;
#ifdef DEBUG_BUILD 
	amTrace((const U8 *)"reset track: %d counter\n",i);
#endif
	}else{
	// else internal counter++; 
	++pCurrentSequence->arTracks[i]->currentState.deltaCounter;
#ifdef DEBUG_BUILD 
	amTrace((const U8 *)"increase track %d counter\n",i);
#endif
	}
	}
#ifdef DEBUG_BUILD 
else{
	//quite normal
	amTrace((const U8 *)"Nothing to send in this track..\n");}
#endif
}
}//track iteration

//TODO: check buffer if not empty end ready send all midi data 

    //handle tempo update
    if(seqState->currentTempo!=seqState->newTempo){
      //update track current tempo
      seqState->currentTempo=seqState->newTempo;

      U32 mode=0,data=0;
     
      U32 dd,nn,cc;
      dd=seqState->timeSignature.dd;
      nn=seqState->timeSignature.nn;
      cc=seqState->timeSignature.cc;
  
      dd=(U32)pow(2.0f,(float)dd);
      
      //convert quaternote duration in microseconds to seconds
      float freq=(float)seqState->currentTempo/1000000.0f;

      //calculate one tick duration in seconds(quaternote duration)
      freq=(freq/(dd/4))/(pCurrentSequence->timeDivision/(dd/4))*nn;
      
      //calculate hz
      freq=1.0f/freq;	
     
      getMFPTimerSettings(freq,&mode,&data);
      tbMode=(U8)mode;
      tbData=(U8)data;
      
    }
    
      //increase our cumulated delta
      ++pCurrentSequence->accumulatedDeltaCounter;
      
    }break;
    case PS_PAUSED:{
      
         if(bNoteOffSent==FALSE){
	  //turn all notes off on external module but only once
	  bNoteOffSent=TRUE;
	  am_allNotesOff(16);
	}
      
    }break;
    case PS_STOPPED:{
      silenceThrCounter=0;
      
      //reset all counters, but only once
      if(bNoteOffSent==FALSE){
	  pCurrentSequence->accumulatedDeltaCounter=0;
	  
	  for (U32 i=0;i<pCurrentSequence->ubNumTracks;i++){
	    pTrk=pCurrentSequence->arTracks[i];
	    pTrk->currentState.deltaCounter=0;
	    pTrk->currentState.pCurrent=pTrk->currentState.pStart;
	  }
	  //turn all notes off on external module
	  bNoteOffSent=TRUE;
	  startPlaying=1;	//to indicate that we have to send MIDI start on next play
	  MIDI_SEND_BYTE((const U8 *)&MIDI_STOP);	//send midi STOP
	  am_allNotesOff(16);
	//done! 
	}
    }break;
  };
 }//pCurrentSequence null check
}


//this will be called from an interrupt in each delta increment
void sequenceUpdate2(void){
  
 static sTrack_t *pTrk=0;
 static sEventList *pCurrent=0;
 static evntFuncPtr myFunc=0; 
 static sSequenceState_t *seqState=0;
 static const U8 MIDI_START=0xFA;
 static const U8 MIDI_STOP=0xFC;
 
 BOOL bNoteOffSent=FALSE;		//flag for sending note off events only once when pausing or stopping the sequence
 static U32 silenceThrCounter=0;	//for tracking silence lenght on all tracks
 
 if(pCurrentSequence){ 
  
  U8 activeTrack=pCurrentSequence->ubActiveTrack;
  seqState=&(pCurrentSequence->arTracks[activeTrack]->currentState);
  
  switch(seqState->playState){
    
    case PS_PLAYING:{
    
    bNoteOffSent=FALSE; //for handling PS_STOPPED and PS_PAUSED states
    
    //for each track (0-> (numTracks-1) ) 
    for (U32 i=0;i<pCurrentSequence->ubNumTracks;i++){
      pTrk=pCurrentSequence->arTracks[i];

      if(pTrk!=0){
	pCurrent=pTrk->currentState.pCurrent;
		
	//sometimes eventlist will be empty, because there will be nothing interesting
	//to send in parsed file, usually in the first track of MIDI 1 file (for example) 
	if(pCurrent!=NULL){ 
	
	//if (internal counter == current event delta)
	if((pCurrent->eventBlock.uiDeltaTime)==(pTrk->currentState.deltaCounter)){
#ifdef DEBUG_BUILD 
	printEventBlock(&(pCurrent->eventBlock));
#endif		
	if(!(pTrk->currentState.bMute)){
	  //output data only when track is not mute
	  myFunc= pCurrent->eventBlock.infoBlock.func;
	  (*myFunc)((void *)pCurrent->eventBlock.dataPtr);
	}else{
	 ;  //TODO: silence whole channel
	 //all_notes_off(U8 channel)
	}
		    
	pCurrent=pCurrent->pNext;
	pTrk->currentState.pCurrent=pCurrent;
	
	//check if next event isn't NULL, if yes do nothing
	// else check if event delta==0 if yes keep sending events, otherwise there is nothing to do
	while(((pCurrent!=0)&&(pCurrent->eventBlock.uiDeltaTime==0))){
	  //send data with delta==0
#ifdef DEBUG_BUILD 
	printEventBlock(&(pCurrent->eventBlock));
#endif		
	  if(!(pTrk->currentState.bMute)){
	    //the same as above
	    myFunc= pCurrent->eventBlock.infoBlock.func;
	    (*myFunc)((void *)pCurrent->eventBlock.dataPtr);
	  }else{
	   ; //TODO: silence whole channel
	   //all_notes_off(U8 channel)
	  }
	//next
	  pCurrent=pCurrent->pNext;
	  pTrk->currentState.pCurrent=pCurrent;
	}
	// done reset internal track counter
	// pMidiTune->arTracks[i]->currentState.pCurrent should point to event with NULL or
	// event with delta higher than 0
	pTrk->currentState.deltaCounter=0;
#ifdef DEBUG_BUILD 
	amTrace((const U8 *)"reset track: %d counter\n",i);
#endif
	}else{
	// else internal counter++; 
	++pCurrentSequence->arTracks[i]->currentState.deltaCounter;
#ifdef DEBUG_BUILD 
	amTrace((const U8 *)"increase track %d counter\n",i);
#endif
	}
	}
#ifdef DEBUG_BUILD 
else{
	//quite normal
	amTrace((const U8 *)"Nothing to send in this track..\n");}
#endif
}
}//track iteration
    //handle tempo update
    if(seqState->currentTempo!=seqState->newTempo){
      //update track current tempo
      seqState->currentTempo=seqState->newTempo;
      U32 mode=0,data=0;
     
      U32 dd,nn,cc;
      dd=seqState->timeSignature.dd;
      nn=seqState->timeSignature.nn;
      cc=seqState->timeSignature.cc;
  
      dd=(U32)pow(2.0f,(float)dd);
      
      //convert quaternote duration in microseconds to seconds
      float freq=(float)seqState->currentTempo/1000000.0f;

      //calculate one tick duration in seconds(quaternote duration)
      freq=(freq/(dd/4))/(pCurrentSequence->timeDivision/(dd/4))*nn;
      
      //calculate hz
      freq=1.0f/freq;	
     
      getMFPTimerSettings(freq,&mode,&data);
      tbMode=(U8)mode;
      tbData=(U8)data;
      
    }
    
    /////////////////////// check for end of the track 
//     if(silenceThrCounter==pCurrentSequence->eotThreshold){
//       //we have meet the threshold, time to decide what to do next
//       // if we play in loop mode: do not change actual state,reset track to the beginning
//       
//       switch(seqState->playMode){
// 	
// 	case S_PLAY_LOOP:{
// 	   pCurrentSequence->accumulatedDeltaCounter=0;	//reset cumulated delta
// 	  
// 	  for (U32 i=0;i<pCurrentSequence->ubNumTracks;i++){
// 	    pTrk=pCurrentSequence->arTracks[i];
// 	    pTrk->currentState.deltaCounter=0;
// 	    pTrk->currentState.pCurrent=pTrk->currentState.pStart;
// 	  }
// 	  // notes off we could maybe do other things too
// 	  am_allNotesOff(16);
// 	  silenceThrCounter=0;
// 	}break;
// 	case S_PLAY_ONCE:{
// 	  //set state only, the rest will be done in next interrupt
// 	  seqState->playState=PS_STOPPED;
// 	  silenceThrCounter=0;
// 	}break;
//       };
//     }else{ 
//       //silence threshold not met, check if all the tracks events are null if yes increase silenceThrCounter
//       //else reset it
//       BOOL bNothing=FALSE;
//       for (U32 i=0;i<pCurrentSequence->ubNumTracks;i++){
// 	pTrk=pCurrentSequence->arTracks[i];
// 	//TODO: incorporate it in the playing state loop, so we can get rid of track iteration (?)
// 	
// 	if(pTrk!=0){
// 	  if(pTrk->currentState.pCurrent!=0){
// 	    bNothing=FALSE;
// 	  }else 
// 	    bNothing=TRUE;
//       } /////////////////////////// end of eot check
//     }//end of track loop
//       if(bNothing==TRUE){
// 	    ++silenceThrCounter;
// 	  }else 
// 	    silenceThrCounter=0;
//       }//end of silence threshold not met
//     
      //increase our cumulated delta
      ++pCurrentSequence->accumulatedDeltaCounter;
      
    }break;
    case PS_PAUSED:{
      
         if(bNoteOffSent==FALSE){
	  //turn all notes off on external module but only once
	  bNoteOffSent=TRUE;
	  am_allNotesOff(16);
	}
      
    }break;
    case PS_STOPPED:{
      silenceThrCounter=0;
      
      //reset all counters, but only once
      if(bNoteOffSent==FALSE){
	  pCurrentSequence->accumulatedDeltaCounter=0;
	  
	  for (U32 i=0;i<pCurrentSequence->ubNumTracks;i++){
	    pTrk=pCurrentSequence->arTracks[i];
	    pTrk->currentState.deltaCounter=0;
	    pTrk->currentState.pCurrent=pTrk->currentState.pStart;
	  }
	  //turn all notes off on external module
	  bNoteOffSent=TRUE;
	  startPlaying=1;	//to indicate that we have to send MIDI start on next play
	  MIDI_SEND_BYTE((const U8 *)&MIDI_STOP);	//send midi STOP
	  am_allNotesOff(16);
	//done! 
	}
    }break;
  };
 }//pCurrentSequence null check
}//sequenceUpdate2() end

//replay control
BOOL isSeqPlaying(void){
  if(pCurrentSequence!=0){
    U8 activeTrack=pCurrentSequence->ubActiveTrack;
    if((pCurrentSequence->arTracks[activeTrack]->currentState.playState==PS_PLAYING)) 
      return TRUE;
    else 
      return FALSE;
  }
  return FALSE;
}


void stopSeq(void){
  if(pCurrentSequence!=0){
    U8 activeTrack=pCurrentSequence->ubActiveTrack;
    if(pCurrentSequence->arTracks[activeTrack]->currentState.playState!=PS_STOPPED){
      pCurrentSequence->arTracks[activeTrack]->currentState.playState=PS_STOPPED;
    }
  }
}

void pauseSeq(){
  sTrack_t *pTrack=0;
  //printf("Pause/Resume.\n");
  if(pCurrentSequence!=0){
    // TODO: handling individual tracks for MIDI 2 type
    // for one sequence(single/multichannel) we will check state of the first track only
    U8 activeTrack=pCurrentSequence->ubActiveTrack;
    pTrack=pCurrentSequence->arTracks[activeTrack];
    
    switch(pTrack->currentState.playState){
      case PS_PLAYING:{
	pTrack->currentState.playState=PS_PAUSED;
      }break;
      case PS_PAUSED:{
	pTrack->currentState.playState=PS_PLAYING;
      }break;
    };
  }
}

void playSeq(void){
  if(pCurrentSequence!=0){
    //set state
    U8 activeTrack=pCurrentSequence->ubActiveTrack;
    if(pCurrentSequence->arTracks[activeTrack]->currentState.playState==PS_STOPPED)
      pCurrentSequence->arTracks[activeTrack]->currentState.playState=PS_PLAYING;
  }
}

void muteTrack(U16 trackNb,BOOL bMute){
  if(((pCurrentSequence!=0)&&(trackNb<AMIDI_MAX_TRACKS))){
    pCurrentSequence->arTracks[trackNb]->currentState.bMute=bMute;
  }
}

void toggleReplayMode(void){
  
  if(pCurrentSequence!=0){
    U8 activeTrack=pCurrentSequence->ubActiveTrack;
 
    switch(pCurrentSequence->arTracks[activeTrack]->currentState.playMode){
      case S_PLAY_ONCE: {
	pCurrentSequence->arTracks[activeTrack]->currentState.playMode=S_PLAY_LOOP;
      }break;
      case S_PLAY_LOOP: {
	pCurrentSequence->arTracks[activeTrack]->currentState.playMode=S_PLAY_ONCE;
      }break;
    }
  }
}

void getCurrentSeq (sSequence_t **pPtr){
  (*pPtr)=pCurrentSequence;
}


