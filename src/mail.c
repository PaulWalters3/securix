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
#include <errno.h>
#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys\stat.h>
#include <time.h>
#include "utility.h"

#define BUFSIZE 2048
#define MAXLINE 160
#define MAXUSER 69

#define SEND 'S'
#define READ 'R'
#define DELETE 'D'
#define LIST 'L'
#define EXIT 'Q'

/****** FUNCTIONS ***************************************/

void do_choice (char choice);
void do_menu (char *choice);
void delete_message (void);
void read_message (void);
void send_message (char *userid, char *subject, char *file_to_send);
int list_messages (void);
void check_mail (void);

/****** VARIABLES ***************************************/

char cur_user[MAXUSER];
char filename[MAXPATH];
int last_message;

/****** FUNCTION: main **********************************/

int main (int argc, char *argv[])

{
	char choice;

	last_message = 0;

	if (DetermineUserID (cur_user) == -1)
		exit (1);

	if ((argc == 2) && (!strcmp(argv[1], "-e"))) {
		check_mail ();
		exit (0);
	}

	if (argc > 2 && argc < 5) {
		send_message (argv[1], argv[2], argv[3]);
		exit (0);
	}
	else if (argc == 2) {
		send_message (argv[1], "", "");
		exit (0);
	}
	if (argc > 1) {
		printf ("Usage: %s [-e]|[userid[subject file_to_read]]", argv[0]);
		exit (1);
	}

	printf ("Mail\n");
	printf ("%s\n", COPYRIGHT);

	strcpy (filename, MAILDIR);
	strcat (filename, cur_user);
	strcpy (filename, mkFileName(filename));

	while (1) {
		do_menu (&choice);
		if (toupper (choice) == EXIT)
			break;
		else
			do_choice (choice);
	}
	return (0);
}

/****** FUNCTION: delete_message *************************/

void delete_message (void)

{
	char line[MAXLINE];
	FILE *fp1, *fp2;
	int del=0, last, i=1;

	if (list_messages())
		return;

	while (!del) {
		printf ("\nMESSAGE # TO DELETE: ");
		scanf ("%d", &del);
		if (del < 1 || del > last_message) {
			printf ("INVALID MESSAGE.\n");
			del = 0;
		}
	}

	chmod (filename, S_IREAD | S_IWRITE);
	if ((fp1 = fopen (filename, "r")) == NULL) {
		perror (filename);
		chmod (filename, S_IREAD);
		return;
	}
	if ((fp2 = fopen (mkFileName(MAILTMP), "w")) == NULL) {
		perror (MAILTMP);
		fclose (fp1);
		chmod (filename, S_IREAD);
		return;
	}

	fscanf (fp1, "%d\n", &last);
	last--;
	fprintf (fp2, "%d\n", last);

	fgets (line, MAXLINE, fp1);
	line[strlen(line)-1]='\0';
	while (i < del) {
		while (strcmp (line, "[eom]")) {
			fprintf (fp2, "%s\n", line);
			fgets (line, MAXLINE, fp1);
			line[strlen(line)-1]='\0';
		}
		i++;
		fprintf (fp2, "%s\n", line);
		fgets (line, MAXLINE, fp1);
		line[strlen(line)-1]='\0';
	}

	while (strcmp (line, "[eom]")) {
		fgets (line, MAXLINE, fp1);
		line[strlen(line)-1] = '\0';
	}

	while (fgets (line, MAXLINE, fp1) != NULL)
		fputs (line, fp2);
	fclose (fp1);
	fclose (fp2);
	unlink (filename);
	rename (mkFileName(MAILTMP), filename);
	chmod (filename, S_IREAD);
	printf ("\nMESSAGE DELETED.\n");
}

/****** FUNCTION: do_choice ******************************/

void do_choice (char choice)

{
	char user[MAXUSER] = "";
	char subject[MAXUSER] = "";
	char file_to_send[MAXPATH] = "";

	switch (toupper(choice)) {
		case SEND   : send_message (user, subject, file_to_send);
				    break;
		case READ   : read_message ();
				    break;
		case DELETE : delete_message ();
				    break;
		case LIST   : list_messages ();
		case EXIT   : break;
		default     : printf ("INVALID CHOICE.\n");
				    break;
	}
}
 
