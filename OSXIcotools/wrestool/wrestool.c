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


WinLibrary *new_winlibrary_from_file(const char *fn)
{
	WinLibrary *fl = calloc(sizeof(WinLibrary), 1);
	
	fl->name = strdup(fn);
	if (!fl->name) {
		fprintf(stderr, "malloc failed");
		free(fl);
		return NULL;
	}
	
	/* open file */
	fl->file = fopen(fl->name, "rb");
	if (fl->file == NULL) {
		fprintf(stderr, "%s error opening file", fl->name);
		free(fl->name);
		free(fl);
		return NULL;
	}
	
	/* identify file and find resource table */
	if (!load_library(fl)) {
		/* error reported by load_library */
		free_winlibrary(fl);
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

