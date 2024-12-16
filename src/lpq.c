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

int main (void)

{
	char far *q;
	char device[MAXFILE];
	FILE *fp;
	int i, j, k, port, status;
	struct SREGS segs;
	union REGS inregs, outregs;

	if (CheckLicense() == -1) {
		Perror ("");
		exit (1);
	}

	printf ("PRINT QUEUE\n%s\n", COPYRIGHT);
	printf ("=================================================\n");

	inregs.x.ax = 0x100;
	int86 (0x2F, &inregs, &outregs);
	if (!outregs.h.al || outregs.h.al == 1) {
		printf ("print queue currently not running.\n");
		exit (0);
	}

	inregs.x.ax = 0x0104;
	int86x (0x2F, &inregs, &outregs, &segs);
	if (outregs.x.cflag) {
		perror ("lpq");
		exit (1);
	}
	q = (char far *) MK_FP (segs.ds, outregs.x.si);

	for (j=0, k=1; q[j] != NULL; j+=64, ++k) {
		printf ("%d: ", k);
		for (i=j; i < j+64; ++i) {
			if (q[i]) {
				putchar (q[i]);
			} else {
				putchar ('\n');
				while (i < j+64)
					++i;
			}
		}
	}

	if (q[0] == NULL)
		printf ("Queue is empty.\n");

	if ((fp = fopen(mkFileName(LPDEVICE), "r")) == NULL) {
		perror (mkFileName(LPDEVICE));
		port = 0;
	} else  {
		fscanf (fp, "%s", device);
		fclose (fp);
		if (!strncmp (strupr(device), "LPT1", 4))
			port = 0;
		else if (!strncmp (strupr(device), "LPT2", 4))
			port = 1;
		else if (!strncmp (strupr(device), "LPT3", 4))
			port = 2;
		else
			port = 0;
	}
	status = biosprint (2, 0, port);
	if (status & 0x01)
		printf ("Printer time out.\n");
	if (status & 0x08)
		printf ("Output error.\n");
	if (status & 0x10)
		printf ("Printer selected.\n");
	if (status & 0x20)
		printf ("Printer out of paper.\n");
	if (status & 0x40)
		printf ("Acknowledge.\n");
	if (status & 0x80)
		printf ("Printer idle.\n");

	inregs.x.ax = 0x105;
	int86 (0x2F, &inregs, &outregs);
	return (0);
}
