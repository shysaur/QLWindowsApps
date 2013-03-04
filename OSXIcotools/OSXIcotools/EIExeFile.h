//
//  EIExeFile.h
//  EXEIconReader
//
//  Created by Daniele Cattaneo on 18/11/12.
//  Copyright 2012 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#include "restypes.h"
#include "osxwres.h"

@interface EIExeFile : NSObject {
  WinLibrary fl;
  char exename[1024];
}

- initWithExeFileURL:(NSURL*)exeFile error:(BOOL*)err;
- (void)dealloc;

- (NSImage*)getIconNSImage;
- (NSData*)getVersionInfo;
- (BOOL)is16Bit;

@end
