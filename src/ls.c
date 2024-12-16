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
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "macros.h"
#include "tility.h"

#define MAXENTRY 512

struct list {
   char  one_per_ln;
   char  hidden;
   char  dir_only;
   char  marks;
   char  all;
   char  no_sort;
   char  rev_sort;
   char  filename [MAXPATH];
};

char curdir[MAXDIR];

void disk_free (void);
void list_array (char entry [MAXENTRY][80], int num, struct list *attrib);
void list_default (void);
void list_files (struct list *attrib);
void list_long (struct list *attrib, int attributes);
void merge (char entry [MAXENTRY][80], int lfirst, int llast, int rfirst, int rlast, int off);
void parse (struct list *attrib, int argc, char *argv []);
void quit (void);
void sort  (char entry [MAXENTRY][80], int first, int last, int off);

char drive='c';
int curdrive;

/************************************************/

int main (int argc, char *argv [])

{
	struct  list attrib = {FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, "*.*"};

	if (CheckLicense() == -1) {
		printf ("%s\n", BADLICENSE);
		exit (1);
	}

	atexit (quit);
	curdrive = getdisk();
	getcwd (curdir, MAXDIR);

	if (argc == 1) {
		list_default ();
		printf ("\n");
		exit (0);
	} else {
		parse (&attrib, argc, argv);
		list_files (&attrib);
		printf ("\n");
	}
	quit();
	return (0);
}

/************************************************/

void disk_free (void)

{
	struct dfree free;
	long avail;

	drive = toupper(drive)-64;
	getdfree (drive, &free);
	if (free.df_sclus == 0xFFFF) {
		return;
	 } else {
		avail = (long) free.df_avail *
			   (long) free.df_sclus *
			   (long) free.df_bsec;
		printf ("  %ld bytes available.\n", avail);
	}
}

/************************************************/

void list_array (char entry[MAXENTRY][80], int num, struct list *attrib)

{
	int j;

	if (attrib->rev_sort) {
		for (j = num; j >= 0; j--)
		if (attrib->one_per_ln)
			printf ("%s\n", entry [j]);
		else
			printf ("%-16s", entry [j]);
	} else {
		for (j = 0; j <= num; j++)
		if (attrib->one_per_ln)
			printf ("%s\n", entry [j]);
		else
			printf ("%-16s", entry [j]);
	}
}

/************************************************/

void list_default (void)

{
	BOOLEAN done;
	char entry [MAXENTRY][80];
	int i=0, j, attrib;
	struct ffblk f;

	attrib = FA_RDONLY + FA_HIDDEN + FA_DIREC + FA_SYSTEM + FA_LABEL + FA_ARCH;
	done = findfirst ("*.*", &f, attrib);
	while (!done) {
		strcpy (entry [i++], f.ff_name);
		done = findnext (&f);
	}
	sort (entry, (int) 0, --i, 0);
	for (j = 0; j <= i; j++)
		printf ("%-16s", entry [j]);
}

/************************************************/

void list_files (struct list *attrib)

{
	boolean done;
	char entry [MAXENTRY][80];
	int attributes = FA_ARCH | FA_DIREC | FA_SYSTEM | FA_RDONLY;
	int i = 0;
	struct ffblk  f;

	/* -a- */
	if (attrib->hidden)
		attributes = attributes | FA_HIDDEN;

	/* -f */
	if (attrib->no_sort) {
		done = findfirst (attrib->filename, &f, attributes);
		while (!done) {
			if (attrib->dir_only && !(f.ff_attrib & FA_DIREC)) {   /* check for -d */
				done = findnext (&f);
				continue;
			} else if (attrib->one_per_ln)
				printf ("%s\n", f.ff_name);
			else
				printf ("%-16s", f.ff_name);
			done = findnext (&f);
		}

	/* -l */
	} else if (attrib->all) {
		list_long (attrib, attributes);
	} else {
		done = findfirst (attrib->filename, &f, attributes);
		while (!done) {
			if (attrib->dir_only && f.ff_attrib != '') {   /* check for -d */
				done = findnext (&f);
				continue;
			}
			strcpy (entry [i++], f.ff_name);
			done = findnext (&f);
		}

		sort (entry, (int) 0, --i, 0);
		list_array (entry, i, attrib);
	}
}

/************************************************/

void list_long (struct list *attrib, int attributes)