/****** FUNCTION: do_menu ********************************/

void do_menu (char *choice)

{
   printf ("\n(S)END MESSAGES\n");
   printf ("(R)EAD MESSAGES\n");
   printf ("(D)ELETE MESSAGES\n");
   printf ("(L)IST MESSAGES\n");
   printf ("(Q)UIT MAIL\n");
   *choice = getch();
}

/****** FUNCTION: list_messages **************************/

int	list_messages (void)
{
	char line[MAXLINE];
	FILE *fp;
	int i;

	if ((fp = fopen (filename, "r")) == NULL) {
		printf ("No messages.\n");
		return (1);
	}
	fscanf (fp, "%d\n", &last_message);
	if (last_message == 0) {
		printf ("No messages.\n");
		fclose (fp);
		return (1);
	}

	i = 0;
	printf ("Message    From         Subject                Date\n");
	printf ("------------------------------------------------------------------------\n");
	while (fgets (line, MAXLINE, fp) != NULL) {
		line[strlen(line)-1] = '\0';
		if (!strcmp (line, "[eom]") || i == 0) {
			if (fgets (line, MAXLINE, fp) != NULL) {
				printf ("%-7d    ", ++i);
				line[strlen(line)-1] = '\0';

				if (i == 1) {
					printf ("%-13.13s", &line[10]);
					fgets (line, MAXLINE, fp);
					line[strlen(line)-1] = '\0';
					printf ("%-23.23s", &line[10]);
					fgets (line, MAXLINE, fp);
					line[strlen(line)-1] = '\0';
					printf ("%-s\n", &line[10]);
				} else {
					fgets (line, MAXLINE, fp);
					line[strlen(line)-1] = '\0';
					printf ("%-13.13s", &line[10]);
					fgets (line, MAXLINE, fp);
					line[strlen(line)-1] = '\0';
					printf ("%-23.23s", &line[10]);
					fgets (line, MAXLINE, fp);
					line[strlen(line)-1] = '\0';
					printf ("%-s\n", &line[10]);
				}
			}
		}
	}
	fclose (fp);
	return (0);
}

/****** FUNCTION: read_message ***************************/

void read_message (void)

{
	char line[MAXLINE];
	FILE *fp;
	int i = 1;
	int message_number;

	if (list_messages())
		return;
	else if ((fp = fopen(filename,"r")) == NULL ) {
		printf ("No messages to read.\n");
		return;
	}

	printf ("\nNUMBER OF MESSAGE TO READ -> ");
	scanf ("%d", &message_number);

	if (message_number <= 0 || message_number > last_message) {
		printf ("INVALID MESSAGE NUMBER\n");
		return;
	}

	printf ("-----------------------------------\n");
	printf ("Message # %d\n", message_number);
	fgets(line,MAXLINE,fp);
	line[strlen(line)-1]='\0';
	while (i < message_number) {
		while (strcmp (line,"[eom]")) {
			fgets(line,MAXLINE,fp);
			line[strlen(line)-1]='\0';
		}
		i++;
		fgets(line,MAXLINE,fp);
		line[strlen(line)-1]='\0';
	}

	i = 0;
	while (1) {
		fgets (line, MAXLINE, fp);
		line[strlen(line)-1]='\0';
		if (!strcmp (line, "[eom]"))
			break;
		puts (line);
		if (i++ == 20) {
			i = 0;
			printf ("--- MORE ---");
			getch();
			printf ("\b\b\b\b\b\b\b\b\b\b\b\b            \b\b\b\b\b\b\b\b\b\b\b\b");
		}
	}

	printf ("-----------------------------------\n");
	fclose (fp);
}

/****** FUNCTION: send_message ***************************/

void send_message (char *user, char *subject, char *file_to_read)

