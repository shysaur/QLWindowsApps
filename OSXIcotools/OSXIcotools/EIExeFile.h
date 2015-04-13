/* EIExeFile.h - Class tasked of representing a windows exe or dll and
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

#import <Cocoa/Cocoa.h>
#include "restypes.h"
#include "osxwres.h"


@class EIVersionInfo;


@interface EIExeFile : NSObject {
  WinLibrary fl;
  NSURL *url;
}

- (instancetype)initWithExeFileURL:(NSURL *)exeFile;
- (void)dealloc;

- (NSImage *)icon;
- (EIVersionInfo *)versionInfo;
- (NSURL *)url;
- (BOOL)is16Bit;

@end
