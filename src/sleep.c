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
#include <stdlib.h>
#include <stdio.h>
#include "utility.h"

int main (int argc, char *argv[])

{
	int secs;

	if (CheckLicense () == -1) {
		Perror (argv[0]);
		exit (1);
	}
	if (argc != 2) {
		printf ("Usage: %s seconds\n", argv[0]);
		exit (1);
	}

	secs = atoi (argv[1]);
	sleep (secs);
	return (0);
}
