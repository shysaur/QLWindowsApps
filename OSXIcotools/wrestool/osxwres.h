/* osxwres.h - wrestool bridge for Mac OS X
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

#ifndef OSXWRES_H
#define OSXWRES_H

#include "wrestool.h"
#include "restable.h"
#include "fileread.h"


extern NSString const *EIIcotoolsErrorDomain;


NSData *get_resource_data (WinLibrary *, char *, char *, char *, wres_error *);
NSError *nserror_from_wreserror(wres_error err);

#endif
