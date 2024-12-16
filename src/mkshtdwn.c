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
#include <ctype.h>
#include <dos.h>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "utility.h"

void install_shtdwn(void);
void remove_shtdwn(void);

int main (void)
{
	int done=0;

	if (RootLogin(0) == -1)
		exit (1);

	while (!done) {
		printf ("(1) -- Install Shutdown Configuration.\n");
		printf ("(2) -- Remove Shutdown Configuration.\n");
		printf ("(q) -- Quit.\n");
		switch (getch()) {
		case '1' : install_shtdwn();
				 break;
		case '2' : remove_shtdwn();
				 break;
		case 'q' :
		case 'Q' : done = 1;
				 break;
		default  : printf ("Invalid Selection.\n");
				 break;
		}
	}
	return(0);
}

void install_shtdwn (void)

{
	char string[80];
	int done=0;
	FILE *fp;
	struct stat statbuf;

	while (!done) {
		printf ("Enter the full path name of the program to be run at shutdown.\n");
		printf ("Example: 	c:\\etc\\park.com\n");
		printf ("-> ");
		gets (string);
		printf ("Is this information correct (Y/N)? ");
		if (toupper(getch()) == 'Y') {
			if (!access(string,0)) {
				done = 1;
			} else {
				printf ("\n");
				perror (string);
			}
		}
	}
	chmod (mkFileName(SHUTDOWN), S_IREAD | S_IWRITE);
	if ((fp = fopen (mkFileName(SHUTDOWN), "w")) == NULL) {
		perror (mkFileName(SHUTDOWN));
		chmod (mkFileName(SHUTDOWN), S_IREAD);
		exit (1);
	}
	fprintf (fp, "%s\n", string);
	fclose (fp);
	chmod (mkFileName(SHUTDOWN), S_IREAD);
	if ((fp = fopen (mkFileName(SHUTDOWN), "r")) == NULL) {
		perror (mkFileName(SHUTDOWN));
		exit (1);
	}
	stat (mkFileName(SHUTDOWN), &statbuf);
	fclose (fp);
	chmod (mkFileName(SHUTDOWNAUTH), S_IWRITE | S_IREAD);
	if ((fp = fopen (mkFileName(SHUTDOWNAUTH), "w")) == NULL) {
		perror (mkFileName(SHUTDOWNAUTH));
		chmod (mkFileName(SHUTDOWNAUTH), S_IREAD);
		exit (1);
	}
	fprintf (fp, "%ld\n%ld\n", statbuf.st_size, statbuf.st_mtime);
	fclose (fp);
	chmod (mkFileName(SHUTDOWNAUTH), S_IREAD);
	printf ("\nShutdown configured.\n");
}

void remove_shtdwn (void)

{
	int done=0;
	FILE *fp;
	struct stat statbuf;

	while (!done) {
		printf ("Are you sure you wish to remove the shutdown configuration (Y/N)? ");
		if (toupper(getch()) == 'Y')
			done = 1;
		else {
			printf ("\nShutdown Configuration Removal ABORTED.\n");
			return;
		}
	}
	chmod (mkFileName(SHUTDOWN), S_IREAD | S_IWRITE);
	if ((fp = fopen (mkFileName(SHUTDOWN), "w")) == NULL) {
		perror(mkFileName(SHUTDOWN));
		chmod(mkFileName(SHUTDOWN), S_IREAD);
		exit(1);
	}
	fclose (fp);
	chmod (mkFileName(SHUTDOWN), S_IREAD);
	if ((fp = fopen (mkFileName(SHUTDOWN), "r")) == NULL) {
		perror (mkFileName(SHUTDOWN));
		exit (1);
	}
	stat (mkFileName(SHUTDOWN), &statbuf);
	fclose (fp);
	chmod (mkFileName(SHUTDOWNAUTH), S_IWRITE | S_IREAD);
	if ((fp = fopen (mkFileName(SHUTDOWNAUTH), "w")) == NULL) {
		perror (mkFileName(SHUTDOWNAUTH));
		chmod (mkFileName(SHUTDOWNAUTH), S_IREAD);
		exit (1);
	}
	fprintf (fp, "%ld\n%ld\n", statbuf.st_size, statbuf.st_mtime);
	fclose (fp);
	chmod (mkFileName(SHUTDOWNAUTH), S_IREAD);
	printf ("\nShutdown Configuration Removed.\n");
}

