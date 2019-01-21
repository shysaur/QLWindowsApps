/* fileread.h - Offset checking routines for file reading
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

#ifndef FILEREAD_H
#define FILEREAD_H

#include <stdbool.h>		/* POSIX/Gnulib */
#include "wrestool.h"

#define BAD_POINTER(fi, x) \
	(!check_offset((fi)->memory, (fi)->total_size, (fi)->name, &(x), sizeof(x)))
#define BAD_OFFSET(fi, x, s) \
	(!check_offset((fi)->memory, (fi)->total_size, (fi)->name, (x), (s)))

#define IF_BAD_POINTER(fi, x) \
	if (BAD_POINTER(fi, x))
#define IF_BAD_OFFSET(fi, x, s) \
	if (BAD_OFFSET(fi, x, s))

#define RETURN_IF_BAD_POINTER(fi, r, x) \
	IF_BAD_POINTER(fi, x) { \
		/*printf("bad_pointer in %s:%d\n", __FILE__, __LINE__);*/ \
		return (r); \
	}
#define RETURN_IF_BAD_OFFSET(fi, r, x, s) \
	IF_BAD_OFFSET(fi, x, s) { \
		/*printf("bad_offset in %s:%d\n", __FILE__, __LINE__);*/ \
		return (r); \
	}

#define RET_NULL_AND_SET_ERR_IF_BAD_POINTER(fi, err, x) \
	IF_BAD_POINTER(fi, x) { \
		/*printf("bad_pointer in %s:%d\n", __FILE__, __LINE__);*/ \
		if (err) *(err) = WRES_ERROR_PREMATUREEND; \
		return NULL; \
	}
#define RET_NULL_AND_SET_ERR_IF_BAD_OFFSET(fi, err, x, s) \
	IF_BAD_OFFSET(fi, x, s) { \
		/*printf("bad_offset in %s:%d\n", __FILE__, __LINE__);*/ \
		if (err) *(err) = WRES_ERROR_PREMATUREEND; \
		return NULL; \
	}

wres_error load_library(WinLibrary *);
void unload_library(WinLibrary *);
bool check_offset(const char *, size_t, const char *, const void *, size_t);

#endif
