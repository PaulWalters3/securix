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
#include <errno.h>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include "utility.h"

#define MAXARCHDISK 99
#define MAXBUF 4096
#define MINARCHSIZE 256
#define MINDEVSPACE 1024

struct header {
	char filename[MAXPATH];
	long start;
	long size;
	long date;
	int  disk_no;
	char skip;
};

char fn_list[MAXPATH];
int  append_to_device (char *fn, int start, int argc, char *argv[], int *flag);
int  find_end_of_arch (char *fn, int flag);
int  close_device (char *fn);
long get_free_space (int disk, char *fn, int flag);
int  get_num_files (char *argv[], int argc, int start);
int  list_files (char *fn, int start, int argc, char *argv[], int flag);
int  write_to_device (char *fn, int start, int argc, char *argv[], int *flag);
int  xtract_from_device (char *fn, int start, int argc, char *argv[], int flag);

int attr = FA_DIREC+FA_HIDDEN+FA_RDONLY+FA_LABEL+FA_SYSTEM+FA_ARCH;
int sil=0, rm=0, rm_flag=0, prmpt;
long size=-1L;
int read_from_list=0;

int main (int argc, char *argv[])

{
	FILE *fp;
	char fn[MAXPATH], msg[10];
	int start_arg = 2;
	int append=0;
	int create=0, table=0, xtract=0;
	int file=0, sz=0, flag=0, pr=0;
	struct ffblk ff;

#ifdef DEBUG
printf ("STARTING ARCH.\n");
#endif

	if (CheckLicense() == -1) {
		Perror (argv[0]);
		exit (1);
	}

	if (argc < 2) {
		printf ("Usage (1): %s {actx}[f][k][l][o][p][s] [file|device] [bytes] [list] [file...]\n", argv[0]);
		exit (1);
	}

	if (strchr(argv[1], 'a') != NULL)
		append = 1;
	if (strchr(argv[1], 'c') != NULL)
		create = 1;
	if (strchr(argv[1], 'o') != NULL)
		rm=1;
	if (strchr(argv[1], 'p') != NULL)
		pr=1;
	if (strchr(argv[1], 't') != NULL)
		table = 1;
	if (strchr(argv[1], 'x') != NULL)
		xtract = 1;
	if (strchr(argv[1], 's') != NULL)
		sil = 1;
	if (strchr(argv[1], 'f') != NULL) {
		file = 1;
		++start_arg;
	}
	if (strchr(argv[1], 'k') != NULL) {
		sz = 1;
		++start_arg;
	}
	if (strchr(argv[1], 'l') != NULL) {
		read_from_list = 1;
		++start_arg;
	}
	if (strlen(argv[1]) > (append+create+table+xtract+file+sz+sil+rm+pr+read_from_list)) {
		printf ("Usage (2): %s {actx}[f][k][l][o][p][s] [file|device] [bytes] [list] [file...]\n", argv[0]);
		exit (1);
	}

	if (!append && !create && !xtract && !table) {
		printf ("Usage (3): %s {actx}[f][k][l][o][p][s] [file|device] [bytes] [list] [file...]\n", argv[0]);
		exit (1);
	}

	if (rm)
		rm_flag = 1;

	if (pr)
		prmpt = 1;

	if (read_from_list) {
		if (file && sz)
			strcpy (fn_list, argv[4]);
		else if (file || sz)
			strcpy (fn_list, argv[3]);
		else
			strcpy (fn_list, argv[2]);
	}

	if (sz) {
		if (file)
			size = atol (argv[3]);
		else
			size = atol (argv[2]);

		if (size < MINARCHSIZE) {
			errno = EMINARCHSIZE;
			sprintf (msg, "%d", MINARCHSIZE);
			perror (msg);
			exit (1);
		}
	}

	/*****
	Get file|device name
	*****/
	if (!file) {
		if ((fp = fopen (mkFileName(ARCHDEFAULT), "rb")) == NULL) {
			errno = ENOARCHDEFAULT;
			Perror (mkFileName(ARCHDEFAULT));
			exit (1);
		}
		fscanf (fp, "%s", fn);
		fclose (fp);
	} else {
		strcpy (fn, argv[2]);
	}
	unix_to_dos_fn (fn);

	if ((strlen(fn) == 2) && (fn[1] == ':')) {
		strcat (fn, ARCHFILE);
		flag = 1;
	} else {

#ifdef DEBUG
printf ("main, fn %s\n", fn);
#endif

		if (!findfirst (fn, &ff, attr)) {

#ifdef DEBUG
printf ("main findfirst, fn %s, ff %s\n", fn, ff.ff_name);
#endif

			if (ff.ff_attrib & FA_DIREC) {
				if (fn[strlen(fn)] == '\\')
					strcat (fn, ARCHFILE);
				else
					sprintf (&fn[strlen(fn)], "\\%s", ARCHFILE);
				flag = 1;
				rm_flag = 0;
			}
		}

#ifdef DEBUG
printf ("MAIN flag=%d\n", flag);
#endif

	}

	attr -= FA_DIREC;

	if (create) {
		if (write_to_device (fn, start_arg, argc, argv, &flag) == -1)
			exit (1);

	} else if (xtract) {
		if (xtract_from_device (fn, start_arg, argc, argv, flag) == -1)
			exit (1);

	} else if (table) {
		if (list_files (fn, start_arg, argc, argv, flag) == -1)
			exit (1);
	} else if (append) {
		if (append_to_device (fn, start_arg, argc, argv, &flag) == -1)
			exit (1);
	}
	return 0;
}

