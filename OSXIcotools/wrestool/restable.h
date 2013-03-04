/* restable.h - Decoding PE and NE resource tables
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

#ifndef RESTABLE_H
#define RESTABLE_H

#include "common/intutil.h"
#include "wrestool.h"

bool compare_resource_id (WinResource *, char *);

/* what is each entry in this directory level for? type, name or language? */
#define WINRESOURCE_BY_LEVEL(x) ((x)==0 ? type_wr : ((x)==1 ? name_wr : lang_wr))

/* does the id of this entry match the specified id? */
#define LEVEL_MATCHES(x) (x == NULL || x ## _wr->id[0] == '\0' || compare_resource_id(x ## _wr, x))

#endif
