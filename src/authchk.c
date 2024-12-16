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
#include <errno.h>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "utility.h"

int check_password (struct passwd_s *userinfo);
void do_authchk (void);
void set_default_user (void);
void fix_chmod (void);
void fix_superuser (void);
void fix_shutdown (void);

int prompt, sil;

int main (int argc, char *argv[])

{
	int pwdokay = 0;
	int attempts = 1;
	struct passwd_s userinfo;

	if (argc > 2) {
		printf ("Usage: %s [-s]\n", argv[0]);
		exit (1);
	}
	if (argc == 2) {
		if (strcmp (argv[1], "-s")) {
			printf ("Usage: %s [-s]\n", argv[0]);
			exit (1);
		}
		sil = 1;
	} else {
		sil = 0;
	}

	printf ("System Authentication Check Utility.\n");

	if (!access (mkFileName(AUTHCHKLCK), 0)) {
		chmod (mkFileName(AUTHCHKLCK), S_IREAD | S_IWRITE);
		unlink (mkFileName(AUTHCHKLCK));
		prompt = 0;
		sil = 1;
	} else {
		printf ("%s\n", COPYRIGHT);
		strcpy (userinfo.userid, "root");
		printf ("Enter Superuser Password: ");
		get_passwd(userinfo.passwd);
		encode (userinfo.passwd);

		/**** Verify old password ****/

		while ((!(pwdokay = check_password (&userinfo))) && attempts++ < 3) {
			printf ("\nSuperuser password incorrect.\n");
			printf ("Enter Superuser Password: ");
			get_passwd(userinfo.passwd);
			encode(userinfo.passwd);
		}

		printf ("\n");
		if (!pwdokay) {
			printf ("Password incorrect.\n");
			printf ("Authchk aborted!\n");
			exit (1);
		}
		prompt = 1;
	}

	do_authchk ();
	if (prompt && !sil) {
		printf ("System Authentication Complete.  Press any key.\n");
		getch();
	}
	return(0);
}

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

void do_authchk (void)

