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

#include <conio.h>
#include <ctype.h>
#include <dos.h>
#include <dir.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utility.h"

int main (int argc, char *argv[])

{
	FILE *fp;
	int rev=0, lines=-10, i=1, j, wait=0;
	long filepos, curpos, strtpos;
	char *ptr, line[160], fn[MAXPATH];

	if (CheckLicense() == -1) {
		Perror (argv[0]);
		exit (1);
	}

	if (argv[1][0] == '-' || argv[1][0] == '+') {
		if ((ptr = strchr(argv[1], 'r')) != NULL) {
			rev = 1;
			lines = 0;
			if (strcmp(argv[1], "-r")) {
				ptr[0] = '\0';
				lines = atoi(argv[1]);
			}
		} else if (!strcmp(argv[1], "-f")) {
			lines = -10;
			rev = 0;
			wait = 1;
		} else {
			lines = atoi(argv[1]);
		}
		if (argc < 3) {
			fp = stdin;
			rev = 0;
		} else if (argc > 3) {
			printf ("Usage: %s [[[+|-]startpoint][r]] [-f] [file]\n", argv[0]);
			exit (1);
		} else {
			unix_to_dos_fn (argv[2]);
			strcpy (fn, argv[2]);
			if ((fp = fopen(fn, "r")) == NULL) {
				perror (argv[2]);
				exit (1);
			}
		}
	} else {
		if (argc < 2) {
			fp = stdin;
			rev = 0;
		} else if (argc > 2) {
			printf ("Usage: %s [[[+|-]startpoint][r]] [-f] [file]\n", argv[0]);
			exit (1);
		} else {
			unix_to_dos_fn (argv[1]);
			strcpy (fn, argv[1]);
			if ((fp = fopen(fn, "r")) == NULL) {
				perror (argv[2]);
				exit (1);
			}
		}
	}

	if (lines > 0) {
		while ((fgets (line, 160, fp) != NULL) && i < lines)
			++i;
	} else if (lines < 0) {
		i = 0;
		while (fgets(line, 160, fp) != NULL)
			++i;
		rewind (fp);
		for (j=0; j<i+lines; ++j) {
			fgets (line, 160, fp);
		}
	}
	if (wait) {
		filepos = ftell (fp);
		while (1) {
			fseek (fp, filepos+1, SEEK_END);
			if ((!feof(fp)) && (ftell(fp) > filepos)) {
				fseek (fp, filepos, SEEK_SET);
				while (fgets (line, 160, fp) != NULL)
					printf ("%s", line);
				filepos = ftell(fp);
			}
			if (kbhit()) {
				fclose (fp);
				exit (0);
			}
			fclose (fp);
			sleep (1);
			if ((fp = fopen(fn, "r")) == NULL)
				exit (1);
		}
	}

	if (!rev) {
		while (fgets (line, 160, fp) != NULL)
			printf ("%s", line);
	} else {
		if (feof(fp))
			exit(0);
		curpos = strtpos = ftell(fp);
		fseek (fp, 0L, SEEK_END);
		filepos = ftell(fp);
		fseek (fp, strtpos, SEEK_SET);
			while (filepos > strtpos) {
			curpos = ftell(fp);
			fgets (line, 160, fp);
			while (ftell(fp) < filepos) {
				curpos = ftell(fp);
				fgets (line, 160, fp);
			}
			printf ("%s", line);
			filepos = curpos;
			fseek (fp, strtpos, SEEK_SET);
		}
	}

	if (fp != stdin)
			fclose (fp);
	return (0);
}