{
	boolean done;
	char    mdate[10], temp[30], mtime[10];
	char    dir, entry [MAXENTRY][80];
	char    archive, hid, sys, read, write;
	int     i = 0;
	FILE *fp;
	struct  ffblk f;
	struct  stat  b;
	char drive[MAXDRIVE], dirdir[MAXDIR], name[MAXFILE], ext[MAXEXT];
	char dest[MAXPATH];
	int offset;

	fnsplit (attrib->filename, drive, dirdir, name, ext);
	sprintf (dest, "%s%s", drive, dirdir);
	if (strlen(dest) > 1) {
		dest[strlen(dest)-1] = '\0';
		offset = 1;
	} else {
		offset = 0;
	}
	if (strcmp (dest, "")) {
		chdir (dest);
		strcpy (attrib->filename, &attrib->filename[strlen(dest)+offset]);
	}
	done = findfirst (attrib->filename, &f, attributes);
	while (!done) {
		/* directory */
		if (f.ff_attrib & FA_DIREC) {
			dir = 'd';
		} else {
			dir = '-';
			if (attrib->dir_only) {
				done = findnext (&f);
				continue;
			}
		}

		if (f.ff_attrib & FA_SYSTEM)
			sys = 's';
		else
			sys = '-';

		if (f.ff_attrib & FA_HIDDEN)
			hid = 'h';
		else
			hid = '-';

		if (f.ff_attrib & FA_ARCH)
			archive = 'a';
		else
			archive = '-';

		if (!access (f.ff_name, 2))
			write = 'w';
		else
			write = '-';

		if (!access (f.ff_name, 4))
			read = 'r';
		else
			read = '-';

		/* time and date */
		fp = fopen (f.ff_name, "rb");
		stat (f.ff_name, &b);
		fclose (fp);
		strcpy (temp, ctime(&b.st_mtime));
		strncpy (mdate, &temp[4], 6);
		mdate[6] = '\0';
		strncpy (mtime, &temp[11], 5);
		mtime[5] = '\0';
		sprintf (entry [i++], "%c%c%c%c%c%c %7ld %s %s %-12s", dir, sys, hid, archive, write, read,
			    f.ff_fsize, mdate, mtime, f.ff_name);
		done = findnext (&f);
	}
	i--;
	sort (entry, (int) 0, i, 28);
	printf ("   TOTAL %3d\n", i+1);
	list_array (entry, i, attrib);
	disk_free ();
}

/************************************************/

void merge (char entry[MAXENTRY][80], int lfirst, int llast, int rfirst, int rlast, int off)

{
	char temp [MAXENTRY][80];
	int  cleft,
		cright,
		index;

	cleft = lfirst;
	cright = rfirst;
	index = lfirst;

	while (cleft <= llast && cright <= rlast) {
		if (strcmp (entry[cleft] + off, entry[cright] + off) < 0) {
			strcpy (temp[index++], entry [cleft++]);
		} else {
			strcpy (temp [index++], entry [cright++]);
		}
	}
	while (cleft <= llast)
		strcpy (temp [index++], entry [cleft++]);
	while (cright <= rlast)
		strcpy (temp [index++], entry [cright++]);
	for (index = lfirst; index <= rlast; index++)
		strcpy (entry [index], temp [index]);
}

/************************************************/

void parse (struct list *attrib, int argc, char *argv [])

{
	int i, j, attr;
	struct ffblk ff;

	for (i = 1; i < argc; i++) {
		if (argv [i][0] == '-') {
			for (j = 1; j < strlen(argv[i]); j++) {
				switch (argv [i][j]) {
					case '1' : attrib->one_per_ln = TRUE;
							break;
					case 'a' : attrib->hidden = TRUE;
							break;
					case 'd' : attrib->dir_only = TRUE;
							break;
					case 'f' : attrib->no_sort = TRUE;
							attrib->hidden = TRUE;
							attrib->all = FALSE;
							attrib->rev_sort = FALSE;
							break;
					case 'l' : if (!attrib->no_sort) {
								attrib->all = TRUE;
								attrib->one_per_ln = TRUE;
							}
							break;
					case 'r' : if (!attrib->no_sort) attrib->rev_sort = TRUE;
							break;
				}
			}
		} else {
			for (j = 0; j < strlen (argv[i]); j++) {
				if (argv[i][j] == '/') {
					argv[i][j] = '\\';
				}
			}
			if (argv[i][1] == ':') {
				drive = argv[i][0];
				setdisk (toupper(drive)-'A');
			}
			if (argv[i][strlen (argv[i]) - 1] == ':') {
				strcpy (attrib->filename, argv[i]);
				strcat (attrib->filename, "*.*");
				setdisk (toupper(argv[i][strlen(argv[i])-2])-'A');
			} else if (argv[i][strlen (argv[i]) - 1] == '\\') {
				strcpy (attrib->filename, argv[i]);
				strcat (attrib->filename, "*.*");
			} else {
				attr = FA_RDONLY + FA_HIDDEN + FA_DIREC + FA_SYSTEM + FA_LABEL + FA_ARCH;

				findfirst (argv[i], &ff, attr);
				if (ff.ff_attrib & FA_DIREC) {
					strcpy (attrib->filename, argv[i]);
					if (strcmp (&argv[i][strlen(argv[i]-2)], "*.*")) {
						strcat (attrib->filename, "\\");
						strcat (attrib->filename, "*.*");
					}
				} else {
					strcpy (attrib->filename, argv[i]);
				}
			}
		}
	}
}

/************************************************/

void quit (void)

{
	setdisk (curdrive);
	chdir (curdir);
}

/************************************************/

void sort (char entry [MAXENTRY][80],
	   int  first, int last, int off)

{
	int middle;

	if (first < last) {
		middle = (first + last) / 2;
		sort (entry, first, middle, off);
		sort (entry, middle + 1, last, off);
		merge (entry, first, middle, middle+1, last, off);
	}
}

/************************************************/
