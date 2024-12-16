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

#include <dir.h>
#include <dos.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utility.h"

int main (int argc, char *argv[])

{
	char filename[MAXPATH], directory[MAXDIR], drive[MAXDRIVE];
	char name[MAXFILE], extension[MAXEXT];
	int i, done;
	FILE *fp;
	long sum, bytes;
	struct ffblk ff;
	int attr = FA_HIDDEN+FA_RDONLY+FA_SYSTEM+FA_ARCH;

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
			if ((fp = fopen (filename, "rb")) == NULL) {
				perror (filename);
				continue;
			}
			sum = (long) fgetc (fp);
			bytes = 0L;

			while (!feof(fp)) {
				sum += (long) fgetc (fp);
				bytes += 1L;
			}
			fclose (fp);
			if (sum == -1L)
				sum = 0L;

			/***
			make number of bytes in blocks of 512
			***/
			bytes = (bytes/512L) + ((bytes%512L) ? 1L : 0L);
			printf ("%s %ld %ld\n", filename, sum, bytes);
			done = findnext (&ff);
		}
	}
	return (0);
}
