
/**  Copyright 2007-2020 Pawel Goralski
    
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <stdio.h>

//#define MANUAL_STEP 1

#include "amidilib.h"
#include "amidiseq.h"       // sequence structs
#include "fmio.h"           // disc i/o
#include "timing/mfp.h"
#include "timing/miditim.h"

// nkt conversion
#include "nkt.h"
#include "seq2nkt.h"

#ifdef MIDI_PARSER_TEST
#include "containers/list.h"
#endif

#include "input/ikbd.h"

#ifdef MANUAL_STEP
extern void updateStepSingle(void);
extern void updateStepMulti(void);
#endif

// display info screen
void printInfoScreen(void); 
void displayTuneInfo(void);
void mainLoop(sSequence_t *pSequence, const char *pFileName);

#ifdef MIDI_PARSER_TEST
void midiParserTest(sSequence_t *pSequence);
#endif

/**
 * main program entry
 */

int main(int argc, char *argv[])
{

    if( ((argc>=1) && strlen(argv[1])!=0)){
      printf("Trying to load %s\n",argv[1]);
    }else{
      printf("No specified midi filename! exiting.\n");
      return 0;
    }

    uint16 iRet = 0;
    retVal iError = AM_OK;
    
    /* init library */
    iError = amInit();
    
    if(iError != AM_OK) return -1;

    // load midi file into memory 
    uint32 ulFileLenght = 0L;
    void *pMidiData = loadFile((uint8 *)argv[1], PREFER_TT, &ulFileLenght);

    if(pMidiData!=NULL)
    {
     printf("Midi file loaded, size: %u bytes.\n",(unsigned int)ulFileLenght);
     
     /* process MIDI*/
      printf("Please wait...\n");

      sSequence_t *pMusicSeq=0; //here we store our sequence data
      iError = amProcessMidiFileData(argv[1], pMidiData, ulFileLenght, &pMusicSeq);

      /* free up buffer with loaded midi file, we don't need it anymore */
      amFree(pMidiData,0);

      if(iError == AM_OK)
      {

	     printf("Sequence name: %s\n",pMusicSeq->pSequenceName);
	     printf("Nb of tracks: %d\n",pMusicSeq->ubNumTracks);
       printf("PPQN: %u\n",pMusicSeq->timeDivision);
	  
	     #ifdef MIDI_PARSER_TEST
        //output loaded midi file to screen/log
        midiParserTest(pMusicSeq);
	     #endif

	     printInfoScreen();    
       mainLoop(pMusicSeq,argv[1]);
	  
	     //unload sequence
	     destroyAmSequence(&pMusicSeq);
	  
      } else {

        amTrace((const uint8*)"Error while parsing. Exiting... \n");
    
        //unload sequence
        destroyAmSequence(&pMusicSeq);
        amDeinit(); //deinit our stuff
        return -1;

      }
     
    } else { /* MIDI loading failed */
      amTrace((const uint8*)"Error: Couldn't read %s file...\n",argv[1]);
      printf( "Error: Couldn't read %s file...\n",argv[1]);
      amDeinit();	//deinit our stuff
      return -1;
    }

 deinstallReplayRout();   
 amDeinit();
 return 0;
}


