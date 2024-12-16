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
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utility.h"

int main (int argc, char *argv[])
{
	char cmd[MAXPATH], mnt_cmd[MAXPATH];
	int rm = 0;
	FILE *fp;

	if (CheckLicense() == -1) {
		Perror (argv[0]);
		exit (1);
	}

	if ((fp = fopen (mkFileName(MNT), "r")) == NULL) {
		Perror (mkFileName(MNT));
		exit (1);
	}
	fscanf (fp, "%s\n", mnt_cmd);
	fclose (fp);

	if (argc != 3) {
		printf ("Usage: %s [-d] drive [path]", argv[0]);
		exit (1);
	}

	if (!strcmp(argv[1], "-d"))
		rm = 1;
	else
		unix_to_dos_fn (argv[2]);

	unix_to_dos_fn (mnt_cmd);

	if (!rm)
		sprintf (cmd, "%s %s %s", mnt_cmd, argv[1], argv[2]);
	else
		sprintf (cmd, "%s %s /d", mnt_cmd, argv[2]);

	if (system (cmd)) {
		errno = EMNTERROR;
		Perror (argv[0]);
		exit (1);
	}

	return (0);
}
