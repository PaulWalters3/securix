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
#include <dir.h>
#include <dos.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utility.h"

int main (int argc, char *argv[])

{
	int i, j;
	struct SREGS segs;
	union REGS inregs, outregs;

	if (CheckLicense() == -1) {
		Perror (argv[0]);
		exit (1);
	}

	if (argc == 1) {
		printf ("Usage: %s [-a][file...]\n", argv[0]);
		exit (1);
	}

	if (!strcmp(argv[1], "-a")) {
		inregs.x.ax = 0x0103;
		int86x (0x2F, &inregs, &outregs, &segs);
		if (outregs.x.cflag) {
			printf ("Cannot remove all jobs from print queue.\n");
			exit (1);
		}
		printf ("Queue empty.\n");
		exit (0);
	}

	for (i=1; i<argc; ++i) {
		for (j=0; j<strlen(argv[i]); ++j) {
			if (argv[i][j] == '/')
				argv[i][j] = '\\';
		}
		segread (&segs);
		inregs.x.ax = 0x0102;
		inregs.x.dx = (unsigned) argv[i];
		int86x (0x2F, &inregs, &outregs, &segs);
		if (outregs.x.cflag) {
			errno = outregs.x.cflag;
			perror (argv[i]);
		}
	}
	return (0);
}
