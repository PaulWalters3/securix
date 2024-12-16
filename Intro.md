
# SECURIX
Version 1.1

Copyright 1992-2024 Paul Walters

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    https://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

## SYSTEM REQUIREMENTS:

SECURIX can be run on any 8088, 80286, 80386, or 80486 type processor, requiring
MS-DOS 3.3 or higher, 2.5 megabytes of free hard disk space, and 640 Kb of memory.

## WHAT IS SECURIX?

SECURIX is a set of utilities that provide system security, "Unix-like" commands, and other helpful PC utilities for DOS.

SECURIX can prohibit unauthorized users from using your system, provided they do not boot from another source such as a floppy drive.  SECURIX provides this service by prompting or a user identification and password to enter into the system.  This password is kept in an encrypted form.  Upon logging into the system, SECURIX executes a shell that has been defined for use by a user.  This shell can be DOS, DOSSHELL, Windows, or any other shell utility.  Upon exiting this shell, the user is logged off the system and a login prompt returns.  Of course this option of logging into the system can be turned off using the SECURIX configuration utility.

SECURIX also provides dozens of other utilities that perform file and directory manipulation, time, date, memory, and other disk functions.  Also, any of these utilities that require pathnames allow a forward slash (/) to be substituted for back slashes (\\).

## INSTALLING SECURIX

If you purchased the 3.5" diskette version of SECURIX, you will have 3 diskettes included in the package.  If you purchased the 5.25" diskette version of SECURIX, you will have 6 diskettes included in the package.  To install SECURIX, place the first diskette into the appropriate drive on your PC.  If that drive is a:, type in the command  a:setup .  If the drive is b:, type in the command b:setup .  Then press the enter or return key.  Securix will load itself into memory.

SECURIX will first prompt for the drive letter to install to.  This drive must be a valid hard drive letter.  SECURIX is not guaranteed to work unless it is installed properly according to these instructions.  After entering the hard drive to install SECURIX to, enter the name of the directory you wish to have SECURIX installed to.  This may be the home directory.  Which ever directory you decide to have SECURIX installed to, a file system will be created that begins to resemble a Unix root file system directory.  SECURIX will allow you to review you installation directory selections and continue when you feel ready.

SECURIX will begin to extract files from the floppy diskettes.  When SECURIX has encountered the end of a diskette, a  message will be displayed so that you may insert the next diskette.  After SECURIX has read in all the files from the installation diskettes, it will create the SECURIX file system and install all those files to their proper locations.

Once all the files have been properly installed, a copy of the disclaimer and license agreement will appear on the screen.  Be sure to read this agreement carefully.  Press any key when you have completed reading.  Once you have read and agreed to the terms of the license agreement a prompt for a serial number will appear on the screen.  You must enter the serial number found on the label included with your copy of SECURIX.  SECURIX will fail to work if this step is not performed.  The serial number must be entered in exactly as shown, paying careful attention to the case of the letters.  After entering the serial number, you must enter the serial number key.  If this key is entered improperly, SECURIX will fail to work properly.  If you do enter these items incorrectly, SECURIX will alert you to the problem and let you try until you have entered in a valid serial number and key.  After entering your serial number and key, enter your name as the person to whom this software is licensed and if applicable the company to which this software is licensed.

After entering your serial number and key, SECURIX will prompt for a series of system default settings that you must provide.  The first of these is the name of the computer system. You may name your system to anything you wish.  After you enter the system name, SECURIX will determine whether you wish to have SECURIX provide login security on the system.  If you would like to have SECURIX prompt for user identifications and passwords to access the system, then enter a n so that system security remains intact.  If you wish to remove login controls from your computer system, enter a y.  Doing this allows you to access your computer system without having to enter a password.  You will have superuser access priveledges at first unless you change the default user identification through the SECURIX configuration utility.

After making a decision on log in security, you must enter the full path name of where the DOS operating system utilities are located.  SECURIX will only allow you to enter valid directories and will check these directories for the presence of the DOS operating system.  SECURIX needs to know where Dos is located so that it can setup many of its files.

