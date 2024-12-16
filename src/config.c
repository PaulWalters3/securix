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
#include <conio.h>
#include <ctype.h>
#include <dir.h>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys\stat.h>
#include "utility.h"

void auth_check (void);
void default_comspec (void);
void default_home (void);
void default_path (void);
void default_shell (void);
void default_user (void);
void dos_dir (void);
void file_attr (void);
void ign_login (void);
void leave_defaults (void);
void mnt_default (void);
void passwd_len (void);
void printer_setup (void);
void set_arch (void);
void set_chdir (void);
void super_user (void);
void system_name (void);
void time_delay (void);
void timezone_config (void);

int main (void)

{
	char ch[3];

	if (RootLogin (0) == -1)
		exit (1);

	while (1) {
		clrscr ();
		fflush (stdin);
		printf ("SECURIX System Configuration Utility.\n");
		printf ("%s\n", COPYRIGHT);
		printf ("-------------------------------------------------\n");
		printf ("1 ..... Arch Default Drive.\n");
		printf ("2 ..... Chdir Alias.\n");
		printf ("3 ..... Default Comspec Environment Variable.\n");
		printf ("4 ..... Default Home Directory.\n");
		printf ("5 ..... Default Login Shell.\n");
		printf ("6 ..... Default Login Userid.\n");
		printf ("7 ..... Default Path.\n");
		printf ("8 ..... DOS Directory.\n");
		printf ("9 ..... File Attribute Controls.\n");
		printf ("10 .... Leave Defaults.\n");
		printf ("11 .... Login Security.\n");
		printf ("12 .... Mount Default Command.\n");
		printf ("13 .... Password Length Restrictions.\n");
		printf ("14 .... Printer Setup.\n");
		printf ("15 .... Superuser Access Controls.\n");
		printf ("16 .... System Authentication.\n");
		printf ("17 .... System Name.\n");
		printf ("18 .... Time Delay after Invalid Login.\n");
		printf ("19 .... Timezone.\n");
		printf ("Q ..... Quit.\n");
		printf ("\nEnter Choice: ");
		gets (ch);
		fflush (stdin);
		if (toupper(ch[0]) == 'Q')
			return (0);

		switch (atoi(ch)) {
		case 1 : set_arch ();
				break;
		case 2 : set_chdir ();
				 break;
		case 3 : default_comspec ();
				 break;
		case 4 : default_home ();
				 break;
		case 5 : default_shell ();
				 break;
		case 6 : default_user ();
				 break;
		case 7 : default_path ();
				break;
		case 8 : dos_dir ();
				break;
		case 9 : file_attr ();
				break;
		case 10 : leave_defaults ();
				break;
		case 11 : ign_login ();
				 break;
		case 12 : mnt_default ();
				break;
		case 13 : passwd_len ();
				 break;
		case 14 : printer_setup ();
				break;
		case 15 : super_user ();
				 break;
		case 16 : auth_check ();
				 break;
		case 17 : system_name ();
				 break;
		case 18 : time_delay ();
				 break;
		case 19 : timezone_config ();
				 break;
		default : printf ("\nINVALID SELECTION\n");
				break;
		}
	}
}

void auth_check (void)
{
	FILE *fp;

	printf ("\nWhen system authentication checking is enabled, special system files\n");
	printf ("are checked at boot time for any corruption.\n");
	if (!access(mkFileName(AUTHCHK), 0)) {
		printf ("Do you wish to remove system authentication checking at boot? ");
		if (toupper(getch()) == 'Y') {
			chmod (mkFileName(AUTHCHK), S_IWRITE | S_IREAD);
			if (unlink (mkFileName(AUTHCHK))) {
				printf ("Unable to remove system authentication checking.\n");
				chmod (mkFileName(AUTHCHK), S_IREAD);
				return;
			}
		}
	} else {
		printf ("Do you wish to enable system authentication checking at boot? ");
		if (toupper(getch()) == 'Y') {
			if ((fp = fopen (mkFileName(AUTHCHK), "w")) == NULL) {
				perror ("Unable to system authentication checking.");
				return;
			}
			fclose (fp);
			chmod (mkFileName(AUTHCHK), S_IREAD);
		}
	}
	printf ("\n");
}

