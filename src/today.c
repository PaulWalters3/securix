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
#include <time.h>
#include "utility.h"

int main (int argc, char *argv[])
{
	time_t t;

	if (CheckLicense() == -1) {
		Perror (argv[0]);
		exit (1);
	}

	t = time (NULL);
	printf ("%s", ctime (&t));
	return (0);
}
