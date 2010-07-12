
/**  Copyright 2007-2010 Pawel Goralski
    e-mail: pawel.goralski@nokturnal.pl
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

#include "include/c_vars.h"

/* file contains all instrument tables for Roland MT-32 family devices */

const char *g_arMIDIcontrollers[]=
{
"Bank Select",
"Modulation",
"Breath Controller",
"3 [undefined]",
"Foot Controller",
"Portamento Time",
"Data Entry (MSB)",
"Main Volume",
"Balance",
"9 [undefined]",
"Pan",
"Expression",
"Effect control 1",
"Effect control 2",
"14 [undefined]",
"15 [undefined]",
"Gen Purpose Controller 1",
"Gen Purpose Controller 2",
"Gen Purpose Controller 3",
"Gen Purpose Controller 4",
"20 [undefined]",
"21 [undefined]",
"22 [undefined]",
"23 [undefined]",
"24 [undefined]",
"25 [undefined]",
"26 [undefined]",
"27 [undefined]",
"28 [undefined]",
"29 [undefined]",
"30 [undefined]",
"31 [undefined]",
/*High resolution continuous controllers (LSB)*/
"Bank Select",
"Modulation Wheel",
"Breath Controller",
"35 [undefined]",
"Foot Controller",
"Portamento Time",
"Data Entry",
"Channel Volume",
"Pan",
"41 [undefined]",
"Pan",
"Expression Controller",
"Effect Control 1",
"Effect Control 2",
"46 [undefined]",
"47 [undefined]",
"General Purpose Controller 1",
"General Purpose Controller 2",
"General Purpose Controller 3",
"General Purpose Controller 4",
"52 [undefined]",
"53 [undefined]",
"54 [undefined]",
"55 [undefined]",
"56 [undefined]",
"57 [undefined]",
"58 [undefined]",
"59 [undefined]",
"60 [undefined]",
"61 [undefined]",
"62 [undefined]",
"63 [undefined]",
"Sustain On/Off",
"Portamento On/Off",
"Sostenuto On/Off",
"Soft Pedal On/Off",
"Legato On/Off",
"Hold 2 On/Off",
"Sound Controller 1   (TG: Sound Variation;   FX: Exciter On/Off)",
"Sound Controller 2   (TG: Harmonic Content;   FX: Compressor On/Off)",
"Sound Controller 3   (TG: Release Time;   FX: Distortion On/Off)",
"Sound Controller 4   (TG: Attack Time;   FX: EQ On/Off)",
"Sound Controller 5   (TG: Brightness;   FX: Expander On/Off)",
"Sound Controller 6   (TG: Undefined;   FX: Reverb On/Off)",
"Sound Controller 7   (TG: Undefined;   FX: Delay On/Off)",
"Sound Controller 8   (TG: Undefined;   FX: Pitch Transpose On/Off)",
"Sound Controller 9   (TG: Undefined;   FX: Flange/Chorus On/Off)",
"Sound Controller 10   (TG: Undefined;   FX: Special Effects On/Off)",
"General Purpose Controller 5",
"General Purpose Controller 6",
"General Purpose Controller 7",
"General Purpose Controller 8",
"Portamento Control (PTC)   (0vvvvvvv is the source Note number)",
"85 [undefined]",
"86 [undefined]",
"87 [undefined]",
"88 [undefined]",
"89 [undefined]",
"90 [undefined]",
"Effects 1 (Ext. Effects Depth)",
"Effects 2 (Tremelo Depth)",
"Effects 3 (Chorus Depth)",
"Effects 4 (Celeste Depth)",
"Effects 5 (Phaser Depth)",
"Data Increment   (0vvvvvvv is n/a; use 0)",
"Data Decrement   (0vvvvvvv is n/a; use 0)",
"Non Registered Parameter Number (LSB)",
"Non Registered Parameter Number (MSB)",
"Registered Parameter Number (LSB)",
"Registered Parameter Number (MSB)",
"102 [undefined]",
"103 [undefined]",
"104 [undefined]",
"105 [undefined]",
"106 [undefined]",
"107 [undefined]",
"108 [undefined]",
"109 [undefined]",
"[XMIDI] Channel Lock",
"[XMIDI] Channel Lock Protect",
"[XMIDI] Voice Protect",
"[XMIDI] Timbre Protect",
"[XMIDI] Patch Bank Select",
"[XMIDI] Indirect Controller Prefix",
"[XMIDI] For Loop Controller",
"[XMIDI] Next/Break Loop Controller",
"[XMIDI] Clear Beat/Bar Count",
"[XMIDI] Callback Trigger",

/* we have specs conflict here between MIDI and XMIDI :// */

"All Sounds off/[XMIDI] Sequence Branch Index",
"Reset all controllers",
"Local Control On/Off",
"All Notes Off",
"Omni Mode Off (also causes ANO)",
"Omni Mode On (also causes ANO)",
"Mono Mode On (Poly Off; also causes ANO)",
"Poly Mode On (Mono Off; also causes ANO)"
};