void default_comspec (void)
{
	char new_comspec[MAXPATH];
	FILE *fp;

	printf ("\nThe default comspec file the full path of the command interpreter.\n");
	printf ("Do you wish to change the default comspec entry? ");
	if (toupper(getch()) == 'Y') {
		chmod (mkFileName(COMSPEC), S_IREAD | S_IWRITE);
		if ((fp = fopen (mkFileName(COMSPEC), "w")) == NULL) {
			chmod (mkFileName(COMSPEC), S_IREAD);
			perror (mkFileName(COMSPEC));
			return;
		}
		fflush (stdin);
		printf ("\nEnter the full path name where the command.com interpreter is located:\n");
		printf ("-> ");
		gets (new_comspec);
		fflush (stdin);
		unix_to_dos_fn (new_comspec);
		fprintf (fp, "%s\n", new_comspec);
		fclose (fp);
		chmod (mkFileName(COMSPEC), S_IREAD);
	}
}

void default_home (void)
{
	char new_home[MAXPATH];
	FILE *fp;

	printf ("\nThe default home directory is the directory considered to be the\n");
	printf ("home directory of the default user, when login controls have been\n");
	printf ("removed from the system.\n");
	printf ("Do you wish to change the default home directory? ");
	if (toupper(getch()) == 'Y') {
		chmod (mkFileName(DFLTHOME), S_IREAD | S_IWRITE);
		if ((fp = fopen (mkFileName(DFLTHOME), "w")) == NULL) {
			chmod (mkFileName(DFLTHOME), S_IREAD);
			perror (mkFileName(DFLTHOME));
			return;
		}
		fflush (stdin);
		printf ("\nEnter the full path name of the new default home directory.\n");
		printf ("-> ");
		gets (new_home);
		unix_to_dos_fn(new_home);
		fflush (stdin);
		fprintf (fp, "%s\n", new_home);
		fclose (fp);
		chmod (mkFileName(DFLTHOME), S_IREAD);
	}
}

void default_path (void)
{
	char new_path[MAXPATH];
	FILE *fp;

	printf ("\nThe default path file contains the path that is to be used when the path\n");
	printf ("environment variable has not been set.\n");
	printf ("Do you wish to change the default path? ");
	if (toupper(getch()) == 'Y') {
		chmod (mkFileName(DEFAULTPATH), S_IREAD | S_IWRITE);
		if ((fp = fopen (mkFileName(DEFAULTPATH), "w")) == NULL) {
			chmod (mkFileName(DEFAULTPATH), S_IREAD);
			perror (mkFileName(DEFAULTPATH));
			return;
		}
		fflush (stdin);
		printf ("\nEnter the new default path:\n");
		printf ("-> ");
		gets (new_path);
		fflush (stdin);
		unix_to_dos_fn(new_path);
		fprintf (fp, "%s\n", new_path);
		fclose (fp);
		chmod (mkFileName(DEFAULTPATH), S_IREAD);
	}
}

