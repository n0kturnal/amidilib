#ifndef MIDI2NKT_H
#define MIDI2NKT_H

#include "c_vars.h"
#include "nkt.h"

sNktSeq *Midi2Nkt(const void *pMidiData, const uint8 *pOutFileName, const Bool bCompress);

#endif // MIDI2NKT_H
