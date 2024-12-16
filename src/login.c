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
#include <fcntl.h>
#include <io.h>
#include <process.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys\stat.h>
#include <time.h>
#include "utility.h"

void add_failed_login (void);
int breakhandler (void);
int check_passwd (void);
static void init (void);
void perm_login (void);
void read_bulletin_message (void);
void read_last_login(void);
void read_login_message (void);
void write_login_info (void);
void execute_shell (void);
void wait_failed_login (void);

/***** Globals ********************************************/

struct passwd_s userinfo;

/**********************************************************/

int main (void)

{
	static int user;
	static int attempts;
	char inst_drive[3];

	ctrlbrk (breakhandler);
	init ();

	/** Change to drive software is installed on. **/
	strcpy (inst_drive, mkFileName(""));
	setdisk (toupper(inst_drive[0])-'A');

	if (!access(mkFileName(IGNLOGIN), 0)) {
		if (!PermLoginOkay ()) {
			perm_login ();
			printf ("FATAL.  Cannot find a shell to execute.\n");
			printf ("Exiting login process to system shell.\n");
			exit (1);
		}
		Perror ("");
	}

	/** Never Exit This Program Once Started **/
	while (1) {
		user = 0;
		attempts = 1;

		while (!user) {
			read_login_message ();

			printf ("Login: ");

			while (!user && attempts++ <= 3) {
				get_userid (userinfo.userid);
				printf ("Password: ");
				get_passwd (userinfo.passwd);
				printf ("\n");
				encode (userinfo.passwd);

				if (check_passwd())
					user = 1;
				else {
					printf ("Login incorrect.\n");
					wait_failed_login();
					add_failed_login ();
					printf ("Login: ");
				}
			}

			if (!user)
				printf ("Login failure.\n");
			attempts = 1;
		}

		read_bulletin_message();
		read_last_login();
		write_login_info();
		chdir(userinfo.home);
		execute_shell();
	}
}

/**********************************************************/

void add_failed_login (void)

{
	char fail[MAXPATH];
	FILE *fp;
	time_t t;

	strcpy (fail, USERCTRLDIR);
	strcat (fail, userinfo.userid);


	if (!access(mkFileName(fail), 0)) {
		strcpy (fail, USERFAILDIR);
		strcat (fail, userinfo.userid);
		chmod (mkFileName(fail), S_IREAD | S_IWRITE);
		time (&t);
		if ((fp=fopen(mkFileName(fail), "wt")) != NULL) {
			fprintf (fp,"%s\n", ctime(&t));
			fclose (fp);
		}
		chmod (mkFileName(fail), S_IREAD);
	}
}

/**********************************************************/

