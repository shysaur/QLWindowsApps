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
#include "extract.h"


NSString *EIIcotoolsErrorDomain = @"EIErrorDomain";


NSData *get_resource_data(WinLibrary *fi, char *type, char *name, char *lang, wres_error *err)
{
  int level;
  int size;
  bool free_it;
  void *memory;
  WinResource* wr;
  NSData *icoData;
  
  if (type == NULL) type = "";
  if (name == NULL) name = "";
  if (lang == NULL) lang = "";
  
  wr = find_resource(fi, type, name, lang, &level, err);
  if (!wr)
    return NULL;
  
  memory = extract_resource(fi, wr, &size, &free_it, type, lang, false, err);
  if (!memory)
    return NULL;
  
  if (free_it) {
    icoData = [[NSData alloc] initWithBytesNoCopy:memory length:size freeWhenDone:YES];
  } else {
    icoData = [[NSData alloc] initWithBytes:memory length:size];
  }
  return icoData;
}


NSError *nserror_from_wreserror(wres_error err)
{
  if (err >= WRES_ERROR_ERRNO_FIRST && WRES_ERROR_ERRNO_LAST >= err) {
    return [NSError errorWithDomain:NSPOSIXErrorDomain code:-err userInfo:nil];
  }
  NSString *desc = [NSString stringWithUTF8String:wres_strerr(err)];
  return [NSError errorWithDomain:EIIcotoolsErrorDomain code:err userInfo:@{NSDebugDescriptionErrorKey: desc}];
}


