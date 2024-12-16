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
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utility.h"

int main (int argc, char *argv[])
{
	FILE *fp;
	int i, j, done, first=1;
	int attr = FA_HIDDEN+FA_RDONLY+FA_SYSTEM+FA_ARCH;
	char look_for[80], buf[250];
	char filename[MAXPATH], directory[MAXDIR], drive[MAXDRIVE];
	char name[MAXFILE], extension[MAXEXT];
	struct ffblk ff;

	if (CheckLicense() == -1) {
		Perror (argv[0]);
		exit (1);
	}

	if (argc < 2) {
		printf ("Usage: %s string [file]\n", argv[0]);
		exit (1);
	}
	strcpy (look_for, argv[1]);

	if (argc == 2) {
		if ((fp = fopen(mkFileName(LOOKDEFAULT), "r")) == NULL) {
			perror (mkFileName(LOOKDEFAULT));
			exit (1);
		}
		while (fgets (buf, 250, fp) != NULL) {
			for (i=0; i<strlen(buf); ++i) {
				if (!strncmp(&buf[i], look_for, strlen(look_for))) {
					printf ("%s", buf);
					break;
				}
			}
		}
		fclose (fp);
	}
	else {
		for (i = 2; i < argc; ++i) {
			unix_to_dos_fn (argv[i]);
			fnsplit (argv[i], drive, directory, name, extension);
			done = findfirst (argv[i], &ff, attr);

			while (!done) {
				sprintf (filename, "%s%s%s", drive, directory, ff.ff_name);
				if ((fp = fopen(filename, "rb")) == NULL) {
					perror (filename);
					done = findnext (&ff);
					continue;
				}
				while (fgets (buf, 250, fp) != NULL) {
					for (j=0; j<strlen(buf); ++j) {
						if (!strncmp(&buf[j], look_for, strlen(look_for))) {
							if (first) {
								first = 0;
								printf ("(%s)\n", filename);
							}
							printf ("%s", buf);
							break;
						}
					}
				}
				fclose (fp);
				first = 1;
				done = findnext (&ff);
			}
		}
	}
	return (0);
}
