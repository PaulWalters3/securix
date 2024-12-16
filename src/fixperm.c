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
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "utility.h"

int main (int argc, char *argv[])
{
	char drive[MAXDRIVE], dirdir[MAXDIR], name[MAXFILE], ext[MAXEXT];
	char dest[MAXPATH], file[MAXPATH], instdrive[2], curdir[MAXPATH];
	int mode, sil = 0, done, offset, curdrive;

	FILE *fp;
	struct ffblk f;

	if (RootLogin (0) == -1)
		exit (1);

	if (argc > 2) {
		printf ("Usage: %s [-s]\n", argv[0]);
		exit (1);
	}
	if (argc == 2 && strcmp(argv[1], "-s")) {
		printf ("Usage: %s [-s]\n", argv[0]);
		exit (1);
	}
	if (argc == 2)
		sil = 1;

	if (!sil)
		printf ("File Permission Check Utility\n%s\n", COPYRIGHT);

	if ((fp = fopen(mkFileName(FIXPERM), "r")) == NULL) {
		perror (mkFileName(FIXPERM));
		exit (1);
	}

	curdrive = getdisk();
	strcpy (instdrive, mkFileName(""));
	setdisk (toupper(instdrive[0])-'A');
	getcwd (curdir, MAXPATH);

	while (!feof(fp)) {
		fscanf (fp, "%s %d", file, &mode);

		fnsplit (file, drive, dirdir, name, ext);
		sprintf (dest, "%s%s", drive, dirdir);
		if (strlen(dest) > 1) {
			dest[strlen(dest)-1] = '\0';
			offset = 1;
		} else {
			offset = 0;
		}
		if (strcmp (dest, "")) {
			chdir (dest);
			strcpy (file, &file[strlen(dest)+offset]);
		}
		done = findfirst (file, &f, FA_DIREC);
		while (!done) {
			/* write only */
			if (f.ff_attrib & FA_DIREC) {
				done = findnext(&f);
				continue;
			}
			if (mode == S_IWRITE) {
				if (access (mkFileName(f.ff_name), 2)) {
					chmod (mkFileName(f.ff_name), mode);
					if (!sil)
						printf ("%s fixed.\n", f.ff_name);
				}
			}
			if (mode == S_IREAD) {
				if (!access (mkFileName(f.ff_name), 2)) {
					chmod (mkFileName(f.ff_name), mode);
					if (!sil)
						printf ("%s fixed.\n", f.ff_name);
				}
			}
			if (mode == (S_IREAD | S_IWRITE)) {
				if (access (mkFileName(f.ff_name), 6)) {
					chmod (mkFileName(f.ff_name), mode);
					if (!sil)
						printf ("%s fixed.\n", f.ff_name);
				}
			}
			done = findnext (&f);
		}
	}
	fclose (fp);
	if (!sil)
		printf ("File permissions checking complete.\n");
	chdir (curdir);
	setdisk(curdrive);
	return (0);
}