static int check_passwd (void)

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
		if (!strcmp (compare.userid, userinfo.userid)) {
			found = 1;
			if (!strcmp (compare.passwd, userinfo.passwd)) {
					okay = 1;
					memcpy ((struct passwd_s *) &userinfo, (struct passwd_s *) &compare,
							sizeof (struct passwd_s));
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

void execute_shell (void)

{
	char sh_nm[MAXPATH], cmd[MAXPATH];

	strcpy (sh_nm, STARTSHDIR);
	strcat (sh_nm, userinfo.shell);
	strcat (sh_nm, ".bat");

	if (access (mkFileName(sh_nm), 0)) {
		perror (userinfo.shell);
		return;
	}
	strcpy (cmd, mkFileName(sh_nm));
	system (cmd);
}

/**********************************************************/

int breakhandler (void)

{
  return (1);
}

/**********************************************************/

void perm_login (void)

{
	FILE *fp;
	struct stat statbuf;
	long msize, mtime;

	read_login_message();

	/*****
	Get Default UserID
	*****/
	if ((fp = fopen (mkFileName(DEFLTUSR), "r")) == NULL) {
		perror (mkFileName(DEFLTUSR));
		printf ("FATAL.  Cannot determine default user.\n");
		fix_default_user ();
		strcpy (userinfo.userid, "root");
	} else {
		fgets (userinfo.userid, 80, fp);
		userinfo.userid[strlen(userinfo.userid)-1] = '\0';
		stat (mkFileName(DEFLTUSR), &statbuf);
		fclose (fp);
		if ((fp = fopen (mkFileName(DEFLTUSRAUTH), "r")) == NULL) {
			perror (mkFileName(DEFLTUSRAUTH));
			printf ("FATAL.  Cannot determine default user.\n");
			fix_default_user();
			strcpy (userinfo.userid, "root");
		} else {
			fscanf (fp, "%ld\n%ld\n", &msize, &mtime);
			fclose (fp);
			if (msize != statbuf.st_size || mtime != statbuf.st_mtime) {
				printf ("FATAL.  Default user file corrupt.\n");
				fix_default_user();
				strcpy (userinfo.userid, "root");
			}
		}
	}

	/*****
	Get Default Shell
	*****/
	if ((fp = fopen (mkFileName(DEFLTSH), "r")) == NULL) {
		perror (mkFileName(DEFLTSH));
		printf ("FATAL.  Cannot find default shell.\n");
		printf ("Executing DOS Command Interpretur.\n");
			strcpy (userinfo.home, "\\");
			strcpy (userinfo.shell, "dos");
			write_login_info ();
			system ("command.com /p");
	}
	fgets (userinfo.shell, 80, fp);
	fclose (fp);
	userinfo.shell[strlen(userinfo.shell)-1] = '\0';

	if ((fp = fopen (mkFileName(DFLTHOME), "r")) == NULL) {
		perror (mkFileName(DFLTHOME));
		printf ("Fatal.  Cannot find default home directory.\n");
		printf ("The root directory will now be the home directory.\n");
		strcpy (userinfo.home, mkFileName("\\"));
	} else {
		fgets (userinfo.home, 80, fp);
		userinfo.home[strlen(userinfo.home)-1] = '\0';
		fclose (fp);
	}
	chdir (userinfo.home);

	write_login_info ();
	execute_shell ();
	printf ("FATAL.  Cannot find a shell to execute.\n");
	printf ("Exiting login process to system shell.\n");
	exit (1);
}

/*********************************************************/

void read_bulletin_message (void)

{
	char line[80];
	FILE *fp;

	if ((fp = fopen (mkFileName(BULLETIN), "r")) != NULL) {
		while (fgets(line,80,fp) != NULL) {
				printf ("%s", line);
		}
		fclose (fp);
	}
}

/*********************************************************/

void read_last_login (void)

{
  char llogin[MAXPATH];
  char out_str[80];
  FILE *fp;

	strcpy (llogin, USERCTRLDIR);
	strcat (llogin, userinfo.userid);

	printf ("LAST SUCCESSFUL LOGIN FOR %s WAS ", userinfo.userid);
	if ((fp = fopen(mkFileName(llogin), "r")) == NULL) {
		printf ("NEVER.\n");
	} else {
		fgets (out_str, 80, fp);
		printf ("ON %s", out_str);
		fclose (fp);
	}

	strcpy (llogin, USERFAILDIR);
	strcat (llogin, userinfo.userid);
	printf ("LAST UNSUCCESSFUL LOGIN FOR %s WAS ", userinfo.userid);
	if ((fp = fopen(mkFileName(llogin), "r")) == NULL) {
		printf ("NEVER.\n");
	} else {
		fgets (out_str, 80, fp);
		printf ("ON %s", out_str);
		fclose (fp);
	}
}

/*********************************************************/

void read_login_message (void)

{
	char line[80];
	FILE *fp;

	/*** Get Machine Name ***/
	if ((fp = fopen (mkFileName(SYSNAME), "r")) != NULL) {
		fgets (line, 80, fp);
		printf ("%s", line);
		fclose (fp);
	}

	/*** Get Login Message ***/
	if ((fp = fopen (mkFileName(MESSAGE), "r")) != NULL) {
		while (fgets(line,80,fp) != NULL) {
				printf ("%s", line);
		}
		fclose (fp);
	}
}

/*********************************************************/

void write_login_info (void)

{
	char llogin[MAXPATH];
	char env_home[MAXDIR];
	char env_user[MAXUSER];
	char env_shell[MAXPATH];
	char env_tz[MAXTZ];
	FILE *fp;
	time_t lt;
	struct stat statbuf;

	strcpy (llogin, USERCTRLDIR);
	strcat (llogin, userinfo.userid);

	chmod (mkFileName(USER), S_IREAD | S_IWRITE);
	chmod (mkFileName(llogin), S_IREAD | S_IWRITE);

	if ((fp = fopen (mkFileName(USER), "w")) == NULL) {
		perror (mkFileName(USER));
		printf ("Unable to validate current user.\n");
		chmod (mkFileName(USER), S_IREAD);
		chmod (mkFileName(llogin), S_IREAD);
	} else {
		fprintf (fp, "%s", userinfo.userid);
		fclose (fp);
		chmod (mkFileName(USER), S_IREAD);

		if ((fp = fopen (mkFileName(llogin), "wt")) == NULL) {
			printf ("Error writing login file.\n");
		} else {
			time(&lt);
			fprintf (fp, "%s", ctime(&lt));
			fprintf (fp, "%ld\n", lt);
			fclose (fp);
		}
		chmod (mkFileName(llogin), S_IREAD);
	}

	sprintf(env_home,"HOME=%s",userinfo.home);
	putenv(env_home);
	sprintf(env_user,"USER=%s",userinfo.userid);
	putenv(env_user);
	sprintf(env_shell,"SH=%s", userinfo.shell);
	putenv(env_shell);
	sprintf(env_tz, "TZ=%s", userinfo.tz);
	putenv(env_tz);

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

/*********************************************************/

void wait_failed_login (void)
{
	FILE *fp;
	int secs;

	if ((fp = fopen(mkFileName(LOGRETRY), "rt")) != NULL) {
		fscanf(fp,"%d",&secs);
		fclose(fp);
		if (secs > 0) {
			printf ("Invalid login.  Wait...\n");
			sleep(secs);
		}
	}
}

/*********************************************************/

static void init (void)
{
	FILE *fp;
	int done;
	time_t t;
	char comspec[MAXPATH], dospath[MAXPATH];
	char rc_file[MAXPATH], authchk[MAXPATH];
	struct ffblk f;

	/*****
	Store System Start Time
	*****/
	chmod (mkFileName(SYSSTART), S_IREAD | S_IWRITE);
	t = time (NULL);
	if ((fp = fopen (mkFileName(SYSSTART), "w")) != NULL) {
		fprintf (fp, "%s", ctime(&t));
		fprintf (fp, "%ld\n", t);
		fclose (fp);
		chmod (mkFileName(SYSSTART), S_IREAD);
	}

	/*****
	Set default path
	*****/
	strcpy (dospath, "PATH=");
	if ((fp = fopen (mkFileName(DEFAULTPATH), "r")) == NULL) {
		perror (mkFileName(DEFAULTPATH));
		sprintf (&dospath[strlen(dospath)], "C:\;C:\DOS;");
	} else {
		fscanf (fp, "%s\n", &dospath[strlen(dospath)]);
		fclose (fp);
	}
	putenv (dospath);

	/*****
	Comspec entry
	*****/
	strcpy (comspec, "COMSPEC=");
	if ((fp = fopen (mkFileName(COMSPEC), "r")) == NULL) {
		perror (mkFileName(COMSPEC));
		sprintf (&comspec[strlen(comspec)], "C:\\DOS\\COMMAND.COM");
	} else {
		fscanf (fp, "%s\n", &comspec[strlen(comspec)]);
		fclose (fp);
	}
	putenv (comspec);

 	/*****
	Check for chdir alias 
	*****/
	if (!access (mkFileName(CD), 0)) {
		system ("doskey cd=chd $1");
		system ("doskey chdir=chd $1");

		system ("doskey rd=rmd $1");
		system ("doskey rmdir=rmd $1");

		system ("doskey md=mkd $1");
		system ("doskey mkdir=mkd $1");
	}

	/*****
	Execute batch files in in startup directory
	*****/
	done = findfirst (mkFileName(STARTUPPROGS), &f, 0);
	while (!done) {
		strcpy (rc_file, STARTUPDIR);
		strcat (rc_file, f.ff_name);
		system (mkFileName(rc_file));
		done = findnext (&f);
	}

	/**** 
	Remove old userid
	*****/
	chmod (mkFileName(USER), S_IREAD | S_IWRITE);
	chmod (mkFileName(LOGINAUTH), S_IREAD | S_IWRITE);
	unlink (mkFileName(USER));
	unlink (mkFileName(LOGINAUTH));

	/*****
	Perform authchk
	*****/
	if (!access (mkFileName(AUTHCHK), 0)) {
		chmod (mkFileName(AUTHCHKLCK), S_IREAD | S_IWRITE);
		if ((fp = fopen (mkFileName(AUTHCHKLCK), "w")) == NULL) {
			perror (mkFileName(AUTHCHKLCK));
		} else {
			fclose (fp);
			chmod (mkFileName(AUTHCHKLCK), S_IREAD);
			sprintf(authchk, "\\etc\\authchk");
			system (mkFileName(authchk));
		}
	}
}
