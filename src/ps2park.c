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
#include <stdio.h>
#include <stdlib.h>
#include "utility.h"

int main (void)

{
	union REGS inregs, outregs;

	if (CheckLicense() == -1) {
		Perror ("ps2park");
		exit (1);
	}
	inregs.h.ah = 0x19;
	inregs.h.dl = 0x80;
	int86 (0x13, &inregs, &outregs);
	if (outregs.h.ah) {
		printf ("PARK FAILED.\n");
		exit (1);
	}
	printf ("DISK PARKED.\n");
	return (0);
}