/**********************************************************/

int  append_to_device (char *fn, int start, int argc, char *argv[], int *flag)
{
	char buf[MAXBUF], list_name[MAXPATH];
	char drive[MAXDRIVE], directory[MAXDIR], extension[MAXEXT];
	char name[MAXFILE], fname[MAXPATH];
	FILE *SRC, *DEST, *HEADER, *LIST;
	int i, j, num, amt, disk_no=1, on_disk=0, head_size=0, done, where, orig_num;
	long avail_space, filebytes=0L, bytestotal=0L;
	struct header info;
	struct stat statbuf;
	struct ffblk ff;

	disk_no = find_end_of_arch (fn, *flag);

	/**********
	Save header area.
	**********/
	/**
	if (flag)
		sprintf (&fn[strlen(fn)], "%02d.fmt", disk_no);
	**/

	_chmod (fn, 1, 0);
	if ((DEST = fopen (fn, "rb+")) == NULL) {
		Perror (fn);
		return (-1);
	}

	if ((HEADER = fopen (mkFileName(ARCHTEMP), "wb")) == NULL) {
		Perror ("arch");
		fclose (DEST);
		_chmod (fn, 1, FA_RDONLY);
		return (-1);
	}

	/****
	Get number of files on disk
	*****/
	fseek (DEST, -((long) sizeof(int)), SEEK_END);
	fread ((int *) &num, sizeof(int), 1, DEST);
	fseek (DEST, -(long) (sizeof(int) + (num * sizeof(struct header))), SEEK_END);
	orig_num = num;

	for (i=0; i<num; ++i) {
		fread ((struct header *) &info, sizeof(struct header), 1, DEST);
		fwrite ((struct header *) &info, sizeof(struct header), 1, HEADER);
	}
	bytestotal = info.start + info.size;
	fseek (DEST, -(long) (sizeof(int) + (num * sizeof(struct header))), SEEK_END);

	/****
	Get available disk space.
	****/
	rm_flag = 0;
	if (fn[1] == ':')
		avail_space = get_free_space (toupper(fn[0])-'A'+1, fn, *flag);
	else
		avail_space = get_free_space (-1, fn, *flag);

	/**
	if (*flag)
		++(*flag);
	**/

	/****
	Initialize header file and datafile.
	****/
	num += get_num_files (argv, argc, start);
	head_size = sizeof(struct header) * num;
	avail_space -= head_size;
	avail_space += bytestotal;

	if (read_from_list) {
		unix_to_dos_fn (fn_list);
		if ((LIST = fopen (fn_list, "r")) == NULL) {
			Perror (fn);
			fclose (HEADER);
			fclose (DEST);
			_chmod (fn, 1, FA_RDONLY);
			return (-1);
		}
	}
	on_disk = orig_num;

#ifdef DEBUG
printf ("append_to_device, (%d), orig on_disk=%d\n", __LINE__, on_disk);
#endif

	for (i = start, j = 1;
		read_from_list ? (fgets(list_name, MAXPATH, LIST) != NULL):(i<argc);
		++i, ++j) {

		if (read_from_list) {
			if (list_name[strlen(list_name)-1] == '\n')
				list_name[strlen(list_name)-1] = '\0';
			unix_to_dos_fn (list_name);
			fnsplit (list_name, drive, directory, name, extension);
			done = findfirst (list_name, &ff, attr);
		} else {
			unix_to_dos_fn (argv[i]);
			fnsplit (argv[i], drive, directory, name, extension);
			done = findfirst (argv[i], &ff, attr);
		}

		while (!done) {
#ifdef DEBUG
printf ("TOP OF WHILE.\n");
printf ("bytes total = %ld\n", bytestotal);
printf ("avail space = %ld\n", avail_space);
#endif

			sprintf (fname, "%s%s%s", drive, directory, ff.ff_name);

			if ((SRC = fopen(fname, "rb")) == NULL) {
				Perror (fname);
				continue;
			}
			stat (fname, &statbuf);
#ifdef DEBUG
printf ("Adding file: %s\n", fname);
printf ("Size: %ld\n", statbuf.st_size);
#endif
			strcpy (info.filename, fname);
			info.start = bytestotal;
			info.size  = statbuf.st_size;
			info.date  = statbuf.st_mtime;
			info.disk_no = disk_no;
			++on_disk;

			if ((bytestotal + info.size) > avail_space) {
				info.size = avail_space - bytestotal;
				info.skip = 1;
				if (!sil) {
					printf ("%-25.25s %-ld bytes SEEK=%ld\n", info.filename, info.size, info.start);
					printf ("This file will be split across volumes.\n");
				}
			} else {
				info.skip = 0;
				if (!sil)
					printf ("%-25.25s %-ld bytes SEEK=%ld\n", info.filename, info.size, info.start);
			}

			fwrite ((struct header *) &info, sizeof(struct header), 1, HEADER);

			filebytes = 0L;

			while (!feof(SRC)) {
				amt = fread ((char *) buf, sizeof(char), MAXBUF, SRC);
				filebytes += amt;
				bytestotal += amt;

				if (bytestotal > avail_space) {
#ifdef DEBUG
printf ("Skipping to next file.\n");
printf ("bytestotal=%ld\navail_space=%ld\n", bytestotal, avail_space);
printf ("filebytes=%ld\namt=%d\n", filebytes, amt);
printf ("statbuf=%ld\n", statbuf.st_size);
#endif
					fwrite ((char *) buf, sizeof(char), amt-((int)(bytestotal-avail_space)), DEST);
					where = amt-((int)(bytestotal-avail_space));
					fclose (DEST);
					fwrite ((int *) &on_disk, sizeof(int), 1, HEADER);
					fclose (HEADER);

					if (close_device (fn) == -1)
						return (-1);

					amt = (int) (bytestotal-avail_space);

#ifdef DEBUG
printf ("Changing amt to %d\n", amt);
#endif
					filebytes    = amt;
					info.size    = statbuf.st_size - info.size;
					info.disk_no = ++disk_no;
#ifdef DEBUG
printf ("HERE. filebytes=amt=%d\n", amt);
printf ("info.size=%ld\nstatbuf=%ld\n", info.size, statbuf.st_size);
#endif
					if (disk_no > MAXARCHDISK) {
						errno = EMARCHDSK;
						Perror ("arch");
						return (-1);
					}
					info.start   = bytestotal = 0L;
					bytestotal  += amt;
					on_disk      = 1;

#ifdef DEBUG
printf ("filebytes = amt = %d\n", amt);
printf ("info.size = %ld\n", info.size);
printf ("statbuf.st_size = %ld\n", statbuf.st_size);
#endif

					if (!prmpt) {
						printf ("Reached end of device.  Insert new volume and press any key to continue.\n%c", 7);
						getch();
					}

					if (flag)
						sprintf (&fn[strlen(fn)-6], "%02d.fmt", disk_no);

					if (rm)
						rm_flag = 1;

					if (fn[1] == ':')
						avail_space = get_free_space (toupper(fn[0])-'A'+1, fn, *flag);
					else
						avail_space = get_free_space (-1, fn, *flag);
					avail_space -= head_size;
					if (avail_space < 0) {
						errno = ENOSPACE;
						Perror ("arch");
						return (-1);
					}
#ifdef DEBUG
printf ("AFTER CALL TO AVAIL SPACE\n");
printf ("avail_space = %ld\nhead_size=%ld\n", avail_space, head_size);
printf ("filebytes = amt = %d\n", amt);
printf ("info.size = %ld\n", info.size);
printf ("statbuf.st_size = %ld\n", statbuf.st_size);
printf ("bytestotal = %ld\n", bytestotal);
#endif

					_chmod (mkFileName(ARCHTEMP), 1, 0);
					if ((HEADER = fopen (mkFileName(ARCHTEMP),"wb")) == NULL) {
						Perror (mkFileName(ARCHTEMP));
						return (-1);
					}

					_chmod (fn, 1, 0);
					if ((DEST = fopen (fn, "wb")) == NULL) {
						Perror (fn);
						fclose (HEADER);
						return (-1);
					}

					bytestotal = amt;
					if ((bytestotal + info.size) > avail_space) {
						info.size = avail_space - bytestotal;
						info.skip = 1;
						if (!sil) {
							printf ("%-25.25s %-ld bytes SEEK=%ld\n", info.filename, info.size, info.start);
							printf ("This file will be split across volumes.\n");
						}
					} else {
						info.skip = 0;
						if (!sil)
							printf ("%-25.25s %-ld bytes SEEK=%ld\n", info.filename, info.size, info.start);
					}
					fwrite ((struct header *) &info, sizeof(struct header), 1, HEADER);
					fwrite ((char *) &buf[where], sizeof(char), amt, DEST);
					continue;
				}
				fwrite ((char *) buf, sizeof(char), amt, DEST);
			}
			fclose (SRC);
			done = findnext(&ff);
		}
	}

#ifdef DEBUG
printf ("append_to_device, (%d), on_disk=%d\n", __LINE__, on_disk);
#endif

	fwrite ((int *) &on_disk, sizeof(int), 1, HEADER);
	fclose (HEADER);
	fclose (DEST);
	fclose (LIST);

	if (close_device (fn) == -1)
		return (-1);

	return (0);
}