void default_shell (void)
{
	char new_shell[MAXSHELL];
	FILE *fp, *sh;
	int sh_no=1, sel=0;

	printf ("\nThe default shell is the shell used on startup of the system when\n");
	printf ("when login controls have been removed.\n");
	printf ("Do you wish to change the default shell? ");
	if (toupper(getch()) == 'Y') {
		fflush (stdin);
		if ((sh = fopen (mkFileName(SHELLS), "r")) == NULL) {
			perror (mkFileName(SHELLS));
			return;
		}
		chmod (mkFileName(DEFLTSH), S_IREAD | S_IWRITE);
		if ((fp = fopen (mkFileName(DEFLTSH), "w")) == NULL) {
			chmod (mkFileName(DEFLTSH), S_IREAD);
			fclose (sh);
			perror (mkFileName(DEFLTSH));
			return;
		}
		printf ("\nAVAILABLE USER SHELLS:\n");
		printf ("======================\n");
		while ( fgets (new_shell, MAXSHELL, sh) != NULL )
			printf ("%d %s", sh_no++, new_shell);
		printf ("\n");
		while ( !sel ) {
			printf ("Enter the number of the shell to use: ");
			scanf ("%d", &sel);
			if (sel < 1 || sel >= sh_no)
				sel = 0;
		}
		rewind (sh);
		sh_no = 1;
		while ( fgets (new_shell, MAXSHELL, sh) != NULL && sh_no != sel)
			sh_no++;
		new_shell[strlen(new_shell)-1] = '\0';
		fclose (sh);
		fprintf (fp, "%s\n", new_shell);
		fclose (fp);
		chmod (mkFileName(DEFLTSH), S_IREAD);
		printf ("\n");
	}
}

void default_user (void)
{
	char new_user[MAXPATH];
	FILE *fp;
	struct stat statbuf;

	printf ("\nThe default userid is the userid used when login controls have\n");
	printf ("been removed removed from the system.\n");
	printf ("Do you wish to change the default userid? ");
	if (toupper(getch()) == 'Y') {
		chmod (mkFileName(DEFLTUSR), S_IREAD | S_IWRITE);
		if ((fp = fopen (mkFileName(DEFLTUSR), "w")) == NULL) {
			chmod (mkFileName(DEFLTUSR), S_IREAD);
			perror (mkFileName(DEFLTUSR));
			return;
		}
		printf ("\nEnter the new default userid.\n");
		printf ("-> ");
		fflush (stdin);
		gets (new_user);
		fprintf (fp, "%s\n", new_user);
		fclose (fp);
		if ((fp  = fopen(mkFileName(DEFLTUSR), "r")) == NULL) {
			chmod (mkFileName(DEFLTUSR), S_IREAD);
			perror (mkFileName(DEFLTUSR));
			return;
		}
		stat (mkFileName(DEFLTUSR), &statbuf);
		fclose (fp);
		chmod (mkFileName(DEFLTUSR), S_IREAD);
		chmod (mkFileName(DEFLTUSRAUTH), S_IREAD | S_IWRITE);
		if ((fp = fopen(mkFileName(DEFLTUSRAUTH), "w")) == NULL) {
			chmod (mkFileName(DEFLTUSRAUTH), S_IREAD);
			perror (mkFileName(DEFLTUSRAUTH));
			return;
		}
		fprintf (fp, "%ld\n%ld\n", statbuf.st_size, statbuf.st_mtime);
		fclose (fp);
		chmod (mkFileName(DEFLTUSRAUTH), S_IREAD);
	}
}

void dos_dir (void)
{
	char new_dir[MAXPATH];
	FILE *fp;

	printf ("\nThis changes the location of the default DOS directory.\n");
	printf ("Do you wish to change the default DOS directory? ");
	if (toupper(getch()) == 'Y') {
		chmod (mkFileName(DOSPATH), S_IREAD | S_IWRITE);
		if ((fp = fopen (mkFileName(DOSPATH), "w")) == NULL) {
			chmod (mkFileName(DOSPATH), S_IREAD);
			perror (mkFileName(DOSPATH));
			return;
		}
		fflush (stdin);
		printf ("\nEnter the full path name where DOS is located:\n");
		printf ("-> ");
		gets (new_dir);
		fflush (stdin);
		unix_to_dos_fn (new_dir);
		if (new_dir[strlen(new_dir)-1] != '\\')
			strcat (new_dir, "\\");
		fprintf (fp, "%s\n", new_dir);
		fclose (fp);
		chmod (mkFileName(DOSPATH), S_IREAD);
	}
}

