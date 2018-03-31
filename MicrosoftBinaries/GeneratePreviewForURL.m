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
#import "EIVersionInfo.h"
#import "Utils.h"


NSString *QWAEscapeStringForHtml(NSString *str)
{
  NSMutableString *res = [[NSMutableString alloc] init];
  NSScanner *scan = [NSScanner scannerWithString:str];
  NSCharacterSet *safe = [NSCharacterSet alphanumericCharacterSet];
  [scan setCharactersToBeSkipped:nil];
  
  while (![scan isAtEnd]) {
    NSString *tmp;
    if ([scan scanCharactersFromSet:safe intoString:&tmp])
      [res appendString:tmp];
    if ([scan scanUpToCharactersFromSet:safe intoString:&tmp]) {
      NSInteger n = [tmp length];
      for (NSInteger i = 0; i < n; i++) {
        [res appendFormat:@"&#x%X;", [tmp characterAtIndex:i]];
      }
    }
  }
  return [res copy];
}


NSString *QWAGetTemplate(void) {
  NSBundle *mbundle;
  NSString *csspath;
  
  mbundle = [NSBundle bundleWithIdentifier:@"com.danielecattaneo.qlgenerator.qlwindowsapps"];

  NSString *name = [QWAUserDefaults() objectForKey:@"StyleName"];
  if ([name isKindOfClass:[NSString class]])
    csspath = [mbundle pathForResource:name ofType:@"html"];
  
  if (csspath == nil) {
    if (floor(NSAppKitVersionNumber) < NSAppKitVersionNumber10_10) {
      csspath = [mbundle pathForResource:@"PreviewTemplateLion" ofType:@"html"];
    } else if (floor(NSAppKitVersionNumber) <= NSAppKitVersionNumber10_10_Max) {
      csspath = [mbundle pathForResource:@"PreviewTemplateYosemite" ofType:@"html"];
    } else {
      csspath = [mbundle pathForResource:@"PreviewTemplateElCapitan" ofType:@"html"];
    }
  }
  
  return [NSString stringWithContentsOfFile:csspath encoding:NSUTF8StringEncoding error:nil];
}


NSString *QWAHTMLVersionInfoForExeFile(EIExeFile *exeFile) {
  NSMutableString *html;
  EIVersionInfo *vir;
  NSString* queryHeader;
  NSArray *resSrch;
  NSBundle *mbundle;
  NSString *value, *node, *vpath;
  
  mbundle = [NSBundle bundleWithIdentifier:@"com.danielecattaneo.qlgenerator.qlwindowsapps"];
  html = [@"<tbody>" mutableCopy];
  vir = [exeFile versionInfo];
  
  queryHeader = @"\\StringFileInfo\\*";
  resSrch = [vir querySubNodesUnder:queryHeader error:NULL];
  if (!resSrch) return @"";
  
  for (node in resSrch) {
    vpath = [NSString stringWithFormat:@"%@\\%@", queryHeader, node];
    value = [vir queryStringValue:vpath error:NULL];
    if (!value) continue;
    
    [html appendString:@"<tr><td>"];
    NSString *tmp = NSLocalizedStringFromTableInBundle(node, @"VersioninfoNames", mbundle, nil);
    [html appendString:QWAEscapeStringForHtml(tmp)];
    [html appendString:@"</td><td>"];
    [html appendString:QWAEscapeStringForHtml(value)];
    [html appendString:@"</td></tr>"];
  }
  
  [html appendString:@"</tbody>"];
  return html;
}


NSString *QWAGetBase64EncodedImageForExeFile(EIExeFile *exeFile, CFStringRef contentTypeUTI, NSURL *url) {
  NSImage *icon;
  NSData *image;
  
  if (UTTypeEqual(contentTypeUTI, (CFStringRef)@"com.microsoft.windows-executable")) {
    icon = [exeFile icon];
    QWAChangeIcon(url, icon);
  }
  
  if (!icon || ![icon isValid])
    icon = [[NSWorkspace sharedWorkspace] iconForFile:[url path]];
  image = [icon TIFFRepresentation];
  return [image base64EncodedStringWithOptions:0];
}


