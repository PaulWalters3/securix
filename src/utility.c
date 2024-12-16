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
#include <dos.h>
#include <errno.h>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define _UTIL_
#include "utility.h"
#undef _UTIL_

/**********************************************************/

int CheckLicense (void)

{
	char sn[MAXLICENSE], lk[MAXLICENSE], key[MAXLICENSE];
	FILE *fp;
	long msize, mtime;
	struct stat statbuf;

	if ((fp = fopen(mkFileName(LICENSE), "r")) == NULL) {
		errno = EBADLIC;
		return (-1);
	}
	fscanf (fp, "%s", sn);
	stat (mkFileName(LICENSE), &statbuf);
	fclose (fp);

	strcpy (key, encode_license(sn));
	if ((fp = fopen(mkFileName(LICENSEAUTH), "r")) == NULL) {
		errno = EBADLIC;
		return (-1);
	}
	fscanf (fp, "%ld\n%ld\n%s\n", &msize, &mtime, lk);
	fclose (fp);
	if (statbuf.st_size != msize || statbuf.st_mtime != mtime) {
		errno = EBADLIC;
		return (-1);
	}
	if (strcmp (key, lk)) {
		errno = EBADLIC;
		return (-1);
	}
	return (0);
}

/**********************************************************/

int chmod_access ()
{
	char user[MAXUSER];
	char root[MAXUSER];
	char *r;
	FILE *fp;
	long mtime, msize;
	struct stat statbuf;

	if ((fp = fopen (mkFileName(USER), "r")) == NULL) {
		errno = ENOCHMOD;
		return (-1);
	}

	stat (mkFileName(USER), &statbuf);
	fscanf (fp, "%s", user);
	fclose (fp);

	if ((fp = fopen (mkFileName(LOGINAUTH), "r")) == NULL) {
		errno = ENOCHMOD;
		return (-1);
	}
	fscanf (fp, "%ld\n%ld\n", &msize, &mtime);
	fclose (fp);
	if (msize != statbuf.st_size || mtime != statbuf.st_mtime) {
		errno = ENOCHMOD;
		return (-1);
	}

	if (CheckLicense() == -1) {
		errno = EBADLIC;
		return (-1);
	}

	if ((fp = fopen (mkFileName(CHMODAUTH), "r")) == NULL) {
		errno = ENOCHMOD;
		return (-1);
	}
	fscanf (fp, "%ld\n%ld\n", &msize, &mtime);
	fclose (fp);

	if ((fp = fopen (mkFileName(CHMOD), "r")) == NULL) {
		errno = ENOCHMOD;
		return (-1);
	}

	stat (mkFileName(CHMOD), &statbuf);
	if (msize != statbuf.st_size || mtime != statbuf.st_mtime) {
		fclose (fp);
		errno = ENOCHMOD;
		return (-1);
	}

	while ( fgets (root, 80, fp) != NULL ) {
		root[strlen(root)-1] = '\0';
		if (!strcmp (user,root)) {
			r=getenv("USER");
			if (!strcmp(user,r)) {
				fclose(fp);
				return (0);
			}
		}
	}
	fclose (fp);
	errno = ENOCHMOD;
	return (-1);
}

/**********************************************************/

int DetermineUserID (char *userid)

{
	char *u;
	FILE *fp;
	long mtime, msize;
	struct stat statbuf;

	if ((fp = fopen(mkFileName(USER), "r")) == NULL) {
		errno = EBADUSER;
		return (-1);
	}

	fscanf (fp, "%s", userid);
	u = getenv ("USER");
	if (strcmp (userid,u)) {
		errno = EBADUSER;
		return (-1);
	}

	stat (mkFileName(USER), &statbuf);
	fclose (fp);
	if ((fp = fopen (mkFileName(LOGINAUTH), "r")) == NULL) {
		errno = EBADUSER;
		return (-1);
	}
	fscanf (fp, "%ld\n%ld\n", &msize, &mtime);
	fclose (fp);
	if (msize != statbuf.st_size || mtime != statbuf.st_mtime) {
		errno = EBADUSER;
		return (-1);
	}

	if (CheckLicense() == -1) {
		errno = EBADLIC;
		return (-1);
	}

	return (0);
}

/**********************************************************/

