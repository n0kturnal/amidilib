#include <stdio.h>
#include <ctype.h> 
#include <osbind.h>
#include "ym2149.h"
#include "c_vars.h"
 
 void hMidiEvent(){
//dummy 
}

extern void turnOffKeyclick(void);

int main(void) {
  
  turnOffKeyclick();
  
  printf("===============================================\n");
  printf("/|\\ ym2149 sound output test..\n");
  printf("'[' or ']' - change octave -/+ \n");
  printf("[q-h] - play note\n");
  printf("',' or '.' - change envelope -/+ \n");
  printf("';' or ''' - change noise generator period -/+ \n");
  printf("[spacebar] - turn off all sounds \n");
  printf("[Esc] - quit\n");
  printf("================================================\n");
  
  U8 noteBaseArray[]={24,36,48,60,72,84,96,108};
  U8 envelopeArray[]={ENV_1,ENV_2,ENV_3,ENV_4,ENV_5,ENV_6,ENV_7,ENV_8};
  
  BOOL bQuitFlag=FALSE;
  U16 input=0L;
  U8 idx=0;
  
  U8 currentBaseIdx=0;
  U8 currentEnvIdx=0;
  U8 noisegenPeriod=15;
  
  U8 noteBase=noteBaseArray[currentBaseIdx];
  U8 currentEnvelope=envelopeArray[currentEnvIdx];
  
  while(bQuitFlag!=TRUE){
    
    input=(U16)Crawcin();
    
    if(input!=0){
    
      switch(toupper((U8)input)){
    
	case 0x1b:{
	  bQuitFlag=TRUE;
	}
	break;
	case ' ':{
	  ymSoundOff();
	}
	break;
	
	case '[':{
	  if(currentBaseIdx!=0){
	    currentBaseIdx--;
	    printf("Changed octave to: %d\n",currentBaseIdx+1);
	    idx=idx-12;
	    U8 hByte=g_arMIDI2ym2149Tone[idx].highbyte;
	    U8 lByte=g_arMIDI2ym2149Tone[idx].lowbyte;
	    U8 envelope=envelopeArray[currentEnvIdx];
	    U16 period=g_arMIDI2ym2149Tone[idx].period;
	  
	    ymDoSound(hByte,lByte,envelope,15,period,noisegenPeriod);
	  }
	}break;
	case ']':{
	if(currentBaseIdx!=7){
	    currentBaseIdx++;
	    printf("Changed octave to: %d\n",currentBaseIdx+1);
	    idx=idx+12;
	    U8 hByte=g_arMIDI2ym2149Tone[idx].highbyte;
	  U8 lByte=g_arMIDI2ym2149Tone[idx].lowbyte;
	  U8 envelope=envelopeArray[currentEnvIdx];
	  U16 period=g_arMIDI2ym2149Tone[idx].period;
	  
	  ymDoSound(hByte,lByte,envelope,15,period,noisegenPeriod);
	  }
	}break;
	case ',':{
	  if(currentEnvIdx!=0){
	    currentEnvIdx--;
	    printf("Changed envelope to: %d\n",currentEnvIdx);
	    U8 hByte=g_arMIDI2ym2149Tone[idx].highbyte;
	  U8 lByte=g_arMIDI2ym2149Tone[idx].lowbyte;
	  U8 envelope=envelopeArray[currentEnvIdx];
	  U16 period=g_arMIDI2ym2149Tone[idx].period;
	  
	  ymDoSound(hByte,lByte,envelope,15,period,noisegenPeriod);
	  }
	
	}break;
	case '.':{
	if(currentEnvIdx!=7){
	    currentEnvIdx++;
	    printf("Changed envelope to: %d\n",currentEnvIdx);
	  U8 hByte=g_arMIDI2ym2149Tone[idx].highbyte;
	  U8 lByte=g_arMIDI2ym2149Tone[idx].lowbyte;
	  U8 envelope=envelopeArray[currentEnvIdx];
	  U16 period=g_arMIDI2ym2149Tone[idx].period;
	  
	  ymDoSound(hByte,lByte,envelope,15,period,noisegenPeriod);
	  }
	}break;
	case ';':{
	 if(noisegenPeriod!=0){
	  noisegenPeriod--;
	  printf("Changed noise generator period to: %d\n",noisegenPeriod);
	  U8 hByte=g_arMIDI2ym2149Tone[idx].highbyte;
	  U8 lByte=g_arMIDI2ym2149Tone[idx].lowbyte;
	  U8 envelope=envelopeArray[currentEnvIdx];
	  U16 period=g_arMIDI2ym2149Tone[idx].period;
	  
	  ymDoSound(hByte,lByte,envelope,15,period,noisegenPeriod);
	 }
	}break;
	case '\'':{
	if(noisegenPeriod!=31){
	  noisegenPeriod++;
	   printf("Changed noise generator period to: %d\n",noisegenPeriod);
	   U8 hByte=g_arMIDI2ym2149Tone[idx].highbyte;
	   U8 lByte=g_arMIDI2ym2149Tone[idx].lowbyte;
	   U8 envelope=envelopeArray[currentEnvIdx];
	   U16 period=g_arMIDI2ym2149Tone[idx].period;
	  
	   ymDoSound(hByte,lByte,envelope,15,period,noisegenPeriod);
	 }
	}break;
	
	case 'Q':{
	  
	  idx=noteBaseArray[currentBaseIdx]+0;
	  U8 hByte=g_arMIDI2ym2149Tone[idx].highbyte;
	  U8 lByte=g_arMIDI2ym2149Tone[idx].lowbyte;
	  U8 envelope=envelopeArray[currentEnvIdx];
	  U16 period=g_arMIDI2ym2149Tone[idx].period;
	  
	  ymDoSound(hByte,lByte,envelope,15,period,noisegenPeriod);
	}
	break;
	
	case 'A':{
	  
	  idx=noteBaseArray[currentBaseIdx]+1;
	  U8 hByte=g_arMIDI2ym2149Tone[idx].highbyte;
	  U8 lByte=g_arMIDI2ym2149Tone[idx].lowbyte;
	  U8 envelope=envelopeArray[currentEnvIdx];
	  U16 period=g_arMIDI2ym2149Tone[idx].period;
	  
	  ymDoSound(hByte,lByte,envelope,15,period,noisegenPeriod);
	}
	break;
	
	case 'W':{
	  
	  idx=noteBaseArray[currentBaseIdx]+2;
	  U8 hByte=g_arMIDI2ym2149Tone[idx].highbyte;
	  U8 lByte=g_arMIDI2ym2149Tone[idx].lowbyte;
	  U8 envelope=envelopeArray[currentEnvIdx];
	  U16 period=g_arMIDI2ym2149Tone[idx].period;
	  
	  ymDoSound(hByte,lByte,envelope,15,period,noisegenPeriod);
	}
	break;
	
	case 'S':{
	  
	  idx=noteBaseArray[currentBaseIdx]+3;
	  U8 hByte=g_arMIDI2ym2149Tone[idx].highbyte;
	  U8 lByte=g_arMIDI2ym2149Tone[idx].lowbyte;
	  U8 envelope=envelopeArray[currentEnvIdx];
	  U16 period=g_arMIDI2ym2149Tone[idx].period;
	  
	  ymDoSound(hByte,lByte,envelope,15,period,noisegenPeriod);
	}
	break;
	
	case 'E':{
	  idx=noteBaseArray[currentBaseIdx]+4;
	  U8 hByte=g_arMIDI2ym2149Tone[idx].highbyte;
	  U8 lByte=g_arMIDI2ym2149Tone[idx].lowbyte;
	  U8 envelope=envelopeArray[currentEnvIdx];
	  U16 period=g_arMIDI2ym2149Tone[idx].period;
	  
	  ymDoSound(hByte,lByte,envelope,15,period,noisegenPeriod);
	}
	break;
	
	case 'D':{
	  idx=noteBaseArray[currentBaseIdx]+5;
	  U8 hByte=g_arMIDI2ym2149Tone[idx].highbyte;
	  U8 lByte=g_arMIDI2ym2149Tone[idx].lowbyte;
	  U8 envelope=envelopeArray[currentEnvIdx];
	  U16 period=g_arMIDI2ym2149Tone[idx].period;
	  
	  ymDoSound(hByte,lByte,envelope,15,period,noisegenPeriod);
	}
	break;  
	
	case 'R':{
	  idx=noteBaseArray[currentBaseIdx]+6;
	  U8 hByte=g_arMIDI2ym2149Tone[idx].highbyte;
	  U8 lByte=g_arMIDI2ym2149Tone[idx].lowbyte;
	  U8 envelope=envelopeArray[currentEnvIdx];
	  U16 period=g_arMIDI2ym2149Tone[idx].period;
	  
	  ymDoSound(hByte,lByte,envelope,15,period,noisegenPeriod);
	}
	break;
	
	case 'F':{
	  idx=noteBaseArray[currentBaseIdx]+7;
	  U8 hByte=g_arMIDI2ym2149Tone[idx].highbyte;
	  U8 lByte=g_arMIDI2ym2149Tone[idx].lowbyte;
	  U8 envelope=envelopeArray[currentEnvIdx];
	  U16 period=g_arMIDI2ym2149Tone[idx].period;
	  
	  ymDoSound(hByte,lByte,envelope,15,period,noisegenPeriod);
	}
	break;
	
	case 'T':{
	  idx=noteBaseArray[currentBaseIdx]+8;
	  U8 hByte=g_arMIDI2ym2149Tone[idx].highbyte;
	  U8 lByte=g_arMIDI2ym2149Tone[idx].lowbyte;
	  U8 envelope=envelopeArray[currentEnvIdx];
	  U16 period=g_arMIDI2ym2149Tone[idx].period;
	  
	  ymDoSound(hByte,lByte,envelope,15,period,noisegenPeriod);
	}
	break;  
	
	case 'G':{
	  idx=noteBaseArray[currentBaseIdx]+9;
	  U8 hByte=g_arMIDI2ym2149Tone[idx].highbyte;
	  U8 lByte=g_arMIDI2ym2149Tone[idx].lowbyte;
	  U8 envelope=envelopeArray[currentEnvIdx];
	  U16 period=g_arMIDI2ym2149Tone[idx].period;
	  
	  ymDoSound(hByte,lByte,envelope,15,period,noisegenPeriod);
	}
	break;
	
	case 'Y':{
	  idx=noteBaseArray[currentBaseIdx]+10;
	  U8 hByte=g_arMIDI2ym2149Tone[idx].highbyte;
	  U8 lByte=g_arMIDI2ym2149Tone[idx].lowbyte;
	  U8 envelope=envelopeArray[currentEnvIdx];
	  U16 period=g_arMIDI2ym2149Tone[idx].period;
	  
	  ymDoSound(hByte,lByte,envelope,15,period,noisegenPeriod);
	}
	break;
	
	case 'H':{
	  idx=noteBaseArray[currentBaseIdx]+11;
	  U8 hByte=g_arMIDI2ym2149Tone[idx].highbyte;
	  U8 lByte=g_arMIDI2ym2149Tone[idx].lowbyte;
	  U8 envelope=envelopeArray[currentEnvIdx];
	  U16 period=g_arMIDI2ym2149Tone[idx].period;
	  
	  ymDoSound(hByte,lByte,envelope,15,period,noisegenPeriod);
	}
	break;
	
    }
      
    }
  
  }
  
  //turn off sound output 
  ymSoundOff();
  printf("Program terminated...\n");

return 0;
}