{
	FILE *fp;
	struct stat statbuf;
	long msize, mtime;

	/****
	Check IGNLOGIN
	****/

	if (!access (mkFileName(IGNLOGIN), 0)) {
		if (PermLoginOkay () == -1) {
			if (!sil)
				Perror ("");
		} else {
			if (!sil)
				printf ("Login security file okay.\n");
		}
	}

	/****
	Check current user file only if program is started from
	the command line.
	*****/
	if (prompt) {
		if ((fp = fopen(mkFileName(USER), "r")) == NULL) {
			if (!sil) {
				printf ("Cannot find user authentication file.\n");
				printf ("Setting user to default user.\n");
			}
			set_default_user ();

		} else {
			stat (mkFileName(USER), &statbuf);
			fclose (fp);

			chmod (mkFileName(LOGINAUTH), S_IREAD | S_IWRITE);
			if ((fp = fopen(mkFileName(LOGINAUTH), "r+")) == NULL) {

				if ((fp = fopen(mkFileName(LOGINAUTH), "w")) == NULL) {
					perror (mkFileName(LOGINAUTH));
					printf ("FATAL.  Unable to validate current user.\n");
					chmod (mkFileName(LOGINAUTH), S_IREAD);

				} else {
					if (!sil)
						printf ("User authentication file corrupt.\n");
					fprintf (fp, "%ld\n%ld\n", statbuf.st_size, statbuf.st_mtime);
					fclose (fp);
					chmod (mkFileName(LOGINAUTH), S_IREAD);
					if (!sil)
						printf ("User authentication file fixed.\n");
				}

			} else {
				fscanf (fp, "%ld\n%ld\n", &msize, &mtime);
				if (msize != statbuf.st_size || mtime != statbuf.st_mtime) {
					rewind (fp);
					if (!sil)
						printf ("User authentication file corrupt.\n");
					fprintf (fp, "%ld\n%ld\n", statbuf.st_size, statbuf.st_mtime);
					fclose (fp);
					chmod (mkFileName(LOGINAUTH), S_IREAD);
					if (!sil)
						printf ("User authentication file fixed.\n");
				} else {
					if (!sil)
						printf ("User authenication file okay.\n");
					fclose (fp);
					chmod (mkFileName(LOGINAUTH), S_IREAD);
				}
			}
		}
	}

	/*****
	Check /etc/su
	*****/

	if ((fp = fopen(mkFileName(SU), "r")) == NULL) {
		if (!sil) {
			printf ("Cannot find superuser authentication file.\n");
			printf ("Creating default superuser authentication file.\n");
		}
		fix_superuser ();

	} else {
		stat (mkFileName(SU), &statbuf);
		fclose (fp);

		chmod (mkFileName(SUAUTH), S_IREAD | S_IWRITE);
		if ((fp = fopen(mkFileName(SUAUTH), "r+")) == NULL) {

			if ((fp = fopen(mkFileName(SUAUTH), "w")) == NULL) {
				perror (mkFileName(SUAUTH));
				printf ("FATAL.  Unable to authenticate superuser file.\n");
				chmod (mkFileName(SUAUTH), S_IREAD);

			} else {
				if (!sil)
					printf ("Superuser authentication file corrupt.\n");
				fprintf (fp, "%ld\n%ld\n", statbuf.st_size, statbuf.st_mtime);
				fclose (fp);
				chmod (mkFileName(SUAUTH), S_IREAD);
				if (!sil) {
					printf ("Superuser authentication file fixed.\n");
					printf ("Check entries in /etc/su for tampering.\n");
				}
			}

		} else {
			fscanf (fp, "%ld\n%ld\n", &msize, &mtime);
			if (msize != statbuf.st_size || mtime != statbuf.st_mtime) {
				rewind (fp);
				if (!sil)
					printf ("Superuser authentication file corrupt.\n");
				fprintf (fp, "%ld\n%ld\n", statbuf.st_size, statbuf.st_mtime);
				fclose (fp);
				chmod (mkFileName(SUAUTH), S_IREAD);
				if (!sil) {
					printf ("Superuser authentication file fixed.\n");
					printf ("Check entries in /etc/su for tampering.\n");
				}
			} else {
				if (!sil)
					printf ("Superuser authenication file okay.\n");
				fclose (fp);
				chmod (mkFileName(SUAUTH), S_IREAD);
			}
		}
	}

	/*****
	Check system shutdown file.
	*****/

	if ((fp = fopen(mkFileName(SHUTDOWN), "r")) == NULL) {
		if (!sil) {
			printf ("Cannot find system shutdown file.\n");
			printf ("Creating default system shutdown file.\n");
		}
		fix_shutdown ();

	} else {
		stat (mkFileName(SHUTDOWN), &statbuf);
		fclose (fp);

		chmod (mkFileName(SHUTDOWNAUTH), S_IREAD | S_IWRITE);
		if ((fp = fopen(mkFileName(SHUTDOWNAUTH), "r+")) == NULL) {

			if ((fp = fopen(mkFileName(SHUTDOWNAUTH), "w")) == NULL) {
				perror (mkFileName(SHUTDOWNAUTH));
				printf ("FATAL.  Unable to authenticate system shutdown file.\n");
				chmod (mkFileName(SHUTDOWNAUTH), S_IREAD);

			} else {
				if (!sil)
					printf ("System shutdown file corrupt.\n");
				fprintf (fp, "%ld\n%ld\n", statbuf.st_size, statbuf.st_mtime);
				fclose (fp);
				chmod (mkFileName(SHUTDOWNAUTH), S_IREAD);
				if (!sil) {
					printf ("System shutdown file fixed.\n");
					printf ("Check /etc/shutdown for tampering.\n");
				}
			}

		} else {
			fscanf (fp, "%ld\n%ld\n", &msize, &mtime);
			if (msize != statbuf.st_size || mtime != statbuf.st_mtime) {
				rewind (fp);
				if (!sil)
					printf ("System shutdown file corrupt.\n");
				fprintf (fp, "%ld\n%ld\n", statbuf.st_size, statbuf.st_mtime);
				fclose (fp);
				chmod (mkFileName(SHUTDOWNAUTH), S_IREAD);
				if (!sil) {
					printf ("System shutdown file fixed.\n");
					printf ("Check /etc/shutdown for tampering.\n");
				}
			} else {
				if (!sil)
					printf ("System shutdown file okay.\n");
				fclose (fp);
				chmod (mkFileName(SHUTDOWNAUTH), S_IREAD);
			}
		}
	}

	/*****
	Check /etc/chmod
	*****/

	if ((fp = fopen(mkFileName(CHMOD), "r")) == NULL) {
		if (!sil) {
			printf ("Cannot find chmod authentication file.\n");
			printf ("Creating default chmod authentication file.\n");
		}
		fix_chmod ();

	} else {
		stat (mkFileName(CHMOD), &statbuf);
		fclose (fp);

		chmod (mkFileName(CHMODAUTH), S_IREAD | S_IWRITE);
		if ((fp = fopen(mkFileName(CHMODAUTH), "r+")) == NULL) {

			if ((fp = fopen(mkFileName(CHMODAUTH), "w")) == NULL) {
				perror (mkFileName(CHMODAUTH));
				printf ("FATAL.  Unable to authenticate chmod file.\n");
				chmod (mkFileName(CHMODAUTH), S_IREAD);

			} else {
				if (!sil)
					printf ("Chmod authentication file corrupt.\n");
				fprintf (fp, "%ld\n%ld\n", statbuf.st_size, statbuf.st_mtime);
				fclose (fp);
				chmod (mkFileName(CHMODAUTH), S_IREAD);
				if (!sil) {
					printf ("Chmod authentication file fixed.\n");
					printf ("Check entries in /etc/chmod for tampering.\n");
				}
			}

		} else {
			fscanf (fp, "%ld\n%ld\n", &msize, &mtime);
			if (msize != statbuf.st_size || mtime != statbuf.st_mtime) {
				rewind (fp);
				if (!sil)
					printf ("Chmod authentication file corrupt.\n");
				fprintf (fp, "%ld\n%ld\n", statbuf.st_size, statbuf.st_mtime);
				fclose (fp);
				chmod (mkFileName(CHMODAUTH), S_IREAD);
				if (!sil) {
					printf ("Chmod authentication file fixed.\n");
					printf ("Check entries in /etc/chmod for tampering.\n");
				}
			} else {
				if (!sil)
					printf ("Chmod authenication file okay.\n");
				fclose (fp);
				chmod (mkFileName(CHMODAUTH), S_IREAD);
			}
		}
	}


	/*****
	Check default user file.
	*****/

	if ((fp = fopen(mkFileName(DEFLTUSR), "r")) == NULL) {
		if (!sil) {
			printf ("Cannot find default user file.\n");
			printf ("Creating default user file.\n");
		}
		fix_default_user ();

	} else {
		stat (mkFileName(DEFLTUSR), &statbuf);
		fclose (fp);

		chmod (mkFileName(DEFLTUSRAUTH), S_IREAD | S_IWRITE);
		if ((fp = fopen(mkFileName(DEFLTUSRAUTH), "r+")) == NULL) {

			if ((fp = fopen(mkFileName(DEFLTUSRAUTH), "w")) == NULL) {
				perror (mkFileName(DEFLTUSRAUTH));
				printf ("FATAL.  Unable to authenticate defaul user file.\n");
				chmod (mkFileName(DEFLTUSRAUTH), S_IREAD);

			} else {
				if (!sil)
					printf ("Default user file corrupt.\n");
				fprintf (fp, "%ld\n%ld\n", statbuf.st_size, statbuf.st_mtime);
				fclose (fp);
				chmod (mkFileName(DEFLTUSRAUTH), S_IREAD);
				if (!sil) {
					printf ("Default user file fixed.\n");
					printf ("Check /etc/default/user for tampering.\n");
				}
			}

		} else {
			fscanf (fp, "%ld\n%ld\n", &msize, &mtime);
			if (msize != statbuf.st_size || mtime != statbuf.st_mtime) {
				rewind (fp);
				if (!sil)
					printf ("Default user file corrupt.\n");
				fprintf (fp, "%ld\n%ld\n", statbuf.st_size, statbuf.st_mtime);
				fclose (fp);
				chmod (mkFileName(DEFLTUSRAUTH), S_IREAD);
				if (!sil) {
					printf ("Default user file fixed.\n");
					printf ("Check /etc/default/user for tampering.\n");
				}
			} else {
				if (!sil)
					printf ("Default user file okay.\n");
				fclose (fp);
				chmod (mkFileName(DEFLTUSRAUTH), S_IREAD);
			}
		}
	}

	/*****
	Check license info.
	*****/

	if (CheckLicense() == -1) {
		if (!sil)
			printf ("System is not properly licensed.\n");
		if ((fp = fopen(mkFileName(LICENSE), "r")) == NULL) {
			if (!sil) {
				perror (mkFileName(LICENSE));
				printf ("Cannot license system.\n");
			}
		} else {
			stat (mkFileName(LICENSE), &statbuf);
			fclose (fp);
			chmod (mkFileName(LICENSEAUTH), S_IREAD | S_IWRITE);
			if ((fp = fopen(mkFileName(LICENSEAUTH), "w")) == NULL) {
				if (!sil) {
					perror (mkFileName(LICENSEAUTH));
					printf ("Cannot license system.\n");
					chmod (mkFileName(LICENSEAUTH), S_IREAD);
				}
			} else {
				fprintf (fp, "%ld\n%ld\n", statbuf.st_size, statbuf.st_mtime);
				fclose (fp);
				chmod (mkFileName(LICENSEAUTH), S_IREAD);
				if (!sil)
					printf ("Re-license system.\n");
			}
		}
	} else {
		if (!sil)
			printf ("System License Okay\n");
	}
}