void file_attr (void)
{
	FILE *fp, *tmp;
	char new_fa[MAXUSER], line[MAXUSER];
	struct stat statbuf;
	char tmp_fn[MAXPATH];

	printf ("\nFile attribute controls grant or revoke the privledge of setting\n");
	printf ("file attributes to specified userid's.\n");
	printf ("Do you wish to:\n");
	printf ("     (G)rant file attribute privledges\n");
	printf ("     (R)evoke file attribute privledges\n");
	printf ("     (Q)uit\n");
	switch (toupper(getch())) {
	case 'G' : printf ("Enter userid to have file attribute privledges: ");
			 fflush (stdin);
			 gets (new_fa);
			 chmod (mkFileName(CHMOD), S_IWRITE | S_IREAD);
			 if ((fp = fopen (mkFileName(CHMOD), "a+")) == NULL) {
				perror (mkFileName(CHMOD));
				chmod (mkFileName(CHMOD), S_IREAD);
				return;
			 }
			 fprintf (fp, "%s\n", new_fa);
			 fclose (fp);
			 chmod (mkFileName(CHMOD), S_IREAD);

			 /* GET STATUS FOR VERIFICATION */
			 if ((fp = fopen (mkFileName(CHMOD), "r")) == NULL) {
				perror (mkFileName(CHMOD));
				return;
			 }
			 stat (mkFileName(CHMOD), &statbuf);
			 fclose (fp);
			 chmod (mkFileName(CHMODAUTH), S_IREAD | S_IWRITE);

			 if ((fp = fopen (mkFileName(CHMODAUTH), "w")) == NULL) {
				perror (mkFileName(CHMODAUTH));
				chmod (mkFileName(CHMODAUTH), S_IREAD);
				return;
			 }
			 fprintf (fp, "%ld\n%ld\n", statbuf.st_size, statbuf.st_mtime);
			 fclose (fp);
			 chmod (mkFileName(CHMODAUTH), S_IREAD);
			 break;
	case 'R' : printf ("Enter userid to have file attribute priveledges REVOKED: ");
			 fflush (stdin);
			 gets (new_fa);
			 chmod (mkFileName(CHMOD), S_IWRITE | S_IREAD);
			 if ((fp = fopen (mkFileName(CHMOD), "r")) == NULL) {
				perror (mkFileName(CHMOD));
				chmod (mkFileName(CHMOD), S_IREAD);
				return;
			 }
			 if ((tmp = fopen (mkFileName(TMP), "w")) == NULL) {
				perror (mkFileName(TMP));
				fclose (fp);
				chmod (mkFileName(CHMOD), S_IREAD);
				return;
			 }
			 while (fgets (line, MAXUSER, fp) != NULL) {
				line[strlen(line)-1] = '\0';
				if (strcmp (line, new_fa)) {
					line[strlen(line)] = '\n';
					line[strlen(line)+1] = '\n';
					fputs (line, tmp);
				}
			 }
			 fclose (fp);
			 fclose (tmp);
			 unlink (mkFileName(CHMOD));
			 strcpy (tmp_fn, mkFileName(TMP));
			 rename (tmp_fn, mkFileName(CHMOD));
			 chmod (mkFileName(CHMOD), S_IREAD);

			 /* GET STATUS FOR VERIFICATION */
			 if ((fp = fopen (mkFileName(CHMOD), "r")) == NULL) {
				perror (mkFileName(CHMOD));
				return;
			 }
			 stat (mkFileName(CHMOD), &statbuf);
			 fclose (fp);

			 chmod (mkFileName(CHMODAUTH), S_IREAD | S_IWRITE);
			 if ((fp = fopen (mkFileName(CHMODAUTH), "w")) == NULL) {
				perror (mkFileName(CHMODAUTH));
				chmod (mkFileName(CHMODAUTH), S_IREAD);
				return;
			 }
			 fprintf (fp, "%ld\n%ld\n", statbuf.st_size, statbuf.st_mtime);
			 fclose (fp);
			 chmod (mkFileName(CHMODAUTH), S_IREAD);
			 break;
	default :  break;
	}
}