void QWAReplaceHtmlPlaceholders(NSMutableString *html, NSDictionary *ph)
{
  NSRegularExpression *phregex, *nameregex;
  NSArray<NSTextCheckingResult *> *phres, *nameres;
  
  phregex = [NSRegularExpression regularExpressionWithPattern:
             @"<!---([^-]*)-->" options:0 error:nil];
  nameregex = [NSRegularExpression regularExpressionWithPattern:
               @"@([A-Z]+)@" options:0 error:nil];
  
  phres = [phregex matchesInString:html options:0 range:NSMakeRange(0, html.length)];
  for (NSTextCheckingResult *phi in [phres reverseObjectEnumerator]) {
    NSRange crange = [phi rangeAtIndex:1];
    NSMutableString *tag = [[html substringWithRange:crange] mutableCopy];
    
    nameres = [nameregex matchesInString:tag options:0 range:NSMakeRange(0, tag.length)];
    for (NSTextCheckingResult *name in [nameres reverseObjectEnumerator]) {
      NSRange nrange = [name rangeAtIndex:1];
      NSString *repl = [ph objectForKey:[tag substringWithRange:nrange]];
      if (repl)
        [tag replaceCharactersInRange:name.range withString:repl];
    }
    
    [html replaceCharactersInRange:phi.range withString:tag];
  }
}


void QWAGeneratePreviewForURL(QLPreviewRequestRef preview, NSURL *url, CFStringRef contentTypeUTI) {
  EIExeFile *exeFile;
  NSMutableString *html;
  NSDictionary *props;
  NSMutableDictionary *elem;
  NSString *icon;
  
  exeFile = [[EIExeFile alloc] initWithExeFileURL:url error:nil];
  if (!exeFile) return;
  if (QLPreviewRequestIsCancelled(preview)) return;
  
  html = [QWAGetTemplate() mutableCopy];
  elem = [NSMutableDictionary dictionary];
  if (QLPreviewRequestIsCancelled(preview)) return;
  
  /* Icon */
  icon = QWAGetBase64EncodedImageForExeFile(exeFile, contentTypeUTI, url);
  [elem setObject:icon forKey:@"ICON"];
  if (QLPreviewRequestIsCancelled(preview)) return;
  
  /* File name and 16-bit badge */
  [elem setObject:[NSString stringWithFormat:@"%d bit", [exeFile bitness]] forKey:@"BADGE"];
  [elem setObject:QWAEscapeStringForHtml([url lastPathComponent]) forKey:@"NAME"];
  
  /* Version info */
  [elem setObject:QWAHTMLVersionInfoForExeFile(exeFile) forKey:@"TABLEBODY"];
  
  /* Generate HTML */
  QWAReplaceHtmlPlaceholders(html, elem);
  
  props = @{(NSString *)kQLPreviewPropertyTextEncodingNameKey : @"UTF-8",
            (NSString *)kQLPreviewPropertyMIMETypeKey : @"text/html"};
  
  QLPreviewRequestSetDataRepresentation(preview,
    (__bridge CFDataRef)[html dataUsingEncoding:NSUTF8StringEncoding], kUTTypeHTML,
    (__bridge CFDictionaryRef)props);
}


/* -----------------------------------------------------------------------------
   Generate a preview for file

   This function's job is to create preview for designated file
   ----------------------------------------------------------------------------- */

OSStatus GeneratePreviewForURL(void *thisInterface, QLPreviewRequestRef preview, 
  CFURLRef url, CFStringRef contentTypeUTI, CFDictionaryRef options) {

  @autoreleasepool {
    QWAGeneratePreviewForURL(preview, (__bridge NSURL*)url, contentTypeUTI);
  }
  return noErr;
}


void CancelPreviewGeneration(void* thisInterface, QLPreviewRequestRef preview) {
  // implement only if supported
}

