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

#include <dos.h>
#include <stdio.h>
#include <stdlib.h>
#include "utility.h"

int main (int argc, char *argv[])

{
	int i;
	unsigned far *segment;
	unsigned next_segment;
	struct MCB {
		char signature;
		unsigned owner;
		unsigned size;
		char reserved[3];
		char filename[8];
	} far *mcb;
	union REGS inregs, outregs;
	struct SREGS segs;


	if (CheckLicense() == -1) {
		printf ("%s\n", BADLICENSE);
		exit (1);
	}

	if (argc == 1) {
		printf ("Usage: %s [pid...]\n", argv[0]);
		exit (1);
	}

	for (i=1; i<argc; ++i) {
		inregs.h.ah = 0x52;
		intdosx (&inregs, &outregs, &segs);
		segment = (unsigned far *) (((long) (segs.es) << 16) + (outregs.x.bx - 2));
		mcb = (struct MCB far *) ((long) (*segment) << 16);
		for (;;) {
			if (mcb->owner == (unsigned) atoi(argv[i])) {
				inregs.h.ah = 0x49;
				segs.es = FP_SEG(mcb) + 1;
				intdosx (&inregs, &outregs, &segs);
			}
			if (mcb->signature != 0x5A) {
				next_segment = FP_SEG(mcb) + mcb->size + 1;
				mcb = (struct MCB far *) ((long) (next_segment) << 16);
			} else {
				break;
			}
		}
	}

	return (0);
}
