@ECHO OFF
REM  ***********************************************************************
REM  This file contains those commands that are to be executed upon startup
REM  startup of the computer system.  These commands perform activities
REM  related to cleaning the disk system.
REM
REM  Copyright 1992 Paul Walters
REM  ***********************************************************************

IF NOT EXIST \TMP\*.* GOTO LABEL

CD \TMP
IF NOT EXIST *.* GOTO LABEL
ATTRIB -R *.*
RM -s *.*
:LABEL



