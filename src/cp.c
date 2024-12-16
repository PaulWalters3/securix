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
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "utility.h"

#define BUFSIZE 1024
#define MAXCP 69


int main (int argc, char *argv[])

{
	FILE *source, *destination;
	char dest[MAXPATH], src[MAXCP][MAXPATH], read_from[MAXPATH];
	char buffer[BUFSIZE], curdir[MAXPATH];
	char drive[MAXDRIVE], dir[MAXDIR], fname[MAXFILE], ext[MAXEXT];
	int i, n, idx=0, done;
	struct ffblk src_ff;
	int ask=0, sil=0, opt=0;
	int attrib;

	if (CheckLicense() == -1) {
		printf ("%s\n", BADLICENSE);
		exit (1);
	}

	if (argc < 3) {
		printf ("Usage: %s [-[v][s]] source destination\n", argv[0]);
		exit(1);
	}

	if (argv[1][0] == '-') {
		opt = 1;
		if (argc != 4) {
			printf ("Usage: %s [-[v][s]] source destination\n", argv[0]);
			exit(1);
		}
		if (strchr (argv[1],'v') != NULL)
			ask = 1;
		if (strchr (argv[1],'s') != NULL)
			sil = 1;
	}
	/** Convert Slashes in filenames **/
	if (!opt) {
		for (i=0; i<strlen(argv[1]); i++)
			if (argv[1][i] == '/')
				argv[1][i] = '\\';
	} else {
		for (i=0; i<strlen(argv[3]); i++)
			if (argv[3][i] == '/')
				argv[3][i] = '\\';
	}

	for (i=0; i<strlen(argv[2]); i++)
		if (argv[2][i] == '/')
			argv[2][i] = '\\';

	if (!opt) {
		done = findfirst (argv[1], &src_ff, 0);
		fnsplit(argv[1], drive, dir, fname, ext);
	} else {
		done = findfirst (argv[2], &src_ff, 0);
		fnsplit(argv[2], drive, dir, fname, ext);
	}

	if (done == -1) {
		if (!opt)
			perror (argv[1]);
		else
			perror (argv[2]);
		exit (1);
	}

	while (!done) {
		strcpy (src[idx++], src_ff.ff_name);
		done = findnext (&src_ff);
	}

	for (i=0; i < idx; i++) {
		if (!opt)
			strcpy (dest, argv[2]);
		else
			strcpy (dest, argv[3]);

		getcwd (curdir, MAXPATH);
		if (!chdir (dest)) {
			chdir (curdir);
			if (dest[strlen(dest)-1] == '\\')
				strcat (dest, src[i]);
			else {
				strcat (dest, "\\");
				strcat (dest, src[i]);
			}
		}
		else if (dest[strlen(dest)-1] == ':') {
			strcat (dest,src[i]);
		} else if (idx > 1) {
			printf ("Destination must be a directory.\n");
			exit (1);
		}

		sprintf (read_from, "%s%s%s", drive, dir, src[i]);
		if (!strcmp(src[i],dest) || !strcmp(src[i],".")) {
			fprintf (stderr, "Filenames are identical.\n");
			exit (1);
		}
		if (ask) {
			printf ("%s -> %s (Y/N)?\n", strupr(read_from), strupr(dest));
			if (toupper(getch()) != 'Y')
				continue;
		} else if (!sil)
			printf("%s -> %s\n", strupr(read_from), strupr(dest));

		if ((source = fopen(read_from,"rb")) == NULL) {
			perror(src[i]);
			exit(1);
		}
		if ((destination = fopen(dest,"wb")) == NULL) {
			perror(dest);
			exit(1);
		}

		while (!feof(source)) {
			n = fread ((char *) buffer, sizeof(char), BUFSIZE, source);
			if (n == -1) {
				fclose(source);
				fclose(destination);
				unlink(dest);
				perror("Read Error:");
				exit(1);
			}
			if (fwrite((char *) buffer, sizeof(char), n, destination) != n) {
				fclose(source);
				fclose(destination);
				unlink(dest);
				perror("Write Error:");
				exit(1);
			}
		}
		fclose(source);
		fclose(destination);
		attrib = _chmod(read_from, 0);
		if (attrib & FA_RDONLY)
			chmod (dest, S_IREAD);
	}
	return(0);
}
