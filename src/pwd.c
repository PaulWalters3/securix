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
#include "utility.h"

int main (void)
{
  char dir[MAXPATH];

   if (CheckLicense() == -1) {
	Perror ("pwd");
	exit (1);
   }
   getcwd (dir, MAXPATH);
   printf ("%s\n", dir);
   return (0);
}
