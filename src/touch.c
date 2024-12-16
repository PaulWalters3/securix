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
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include "utility.h"

int main (int argc, char *argv[])

{
	char filename[MAXPATH], directory[MAXDIR], drive[MAXDRIVE];
	char name[MAXFILE], extension[MAXEXT];
	time_t t;
	struct tm *ts;
	int i, done;
	int attr = FA_DIREC+FA_HIDDEN+FA_RDONLY+FA_LABEL+FA_SYSTEM+FA_ARCH;
	unsigned year, month, day, hour, minute, seconds, fd;
	union REGS inregs, outregs;
	struct SREGS segs;
	struct ffblk ff;


	if (CheckLicense() == -1) {
		Perror (argv[0]);
		exit (1);
	}
	if (argc < 2) {
		printf ("Usage: %s file...\n", argv[0]);
		exit (1);
	}

	for (i=1; i<argc; i++) {
		unix_to_dos_fn (argv[i]);
		fnsplit (argv[i], drive, directory, name, extension);
		done = findfirst (argv[i], &ff, attr);

		while (!done) {
			sprintf (filename, "%s%s%s", drive, directory, ff.ff_name);
			if (access (filename, 0)) {
				perror (filename);
				done = findnext (&ff);
				continue;
			}

			t = time (NULL);
			ts = localtime (&t);

			/* Open File */
			inregs.h.ah = 0x3D;
			inregs.h.al = 2;
			segread (&segs);
			inregs.x.dx = (unsigned) filename;
			intdosx (&inregs, &outregs, &segs);
			if (outregs.x.cflag) {
				perror (filename);
				continue;
			} else {
				fd = outregs.x.ax;

				year = 1900 + ts->tm_year;
				month = ts->tm_mon + 1;
				day = ts->tm_mday;
				hour = ts->tm_hour;
				minute = ts->tm_min;
				seconds = ts->tm_sec / 2;

				inregs.x.ax = 0x5701;
				inregs.x.bx = fd;
				inregs.x.dx = (year - 1980) << 9;
				inregs.x.dx += month << 5;
				inregs.x.dx += day;
				inregs.x.cx = hour << 11;
				inregs.x.cx += minute << 5;
				inregs.x.cx += seconds;
				intdosx (&inregs, &outregs, &segs);
				if (outregs.x.cflag)
					perror (filename);
				inregs.h.ah = 0x3E;
				inregs.x.bx = fd;
				intdosx (&inregs, &outregs, &segs);
			}
			done = findnext (&ff);
		}
	}
	return (0);
}
