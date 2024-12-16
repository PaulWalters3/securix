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
#include "utility.h"

int main (void)

{
	char sn[MAXLICENSE], lk[MAXLICENSE];
	char sysuser[MAXUSER], syscomp[MAXUSER];
	char *key;
	FILE *fp;
	struct stat statbuf;

	printf ("Enter Serial Number: ");
	gets (sn);
	fflush (stdin);

	printf ("Enter License Key: ");
	gets (lk);
	fflush (stdin);

	key = encode_license (sn);
	if (strcmp (lk, key)) {
		printf ("Invalid License Key.\n");
		exit (1);
	}

	printf ("Enter your name: ");
	gets (sysuser);
	fflush (stdin);

	printf ("Enter company name: ");
	gets (syscomp);
	fflush (stdin);

	chmod (mkFileName(LICENSE), S_IREAD | S_IWRITE);
	if ((fp = fopen(mkFileName(LICENSE), "w")) == NULL) {
		perror (mkFileName(LICENSE));
		printf ("Cannot brand system.\n");
		exit (1);
	}
	fprintf (fp, "%s\n%s\n%s\n", sn, sysuser, syscomp);
	fclose (fp);
	chmod (mkFileName(LICENSE), S_IREAD);

	if ((fp = fopen(mkFileName(LICENSE), "r")) == NULL) {
		perror (mkFileName(LICENSE));
		printf ("Cannot brand system.\n");
		exit (1);
	}
	stat (mkFileName(LICENSE), &statbuf);
	fclose (fp);
	chmod (mkFileName(LICENSEAUTH), S_IREAD | S_IWRITE);

	if ((fp = fopen(mkFileName(LICENSEAUTH), "w")) == NULL) {
		perror (mkFileName(LICENSEAUTH));
		printf ("Cannot brand system.\n");
		exit (1);
	}
	fprintf (fp, "%ld\n%ld\n%s\n", statbuf.st_size, statbuf.st_mtime, lk);
	fclose (fp);
	chmod (mkFileName(LICENSEAUTH), S_IREAD);
	return (0);
}