/**
* instruments can be assigned to 1-8 part (channel 2-9, can be remmaped to channel 1-8)
*/
const char *g_arCM32Linstruments[]=
{
"AcouPiano 1",
"AcouPiano 2",
"AcouPiano 3",
"ElecPiano 1",
"ElecPiano 2",
"ElecPiano 3",
"ElecPiano 4",
"Honkytonk",
"Elec Org 1",
"Elec Org 2",
"Elec Org 3",
"Elec Org 4",
"Pipe Org 1",
"Pipe Org 2",
"Pipe Org 3",
"Accordion",
"Harpsi 1",
"Harpsi 2",
"Harpsi 3",
"Clavi 1",
"Clavi 2",
"Clavi 3",
"Celesta 1",
"Celesta 2",
"SynBrass 1",
"SynBrass 2",
"SynBrass 3",
"SynBrass 4",
"Syn Bass 1",
"Syn Bass 2",
"Syn Bass 3",
"Syn Bass 4",
"Fantasy",
"Harmo Pan",
"Chorale",
"Glasses",
"Sountrack",
"Atmosphere",
"Warm Bell",
"Funny Vox",
"Echo Bell",
"Ice Rain",
"Oboe 2001",
"Echo Pan",
"DoctorSolo",
"Schooldaze",
"Bellsinger",
"SquareWave",
"Str Sect 1",
"Str Sect 2",
"Str Sect 3",
"Pizzicato",
"Violin 1",
"Violin 2",
"Cello 1",
"Cello 2",
"Contrabass",
"Harp 1",
"Harp 2",
"Guitar 1",
"Guitar 2",
"Elec Gtr 1",
"Elec Gtr 2",
"Sitar",
"AcouBass 1",
"AcouBass 2",
"ElecBass 1",
"ElecBass 2",
"SlapBass 1",
"SlapBass 2",
"Fretless 1",
"Fretless 2",
"Flute 1",
"Flute 2",
"Piccolo 1",
"Piccolo 2",
"Recorder",
"Pan Pipes",
"Sax 1",
"Sax 2",
"Sax 3",
"Sax 4",
"Clarinet 1",
"Clarinet 2",
"Oboe",
"Engl Horn",
"Bassoon",
"Harmonica",
"Trumpet 1",
"Trumpet 2",
"Trombone 1",
"Trombone 2",
"Fr Horn 1",
"Fr Horn 2",
"Tuba",
"Brs Sect 1",
"Brs Sect 2",
"Vibe 1",
"Vibe 2",
"Syn Mallet",
"Windbell",
"Glock",
"Tube Bell",
"Xylophone",
"Marimba",
"Koto",
"Sho",
"Shakuhachi",
"Whistle 1",
"Whistle 2",
"Bottleblow",
"BreathPipe",
"Timpani",
"MelodicTom",
"Deep Snare",
"ElecPerc 1",
"ElecPerc 2",
"Taiko",
"Taiko Rim",
"Cymbal",
"Castanets",
"Triangle",
"Orche Hit",
"Telephone",
"Bird Tweet",
"OneNoteJam",
"WaterBells",
"JungleTune"
};

