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
#include <dir.h>
#include <dos.h>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "utility.h"

int remove_user (char userid[MAXUSER]);
int remove_user_dir (char dir[MAXPATH]);

/**********************************************************/

int main (int argc, char *argv[])

{
	char userid[MAXUSER];

	if (argc != 2) {
		printf ("Usage: %s userid\n", argv[0]);
		exit(1);
	}

	if (RootLogin(0) == -1) {
		printf ("Sorry, not superuser.  Access denied\n");
		exit(1);
	}

	strcpy (userid, argv[1]);
	if (remove_user (userid)) {
		fprintf (stderr, "\nUnable to remove user %s from system.\n", userid);
		exit(1);
	}
	return (0);
}

/**********************************************************/

int remove_user (char userid[MAXUSER])

{
	struct passwd_s userinfo;
	char dir[MAXPATH], buffer[MAXPATH], tmp_fn[MAXPATH];
	int found = 0;
	FILE *fp1, *fp2;

	chmod (mkFileName(PASSWD), S_IWRITE);
	if ((fp1 = fopen (mkFileName(PASSWD), "r+")) == NULL) {
		perror (mkFileName(PASSWD));
		chmod (mkFileName(PASSWD), S_IREAD);
		return (1);
	}
	if ((fp2 = fopen (mkFileName(PASSWDTMP), "w")) == NULL) {
		perror ("passwd.tmp");
		fclose (fp1);
		chmod (mkFileName(PASSWD), S_IREAD);
		return (1);
	}

	dir[0] = '\0';
	while ( !feof(fp1) ) {
		if (fread ((struct passwd_s *) &userinfo, sizeof(struct passwd_s), 1, fp1) == 0)
			break;
		if (!strcmp (userinfo.userid, userid)) {
			found = 1;
			strcpy (dir, userinfo.home);
		} else
			fwrite ((struct passwd_s *) &userinfo, sizeof(struct passwd_s), 1, fp2);
	}
	fclose (fp1);
	fclose (fp2);
	if (unlink (mkFileName(PASSWD))) {
		perror ("PANIC: passwd missing");
		while (1);
	}
	strcpy (tmp_fn, mkFileName(PASSWDTMP));
	if (rename (tmp_fn, mkFileName(PASSWD))) {
		perror ("PANIC: cannot rename");
		while (1);
	}
	chmod (mkFileName(PASSWD), S_IREAD);

	/* remove last login times files */
	if (found) {
		sprintf (buffer, "%s%s", USERCTRLDIR, userid);
		chmod (mkFileName(buffer), S_IWRITE | S_IREAD);
		unlink (mkFileName(buffer));
		sprintf (buffer, "%s%s", USERFAILDIR, userid);
		chmod (mkFileName(buffer), S_IWRITE | S_IREAD);
		unlink (mkFileName(buffer));

		sprintf (buffer, "%s%s", MAILDIR, userid);
		chmod (mkFileName(buffer), S_IWRITE | S_IREAD);
		unlink (mkFileName(buffer));

		printf ("Remove user directory %s? ", dir);
		switch (toupper(getche())) {
			case 'Y' : return ( remove_user_dir (dir) );
		}
		return (0);
	}
	return (1);
}

/**********************************************************/

int remove_user_dir (char dir[MAXPATH])

{
	int done;
	char olddir[MAXPATH];
	struct ffblk ff;

	getcwd (olddir, MAXPATH);
	if (chdir (dir))
		return (1);

	done = findfirst ("*.*", &ff, FA_RDONLY | FA_HIDDEN | FA_SYSTEM | FA_ARCH | FA_DIREC);
	while (!done) {
		if (ff.ff_attrib & FA_DIREC) {
			if (strcmp(ff.ff_name, ".") && strcmp(ff.ff_name, ".."))
				remove_user_dir (ff.ff_name);
		}
		_chmod (ff.ff_name, 1, 0);
		unlink (ff.ff_name);
		done = findnext (&ff);
	}
	if (chdir (olddir)) {
		perror (olddir);
		return (1);
	}
	if (rmdir (dir)) {
		perror (dir);
		return (1);
	}
	return (0);
}