void encode (char password[MAXPASS])
{
	register int i;
	char encoded[MAXPASS];

	for (i = 0; i < strlen (password); i++)
		encoded[i] = (password[i]*strlen(password)+47) % 91 + 33;
	encoded[i] = '\0';
	strcpy (password, encoded);
}

/**********************************************************/

char *encode_license (char license[MAXPASS])

{
	int i;
	static char encoded[MAXLICENSE];

	for (i = 0; i < strlen(license); ++i)
		encoded[i] = (license[i] % 26) + 65;
	encoded[i] = '\0';
	return (encoded);
}

/**********************************************************/

void fix_default_user (void)

{
	FILE *fp;
	struct stat statbuf;

	chmod (mkFileName(DEFLTUSR), S_IREAD | S_IWRITE);
	if ((fp = fopen (mkFileName(DEFLTUSR), "w")) == NULL) {
		errno = ENOCDEFLTUSER;
		Perror (mkFileName(DEFLTUSR));
		chmod (mkFileName(DEFLTUSR), S_IREAD);
	} else {
		fprintf (fp, "root\n");
		fclose (fp);
		chmod (mkFileName(DEFLTUSR), S_IREAD);

		if ((fp = fopen(mkFileName(DEFLTUSR), "r")) == NULL) {
			errno = ENOVDEFLTUSER;
			Perror (mkFileName(DEFLTUSR));

		} else {
			stat (mkFileName(DEFLTUSR), &statbuf);
			fclose (fp);
			chmod (mkFileName(DEFLTUSRAUTH), S_IREAD | S_IWRITE);
			if ((fp = fopen (mkFileName(DEFLTUSRAUTH), "w")) == NULL) {
				errno = ENOVDEFLTUSER;
				Perror (mkFileName(DEFLTUSR));
				chmod (mkFileName(DEFLTUSRAUTH), S_IREAD);
			} else {
				fprintf (fp, "%ld\n%ld\n", statbuf.st_size, statbuf.st_mtime);
				fclose (fp);
				chmod (mkFileName(DEFLTUSRAUTH), S_IREAD);
			}
		}
	}
}

/**********************************************************/

void fix_passwd (void)

{
	char cmd[MAXPATH];
	FILE *fp;
	struct passwd_s userinfo;

	if ((fp = fopen (mkFileName(ROOTSHADOW), "r")) == NULL) {
		errno = EBADSHADOW;
		Perror (mkFileName(ROOTSHADOW));
		chmod (mkFileName(PASSWD), S_IREAD | S_IWRITE);

		if ((fp = fopen (mkFileName(PASSWD), "w")) == NULL) {
			errno = EBADCPASSWD;
			Perror (mkFileName(PASSWD));
			chmod (mkFileName(PASSWD), S_IREAD);
			return;
		}
		fclose (fp);
		sprintf (cmd, "%s root %s", mkFileName("adduser"), ADDUSERKEY);
	} else {
		fread ((struct passwd_s *) &userinfo, sizeof (struct passwd_s), 1, fp);
		fclose (fp);
		chmod (mkFileName(PASSWD), S_IREAD | S_IWRITE);
		if ((fp = fopen (mkFileName(PASSWD), "w")) == NULL) {
			errno = EBADCPASSWD;
			Perror (mkFileName(PASSWD));
			chmod (mkFileName(PASSWD), S_IREAD);
			return;
		}
		fwrite ((struct passwd_s *) &userinfo, sizeof (struct passwd_s), 1, fp);
		fclose (fp);
		chmod (mkFileName(PASSWD), S_IREAD);
	}
}

/**********************************************************/

void get_passwd (char password[MAXPASS])
{
	int i=0;

     while ((password[i]=getch()) != '\r') {
		if (password[i]=='\b'&&i>0) {
			i--;
			printf("\b \b");
		} else {
			i++;
			printf(".");
		}
	}
	password[i]='\0';
}

/**********************************************************/

void get_userid (char userid[MAXUSER])
{
	int i=0;

	while ((userid[i]=getch()) != '\r') {
		if (userid[i]=='\b') {
			userid[--i]='\0';
			printf("\b \b");
		} else {
			userid[++i]='\0';
			printf("%c", userid[i-1]);
		}
	}
	printf("\n");
	userid[i]='\0';
}

/**********************************************************/

