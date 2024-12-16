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
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utility.h"

int remove_dir (char dir[MAXPATH]);

int main (int argc, char *argv[])
{
	int i, recurs=0;

	if (CheckLicense () == -1) {
		Perror (argv[0]);
		exit (1);
	}
	if ((argc == 3) && (!strcmp (argv[1], "-r")))
		recurs = 1;
	else if (argc != 2) {
		printf ("Usage: %s [-r] dir\n", argv[0]);
		exit(1);
	}
	if (recurs) {
		unix_to_dos_fn (argv[2]);
		i = remove_dir (argv[2]);
		return (i);
	}

	unix_to_dos_fn (argv[1]);
	if (rmdir (argv[1])) {
		perror (argv[1]);
		return (1);
	}
	return (0);
}

int remove_dir (char dir[MAXPATH])

{
	int done;
	char olddir[MAXPATH];
	struct ffblk ff;

	getcwd (olddir, MAXPATH);
	if (chdir (dir))
		return (1);

	done = findfirst ("*.*", &ff, FA_RDONLY | FA_HIDDEN | FA_SYSTEM | FA_ARCH | FA_DIREC);
	while (!done) {
		if (ff.ff_attrib & FA_DIREC) {
			if (strcmp(ff.ff_name, ".") && strcmp(ff.ff_name, ".."))
				remove_dir (ff.ff_name);
		}
		_chmod (ff.ff_name, 1, 0);
		unlink (ff.ff_name);
		done = findnext (&ff);
	}
	if (chdir (olddir)) {
		perror (olddir);
		return (1);
	}
	if (rmdir (dir)) {
		perror (dir);
		return (1);
	}
	return (0);
}
