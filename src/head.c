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
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utility.h"

int main (int argc, char *argv[])

{
	char ch, line[160];
	int cmd_ln=0, lines=10, i=0, j, strt;
	FILE *fp;

	if (CheckLicense() == -1) {
		Perror (argv[0]);
		exit (1);
	}

	if (argv[1][0] == '-') {
		lines = atoi (&argv[1][1]);
		if (argc > 2) {
			cmd_ln = 1;
			strt = 2;
		}
	} else {
		lines = 10;
		if (argc > 1) {
			cmd_ln = 1;
			strt = 1;
		}
	}

	if (!cmd_ln) {
		while ((ch = getch()) != EOF && i < lines) {
			if (ch == 26)
				exit(0);
			putc (ch, stdout);
			if (ch == '\n')
				++i;
		}
		exit (0);
	}

	for (j = strt; j < argc; ++j) {
		unix_to_dos_fn (argv[j]);
		if ((fp = fopen(argv[j], "r")) == NULL) {
			perror (argv[j]);
			continue;
		}
		i = 0;
		while ((fgets(line, 160, fp) != NULL) && i < lines) {
			printf ("%s", line);
			++i;
		}
		fclose (fp);
	}
	return (0);
}
