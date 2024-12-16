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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utility.h"

#define MAXBUF 256

int main (int argc, char *argv[])

{
	char ch, buf[MAXBUF], fn[MAXPATH], *ptr, *pattern;
	int from_stdin=0, b_arg=1, squeeze=0, screen=23, search=0, start=0;
	int i, done=0, amount, prev_was_blank, cur_is_blank;
	long here;
	FILE *fp;
	union REGS inregs, outregs;


	if (CheckLicense() == -1) {
		Perror (argv[0]);
		exit (1);
	}

	if (argc == 1)
		from_stdin = 1;

	for (b_arg=1; b_arg<argc; ++b_arg) {
		if (argv[b_arg][0] == '-') {
			if (!strncmp(argv[b_arg], "-s", 2))
				squeeze = 1;
			else if ((ptr = strchr(argv[b_arg], 's')) != NULL) {
				squeeze = 1;
				ptr[0] = '\0';
				screen = atoi (&argv[b_arg][1]);
				if (screen <= 0) {
					printf ("Usage: %s [-[size][s]][+lines][+/pattern]", argv[0]);
					exit (1);
				}
			} else {
				screen = atoi (&argv[b_arg][1]);
				if (screen <= 0) {
					printf ("Usage: %s [-[size][s]][+lines][+/pattern]", argv[0]);
					exit (1);
				}
			}
		} else if (argv[b_arg][0] == '+') {
			if (argv[b_arg][1] == '/') {
				pattern = malloc(strlen(argv[b_arg]));
				strcpy (pattern, &argv[b_arg][2]);
				search = 1;
			} else {
				start = atoi(&argv[b_arg][1]);
				if (start < 0) {
					printf ("Usage: %s [-[size][s]][+lines][+/pattern]", argv[0]);
					exit (1);
				}
			}
		} else
			break;
	}

	if (b_arg >= argc)
		from_stdin = 1;

	if (from_stdin) {
		if ((fp = fopen (mkFileName(TMP), "w+")) == NULL) {
			perror (mkFileName(TMP));
			exit (1);
		}

		while ((ch = fgetc(stdin)) != EOF) {
			if (ch == 26 || ch == 3)
				break;
			fputc (ch, fp);
		}
		fclose (fp);
		if ((fp = fopen (mkFileName(TMP), "r")) == NULL) {
			perror (mkFileName(TMP));
			exit (1);
		}
	} else {
		strcpy (fn, argv[b_arg]);
		for (i=0; i<strlen(fn); ++i) {
			if (fn[i] == '/')
				fn[i] = '\\';
		}
		if ((fp = fopen (fn, "r")) == NULL) {
			perror (fn);
			exit (1);
		}
		++b_arg;
	}

	if (search) {
		here = ftell (fp);
		while ((fgets (buf, MAXBUF, fp) != NULL) && (!done)) {
			for (i=0; i<strlen(buf) && !done; ++i) {
				if (!strncmp(&buf[i], pattern, strlen(pattern)))
					done = 1;
			}
			if (!done)
				here = ftell (fp);
		}
		if (!done) {
			printf ("Pattern not found.\n");
			exit (1);
		}
		fseek (fp, here, SEEK_SET);
	} else if (start) {
		for (i=1; i < start; ++i) {
			if (fgets (buf, MAXBUF, fp) == NULL) {
				perror ("more");
				exit (1);
			}
		}
	}

	amount = screen-1;
	done = 0;
	prev_was_blank = cur_is_blank = 0;

	while ( (fgets(buf, MAXBUF, fp) != NULL) && !done ) {
		if (cur_is_blank) {
			cur_is_blank = 0;
			prev_was_blank = 1;
		} else
			prev_was_blank = 0;
		if (!strcmp(buf, "\n"))
			cur_is_blank = 1;

		for (i = 0; i < amount; i++) {
			if (squeeze) {
				if (!cur_is_blank && !prev_was_blank)
					printf ("%s", buf);
			} else {
				printf ("%s", buf);
			}
			if (fgets (buf, MAXBUF, fp) == NULL)
				break;
			if (cur_is_blank) {
				cur_is_blank = 0;
				prev_was_blank = 1;
			} else {
				prev_was_blank = 0;
			}
		}
		if ( i >= amount ) {
			printf ("%s-- MORE --", buf);
			inregs.h.ah = 0;
			int86 (0x16, &inregs, &outregs);
			ch = outregs.h.ah;

			switch (ch) {
			case 0x1c : amount = 0;        // <CR>
					  break;
			case 0x39 : amount = screen-1;   // <SP>
					  break;
			case 0x20 : amount = screen/2; // <D>
					  break;
			case 0x14 : rewind (fp);       // <T>
					  amount = screen;
					  break;
			case 0x31 : fclose (fp);       // <N>
					  if (!from_stdin) {
						  if (b_arg < argc) {
							strcpy (fn, argv[b_arg]);
							for (i=0; i<strlen(fn); ++i) {
								if (fn[i] == '/')
									fn[i] = '\\';
							}
							if ((fp = fopen (fn, "r")) == NULL) {
								perror (fn);
								exit (1);
							}
							++b_arg;
						  } else {
							done = 1;
						  }
					  } else {
						done = 1;
					  }
					  break;
			case 0x10 : amount = 0;        // <Q>
					  done = 1;
					  break;
			}
			printf ("\b\b\b\b\b\b\b\b\b\b          \b\b\b\b\b\b\b\b\b\b");
			fflush(stdout);
		 } else {
			break;
		 }
	}
	fclose (fp);
	return (0);
}
