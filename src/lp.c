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

void install_q (void);

int main (int argc, char *argv[])

{
	char device[MAXFILE];
	int b_arg = 1, i;
	struct SREGS segs;
	struct submit_packet {
		char level;
		char *offset;
		char *segment;
	} packet;
	union REGS inregs, outregs;

	if (CheckLicense() == -1) {
		Perror (argv[0]);
		exit (1);
	}

	install_q ();

	if (argc == 1)
		exit (0);

	if (argv[1][0] == '-') {
		b_arg = 2;
		if (argv[1][1] == 'd') {
			strcpy (device, &argv[1][2]);
		} else {
			printf ("Usage: %s [-d{device}] [files...]", argv[0]);
			exit (1);
		}
	}

	for (i=b_arg; i<argc; ++i) {
		packet.level = 0;
		segread (&segs);
		packet.segment = (char *) segs.ds;

		unix_to_dos_fn (argv[i]);

		packet.offset = argv[i];
		inregs.x.ax = 0x0101;
		inregs.x.dx = (unsigned) &(packet);
		int86x (0x2F, &inregs, &outregs, &segs);

		if (outregs.x.cflag) {
			errno = outregs.x.cflag;
			perror (argv[i]);
		}
	}
	return (0);
}

void install_q (void)
{
	FILE *fp;
	char cmd[MAXPATH], device[MAXFILE];
	int size;
	union REGS inregs, outregs;

	inregs.x.ax = 0x100;
	int86 (0x2F, &inregs, &outregs);
	strcpy (cmd, "print");

	if (!outregs.h.al) {
		if ((fp = fopen (mkFileName(LPDEVICE), "r")) == NULL) {
			perror (mkFileName(LPDEVICE));
		} else {
			fscanf (fp, "%s", device);
			fclose (fp);
			sprintf (&cmd[strlen(cmd)], " /d:%s", device);
		}
		if ((fp = fopen (mkFileName(LPBUFFER), "r")) == NULL) {
			perror (mkFileName(LPBUFFER));
		} else {
			fscanf (fp, "%d", &size);
			fclose (fp);
			sprintf (&cmd[strlen(cmd)], " /b:%d", size);
		}
		if ((fp = fopen (mkFileName(LPQUEUE), "r")) == NULL) {
			perror (mkFileName(LPQUEUE));
		} else {
			fscanf (fp, "%d", &size);
			fclose (fp);
			sprintf (&cmd[strlen(cmd)], " /q:%d", size);
		}
		system (cmd);
	}
}
