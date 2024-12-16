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
#include <dir.h>
#include <dos.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utility.h"

int main (int argc, char *argv[])

{
  int i, j, done,ret;
  struct ffblk f;
  char r_name[MAXPATH], drive[MAXDRIVE], dir[MAXDIR], fname[MAXFILE];
  char ext[MAXEXT];
  int ver=0, sil=0, opt=0;


	if (CheckLicense() == -1) {
		Perror ("rm");
		exit (1);
	}
	if (argc < 2) {
		fprintf (stderr, "Usage: %s [-[s][v]] file ...\n", argv[0]);
		exit (1);
	}

	if (argv[1][0] == '-') {
		opt = 1;
		if (argc == 2) {
			fprintf (stderr, "Usage: %s [-[s][v]] file ... \n", argv[0]);
			exit(1);
		}
		if (strchr (argv[1],'v') != NULL)
			ver = 1;
		if (strchr (argv[1],'s') != NULL)
			sil = 1;
	}

	for (i = 1+opt; i < argc; i++) {
		for (j=0; j < strlen(argv[i]); j++)
			if (argv[i][j] == '/')
				argv[i][j] = '\\';
		done = findfirst (argv[i], &f, 0);
		if (done) {
			perror ("rm");
			exit (1);
		}
		ret = fnsplit (argv[i], drive, dir, fname, ext);
		while (!done) {
			if (ret & DIRECTORY)
				if (ret & DRIVE)
					sprintf (r_name, "%s%s%s", drive, dir, f.ff_name);
				else
					sprintf (r_name, "%s%s", dir, f.ff_name);
			else if (ret & DRIVE)
				sprintf (r_name, "%s%s", drive, f.ff_name);
			else
				strcpy (r_name, f.ff_name);
			if (ver) {
				printf("%s (Y/N)?\n", strupr(r_name));
				if (toupper(getch()) != 'Y') {
					done = findnext(&f);
					continue;
				}
			} else if (!sil) {
				printf("%s\n", strupr(r_name));
			}
			if (remove (r_name) == -1)
				perror ("rm");
			done = findnext (&f);
		}
	}
	return(0);
}