char *mkFileName (char fn[MAXPATH])
{
	static char tmp[MAXPATH];
	FILE *fp;

	if ((fp = fopen ("c:\\securix", "r")) == NULL)
		return (fn);
/*****
Version 1.0
	fscanf (fp, "%c", &tmp[0]);
	tmp[1] = ':';
	tmp[2] = '\0';
*****/
	fgets (tmp, MAXPATH, fp);
	fclose (fp);
	if (tmp[strlen(tmp)-1] == '\n')
		tmp[strlen(tmp)-1] = '\0';

	strcat (tmp, fn);
	return (tmp);
}

/**********************************************************/

int RootLogin (int sil)
{
	char user[MAXUSER];
	char root[MAXUSER];
	char *r;
	FILE *fp;
	long mtime, msize;
	struct stat statbuf;

	if ((fp = fopen (mkFileName(USER), "r")) == NULL) {
		errno = ENOTROOT;
		Perror (mkFileName(USER));
		return (-1);
	}

	stat (mkFileName(USER), &statbuf);
	fscanf (fp, "%s", user);
	fclose (fp);

	if ((fp = fopen (mkFileName(LOGINAUTH), "r")) == NULL) {
		errno = ENOTROOT;
		if (!sil)
			Perror (mkFileName(LOGINAUTH));
		return (-1);
	}
	fscanf (fp, "%ld\n%ld\n", &msize, &mtime);
	fclose (fp);
	if (msize != statbuf.st_size || mtime != statbuf.st_mtime) {
		errno = ENOTROOT;
		if (!sil)
			Perror ("");
		return (-1);
	}

	if (CheckLicense() == -1) {
		errno = EBADLIC;
		if (!sil)
			perror ("");
		return (-1);
	}

	if (!strcmp (user, "root"))
		return (0);

	if ((fp = fopen (mkFileName(SUAUTH), "r")) == NULL) {
		errno = ENOTROOT;
		if (!sil)
			Perror ("");
		return (-1);
	}
	fscanf (fp, "%ld\n%ld\n", &msize, &mtime);
	fclose (fp);

	if ((fp = fopen (mkFileName(SU), "r")) == NULL) {
		errno = ENOTROOT;
		if (!sil)
			Perror (mkFileName(SU));
		return (-1);
	}

	stat (mkFileName(SU), &statbuf);
	if (msize != statbuf.st_size || mtime != statbuf.st_mtime) {
		errno = ENOTROOT;
		if (!sil)
			Perror ("");
		fclose (fp);
		return (-1);
	}

	while ( fgets (root, 80, fp) != NULL ) {
		root[strlen(root)-1] = '\0';
		if (!strcmp (user,root)) {
			r=getenv("USER");
			if (!strcmp(user,r)) {
				fclose(fp);
				return (0);
			}
		}
	}
	errno = ENOTROOT;
	if (!sil)
		Perror ("");
	fclose (fp);
	return (-1);
}

/**********************************************************/

void update_root (struct passwd_s *userinfo)

{
	FILE *fp;
	struct stat statbuf;

	chmod (mkFileName(ROOTSHADOW), S_IREAD | S_IWRITE);
	if ((fp = fopen (mkFileName(ROOTSHADOW), "w")) == NULL) {
		errno = EBADCSHADOW;
		Perror (mkFileName(ROOTSHADOW));
		chmod (mkFileName(ROOTSHADOW), S_IREAD);
		return;
	}
	fwrite ((struct passwd_s *) userinfo, sizeof(struct passwd_s), 1, fp);
	fclose (fp);
	chmod (mkFileName(ROOTSHADOW), S_IREAD);

	if ((fp = fopen (mkFileName(ROOTSHADOW), "r")) == NULL) {
		errno = EBADCSHADOW;
		Perror (mkFileName(ROOTSHADOW));
		return;
	}
	stat (mkFileName(ROOTSHADOW), &statbuf);
	fclose (fp);

	chmod (mkFileName(ROOTSHADOWAUTH), S_IREAD | S_IWRITE);
	if ((fp = fopen(mkFileName(ROOTSHADOWAUTH), "w")) == NULL) {
		errno = EBADVSHADOW;
		Perror (mkFileName(ROOTSHADOWAUTH));
		chmod (mkFileName(ROOTSHADOWAUTH), S_IREAD);
		return;
	}

	fprintf (fp, "%ld\n%ld\n", statbuf.st_size, statbuf.st_mtime);
	fclose (fp);
	chmod (mkFileName(ROOTSHADOWAUTH), S_IREAD);
}

/**********************************************************/