void ign_login (void)
{
	FILE *fp;
	struct stat statbuf;

	if (!access (mkFileName(IGNLOGIN), 0)) {
		printf ("\nDo you wish to add login controls? ");
		if (toupper (getch()) == 'Y') {
			chmod (mkFileName(IGNLOGIN), S_IREAD | S_IWRITE);
			chmod (mkFileName(IGNLOGINAUTH), S_IREAD | S_IWRITE);
			if (unlink (mkFileName(IGNLOGIN))) {
				printf ("\nUnable to add login controls.\n");
				chmod (mkFileName(IGNLOGIN), S_IREAD);
				chmod (mkFileName(IGNLOGINAUTH), S_IREAD);
				return;
			}
			if (unlink (mkFileName(IGNLOGINAUTH))) {
				Perror (mkFileName(IGNLOGIN));
				chmod (mkFileName(IGNLOGINAUTH), S_IREAD);
				return;
			}
		}
	} else {
		printf ("\nDo you wish to remove login controls? ");
		if (toupper (getch()) == 'Y') {
			if ((fp = fopen (mkFileName(IGNLOGIN), "w")) == NULL) {
				printf ("\nUnable to remove login controls.\n");
				return;
			}
			fclose (fp);
			if ((fp = fopen (mkFileName(IGNLOGIN), "r")) == NULL) {
				printf ("\nUnable to remove login controls.\n");
				unlink (mkFileName(IGNLOGIN));
				return;
			}
			stat (mkFileName(IGNLOGIN), &statbuf);
			fclose (fp);
			if ((fp = fopen (mkFileName(IGNLOGINAUTH), "w")) == NULL) {
				printf ("\nUnable to remove login controls.\n");
				unlink (mkFileName(IGNLOGIN));
				return;
			}
			fprintf (fp, "%ld\n%ld\n", statbuf.st_size, statbuf.st_mtime);
			fclose (fp);
			chmod (mkFileName(IGNLOGIN), S_IREAD);
			chmod (mkFileName(IGNLOGINAUTH), S_IREAD);
		}
	}
	printf ("\n");
}


void leave_defaults (void)

{
	char message[MAXPATH], rings[MAXEXT];
	FILE *fp;

	printf ("\nLeave is a program that will alert you to when it is a certain\n");
	printf ("time (just like an alarm clock).  Leave defaults include a message\n");
	printf ("that is displayed when it is time to leave, and the number of times\n");
	printf ("to ring a bell.\n");
	printf ("Do you wish to change the leave defaults? ");
	if (toupper(getch()) == 'Y') {
		printf ("\nEnter new leave message: ");
		gets (message);
		fflush (stdin);
		printf ("Enter number of bell rings: ");
		gets (rings);
		fflush (stdin);
		chmod (mkFileName(LEAVE), S_IREAD | S_IWRITE);
		if ((fp = fopen (mkFileName(LEAVE), "w")) == NULL) {
			perror (mkFileName(LEAVE));
			chmod (mkFileName(LEAVE), S_IREAD);
			printf ("Cannot change leave defaults.\n");
			return;
		}
		fprintf (fp, "%s\n%d\n", message, atoi(rings));
		fclose (fp);
		chmod (mkFileName(LEAVE), S_IREAD);
	}
}

/**********************************************************/

void mnt_default (void)
{
	char new_mnt[MAXPATH];
	FILE *fp;

	printf ("\nThis option changes the program used by the mnt program to\n");
	printf ("to perform join operations.  Normally this is the DOS join\n");
	printf ("program, however, the DOS subst program may also be used with\n");
	printf ("the mnt command.\n");
	printf ("Do you wish to chnage the DOS program used with the mnt command? ");
	if (toupper(getch()) == 'Y') {
		chmod (mkFileName(MNT), S_IREAD | S_IWRITE);
		if ((fp = fopen (mkFileName(MNT), "w")) == NULL) {
			chmod (mkFileName(MNT), S_IREAD);
			perror (mkFileName(MNT));
			return;
		}
		fflush (stdin);
		printf ("\nEnter the full path name of the the new program:\n");
		printf ("-> ");
		gets (new_mnt);
		fflush (stdin);
		unix_to_dos_fn (new_mnt);
		fprintf (fp, "%s\n", new_mnt);
		fclose (fp);
		chmod (mkFileName(MNT), S_IREAD);
	}
}

