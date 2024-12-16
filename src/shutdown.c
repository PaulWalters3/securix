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
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys\stat.h>
#include <string.h>
#include <time.h>
#include "utility.h"

#define MAXLINE 80

int breakhandler(void);

int main (void)
{
	FILE *fp;
	char shtline[MAXLINE];
	struct stat statbuf;
	time_t t;
	long msize, mtime;

	if (CheckLicense() == -1) {
		Perror ("shutdown");
		exit (1);
	}
	ctrlbrk (breakhandler);

	if ((fp = fopen(mkFileName(SHUTDOWN), "r")) != NULL) {
		stat (mkFileName (SHUTDOWN), &statbuf);
		if (statbuf.st_size == 0) {
			strcpy (shtline, "");
		} else {
			fgets (shtline, MAXLINE, fp);
			shtline[strlen(shtline)-1] = '\0';
		}
		fclose (fp);
		if ((fp = fopen(mkFileName(SHUTDOWNAUTH), "r")) == NULL) {
			printf ("Shutdown procedure file corrupt.\n");
			printf ("Not executing %s.\n", shtline);
		} else {
			fscanf (fp, "%ld\n%ld\n", &msize, &mtime);
			fclose (fp);
			if (msize != statbuf.st_size || mtime != statbuf.st_mtime) {
				printf ("Shutdown procedure file corrupt.\n");
				printf ("Not executing %s.\n", shtline);
			} else {
				if (strlen(shtline) > 0) {
					if (!access (shtline, 0)) {
						system (shtline);
					} else {
						perror (shtline);
					}
				}
					
			}
		}
	}
	t = time(NULL);
	printf ("System Shutdown at %s", ctime(&t));
	printf ("     旼컴컴컴컴컴컴컴컴컴컴컴컴컴컴커\n");
	printf ("     �    Safe To Turn Off Power    �\n");
	printf ("     읕컴컴컴컴컴컴컴컴컴컴컴컴컴컴켸\n");
	while (1);
}

int breakhandler (void) 
{
	return(1);
}
