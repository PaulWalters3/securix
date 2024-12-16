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

#include <ctype.h>
#include <dir.h>
#include <dos.h>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utility.h"

int main (int argc, char *argv[])

{
	char curdir[MAXPATH], fn[MAXPATH];
	int curdisk;
	int i, done, start_arg=1, change=0, attrib;
	int r_rd=0, r_hd=0, r_sy=0, r_vl=0, r_dr=0, r_ar=0;
	int a_rd=0, a_hd=0, a_sy=0, a_vl=0, a_dr=0, a_ar=0;
	int rd=0, hd=0, sy=0, vl=0, dr=0, ar=0;
	char drive[MAXDRIVE], dirdir[MAXDIR], name[MAXFILE], ext[MAXEXT];
	struct ffblk f;

	if (chmod_access() == -1) {
		Perror ("chmod");
		exit (1);
	}

	if (argc < 2 || argc > 3) {
		printf ("Usage: %s [[[-][+]][w][h][s][v][d][a]...] filelist\n", argv[0]);
		exit (1);
	}

	for (i = 1; i < argc; ++i) {
		if (argv[i][0] == '-') {
			change = 1;
			if (strchr (argv[i], 'w'))
				r_rd = 1;
			if (strchr (argv[i], 'h'))
				r_hd = 1;
			if (strchr (argv[i], 's'))
				r_sy = 1;
			if (strchr (argv[i], 'v'))
				r_vl = 1;
			if (strchr (argv[i], 'd'))
				r_dr = 1;
			if (strchr (argv[i], 'a'))
				r_ar = 1;
		} else if (argv[i][0] == '+') {
			change = 1;
			if (strchr (argv[i], 'w'))
				a_rd = 1;
			if (strchr (argv[i], 'h'))
				a_hd = 1;
			if (strchr (argv[i], 's'))
				a_sy = 1;
			if (strchr (argv[i], 'v'))
				a_vl = 1;
			if (strchr (argv[i], 'd'))
				a_dr = 1;
			if (strchr (argv[i], 'a'))
				a_ar = 1;
		} else {
			start_arg = i;
			break;
		}
	}

	for (i = 0; i < strlen (argv[start_arg]); ++i) {
		if (argv[start_arg][i] == '/')
			argv[start_arg][i] = '\\';
	}

	curdisk = getdisk ();
	if (argv[start_arg][1] == ':') {
		setdisk (toupper(argv[start_arg][0])-'A');
		strcpy (fn, &argv[start_arg][2]);
	} else {
		strcpy (fn, argv[start_arg]);
	}
	getcwd (curdir, MAXPATH);

	fnsplit (fn, drive, dirdir, name, ext);
	if (strcmp (dirdir, ""))
		chdir (dirdir);

	sprintf (fn, "%s%s", name, ext);
	attrib = FA_RDONLY + FA_HIDDEN + FA_DIREC + FA_SYSTEM + FA_LABEL + FA_ARCH;
	done = findfirst (fn, &f, attrib);

	if (!change) {
		while (!done) {
			if ((attrib = _chmod (f.ff_name, 0)) == -1)
				perror (f.ff_name);

			if (attrib & FA_RDONLY)
				rd = 1;
			if (attrib & FA_HIDDEN)
				hd = 1;
			if (attrib & FA_SYSTEM)
				sy = 1;
			if (attrib & FA_LABEL)
				vl = 1;
			if (attrib & FA_DIREC)
				dr = 1;
			if (attrib & FA_ARCH)
				ar = 1;
			printf ("%-15.15s %c%c%c%c%c%c\n", f.ff_name, (dr==1?'d':'-'),
				(sy==1?'s':'-'), (hd==1?'h':'-') , (ar==1?'a':'-'),
				(rd==1?'r':'w'), (vl==1?'l':'-'));
			dr = sy = hd = ar = rd = vl = 0;
			done = findnext (&f);
		}
	} else {
		while (!done) {
			if ((attrib = _chmod (f.ff_name, 0)) == -1) {
				perror (f.ff_name);
				continue;
			}

			if ((attrib & FA_RDONLY) && (a_rd))
				attrib -= FA_RDONLY;
			if (!(attrib & FA_RDONLY) && (r_rd))
				attrib += FA_RDONLY;

			if ((attrib & FA_HIDDEN) && (r_hd))
				attrib -= FA_HIDDEN;
			if (!(attrib & FA_HIDDEN) && (a_hd))
				attrib += FA_HIDDEN;

			if ((attrib & FA_SYSTEM) && (r_sy))
				attrib -= FA_SYSTEM;
			if (!(attrib & FA_SYSTEM) && (a_sy))
				attrib += FA_SYSTEM;

			if ((attrib & FA_LABEL) && (r_vl))
				attrib -= FA_LABEL;
			if (!(attrib & FA_LABEL) && (a_vl))
				attrib += FA_LABEL;

			if ((attrib & FA_DIREC) && (r_dr))
				attrib -= FA_DIREC;
			if (!(attrib & FA_DIREC) && (a_dr))
				attrib += FA_DIREC;

			if ((attrib & FA_ARCH) && (r_ar))
				attrib -= FA_ARCH;
			if (!(attrib & FA_ARCH) && (a_ar))
				attrib += FA_ARCH;

			if (attrib < 0)
				attrib = 0;
			if (_chmod (f.ff_name, 1, attrib) == -1)
				perror (f.ff_name);
			done = findnext (&f);
		}
	}
	chdir (curdir);
	setdisk (curdisk);
	return (0);
}
