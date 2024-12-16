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

#include <dir.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "utility.h"

int main (int argc, char *argv[])
{
	char cmd_ln[MAXPATH];
	unsigned long t1, t2, diff;
	time_t start, stop;
	double hours, mins, secs, secs1;
	int i, j=1;
	FILE *fp;

	if (CheckLicense() == -1) {
		Perror (argv[0]);
		exit (1);
	}
	if (argc < 2) {
		printf ("Usage: %s [-f filename] commmand [args...]\n", argv[0]);
		exit (1);
	}

	if (!strcmp (argv[1], "-f")) {
		j = 3;
		if (argc < 4) {
			printf ("Usage: %s [-f filename] command [args...]\n",argv[0]);
			exit (1);
		}
		for (i=0; i < strlen(argv[2]); i++) {
			if (argv[2][i] == '/')
				argv[2][i] = '\\';
		}
		if ((fp = fopen (argv[2], "w")) == NULL) {
			perror (argv[2]);
			exit (1);
		}
	} else {
		fp = stdout;
	}

	strcpy (cmd_ln, "");
	for (i = j; i < argc; i++) 
		sprintf (&cmd_ln[strlen(cmd_ln)], "%s ", argv[i]);

	start = time(NULL);
	t1 = clock();
	if (system (cmd_ln) == -1) {
		fprintf (fp, "%s:%s\n", cmd_ln, strerror(errno));
	} else {
		t2 = clock();
		stop = time(NULL);
		diff = t2 - t1;
		secs = diff / CLK_TCK;
		hours = diff / CLK_TCK / 3600;
		mins = hours - (int) hours;
		mins = (mins * 100) * 60 / 100;
		secs1 = secs - (((int) mins) * 60);

		fprintf (fp, "\nTimer Statistics\n");
		fprintf (fp, "-------------------------------------\n");
		fprintf (fp, "Command:     %s\n", cmd_ln);
		fprintf (fp, "Start Time:  %s", ctime(&start));
		fprintf (fp, "Stop Time:   %s", ctime(&stop));
		if (hours > 1.0) 
			fprintf (fp, "Execution Time: %d hrs : %d mins : %d secs\n", (int) hours, (int) mins, (int) secs1);
		else if (mins > 1.0)
			fprintf (fp, "Execution Time: %d mins : %d secs\n", (int) mins, (int) secs1);
		else 
			fprintf (fp, "Execution Time: %4.2lf seconds\n", secs);
	}
	if (fp != stdout)
		fclose (fp);
	return (0);
}
