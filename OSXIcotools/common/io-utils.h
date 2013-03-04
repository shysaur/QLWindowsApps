/* io-utils.h - Various utilities dealing with files.
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

#ifndef COMMON_IO_UTILS_H
#define COMMON_IO_UTILS_H

#include <sys/types.h>		/* POSIX */
#include <sys/stat.h>		/* Gnulib/POSIX */
#include <stdio.h>		/* Gnulib/C89 */
#include "common.h"		/* common */
#include "llist.h"		/* common */

bool file_exists(const char *file);
#define is_directory(x)		S_ISDIR(stat_mode(x))
mode_t stat_mode(const char *file);
off_t file_size(const char *file);
char *create_temporary_file(char *base);
#if 0
char *read_line(FILE *in);
char *backticks(const char *program, char *const args[], int *rc);
#endif
LList *read_directory(const char *dir);
/* ssize_t xread(int fd, void *buf, size_t count); */
/* ssize_t xwrite(int fd, const void *buf, size_t count); */
int fskip(FILE *file, uint32_t bytes);
int fpad(FILE *file, char byte, uint32_t bytes);

#endif
