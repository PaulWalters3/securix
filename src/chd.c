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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utility.h"

int main (int argc, char *argv[])
{
	char *h, curdir[MAXPATH], outbuf[MAXPATH];
	int i;

	if (CheckLicense() == -1) {
		Perror (argv[0]);
		exit (1);
	}

	if (argc == 1) {
		h = getenv("HOME");
		if (chdir(h)) {
			perror (h);
			exit(1);
		}
	} else if (argc != 2) {
		fprintf (stderr, "Usage: %s dir\n", argv[0]);
		exit(1);
	} else {
		for (i=0; i<strlen(argv[1]); i++) {
			if (argv[1][i] == '/')
				argv[1][i] = '\\';
		}
		if (chdir(argv[1])) {
			perror (argv[1]);
			exit (1);
		}
	}

	getcwd (curdir, MAXPATH);
	sprintf (outbuf, "CWD=%s", curdir);
	putenv (outbuf);
	return(0);
}
