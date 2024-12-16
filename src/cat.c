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
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utility.h"

int main (int argc, char *argv[])

{
	char ch;
	FILE *fp1, *fp2;
	int i;

	if (CheckLicense() == -1) {
		Perror (argv[0]);
		exit (1);
	}
	if ((argc != 2) && (argc !=3)) {
		/**
		printf ("ARGS = %d\n", argc);
		for (i = 0; i <= argc; i++)
			printf ("ARGV[%d] = %s\n", i, argv[i]);
		**/
		printf ("Usage: %s file1 [file2]\n", argv[0]);
		exit (1);
	}

	if (argv[1][0] == '-') {
		while ((ch = getche ()) != 26) {
			if (ch == 13)
				printf ("\n\r");
		}
	} else {
		for (i=0; i < strlen(argv[1]); i++)
			if (argv[1][i] == '/')
				argv[1][i] = '\\';

		if ((fp1 = fopen (argv[1], "r")) == NULL) {
			perror (argv[1]);
		} else {
			while (!(feof (fp1))) {
				ch = getc (fp1);
				printf ("%c", ch);
			}
			fclose (fp1);
		}
	}

	if (argc != 3)
		return (0);

	if (argv[2][0] == '-') {
		while ((ch = getche ()) != 26) {
			if (ch == '\n')
				printf ("\n\r");
		}
	} else {
		for (i=0; i < strlen(argv[2]); i++)
			if (argv[2][i] == '/')
				argv[2][i] = '\\';

		if ((fp2 = fopen (argv[2], "r")) == NULL) {
			perror (argv[2]);
		} else {
			while (!(feof (fp2))) {
				ch = getc (fp2);
				printf ("%c", ch);
			}
		}
		fclose (fp2);
	}
	return (0);
}
