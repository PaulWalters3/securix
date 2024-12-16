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

#include <alloc.h>
#include <dos.h>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "utility.h"

int main (void)

{
	FILE *fp;
	char sysuser[MAXUSER], syscomp[MAXUSER];
	char sysname[MAXUSER];
	char license[MAXLICENSE];
	char userid[MAXUSER];
	time_t t;
	struct tm *current;

	if (CheckLicense() == -1) {
		Perror ("");
		exit (1);
	}
	if ((fp = fopen (mkFileName(SYSNAME), "r")) == NULL) {
		strcpy (sysname, "NONE");
	} else {
		fgets (sysname, MAXUSER, fp);
		sysname[strlen(sysname)-1] = '\0';
		fclose (fp);
	}

	if (DetermineUserID (userid) == -1)
		exit (1);

	if ((fp = fopen (mkFileName(LICENSE), "r")) == NULL) {
		strcpy (license, "IMPROPER LICENSE.");
		strcpy (sysuser, "NO OWNER.");
		strcpy (syscomp, "");
	} else {
		fscanf (fp, "%s\n", license);
		fgets (sysuser, MAXUSER, fp);
		sysuser[strlen(sysuser)-1] = '\0';
		fgets (syscomp, MAXUSER, fp);
		syscomp[strlen(syscomp)-1] = '\0';
		fclose (fp);
	}

	printf ("SYSTEM INFORMATION\n");
	printf ("%s\n", VERSION);
	printf ("%s\n", COPYRIGHT);
	printf ("-------------------------------------------------\n");
	printf ("SYSTEM NAME    : %s\n", sysname);
	printf ("SERIAL NUMBER  : %s\n", license);
	printf ("LICENSED TO    : %s\n", sysuser);
	if (syscomp[0])
		printf ("                 %s\n", syscomp);
	printf ("CURRENT USER   : %s\n", userid);

	if (!access (mkFileName(IGNLOGIN), 0)) {
		if (!PermLoginOkay ())
			printf ("LOGIN SECURITY : OFF\n");
		else
			printf ("LOGIN SECURITY : ON\n");
	} else {
		printf ("LOGIN SECURITY : ON\n");
	}
	t = time(NULL);
	current = localtime(&t);
	printf ("SYSTEM TIME    : %s", asctime(current));

	return (0);
}
