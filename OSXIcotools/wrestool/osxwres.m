/* osxwres.m - wrestool bridge for Mac OS X 
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

#import <Cocoa/Cocoa.h>
#include "osxwres.h"
#include "restypes.h"
#include "restable.h"
#include "common/common.h"


NSData *
get_resource_data (WinLibrary *fi, char *type, char *name, char *lang, extract_error *err)
{
  int level;
  int size;
  bool free_it;
  void *memory;
  WinResource* wr;
  NSData *icoData;
  
  *err = EXTR_NOERR;
  
  if (type == NULL) type = "";
  if (name == NULL) name = "";
  if (lang == NULL) lang = "";
  
  wr = find_resource(fi, type, name, lang, &level);
  if (wr == NULL) {
    *err = EXTR_NOTFOUND;
    return NULL;
  }
  
  memory = extract_resource(fi, wr, &size, &free_it, type, lang, false);
  if (memory == NULL) {
    *err = EXTR_FAIL;
    return NULL;
  }
  
  if (free_it) {
    icoData = [[NSData alloc] initWithBytesNoCopy:memory length:size freeWhenDone:YES];
  } else {
    icoData = [[NSData alloc] initWithBytes:memory length:size];
  }
  return icoData;
}