/**********************************************************/

void passwd_len (void)
{
	int len = -1;
	FILE *fp;

	printf ("\nSetting a password length restriction requires all new passwords to\n");
	printf ("be at least the number of characters given long.\n");
	printf ("Do you wish to change the password length restriction? ");
	if (toupper(getch()) == 'Y') {
		while (len < 0) {
			printf ("\nEnter the length of all new passwords: ");
			scanf ("%d", &len);
			if (len < 0)
				printf ("\nInvalid password length.\n");
		}
		chmod (mkFileName(PASSWDLEN), S_IREAD | S_IWRITE);
		if ((fp = fopen(mkFileName(PASSWDLEN), "w")) == NULL) {
			perror (mkFileName(PASSWDLEN));
			chmod (mkFileName(PASSWDLEN), S_IREAD);
			return;
		}
		fprintf (fp, "%d\n", len);
		fclose (fp);
		chmod (mkFileName(PASSWDLEN), S_IREAD);
	}
}

void printer_setup (void)
{
	char device[MAXFILE];
	FILE *fp;

	printf ("\nPrinter setup allows you to configure your default printer to.  In\n");
	printf ("the print setup you may change the default print device, the\n");
	printf ("default buffer size, and the default queue size.\n");
	printf ("Do you wish to change the printer setup? ");
	if (toupper(getch()) == 'Y') {
		printf ("\n");
		for (;;) {
			printf ("Enter new print device (ie. LPT1): ");
			gets (device);
			fflush (stdin);
			if (!strcmp(strupr(device), "LPT1"))
				break;
			if (!strcmp(strupr(device), "LPT2"))
				break;
			if (!strcmp(strupr(device), "LPT3"))
				break;
			if (!strcmp(strupr(device), "COM1"))
				break;
			if (!strcmp(strupr(device), "COM2"))
				break;
			if (!strcmp(strupr(device), "COM3"))
				break;
			if (!strcmp(strupr(device), "COM4"))
				break;
			printf ("Invalid device name.\n");
		}

		chmod (mkFileName(LPDEVICE), S_IREAD | S_IWRITE);
		if ((fp = fopen (mkFileName(LPDEVICE), "w")) == NULL) {
			perror (mkFileName(LPDEVICE));
			chmod (mkFileName(LPDEVICE), S_IREAD);
			printf ("Cannot change printer device defaults.\n");
		} else {
			fprintf (fp, "%s\n", device);
			fclose (fp);
			chmod (mkFileName(LPDEVICE), S_IREAD);
		}

		printf ("Enter size (in bytes) of print buffer: ");
		gets (device);
		fflush (stdin);
		chmod (mkFileName(LPBUFFER), S_IREAD | S_IWRITE);
		if ((fp = fopen (mkFileName(LPBUFFER), "w")) == NULL) {
			perror (mkFileName(LPBUFFER));
			chmod (mkFileName(LPBUFFER), S_IREAD);
			printf ("Cannot change print buffer defaults.\n");
		} else {
			fprintf (fp, "%d\n", atoi(device));
			fclose (fp);
			chmod (mkFileName(LPBUFFER), S_IREAD);
		}

		printf ("Enter maximum size (in # files) of print queue: ");
		gets (device);
		fflush (stdin);
		chmod (mkFileName(LPQUEUE), S_IREAD | S_IWRITE);
		if ((fp = fopen (mkFileName(LPQUEUE), "w")) == NULL) {
			perror (mkFileName(LPQUEUE));
			chmod (mkFileName(LPQUEUE), S_IREAD);
			printf ("Cannot change print queue defaults.\n");
		} else {
			fprintf (fp, "%d\n", atoi(device));
			fclose (fp);
			chmod (mkFileName(LPQUEUE), S_IREAD);
		}

	}

}

