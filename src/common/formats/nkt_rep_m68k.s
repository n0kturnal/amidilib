;    Copyright 2014 Pawel Goralski
;    e-mail: pawel.goralski@nokturnal.pl
;    This file is part of AMIDILIB.
;    See license.txt for licensing information.

;    midi sequence replay from custom NKT structure

    include "timing/mfp_m68k.inc"
    include "common_m68k.inc"

        xref    update
        xref    _updateStepNkt

        xdef _NktInstallReplayRout	; initialises Nkt replay interrupt routine (single / multitrack) on selected timer
        xdef _NktDeinstallReplayRout	; removes Nkt replay routine from system

        even

replayNkt:
        movem.l   d0-7/a0-6,-(a7)	;save registers
        move.w    sr,-(a7)

        or.w        #$0700,sr           ; disable interrupts

        move.l    stopTimerPtr,a0       ;stop timer
        jsr     (a0)

        jsr 	_updateStepNkt          ; update sequence state and send events / copy events to internal send buffer

        if (IKBD_MIDI_SEND_DIRECT==1)
        echo	"[midiReplay.s] IKBD MIDI DATA SEND DIRECT ENABLED"

        move.w	#0,d1
        move.l 	#_MIDIsendBuffer,a0
        move.w	_MIDIbytesToSend,d1

        cmpi.w   #0,d1
        beq.s   .done       ;if 0 bytes do nothing
.send:
      ; slap data to d0
      move.w	(a0)+,d0	;get word
      ;clr.w	(a0)+ 	;clear it

      move.w	d0,d2		;make copy
      andi.w	#$FF00,d2
      lsr.w	#8,d2
.wait1:
      btst	#1,$fffffc04.w	;is data register empty?
      beq.s	.wait1		;no, wait!
      move.b	d2,$fffffc06.w	;write to MIDI data register
      subq.w	#1,d1
      cmpi.w	#0,d1
      beq.s	.done

      ;not done
      move.w	d0,d2
      andi.w	#$00FF,d2
.wait2:
      btst	#1,$fffffc04.w	;is data register empty?
      beq.s	.wait2		;no, wait!
      move.b	d2,$fffffc06.w	;write to MIDI data register

      subq.w	#1,d1
      cmpi.w	#0,d1
      beq.s	.done

      bra.s	.send
.done:
        move.w	#0,_MIDIbytesToSend
        else
        echo	"[midiReplay.s] IKBD MIDI DATA SEND DIRECT DISABLED"
        endif

        move.l  updateTimerPtr,a0   ;setup/update timer
        jsr   (a0)

.finish:
        bclr.b	  #0,$fffffa0f  	;clear IRQ in service bit TiB
        move.w    (a7)+,sr             ;restore sr
        movem.l   (a7)+,d0-7/a0-6	;restore registers
        rte


; installs / deinstalls MIDI replay (single/multitrack) on selected Timer Interrupt

; MFP_TiC     - installs / deinstalls replayRout on TiC
;             note: this non intrusive, steals original TiC vector and adds special jumptable
;             which launches music update routine if needed and after that runs old Timer C handler
; MFP_TiB     - standard custom vector
; MFP_TiA     - * unimplemented *

; #####################################################################################
_NktInstallReplayRout:
        movem.l	d0-d7/a0-a6,-(sp)
        move.l	$40(sp),d1  	; mode         ; MFP interrupt mode
        move.l  $44(sp),d0  	; data         ; MFP interrupt data
        move.w	$48(sp),timerReplayType       ; timer type

        move.w  #0,isMultitrackReplay
        move.w  #0,midiIntCounter

        ; setup
        move.b	d1,tMode                ;save parameters for later
        move.b	d0,tData

        move.l  #replayNkt, update
        move.l  #stopTiB, stopTimerPtr
        move.l  #updateTiB, updateTimerPtr

        bsr.w	_super_on
        move.w	sr,-(sp)                ;save status register
        or.w	#$0700,sr               ;turn off all interupts

; TODO check timerReplay type parameter

        ; MFP_TiB
;       ###############################################
        ; TODO  setup based on requested timer
        ; this for TiB
        move.l    stopTimerPtr,a0
        jsr     (a0)

        move.l	  $120,oldVector

        move.l    updateTimerPtr,a0
        jsr       (a0)
;       ##################################################

        move.w 	  (sp)+,sr 		;restore Status Register
        bsr.w	  _super_off
        movem.l (sp)+,d0-d7/a0-a6	;restore registers
        rts

_NktDeinstallReplayRout:
        movem.l	  d0-d7/a0-a6,-(sp)
        bsr.w	_super_on

        move.w	sr,-(a7)		;save status register
        or.w	#$0700,sr
; MFP_TiB
        move.l    stopTimerPtr,a0
        jsr     (a0)
        move.l	 oldVector,$120	;restore old vector

; MFP_TiC todo
        move.w	(sp)+,sr	;restore Status Register

        bsr.w	_super_off
        movem.l (sp)+,d0-d7/a0-a6
        rts
