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
#import "NSData+QWABase64.h"


#define NSAppKitVersionNumber10_6 1038
#define NSAppKitVersionNumber10_9 1265


NSString *QWAGetCascadingStyleSheet(void) {
  NSBundle *mbundle;
  NSString *csspath;
  
  mbundle = [NSBundle bundleWithIdentifier:@"com.danielecattaneo.qlgenerator.qlwindowsapps"];

  if (floor(NSAppKitVersionNumber) <= NSAppKitVersionNumber10_6) {
    csspath = [mbundle pathForResource:@"PreviewStyleSL" ofType:@"css"];
  } else if (floor(NSAppKitVersionNumber) <= NSAppKitVersionNumber10_9) {
    csspath = [mbundle pathForResource:@"PreviewStyleML" ofType:@"css"];
  } else {
    csspath = [mbundle pathForResource:@"PreviewStyleYE" ofType:@"css"];
  }
  
  return [NSString stringWithContentsOfFile:csspath encoding:NSUTF8StringEncoding error:nil];
}


NSString *QWAHTMLVersionInfoForExeFile(EIExeFile *exeFile) {
  NSMutableString *html;
  NSData *vinfo;
  EIVersionInfoReader *vir;
  NSString* queryHeader;
  NSArray *resSrch;
  NSBundle *mbundle;
  NSStringEncoding resEnc;
  NSData *item;
  NSString *temp, *node, *vpath;
  
  mbundle = [NSBundle bundleWithIdentifier:@"com.danielecattaneo.qlgenerator.qlwindowsapps"];
  html = [[@"<table id=\"vertable\"><tbody>" mutableCopy] autorelease];
  
  vinfo = [exeFile versionInfo];
  vir = [[EIVersionInfoReader alloc] initWithBlock:vinfo is16Bit:[exeFile is16Bit]];
  [vir autorelease];
  
  queryHeader = @"\\StringFileInfo\\*";
  resSrch = [vir querySubNodesUnder:queryHeader error:NULL];
  if (!resSrch) return @"";
  
  if ([exeFile is16Bit])
    resEnc = NSWindowsCP1252StringEncoding;
  else
    resEnc = NSUTF16LittleEndianStringEncoding;
  
  for (node in resSrch) {
    vpath = [NSString stringWithFormat:@"%@\\%@", queryHeader, node];
    item = [vir queryValue:vpath error:NULL];
    if (!item) continue;
    
    temp = [[NSString alloc] initWithData:item encoding:resEnc];
    [html appendString:@"<tr><td>"];
    [html appendString:NSLocalizedStringFromTableInBundle(node, @"VersioninfoNames", mbundle, nil)];
    [html appendString:@"</td><td>"];
    [html appendString:temp];
    [html appendString:@"</td></tr>"];
    [temp release];
  }
  
  [html appendString:@"</tbody></table>"];
  return html;
}


NSString *QWAGetBase64EncodedImageForExeFile(EIExeFile *exeFile, CFStringRef contentTypeUTI, CFURLRef url) {
  NSImage *icon = nil;
  NSData *image;
  
  if (UTTypeEqual(contentTypeUTI, (CFStringRef)@"com.microsoft.windows-executable"))
    icon = [exeFile icon];
  if (!icon || ![icon isValid])
    icon = [[NSWorkspace sharedWorkspace] iconForFile:[(NSURL*)url path]];
  image = [icon TIFFRepresentation];
  return [image qwa_base64Encoding];
}


/* -----------------------------------------------------------------------------
   Generate a preview for file

   This function's job is to create preview for designated file
   ----------------------------------------------------------------------------- */

OSStatus GeneratePreviewForURL(void *thisInterface, QLPreviewRequestRef preview, 
  CFURLRef url, CFStringRef contentTypeUTI, CFDictionaryRef options) {
  NSAutoreleasePool *pool;
  EIExeFile *exeFile;
  NSMutableString *html;
  NSDictionary *props;
  
  pool = [[NSAutoreleasePool alloc] init];
  html = [NSMutableString string];
  
  exeFile = [[EIExeFile alloc] initWithExeFileURL:(NSURL*)url];
  if (!exeFile) goto cleanup;
  [exeFile autorelease];
  if (QLPreviewRequestIsCancelled(preview)) goto cleanup;
  
  /* Header: use an inline style sheet */
  [html appendString:@"<html><head><style type=\"text/css\">"];
  [html appendString:QWAGetCascadingStyleSheet()];
  [html appendString:@"</style></head><body>"];
  
  if (QLPreviewRequestIsCancelled(preview)) goto cleanup;
  
  /* Icon div */
  [html appendString:@"<div id=\"icon\"><img id=\"icon_img\" src=\"data:image/tiff;base64,"];
  [html appendString:QWAGetBase64EncodedImageForExeFile(exeFile, contentTypeUTI, url)];
  [html appendString:@"\"></div>"];
  
  if (QLPreviewRequestIsCancelled(preview)) goto cleanup;
  
  /* File name and 16-bit badge */
  [html appendString:@"<div id=\"exename\">"];
  if ([exeFile is16Bit])
    [html appendString:@"<div class=\"badge\">16 bit</div>"];
  [html appendFormat:@"<h1>%@</h1></div>", [[(NSURL*)url path] lastPathComponent]];
  
  /* Version info */
  [html appendString:@"<div id=\"info\">"];
  [html appendString:QWAHTMLVersionInfoForExeFile(exeFile)];
  [html appendString:@"</div>"];
  
  /* Trailer */
  [html appendString:@"</body></html>"];
  
  props = @{(NSString *)kQLPreviewPropertyTextEncodingNameKey : @"UTF-8",
            (NSString *)kQLPreviewPropertyMIMETypeKey : @"text/html"};
  
  QLPreviewRequestSetDataRepresentation(preview,
    (CFDataRef)[html dataUsingEncoding:NSUTF8StringEncoding], kUTTypeHTML,
    (CFDictionaryRef)props);
    
cleanup:
  [pool release];
  return noErr;
}


void CancelPreviewGeneration(void* thisInterface, QLPreviewRequestRef preview) {
  // implement only if supported
}