The next process of the installation requires you to make a decision on how you would like the chdir, mkdir, and rmdir functions to operate.  Securix provides three utilities, chd, mkd, and rmd which provide the same functionallity as their Dos counterparts, however as stated before, they will accept a forward slash as valid input in a path name in addition to a backward slash.  If you would like, SECURIX can install a DOSKEY alias which allows these three programs to execute instead of their DOS counterparts.  If you would like to have this functionallity, type a n at the prompt so the chdir alias is not removed, otherwise type y.  This is also a function that can be reconfigured at a later time using the SECURIX configuration utility.  After installing the chdir alias, SECURIX will prompt for a timezone.  You should select the timezone that corresponds to your geographic location.

Next, SECURIX prompts for the root user identification's password.  The root user identification has Superuser access capabilities. Even if you do not enable login security, you must choose a password.  Many of the SECURIX utilities require this password in order for proper use.

SECURIX next displays a message telling you that your current autoexec.bat file will be moved and replaced with the SECURIX autoexec.bat file.  This is necessary so that SECURIX can function properly.  If you did not enable system security, then you do not have to have the autoexec.bat file moved.  However, if you wish to have login security enabled at a later date, you will have to install the SECURIX autoexec.bat file located in the /etc directory.  If there are things you wish to be performed, your old autoexec.bat file can be moved to a directory where all the batch files contained in that directory are executed upon boot of the computer system.  It is not recommended that a program such as Windows or DOSSHELL be started in your old autoexec.bat file.  This will defeat the purpose of keeping users out of SECURIX.  You may configure these programs to execute upon startup using SECURIX chsh utility and the SECURIX configuration utility.

Finally, SECURIX checks the file permissions, and then is done.  To activate SECURIX, reboot your computer.

## USING ON-LINE MANUAL PAGES

SECURIX provides a set of on-line manual reference pages.  These manual pages are referenced by using the SECURIX man command, which is similar to the Unix man command.  To use the man command, at the DOS prompt type in the word man followed by the name of the command you need help with.  Man will display a pagefull of information at a time.  To display the next page of information, press the space bar at the MORE prompt.  To display the next line of information, press the enter key at the MORE prompt.  To rewind to the top of the manual page press the T key, and to exit the MORE prompt without reading the entire manual page, press the Q key.  To get more information, see the man manual page.

## SECURIX System Security

SECURIX provides your computer system with a set of system utilities that prevent unauthorized access.  The first level of SECURIX security is provided at boot time.  Provided the autoexec.bat file remains unchanged, when the system is booted, the SECURIX login program is invoked.  This program is never exited.

At first, the login program initializes itself.  It sets up the chdir alias if it has been confirgured.  The chdir alias allows all directory programs (chdir, mkdir, and rmdir) to use forward slashes in the path names given to those programs.  Next, any batch files that exist in the /etc/startup.rc directory are executed.  It is necessary to make sure that these files do not start any programs that do not exit other then memory-resident programs.  If any of these batch files start Windows or DOS, then SECURIX can never take over from these programs until they are exited.  This is definately not desired if you have login security enabled.  Next, if configured to do so, the system performs an authentication check.  This utility checks the authentication of system files.  The option of performing this check at boot time is configurable in the SECURIX configuration utility.

After performing its initialization, SECURIX checks to see if login security has been enabled.  If it is not, the default user shell is executed.  The default user shell is the program that is executed at boot time when login security is disabled.  This default shell is configurable through the SECURIX configuration utility.  If login security is enabled, the computer system name is displayed, then the message contained in the file /etc/login.msg is displayed.  After these items are displayed, a prompt for a Login is displayed.  At this prompt, enter your userid.  If it is the first time logging into the system or you have not configured other user identifications, then login as the root user identification.  After entering the user identification, enter the password for that user identification.  If your login is correct, the message contained in /etc/bulletin.msg is displayed, then the login.bat file in your home directory is executed, and finally you user shell is entered.  If you enter an invalid user identification or password, a prompt for userid and password will appear again.  If desired, a waiting period between login attempts may be configured.  If you are using Dos, you may log out of SECURIX by typing exit or logout.  If you are using some other shell, just exiting from that shell will log you out from SECURIX and present you with another login prompt.