/** 
* drumset like in MT-32, but with additional sfx *only* for part 10 (channel 11)
* the mapping is key note -> sound, program number 128 
*/

const char *g_arCM32Lrhythm[]=
{
"00 [no sound]",
"01 [no sound]",
"02 [no sound]",
"03 [no sound]",
"04 [no sound]",
"05 [no sound]",
"06 [no sound]",
"07 [no sound]",
"08 [no sound]",
"09 [no sound]",
"0A [no sound]",
"0B [no sound]",
"0C [no sound]",
"0D [no sound]",
"0E [no sound]",
"0F [no sound]",
"10 [no sound]",
"11 [no sound]",
"12 [no sound]",
"13 [no sound]",
"14 [no sound]",
"15 [no sound]",
"16 [no sound]",
"17 [no sound]",
"18 [no sound]",
"19 [no sound]",
"1A [no sound]",
"1B [no sound]",
"1C [no sound]",
"1D [no sound]",
"1E [no sound]",
"1F [no sound]",
"20 [no sound]",
"21 [no sound]",
"22 [no sound]",
"Acoustic Bass Drum",
"Acoustic Bass Drum",
"Rim Shot",
"Acoustic Snare Drum",
"Hand Clap",
"Electronic Snare Drum",
"Acoustic Low Tom",
"Closed High Hat",
"Acoustic Low Tom",
"Open High Hat 2",
"Acoustic Middle Tom",
"Open High Hat 1",
"Acoustic Middle Tom",
"Acoustic High Tom",
"Crash Cymbal",
"Acoustic High Tom",
"Ride Cymbal",
"34 [no sound]",
"35 [no sound]",
"Tambourine",
"37 [no sound]",
"Cowbell",
"39 [no sound]",
"3A [no sound]",
"3B [no sound]",
"High Bongo",
"Low Bongo",
"Mute High Conga",
"High Conga",
"Low Conga",
"High Timbale",
"Low Timbale",
"High Agogo",
"Low Agogo",
"Cabasa",
"Maracas",
"Short Whistle",
"Long Whistle",
"Quijada",
"4A [no sound]",
"Claves",
"Laughing",
"Screaming",
"Punch",
"Heartbeat",
"Footsteps1",
"Footsteps2",
"Applause",
"Creaking",
"Door",
"Scratch",
"Windchime",
"Engine",
"Car-stop",
"Car-pass",
"Crash",
"Siren",
"Train",
"Jet",
"Helicopter",
"Starship",
"Pistol",
"Machinegun",
"Lasergun",
"Explosion",
"Dog",
"Horse",
"Birds",
"Rain",
"Thunder",
"Wind",
"Waves",
"Stream",
"Bubble",
"6D [no sound]",
"6E [no sound]",
"6F [no sound]",
"70 [no sound]",
"71 [no sound]",
"72 [no sound]",
"73 [no sound]",
"74 [no sound]",
"75 [no sound]",
"76 [no sound]",
"77 [no sound]",
"78 [no sound]",
"79 [no sound]",
"7A [no sound]",
"7B [no sound]",
"7C [no sound]",
"7D [no sound]",
"7E [no sound]",
"7F [no sound]"
};

