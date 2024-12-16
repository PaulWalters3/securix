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
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "utility.h"

int main (void)
{
	char start_tm[30];
	time_t start_secs, t;
	double secs_up, hours, mins;
	FILE *fp;

	if (CheckLicense() == -1) {
		Perror ("");
		exit (1);
	}
	if ((fp = fopen (mkFileName(SYSSTART), "r")) == NULL) {
		perror (mkFileName(SYSSTART));
		exit (1);
	}
	fgets (start_tm, 30, fp);
	fscanf (fp, "%ld", &start_secs);
	fclose (fp);
	t = time (NULL);
	secs_up = difftime (t, start_secs);
	hours = secs_up / 3600;
	mins = hours - (int) hours;
	mins = (mins * 100) * 60 / 100;
	printf ("System started on %s", start_tm);
	printf ("System up for %d hours and %d minutes.\n", (int) hours, (int) mins);
	return (0);
}