void set_default_user (void)

{
	char userid[MAXUSER];
	char user_env[MAXUSER];
	FILE *fp;
	struct stat statbuf;

	if ((fp = fopen (mkFileName(DEFLTUSR), "r")) == NULL) {
		perror (mkFileName(DEFLTUSR));
		chmod (mkFileName(DEFLTUSR), S_IREAD | S_IWRITE);
		fix_default_user ();
		chmod (mkFileName(DEFLTUSR), S_IREAD);
		if ((fp = fopen (mkFileName(DEFLTUSR), "r")) == NULL) {
			if (!sil) {
				perror (mkFileName(DEFLTUSR));
				printf ("Userid \"root\" will now be the default user.\n");
				strcpy (userid, "root");
			}
		} else {
			fgets (userid, MAXUSER, fp);
			userid[strlen(userid)-1] = '\0';
			fclose (fp);
		}
	} else {
		fgets (userid, MAXUSER, fp);
		userid[strlen(userid)-1] = '\0';
		fclose (fp);
	}

	chmod (mkFileName(USER), S_IREAD | S_IWRITE);
	if ((fp = fopen(mkFileName(USER), "w")) == NULL) {
		perror (mkFileName(USER));
		printf ("FATAL.  System Error.  Cannot validate current user identification.\n");
		chmod (mkFileName(USER), S_IREAD);
	} else {
		fprintf (fp, "%s\n", userid);
		fclose (fp);
		chmod (mkFileName(USER), S_IREAD);
		if ((fp = fopen (mkFileName(USER), "r")) == NULL) {
			perror (mkFileName(USER));
			printf ("FATAL.  System Error.  Cannot validate current user identification.\n");
		} else {
			stat (mkFileName(USER), &statbuf);
			fclose (fp);
			chmod (mkFileName(LOGINAUTH), S_IREAD | S_IWRITE);
			if ((fp = fopen (mkFileName(LOGINAUTH), "w")) == NULL) {
				perror (mkFileName(LOGINAUTH));
				printf ("FATAL.  System Error.  Cannot validate current user identification.\n");
			} else {
				fprintf (fp, "%ld\n%ld\n", statbuf.st_size, statbuf.st_mtime);
				fclose (fp);
				chmod (mkFileName(LOGINAUTH), S_IREAD);
				sprintf (user_env, "USER=%s", userid);
				putenv (user_env);
			}
		}
	}
}