int prog_installed (char prog[MAXFILE])

{
	int i, count = 0;
	unsigned far *mcb_seg;
	unsigned next_mcb_seg;
	union REGS inregs, outregs;
	struct SREGS segs;
	char fn[8];

	struct MCB {
		char signature;
		unsigned owner;
		unsigned size;
		char reserved[3];
		char filename[8];
	} far *mcb;

	inregs.h.ah = 0x52;
	intdosx (&inregs, &outregs, &segs);

	mcb_seg = (unsigned far *) (((long) (segs.es) << 16) + (outregs.x.bx - 2));
	mcb = (struct MCB far *) ((long) (*mcb_seg) << 16);

	for (;;) {
		for (i=0; i<8; ++i) {
			if ((mcb->filename[i] != NULL) && (mcb->filename[i] != ' '))
				fn[i] = mcb->filename[i];
			else
				break;
		}
		fn[i] = '\0';
		if (mcb->owner == FP_SEG(mcb) + 1) {
			if (!strcmp(strupr(prog), fn))
				++count;
		}
		if (mcb->signature != 0x5a) {
			next_mcb_seg = FP_SEG(mcb) + mcb->size + 1;
			mcb = (struct MCB far *) ((long) (next_mcb_seg) << 16);
		} else {
			break;
		}
	}

	if (count > 1) {
		errno = EPROGINST;
		return (1);
	}
	return (0);
}

/**********************************************************/

unsigned get_prog_size (char prog[MAXFILE])

{
	int i;
	unsigned size=0;
	unsigned far *mcb_seg;
	unsigned next_mcb_seg;
	union REGS inregs, outregs;
	struct SREGS segs;
	char fn[8];

	struct MCB {
		char signature;
		unsigned owner;
		unsigned size;
		char reserved[3];
		char filename[8];
	} far *mcb;

	inregs.h.ah = 0x52;
	intdosx (&inregs, &outregs, &segs);

	mcb_seg = (unsigned far *) (((long) (segs.es) << 16) + (outregs.x.bx - 2));
	mcb = (struct MCB far *) ((long) (*mcb_seg) << 16);

	for (;;) {
		for (i=0; i<8; ++i) {
			if ((mcb->filename[i] != NULL) && (mcb->filename[i] != ' '))
				fn[i] = mcb->filename[i];
			else
				break;
		}
		fn[i] = '\0';
		if (mcb->owner == FP_SEG(mcb) + 1) {
			if (!strcmp(strupr(prog), fn)) {
				size = mcb->size;
				break;
			}
		}
		if (mcb->signature != 0x5a) {
			next_mcb_seg = FP_SEG(mcb) + mcb->size + 1;
			mcb = (struct MCB far *) ((long) (next_mcb_seg) << 16);
		} else {
			break;
		}
	}
	return (size);
}

/**********************************************************/

void unix_to_dos_fn (char *fn)
{
	char tmp[MAXPATH];
	int i;

	for (i=0; i<strlen(fn); ++i) {
		if (fn[i] == '/')
			fn[i] = '\\';
	}
	strcpy (tmp, strupr(fn));
	strcpy (fn, tmp);
}

/**********************************************************/

int PermLoginOkay (void)
{
	FILE *fp;
	long mtime, msize;
	struct stat statbuf;

	if (access (mkFileName(IGNLOGIN), 0))
		return (0);

	if ((fp = fopen (mkFileName(IGNLOGIN), "r")) == NULL) {
		_chmod (mkFileName(IGNLOGIN), 1, 0);
		_chmod (mkFileName(IGNLOGINAUTH), 1, 0);
		unlink (mkFileName(IGNLOGIN));
		unlink (mkFileName(IGNLOGINAUTH));
		errno = EBADIGNLOGIN;
		return (-1);
	}
	stat (mkFileName(IGNLOGIN), &statbuf);
	fclose (fp);
	if ((fp = fopen (mkFileName(IGNLOGINAUTH), "r")) == NULL) {
		_chmod (mkFileName(IGNLOGIN), 1, 0);
		_chmod (mkFileName(IGNLOGINAUTH), 1, 0);
		unlink (mkFileName(IGNLOGIN));
		unlink (mkFileName(IGNLOGINAUTH));
		errno = EBADIGNLOGIN;
		return (-1);
	}
	fscanf (fp, "%ld\n%ld\n", &msize, &mtime);
	if (msize != statbuf.st_size || mtime != statbuf.st_mtime) {
		_chmod (mkFileName(IGNLOGIN), 1, 0);
		_chmod (mkFileName(IGNLOGINAUTH), 1, 0);
		unlink (mkFileName(IGNLOGIN));
		unlink (mkFileName(IGNLOGINAUTH));
		errno = EBADIGNLOGIN;
		return (-1);
	}
	fclose (fp);
	return (0);
}

