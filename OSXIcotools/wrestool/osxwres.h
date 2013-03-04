/* osxwres.h - wrestool bridge for Mac OS X / NeXTSTEP(untested) / GnuStep(untested)
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

typedef enum {
  EXTR_NOERR = 0,     //No error occured
  EXTR_FAIL = 1,      //Error in extracting resource
  EXTR_NOTFOUND = 2   //Resource not found
} extract_error;

NSData *nsdata_default_resource (WinLibrary *, char *, char *, char *, extract_error *);

#endif