void mainLoop(sSequence_t *pSequence, const char *pFileName)
{
      //install replay rout
#ifdef MANUAL_STEP
    initAmSequenceManual(pSequence);
#else
    initAmSequence(pSequence,MFP_TiC);
#endif

	  amMemSet(Ikbd_keyboard, KEY_UNDEFINED, sizeof(Ikbd_keyboard));
	  Ikbd_mousex = Ikbd_mousey = Ikbd_mouseb = Ikbd_joystick = 0;
	   
	  /* Install our asm ikbd handler */
	  Supexec(IkbdInstall);
	  Bool bQuit=FALSE;
	  
	  //####
      while(bQuit!=TRUE)
      {

	    //check keyboard input  
	    for (uint16 i=0; i<128; ++i) {
	    
	      if (Ikbd_keyboard[i]==KEY_PRESSED) {
	      Ikbd_keyboard[i]=KEY_UNDEFINED;
	      
	      switch(i){
		case SC_ESC:{
		  bQuit=TRUE;
		  //stop sequence
		  stopAmSequence();
		}break;
		case SC_P:{
		  //starts playing sequence if is stopped
		  playAmSequence();
		 }break;
		case SC_R:{
		  //pauses sequence when is playing
		   pauseAmSequence();
		 }break;
		 case SC_M:{
		   //toggles between play once and play in loop
		  toggleAmReplayMode();
		 }break;
		case SC_I:{
		   //displays current track info
		  displayTuneInfo();
		 }break;

        case SC_D:{
          //dumps loaded sequence to NKT file
          {
            printf("Convert sequence to NKT format...\n");
            char tempName[128]={0};
            char *pTempPtr=0;
            sNktSeq *pNktSeq=0;

            //set midi output name
             strncpy(tempName,pFileName,128);
             pTempPtr=strrchr(tempName,'.');
             memcpy(pTempPtr+1,"NKT",4);

            if(Seq2NktFile(pSequence, tempName, FALSE)<0){
               printf("Error during NKT format conversion..\n");
            }else{
                printf("File saved: %s.\n",tempName);
            }

          }
        } break;
		case SC_H:{
		   //displays help/startup screen
		  printInfoScreen();
		 }break;

		 case SC_SPACEBAR:{
		  stopAmSequence();
		 }break;
#ifdef MANUAL_STEP
          case SC_ENTER:{

            for(int i=0;i<SEQUENCER_UPDATE_HZ;++i){

                if(pSequence->seqType==ST_SINGLE){
                    updateStepSingle();
                }
                else if(pSequence->seqType==ST_MULTI){
                    updateStepMulti();
                }else if(pSequence->seqType==ST_MULTI_SUB)
                {
                   AssertMsg(0,!!! ST_MULTI_SUB update TODO...\n);
                }
            }

            printAmSequenceState();

            // clear buffer after each update step
            MIDIbytesToSend=0;
            amMemSet(MIDIsendBuffer,0,32*1024);

          }break;
#endif
	      };
	      //end of switch
	    }
	    
	    if (Ikbd_keyboard[i]==KEY_RELEASED) {
	      Ikbd_keyboard[i]=KEY_UNDEFINED;
	    }
	      
	   } //end of for 	  
	   
	
	  }
	/* Uninstall our ikbd handler */
	Supexec(IkbdUninstall);
}


void printInfoScreen(void){
  
  const sAMIDI_version *pInfo=amGetVersionInfo();
  
  printf("\n=========================================\n");
  printf(LIB_NAME);
  printf("v.%d.%d.%d\t",pInfo->major,pInfo->minor,pInfo->patch);
  printf("date: %s %s\n",__DATE__,__TIME__);
  
  printf("    [i] - display tune info\n");
  printf("    [p] - play loaded tune\n");
  printf("    [r] - pause/unpause played sequence \n");
  printf("    [m] - toggle play once / loop mode \n");
  printf("    [d] - Convert sequence to NKT format.\n");
  printf("    [h] - show this help screen \n");
  printf("\n    [spacebar] - stop sequence replay \n");
  printf("    [Esc] - quit\n");
  printf(AMIDI_INFO);
  printf("==========================================\n");
  printf("Ready...\n");
} 

void displayTuneInfo(void)
{
  const sSequence_t *pPtr = getActiveAmSequence();
  
  const uint32 tempo = pPtr->arTracks[0]->currentState.currentTempo;
  const uint16 td = pPtr->timeDivision;
  const uint16 numTrk = pPtr->ubNumTracks;
  
  printf("Sequence name %s\n",pPtr->pSequenceName);
  printf("PPQN/TD: %u\t",td);
  printf("Tempo: %u [ms]\n",tempo);
  
  printf("Number of tracks: %d\n",numTrk);
  
  for(uint16 i=0;i<numTrk;++i)
  {
    uint8 *pTrkName = pPtr->arTracks[i]->pTrackName;
    printf("[Track no. %d] %s\n",(i+1),pTrkName);
  }
  
  printf("\nReady...\n");
}


#ifdef MIDI_PARSER_TEST
void midiParserTest(sSequence_t *pSequence){
   amTrace((const uint8*)"Parsed MIDI read test\n");
   amTrace((const uint8*)"Sequence name: %s\n",pSequence->pSequenceName);
   amTrace((const uint8*)"Nb of tracks: %d\n",pSequence->ubNumTracks);
   amTrace((const uint8*)"Td/PPQN: %u\n",pSequence->timeDivision);
   amTrace((const uint8*)"Active track: %d\n",pSequence->ubActiveTrack);
	  
	  //output data loaded in each track
  for (uint16 i=0;i<pSequence->ubNumTracks;++i){
    sTrack_t *p=pSequence->arTracks[i];
    
    if(p!=0){
	amTrace((const uint8*)"Track #[%d] \t",i+1);
	amTrace((const uint8*)"Track name: %s\n",p->pTrackName);
	amTrace((const uint8*)"Track ptr %p\n",p->pTrkEventList);
	
	//print out all events
	if(p->pTrkEventList!=0){
	  sEventList *pEventList=p->pTrkEventList;
	
	  while(pEventList!=0){
	    printEventBlock(&(pEventList->eventBlock));
	    pEventList=pEventList->pNext;
	  }
	}
    }
  }
}
#endif