/**********************************************************/

void Perror (char *str)

{
	char error[MAXPATH];

	if (str[0])
		printf ("%s:", str);

	switch (errno) {
	case EZERO	: strcpy (error, "No error.\n");
				  break;
	case EINVFNC	: strcpy (error, "Invalid function number.\n");
				  break;
	case ENOFILE   : strcpy (error, "File not found.\n");
				  break;
	case ENOPATH	: strcpy (error, "Path not found.\n");
				  break;
	case EMFILE	: strcpy (error, "Too many files are open.\n");
				  break;
	case EACCES	: strcpy (error, "Permission denied.\n");
				  break;
	case EBADF	: strcpy (error, "Bad file number.\n");
				  break;
	case ECONTR	: strcpy (error, "Memory blocks destroyed.\n");
				  break;
	case ENOMEM	: strcpy (error, "Not enough core.\n");
				  break;
	case EINVMEM	: strcpy (error, "Invalid memory block address.\n");
				  break;
	case EINVENV 	: strcpy (error, "Invalid environment.\n");
				  break;
	case EINVFMT	: strcpy (error, "Invalid format.\n");
				  break;
	case EINVACC	: strcpy (error, "Invalid access code.\n");
				  break;
	case EINVDAT	: strcpy (error, "Invalid data.\n");
				  break;
	case ENODEV	: strcpy (error, "No such device.\n");
				  break;
	case ECURDIR 	: strcpy (error, "Attempt to remove current directory.\n");
				  break;
	case ENOTSAM 	: strcpy (error, "Not same device.\n");
				  break;
	case ENMFILE 	: strcpy (error, "No more files.\n");
				  break;
	case EINVAL  	: strcpy (error, "Invalid argument.\n");
				  break;
	case E2BIG   	: strcpy (error, "Arg list too long.\n");
				  break;
	case ENOEXEC 	: strcpy (error, "Exec format error.\n");
				  break;
	case EXDEV   	: strcpy (error, "Cross-device link.\n");
				  break;
	case EDOM    	: strcpy (error, "Math argument.\n");
				  break;
	case ERANGE  	: strcpy (error, "Result too large.\n");
				  break;
	case EEXIST	: strcpy (error, "File already exists.\n");
				  break;
	case EBADLIC	: strcpy (error, BADLICENSE);
				  break;
	case EBADUSER  : strcpy (error, BADUSERID);
				  break;
	case ENOCHMOD	: strcpy (error, NOCHMOD);
				  break;
	case ENOTROOT	: strcpy (error, NOTROOT);
				  break;
	case ENOSPACE	: strcpy (error, NOSPACE);
				  break;
	case ENOCDEFLTUSER 	: strcpy (error, NOCDEFLTUSER);
					  break;
	case EBADCPASSWD 	: strcpy (error, BADCPASSWD);
					  break;
	case EBADCSHADOW	: strcpy (error, BADCSHADOW);
					  break;
	case EBADVSHADOW	: strcpy (error, BADVSHADOW);
					  break;
	case EBADSHADOW	: strcpy (error, BADSHADOW);
					  break;
	case ENOVDEFLTUSER	: strcpy (error, NOVDEFLTUSER);
					  break;
	case EPROGINST		: strcpy (error, PROGINSTALLED);
					  break;
	case ENOARCHDEFAULT : strcpy (error, NOARCHDEFAULT);
					  break;
	case EMINARCHSIZE	: strcpy (error, MINARCHERROR);
					  break;
	case EMARCHDSK		: strcpy (error, REACHEDMAXARCHDISK);
					  break;
	case EMNTERROR		: strcpy (error, MNTERROR);
					  break;
	case EBADIGNLOGIN   : strcpy (error, BADIGNLOGIN);
					  break;
	case EARCHNOTBEGIN	: strcpy (error, ARCHNOTBEGIN);
					  break;
	default			: strcpy (error, "\n");
					  break;
	}
	printf ("%s", error);
}