The root user identification has all Superuser access priveledges.  With the root user id, all SECURIX programs can be accessed and used.  Other individual user identifications can also be granted superuser access priveledges.  This is done through the SECURIX configuration utility.  Of course, only users with superuser access priveledges can run the SECURIX configuration utility.  Users who do not have superuser access priveledges can temporary give themselves superuser access priveledges by running the su program to make themselves superuser.  To do this, a user must know the root login identification.  While logged into SECURIX, type in su and press enter or return.  If your user identification has superuser access priveledges, then you userid will effectively become that of root until you exit.  If you do not have superuser access priveledges, then a prompt for the root password will appear.  If you enter the correct password, a new DOS command interpretur will invoke itself and you will have root access priveledges.  To exit from having root access priveledges, type exit or logout. You can also specify a user identification that you wish to become.  You will have to enter a valid password to become that userid.  See the su manual page for more information.

To add or remove users to the system, invoke the adduser and rmuser programs respectively.  Of course, you must have superuser access priveledges to use these programs.  Any user may change their password by invoking the passwd command.  Passwords are stored in an encrypted format in the /etc/passwd file.  The root user id can change anyone's password.  If passwd is invoked with the -f option, a user can change their personal information stored in the password database.  Associated with each user id is a login shell.  Users can change their login shell by invoking the chsh program.  The login shell is the program executed after a user a sucessfully logged into SECURIX.  This program is usually an operating system shell or menu system, such as DOS, DosShell, or Windows.  When you exit from your login shell, a login prompt will reappear.  Superuser's can add and remove shells available for use through the mkshell program.

The chmod program provides for changing file attributes.  Only persons granted chmod access can use this program to change the attributes of files.  In order for this option to work properly, the DOS attrib program and any other programs that change file attributes should be removed from your system.  This will give only those users specified the ability to change file attributes.

The following is a quick summary of all SECURIX commands used in the security of the system:

* adduser - allows for new users to granted access to the system.
* authchk - authenticates and checks for system tampering of important files.
* chsh - changes the login shell used by a particular user.
* finger - displays information about all SECURIX users.
* fixperm - checks and fixes file permissions according to file /etc/perms.
* login - main SECURIX program providing main level of system security.
* lock - temporarily locks a terminal until a password is entered.
* mkshell - creates new system login shells.
* passwd - changes a users password and personal information.
* rmuser  - deletes a particular user from the system, so they no longer have access.
* su - allows a user to temporarily become another user such as the superuser.
* w - shows the userid of the person currently logged into the system.

For more information on any of these commands, see their respective manual pages.

## SECURIX and Unix

SECURIX provides some Unix operating system commands available for use on your Dos PC.  These commands include ls, man, cp, mv, rm, and many others.

As with every SECURIX command, pathnames given as parameters to these commands may given using the forward slash as used in Unix, instead of a backslash, as required in Dos.  For example, to show the contents of the root directory, the command "ls /" and "ls \" are both equal.  To show the contents of the c:\dos directory, the command "ls c:/dos" is the same as "ls c:\dos." SECURIX provides this capability to make all "Unix-like" commands feel like the real thing.

Most Unix commands that are imitated here are done so as faithfully as possible, keeping as many of their original capabilities and options available.  Here is a complete list of the "Unix-like" commands available:

* cat - displays a file.
* cp - copies a file.
* df - displays disk storage information.
* finger - displays all users and their personal information, on the SECURIX system.
* head - displays the beginning of a file.
* kill - frees up a process currently in memory.
* leave - alarm clock.
* look - looks for the occurence of a character string in a file.
* lp - prints a file.
* lpq - displays current print queue information and printer status.
* lprm - removes a file from the print queue.
* ls - lists files and directories.
* man - provides lookup of manul reference pages.
* more - displays contents of a file in pages.
* mv - renames a file.
* passwd  - changes a user password and personal information.
* ps - displays all Dos processes currently running.
* pwd - prints current working directory.
* rm - removes a file.
* sum - generates a checksum.
* tail - displays tail-end of a file.
* touch - updates a file date and time stamp.
* w - displays current users logged into the system.

