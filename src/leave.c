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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "utility.h"

#define ATTR 0x7900

void interrupt handler(void);
void interrupt ( *oldhandler)(void);

int hr, mn, rings;
char *message;
char far *InDos;
char far *CriticalError;

int main (int argc, char *argv[])

{
	struct SREGS segs;
	union REGS inregs, outregs;
	unsigned segment, offset;
	char line[80];
	FILE *fp;

	if (CheckLicense() == -1) {
		Perror (argv[0]);
		exit (1);
	}
	if (prog_installed ("LEAVE")) {
		Perror ("LEAVE");
		exit (1);
	}

	if (argc == 1) {
		printf ("Enter time for alarm [hhmm]: ");
		scanf ("%s", line);
		mn = atoi (&line[2]);
		line[2] = '\0';
		hr = atoi (line);
	} else if (argc == 2) {
		mn = atoi (&argv[1][2]);
		argv[1][2] = '\0';
		hr = atoi (argv[1]);
	} else {
		printf ("Usage: %s [hhmm]\n", argv[0]);
		exit(1);
	}

	if ((fp = fopen(mkFileName(LEAVE), "r")) == NULL) {
		rings = 1;
		message = malloc(20);
		sprintf (message, "TIME TO LEAVE!");
	} else {
		fgets (line, 80, fp);
		line[strlen(line)-1] = '\0';
		message = malloc(strlen(line));
		strcpy (message, line);
		fscanf (fp, "%d", &rings);
		fclose (fp);
	}

	printf ("Alarm set for %02d:%02d.\n", hr, mn);
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
	keep (0, (_SS+((_SP+64)/16)-_psp));
*/
	keep (0, get_prog_size("LEAVE")+(unsigned) 32);
	return (0);
}

void interrupt handler (void)

{
	int i, hour, min;
	float hours, mins;
	long t;
	unsigned int (far *screen)[80];
	struct text_info *scr_mode;
	union REGS inregs, outregs;

	if ((*InDos != 1) || (*CriticalError != 0))
		return;

	t= biostime(0,0L);
	hours = t / CLK_TCK / 3600;
	mins = hours - (int) hours;
	mins = (mins * 100) * 60 / 100;
	hour = hours;
	min = mins;

	if (hour == hr && min == mn) {
		gettextinfo (scr_mode);
		if (scr_mode->currmode == C80)
			screen = MK_FP (0xB800,0);
		else
			screen = MK_FP(0xB000,0);

		for (i=0; i < strlen(message); ++i)
			screen[0][i] = message[i] + ATTR;

		for (i=0; i < rings; ++i) {
			inregs.h.ah = 0x0e;
			inregs.h.al = 7;
			int86 (0x10, &inregs, &outregs);
		}
	}
	oldhandler ();
}