/**********************************************************/

int find_end_of_arch (char *fn, int flag)
{
	char look_for[MAXPATH];
	char drive[MAXDRIVE], directory[MAXDIR], file[MAXFILE], extension[MAXEXT];
	FILE *fp;
	int end_of_arch=0, disk_no=0, num, i;
	struct header info;
	struct ffblk ff;

	while (!end_of_arch) {
		++disk_no;
		printf ("Please insert volume %02d of archive into drive and press a key when ready.\n%c", disk_no, 7);
		getch ();

		if (flag) {
			if (fn[1] == ':')
				sprintf (look_for, "%c:%s%02d.fmt", fn[0], ARCHFILE, disk_no);
			else
				sprintf (look_for, "%s%02d.fmt", disk_no, fn);

			unix_to_dos_fn (look_for);
			fnsplit (look_for, drive, directory, file, extension);

			if (findfirst (look_for, &ff, attr)) {
				errno = EARCHNOTBEGIN;
				Perror ("arch");
				return (0);
			}
			sprintf (fn, "%s%s%s", drive, directory, ff.ff_name);
		}

		if ((fp = fopen(fn, "rb")) == NULL) {
			Perror (fn);
			return (0);
		}

		/****
		Get number of files on disk
		*****/
		fseek (fp, -((long) sizeof(int)), SEEK_END);
		fread ((int *) &num, sizeof(int), 1, fp);
		fseek (fp, -(long) (sizeof(int) + (num * sizeof(struct header))), SEEK_END);

		for (i = 0; i < num; ++i) {
			fread ((struct header *) &info, sizeof(struct header), 1, fp);
		}
		fclose (fp);
		if (!info.skip)
			end_of_arch = 1;
	}
	return (disk_no);
}

