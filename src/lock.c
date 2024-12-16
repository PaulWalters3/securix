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
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utility.h"

int breakhandler(void);

int main (void)

{
  int i=0;
  char passwd[69];
  char verify[69];

	if (CheckLicense() == -1) {
		Perror ("lock");
		exit (1);
	}
	ctrlbrk(breakhandler);
	printf ("Enter password: ");
	while ((passwd[i]=getch()) != '\r') {
		if (passwd[i]=='\b'&&i>0) {
			passwd[--i] = '\0';
			printf ("\b \b");
		} else {
			passwd[++i] = '\0';
			printf (".");
		}
	}
	passwd[i]='\0';
	printf ("\n");
	printf ("Terminal locked.\n");

	do {
		i=0;
		printf ("Lock password: ");
		while ((verify[i]=getch()) != '\r') {
			if (verify[i]=='\b'&&i>0) {
				verify[--i] = '\0';
				printf ("\b \b");
			} else {
				verify[++i] = '\0';
				printf (".");
			}
		}
		verify[i]='\0';
		printf ("\n");
	} while (strcmp(passwd,verify));
	return (0);
}

int breakhandler(void)
{
	return(1);
}
