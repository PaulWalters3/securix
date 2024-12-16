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
#include <string.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include "utility.h"

int xtract_files (char from);
int make_filesystems (void);
int move_files (void);
void set_license (void);
void do_auth_chk (void);
void set_default_user (void);
void fix_chmod (void);
void fix_superuser (void);
void fix_shutdown (void);
void do_set_perms (void);
void set_sys_name (void);
void set_security (void);
void dos_dir(void);
void default_comspec(void);
void set_chdir(void);
void timezone_config(void);
void set_root_passwd(void);
void edit_batch (void);
void move_autoexec (void);
void endinstall (int status);

#define MAXBUF 8192

struct header {
	char filename[MAXPATH];
	long start;
	long size;
	long date;
	int  disk_no;
	char skip;
};

char *new_comspec;
char new_dir[MAXPATH];
char drive;
char security;

int main (int argc, char *argv[])
{
	char from, buf[MAXPATH], install_path[MAXPATH];
	FILE *fptr;

	if (!access ("c:\\securix", 0)) {
		printf ("SECURIX IS ALREADY INSTALLED.\n");
		endinstall (1);
	}

	if (argc != 1) {
		printf ("Usage: %s\n", argv[0]);
		endinstall (1);
	}

	from = toupper(argv[0][0]);

	clrscr ();
	printf ("SECURIX SETUP UTILITY.\n");
	printf ("%s\n", COPYRIGHT);
	printf ("=================================================\n");

	/*****
	Get installation drive information.
	*****/
	while (1) {
		printf ("Please enter the drive letter on which SECURIX is to be installed.\n");
		printf ("Installation Drive: ");
		gets (buf);
		fflush (stdin);
		drive = toupper(buf[0]);
		if (drive >= 'C' && drive <= 'Z')
			break;
	}
	while (1) {
		fflush (stdout);
		gotoxy (1,6);
		printf ("Please enter the directory to where SECURIX is to be installed.\n");
		printf ("Installation Directory (ie. \\SECURIX):                          \b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
		gets (buf);
		sprintf (install_path, "%c:%s", drive, buf);
		printf ("Install SECURIX to %s? ", install_path);
		if (toupper(getche()) != 'Y') {
			printf ("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
			printf ("                             ");
			printf ("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
			fflush (stdout);
			continue;
		}
		unix_to_dos_fn (install_path);
		if (install_path[strlen(install_path)-1] == '\\')
			install_path[strlen(install_path)-1] = '\0';
		if (!mkdir (install_path))
			break;
		printf ("Could not make directory %s.  Please try again.\n", install_path);
	}

	if ((fptr = fopen ("c:\\securix", "w")) == NULL) {
		printf ("Cannot open file c:\\securix to install SECURIX.\n");
		endinstall (1);
	}
	fprintf (fptr, "%s\n", install_path);
	fclose (fptr);
	_chmod ("c:\\securix", 1, FA_RDONLY | FA_HIDDEN | FA_SYSTEM);

	/*****
	Set current disk to the installation drive.
	*****/
	setdisk (drive-'A');
	if (getdisk() != (drive-'A')) {
		printf ("Cannot find the drive you specified as %c to install SECURIX to.\n", drive);
		endinstall (-1);
	}

	fflush (stdout);
	/*****
	Change the current directory to /tmp.
	*****/
	sprintf (buf, "%s\\tmp", install_path);
	while (chdir (buf) == -1) {
		if (mkdir (buf) == -1) {
			printf ("Cannot create directory %s.\n", buf);
			endinstall (-1);
		}
	}

	/*****
	Extract all files from installation disks.
	*****/
	if (xtract_files(from) == -1)
		endinstall (-1);

	if (make_filesystems() == -1)
		endinstall (-1);

	if (move_files() == -1)
		endinstall (-1);

	set_license();
	do_auth_chk();
	set_sys_name();
	set_security();
	dos_dir();
	default_comspec();
	set_chdir();
	timezone_config();
	set_root_passwd();
	edit_batch();
	move_autoexec();
	do_set_perms();
	printf ("SECURIX installation complete!\n");
	exit (0);
}

/**********************************************************/

int xtract_files (char from)
{
	int attr = FA_DIREC+FA_HIDDEN+FA_RDONLY+FA_LABEL+FA_SYSTEM+FA_ARCH;
	char look_for[MAXPATH];
	char drive[MAXDRIVE], directory[MAXDIR], file[MAXFILE], extension[MAXEXT];
	char buf[MAXBUF], prev[MAXPATH], fn[MAXPATH];
	FILE *SRC, *DEST;
	long pos, bytesread;
	int  i, num, amt, disk_no = 1;
	struct header info;
	struct ffblk ff;

	gotoxy (1, 10);
	printf ("Extracting files from device %c:          \n", from);
	printf ("===============================\n");

	sprintf (look_for, "%c:SECURX*.*", from);

	unix_to_dos_fn (look_for);
	fnsplit (look_for, drive, directory, file, extension);

	if (findfirst (look_for, &ff, attr)) {
		printf ("Could not find files to install.\n");
		return (-1);
	}
	sprintf (fn, "%s%s%s", drive, directory, ff.ff_name);

	if ((SRC = fopen (fn, "rb")) == NULL) {
		printf ("Could not install from device %c.\n", from);
		return (-1);
	}

	strcpy (prev, "");

	/****
	Get number of files on disk
	*****/
	fseek (SRC, -((long) sizeof(int)), SEEK_END);
	fread ((int *) &num, sizeof(int), 1, SRC);
	fseek (SRC, -(long) (sizeof(int) + (num * sizeof(struct header))), SEEK_END);

	for (i = 0; i < num; ++i) {
		fread ((struct header *) &info, sizeof(struct header), 1, SRC);
		pos = ftell (SRC);

		if (strcmp (prev, info.filename)) {
			strcpy (prev, info.filename);
			if ((DEST = fopen (info.filename, "wb")) == NULL) {
				Perror (info.filename);
				continue;
			}
		}
		fseek (SRC, info.start, SEEK_SET);
		gotoxy (1, 12);
		printf ("                                             \n");
		gotoxy (1, 12);
		printf ("%-25.25s\n", info.filename);

		for (bytesread=0L; bytesread < info.size; bytesread += (long)amt) {
			if ((bytesread+MAXBUF) > info.size) {
				amt = fread ((char *) buf, sizeof(char), (int)(info.size-bytesread), SRC);
			} else {
				amt = fread ((char *) buf, sizeof(char), MAXBUF, SRC);
			}
			fwrite ((char *) buf, sizeof(char), amt, DEST);
		}

		if (info.skip) {
			++disk_no;
			i = -1;
			fclose (SRC);
			sprintf (&fn[strlen(fn)-6], "%02d.fmt", disk_no);

			for (;;) {
				printf ("Please insert SECURIX Installation disk %02d and ", disk_no);
				printf ("press any key to continue.%c\n", 7);
				getch ();
				gotoxy (1,13);
				printf ("                                                                          ");

				if ((SRC = fopen (fn, "rb")) == NULL) {
					gotoxy (1,13);
					printf ("Incorrect volume in drive.%c\n", 7);
					sleep (2);
					gotoxy (1,13);
					printf ("                              \n");
					gotoxy (1,13);
					continue;
				}
				break;
			}

			/****
			Get number of files on disk
			*****/
			fseek (SRC, -((long) sizeof(int)), SEEK_END);
			fread ((int *) &num, sizeof(int), 1, SRC);
			fseek (SRC, -(long) (sizeof(int) + (num * sizeof(struct header))), SEEK_END);
		} else {
			fclose (DEST);
			fseek (SRC, pos, SEEK_SET);
		}
	}
	fclose (SRC);
	return (0);
}

/**********************************************************/

int make_filesystems (void)
{
	char dir_to_make[MAXPATH];
	FILE *fp;

	if ((fp = fopen (mkFileName("\\tmp\\filesys.dat"), "r")) == NULL) {
		printf ("Cannot make filesystem.\n");
		return (-1);
	}
	gotoxy (1, 10);
	printf ("Making filesystem.                        \n");
	printf ("------------------------------------------\n");

	while (fgets (dir_to_make, MAXPATH, fp) != NULL) {

		if (!dir_to_make[0])
			break;

		if (dir_to_make[strlen(dir_to_make)-1] == '\n')
			dir_to_make[strlen(dir_to_make)-1] = '\0';

		unix_to_dos_fn (dir_to_make);

		gotoxy (1, 12);
		printf ("                                                  \n");
		gotoxy (1, 12);
		printf ("%s", mkFileName(dir_to_make));

		if (mkdir (mkFileName(dir_to_make)) == -1) {
			printf ("Cannot create directory %s.\n", mkFileName(dir_to_make));
			continue;
		}
	}
	fclose (fp);
	return (0);
}

/**********************************************************/

int move_files (void)
{
	char *ptr;
	char file[MAXFILE], location[MAXPATH], buf[MAXPATH];
	FILE *fl;

	if ((fl = fopen (mkFileName("\\tmp\\files.dat"), "r")) == NULL) {
		printf ("Cannot find file list.\n");
		return (-1);
	}

	gotoxy (1, 10);
	printf ("Installing files.                         \n");
	printf ("------------------------------------------\n");

	while (fgets (buf, MAXPATH, fl) != NULL) {

		if (buf[strlen(buf)-1] == '\n')
			buf[strlen(buf)-1] = '\0';

		if (!buf[0])
			break;

		if ((ptr = strchr (buf, ' ')) == NULL)
			break;

		strcpy (location, &ptr[1]);
		ptr[0] = '\0';
		strcpy (file, buf);

		gotoxy (1, 12);
		printf ("                                                  ");
		gotoxy (1, 12);
		printf ("%s", mkFileName(location));

		if (file[0] && location[0]) {
			if (rename (file, mkFileName(location)) == -1) {
				gotoxy (1, 13);
				printf ("Cannot install %s to %s. Error=%d.%c\n", file, mkFileName(location), errno, 7);
				sleep (3);
				gotoxy (1, 13);
				printf ("                                                                            \n");
			} else {
				unlink (file);
			}
		}
	}
	fclose (fl);
	return (0);
}

/**********************************************************/

void set_license (void)

{
	char disclaimer[80];
	char sn[MAXLICENSE], lk[MAXLICENSE];
	char sysuser[MAXUSER], syscomp[MAXUSER];
	char *key;
	FILE *fp;
	int i=0;
	struct stat statbuf;

	clrscr ();

	if ((fp = fopen (mkFileName("\\usr\\doc\\disclaim"), "r")) != NULL) {
		while (fgets (disclaimer, 80, fp) != NULL) {
			printf ("%s", disclaimer);
			if (++i == 23) {
				printf ("--- PRESS ANY KEY TO CONTINUE ---");
				fflush (stdout);
				getch ();
				printf ("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
				printf ("                                 ");
				printf ("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
				fflush (stdout);
				i = 0;
			}
		}
		fclose (fp);
	} else {
		printf ("%s\n", PRODUCTNAME);
		printf ("%s\n", VERSION);
		printf ("%s\n\n", COPYRIGHT);
		printf ("**************************************************************\n");
		printf ("**  Please review the license agreement before proceeding.  **\n");
		printf ("**************************************************************\n");
	}
	printf ("--- PRESS ANY KEY TO CONTINUE ---");
	fflush (stdout);
	getch ();
	clrscr ();

	printf ("SERIAL NUMBER ACTIVATION:\n");
	printf ("You must now provide SECURIX with the serial number and activation key\n");
	printf ("supplied with your software.  Failure to do this will render your software\n");
	printf ("unusable.  The  serial number and activation key can be found on the SECURIX\n");
	printf ("registration card.\n\n");


	for (;;) {
		printf ("Enter SECURIX Serial Number: ");
		gets (sn);
		fflush (stdin);

		printf ("Enter SECURIX Activation Key: ");
		gets (lk);
		fflush (stdin);

		key = encode_license (sn);
		if (strcmp (lk, key)) {
			printf ("Invalid Serial Number or Activation Key.  Please try again.\n");
			continue;
		}
		break;
	}

	printf ("Enter name of person to whom software is licensed.\n");
	printf ("NAME: ");
	gets (sysuser);
	fflush (stdin);

	printf ("Enter company name (if applicable) to which software is licensed.\n");
	printf ("COMPANY: ");
	gets (syscomp);
	fflush (stdin);

	chmod (mkFileName(LICENSE), S_IREAD | S_IWRITE);
	if ((fp = fopen(mkFileName(LICENSE), "w")) == NULL) {
		perror (mkFileName(LICENSE));
		printf ("Cannot brand system.\n");
		return;
	}
	fprintf (fp, "%s\n%s\n%s\n", sn, sysuser, syscomp);
	fclose (fp);
	chmod (mkFileName(LICENSE), S_IREAD);

	if ((fp = fopen(mkFileName(LICENSE), "r")) == NULL) {
		perror (mkFileName(LICENSE));
		printf ("Cannot brand system.\n");
		return;
	}
	stat (mkFileName(LICENSE), &statbuf);
	fclose (fp);
	chmod (mkFileName(LICENSEAUTH), S_IREAD | S_IWRITE);

	if ((fp = fopen(mkFileName(LICENSEAUTH), "w")) == NULL) {
		perror (mkFileName(LICENSEAUTH));
		printf ("Cannot brand system.\n");
		return;
	}
	fprintf (fp, "%ld\n%ld\n%s\n", statbuf.st_size, statbuf.st_mtime, lk);
	fclose (fp);
	chmod (mkFileName(LICENSEAUTH), S_IREAD);
}

/**********************************************************/


void do_auth_chk (void)

{
	FILE *fp;
	struct stat statbuf;
	long msize, mtime;

	/****
	Check IGNLOGIN
	****/

	if (!access (mkFileName(IGNLOGIN), 0)) {
		if (PermLoginOkay () == -1)
			Perror ("");
	}

	set_default_user ();

	/*****
	Check /etc/su
	*****/

	if ((fp = fopen(mkFileName(SU), "r")) == NULL) {

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
				fprintf (fp, "%ld\n%ld\n", statbuf.st_size, statbuf.st_mtime);
				fclose (fp);
				chmod (mkFileName(SUAUTH), S_IREAD);
			}

		} else {
			fscanf (fp, "%ld\n%ld\n", &msize, &mtime);
			if (msize != statbuf.st_size || mtime != statbuf.st_mtime) {
				rewind (fp);
				fprintf (fp, "%ld\n%ld\n", statbuf.st_size, statbuf.st_mtime);
				fclose (fp);
				chmod (mkFileName(SUAUTH), S_IREAD);
			} else {
				fclose (fp);
				chmod (mkFileName(SUAUTH), S_IREAD);
			}
		}
	}

	/*****
	Check system shutdown file.
	*****/

	if ((fp = fopen(mkFileName(SHUTDOWN), "r")) == NULL) {

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
				fprintf (fp, "%ld\n%ld\n", statbuf.st_size, statbuf.st_mtime);
				fclose (fp);
				chmod (mkFileName(SHUTDOWNAUTH), S_IREAD);
			}

		} else {
			fscanf (fp, "%ld\n%ld\n", &msize, &mtime);
			if (msize != statbuf.st_size || mtime != statbuf.st_mtime) {
				rewind (fp);
				fprintf (fp, "%ld\n%ld\n", statbuf.st_size, statbuf.st_mtime);
				fclose (fp);
				chmod (mkFileName(SHUTDOWNAUTH), S_IREAD);
			} else {
				fclose (fp);
				chmod (mkFileName(SHUTDOWNAUTH), S_IREAD);
			}
		}
	}

	/*****
	Check /etc/chmod
	*****/

	if ((fp = fopen(mkFileName(CHMOD), "r")) == NULL) {

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
				fprintf (fp, "%ld\n%ld\n", statbuf.st_size, statbuf.st_mtime);
				fclose (fp);
				chmod (mkFileName(CHMODAUTH), S_IREAD);
			}

		} else {
			fscanf (fp, "%ld\n%ld\n", &msize, &mtime);
			if (msize != statbuf.st_size || mtime != statbuf.st_mtime) {
				rewind (fp);
				fprintf (fp, "%ld\n%ld\n", statbuf.st_size, statbuf.st_mtime);
				fclose (fp);
				chmod (mkFileName(CHMODAUTH), S_IREAD);
			} else {
				fclose (fp);
				chmod (mkFileName(CHMODAUTH), S_IREAD);
			}
		}
	}


	/*****
	Check default user file.
	*****/

	if ((fp = fopen(mkFileName(DEFLTUSR), "r")) == NULL) {

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
				fprintf (fp, "%ld\n%ld\n", statbuf.st_size, statbuf.st_mtime);
				fclose (fp);
				chmod (mkFileName(DEFLTUSRAUTH), S_IREAD);
			}

		} else {
			fscanf (fp, "%ld\n%ld\n", &msize, &mtime);
			if (msize != statbuf.st_size || mtime != statbuf.st_mtime) {
				rewind (fp);
				fprintf (fp, "%ld\n%ld\n", statbuf.st_size, statbuf.st_mtime);
				fclose (fp);
				chmod (mkFileName(DEFLTUSRAUTH), S_IREAD);
			} else {
				fclose (fp);
				chmod (mkFileName(DEFLTUSRAUTH), S_IREAD);
			}
		}
	}

	/*****
	Check license info.
	*****/

	if (CheckLicense() == -1) {
		printf ("System is not properly licensed.\n");
		if ((fp = fopen(mkFileName(LICENSE), "r")) == NULL) {
			printf ("System is not properly licensed.\n");
		} else {
			stat (mkFileName(LICENSE), &statbuf);
			fclose (fp);
			chmod (mkFileName(LICENSEAUTH), S_IREAD | S_IWRITE);
			if ((fp = fopen(mkFileName(LICENSEAUTH), "w")) == NULL) {
				printf ("Cannot license system.\n");
				chmod (mkFileName(LICENSEAUTH), S_IREAD);
			} else {
				fprintf (fp, "%ld\n%ld\n", statbuf.st_size, statbuf.st_mtime);
				fclose (fp);
				chmod (mkFileName(LICENSEAUTH), S_IREAD);
				printf ("Re-license system.\n");
			}
		}
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
			strcpy (userid, "root");
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

/**********************************************************/

void do_set_perms (void)
{
	char drive[MAXDRIVE], dirdir[MAXDIR], name[MAXFILE], ext[MAXEXT];
	char dest[MAXPATH], file[MAXPATH], instdrive[2], curdir[MAXPATH];
	int mode, done, offset, curdrive;

	FILE *fp;
	struct ffblk f;

	clrscr ();
	printf ("\nChecking file permissions...\n");

	if ((fp = fopen(mkFileName(FIXPERM), "r")) == NULL) {
		perror (mkFileName(FIXPERM));
		return;
	}

	curdrive = getdisk();
	strcpy (instdrive, mkFileName(""));
	setdisk (toupper(instdrive[0])-'A');
	getcwd (curdir, MAXPATH);

	while (!feof(fp)) {
		fscanf (fp, "%s %d", file, &mode);

		fnsplit (file, drive, dirdir, name, ext);
		sprintf (dest, "%s%s", drive, dirdir);
		if (strlen(dest) > 1) {
			dest[strlen(dest)-1] = '\0';
			offset = 1;
		} else {
			offset = 0;
		}
		if (strcmp (dest, "")) {
			chdir (dest);
			strcpy (file, &file[strlen(dest)+offset]);
		}
		done = findfirst (file, &f, FA_DIREC);
		while (!done) {
			/* write only */
			if (f.ff_attrib & FA_DIREC) {
				done = findnext(&f);
				continue;
			}
			if (mode == S_IWRITE) {
				if (access (mkFileName(f.ff_name), 2)) {
					chmod (mkFileName(f.ff_name), mode);
				}
			}
			if (mode == S_IREAD) {
				if (!access (mkFileName(f.ff_name), 2)) {
					chmod (mkFileName(f.ff_name), mode);
				}
			}
			if (mode == (S_IREAD | S_IWRITE)) {
				if (access (mkFileName(f.ff_name), 6)) {
					chmod (mkFileName(f.ff_name), mode);
				}
			}
			done = findnext (&f);
		}
	}
	fclose (fp);
	chdir (curdir);
	setdisk(curdrive);
}

/**********************************************************/

void set_sys_name (void)
{
	char new_name[MAXUSER];
	FILE *fp;

	clrscr ();
	printf ("Enter the name by which you wish this system to be known.\n");
	printf ("\nSYSTEM NAME: ");
	fflush (stdin);
	gets (new_name);
	chmod (mkFileName(SYSNAME), S_IREAD | S_IWRITE);
	if ((fp = fopen (mkFileName(SYSNAME), "w")) == NULL) {
		perror (mkFileName(SYSNAME));
		chmod (mkFileName(SYSNAME), S_IREAD);
		return;
	}
	if (!new_name[0])
		strcpy (new_name, "SECURIX");

	fprintf (fp, "%s\n", new_name);
	fclose (fp);
	chmod (mkFileName(SYSNAME), S_IREAD);
}

/**********************************************************/

void set_security (void)
{
	FILE *fp;
	struct stat statbuf;

	security=0;

	clrscr ();
	printf ("\nSYSTEM SECURITY.\n");
	printf ("If you wish to add login controls, every time a user wishes to access\n");
	printf ("this computer system, they will have to enter a unique user identification\n");
	printf ("and password.  ");

	if (!access (mkFileName(IGNLOGIN), 0)) {
		printf ("Currently system security is not enabled.\n");
		printf ("Do you wish to add login controls? ");
		if (toupper (getch()) == 'Y') {
			security=1;
			chmod (mkFileName(IGNLOGIN), S_IREAD | S_IWRITE);
			chmod (mkFileName(IGNLOGINAUTH), S_IREAD | S_IWRITE);
			if (unlink (mkFileName(IGNLOGIN))) {
				printf ("\nUnable to add login controls.\n");
				chmod (mkFileName(IGNLOGIN), S_IREAD);
				chmod (mkFileName(IGNLOGINAUTH), S_IREAD);
			}
			if (unlink (mkFileName(IGNLOGINAUTH))) {
				Perror (mkFileName(IGNLOGIN));
				chmod (mkFileName(IGNLOGINAUTH), S_IREAD);
			}
		}
	} else {
		printf ("Currently system security is enabled.\n");
		printf ("Do you wish to remove login controls? ");
		if (toupper (getch()) == 'Y') {
			if ((fp = fopen (mkFileName(IGNLOGIN), "w")) == NULL) {
				printf ("\nUnable to remove login controls.\n");
			}
			fclose (fp);
			if ((fp = fopen (mkFileName(IGNLOGIN), "r")) == NULL) {
				printf ("\nUnable to remove login controls.\n");
				unlink (mkFileName(IGNLOGIN));
			}
			stat (mkFileName(IGNLOGIN), &statbuf);
			fclose (fp);
			if ((fp = fopen (mkFileName(IGNLOGINAUTH), "w")) == NULL) {
				printf ("\nUnable to remove login controls.\n");
				unlink (mkFileName(IGNLOGIN));
			}
			fprintf (fp, "%ld\n%ld\n", statbuf.st_size, statbuf.st_mtime);
			fclose (fp);
			chmod (mkFileName(IGNLOGIN), S_IREAD);
			chmod (mkFileName(IGNLOGINAUTH), S_IREAD);
		}
		else
			security=1;
	}
}

/**********************************************************/

void default_comspec (void)
{
	FILE *fp;

	chmod (mkFileName(COMSPEC), S_IREAD | S_IWRITE);
	if ((fp = fopen (mkFileName(COMSPEC), "w")) == NULL) {
		chmod (mkFileName(COMSPEC), S_IREAD);
		perror (mkFileName(COMSPEC));
		return;
	}
	new_comspec = getenv ("COMSPEC");
	unix_to_dos_fn (new_comspec);
	fprintf (fp, "%s\n", new_comspec);
	fclose (fp);
	chmod (mkFileName(COMSPEC), S_IREAD);
}

/**********************************************************/

void dos_dir (void)
{
	FILE *fp;
	char curdir[MAXPATH], buf[MAXPATH];

	chmod (mkFileName(DOSPATH), S_IREAD | S_IWRITE);
	if ((fp = fopen (mkFileName(DOSPATH), "w")) == NULL) {
		chmod (mkFileName(DOSPATH), S_IREAD);
		perror (mkFileName(DOSPATH));
		return;
	}
	fflush (stdin);
	getcwd (curdir, MAXPATH);

	while (1) {
		printf ("\nEnter the full path name where DOS is located:\n");
		printf ("-> ");
		gets (new_dir);
		fflush (stdin);
		unix_to_dos_fn (new_dir);
		if (new_dir[strlen(new_dir)-1] == '\\')
			new_dir[strlen(new_dir)-1] = '\0';

		if (!chdir (new_dir)) {
			chdir (curdir);
			sprintf (buf, "%s\\command.com", new_dir);
			if (!access (buf, 0)) {
				fprintf (fp, "%s\n", new_dir);
				fclose (fp);
				chmod (mkFileName(DOSPATH), S_IREAD);
				break;
			}
		}
		printf ("Directory %s does not exist.\n", new_dir);
	}
}

/**********************************************************/

void set_chdir (void)
{
	FILE *fp;

	clrscr ();
	printf ("USING THE SECURIX DIRECTORY COMMANDS\n");
	printf ("------------------------------------\n");
	printf ("To use the SECURIX directory commands (chdir, mkdir, rmdir), the system\n");
	printf ("must be configured to use these commands by installing the chdir alias.\n");
	printf ("When the chdir alias is installed, all path names given to the directory\n");
	printf ("commands no longer have to contain backslashes (\\).  Forward slashes\n");
	printf ("may substituted in their place.\n");
	if (!access(mkFileName(CD), 0)) {
		printf ("Do you wish to remove the chdir alias? ");
		if (toupper(getch()) == 'Y') {
			chmod (mkFileName(CD), S_IWRITE | S_IREAD);
			if (unlink (mkFileName(CD))) {
				printf ("Unable to remove the chdir alias\n");
				chmod (mkFileName(CD), S_IREAD);
				return;
			}
		}
	} else {
		printf ("Do you wish to install the chdir alias? ");
		if (toupper(getch()) == 'Y') {
			if ((fp = fopen (mkFileName(CD), "w")) == NULL) {
				perror ("Unable to install chdir alias");
				return;
			}
			fclose (fp);
			chmod (mkFileName(CD), S_IREAD);
		}
	}
	printf ("\n");
}

/**********************************************************/

void timezone_config (void)

{
	char tz[MAXUSER];
	int tz_no = 1, sel = 0;
	FILE *fp;

	clrscr ();
	printf ("TIMEZONE INITIALIZATION\n");
	printf ("-----------------------\n");
	printf ("Please select the timezone from the following list that represents\n");
	printf ("your location.\n");

	if ((fp = fopen (mkFileName(TIMEZONES), "r")) == NULL) {
		perror (mkFileName(TIMEZONES));
		printf ("Setting timezone to EST\n");
		strcpy (tz, "EST5EDT");
		return;
	}
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
	chmod (mkFileName(TZ), S_IREAD | S_IWRITE);
	if ((fp = fopen (mkFileName(TZ), "w")) == NULL) {
		perror (mkFileName(TZ));
		chmod (mkFileName(TZ), S_IREAD);
		return;
	}
	fprintf (fp, "%s\n", tz);
	fclose (fp);
	chmod (mkFileName(TZ), S_IREAD);
}

/**********************************************************/

void set_root_passwd (void)
{
	char buf[MAXPATH];

	clrscr ();
	printf ("ROOT PASSWORD\n");
	printf ("-------------\n");
	printf ("You must now select a password for the root user account.\n");
	printf ("This password will be used with the root user identification.\n");
	printf ("The root user identification is the default user and has all\n");
	printf ("system administration priveledges.\n");
	printf ("\nThe root password can be changed by logging in as root and\n");
	printf ("typing passwd.\n");
	sprintf (buf, "%s root %s",  mkFileName("\\etc\\adduser"), ADDUSERKEY);
	system (buf);
}

/**********************************************************/

#define AUTOEXEC_PATH 13
#define CLEANUP_LINE 9
#define LOGIN_PATH 4
#define STARTUP_BAT
#define MAXLINE 80

void edit_batch (void)
{
	char line[MAXLINE];
	char bin[MAXPATH], dos[MAXPATH], usrbin[MAXPATH], buf[MAXPATH];
	int i;
	FILE *fp1, *fp2;

	strcpy (bin, mkFileName("\\bin"));
	strcpy (dos, new_dir);
	strcpy (usrbin, mkFileName("\\usr\\bin"));

	/*****
	Edit autoexec.bat file to contain proper path information.
	*****/
	if ((fp1 = fopen (mkFileName ("\\etc\\autoexec.bat"), "r")) == NULL) {
		Perror (mkFileName("\\etc\\autoexec.bat"));
	}
	else {
		if ((fp2 = fopen (mkFileName (TMP), "w")) != NULL) {
			for (i=1; i < AUTOEXEC_PATH; ++i) {
				fgets (line, MAXLINE, fp1);
				fputs (line, fp2);
			}
			fgets (line, MAXLINE, fp1);
			fgets (line, MAXLINE, fp1);
			fprintf (fp2, "PATH=%s;%s;%s;%%home%%\n", bin, dos, usrbin);
			fprintf (fp2, "SET COMSPEC=%s\n", new_comspec);
			fprintf (fp2, "SET TEMP=%s\n", mkFileName("\\tmp"));
			fprintf (fp2, ":LOOP\n");
			fprintf (fp2, "%s\n", mkFileName("\\bin\\login"));
			fprintf (fp2, "GOTO LOOP\n");
			fclose (fp1);
			fclose (fp2);
			_chmod (mkFileName("\\etc\\autoexec.bat"), 1, 0);
			unlink (mkFileName("\\etc\\autoexec.bat"));
			strcpy (buf, mkFileName("\\etc\\autoexec.bat"));
			rename (mkFileName(TMP), buf);
			_chmod (mkFileName("\\etc\\autoexec.bat"), 1, FA_RDONLY);
		} else
			Perror (mkFileName(TMP));
			fclose (fp1);
	}

	/*****
	Edit login.bat file to contain proper path information.
	*****/
	if ((fp1 = fopen (mkFileName ("\\etc\\newuser\\login.bat"), "r")) == NULL) {
		Perror (mkFileName("\\etc\\newuser\\login.bat"));
	}
	else {
		if ((fp2 = fopen (mkFileName (TMP), "w")) != NULL) {
			for (i=1; i < LOGIN_PATH; ++i) {
				fgets (line, MAXLINE, fp1);
				fputs (line, fp2);
			}
			fgets (line, MAXLINE, fp1);
			fprintf (fp2, "PATH=%s;%s;%s;%%home%%\n", bin, dos, usrbin);
			while (fgets (line, MAXLINE, fp1) != NULL) {
				fputs (line, fp2);
			}
			fclose (fp1);
			fclose (fp2);
			_chmod (mkFileName("\\etc\\newuser\\login.bat"), 1, 0);
			unlink (mkFileName("\\etc\\newuser\\login.bat"));
			strcpy (buf, mkFileName("\\etc\\newuser\\login.bat"));
			rename (mkFileName(TMP), buf);
			_chmod (mkFileName("\\etc\\newuser\\login.bat"), 1, FA_RDONLY);
		} else
			Perror (mkFileName(TMP));
			fclose (fp1);
	}

	/*****
	Edit \users\root\login.bat file to contain proper path information.
	*****/
	if ((fp1 = fopen (mkFileName ("\\users\\root\\login.bat"), "r")) == NULL) {
		Perror (mkFileName("\\users\\root\\login.bat"));
	}
	else {
		if ((fp2 = fopen (mkFileName (TMP), "w")) != NULL) {
			for (i=1; i < LOGIN_PATH; ++i) {
				fgets (line, MAXLINE, fp1);
				fputs (line, fp2);
			}
			fgets (line, MAXLINE, fp1);
			fprintf (fp2, "PATH=%s;%s;%s;%%home%%\n", bin, dos, usrbin);
			while (fgets (line, MAXLINE, fp1) != NULL) {
				fputs (line, fp2);
			}
			fclose (fp1);
			fclose (fp2);
			_chmod (mkFileName("\\users\\root\\login.bat"), 1, 0);
			unlink (mkFileName("\\users\\root\\login.bat"));
			strcpy (buf, mkFileName("\\users\\root\\login.bat"));
			rename (mkFileName(TMP), buf);
			_chmod (mkFileName("\\users\\root\\login.bat"), 1, FA_RDONLY);
		} else
			Perror (mkFileName(TMP));
			fclose (fp1);
	}


	/*****
	Modify /etc/default/path file
	*****/

	if ((fp1 = fopen (mkFileName("\\etc\\default\\path"), "w")) != NULL) {
		fprintf (fp1, "%s;%s;%s;%%home%%\n", bin, dos, usrbin);
		fclose (fp1);
		_chmod (mkFileName("\\etc\\default\\path"), 1, FA_RDONLY);
	} else
		Perror (mkFileName("\\etc\\default\\path"));


	/*****
	Modify /etc/startup.rc/cleanup.bat file
	*****/

	if ((fp1 = fopen (mkFileName("\\etc\\startup.rc\\cleanup.bat"), "r")) != NULL) {
		if ((fp2 = fopen (mkFileName(TMP), "w")) != NULL) {
			for (i=0; i < CLEANUP_LINE; ++i) {
				fgets (line, MAXLINE, fp1);
				fputs (line, fp2);
			}
			fprintf (fp2, "IF NOT EXIST %s GOTO LABEL\n\n", mkFileName("\\tmp\\*.*"));
			fprintf (fp2, "PATH=%s;%s;%s\n", bin, new_dir, usrbin);
			fprintf (fp2, "CD %s\n", mkFileName("\\tmp"));
			fprintf (fp2, "IF NOT EXIST %c:*.* GOTO LABEL\n", drive);
			fprintf (fp2, "ATTRIB -R %c:*.*\n", drive);
			fprintf (fp2, "RM -s %c:*.*\n", drive);
			fprintf (fp2, ":LABEL\n");
			fclose (fp1);
			fclose (fp2);
			_chmod (mkFileName("\\etc\\startup.rc\\cleanup.bat"), 1, 0);
			unlink (mkFileName("\\etc\\startup.rc\\cleanup.bat"));
			strcpy (buf, mkFileName("\\etc\\startup.rc\\cleanup.bat"));
			rename (mkFileName(TMP), buf);
			_chmod (mkFileName("\\etc\\startup.rc\\cleanup.bat"), 1, FA_RDONLY);
		}
		else {
			fclose (fp1);
			Perror (mkFileName(TMP));
		}
	}
	else
		Perror (mkFileName("\\etc\\startup.rc\\cleanup.bat"));

	/*****
	Modify dos.bat and dosshell.bat files.
	*****/
	unix_to_dos_fn (new_dir);
	if ((fp1 = fopen (mkFileName("\\etc\\startsh\\dos.bat"), "w")) != NULL) {
		fprintf (fp1, "@ECHO OFF\n");
		fprintf (fp1, "CALL LOGIN\n");
		fprintf (fp1, "%s\\command\n", new_dir);
		fclose (fp1);
		_chmod (mkFileName("\\etc\\startsh\\dos.bat"), 1, FA_RDONLY);
	}
	else
		Perror (mkFileName("\\etc\\startsh\\dos.bat"));

	if ((fp1 = fopen (mkFileName("\\etc\\startsh\\dosshell.bat"), "w")) != NULL) {
		fprintf (fp1, "@ECHO OFF\n");
		fprintf (fp1, "CALL LOGIN\n");
		fprintf (fp1, "%s\\command /C %s\\DOSSHELL\n", new_dir, new_dir);
		fclose (fp1);
		_chmod (mkFileName("\\etc\\startsh\\dosshell.bat"), 1, FA_RDONLY);
	}
	else
		Perror (mkFileName("\\etc\\startsh\\dosshell.bat"));

	/*****
	Modify /etc/mnt and /etc/shutdown files.
	*****/

	if ((fp1 = fopen (mkFileName("\\etc\\mnt"), "w")) != NULL) {
		fprintf (fp1, "%s\\join.exe\n", new_dir);
		fclose (fp1);
		_chmod (mkFileName("\\etc\\mnt"), 1, FA_RDONLY);
	}
	else
		Perror (mkFileName("\\etc\\mnt"));

	if ((fp1 = fopen (mkFileName("\\etc\\shutdown"), "w")) != NULL) {
		fprintf (fp1, "%s\n", mkFileName("pcpark.exe"));
		fclose (fp1);
		_chmod (mkFileName("\\etc\\shutdown"), 1, FA_RDONLY);
	}
	else
		Perror (mkFileName("\\etc\\mnt"));

}

/**********************************************************/

void move_autoexec(void)
{
	char buf[MAXPATH];

	clrscr();
	printf ("AUTOEXEC.BAT FILE INSTALLATION\n");
	printf ("==============================\n");
	if (security) {
		printf ("In order for SECURIX to complete installation, the autoexec.bat\n");
		printf ("file must be changed.  Your old autoexec.bat file will be moved\n");
		printf ("to the directory %sautoexec.bat.  Commands in your\n", mkFileName("\\etc\\startup.rc\\"));
		printf ("autoexec.bat file will execute as normal when the system is booted.\n");
		printf ("However, if this file starts a program that does not exit, such as\n");
		printf ("Windows, then SECURIX will not be able to limit access to the\n");
		printf ("computer system.  If you wish to have a favorite program execute\n");
		printf ("when you boot the system, you may do so by changing your login\n");
		printf ("shell to the program you wish to have executed.  See the chsh\n");
		printf ("manual page for more information.\n");
		printf ("\nPRESS ANY KEY.\n");
		getch ();
		_chmod ("c:\\autoexec.bat", 1, 0);
		sprintf (buf, "copy c:\\autoexec.bat %s > 0", mkFileName("\\etc\\startup.rc\\autoexec.bat"));
		system (buf);
		sprintf (buf, "copy %s c:\\autoexec.bat > 0", mkFileName("\\etc\\autoexec.bat"));
		system (buf);
		_chmod ("c:\\autoexec.bat", 1, FA_RDONLY);
	}
	else {
		printf ("In order for SECURIX to take control of your system at boot time,\n");
		printf ("your autoexec.bat file must be replaced with the SECURIX autoexec.bat\n");
		printf ("file.  If you wish to only install the SECURIX utilities, then you do\n");
		printf ("not have to perform this step.  If you would like to have SECURIX\n");
		printf ("control your system at a later time, all you would need to do is\n");
		printf ("replace your autoexec.bat file with the file %s.\n", mkFileName ("\\etc\\autoexec.bat"));
		printf ("If you install the SECURIX autoexec.bat file, your autoexec.bat file\n");
		printf ("will be moved to the directory %sautoexec.bat.\n", mkFileName("\\etc\\startup.rc\\"));
		printf ("Commands in your autoexec.bat file will execute as normal when the\n");
		printf ("system is booted.  However, if this file starts a program that doesn't\n");
		printf ("exit such as Windows, then SECURIX will not be able to limit access to\n");
		printf ("the computer system.  If you wish to have a favorite program execute\n");
		printf ("when you boot the system, you may do so by changing your login\n");
		printf ("shell to the program you wish to have executed.  See the chsh\n");
		printf ("manual page for more information.\n");
		printf ("Do you want to install the SECURIX autoexec.bat file? ");
		if (toupper(getche()) == 'Y') {
			_chmod ("c:\\autoexec.bat", 1, 0);
			sprintf (buf, "copy c:\\autoexec.bat %s", mkFileName("\\etc\\startup.rc\\autoexec.bat"));
			system (buf);
			sprintf (buf, "copy %s c:\\autoexec.bat", mkFileName("\\etc\\autoexec.bat"));
			system (buf);
			_chmod ("c:\\autoexec.bat", 1, FA_RDONLY);
		}
	}
}

/**********************************************************/

void endinstall (int status)
{
	if (status == 1)
		exit (1);
	if (status == -1) {
		printf ("SECURIX INSTALLATION ABORTED.\n");
		_chmod ("c:\\securix", 1, 0);
		unlink ("c:\\securix");
		exit (1);
	}
}

/**********************************************************/