const char *g_arCM500sfx[]=
{
"0 [no sound]",
"1 [no sound]",
"2 [no sound]",
"3 [no sound]",
"4 [no sound]",
"5 [no sound]",
"6 [no sound]",
"7 [no sound]",
"8 [no sound]",
"9 [no sound]",
"10 [no sound]",
"11 [no sound]",
"12 [no sound]",
"13 [no sound]",
"14 [no sound]",
"15 [no sound]",
"16 [no sound]",
"17 [no sound]",
"18 [no sound]",
"19 [no sound]",
"20 [no sound]",
"21 [no sound]",
"22 [no sound]",
"23 [no sound]",
"24 [no sound]",
"25 [no sound]",
"26 [no sound]",
"27 [no sound]",
"28 [no sound]",
"29 [no sound]",
"30 [no sound]",
"31 [no sound]",
"32 [no sound]",
"33 [no sound]",
"34 [no sound]",
"35 [no sound]",
"36 [no sound]",
"37 [no sound]",
"38 [no sound]",
"High Q",
"Slap",
"Scratch Push",
"Scrach Pull",
"Sticks",
"Square Click",
"Metronome Click",
"Metronome Bell",
"Guitar sliding finger",
"Guitar cutting noise (down)",
"Guitar cutting noise (up)",
"String slap of double bass",
"Fl. Key Click",
"Laughing",
"Screaming",
"Punch",
"Heart Beat",
"Footsteps 1",
"Footsteps 2",
"Applause",
"Door Creaking",
"Door",
"Scratch",
"Windchime",
"Car-Engine",
"Car-Stop",
"Car-Pass",
"Car-Crash",
"Siren",
"Train",
"Jetplane",
"Helicopter",
"Starship",
"Gun Shot",
"Machine Gun",
"Lasergun",
"Explosion",
"Dog",
"Horse-Gallop",
"Birds",
"Rain",
"Thunder",
"Wind",
"Seashore",
"Stream",
"Bubble",
"85 [no sound]",
"86 [no sound]",
"87 [no sound]",
"88 [no sound]",
"89 [no sound]",
"90 [no sound]",
"91 [no sound]",
"92 [no sound]",
"93 [no sound]",
"94 [no sound]",
"95 [no sound]",
"96 [no sound]",
"97 [no sound]",
"98 [no sound]",
"99 [no sound]",
"100 [no sound]",
"101 [no sound]",
"102 [no sound]",
"103 [no sound]",
"104 [no sound]",
"105 [no sound]",
"106 [no sound]",
"107 [no sound]",
"108 [no sound]",
"109 [no sound]",
"110 [no sound]",
"111 [no sound]",
"112 [no sound]",
"113 [no sound]",
"114 [no sound]",
"115 [no sound]",
"116 [no sound]",
"117 [no sound]",
"118 [no sound]",
"119 [no sound]",
"120 [no sound]",
"121 [no sound]",
"122 [no sound]",
"123 [no sound]",
"124 [no sound]",
"125 [no sound]",
"126 [no sound]",
"127 [no sound]",
"128 [no sound]"
};

const char *g_arMT32instruments[]=
{
"AcouPiano 1",
"AcouPiano 2",
"AcouPiano 3",
"ElecPiano 1",
"ElecPiano 2",
"ElecPiano 3",
"ElecPiano 4",
"Honkytonk",
"Elec Org 1",
"Elec Org 2",
"Elec Org 3",
"Elec Org 4",
"Pipe Org 1",
"Pipe Org 2",
"Pipe Org 3",
"Accordion",
"Harpsi 1",
"Harpsi 2",
"Harpsi 3",
"Clavi 1",
"Clavi 2",
"Clavi 3",
"Celesta 1",
"Celesta 2",
"SynBrass 1",
"SynBrass 2",
"SynBrass 3",
"SynBrass 4",
"Syn Bass 1",
"Syn Bass 2",
"Syn Bass 3",
"Syn Bass 4",
"Fantasy",
"Harmo Pan",
"Chorale",
"Glasses",
"Sountrack",
"Atmosphere",
"Warm Bell",
"Funny Vox",
"Echo Bell",
"Ice Rain",
"Oboe 2001",
"Echo Pan",
"DoctorSolo",
"Schooldaze",
"Bellsinger",
"SquareWave",
"Str Sect 1",
"Str Sect 2",
"Str Sect 3",
"Pizzicato",
"Violin 1",
"Violin 2",
"Cello 1",
"Cello 2",
"Contrabass",
"Harp 1",
"Harp 2",
"Guitar 1",
"Guitar 2",
"Elec Gtr 1",
"Elec Gtr 2",
"Sitar",
"AcouBass 1",
"AcouBass 2",
"ElecBass 1",
"ElecBass 2",
"SlapBass 1",
"SlapBass 2",
"Fretless 1",
"Fretless 2",
"Flute 1",
"Flute 2",
"Piccolo 1",
"Piccolo 2",
"Recorder",
"Pan Pipes",
"Sax 1",
"Sax 2",
"Sax 3",
"Sax 4",
"Clarinet 1",
"Clarinet 2",
"Oboe",
"Engl Horn",
"Bassoon",
"Harmonica",
"Trumpet 1",
"Trumpet 2",
"Trombone 1",
"Trombone 2",
"Fr Horn 1",
"Fr Horn 2",
"Tuba",
"Brs Sect 1",
"Brs Sect 2",
"Vibe 1",
"Vibe 2",
"Syn Mallet",
"Windbell",
"Glock",
"Tube Bell",
"Xylophone",
"Marimba",
"Koto",
"Sho",
"Shakuhachi",
"Whistle 1",
"Whistle 2",
"Bottleblow",
"BreathPipe",
"Timpani",
"MelodicTom",
"Deep Snare",
"ElecPerc 1",
"ElecPerc 2",
"Taiko",
"Taiko Rim",
"Cymbal",
"Castanets",
"Triangle",
"Orche Hit",
"Telephone",
"Bird Tweet",
"OneNoteJam",
"WaterBells",
"JungleTune"
};

