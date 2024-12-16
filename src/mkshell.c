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
#include <dos.h>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "utility.h"

void install_shell(void);
void remove_shell(void);
void get_shell (char shell[MAXSHELL]);
void remove_from_file (char shell[MAXSHELL]);
void	remove_from_newuser (char shell[MAXSHELL]);
void	remove_from_startup (char shell[MAXSHELL]);

/**********************************************************/

int main (void)
{
	int done=0;

	if (CheckLicense () == -1) {
		Perror ("");
		exit (1);
	}

	printf ("mkshell\n%s\n", COPYRIGHT);
	printf ("=====================================\n");

	if (RootLogin(0) == -1)
		exit (1);

	while (!done) {
		printf ("(1) -- Install a new user shell.\n");
		printf ("(2) -- Remove a shell.\n");
		printf ("(Q) -- Quit.\n");
		switch (getch()) {
		case '1' : install_shell();
				 break;
		case '2' : remove_shell();
				 break;
		case 'q' :
		case 'Q' : done = 1;
				 break;
		default  : printf ("Invalid Selection.\n");
				 break;
		}
		printf ("\n");
	}
	return(0);
}

/**********************************************************/

void install_shell (void)
{
	char shell[MAXSHELL], cmdline[MAXPATH], cmdpath[MAXPATH], fn[MAXPATH];
	FILE *fp;
	int i;

	fflush (stdin);
	printf ("\nShell installation may be terminated at anytime by pressing\n");
	printf ("the enter key at the beginning of a prompt.\n\n");

	printf ("Enter the name of the new shell to install: ");
	gets (shell);
	fflush (stdin);
	if (!shell[0])
		return;

	printf ("Enter the command line to start the shell: ");
	gets (cmdline);
	fflush (stdin);
	if (!cmdline[0])
		return;

	printf ("Enter the directory where the shell is found: ");
	gets (cmdpath);
	fflush (stdin);
	if (!cmdpath[0])
		return;
	for (i=0; i<strlen(cmdpath); ++i) {
		if (cmdpath[i] == '/')
			cmdpath[i] = '\\';
	}

	sprintf (fn, "%s\\%s", cmdpath, cmdline);
	if (access (fn, 0)) {
		perror (fn);
		printf ("Cannot add shell %s.\n", shell);
		return;
	}

	sprintf (fn, "%s%s.bat", STARTSHDIR, shell);

	if ((fp = fopen(mkFileName(fn), "w")) == NULL) {
		perror (mkFileName(fn));
		printf ("Cannot add shell %s.\n", shell);
		return;
	}

	fprintf (fp, "@ECHO OFF\n");
	fprintf (fp, "CALL LOGIN\n");
	fprintf (fp, "PATH=%s;\%PATH\%\n", strupr(cmdpath));
	fprintf (fp, "COMMAND /C %s\n", cmdline);
	fclose (fp);
	chmod (mkFileName(fn), S_IREAD);

	chmod (mkFileName(SHELLS), S_IREAD | S_IWRITE);
	if ((fp = fopen(mkFileName(SHELLS), "a")) == NULL) {
		perror (mkFileName(SHELLS));
		printf ("Cannot add shell %s.\n", shell);
		chmod (mkFileName(SHELLS), S_IREAD);
	}
	fprintf (fp, "%s\n", shell);
	fclose (fp);
	chmod (mkFileName(SHELLS), S_IREAD);

	sprintf (fn, "%s%s", NEWUSERDIR, shell);
	mkdir (mkFileName(fn));

	printf ("\nShell %s successfully added to system.\n\n", strupr(shell));
	printf ("See the mkshell manual page for more information about adding a shell\n");
	printf ("to the system.  If there are any special files unique to each user,\n");
	printf ("default copies of these files should be placed in the directory called\n");
	printf ("%s%s.  These default files are copied to a user's\n", NEWUSERDIR, strupr(shell));
	printf ("home directory every time a user is added to the system or a user\n");
	printf ("changes their shell.  Also, the script %s%s.bat should be\n", STARTSHDIR, strupr(shell));
	printf ("modified so that these user files are copied from the users home directory\n");
	printf ("to the home directory of the shell, when a user logs into the system.  Of\n");
	printf ("course, should a user modify any of these files, the same modifications\n");
	printf ("must be made to the same files in their home directory.  In the startup\n");
	printf ("script these files should be copied before executing the shell.\n");
	printf ("\nPress any key to continue.\n");
	getch();
}

/**********************************************************/

void remove_shell (void)
{
	char shell[MAXSHELL];

	fflush (stdin);
	get_shell (shell);
	remove_from_file (shell);
	remove_from_newuser (shell);
	remove_from_startup (shell);
	printf ("Shell %s removed from system.\n", shell);
	fflush (stdin);
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
	printf ("\nCHOOSE A SHELL TO REMOVE:\n");
	printf ("========================\n");
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

void remove_from_file (char shell[MAXSHELL])
{
	char buf[MAXSHELL], fn[MAXPATH];
	FILE *fp1, *fp2;

	chmod (mkFileName(SHELLS), S_IREAD | S_IWRITE);
	if ((fp1 = fopen (mkFileName(SHELLS), "r")) == NULL) {
		perror (mkFileName(SHELLS));
		exit (1);
	}

	chmod (mkFileName(TMP), S_IREAD | S_IWRITE);
	if ((fp2 = fopen (mkFileName(TMP), "w")) == NULL) {
		perror (mkFileName(TMP));
		fclose (fp1);
		chmod (mkFileName(SHELLS), S_IREAD);
		exit (1);
	}

	while (fgets (buf, MAXSHELL, fp1) != NULL) {
		if (buf[strlen(buf)-1] == '\n')
			buf[strlen(buf)-1] = '\0';
		if (!strcmp(buf, shell))
			continue;
		fprintf (fp2, "%s\n", buf);
	}
	fclose (fp1);
	fclose (fp2);
	remove (mkFileName(SHELLS));
	strcpy (fn, mkFileName(SHELLS));
	rename (mkFileName(TMP), fn);
	chmod (fn, S_IREAD);
}

/**********************************************************/

void remove_from_newuser (char shell[MAXSHELL])
{
	int cd, done;
	char fn[MAXPATH], cdir[MAXPATH], indrive[3];
	struct ffblk f;

	cd = getdisk();
	strcpy (indrive, mkFileName(""));
	setdisk (toupper(indrive[0])-'A');

	getcwd (cdir, MAXPATH);
	sprintf (fn, "%s%s", NEWUSERDIR, shell);
	if (chdir (fn)) {
		perror (fn);
		return;
	}

	done = findfirst ("*.*", &f, 0);
	while (!done) {
		chmod (f.ff_name, S_IREAD | S_IWRITE);
		if (unlink (f.ff_name))
			perror (f.ff_name);
		done = findnext (&f);
	}
	if (chdir (".."))
		perror ("..");
	if (rmdir (shell))
		perror (shell);
	if (chdir (cdir))
		perror (cdir);
	setdisk (cd);
}

/**********************************************************/

void remove_from_startup (char shell[MAXSHELL])
{
	char fn[MAXPATH];

	sprintf (fn, "%s%s.bat", STARTSHDIR, shell);
	chmod (mkFileName(fn), S_IREAD | S_IWRITE);
	if (unlink(mkFileName(fn)))
		perror (mkFileName(fn));
}

/**********************************************************/
