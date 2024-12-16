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
#include <process.h>
#include <stdio.h>
#include <string.h>
#include "utility.h"

int main (int argc, char *argv[])

{
  char manfile[80];
  char output[80];
  FILE *manpage;
  int amount = 20;
  int done = 0;
  int i;

  if (CheckLicense() == -1) {
	Perror(argv[0]);
	exit (1);
  }
   if (argc != 2) {
	fprintf (stderr, "Usage: man command\n");
	exit (1);
   }
	if (strchr (argv[1], '.') == NULL)
		sprintf (manfile, "%s%s.", MANDIR, argv[1]);
	else
		sprintf (manfile,"%s%s", MANDIR, argv[1]);

   if ((manpage = fopen (mkFileName(manfile), "r")) == NULL) {
		sprintf (manfile, "c:\\dos\\help.exe %s%c", argv[1], '\0');
		strcpy (argv[0], "help");
		if ( spawnlp (P_OVERLAY, "help.exe", argv[1], argv[1], "") != 0) {
			fprintf (stderr, "Manual page entry not found for %s\n", argv[1]);
		}
	exit (1);
   }

   while ( (fgets(output, 80, manpage) != NULL) && !done ) {
	 for (i = 0; i < amount; i++) {
		printf ("%s", output);
		if (fgets (output, 80, manpage) == NULL)
			break;
	 }
	 if ( i >= amount ) {
		printf ("%s-- MORE --", output);
		switch (toupper(getch ())) {
		case '\r' : amount = 1;
				 break;
		case ' ' : amount = 20;
				 break;
		case 'D' : amount = 10;
				 break;
		case 'T' : rewind (manpage);
				 amount = 20;
				 break;
		case 'Q' : amount = 0;
				 done = 1;
				 break;
		}
		printf ("\b\b\b\b\b\b\b\b\b\b          \b\b\b\b\b\b\b\b\b\b");
		fflush(stdout);
	 } else {
		break;
	 }
   }
   return(0);
}

