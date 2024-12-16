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

#include <dir.h>

#define MAXUSER 69
#define MAXPASS 69
#define MAXSHELL 20
#define MAXLICENSE 9
#define MAXPHONE 14
#define MAXTZ 10

#define COPYRIGHT "Copyright 1992 Paul Walters"
#define PRODUCTNAME "SECURIX"
#define VERSION "Version 1.1"

#define ADDUSERKEY "476983"

/****
ERROR MESSAGES
****/
#define ARCHNOTBEGIN "Cannot find beginning of archive.\n"
#define BADLICENSE "Sorry, this system is not properly licensed.\n"
#define BADCPASSWD "Cannot create passwd file.  Bad system.\n"
#define BADCSHADOW "Cannot create shadow file.  Bad system.\n"
#define BADVSHADOW "Cannot validate shadow file.  Bad system.\n"
#define BADSHADOW "Password file corrupt.  Must be rebuilt.\n"
#define BADUSERID "FATAL. Cannot determine user identification.\n"
#define BADIGNLOGIN "Login Security File Corrupt.  Login Security Added.\n"
#define MNTERROR "Error.\n"
#define NOARCHDEFAULT "Cannot find arch default file (/etc/default/arch).\n"
#define NOTROOT "Sorry, you do not have Superuser access privledges.\n"
#define NOCHMOD "Sorry, you do not have permission to change file attributes.\n"
#define NOSPACE "Sorry, no space available on device.\n"
#define NOCDEFLTUSER "FATAL.  System Error.  Cannot create /etc/default/user file.\n"
#define NOVDEFLTUSER "FATAL.  System Error.  Cannot validate /etc/default/user file.\n"
#define MINARCHERROR "Specified file size to small.\n"
#define PROGINSTALLED "Program already installed.\n"
#define REACHEDMAXARCHDISK "No more disks can be used in archive.\n"

/****
ERROR NUMBERS
****/

#define EARCHNOTBEGIN 90
#define EBADLIC 100
#define EBADCPASSWD 110
#define EBADCSHADOW 113
#define EBADVSHADOW 114
#define EBADSHADOW 115
#define EBADIGNLOGIN 116
#define EBADUSER 120
#define EMARCHDSK 123
#define EMINARCHSIZE 125
#define EMNTERROR 127
#define ENOARCHDEFAULT 130
#define ENOCDEFLTUSER 140
#define ENOVDEFLTUSER 141
#define ENOCHMOD 145
#define ENOTROOT 150
#define ENOSPACE 155
#define EPROGINST 170

/*****
FILE DEFINITIONS
*****/
#define ARCHDEFAULT "\\etc\\default\\arch"
#define ARCHFILE "arch_v"
#define ARCHTEMP "\\tmp\\arch.tmp"
#define AUTHCHK "\\etc\\authchk.run"
#define AUTHCHKLCK "\\etc\\auth\\authchk.lck"
#define BULLETIN "\\etc\\bulletin.msg"
#define CD "\\etc\\cd.set"
#define CHMOD "\\etc\\chmod"
#define CHMODAUTH "\\etc\\auth\\chmod.ath"
#define COMSPEC "\\etc\\default\\comspec"
#define DFLTHOME "\\etc\\default\\home"
#define DEFAULTPATH "\\etc\\default\\path"
#define DEFLTSH "\\etc\\default\\shell"
#define DEFLTUSR "\\etc\\default\\user"
#define DEFLTUSRAUTH "\\etc\\auth\\defltusr.ath"
#define DOSPATH "\\etc\\dospath"
#define FIXPERM "\\etc\\perms"
#define IGNLOGIN "\\etc\\auth\\ignlogin"
#define IGNLOGINAUTH "\\etc\\auth\\ignlogin.ath"
#define LEAVE "\\etc\\default\\leave"
#define LICENSE "\\etc\\license"
#define LICENSEAUTH "\\etc\\auth\\license.ath"
#define LOGINAUTH "\\etc\\auth\\login.ath"
#define LOGRETRY "\\etc\\logretry"
#define LOOKDEFAULT "\\usr\\doc\\dict"
#define LPBUFFER "\\etc\\default\\lp\\buffer"
#define LPDEVICE "\\etc\\default\\lp\\device"
#define LPQUEUE "\\etc\\default\\lp\\queue"
#define MAILDIR "\\usr\\spool\\mail\\"
#define MAILTMP "\\usr\\spool\\mail\\mail.tmp"
#define MANDIR "\\usr\\man\\"
#define MESSAGE "\\etc\\login.msg"
#define MNT "\\etc\\mnt"
#define NEWMAIL "\\etc\\newuser\\mail\\welcome"
#define NEWUSERDIR "\\etc\\newuser\\"
#define PASSWD "\\etc\\passwd"
#define PASSWDLEN "\\etc\\passwd.len"
#define PASSWDTMP "\\etc\\passwd.tmp"
#define ROOTSHADOW "\\etc\\default\\shadow"
#define ROOTSHADOWAUTH "\\etc\\auth\\shadow.ath"
#define SHELLS "\\etc\\shells"
#define SHUTDOWN "\\etc\\shutdown"
#define SHUTDOWNAUTH "\\etc\\auth\\shutdown.ath"
#define STARTSHDIR "\\etc\\startsh\\"
#define STARTUPDIR "\\etc\\startup.rc\\"
#define STARTUPPROGS "\\etc\\startup.rc\\*.bat"
#define SU "\\etc\\su"
#define SUAUTH "\\etc\\auth\\su.ath"
#define SYSNAME "\\etc\\sysname"
#define SYSSTART "\\etc\\sysstart"
#define TIMEZONES "\\etc\\timezone"
#define TMP "\\tmp\\file.tmp"
#define TZ "\\etc\\default\\timezone"
#define USER "\\etc\\auth\\user"
#define USERCTRLDIR "\\etc\\users\\"
#define USERDIR "\\users\\"
#define USERFAILDIR "\\etc\\users\\failed\\"

#ifdef _UTIL_
#define EXTERNAL
#else
#define EXTERNAL extern
#endif

struct passwd_s {
	char userid[MAXUSER];
	int userno;
	char passwd[MAXPASS];
	char home[MAXPATH];
	char shell[MAXSHELL];
	char name[MAXUSER];
	char phone[MAXPHONE];
	char tz[MAXTZ];
	char reserved[24];
};

EXTERNAL int chmod_access (void);
EXTERNAL int CheckLicense (void);
EXTERNAL int DetermineUserID (char *userid);
EXTERNAL void Perror (char *str);
EXTERNAL void encode (char password[MAXPASS]);
EXTERNAL void get_passwd (char password[MAXPASS]);
EXTERNAL unsigned get_prog_size (char fn[MAXFILE]);
EXTERNAL void get_userid (char userid[MAXUSER]);
EXTERNAL void fix_default_user (void);
EXTERNAL void fix_passwd (void);
EXTERNAL char *mkFileName (char fn[MAXPATH]);
EXTERNAL int PermLoginOkay (void);
EXTERNAL int prog_installed (char fn[MAXFILE]);
EXTERNAL int RootLogin (int silent);
EXTERNAL char *encode_license (char license[MAXPASS]);
EXTERNAL void unix_to_dos_fn (char *fn);
EXTERNAL void update_root (struct passwd_s *userinfo);
