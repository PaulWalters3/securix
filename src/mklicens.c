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

#include <errno.h>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include "utility.h"

int main (int argc, char *argv[])

{
	int i, j, k, num, last=0;
	char fn[MAXPATH];
	char serial[MAXLICENSE];
	char *key;
	FILE *fp;

	if (argc != 2) {
		printf ("Usage: %s num_to_generate\n", argv[0]);
		exit (1);
	}

	num = atoi (argv[1]);
	randomize();
	for (j = 0; j < num; ++j) {
		for (i = 0; i < MAXLICENSE; ++i) {
			while (1) {
				serial[i] = random(122);
				if (serial[i] > 47 && serial[i] < 58)
					break;
				if (serial[i] > 64 && serial[i] < 91)
					break;
				if (serial[i] > 96)
					break;
			}
		}
		serial[MAXLICENSE] = '\0';
		key = encode_license (serial);

		k = last;
		while (1) {
			++k;
			sprintf (fn, "lic%04d", k);
			if (access (fn, 0))
				break;
		}

		if ((fp = fopen(fn, "w")) == NULL) {
			perror (fn);
			exit (1);
		}

		fprintf (fp, "%s\n", PRODUCTNAME);
		fprintf (fp, "%s\n", COPYRIGHT);
		fprintf (fp, "-------------------------------------------------\n");
		fprintf (fp, "This is your product serial number and license\n");
		fprintf (fp, "key.  If you have not properly licensed this\n");
		fprintf (fp, "product, see the enclosed license agreement on\n");
		fprintf (fp, "how to do so.\n\n\n");
		fprintf (fp, "     SERIAL NUMBER : %s\n\n", serial);
		fprintf (fp, "     %s ACTIVATION KEY   : %s\n\n", PRODUCTNAME, key);
		fclose (fp);

		if ((fp = fopen("used", "a")) == NULL) {
			printf ("SERIAL NUMBER : %s\nKEY : %s\n", serial, key);
		} else {
			fprintf (fp, "SERIAL NUMBER : %s\nKEY : %s\n", serial, key);
			fclose (fp);
		}
	}
	return (0);
}
