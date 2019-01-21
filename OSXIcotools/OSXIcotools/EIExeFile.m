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


- (instancetype)initWithExeFileURL:(NSURL *)exeFile  error:(NSError **)oerr
{
  self = [super init];
  if (!self) return nil;
  
  wres_error err;
  fl = new_winlibrary_from_file([exeFile fileSystemRepresentation], &err);
  if (!fl) {
    if (oerr)
      *oerr = nserror_from_wreserror(err);
    return nil;
  }
  
  return self;
}


- (void)logError:(wres_error)err
{
  if (!fl)
    NSLog(@"%s", wres_strerr(err));
  else if (err == WRES_ERROR_RESNOTFOUND)
    EILog(@"%s: %s", fl->name, wres_strerr(err));
  else
    NSLog(@"%s: %s", fl->name, wres_strerr(err));
}


- (NSData *)iconData
{
  wres_error err;
  NSData *icodata = get_resource_data(fl, "14", NULL, NULL, &err);
  
  if (!icodata) {
    [self logError:err];
    return nil;
  }
  return icodata;
}


- (NSImage*)icon
{
  NSData *icodata = [self iconData];
  if (!icodata)
    return nil;
  
  return [[NSImage alloc] initWithData:icodata];
}


- (EIVersionInfo *)versionInfo {
  wres_error err;
  uint32_t sysLocale;
  NSString *localeIdent;
  char sysLocaleStr[64];
  
  localeIdent = [[NSLocale currentLocale] localeIdentifier];
  sysLocale = [NSLocale windowsLocaleCodeFromLocaleIdentifier:localeIdent];
  
  sprintf(sysLocaleStr, "%d", sysLocale);
  //try with the current selected locale in the OS
  NSData *verdata = get_resource_data(fl, "16", NULL, sysLocaleStr, NULL);
  if (!verdata) {
    //if failure, try the en-US locale (the majority of apps, if they're not neutral, use this)
    verdata = get_resource_data(fl, "16", NULL, "1033", NULL);
    if (!verdata) {
      //else, pick the first locale we find, and go with it.
      verdata = get_resource_data(fl, "16", NULL, NULL, &err);
      if (!verdata) {
        [self logError:err];
        return nil;
      }
    }
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