/**********************************************************/

void set_arch (void)
{
	char new_arch[MAXPATH];
	FILE *fp;

	printf ("\nThis option changes the default device used by the arch\n");
	printf ("program.  Do you wish to change the default arch device? ");
	if (toupper(getch()) == 'Y') {
		chmod (mkFileName(ARCHDEFAULT), S_IREAD | S_IWRITE);
		if ((fp = fopen (mkFileName(ARCHDEFAULT), "w")) == NULL) {
			chmod (mkFileName(ARCHDEFAULT), S_IREAD);
			perror (mkFileName(ARCHDEFAULT));
			return;
		}
		fflush (stdin);
		printf ("\nEnter the new device name: ");
		gets (new_arch);
		fflush (stdin);
		unix_to_dos_fn (new_arch);
		fprintf (fp, "%s\n", new_arch);
		fclose (fp);
		chmod (mkFileName(ARCHDEFAULT), S_IREAD);
	}
}

void set_chdir (void)
{
	FILE *fp;

	printf ("\nWhen the chdir alias is installed, all path names given to the directory\n");
	printf ("commands (chdir, mkdir, rmdir), no longer have to contain backslashes (\\). \n");
	printf ("Forward slashes may substituted in their place.\n");
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

void super_user (void)
{
	FILE *fp, *tmp;
	char new_su[MAXUSER], line[MAXUSER];
	struct stat statbuf;
	char tmp_fn[MAXPATH];

	printf ("\nSuperuser access controls grant or revoke superuser access privledges\n");
	printf ("to specified userid's.\n");
	printf ("Do you wish to:\n");
	printf ("     (G)rant superuser privledges\n");
	printf ("     (R)evoke superuser privledges\n");
	printf ("     (Q)uit\n");
	switch (toupper(getch())) {
	case 'G' : printf ("Enter userid to have superuser access: ");
			 fflush (stdin);
			 gets (new_su);
			 chmod (mkFileName(SU), S_IWRITE | S_IREAD);
			 chmod (mkFileName(SUAUTH), S_IWRITE | S_IREAD);
			 if ((fp = fopen (mkFileName(SU), "a+")) == NULL) {
				perror (mkFileName(SU));
				chmod (mkFileName(SU), S_IREAD);
				chmod (mkFileName(SUAUTH), S_IREAD);
				return;
			 }
			 fprintf (fp, "%s\n", new_su);
			 fclose (fp);
			 /* GET STATUS FOR VERIFICATION */
			 if ((fp = fopen (mkFileName(SU), "r")) == NULL) {
				perror (mkFileName(SU));
				chmod (mkFileName(SU), S_IREAD);
				chmod (mkFileName(SUAUTH), S_IREAD);
				return;
			 }
			 stat (mkFileName(SU), &statbuf);
			 fclose (fp);
			 chmod (mkFileName(SU), S_IREAD);
			 if ((fp = fopen (mkFileName(SUAUTH), "w")) == NULL) {
				perror (mkFileName(SUAUTH));
				chmod (mkFileName(SUAUTH), S_IREAD);
				return;
			 }
			 fprintf (fp, "%ld\n%ld\n", statbuf.st_size, statbuf.st_mtime);
			 fclose (fp);
			 chmod (mkFileName(SUAUTH), S_IREAD);
			 break;
	case 'R' : printf ("Enter userid to have superuser access REVOKED: ");
			 fflush (stdin);
			 gets (new_su);
			 chmod (mkFileName(SU), S_IWRITE | S_IREAD);
			 chmod (mkFileName(SUAUTH), S_IWRITE | S_IREAD);
			 if ((fp = fopen (mkFileName(SU), "r")) == NULL) {
				perror (mkFileName(SU));
				chmod (mkFileName(SU), S_IREAD);
				chmod (mkFileName(SUAUTH), S_IREAD);
				return;
			 }
			 if ((tmp = fopen (mkFileName(TMP), "w")) == NULL) {
				perror (mkFileName(TMP));
				fclose (fp);
				chmod (mkFileName(SU), S_IREAD);
				chmod (mkFileName(SUAUTH), S_IREAD);
				return;
			 }
			 while (fgets (line, MAXUSER, fp) != NULL) {
				line[strlen(line)-1] = '\0';
				if (strcmp (line, new_su)) {
					line[strlen(line)] = '\n';
					line[strlen(line)+1] = '\n';
					fputs (line, tmp);
				}
			 }
			 fclose (fp);
			 fclose (tmp);
			 unlink (mkFileName(SU));
			 strcpy (tmp_fn, mkFileName(TMP));
			 rename (tmp_fn, mkFileName(SU));

			 /* GET STATUS FOR VERIFICATION */
			 if ((fp = fopen (mkFileName(SU), "r")) == NULL) {
				perror (mkFileName(SU));
				chmod (mkFileName(SU), S_IREAD);
				chmod (mkFileName(SUAUTH), S_IREAD);
				return;
			 }
			 stat (mkFileName(SU), &statbuf);
			 fclose (fp);
			 chmod (mkFileName(SU), S_IREAD);
			 if ((fp = fopen (mkFileName(SUAUTH), "w")) == NULL) {
				perror (mkFileName(SUAUTH));
				chmod (mkFileName(SUAUTH), S_IREAD);
				return;
			 }
			 fprintf (fp, "%ld\n%ld\n", statbuf.st_size, statbuf.st_mtime);
			 fclose (fp);
			 chmod (mkFileName(SUAUTH), S_IREAD);
			 break;
	default :  break;
	}
}

void system_name (void)
{
	char new_name[MAXUSER];
	FILE *fp;

	printf ("\nEnter new system name: ");
	fflush (stdin);
	gets (new_name);
	chmod (mkFileName(SYSNAME), S_IREAD | S_IWRITE);
	if ((fp = fopen (mkFileName(SYSNAME), "w")) == NULL) {
		perror (mkFileName(SYSNAME));
		chmod (mkFileName(SYSNAME), S_IREAD);
		return;
	}
	fprintf (fp, "%s\n", new_name);
	fclose (fp);
	chmod (mkFileName(SYSNAME), S_IREAD);
}

void time_delay (void)
{
	int delay = -1;
	FILE *fp;

	printf ("\nTime delay after invalid login is the number of seconds the system\n");
	printf ("waits after an invalid login attempt before prompting for the login\n");
	printf ("again.\n");
	printf ("Do you wish to change the number of seconds between login attempts? ");
	if (toupper(getch()) == 'Y') {
		while (delay < 0) {
			printf ("\nEnter the number of seconds to delay: ");
			scanf ("%d", &delay);
			if (delay < 0)
				printf ("\nInvalid delay period.\n");
		}
		chmod (mkFileName(LOGRETRY), S_IREAD | S_IWRITE);
		if ((fp = fopen(mkFileName(LOGRETRY), "w")) == NULL) {
			perror (mkFileName(LOGRETRY));
			chmod (mkFileName(LOGRETRY), S_IREAD);
			return;
		}
		fprintf (fp, "%d\n", delay);
		fclose (fp);
		chmod (mkFileName(LOGRETRY), S_IREAD);
	}
}

void timezone_config (void)

{
	char tz[MAXUSER];
	int tz_no = 1, sel = 0;
	FILE *fp;

	printf ("\nThe timezone file contains the timezone information of where the\n");
	printf ("system is located.\n");
	printf ("Do you wish to change the timezone format? ");
	if (toupper(getch()) != 'Y')
		return;

	if ((fp = fopen (mkFileName(TIMEZONES), "r")) == NULL) {
		perror (mkFileName(TIMEZONES));
		printf ("Setting timezone to EST\n");
		strcpy (tz, "EST5EDT");
		return;
	}
	printf ("\nAVAILABLE TIMEZONES:\n");
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

