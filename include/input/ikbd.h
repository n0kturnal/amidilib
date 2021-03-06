/*
	IKBD 6301 interrupt routine

	Copyright (C) 2002	Patrice Mandin

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public
	License along with this library; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
*/

#ifndef _IKBD_H
#define _IKBD_H

#include "scancode.h"

/* Const */

#define IKBD_JOY_UP	(1<<0)
#define IKBD_JOY_DOWN	(1<<1)
#define IKBD_JOY_LEFT	(1<<2)
#define IKBD_JOY_RIGHT	(1<<3)
#define IKBD_JOY_FIRE	(1<<7)

/* Variables */

extern uint8	Ikbd_keyboard[128];	/* Keyboard table */
extern uint16	Ikbd_mouseb;		/* Mouse on port 0, buttons */
extern int16 	Ikbd_mousex;		/* Mouse X relative motion */
extern int16 	Ikbd_mousey;		/* Mouse Y relative motion */
extern uint16	Ikbd_joystick;		/* Joystick on port 1 */
				
/* Functions */ 
extern void IkbdInstall(void);
extern void IkbdUninstall(void);
extern void turnOffKeyclick(void);

static const uint8 KEY_PRESSED = 0xff;
static const uint8 KEY_UNDEFINED = 0x80;
static const uint8 KEY_RELEASED = 0x00;

#endif /* _IKBD_H */
