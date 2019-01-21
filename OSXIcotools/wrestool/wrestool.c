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

#include <stdio.h>
#include "fileread.h"
#include "wrestool.h"


WinLibrary *new_winlibrary_from_file(const char *fn, wres_error *err)
{
	WinLibrary *fl = calloc(sizeof(WinLibrary), 1);
	
	fl->name = strdup(fn);
	if (!fl->name) {
		if (err) *err = WRES_ERROR_OUTOFMEMORY;
		free(fl);
		return NULL;
	}
	
	/* open file */
	fl->file = fopen(fl->name, "rb");
	if (fl->file == NULL) {
		if (err) *err = -errno;
		free(fl->name);
		free(fl);
		return NULL;
	}
	
	/* identify file and find resource table */
	wres_error e;
	if ((e = load_library(fl))) {
		/* error reported by load_library */
		free_winlibrary(fl);
		if (err) *err = e;
		return NULL;
	}
	
	return fl;
}


void free_winlibrary(WinLibrary *fl)
{
	unload_library(fl);
	if (fl->file)
    	fclose(fl->file);
  	free(fl->name);
}


const char *wres_strerr(wres_error err)
{
	if (err < WRES_ERROR_FIRST || WRES_ERROR_LAST < err)
		return "unknown error code";
	
	if (err >= WRES_ERROR_ERRNO_FIRST && WRES_ERROR_ERRNO_LAST >= err)
		return strerror(-err);
	const char *errors[] = {
		"no error", /* WRES_ERROR_NONE */
		"unknown error", /* WRES_ERROR_UNKNOWN */
		"out of memory", /* WRES_ERROR_OUTOFMEMORY */
		"suitable resource not found", /* WRES_ERROR_RESNOTFOUND */
		"not a PE or NE executable", /* WRES_ERROR_WRONGFORMAT */
		"no resource directory found", /* WRES_ERROR_NORESDIR */
		"corrupt file, premature end", /* WRES_ERROR_PREMATUREEND */
		"no resources found", /* WRES_ERROR_NORESOURCES */
		"invalid resource table", /* WRES_ERROR_INVALIDRESTABLE */
		"invalid function argument", /* WRES_ERROR_INVALIDPARAM */
		"unsupported resource type", /* WRES_ERROR_UNSUPPRESTYPE */
		"invalid section layout", /* WRES_ERROR_INVALIDSECLAYOUT */
	};
	return errors[err];
}

