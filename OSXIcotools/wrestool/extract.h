/* extract.h
 *
 * Copyright (C) 1998 Oskar Liljeblad
 * Copyright (C) 2017 Daniele Cattaneo
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

#ifndef EXTRACT_H
#define EXTRACT_H

#include "wrestool.h"


void *extract_resource(WinLibrary *, WinResource *, int *, bool *, char *, char *, bool);
void set_raw_extraction(bool);
void set_output_dir(char *);


#endif /* extract_h */
