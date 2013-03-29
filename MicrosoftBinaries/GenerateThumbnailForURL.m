/* GenerateThumbnailForURL.m
 *
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

#include <CoreFoundation/CoreFoundation.h>
#include <CoreServices/CoreServices.h>
#include <QuickLook/QuickLook.h>
#import <Cocoa/Cocoa.h>
#import "EIExeFile.h"


#define MAX_NETWORK_PREVIEW ((1024*1024*5))


BOOL isFileOnNetworkDrive(CFURLRef url) {
  FSRef ref;
  /* Using deprecated APIs to support 10.5 */
  if (CFURLGetFSRef(url, &ref)) {
    FSCatalogInfo ci;
    if (FSGetCatalogInfo(&ref, kFSCatInfoVolume, &ci, NULL, NULL, NULL) == noErr) {
      GetVolParmsInfoBuffer vpi;
      if (FSGetVolumeParms(ci.volume, &vpi, sizeof(vpi)) == noErr) {
        return (vpi.vMServerAdr != 0);
      } else return NO;
    } else return NO;
  } else return NO;
}


/* -----------------------------------------------------------------------------
 Generate a thumbnail for file
 
 This function's job is to create thumbnail for designated file as fast as possible
 ----------------------------------------------------------------------------- */

OSStatus GenerateThumbnailForURL(void *thisInterface, QLThumbnailRequestRef thumbnail,
           CFURLRef url, CFStringRef contentTypeUTI, CFDictionaryRef options, CGSize maxSize)
{
  //No icon for DLLs
  if (!UTTypeEqual(contentTypeUTI, (CFStringRef)@"com.microsoft.windows-executable")) return noErr;
  
  NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
  
  MDItemRef mdirf = MDItemCreateWithURL(kCFAllocatorDefault, (CFURLRef)url);
  if (mdirf) {
    NSNumber *fsize = MDItemCopyAttribute(mdirf, kMDItemFSSize);
    if ([fsize compare:[NSNumber numberWithLong:MAX_NETWORK_PREVIEW]] == NSOrderedDescending) {
      NSLog(@"Canceled thumbnail of %@ because file is big and not local.", url);
      [pool release];
    }
  }
  
  BOOL err;
  EIExeFile *exeFile;
  exeFile = [[EIExeFile alloc] initWithExeFileURL:(NSURL*)url error:&err];
  if (err) goto cleanup; //here goto is useful!
  if (QLThumbnailRequestIsCancelled(thumbnail)) goto cleanup;
  
  NSImage *icon = [exeFile getIconNSImage];
  if (!icon) goto cleanup;
  if (![icon isValid]) goto cleanup;
  if (QLThumbnailRequestIsCancelled(thumbnail)) goto cleanup;
   
  NSRect rect;
  rect.origin.x = 0;
  rect.origin.y = 0;
  rect.size = NSSizeFromCGSize(maxSize);
  CGImageRef qlres = [icon CGImageForProposedRect:&rect context:nil hints:nil];
  QLThumbnailRequestSetImage(thumbnail, qlres, NULL);
  
  if (mdirf) {
    CFBooleanRef custico = MDItemCopyAttribute(mdirf, kMDItemFSHasCustomIcon);
    if (custico == kCFBooleanFalse) {
      [[NSWorkspace sharedWorkspace] setIcon:icon forFile:[(NSURL*)url path] options:0];
    }
  }
  
  cleanup:
  [exeFile release];
  [pool release];
  return noErr;
}

void CancelThumbnailGeneration(void* thisInterface, QLThumbnailRequestRef thumbnail)
{
    // implement only if supported
}