{
	FILE *fp1, *fp2, *fp3;
	int i, j, last, from_file=1;
	char line[MAXLINE];
	char userfile[MAXPATH];
	char mail_buf[BUFSIZE+6];
	time_t t;

	if (!strcmp (user, "")) {
		printf ("SEND TO: ");
		fflush (stdin);
		gets (user);
	}

	strcpy (userfile, MAILDIR);
	strcat (userfile, user);
	strcpy (userfile, mkFileName(userfile));

	if (access (userfile, 0)) {
		printf ("\nNo such user.\n");
		return;
	}

	if (!strcmp (subject, "")) {
		from_file = 0;
		printf ("SUBJECT: ");
		fflush (stdin);
		gets (subject);
		fflush (stdin);
		printf ("TYPE MESSAGE.  CTRL-Z ENDS MESSAGE\n");
		printf ("%d MAXIMUM CHARACTERS.\n", BUFSIZE);
		printf ("==================================\n");
		i = 0;
		fflush (stdout);
		while (((mail_buf[i] = getch()) != 26) && (i < BUFSIZE)) {
			if (mail_buf[i] == '\r') {
				mail_buf[i] = '\n';
				printf ("%c", mail_buf[i]);
			} else if ((mail_buf[i] == '\b') && (i > 0) && (mail_buf[i-1] != '\n')) {
				printf ("\b");
				i--;
				continue;
			} else if (mail_buf[i] == '\b')
				continue;
			else
				printf ("%c", mail_buf[i]);

			i++;
		}
	}
	chmod (userfile, S_IREAD | S_IWRITE);
	if ((fp1 = fopen (userfile, "r")) == NULL) {
		perror (userfile);
		chmod (userfile, S_IREAD);
		return;
	}
	if ((fp2 = fopen (mkFileName(MAILTMP), "w")) == NULL) {
		perror (MAILTMP);
		fclose (fp1);
		chmod (userfile, S_IREAD);
		return;
	}
	fscanf (fp1, "%d\n", &last);
	last++;
	fprintf (fp2, "%d\n", last);
	while (fgets (line, MAXLINE, fp1) != NULL)
		fputs (line, fp2);
	fclose (fp1);

	t = time(NULL);
	fprintf (fp2, "TO:       %s\n", user);
	fprintf (fp2, "FROM:     %s\n", cur_user);
	fprintf (fp2, "SUBJECT:  %s\n", subject);
	fprintf (fp2, "DATE:     %s\n", ctime(&t));
	if (!from_file) {
		mail_buf[i-1] = '\n';
		mail_buf[i] = '[';
		mail_buf[i+1] = 'e';
		mail_buf[i+2] = 'o';
		mail_buf[i+3] = 'm';
		mail_buf[i+4] = ']';
		mail_buf[i+5] = '\n';
		for (j=0; j<i+6; j++)
			fprintf (fp2, "%c", mail_buf[j]);
	} else {
		if ((fp3 = fopen (file_to_read, "r")) == NULL) {
			perror (file_to_read);
			exit (1);
		}
		while (fgets (line, MAXLINE, fp3) != NULL)
			fputs (line, fp2);
		if (line[strlen(line)-1] == '\n')
			fprintf (fp2, "[eom]\n");
		else
			fprintf (fp2, "\n[eom]\n");
		fclose (fp3);
	}

	fclose (fp2);
	unlink (userfile);
	rename (mkFileName(MAILTMP), userfile);
	chmod (userfile, S_IREAD);
}

void check_mail (void)
{
	int fd;
	long last;
	char filepath[MAXPATH];
	char tmp[30];
	FILE *fp;
	struct stat statbuf;

	strcpy (filepath, USERCTRLDIR);
	strcat (filepath, cur_user);
	if ((fp = fopen(mkFileName(filepath), "r")) == NULL)
		last = -1;
	fgets (tmp, 30, fp);
	fscanf (fp, "%ld", &last);
	fclose (fp);

	strcpy (filepath, MAILDIR);
	strcat (filepath, cur_user);

	fd = open (mkFileName(filepath), O_RDONLY | O_TEXT);
	if (filelength (fd) > 10) {
		fstat (fd, &statbuf);
		if (statbuf.st_mtime > last && last != -1)
			printf ("You have new mail.\n");
		else
			printf ("You have mail.\n");
	}
	else
		printf ("No mail for user %s\n", cur_user);

	close (fd);
}
