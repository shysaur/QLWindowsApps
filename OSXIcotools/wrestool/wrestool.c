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
#include "io-utils.h"


WinLibrary *new_winlibrary_from_file(const char *fn)
{
	WinLibrary *fl = calloc(sizeof(WinLibrary), 1);
	
	fl->name = strdup(fn);
	if (!fl->name) {
		fprintf(stderr, "malloc failed");
		return NULL;
	}
	
	/* get file size */
	fl->total_size = (int)file_size(fl->name);
	if (fl->total_size == -1) {
		fprintf(stderr, "%s total size = -1", fl->name);
		return NULL;
	}
	if (fl->total_size == 0) {
		fprintf(stderr, "%s: file has a size of 0", fl->name);
		return NULL;
	}
	
	/* open file */
	fl->file = fopen(fl->name, "rb");
	if (fl->file == NULL) {
		fprintf(stderr, "%s error opening file", fl->name);
		return NULL;
	}
	
	/* read all of file */
	fl->memory = malloc(fl->total_size);
	if (fread(fl->memory, fl->total_size, 1, fl->file) != 1) {
		fprintf(stderr, "%s error reading file contents", fl->name);
		return NULL;
	}
	
	/* identify file and find resource table */
	if (!read_library(fl)) {
		/* error reported by read_library */
		return NULL;
	}
	
	return fl;
}


void free_winlibrary(WinLibrary *fl)
{
	if (fl->file)
    	fclose(fl->file);
  	free(fl->memory);
  	free(fl->name);
}

