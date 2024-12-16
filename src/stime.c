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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utility.h"

int main (int argc, char *argv[])
{
	char newtime[10];
	struct time t;

	if (CheckLicense() == -1) {
		printf ("%s\n", BADLICENSE);
		exit (1);
	}

	if (argc > 2) {
		printf ("Usage: %s [hhmmss]\n", argv[0]);
		exit (1);
	}

	if (argc == 1) {
		printf ("Enter new system time [hhmmss]: ");
		gets (newtime);
		fflush (stdin);
	} else {
		strcpy (newtime, argv[1]);
	}

	if (strlen(newtime) != 6) {
		printf ("Bad conversion\n");
		printf ("Usage: %s [hhmmss]\n", argv[0]);
		exit (1);
	}

	t.ti_sec = atoi (&newtime[4]);
	newtime[4] = '\0';
	t.ti_min = atoi (&newtime[2]);
	newtime[2] = '\0';
	t.ti_hour = atoi (newtime);
	t.ti_hund = 0;

	if (t.ti_sec < 0 || t.ti_sec > 59) {
		printf ("Bad conversion\n");
		printf ("Usage: %s [hhmmss]\n", argv[0]);
		exit (1);
	}
	if (t.ti_min < 0 || t.ti_min > 59) {
		printf ("Bad conversion\n");
		printf ("Usage: %s [hhmmss]\n", argv[0]);
		exit (1);
	}
	if (t.ti_hour < 0 || t.ti_hour > 24) {
		printf ("Bad conversion\n");
		printf ("Usage: %s [hhmmss]\n", argv[0]);
		exit (1);
	}

	settime (&t);
	return(0);
}
