/* GeneratePreviewForURL.m
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
#import "EIVersionInfoReader.h"

/* -----------------------------------------------------------------------------
   Generate a preview for file

   This function's job is to create preview for designated file
   ----------------------------------------------------------------------------- */

#define NSAppKitVersionNumber10_6 1038
#define NSAppKitVersionNumber10_9 1265

OSStatus GeneratePreviewForURL(void *thisInterface, QLPreviewRequestRef preview, 
           CFURLRef url, CFStringRef contentTypeUTI, CFDictionaryRef options)
{
  NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
  
  NSBundle *mbundle = [NSBundle bundleWithIdentifier:@"com.danielecattaneo.qlgenerator.qlwindowsapps"];
  
  BOOL err;
  EIExeFile *exeFile;
  exeFile = [[[EIExeFile alloc] initWithExeFileURL:(NSURL*)url error:&err] autorelease];
  if (err) goto cleanup; //here goto is useful!
  if (QLPreviewRequestIsCancelled(preview)) goto cleanup;
  
  NSImage *icon = nil;
  if (UTTypeEqual(contentTypeUTI, (CFStringRef)@"com.microsoft.windows-executable")) {
    icon = [exeFile getIconNSImage];
  }
  if (!icon) {
    icon = [[NSWorkspace sharedWorkspace] iconForFile:[(NSURL*)url path]];
  }
  NSData *image = [icon TIFFRepresentation];
  if (!image) {
    image = [[[NSWorkspace sharedWorkspace] iconForFile:[(NSURL*)url path]] TIFFRepresentation];
  }  

  // Before proceeding make sure the user didn't cancel the request
  if (QLPreviewRequestIsCancelled(preview)) goto cleanup;
  if (err) goto cleanup;
  
  NSMutableDictionary *props = [[[NSMutableDictionary alloc] init] autorelease];
  [props setObject:@"UTF-8" forKey:(NSString *)kQLPreviewPropertyTextEncodingNameKey];
  [props setObject:@"text/html" forKey:(NSString *)kQLPreviewPropertyMIMETypeKey];

  NSString *csspath;
  if (floor(NSAppKitVersionNumber) <= NSAppKitVersionNumber10_6) {
    csspath = [mbundle pathForResource:@"PreviewStyleSL" ofType:@"css"];
  } else if (floor(NSAppKitVersionNumber) <= NSAppKitVersionNumber10_9) {
    csspath = [mbundle pathForResource:@"PreviewStyleML" ofType:@"css"];
  } else {
    csspath = [mbundle pathForResource:@"PreviewStyleYE" ofType:@"css"];
  }
  NSMutableString *html = [[[NSMutableString alloc] init] autorelease];
  //[html appendString:@"<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\" \"http://www.w3.org/TR/html4/loose.dtd\">"];
  [html appendFormat:@"<html><head><style type=\"text/css\">%@</style></head><body>", [NSString stringWithContentsOfFile:csspath encoding:NSUTF8StringEncoding error:nil]];
  
  NSString *b64img;
  b64img = [image base64Encoding];
  [html appendFormat:@"<div id=\"icon\"><img id=\"icon_img\" src=\"data:image/tiff;base64,%@\"></div>", b64img];
  
  
  
  [html appendString:@"<div id=\"exename\">"];
  if ([exeFile is16Bit]) {
    [html appendString:@"<div class=\"badge\">16 bit</div>"];
  }
  [html appendFormat:@"<h1>%@</h1></div>", [(NSURL*)url lastPathComponent]];
  
  
  
  [html appendString:@"<div id=\"info\"><table id=\"vertable\"><tbody>"];
  
  EIVERSION_ERR virErr;
  NSData *vinfo = [exeFile getVersionInfo];
  EIVersionInfoReader *vir = [[[EIVersionInfoReader alloc] initWithBlock:vinfo is16Bit:[exeFile is16Bit]] autorelease];
  NSString* queryHeader = @"\\StringFileInfo\\*";
  NSArray *resSrch = [vir querySubNodesUnder:queryHeader error:&virErr];
  
  if (!virErr) {;
    NSStringEncoding resEnc;
    if ([exeFile is16Bit]) {
      resEnc = NSWindowsCP1252StringEncoding;
    } else {
      resEnc = NSUTF16LittleEndianStringEncoding;
    }
    
    for (int c=0; c<[resSrch count]; c++) {
      NSData* item = [vir queryValue:[NSString stringWithFormat:@"%@\\%@", queryHeader, [resSrch objectAtIndex:c]] error:&virErr];
      if (!virErr) {
        NSString* temp = [[NSString alloc] initWithData:item encoding:resEnc];
        [html appendFormat:@"<tr><td>%@</td><td>%@</td></tr>", NSLocalizedStringFromTableInBundle([resSrch objectAtIndex:c], @"VersioninfoNames", mbundle, nil), temp];
        [temp release];
      }
    }
    err = NO;
  }
  
  [html appendString:@"</tbody></table></div>"];

  
  
  
  [html appendString:@"</body></html>"];
  
  
  QLPreviewRequestSetDataRepresentation(
          preview,
          (CFDataRef)[html dataUsingEncoding:NSUTF8StringEncoding],
          kUTTypeHTML,
          (CFDictionaryRef)props
  );
    
  cleanup:
  [pool release];
  return noErr;
}

void CancelPreviewGeneration(void* thisInterface, QLPreviewRequestRef preview)
{
    // implement only if supported
}
