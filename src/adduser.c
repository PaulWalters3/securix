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
#include "macros.h"
#include "utility.h"

void append_file (void);
void get_shell (char shell[MAXSHELL]);
void get_timezone (char tz[MAXTZ]);

/**********************************************************/

struct passwd_s userinfo;

int main (int argc, char *argv[])

{
	char    verify[MAXPASS];
	char    maildir[MAXPATH];
	char    str[MAXPATH], str1[MAXPATH], syscall[MAXPATH];
	char    newhome[MAXPATH], tmp[MAXPATH];
	int     attempts = 1, len, setup=0;
	boolean pwdokay = FALSE;
	FILE    *fp;

	if ((argc == 3) && (!strcmp(argv[2], ADDUSERKEY))) {
		strcpy (userinfo.userid, argv[1]);
		setup = 1;
	}
	else if (CheckLicense() == -1) {
		Perror (argv[0]);
		exit (1);
	}

	if (argc == 2) {
		strcpy (userinfo.userid, argv[1]);

	} else if ((argc == 3) && (strcmp (argv[2], ADDUSERKEY))) {

		printf ("Usage: %s [userid]\n", argv[0]);
		exit (1);

	}
	else if ((argc != 1) && (!setup)) {
		printf ("Usage: %s [userid]\n", argv[0]);
		exit (1);
	}
	if (argc != 3) {
		if (RootLogin(0) == -1)
			exit (1);
	}

	if (argc == 1) {
		printf ("User ID: ");
		get_userid (userinfo.userid);
	}

	if ((fp = fopen (mkFileName(PASSWDLEN), "r")) == NULL) {
		len = 5;
	} else {
		fscanf (fp, "%d", &len);
		fclose (fp);
		if (len < 1)
			len = 5;
	}

	while (!pwdokay && attempts++ <= 3) {
		printf ("User password: ");
		get_passwd (userinfo.passwd);

		if (strlen (userinfo.passwd) < len) {
			printf ("\nPassword must be at least %d characters.\n", len);
			continue;
		}

		printf ("\nVerify: ");
		get_passwd (verify);

		if (!strcmp (userinfo.passwd, verify)) {
			printf ("\n");
			pwdokay = TRUE;
		} else {
			printf ("\nIncorrect.\nPlease try again.\n");
		}
	}

	if (pwdokay) {
		strcpy (userinfo.home, USERDIR);
		strcat (userinfo.home, userinfo.userid);
		strcpy (userinfo.home, mkFileName (userinfo.home));
		if (!setup) {
			newhome[0] = '\0';
			printf ("Home directory for %s [%s]: ", userinfo.userid, userinfo.home);
			gets (newhome);
			if (newhome[0] != '\0')
				strcpy (userinfo.home, newhome);
			printf ("Make home directory %s? ", userinfo.home);
			if (toupper(getch()) == 'Y')
				mkdir  (userinfo.home);

			printf ("\n");
		}
		/****
		Handle Mail
		****/
		strcpy (maildir, MAILDIR);
		strcat (maildir, userinfo.userid);
		fp = fopen (mkFileName(maildir), "w");
		fprintf (fp, "0\n");
		fclose (fp);
		sprintf (syscall, "mail %s \"Welcome Aboard\" %s", userinfo.userid, mkFileName(NEWMAIL));
		system (syscall);

		if (!setup) {
			get_shell (userinfo.shell);
			get_timezone (userinfo.tz);
		} else {
			strcpy (userinfo.shell, "dos");
			strcpy (userinfo.tz, "EST5EDT");
		}

		if (!setup) {
			fflush (stdin);
			printf ("\nUser %s's real name: ", userinfo.userid);
			gets (userinfo.name);
			printf ("User %s's phone number: ", userinfo.userid);
			gets (userinfo.phone);
		} else {
			strcpy (userinfo.name, "Superuser");
			strcpy (userinfo.phone, "None");
		}

		append_file ();

		if (!setup) {
			printf ("Copy login files to home directory %s? ", userinfo.home);
			if (toupper (getch()) == 'Y') {
				strcpy (str1, NEWUSERDIR);
				strcat (str1, "*.*");
				printf ("\n");
				strcpy (tmp, mkFileName ("\\tmp\\00"));
				sprintf (str, "cp -s %s %s > %s", mkFileName(str1), userinfo.home, tmp);
				system (str);
				strcpy (str, NEWUSERDIR);
				strcat (str, userinfo.shell);
				strcat (str, "\\*.*");
				printf ("\n");
				sprintf (str1, "cp -s %s %s > %s", mkFileName(str), userinfo.home, tmp);
				system (str1);
			} else {
				printf ("\n");
			}
		}

		if (!strcmp (userinfo.userid, "root"))
			update_root (&userinfo);

		if (!setup)
			printf ("User %s added to the system.\n", userinfo.userid);
	} else {
		printf ("Adduser aborted.\n");
		exit (1);
	}

	return (0);
}

/**********************************************************/

void append_file (void)
{
	FILE *fp1, *fp2;
	int max=0;
	struct passwd_s info;
	char tmp_fn[MAXPATH];

	chmod (mkFileName(PASSWD), S_IREAD | S_IWRITE);
	if ((fp1 = fopen (mkFileName(PASSWD), "r")) == NULL) {
		perror (mkFileName(PASSWD));
		chmod (mkFileName(PASSWD), S_IREAD);
		exit (1);
	}

	if ((fp2 = fopen (mkFileName(PASSWDTMP), "w")) == NULL) {
		perror (mkFileName(PASSWDTMP));
		fclose (fp1);
		chmod (mkFileName(PASSWD), S_IREAD);
		exit (1);
	}

	while (!feof (fp1)) {
		if (fread ((struct passwd_s *) &info, sizeof(struct passwd_s), 1, fp1) == 0)
			break;
		fwrite ((struct passwd_s *) &info, sizeof(struct passwd_s), 1, fp2);
		if (max <= info.userno)
			max = info.userno;
	}
	fclose (fp1);
	printf ("User %s's user number is %d.\n", userinfo.userid, max+1);
	userinfo.userno = max+1;

	encode (userinfo.passwd);
	fwrite ((struct passwd_s *) &userinfo, sizeof(struct passwd_s), 1, fp2);
	fflush (stdout);
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
	printf ("AVAILABLE USER SHELLS:\n");
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

void get_timezone (char tz[MAXTZ])

{
	int tz_no = 1;
	int sel = 0;
	FILE *fp;

	if ((fp = fopen (mkFileName(TIMEZONES), "r")) == NULL) {
		perror (mkFileName(TIMEZONES));
		printf ("Setting timezone to EST\n");
		strcpy (tz, "EST5EDT");
		return;
	}
	printf ("AVAILABLE TIMEZONES:\n");
	printf ("====================\n");
	while ( fgets (tz, MAXTZ, fp) != NULL )
		printf ("%d %s", tz_no++, tz);
	printf ("\n");
	while ( !sel ) {
		printf ("Enter the number of the timezone to use: ");
		scanf ("%d", &sel);
		if (sel < 1 || sel >= tz_no)
			sel = 0;
	}
	rewind (fp);
	tz_no = 1;
	while ( fgets (tz, 80, fp) != NULL && tz_no != sel)
		tz_no++;
	tz[strlen(tz)-1] = '\0';
	fclose (fp);
}

