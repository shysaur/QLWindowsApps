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


typedef void (*DoResourceCallback) (WinLibrary *, WinResource *, WinResource *, WinResource *, WinResource *);
wres_error do_resources (WinLibrary *, const char *, const char *, const char *, DoResourceCallback);
void print_resources_callback (WinLibrary *, WinResource *, WinResource *, WinResource *, WinResource *);

WinResource *list_resources(WinLibrary *, WinResource *, int *, wres_error *);
WinResource *find_resource(WinLibrary *, const char *, const char *, const char *, int *, wres_error *);
void *get_resource_entry(WinLibrary *, WinResource *, size_t *, wres_error *);


#endif
