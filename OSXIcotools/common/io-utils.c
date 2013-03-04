/* io-utils.c - Various utilities dealing with files.
 *
 * Copyright (C) 1998 Oskar Liljeblad
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif
/* As recommended by autoconf for AC_HEADER_DIRENT */
#if HAVE_DIRENT_H
# include <dirent.h>
# define NAMLEN(dirent) strlen((dirent)->d_name)
#else
# define dirent direct
# define NAMLEN(dirent) (dirent)->d_namlen
# if HAVE_SYS_NDIR_H
#  include <sys/ndir.h>
# endif
# if HAVE_SYS_DIR_H
#  include <sys/dir.h>
# endif
# if HAVE_NDIR_H
#  include <ndir.h>
# endif
#endif
/* As recommended by autoconf for AC_HEADER_SYS_WAIT */
#include <sys/types.h>
#if HAVE_SYS_WAIT_H
# include <sys/wait.h>
#endif
#ifndef WEXITSTATUS
# define WEXITSTATUS(stat_val) ((unsigned)(stat_val) >> 8)
#endif
#ifndef WIFEXITED
# define WIFEXITED(stat_val) (((stat_val) & 255) == 0)
#endif
# include <sys/time.h>		/* Gnulib/POSIX */
# include <time.h>		/* Gnulib/POSIX */
/* POSIX */
#include <unistd.h>		/* Gnulib/POSIX */
#include <fcntl.h>		/* Gnulib/POSIX */
#include <sys/stat.h>		/* Gnulib/POSIX */
#include <stdio.h>		/* C89 */
#include <stdlib.h>		/* C89 */
#include <errno.h>		/* C89 */
#include "xalloc.h"		/* Gnulib */
#include "xvasprintf.h"		/* Gnulib */
#include "strbuf.h"		/* common */
#include "error.h"		/* common */
#include "string-utils.h"	/* common */
#include "llist.h"		/* common */

/**
 * Return true if the file exists, even if it may be a symbolic
 * link that refers to a non-existing file.
 */
bool
file_exists(const char *file)
{
#if HAVE_LSTAT64
    struct stat64 statbuf;
    return (lstat64(file, &statbuf) != -1);
#else
    struct stat statbuf;
    return (stat(file, &statbuf) != -1);
#endif
}

/**
 * Do a stat on the specified file, and return it's mode.
 * If there is an error when statting the file, return 0.
 */
mode_t
stat_mode(const char *file)
{
	struct stat statbuf;
	return (stat(file, &statbuf) == -1 ? (mode_t) 0 : statbuf.st_mode);
}

/**
 * Do a stat on the specified file, and return it's size.
 * If there is an error when statting the file, return -1.
 * FIXME. this should really return into a size_t variable.
 */
off_t
file_size(const char *file)
{
	struct stat statbuf;
	return (stat(file, &statbuf) == -1 ? (off_t) -1 : statbuf.st_size);
}

/**
 * Read a line into a new string.
 * If end of file is reached or an error occurs, NULL is returned.
 * To distinguish between these two results, call ferror. If ferror
 * returns FALSE, then the end of file has been reached.
 *
 * @returns
 *   The null-terminated string read, including newline character.
 *   This string should be free'd when no longer used.
 * @param in
 *   File to read from.
 */
#if 0
char *
read_line(FILE *in)
{
	char *out = NULL;
	size_t size = 0;

	clearerr(in);
	if (getline(&out, &size, in) < 0)
		return NULL;

	return out;
}
#endif

#if 0
/**
 * Create a temporary file. This file will be created with
 * mode 0600 in the temporary directory unless `base' is an
 * absolute filename. The temporary directory is determined
 * by these steps:
 *
 *   * The directory specified by the TMPDIR environment variable,
 *     unless the program is SUID or SGID enabled.
 *   * The value of the P_tmpdir macro.
 *   * The directory `/tmp'.
 *
 * The temporary file will be created with `mkstemp', and
 * changed to mode 0600.
 *
 * @params base
 *   Basename for the temporary file, or "file" if NULL.
 * @returns
 *   The name of the temporary file created, which should be
 *   freed when no longer used. NULL is returned if there was
 *   an error (errno will contain an error code).
 */
char *
create_temporary_file(const char *base)
{
	char *tmpdir;
	char *name;
	int fd;

	if (base == NULL)
		base = "file";
	if (base[0] != '/') {
		tmpdir = NULL;
		if (getuid() == geteuid() && getgid() == getegid())
			tmpdir = getenv("TMPDIR");
		if (tmpdir == NULL)
			tmpdir = P_tmpdir;
		if (tmpdir == NULL)
			tmpdir = "/tmp";
		name = xasprintf("%s/%sXXXXXX", tmpdir, base);
	} else {
		name = xasprintf("%sXXXXXX", base);
	}

	fd = mkstemp(name);
	if (fd < 0)
		return NULL;
	if (fchmod(fd, 0600) < 0)
		return NULL;
	if (close(fd) < 0)
		return NULL;

	return name;
}
#endif

#if 0
char *
backticks(const char *program, char *const args[], int *status)
{
	int child_pipe[2];
	int child_pid;
	FILE *pipe_file;
	char *line;
	StrBuf *out;

	if (pipe(child_pipe) == -1)
		return NULL;

	child_pid = fork();
	if (child_pid == -1)
		return NULL;
	if (child_pid == 0) {
		if (close(child_pipe[0]) == -1)
			die_errno(NULL);
		if (close(STDOUT_FILENO) == -1)
			die_errno(NULL);
		if (dup2(child_pipe[1], STDOUT_FILENO) == -1)
			die_errno(NULL);
		execvp(program, args);
		die_errno(NULL);
	}

	if (close(child_pipe[1]) == -1)
		return NULL;
	pipe_file = fdopen(child_pipe[0], "r");
	if (pipe_file == NULL)
		return NULL;

	out = strbuf_new();
	if (out == NULL)
		return NULL;
	while ((line = read_line(pipe_file)) != NULL) {
		strbuf_append(out, line);
		free(line);
	}
	if (errno != 0)
		return NULL;

	if (waitpid(child_pid, status, 0) != child_pid)
		return NULL;
	if (fclose(pipe_file) != 0)
		return NULL;

	return strbuf_free_to_string(out);
}
#endif

/**
 * Read a directory into a glib list. Each list entry should
 * be freed with free and the list with g_list_free.
 * FIXME: provide Iterator directory_iterator(const char *dir)!
 */
LList *
read_directory(const char *dir)
{
	DIR *dp;
	struct dirent *ep;
	LList *files = llist_new();

	dp = opendir(dir);
	if (dp == NULL)
		return NULL;

	while ((ep = readdir(dp)) != NULL)
		llist_add(files, xstrdup(ep->d_name));

#if CLOSEDIR_VOID
    	closedir(dp);
#else
	if (closedir(dp) == -1)
		return NULL;
#endif

	return files;
}

/**
 * Read and discard some number of bytes from a stream.
 */
int
fskip(FILE *file, uint32_t bytes)
{
	for (; bytes > 0; bytes--) {
		if (fgetc(file) == EOF)
			return -1;
	}
	return 0;
}

/**
 * Write a number of bytes of the same value to a stream.
 */
int
fpad(FILE *file, char byte, uint32_t bytes)
{
	for (; bytes > 0; bytes--) {
		if (fwrite(&byte, 1, 1, file) != 1)
			return -1;
	}
	return 0;
}