/**********************************************************/

long get_free_space (int disk, char *fn, int flag)
{
	static int drv;
	char look_for[MAXPATH];
	char drive[MAXDRIVE], directory[MAXDIR], file[MAXFILE], extension[MAXEXT];
	int done;
	long avail;
	struct dfree free;
	struct ffblk ff;

	/****
	Erase all occurances of previous archives from disk.
	****/
	if (flag) {
#ifdef DEBUG
printf ("get_free_disk_space, fn = %s\n", fn);
#endif
		if (fn[1] == ':')
			sprintf (look_for, "%c:%s??.fmt", fn[0], ARCHFILE);

		else if (flag == 1)
			sprintf (look_for, "%s??.fmt", fn);

		if (flag <= 1) {

			unix_to_dos_fn (look_for);
			fnsplit (look_for, drive, directory, file, extension);

#ifdef DEBUG
printf ("get_free_disk_space, (%d), look_for = %s\n", __LINE__, look_for);
#endif

			done = findfirst (look_for, &ff, attr);

#ifdef DEBUG
Perror ("get_free_disk_space");
printf ("get_free_disk_space (%d), ff.ff_name = %s, done = %d\n", __LINE__, ff.ff_name, done);
#endif

			while (!done) {
				sprintf (look_for, "%s%s%s", drive, directory, ff.ff_name);
				if (rm_flag) {
					_chmod (look_for, 1, 0);
					unlink (look_for);
				}
				done = findnext (&ff);
			}
		}
	} else {
		unix_to_dos_fn (fn);
		if (rm_flag) {
			_chmod (fn, 1, 0);
			unlink (fn);
		}
	}

	if (disk != -1)
		drv = disk;
	else
		drv = getdisk()+1;

	getdfree(drv, &free);

#ifdef DEBUG
printf ("DISK FREE %d\n", drv);
printf ("curdisk = %d\n", getdisk()+1);
#endif

	if (free.df_sclus == 0xFFFF) {
		printf ("Cannot determine device size.\n");
		exit (1);
	}

	avail = (long) free.df_avail * (long) free.df_bsec * (long) free.df_sclus;

#ifdef DEBUG
printf ("AVAILABLE FREE SPACE : %ld\nsize=%ld", avail, size);
#endif

	if (avail < MINDEVSPACE) {
		errno = ENOSPACE;
		Perror ("");
		exit (1);
	}

	if ((size >= MINARCHSIZE ) && (size < avail))
		avail = size;
	else
		avail -= MINDEVSPACE;


	return (avail);
}
	