In addtion, the commands chd, mkd, and rmd can take the place of the Dos chdir, mkdir, and rmdir commands, to provide "Unix-like" directory manipulation capabilities.

These commands will be explained in greater detail in the following sections along with other SECURIX commands to be used in conjunction with these Unix commands.  For additional information on any of these commands, see the appropriate manual page.

## File Filters and Manipulation

SECURIX provides many different file filters and other programs for file manipulation.  The following section will list each of these commands, and their intended purpose.  For a more detailed look at these commands, see their appropriate manual page.

* cat - The cat program concatenates two files.  The output of this command is displayed on the screen.  This information may be redirected to a file or piped to another program.  The cat program is useful for displaying the contents of files.

* cp - The cp program copies the contents of one file to the filename given.  It can also copy multiple files to a given directory.

* head - The head program displays the first 10 lines of a file by default.  Head can search for a pattern in a file and display the next 10 lines of a file.

* look - The look program looks for all occurences of the given character string in the given file(s).  If no file is given, then look searchs the file /usr/doc/dict for all occurences of the given character string.

* more - The more program displays the contents of a file a pagefull at a time, prompting the user when they are ready to display the next screenful.

* mv - The mv programs renames a file to the given file name, or will mv a file or files to the given directory.

* rm - The rm program removes the given file(s).

* sum - The sum program will generate a checksum for the given files and provide the number of 512Kb blocks in the file.

* tail - The tail program will display the last ten lines of a file.  It can continuously check to see if the file has been updated and display this new information.  It also can display a file in reverse.

* touch - The touch program updates the date and time of a file to the current date and time.

* arch - The arch program provides for system backup and restoration.

* chd - The chd program changes the current working directory.  If no directory is given, the chd will make the directory given by the environment variable HOME, the current directory.

* chmod - The chmod program changes a files attributes.

* mkd - The mkd program creates a directory.

* rmd - The rmd program removes a directory or can recursively remove all files and directories in the given directory.

* lp - The lp program installs the print queue and prints a text file.

* lpq - The lpq program displays the contents of the print queue.

* lprm - The lpqrm program removes a file from the print queue.

* ls - The ls program lists all files and directories.

## Date and Time Utilities

SECURIX provides some date and time utilities to help you keep track of time.  The program clock is a Terminate-Stay-Resident (TSR) program which displays the current time in the upper right hand corner of you screen.  Another TSR program called leave, is a program which will notify you when a certain time has been reached.  When you invoke leave, you give the hours and minutes of when you want to be notified, or leave will prompt you for them.  When the time is reached, a message will be displayed in the upper-left corner of the screen.  This message is changeable through the SECURIX configuration utility.

The SECURIX programs mytime and today print the current date and time.  The mytime program differs from the today program, in that it prints the time according to your timezone.  The TZ environment variable provides mytime with the proper timezone information.  The format of this variable is AAA#BBB, where AAA is the timezone label, # is the number of hours offset from GMT, and BBB is the timezone label when daylight savings time is in effect.  The mytime program has different affects on different computers because of the way daylight savings time is handled in the bios.

The SECURIX programs sdate and stime, set the current date and time.  sdate requires a parameter of the form mmddyy.  If this parameter is not given, you will be prompted for the correct information.  stime requires a parameter hhmmss.  Again, if this parameter is not given, you will be prompted for the correct information.

The timep program will time the execution of the given command.  Timing statistics are generated after the command has executed.  The sleep program will halt all processing for the number of seconds given at the command line.

For more information on any of these commands, see their respective manual page.

## Disk and Memory Utilities

SECURIX provides several utilities for disk and memory status and usage information.  A short overview of these utilities is given here.  To find out more about these utilities, refer to each individual manual page.

A disk utility, called df, is used to display current disk usage.  The program displays the size of the current drive, the number of used and free bytes, and the percentage of free space. An optional parameter may be given to specify a particular drive.  The current drive is the default.

The pwd utility is a simple program which displays the current working directory.  This command may be useful if you do not display the current directory in your Dos prompt.  The pcpark utility and the ps2park utility are provided to park the heads of your hard drive.  You can figure out which program to use by the type of machine you have.

