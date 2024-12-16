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
#include <errno.h>
#include <io.h>
#include <process.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "utility.h"

int check_password (struct passwd_s *userinfo);
int get_user_info (struct passwd_s *userinfo);
void write_login_info (struct passwd_s *userinfo);

int main (int argc, char *argv[])

{
	char olddir[MAXPATH];
	static struct passwd_s olduser, newuser;

	if (DetermineUserID(olduser.userid) == -1)
		exit (1);

	if (argc == 1)
		strcpy (newuser.userid, "root");

	else if (argc == 2)
		strcpy (newuser.userid, argv[1]);

	else {
		printf ("Usage: %s [userid]\n", argv[0]);
		exit (1);
	}

	if (RootLogin(1) == -1) {
		printf ("Password: ");
		get_passwd(newuser.passwd);
		encode (newuser.passwd);

		/**** Verify password ****/

		if (!check_password (&newuser)) {
			printf ("\nPassword incorrect.\n");
			exit (1);
		}

		printf ("\n");
	} else if (!get_user_info (&newuser)) {
		printf ("Unable to SU.\n");
		exit (1);
	}

	strcpy (olduser.home, getenv("HOME"));
	strcpy (olduser.shell, getenv("SH"));
	strcpy (olduser.tz, getenv("TZ"));

	write_login_info (&newuser);

	getcwd (olddir, MAXPATH);
	chdir (newuser.home);
	spawnlp (P_WAIT, "command.com", NULL);
	write_login_info (&olduser);
	chdir (olddir);
	return (0);
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
		printf ("FATAL.  Password file missing.\n");
		fix_passwd ();
		return (0);
	}

	while (!feof (fp) && !found) {
		if (fread ((struct passwd_s *) &compare, sizeof(struct passwd_s), 1, fp) == 0)
			break;
		if (!strcmp (compare.userid, userinfo->userid)) {
			found = 1;
			if (!strcmp (compare.passwd, userinfo->passwd)) {
					okay = 1;
					strcpy (userinfo->home, compare.home);
					strcpy (userinfo->tz, compare.tz);
					strcpy (userinfo->shell, compare.shell);
			}
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

void write_login_info (struct passwd_s *userinfo)

{
	char env_home[MAXDIR];
	char env_user[MAXUSER];
	char env_shell[MAXPATH];
	char env_tz[MAXTZ];
	FILE *fp;
	struct stat statbuf;
	static int first_tm=1;

	chmod (mkFileName(USER), S_IREAD | S_IWRITE);
	if ((fp = fopen (mkFileName(USER), "w")) == NULL) {
		perror (mkFileName(USER));
		printf ("Unable to validate current user.\n");
		chmod (mkFileName(USER), S_IREAD);

	} else {
		fprintf (fp, "%s", userinfo->userid);
		fclose (fp);
		chmod (mkFileName(USER), S_IREAD);
	}

	if (first_tm) {
		sprintf(env_home,"HOME=%s",userinfo->home);
		putenv(env_home);
		sprintf(env_user,"USER=%s",userinfo->userid);
		putenv(env_user);
		sprintf(env_shell,"SH=%s", userinfo->shell);
		putenv(env_shell);
		sprintf(env_tz, "TZ=%s", userinfo->tz);
		putenv(env_tz);
		first_tm = 0;
	}

	if ((fp = fopen (mkFileName(USER), "r")) == NULL) {
		perror (mkFileName(USER));
		printf ("Cannot validate current user.\n");
	} else {
		stat (mkFileName(USER), &statbuf);
		fclose (fp);
		chmod (mkFileName(LOGINAUTH), S_IREAD | S_IWRITE);
		if ((fp = fopen (mkFileName(LOGINAUTH), "w")) == NULL) {
			printf ("Error writing login file.\n");
			return;
		}
		fprintf (fp, "%ld\n%ld\n", statbuf.st_size, statbuf.st_mtime);
		fclose (fp);
		chmod (mkFileName(LOGINAUTH), S_IREAD);
	}
}

/**********************************************************/

int get_user_info (struct passwd_s *userinfo)

{
	struct passwd_s compare;
	int found=0;
	FILE *fp;

	if ((fp = fopen (mkFileName(PASSWD), "r")) == NULL) {
		perror (mkFileName(PASSWD));
		printf ("FATAL.  Password file missing.\n");
		fix_passwd ();
		return (0);
	}

	while (!feof (fp) && !found) {
		if (fread ((struct passwd_s *) &compare, sizeof(struct passwd_s), 1, fp) == 0)
			break;
		if (!strcmp (compare.userid, userinfo->userid)) {
			found = 1;
			strcpy (userinfo->home, compare.home);
			strcpy (userinfo->tz, compare.tz);
			strcpy (userinfo->shell, compare.shell);
		}
	}
	fclose (fp);
	return (found);
}