const char *g_arMT32rhythm[]=
{
"00 [no sound]",
"01 [no sound]",
"02 [no sound]",
"03 [no sound]",
"04 [no sound]",
"05 [no sound]",
"06 [no sound]",
"07 [no sound]",
"08 [no sound]",
"09 [no sound]",
"0A [no sound]",
"0B [no sound]",
"0C [no sound]",
"0D [no sound]",
"0E [no sound]",
"0F [no sound]",
"10 [no sound]",
"11 [no sound]",
"12 [no sound]",
"13 [no sound]",
"14 [no sound]",
"15 [no sound]",
"16 [no sound]",
"17 [no sound]",
"18 [no sound]",
"19 [no sound]",
"1A [no sound]",
"1B [no sound]",
"1C [no sound]",
"1D [no sound]",
"1E [no sound]",
"1F [no sound]",
"20 [no sound]",
"21 [no sound]",
"Acou BD",
"Acou BD",
"Rim Shot",
"Acou SD",
"Hand Clap",
"Elec SD",
"Acou Low Tom",
"Clsd Hi Hat",
"Acou Low Tom",
"Open Hi Hat 2",
"Acou Mid Tom",
"Open Hi Hat 1",
"Acou Mid Tom",
"Acou Mid Tom",
"Acou Hi Tom",
"Crash Cym",
"Acou Hi Tom",
"Ride Cym",
"34 [no sound]",
"35 [no sound]",
"Tambourine",
"37 [no sound]",
"Cowbell",
"39 [no sound]",
"3A [no sound]",
"3B [no sound]",
"High Bongo",
"Low Bongo",
"Mt High Conga",
"High Conga",
"Low Conga",
"High Timbale",
"Low Timbale",
"High Agogo",
"Low Agogo",
"Cabasa",
"Maracas",
"Smba Whis S",
"Smba Whis L",
"Quijada",
"4A [no sound]",
"Claves",
"4C [no sound]",
"4D [no sound]",
"4E [no sound]",
"4F [no sound]",
"50 [no sound]",
"51 [no sound]",
"52 [no sound]",
"53 [no sound]",
"54 [no sound]",
"55 [no sound]",
"56 [no sound]",
"57 [no sound]",
"58 [no sound]",
"59 [no sound]",
"5A [no sound]",
"5B [no sound]",
"5C [no sound]",
"5D [no sound]",
"5E [no sound]",
"5F [no sound]",
"60 [no sound]",
"61 [no sound]",
"62 [no sound]",
"63 [no sound]",
"64 [no sound]",
"65 [no sound]",
"66 [no sound]",
"67 [no sound]",
"68 [no sound]",
"69 [no sound]",
"6A [no sound]",
"6B [no sound]",
"6C [no sound]",
"6D [no sound]",
"6E [no sound]",
"6F [no sound]",
"70 [no sound]",
"71 [no sound]",
"72 [no sound]",
"73 [no sound]",
"74 [no sound]",
"75 [no sound]",
"76 [no sound]",
"77 [no sound]",
"78 [no sound]",
"79 [no sound]",
"7A [no sound]",
"7B [no sound]",
"7C [no sound]",
"7D [no sound]",
"7E [no sound]",
"7F [no sound]"
};

