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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/timeb.h>
#include <time.h>
#include "utility.h"

int main (int argc, char *argv[])
{
	FILE *fp;
	char system_tz[20], out_tz[20], local_tz[20];
	time_t system_hrs=0;
	struct tm *system;
	struct timeb tb;


	if (CheckLicense() == -1) {
		Perror (argv[0]);
		exit (1);
	}

	if ((fp = fopen (TZ, "r")) != NULL) {
		fscanf (fp, "%s", system_tz);
		fclose (fp);
	}

	strcpy (local_tz, getenv ("TZ"));

	sprintf (out_tz, "TZ=%s", system_tz);
	putenv (out_tz);
	tzset ();

	system_hrs = time (NULL);
	sprintf (out_tz, "TZ=%s", local_tz);
	putenv (out_tz);
	tzset ();

	system = gmtime (&system_hrs);
	system_hrs = mktime (system);

	ftime (&tb);
	system_hrs -= (tb.timezone * 60);

	printf ("%s", ctime(&system_hrs));
	return (0);
}
