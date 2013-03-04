#include <CoreFoundation/CoreFoundation.h>
#include <CoreServices/CoreServices.h>
#include <QuickLook/QuickLook.h>
#import <Cocoa/Cocoa.h>
#import "EIExeFile.h"

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
  
  BOOL err;
  EIExeFile *exeFile;
  exeFile = [[EIExeFile alloc] initWithExeFileURL:(NSURL*)url error:&err];
  if (err) goto cleanup; //here goto is useful!
  if (QLThumbnailRequestIsCancelled(thumbnail)) goto cleanup;
  
  NSImage *icon = [exeFile getIconNSImage];
  if (!icon) goto cleanup;
  if (![icon isValid]) goto cleanup;
  if (QLThumbnailRequestIsCancelled(thumbnail)) goto cleanup;
    
  CFDictionaryRef properties = NULL;
  /*CFTypeRef keys[1] = {kQLThumbnailOptionIconModeKey};
  CFTypeRef values[1] = {kCFBooleanFalse};
  properties = CFDictionaryCreate(kCFAllocatorDefault, (const void**)keys, (const void**)values, 1, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
  */
   
  NSRect rect;
  rect.origin.x = 0;
  rect.origin.y = 0;
  rect.size = NSSizeFromCGSize(maxSize);
  CGImageRef qlres = [icon CGImageForProposedRect:&rect context:nil hints:nil];
  QLThumbnailRequestSetImage(thumbnail, qlres, properties);
  
  MDItemRef mdirf = MDItemCreateWithURL(kCFAllocatorDefault, (CFURLRef)url);
  if (mdirf) {
    CFBooleanRef custico = MDItemCopyAttribute(mdirf, kMDItemFSHasCustomIcon);
    if (custico == kCFBooleanFalse) {
      /*BOOL ierr =*/ [[NSWorkspace sharedWorkspace] setIcon:icon forFile:[(NSURL*)url path] options:0];
      //NSLog(@"Set custom icon for executable file %@ %@.", url, ierr?@"OK":@"error");
    }
    //CFRelease(custico); Depsite what clang says we Cannot Release A CFBooleanRef because it's not really a ref!
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
