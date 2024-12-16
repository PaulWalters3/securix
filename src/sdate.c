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
	char newdate[10];
	struct date d;

	if (CheckLicense() == -1) {
		printf ("%s\n", BADLICENSE);
		exit (1);
	}

	if (argc > 2) {
		printf ("Usage: %s [mmddyy]\n", argv[0]);
		exit (1);
	}

	if (argc == 1) {
		printf ("Enter new system date [mmddyy]: ");
		gets (newdate);
		fflush (stdin);
	} else {
		strcpy (newdate, argv[1]);
	}

	if (strlen(newdate) != 6) {
		printf ("Bad conversion\n");
		printf ("Usage: %s [mmddyy]\n", argv[0]);
		exit (1);
	}

	d.da_year = 1900 + atoi (&newdate[4]);
	newdate[4] = '\0';
	d.da_day = atoi (&newdate[2]);
	newdate[2] = '\0';
	d.da_mon = atoi (newdate);

	if (d.da_mon < 1 || d.da_mon > 12) {
		printf ("Bad conversion\n");
		printf ("Usage: %s [mmddyy]\n", argv[0]);
		exit (1);
	}

	switch (d.da_mon) {
	case 1:
	case 3:
	case 5:
	case 7:
	case 8:
	case 10:
	case 12: 	if (d.da_day < 1 || d.da_day > 31) {
				printf ("Bad conversion\n");
				printf ("Usage: %s [mmddyy]\n", argv[0]);
				exit (1);
			}
			break;
	case 4:
	case 6:
	case 9:
	case 11:	if (d.da_day < 1 || d.da_day > 30) {
				printf ("Bad conversion\n");
				printf ("Usage: %s [mmddyy]\n", argv[0]);
				exit (1);
			}
			break;
	case 2:	if (!(d.da_year%4)) {
				if (d.da_day < 1 || d.da_day > 29) {
					printf ("Bad conversion\n");
					printf ("Usage: %s [mmddyy]\n", argv[0]);
					exit (1);
				}
			} else if (d.da_day < 1 || d.da_day > 28) {
				printf ("Bad conversion\n");
				printf ("Usage: %s [mmddyy]\n", argv[0]);
				exit (1);
			}
			break;
	}

	setdate (&d);
	return(0);
}