void fix_superuser (void)

{
	FILE *fp;
	struct stat statbuf;

	chmod (mkFileName(SU), S_IREAD | S_IWRITE);
	if ((fp = fopen (mkFileName(SU), "w")) == NULL) {
		perror (mkFileName(SU));
		printf ("FATAL.  System Error.  Cannot create /etc/su file.\n");
		chmod (mkFileName(SU), S_IREAD);
	} else {
		fprintf (fp, "root\n");
		fclose (fp);
		chmod (mkFileName(SU), S_IREAD);
		if ((fp = fopen(mkFileName(SU), "r")) == NULL) {
			perror (mkFileName(SU));
			printf ("FATAL.  System Error.  Cannot validate /etc/su file.\n");
		} else {
			stat (mkFileName(SU), &statbuf);
			fclose (fp);
			chmod (mkFileName(SUAUTH), S_IREAD | S_IWRITE);
			if ((fp = fopen (mkFileName(SUAUTH), "w")) == NULL) {
				perror (mkFileName(SUAUTH));
				printf ("FATAL.  System Error.  Cannot validate /etc/su file.\n");
				chmod (mkFileName(SUAUTH), S_IREAD);
			} else {
				fprintf (fp, "%ld\n%ld\n", statbuf.st_size, statbuf.st_mtime);
				fclose (fp);
				chmod (mkFileName(SUAUTH), S_IREAD);
			}
		}
	}
}

