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

#include <alloc.h>
#include <bios.h>
#include <dos.h>
#include <stdio.h>
#include <stdlib.h>
#include "utility.h"

int main (void)
{
	void far (*xms)(void);
	struct SREGS segs;
	union REGS inregs, in, outregs, out;
	long ext_size;
	long conventional_size, extended_avail;
	unsigned long conventional_avail=0L;

	if (CheckLicense() == -1) {
		Perror ("fmem");
		exit (1);
	}

	/** Get total extended memory **/
	inregs.h.ah = 0x88;
	int86x (0x15, &inregs, &outregs, &segs);
	ext_size = outregs.x.ax;

	conventional_size = ((long) biosmemory()) * 1024L;

	inregs.x.ax = 0x4300;
	int86 (0x2F, &inregs, &outregs);
	if (outregs.h.al != 0x80) {
		extended_avail = 0;
	} else {
		/* Get total free extended memory */
		inregs.h.ah = 8;
		in.x.ax = 0x4310;
		int86x (0x2F, &in, &out, &segs);

		xms = (void far *) (((long) (segs.es) << 16) + out.x.bx);
		_AX = inregs.x.ax;
		_BX = inregs.x.bx;
		_CX = inregs.x.cx;
		_DX = inregs.x.dx;
		_SI = inregs.x.si;
		_DI = inregs.x.di;
		xms();
		outregs.x.bx = _BX;
		outregs.x.ax = _AX;
		outregs.x.cx = _CX;
		outregs.x.dx = _DX;
		outregs.x.si = _SI;
		outregs.x.di = _DI;
		extended_avail = (long) outregs.x.dx * 1024L;
	}

	conventional_avail = farcoreleft() + (unsigned long)get_prog_size("FMEM")*16 + (unsigned long)16;

	printf ("Memory Usage (bytes):\n");
	printf ("Conventional Memory    Available    Extended Memory    Available\n");
	printf ("================================================================\n");
	printf ("%-6ld                 %-6lu       %-8ld           %-8ld\n",
			conventional_size, conventional_avail, ext_size, extended_avail);
	return (0);

}
