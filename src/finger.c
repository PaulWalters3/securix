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
#include "utility.h"

int	main (int argc, char *argv[])
{
	char file[MAXPATH], last[20], sysname[80];
	char match[MAXUSER];
	FILE *fp, *lfp;
	struct passwd_s finger;

	if (CheckLicense() == -1) {
		Perror (argv[0]);
		exit (1);
	}
	if (argc > 2) {
		printf ("Usage: %s [userid]\n", argv[0]);
		exit (1);
	}

	if (argc == 2)
		strcpy (match, argv[1]);

	if ((fp = fopen (mkFileName(SYSNAME),"r")) != NULL) {
		fgets (sysname, 80, fp);
		sysname[strlen(sysname)-1] = '\0';
		fclose (fp);
	} else {
		strcpy (sysname, "LIST OF");
	}

	if ((fp = fopen (mkFileName(PASSWD), "r")) == NULL) {
		perror ("passwd");
		exit (1);
	}

	printf ("%s USER IDENTIFICATIONS\n", sysname);
	printf ("-----------------------------------\n");

	while (!feof (fp)) {
		if (fread ((struct passwd_s *) &finger, sizeof (struct passwd_s), 1, fp) == 0)
			break;
		if (argc == 2)
			if (strcmp (finger.userid, match))
				continue;
		printf ("USER-> %s \n", finger.userid);
		printf ("USER # %d \n", finger.userno);
		printf ("HOME-> %s \n", finger.home);
		printf ("SHELL-> %s \n", finger.shell);
		printf ("TIMEZONE-> %s \n", finger.tz);
		if (strcmp (finger.name, ""))
			printf ("NAME-> %s     PHONE-> %s\n", finger.name, finger.phone);
		printf ("LAST LOGIN-> ");
		strcpy (file, USERCTRLDIR);
		strcat (file, finger.userid);
		if ((lfp = fopen (mkFileName(file), "r")) != NULL ) {
			fgets (last, 80, lfp);
			fclose (lfp);
			printf ("%s", last);
		} else
			printf ("NEVER.\n");
		printf ("\n");
	}
	fclose (fp);
	return (0);
}