/**********************************************************/

int  get_num_files (char *argv[], int argc, int start)
{
	char buf[MAXPATH];
	FILE *fp;
	int done, i, count=0;
	struct ffblk ff;

	if (read_from_list) {

#ifdef DEBUG
printf ("read from list: %s\n", fn_list);
#endif

		unix_to_dos_fn (fn_list);
		if ((fp = fopen (fn_list, "r")) == NULL) {
			Perror (fn_list);
			return (0);
		}
#ifdef DEBUG
printf ("Getting number of files from %s.\n", fn_list);
#endif
		while (fgets (buf, MAXPATH, fp) != NULL) {
			if (buf[strlen(buf)-1] == '\n')
				buf[strlen(buf)-1] = '\0';
			unix_to_dos_fn (buf);

			done = findfirst (buf, &ff, attr);
			while (!done) {
				++count;
				done = findnext (&ff);
			}
		}
		fclose (fp);

#ifdef DEBUG
printf ("Number of files to write: %d\n", count);
#endif
		return (count);
	}
	for (i = start; i < argc; ++i) {
		unix_to_dos_fn (argv[i]);
		done = findfirst (argv[i], &ff, attr);
		while (!done) {
			++count;
			done = findnext (&ff);
		}
	}
	return (count);
}

/**********************************************************/

int list_files (char *fn, int start, int argc, char *argv[], int flag)
{
	char look_for[MAXPATH], dt_buf[30];
	char drive[MAXDRIVE], directory[MAXDIR], file[MAXFILE], extension[MAXEXT];
	FILE *fp;
	int i, j, all=1, match, num;
	struct header info;
	struct ffblk ff;

	if (flag) {
		if (fn[1] == ':')
			sprintf (look_for, "%c:%s??.fmt", fn[0], ARCHFILE);
		else
			sprintf (look_for, "%s??.fmt", fn);

		unix_to_dos_fn (look_for);
		fnsplit (look_for, drive, directory, file, extension);

		if (findfirst (look_for, &ff, attr)) {
			Perror ("arch");
			return (-1);
		}
		sprintf (fn, "%s%s%s", drive, directory, ff.ff_name);
	}

	if ((fp = fopen(fn, "rb")) == NULL) {
		Perror (fn);
		return (-1);
	}

	/****
	Are we looking for specific files?
	****/
	if (start < argc)
		all = 0;

	/****
	Get number of files on disk
	*****/
	fseek (fp, -((long) sizeof(int)), SEEK_END);
	fread ((int *) &num, sizeof(int), 1, fp);
	fseek (fp, -(long) (sizeof(int) + (num * sizeof(struct header))), SEEK_END);

#ifdef DEBUG
printf ("list_files (%d), num=%d\n", __LINE__, num);
#endif

	for (i = 0; i < num; ++i) {
		fread ((struct header *) &info, sizeof(struct header), 1, fp);

		if (!all) {
			for (j = start; j < argc; ++j) {
				match = 0;
				unix_to_dos_fn (argv[j]);
				if (!strcmp (argv[j], info.filename)) {
					match = 1;
					break;
				}
			}
			if (!match)
				continue;
		}

		if (info.skip)
			i = num;

		strcpy (dt_buf, &ctime(&info.date)[4]);
		dt_buf[12] = '\0';

		if (!sil)
			printf ("%-25.25s %-10ld %s\n", info.filename, info.size, dt_buf);
	}

	fclose (fp);
	return (0);
}