The last of the disk utilities performs the same functionallity as the Dos subst and join commands.  The command performed depends on how this program is configured through the SECURIX configuration program.  If this program is configured to use the join command, then a physical disk drive can be associated with a directory on another drive.  If the program is configured to use the subst command, then a physical directory can be configured as a logical drive.

There are three main memory utilities.  The first is the fmem program which displays the amount of memory available and used.  The second program is the ps program which is similar to the Unix ps program.  This program displays all processes currently in memory, their size, and their process id (PID).  The kill command is used to stop a process, by freeing up the memory it is using.  The kill command takes a process id as a parameter.

## Miscellaneous Utilities

SECURIX provides several other miscellaneous utilities.  The following lists each and briefly gives a summary of each.  For more information on these, see their respective manual pages.

* clear - Clears the screen.
* config - Configures the SECURIX system.  See next chapter for more information.
* license - Validates your copy of SECURIX.
* mail - Allows mail to be sent to other SECURIX users.
* man - On-line manual pages lookup command.
* mkshtdown - Creates a shutdown procedure for your system.
* shutdown - Will shutdown your system down according to the shutdown procedure.
* sysinfo - Displays SECURIX status information.
* sysstart - Displays amount of time system has been powered on for.

## SECURIX Configuration

The following section explains the use of the SECURIX configuration utility.  Only users granted Superuser access priveledges may use this utility.  To run the utility, change directories to /etc in the SECURIX installation directory.  The program name is config.

1 The arch program default drive specification.  By specifying a default drive used by the arch program, this parameter does not have to be given to the arch program every time you use it, as long as you are using the default drive.  See the arch manual page for more information.

2 The chdir alias.  By installing the chdir alias, SECURIX replaces the standard Dos directory commands with its own.  These commands allow the use of forward slashes in path names in addition to backslashes.  See the chd, mkd, and rmd manual pages for more information.

3 The default comspec environment variable, tells SECURIX where to find the Dos command interpretur.  SECURIX uses this value to set the comspec environment variable.

4 The default home directory is the home directory that is used when login security has been disabled.  See the home manual page for more information.

5 The default shell is the shell that is used when login security has been disabled.  This shell includes items such as Dos, DosShell, and Windows.

6 The default userid is the userid in effect when login security is disabled.  All SECURIX programs base their access privledges based on this userid.

7 The default path is the path that is set up when login security is disabled.  This keeps users from having to add the path to an autoexec.bat file or startup.rc/*.bat file.

8 The Dos directory is the directory in which the Dos operating system is installed.

9 File attribute controls specify which userid's have permission to use the SECURIX chmod program.  If a user tries to use this program who is not listed in the chmod permission file, then they will be unable to change file permissions.  See the chmod manual page for more information.

10 The leave defaults option allows a user to specify the message that is displayed when using the leave program.  The leave program is a TSR program which acts as an alarm clock.  Also the number of bells to sound is specified.  See the leave manual page for more information.

11 The login security option is where login security is enabled and disabled.  Disabling login security allows users to access the system without having to enter a valid userid and password.

12 The mount default command specifies whether or not to use the Dos subst command or the Dos join command for the SECURIX mnt command. See the mnt manual page for more information.

13 Password length restrictions require that all new passwords must be of a certain length.

14 Printer setup allows for the default configuration of the print queue.  Through this option, the default print device, the default print buffer size, and the default queue size can be specified.  See the lp, lpq, and lprm manual pages for more information.

15 Superuser access controls specify which user identifications have superuser access privledges.  Users with these privledges can access all of the SECURIX programs.

16 System authentication specifies whether SECURIX should authenticate special system files at boot time.  See the authchk manual page for more information.

17 System name specifies the system name of the system.

18 Time delay after invalid login specifies the number of seconds to wait after an invalid login attempt before prompting for a userid again.

19 Timezone specifies the timezone that the SECURIX PC is located in.  Currently, the valid timezones are EST, CST, MST, and PST.  These may be changed by modifying the /etc/timezone file in SECURIX installation directory.  See the timezone manual page for more information.

