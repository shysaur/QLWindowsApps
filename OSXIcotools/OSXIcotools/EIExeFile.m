/* EIExeFile.m - Class tasked of representing a windows exe or dll and
 * its resource contents.
 *
 * Copyright (C) 2012-13 Daniele Cattaneo
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

#import "EIExeFile.h"
#import "EIVersionInfo.h"
#include <stdlib.h>
#include "wrestool.h"



#ifdef DEBUG
#  define EILog(...) NSLog(__VA_ARGS__)
#else
#  define EILog(...)
#endif


@implementation EIExeFile


- (instancetype)initWithExeFileURL:(NSURL *)exeFile {
  self = [super init];
  if (!self) return nil;
  
  fl = new_winlibrary_from_file([exeFile fileSystemRepresentation]);
  if (!fl)
    return nil;
  
  return self;
}


- (NSImage*)icon {
  extract_error err;
  NSData *icodata = get_resource_data(fl, "14", NULL, NULL, &err);
    
  if (err) {
    if (err == EXTR_NOTFOUND)
      EILog(@"%s: suitable resource not found", fl->name);
    else
      NSLog(@"%s: error in extracting resource", fl->name);
    return nil;
  }

  return [[NSImage alloc] initWithData:icodata];
}


- (EIVersionInfo *)versionInfo {
  extract_error err;
  uint32_t sysLocale;
  NSString *localeIdent;
  char sysLocaleStr[64];
  
  localeIdent = [[NSLocale currentLocale] localeIdentifier];
  sysLocale = [NSLocale windowsLocaleCodeFromLocaleIdentifier:localeIdent];
  
  sprintf(sysLocaleStr, "%d", sysLocale);
  //try with the current selected locale in the OS
  NSData *verdata = get_resource_data(fl, "16", NULL, sysLocaleStr, &err);
  if (err) {
    //if failure, try the en-US locale (the majority of apps, if they're not neutral, use this)
    verdata = get_resource_data(fl, "16", NULL, "1033", &err);
    if (err) {
      //else, pick the first locale we find, and go with it.
      verdata = get_resource_data(fl, "16", NULL, NULL, &err);
    }
  }
  
  if (err) {
    if (err == EXTR_NOTFOUND)
      EILog(@"%s: suitable resource not found", fl->name);
    else
      NSLog(@"%s: error in extracting resource", fl->name);
    return nil;
  }
  
  return [[EIVersionInfo alloc] initWithData:verdata is16Bit:(fl->binary_type == NE_BINARY)];
}


- (NSURL *)url {
  return url;
}


- (int)bitness {
  static const int bitnesses[] = {16, 32, 64};
  return bitnesses[fl->binary_type];
}


- (void)dealloc {
  if (fl)
    free_winlibrary(fl);
}


@end
