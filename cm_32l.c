
/**  Copyright 2007-2010 Pawel Goralski
    e-mail: pawel.goralski@nokturnal.pl
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

/* Roland CM-32L, CM-64 static tables */
#include	"include/c_vars.h"
#include	"include/amidilib.h"

/*instruments can be assigned to 1-8 part (channel 2-9, can be remmaped to channel 1-8)*/
extern const U8 *g_arCM32Linstruments[];


/* drumset like in MT-32, but with additional sfx *only* for part 10 (channel 11)*/
/* the mapping is key note -> sound, program number 128 */
extern const U8 *g_arCM32Lrhythm[];


/* MT32 reset to factory defaults SysEx message */
static U8 const g_arReset[]={0xf0,0x41,0x10,0x16,0x12,0x7f,0x01,0xf7};

/* MT32toGM bank patches */
/*give an include ?*/

static const U8 g_arMT32toGM[]={
0x00
};

static const U8 g_arMT32emptyBank[]={
0x00

};


/* module default settings table */

const U8 *getCM32LInstrName(U8 ubInstrNb)
{

 return(g_arCM32Linstruments[ubInstrNb]);
}

const U8 *getCM32LRhythmName(U8 ubNoteNb)
{

 return(g_arCM32Lrhythm[ubNoteNb]);
}

void MT32Reset(void)
{
	MIDI_SEND_DATA(8,g_arReset);
}

void patchMT32toGM(void)
{

}