void fix_chmod (void)

{
	FILE *fp;
	struct stat statbuf;

	chmod (mkFileName(CHMOD), S_IREAD | S_IWRITE);
	if ((fp = fopen (mkFileName(CHMOD), "w")) == NULL) {
		perror (mkFileName(CHMOD));
		printf ("FATAL.  System Error.  Cannot create /etc/chmod file.\n");
		chmod (mkFileName(CHMOD), S_IREAD);
	} else {
		fprintf (fp, "root\n");
		fclose (fp);
		chmod (mkFileName(CHMOD), S_IREAD);
		if ((fp = fopen(mkFileName(CHMOD), "r")) == NULL) {
			perror (mkFileName(CHMOD));
			printf ("FATAL.  System Error.  Cannot validate /etc/chmod file.\n");
		} else {
			stat (mkFileName(CHMOD), &statbuf);
			fclose (fp);
			chmod (mkFileName(CHMODAUTH), S_IREAD | S_IWRITE);
			if ((fp = fopen (mkFileName(CHMODAUTH), "w")) == NULL) {
				perror (mkFileName(CHMODAUTH));
				printf ("FATAL.  System Error.  Cannot validate /etc/chmod file.\n");
				chmod (mkFileName(CHMODAUTH), S_IREAD);
			} else {
				fprintf (fp, "%ld\n%ld\n", statbuf.st_size, statbuf.st_mtime);
				fclose (fp);
				chmod (mkFileName(CHMODAUTH), S_IREAD);
			}
		}
	}
}

void fix_shutdown (void)

{
	FILE *fp;
	struct stat statbuf;

	chmod (mkFileName(SHUTDOWN), S_IREAD | S_IWRITE);
	if ((fp = fopen (mkFileName(SHUTDOWN), "w")) == NULL) {
		perror (mkFileName(SHUTDOWN));
		printf ("FATAL.  System Error.  Cannot create /etc/shutdown file.\n");
		chmod (mkFileName(SHUTDOWN), S_IREAD);
	} else {
		fclose (fp);
		chmod (mkFileName(SHUTDOWN), S_IREAD);
		if ((fp = fopen(mkFileName(SHUTDOWN), "r")) == NULL) {
			perror (mkFileName(SHUTDOWN));
			printf ("FATAL.  System Error.  Cannot validate /etc/shutdown file.\n");
		} else {
			stat (mkFileName(SHUTDOWN), &statbuf);
			fclose (fp);
			chmod (mkFileName(SHUTDOWNAUTH), S_IREAD | S_IWRITE);
			if ((fp = fopen (mkFileName(SHUTDOWNAUTH), "w")) == NULL) {
				perror (mkFileName(SHUTDOWNAUTH));
				printf ("FATAL.  System Error.  Cannot validate /etc/shutdown file.\n");
				chmod (mkFileName(SHUTDOWNAUTH), S_IREAD);
			} else {
				fprintf (fp, "%ld\n%ld\n", statbuf.st_size, statbuf.st_mtime);
				fclose (fp);
				chmod (mkFileName(SHUTDOWNAUTH), S_IREAD);
			}
		}
	}
}