/**********************************************************/

int  write_to_device (char *fn, int start, int argc, char *argv[], int *flag)
{
	char buf[MAXBUF], list_name[MAXPATH];
	char drive[MAXDRIVE], directory[MAXDIR], extension[MAXEXT];
	char name[MAXFILE], fname[MAXPATH];
	FILE *SRC, *DEST, *HEADER, *LIST;
	int i, j, num, amt, disk_no=1, on_disk=0, head_size=0, done, where;
	long avail_space, filebytes=0L, bytestotal=0L;
	struct header info;
	struct stat statbuf;
	struct ffblk ff;

	/****
	Get available disk space.
	****/
	if (fn[1] == ':')
		avail_space = get_free_space (toupper(fn[0])-'A'+1, fn, *flag);
	else
		avail_space = get_free_space (-1, fn, *flag);

#ifdef DEBUG
printf ("%ld %c\n", avail_space, fn[1]);
#endif

	/**
	if (*flag)
		++(*flag);
	**/

	/****
	Initialize header file and datafile.
	****/
	num = get_num_files (argv, argc, start);

#ifdef DEBUG
printf ("NUM=%d struct header = %d\n", num, sizeof(struct header));
#endif

	head_size = sizeof(struct header) * num;

#ifdef DEBUG
printf ("avail_space =%ld   head_size=%d  num=%d\n", avail_space, head_size, num);
#endif

	avail_space -= (long) head_size;

#ifdef DEBUG
printf ("avail_space =%ld   head_size=%d  num=%d\n", avail_space, head_size, num);
#endif

	_chmod (mkFileName(ARCHTEMP), 1, 0);
	if ((HEADER = fopen (mkFileName(ARCHTEMP), "wb")) == NULL) {
		Perror (mkFileName(ARCHTEMP));
		return (-1);
	}

#ifdef DEBUG
printf ("write_to_device, fn=%s\n", fn);
#endif

	if (*flag)
		sprintf (&fn[strlen(fn)], "%02d.fmt", disk_no);

#ifdef DEBUG
printf ("write_to_device (2), fn=%s\n", fn);
#endif

	_chmod (fn, 1, 0);
	if ((DEST = fopen (fn, "wb")) == NULL) {
		Perror (fn);
		fclose (HEADER);
		return (-1);
	}

	if (read_from_list) {
		unix_to_dos_fn (fn_list);
		if ((LIST = fopen (fn_list, "r")) == NULL) {
			Perror (fn);
			fclose (HEADER);
			fclose (DEST);
			return (-1);
		}
	}
	on_disk = 0;

	for (i = start, j = 1;
		read_from_list ? (fgets(list_name, MAXPATH, LIST) != NULL):(i<argc);
		++i, ++j) {

		if (read_from_list) {
			if (list_name[strlen(list_name)-1] == '\n')
				list_name[strlen(list_name)-1] = '\0';
			unix_to_dos_fn (list_name);
			fnsplit (list_name, drive, directory, name, extension);
			done = findfirst (list_name, &ff, attr);
		} else {
			unix_to_dos_fn (argv[i]);
			fnsplit (argv[i], drive, directory, name, extension);
			done = findfirst (argv[i], &ff, attr);
		}

		while (!done) {
			sprintf (fname, "%s%s%s", drive, directory, ff.ff_name);

			if ((SRC = fopen(fname, "rb")) == NULL) {
				Perror (fname);
				continue;
			}
			stat (fname, &statbuf);
			strcpy (info.filename, fname);
			info.start = bytestotal;
			info.size  = statbuf.st_size;
			info.date  = statbuf.st_mtime;
			info.disk_no = disk_no;
			++on_disk;

			if ((bytestotal + info.size) > avail_space) {
				info.size = avail_space - bytestotal;
				info.skip = 1;
				if (!sil) {
					printf ("%-25.25s %-ld bytes SEEK=%ld\n", info.filename, info.size, info.start);
					printf ("This file will be split across volumes.\n");
				}
			} else {
				info.skip = 0;
				if (!sil)
					printf ("%-25.25s %-ld bytes SEEK=%ld\n", info.filename, info.size, info.start);
			}

			fwrite ((struct header *) &info, sizeof(struct header), 1, HEADER);

			filebytes = 0L;

			while (!feof(SRC)) {
				amt = fread ((char *) buf, sizeof(char), MAXBUF, SRC);
				filebytes += amt;
				bytestotal += amt;

				if (bytestotal > avail_space) {
					fwrite ((char *) buf, sizeof(char), amt-((int)(bytestotal-avail_space)), DEST);
					where = amt-((int)(bytestotal-avail_space));
					fclose (DEST);
					fwrite ((int *) &on_disk, sizeof(int), 1, HEADER);
					fclose (HEADER);

					if (close_device (fn) == -1)
						return (-1);

					amt = (int) (bytestotal-avail_space);

					filebytes    = amt;
					info.size    = statbuf.st_size - info.size;
					info.disk_no = ++disk_no;
					if (disk_no > MAXARCHDISK) {
						errno = EMARCHDSK;
						Perror ("arch");
						return (-1);
					}
					info.start   = bytestotal = 0L;
					bytestotal  += amt;
					on_disk      = 1;

					if (!prmpt) {
						printf ("Reached end of device.  Insert new volume and press any key to continue.\n%c", 7);
						getch();
					}

					if (flag)
						sprintf (&fn[strlen(fn)-6], "%02d.fmt", disk_no);

					if (fn[1] == ':')
						avail_space = get_free_space (toupper(fn[0])-'A'+1, fn, *flag);
					else
						avail_space = get_free_space (-1, fn, *flag);
					avail_space -= head_size;

					_chmod (mkFileName(ARCHTEMP), 1, 0);
					if ((HEADER = fopen (mkFileName(ARCHTEMP),"wb")) == NULL) {
						Perror (mkFileName(ARCHTEMP));
						return (-1);
					}

					_chmod (fn, 1, 0);
#ifdef DEBUG
printf ("Opening fn %s\n", fn);
#endif

					if ((DEST = fopen (fn, "wb")) == NULL) {
						Perror (fn);
						fclose (HEADER);
						return (-1);
					}

					bytestotal = amt;
					if ((bytestotal + info.size) > avail_space) {
						info.size = avail_space - bytestotal;
						info.skip = 1;
						if (!sil) {
							printf ("%-25.25s %-ld bytes SEEK=%ld\n", info.filename, info.size, info.start);
							printf ("This file will be split across volumes.\n");
						}
					} else {
						info.skip = 0;
						if (!sil)
							printf ("%-25.25s %-ld bytes SEEK=%ld\n", info.filename, info.size, info.start);
					}
					fwrite ((struct header *) &info, sizeof(struct header), 1, HEADER);
					fwrite ((char *) &buf[where], sizeof(char), amt, DEST);
					continue;
				}
				fwrite ((char *) buf, sizeof(char), amt, DEST);
			}
			fclose (SRC);
			done = findnext(&ff);
		}
	}

#ifdef DEBUG
printf ("write_to_device, (%d), on_disk=%d\n", __LINE__, on_disk);
#endif

	fwrite ((int *) &on_disk, sizeof(int), 1, HEADER);
	fclose (HEADER);
	fclose (DEST);
	fclose (LIST);

	if (close_device (fn) == -1)
		return (-1);

	return (0);
}