/* MIDI notes to international note mapping */
/* notes are mapped from 21 to 108 */

const char *g_arMIDI2key[]={
/*0 */ "C-1",
/*1 */ "C-1#",
/*2 */ "D-1",
/*3 */ "D-1#",
/*4 */ "E-1",
/*5 */ "F-1",
/*6 */ "F-1#",
/*7 */ "G-1",
/*8 */ "G-1#",
/*9*/ "A-1",
/*10 */ "A-1#",
/*11 */ "B-1#",
/*12 */ "C",
/*13 */ "C#",
/*14 */ "D",
/*15 */ "D#",
/*16 */ "E",
/*17 */ "F",
/*18 */ "F#",
/*19 */ "G",
/*20 */ "G#",
/*21*/ "A0",
/*22*/ "A0#",
/*23*/ "B0",
/*24*/ "C1",
/*25*/ "C1#",
/*26*/ "D1",
/*27*/ "D1#",
/*28*/ "E1",
/*29*/ "F1",
/*30*/ "F1#",
/*31*/ "G1",
/*32*/ "G1#",
/*33*/ "A1",
/*34*/ "A1#",
/*35*/ "B1",
/*36*/ "C2",
/*37*/ "C2#",
/*38*/ "D2",
/*39*/ "D2#",
/*40*/ "E2",
/*41*/ "F2",
/*42*/ "F2#",
/*43*/ "G2",
/*44*/ "G2#",
/*45*/ "A2",
/*46*/ "A2#",
/*47*/ "B2",
/*48*/ "C3",
/*49*/ "C3#",
/*50*/ "D3",
/*51*/ "D3#",
/*52*/ "E3",
/*53*/ "F3",
/*54*/ "F3#",
/*55*/ "G3",
/*56*/ "G3#",
/*57*/ "A3",
/*58*/ "A3#",
/*59*/ "B3",
/*60*/ "C4", /*  */
/*61*/ "C4#",
/*62*/ "D4",
/*63*/ "D4#",
/*64*/ "E4",
/*65*/ "F4",
/*66*/ "F4#",
/*67*/ "G4",
/*68*/ "G4#",
/*69*/ "A4",
/*70*/ "A4#",
/*71*/ "B4",
/*72*/ "C5",
/*73*/ "C5#",
/*74*/ "D5",
/*75*/ "D5#",
/*76*/ "E5",
/*77*/ "F5",
/*78*/ "F5#",
/*79*/ "G5",
/*80*/ "G5#",
/*81*/ "A5",
/*82*/ "A5#",
/*83*/ "B5",
/*84*/ "C6",
/*85*/ "C6#",
/*86*/ "D6",
/*87*/ "D6#",
/*88*/ "E6",
/*89*/ "F6",
/*90*/ "F6#",
/*91*/ "G6",
/*92*/ "G6#",
/*93*/ "A6",
/*94*/ "A6#",
/*95*/ "B6",
/*96*/ "C7",
/*97*/ "C7#",
/*98*/ "D7",
/*99*/ "D7#",
/*100*/ "E7",
/*101*/ "F7",
/*102*/ "F7#",
/*103*/ "G7",
/*104*/ "G7#",
/*105*/ "A7",
/*106*/ "A7#",
/*107*/ "B7",
/*108*/ "C8",
/*109*/ "C8",
/*110*/ "C8",
/*111*/ "C8",
/*112*/ "C8",
/*113*/ "C8",
/*114*/ "C8",
/*115*/ "C8",
/*116*/ "C8",
/*117*/ "C8",
/*118*/ "C8",
/*119*/ "C8",
/* 120 */ "C8",
/* 121 */ "C8",
/* 122 */ "C8",
/* 123 */ "C8",
/* 124 */ "C8",
/* 125 */ "C8",
/* 126 */ "C8",
/* 127 */ "C8"
};

