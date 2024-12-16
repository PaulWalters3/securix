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

int main (void)

{
  char user[MAXUSER];
  char login[MAXUSER];
  char logtime[100];
  FILE *fp;

	if (DetermineUserID(user) == -1)
		exit (1);

	strcpy (logtime, USERCTRLDIR);
	strcat (logtime,user);
	if ((fp = fopen (mkFileName(logtime), "r")) == NULL) {
		exit (1);
	}
	fgets (login, MAXUSER, fp);
	fclose (fp);

	printf ("USERID                   LOGIN @\n");
	printf ("-------------------------------------------------\n");
	printf ("%-20s     %s", user, login);
	return (0);
}
