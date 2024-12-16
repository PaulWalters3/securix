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

//   DOS Programming: The Complete Reference

#include <dos.h>
#include <math.h>
#include <process.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "utility.h"

int main (void)

{
	int i;
	unsigned ppid;
	unsigned far *mcb_seg;
	unsigned next_mcb_seg;
	union REGS inregs, outregs;
	struct SREGS segs;
	time_t t;

	struct MCB {
		char signature;
		unsigned owner;
		unsigned size;
		char reserved[3];
		char filename[8];
	} far *mcb;

	if (CheckLicense() == -1) {
		printf ("%s\n", BADLICENSE);
		exit (1);
	}

	t = time(NULL);
	printf ("%s", ctime(&t));
	printf ("PID     PPID     SIZE     PROCESS\n");
	printf ("==================================\n");

	inregs.h.ah = 0x52;
	intdosx (&inregs, &outregs, &segs);

	mcb_seg = (unsigned far *) (((long) (segs.es) << 16) + (outregs.x.bx - 2));
	mcb = (struct MCB far *) ((long) (*mcb_seg) << 16);

	for (;;) {
		if (mcb->owner == FP_SEG(mcb) + 1) {
			printf ("%-5u   ", mcb->owner);
			ppid = mcb->owner + 0x16;
			printf ("%-5u    ", ppid);
			printf ("%-6ld   ", (long)mcb->size*16);

			for (i=0; i<8; ++i) {

				if ((mcb->filename[i]) != NULL && (mcb->filename[i] != ' '))
					printf("%c", mcb->filename[i]);
				else
					break;
			}
			printf ("\n");
		}
		if (mcb->signature != 0x5a) {
			next_mcb_seg = FP_SEG(mcb) + mcb->size + 1;
			mcb = (struct MCB far *) ((long) (next_mcb_seg) << 16);
		} else {
			break;
		}
	}
	return (0);
}
