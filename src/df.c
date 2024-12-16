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

#include <ctype.h>
#include <dos.h>
#include <stdio.h>
#include <stdlib.h>
#include "utility.h"

int main (int argc, char *argv[])
{
	struct dfree free;
	long avail, total, used;
	float perfree, perused;
	int drive;

	if (CheckLicense() == -1) {
		Perror (argv[0]);
		exit (1);
	}
	if (argc == 2)
		drive = toupper(argv[1][0]) - 64;
	else
		drive = getdisk() + 1;

	getdfree (drive, &free);
	if (free.df_sclus == 0xFFFF) {
		fprintf (stderr, "Drive not specified.\n");
		exit (1);
	} else {
		avail = (long) free.df_avail *
				(long) free.df_sclus *
				(long) free.df_bsec;
		total = (long) free.df_total *
				(long) free.df_sclus *
				(long) free.df_bsec;
		used = total - avail;
		perfree = (float) avail / (float) total;
		perused = (float) used /(float) total;
		printf ("DRIVE     USED          FREE          TOTAL         % USED     % FREE\n");
		printf ("=====================================================================\n");
		printf ("  %c       %-13ld %-13ld %-13ld %-7.2f    %-7.2f\n",
				 'A'+drive-1, used, avail, total, perused*100, perfree*100);
	}
	return (0);
}
