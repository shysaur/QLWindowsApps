/* wrestool.h - Common definitions for wrestool
 *
 * Copyright (C) 1998 Oskar Liljeblad
 * Copyright (C) 2012 Daniele Cattaneo
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

#ifndef WRESTOOL_H
#define WRESTOOL_H

#include <stdbool.h>
#include <unistd.h>		/* POSIX */
#include <stdarg.h>		/* C89 */
#include <stdio.h>		/* C89 */
#include <stdlib.h>		/* C89 */
#include <stdint.h>		/* POSIX/Gnulib */
#include <string.h>		/* C89 */
#include <errno.h>		/* C89 */
#include <getopt.h>		/* GNU Libc/Gnulib */


#define NE_BINARY		0
#define PE_BINARY		1
#define PEPLUS_BINARY	2

typedef struct _WinLibrary {
	char *name;
	FILE *file;
	char *memory;
	uint8_t *first_resource;
	int binary_type;
	off_t total_size;
} WinLibrary;

typedef struct _WinResource {
	char id[256];
	void *this;
	void *children;
	int level;
	bool numeric_id;
	bool is_directory;
} WinResource;

#define WINRES_ID_MAXLEN (256)


typedef int wres_error;
enum {
	WRES_ERROR_ERRNO_FIRST = -ELAST,
	WRES_ERROR_ERRNO_LAST = -1,
	WRES_ERROR_NONE = 0,
	WRES_ERROR_UNKNOWN,
	WRES_ERROR_OUTOFMEMORY,
	WRES_ERROR_RESNOTFOUND,
	WRES_ERROR_WRONGFORMAT,
	WRES_ERROR_NORESDIR,
	WRES_ERROR_PREMATUREEND,
	WRES_ERROR_NORESOURCES,
	WRES_ERROR_INVALIDRESTABLE,
	WRES_ERROR_INVALIDPARAM,
	WRES_ERROR_UNSUPPRESTYPE,
	WRES_ERROR_INVALIDSECLAYOUT,
	WRES_ERROR_INVALIDDIB,
	
	WRES_ERROR_END,
	WRES_ERROR_FIRST = WRES_ERROR_ERRNO_FIRST,
	WRES_ERROR_LAST = WRES_ERROR_END-1,
};


WinLibrary *new_winlibrary_from_file(const char *fn, wres_error *);
void free_winlibrary(WinLibrary *fl);
const char *wres_strerr(wres_error);


#endif
