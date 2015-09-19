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


BOOL QWAIsFileOnNetworkDrive(CFURLRef url) {
  NSURL *volume;
  NSNumber *local;
  
  if (![(NSURL*)url getResourceValue:&volume forKey:NSURLVolumeURLKey error:nil])
    return NO;
  
  if (![volume getResourceValue:&local forKey:NSURLVolumeIsLocalKey error:nil])
    return NO;
  
  return ![local boolValue];
}


/* -----------------------------------------------------------------------------
 Generate a thumbnail for file
 
 This function's job is to create thumbnail for designated file as fast as possible
 ----------------------------------------------------------------------------- */

OSStatus GenerateThumbnailForURL(void *thisInterface, QLThumbnailRequestRef thumbnail,
  CFURLRef url, CFStringRef contentTypeUTI, CFDictionaryRef options, CGSize maxSize) {
  NSAutoreleasePool *pool;
  MDItemRef mdirf;
  NSNumber *fsize;
  EIExeFile *exeFile;
  NSImage *icon;
  NSRect rect;
  CGImageRef qlres;
  NSNumber *finfo;
  
  //No icon for DLLs
  if (!UTTypeEqual(contentTypeUTI, (CFStringRef)@"com.microsoft.windows-executable"))
    return noErr;
  
  pool = [[NSAutoreleasePool alloc] init];
  
  mdirf = MDItemCreateWithURL(kCFAllocatorDefault, (CFURLRef)url);
  if (mdirf && QWAIsFileOnNetworkDrive(url)) {
    fsize = MDItemCopyAttribute(mdirf, kMDItemFSSize);
    
    if ([fsize compare:@(MAX_NETWORK_PREVIEW)] == NSOrderedDescending) {
      NSLog(@"Canceled thumbnail of %@ because file is big and not local.", url);
      [fsize release];
      CFRelease(mdirf);
      [pool release];
      return noErr;
    }
    [fsize release];
  }
  
  exeFile = [[EIExeFile alloc] initWithExeFileURL:(NSURL*)url];
  if (!exeFile) goto cleanup;
  if (QLThumbnailRequestIsCancelled(thumbnail)) goto cleanup;
  
  icon = [exeFile icon];
  if (!icon) goto cleanup;
  if (![icon isValid]) goto cleanup;
  if (QLThumbnailRequestIsCancelled(thumbnail)) goto cleanup;
  
  rect.origin = NSZeroPoint;
  rect.size = NSSizeFromCGSize(maxSize);
  qlres = [icon CGImageForProposedRect:&rect context:nil hints:nil];
  QLThumbnailRequestSetImage(thumbnail, qlres, NULL);
  
  if (mdirf) {
    finfo = MDItemCopyAttribute(mdirf, (CFStringRef)@"kMDItemFSFinderFlags");
    if (!([finfo integerValue] & kHasCustomIcon))
      [[NSWorkspace sharedWorkspace] setIcon:icon forFile:[(NSURL*)url path] options:0];
  }
  
cleanup:
  [exeFile release];
  if (mdirf)
    CFRelease(mdirf);
  [pool release];
  return noErr;
}


void CancelThumbnailGeneration(void* thisInterface, QLThumbnailRequestRef thumbnail) {
    // implement only if supported
}