/**********************************************************/

int close_device (char *fn)
{
	char buf[MAXBUF];
	FILE *HEADER, *SRC;
	int amt;

	if ((HEADER = fopen (mkFileName(ARCHTEMP), "rb")) == NULL) {
		Perror (mkFileName(ARCHTEMP));
		return (-1);
	}
	if ((SRC = fopen (fn, "ab")) == NULL) {
		Perror (fn);
		fclose (HEADER);
		return (-1);
	}
	while (!feof(HEADER)) {
		amt = fread ((char *) buf, sizeof(char), MAXBUF, HEADER);
		fwrite ((char *) buf, sizeof(char), amt, SRC);
	}
	fclose (HEADER);
	fclose (SRC);
	unlink (mkFileName(ARCHTEMP));
	_chmod (fn, 1, FA_RDONLY);
	return (0);
}

/**********************************************************/

int  xtract_from_device (char *fn, int start, int argc, char *argv[], int flag)
{
	char look_for[MAXPATH];
	char drive[MAXDRIVE], directory[MAXDIR], file[MAXFILE], extension[MAXEXT];
	char buf[MAXBUF], prev[MAXPATH];
	FILE *SRC, *DEST;
	int  i, j, all=1, num, match, amt, disk_no = 1;
	long pos, bytesread;
	struct header info;
	struct ffblk ff;
#ifdef DEBUG
struct header temp;
#endif

	if (flag) {
		if (fn[1] == ':')
			sprintf (look_for, "%c:%s*.*", fn[0], ARCHFILE);
		else
			sprintf (look_for, "%s*.*", fn);

		unix_to_dos_fn (look_for);
		fnsplit (look_for, drive, directory, file, extension);

		if (findfirst (look_for, &ff, attr)) {
			Perror ("arch");
			return (-1);
		}
		sprintf (fn, "%s%s%s", drive, directory, ff.ff_name);
	}

	if ((SRC = fopen (fn, "rb")) == NULL) {
		Perror (fn);
		return (-1);
	}

	strcpy (prev, "");

	/****
	Get number of files on disk
	*****/
	fseek (SRC, -((long) sizeof(int)), SEEK_END);
	fread ((int *) &num, sizeof(int), 1, SRC);
	fseek (SRC, -(long) (sizeof(int) + (num * sizeof(struct header))), SEEK_END);

	/****
	Are we looking for specific files?
	****/
	if (start < argc)
		all = 0;

	for (i = 0; i < num; ++i) {
		fread ((struct header *) &info, sizeof(struct header), 1, SRC);
		pos = ftell (SRC);

		if (!all) {
			for (j = start; j < argc; ++j) {
				match = 0;
				unix_to_dos_fn (argv[j]);
				if (!strcmp (argv[j], info.filename)) {
					match = 1;
					break;
				}
			}
			if (info.skip && !match) {
				fclose (SRC);

				if (!prmpt) {
					printf ("Reached end of input device.  Insert new volume and.\n");
					printf ("press any key to continue.\n");
					getch ();
				}
				i = -1;
				++disk_no;

				if (flag)
					sprintf (&fn[strlen(fn)-6], "%02d.fmt", disk_no);

				if ((SRC = fopen (fn, "rb")) == NULL) {
					Perror (fn);
					return (-1);
				}
				/****
				Get number of files on disk
				*****/
				fseek (SRC, -((long) sizeof(int)), SEEK_END);
				fread ((int *) &num, sizeof(int), 1, SRC);
				fseek (SRC, -(long) (sizeof(int) + (num * sizeof(struct header))), SEEK_END);
			}
			if (!match)
				continue;
		}

		if (strcmp (prev, info.filename)) {
			strcpy (prev, info.filename);
			if ((DEST = fopen (info.filename, "wb")) == NULL) {
				Perror (info.filename);
				continue;
			}
		}
		fseek (SRC, info.start, SEEK_SET);
		if (!sil)
			printf ("%-25.25s %-ld bytes\n", info.filename, info.size);
		if (info.skip && !sil)
			printf ("File is split across volumes.\n");

		for (bytesread=0L; bytesread < info.size; bytesread += (long)amt) {
			if ((bytesread+MAXBUF) > info.size) {
				amt = fread ((char *) buf, sizeof(char), (int)(info.size-bytesread), SRC);
			} else {
				amt = fread ((char *) buf, sizeof(char), MAXBUF, SRC);
			}
			fwrite ((char *) buf, sizeof(char), amt, DEST);
		}

		if (info.skip) {

#ifdef DEBUG
fread ((struct header *) &temp, sizeof (struct header), 1, SRC);
printf ("xtract, (%d), filename = %s\n", __LINE__, temp.filename);
#endif
			fclose (SRC);

			if (!prmpt) {
				printf ("Reached end of input device.  Insert new volume and\n");
				printf ("press any key to continue.\n");
				getch ();
			}
			i = -1;
			++disk_no;

			if (flag)
				sprintf (&fn[strlen(fn)-6], "%02d.fmt", disk_no);

			if ((SRC = fopen (fn, "rb")) == NULL) {
				Perror (fn);
				return (-1);
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
