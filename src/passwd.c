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
#include <dir.h>
#include <dos.h>
#include <errno.h>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "macros.h
#include "utility.h"

void change_info (void);
void change_password (struct passwd_s *userinfo);
int check_password (struct passwd_s *userinfo);
void get_shell (char shell[MAXSHELL]);
void get_timezone (char tz[MAXTZ]);

/**********************************************************/

int main (int argc, char *argv[])

{
	int pwdokay=0;
	int attempts = 1, len;
	char npasswd[MAXPASS];
	char verify[MAXPASS];
	struct passwd_s userinfo;
	FILE *fp;

	if (CheckLicense() == -1) {
		Perror (argv[0]);
		exit (1);
	}

	if (argc > 2) {
		printf ("Usage: %s [-f][userid]\n", argv[0]);
		exit (1);
	}

	if (argc == 2) {
		if (!strcmp (argv[1], "-f")) {
			change_info ();
			exit (0);
		}
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
	if (argc == 1) {
		/**** Get password ****/
		printf ("Password: ");
		get_passwd(userinfo.passwd);
		encode (userinfo.passwd);

		/**** Verify old password ****/

		while ((!(pwdokay = check_password (&userinfo))) && attempts++ < 3) {
			printf ("\nPassword incorrect.\n");
			printf ("Password: ");
			get_passwd(userinfo.passwd);
			encode(userinfo.passwd);
		}

		if (!pwdokay) {
			fprintf (stderr, "\nPassword change aborted.\n");
			exit (1);
		}
		printf ("\n");
	}

	/**** Get new password ****/

	attempts = 1;
	pwdokay = 0;

	if ((fp = fopen (mkFileName(PASSWDLEN), "r")) == NULL) {
		len = 5;
	} else {
		fscanf (fp, "%d", &len);
		fclose (fp);
		if (len < 1)
			len = 5;
	}


	while (!pwdokay && attempts++ <= 3) {
		printf ("New password: ");
		get_passwd(npasswd);

		if (strlen (npasswd) < len) {
			fprintf (stderr, "\nPassword must be at least %d characters.\n", len);
			continue;
		}
		printf ("\nVerify: ");
		get_passwd(verify);
		printf ("\n");

		if (!strcmp (npasswd, verify))
			pwdokay = 1;
		else
			fprintf (stderr, "Try again.\n");
	}

	if (pwdokay) {
		strcpy (userinfo.passwd, npasswd);
		encode (userinfo.passwd);
		change_password (&userinfo);
		if (!strcmp(userinfo.userid, "root"))
			update_root (&userinfo);
		printf ("Password changed.\n");
	} else {
		printf ("Password change aborted.\n");
	}

	return (0);
}

/**********************************************************/

void change_password (struct passwd_s *userinfo)

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
		if (!strcmp (compare.userid, userinfo->userid))
			strcpy (compare.passwd, userinfo->passwd);
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

int check_password (struct passwd_s *userinfo)

{
	struct passwd_s compare;
	int found=0;
	int okay=0;
	FILE *fp;

	if ((fp = fopen (mkFileName(PASSWD), "r")) == NULL) {
		perror (mkFileName(PASSWD));
		return (0);
	}

	while (!feof (fp) && !found) {
		if (fread ((struct passwd_s *) &compare, sizeof(struct passwd_s), 1, fp) == 0)
			break;
		if (!strcmp (compare.userid, userinfo->userid)) {
			found = 1;
			if (!strcmp (compare.passwd, userinfo->passwd))
				okay = 1;
		}
	}
	fclose (fp);
	if (okay) {
		return (1);
	} else {
		return (0);
	}
}

/**********************************************************/

void change_info (void)

{
	struct passwd_s compare, userinfo;
	FILE *fp1, *fp2;
	char tmp_fn[MAXPATH];

	if (DetermineUserID (userinfo.userid) == -1)
		exit (1);


	printf ("Changing information for user %s.\n", userinfo.userid);
	printf ("Real Name: ");
	fflush (stdin);
	gets (userinfo.name);
	printf ("Phone Number: ");
	fflush (stdin);
	gets (userinfo.phone);
	get_shell (userinfo.shell);
	get_timezone (userinfo.tz);

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
		if (!strcmp (compare.userid, userinfo.userid)) {
			strcpy (compare.name, userinfo.name);
			strcpy (compare.phone, userinfo.phone);
			strcpy (compare.shell, userinfo.shell);
			strcpy (compare.tz, userinfo.tz);
		}
		fwrite ((struct passwd_s *) &compare, sizeof(struct passwd_s), 1, fp2);
   }
   fclose (fp1);
   fclose (fp2);
   unlink (mkFileName(PASSWD));
   strcpy (tmp_fn, mkFileName(PASSWDTMP));
   rename (tmp_fn, mkFileName(PASSWD));
   chmod (mkFileName(PASSWD), S_IREAD);
   if (!strcmp(userinfo.userid, "root"))
	update_root (&userinfo);
   printf ("User information changed for user %s.\n", userinfo.userid);
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

/**********************************************************/
