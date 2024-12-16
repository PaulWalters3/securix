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
#include <errno.h>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "macros.h"
#include "utility.h"

void change_shell (struct passwd_s *userinfo);
void get_shell (char shell[MAXSHELL]);

/**********************************************************/

int main (int argc, char *argv[])

{
	struct passwd_s userinfo;
	char str1[80], str[80];

	if (CheckLicense() == -1) {
		Perror (argv[0]);
		exit (1);
	}

	if (argc > 2) {
		printf ("Usage: %s [userid]\n", argv[0]);
		exit (1);
	}

	if (argc == 2 && (RootLogin(0) == -1)) {
		printf ("Sorry, not superuser.  Access denied\n");
		exit (1);
	}

	if (argc == 1) {
		if (DetermineUserID (userinfo.userid) == -1)
			exit (1);
	} else {
		strcpy (userinfo.userid, argv[1]);
	}

	get_shell (userinfo.shell);
	change_shell (&userinfo);

	printf ("Copy login files to home directory %s? ", userinfo.home);
	if (toupper (getch()) == 'Y') {
		strcpy (str, NEWUSERDIR);
		strcat (str, userinfo.shell);
		strcat (str, "\\*.*");
		printf ("\n");
		sprintf (str1, "cp -s %s %s", mkFileName(str), userinfo.home);
		system (str1);
		printf ("\n");
	} else {
		printf ("\n");
	}
	printf ("Shell changed.\n");
	return (0);
}

/**********************************************************/

void change_shell (struct passwd_s *userinfo)

{
	struct passwd_s compare;
	FILE *fp1, *fp2;
	char tmp_fn[MAXPATH];

	chmod (mkFileName(PASSWD), S_IWRITE);
	if ((fp1 = fopen (mkFileName(PASSWD), "r+")) == NULL) {
		chmod(mkFileName(PASSWD), S_IREAD);
		perror (mkFileName(PASSWD));
		exit (1);
	}
	if ((fp2 = fopen (mkFileName(PASSWDTMP), "w")) == NULL) {
		perror(mkFileName(PASSWDTMP));
		fclose (fp1);
		chmod (mkFileName(PASSWD), S_IREAD);
		exit (1);
	}

	while ( !feof(fp1) ) {
		if (fread ((struct passwd_s *) &compare, sizeof(struct passwd_s), 1, fp1) == 0)
			break;
		if (!strcmp (compare.userid, userinfo->userid)) {
			strcpy (compare.shell, userinfo->shell);
			strcpy (userinfo->home, compare.home);
		}
		fwrite ((struct passwd_s *) &compare, sizeof(struct passwd_s), 1, fp2);
   }
   fclose (fp1);
   fclose (fp2);
   unlink (mkFileName(PASSWD));
   strcpy (tmp_fn, mkFileName(PASSWDTMP));
   rename (tmp_fn, mkFileName(PASSWD));
   chmod (mkFileName(PASSWD), S_IREAD);
}

/**********************************************************/

void get_shell (char shell[MAXSHELL])

{
	int sh_no = 1;
	int sel = 0;
	FILE *sh;

	if ((sh = fopen (mkFileName(SHELLS), "r")) == NULL) {
		strcpy (shell, "dos");
		return;
	}
	printf ("\nAVAILABLE USER SHELLS:\n");
	printf ("======================\n");
	while ( fgets (shell, MAXSHELL, sh) != NULL )
		printf ("%d %s", sh_no++, shell);
	printf ("\n");
	while ( !sel ) {
		printf ("Enter the number of the shell to use: ");
		scanf ("%d", &sel);
		if (sel < 1 || sel >= sh_no)
			sel = 0;
	}
	rewind (sh);
	sh_no = 1;
	while ( fgets (shell, 80, sh) != NULL && sh_no != sel)
		sh_no++;
	shell[strlen(shell)-1] = '\0';
	fclose (sh);
}


/**********************************************************/
