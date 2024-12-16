/*
 * Copyright 1992-2024 Paul Walters
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <bios.h>
#include <conio.h>
#include <dos.h>
#include <errno.h>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "utility.h"

#define ATTR 0x7900

void interrupt handler(void);
void interrupt ( *oldhandler)(void);

char far *InDos;
char far *CriticalError;

int main (void)
{
	struct SREGS segs;
	union REGS inregs, outregs;
	unsigned segment, offset;

	if (CheckLicense() == -1) {
		Perror ("CLOCK");
		exit (1);
	}

	if (prog_installed ("CLOCK")) {
		Perror ("CLOCK");
		exit (1);
	}

	printf ("Clock Installed.\n");
	printf ("%s\n", COPYRIGHT);

	/* Get InDos flag */
	inregs.h.ah = 0x34;
	intdosx (&inregs, &outregs, &segs);
	InDos = (char far *) (((long) (segs.es) << 16) + outregs.x.bx);
	segment = segs.es;
	offset = outregs.x.bx;

	/* Get DOS version and CriticalError */
	inregs.x.ax = 0x3001;
	intdos (&inregs, &outregs);
	if (outregs.h.al < 3)
		++offset;
	else
		--offset;
	CriticalError = (char far *) (((long) (segment) << 16) + offset);

	oldhandler = getvect (0x1c);
	setvect (0x1c, handler);
/*
	keep (0, (_SS+(_SP/16)-_psp));
*/
	keep(0, get_prog_size("CLOCK")+(unsigned) 32);
	return (0);
}


void interrupt handler (void)
{
	static int pm = 0;
	static int hour, min;
	static float hours, mins;
	static long t;
	static unsigned int (far *screen)[80];
	static struct text_info *scr_mode;

	if ((*InDos != 1) || (*CriticalError != 0))
		return;

	gettextinfo (scr_mode);
	if (scr_mode->currmode == C80)
		screen = MK_FP (0xB800,0);
	else
		screen = MK_FP(0xB000,0);
	t= biostime(0,0L);
	hours = t / CLK_TCK / 3600;
	mins = hours - (int) hours;
	mins = (mins * 100) * 60 / 100;
	hour = hours;
	min = mins;

	if (hour > 12) {
		pm = 1;
		hour -= 12;
	}

	if (hour > 9) {
		screen[0][74] = 1 + '0' + ATTR;
		screen[0][75] = (hour-10) + '0' + ATTR;
	} else {
		screen[0][74] = 0 + '0' + ATTR;
		screen[0][75] = hour + '0' + ATTR;
	}
	screen[0][76] = ':' + ATTR;
	if (min > 49) {
		screen[0][77] = 5 + '0' + ATTR;
		screen[0][78] = (min-50) + '0' + ATTR;
	} else if (min > 39) {
		screen[0][77] = 4 + '0' + ATTR;
		screen[0][78] = (min-40) + '0' + ATTR;
	} else if (min > 29) {
		screen[0][77] = 3 + '0' + ATTR;
		screen[0][78] = (min-30) + '0' + ATTR;
	} else if (min > 19) {
		screen[0][77] = 2 + '0' + ATTR;
		screen[0][78] = (min-20) + '0' + ATTR;
	} else if (min > 9) {
		screen[0][77] = 1 + '0' + ATTR;
		screen[0][78] = (min-10) + '0' + ATTR;
	} else {
		screen[0][77] = 0 + '0' + ATTR;
		screen[0][78] = min + '0' + ATTR;
	}
	if (pm)
		screen[0][79] = 'p' + ATTR;
	else
		screen[0][79] = 'a' + ATTR;
	oldhandler ();
}
