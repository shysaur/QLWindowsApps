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
#import "Utils.h"


#define MAX_NETWORK_PREVIEW ((1024*1024*5))


BOOL QWAIsFileOnNetworkDrive(NSURL *url) {
  NSURL *volume;
  NSNumber *local;
  
  if (![url getResourceValue:&volume forKey:NSURLVolumeURLKey error:nil])
    return NO;
  
  if (![volume getResourceValue:&local forKey:NSURLVolumeIsLocalKey error:nil])
    return NO;
  
  return ![local boolValue];
}


void QWAGenerateThumbnailForURL(QLThumbnailRequestRef thumbnail,
  NSURL *url, CFStringRef contentTypeUTI, CGSize maxSize) {
  NSNumber *fsize;
  EIExeFile *exeFile;
  NSImage *icon;
  NSRect rect;
  CGImageRef qlres;
  
  //No icon for DLLs
  if (!UTTypeEqual(contentTypeUTI, (CFStringRef)@"com.microsoft.windows-executable"))
    return;
  
  if (QWAIsFileOnNetworkDrive(url)) {
    if ([url getResourceValue:&fsize forKey:NSURLFileSizeKey error:nil]) {
      if ([fsize compare:@(MAX_NETWORK_PREVIEW)] == NSOrderedDescending) {
        NSLog(@"Canceled thumbnail of %@ because file is big and not local.", url);
        return;
      }
    }
  }
  
  exeFile = [[EIExeFile alloc] initWithExeFileURL:url error:nil];
  if (!exeFile) return;
  if (QLThumbnailRequestIsCancelled(thumbnail)) return;
  
  icon = [exeFile icon];
  if (!icon) return;
  if (![icon isValid]) return;
  if (QLThumbnailRequestIsCancelled(thumbnail)) return;
  
  rect.origin = NSZeroPoint;
  rect.size = NSSizeFromCGSize(maxSize);
  qlres = [icon CGImageForProposedRect:&rect context:nil hints:nil];
  
  CFDictionaryRef properties = (__bridge CFDictionaryRef)@{
    (__bridge NSString *)kQLThumbnailPropertyIconFlavorKey: @(kQLThumbnailIconPlainFlavor)};
  QLThumbnailRequestSetImage(thumbnail, qlres, properties);
}


/* -----------------------------------------------------------------------------
 Generate a thumbnail for file
 
 This function's job is to create thumbnail for designated file as fast as possible
 ----------------------------------------------------------------------------- */

OSStatus GenerateThumbnailForURL(void *thisInterface, QLThumbnailRequestRef thumbnail,
  CFURLRef url, CFStringRef contentTypeUTI, CFDictionaryRef options, CGSize maxSize) {
  
  @autoreleasepool {
    QWAGenerateThumbnailForURL(thumbnail, (__bridge NSURL*)url, contentTypeUTI, maxSize);
  }
  return noErr;
}


void CancelThumbnailGeneration(void* thisInterface, QLThumbnailRequestRef thumbnail) {
    // implement only if supported
}
