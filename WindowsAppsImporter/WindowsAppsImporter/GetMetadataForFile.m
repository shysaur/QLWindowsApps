//
//  GetMetadataForFile.m
//  WindowsAppsImporter
//
//  Created by Daniele Cattaneo on 16/12/17.
//  Copyright © 2017 danielecattaneo. All rights reserved.
//

#include <CoreFoundation/CoreFoundation.h>
#import <Foundation/Foundation.h>
#import "EIExeFile.h"
#import "EIVersionInfo.h"


Boolean GetMetadataForFile(void *thisInterface, CFMutableDictionaryRef attributes, CFStringRef contentTypeUTI, CFStringRef pathToFile);


BOOL EIMetadataForFile(NSURL *url, NSMutableDictionary *attr)
{
  static NSSet *recognizedTags;
  static dispatch_once_t onceToken;
  dispatch_once(&onceToken, ^{
    recognizedTags = [NSSet setWithArray:@[
      @"com_danielecattaneo_windowsappsimporter_companyname",
      @"com_danielecattaneo_windowsappsimporter_filedescription",
      @"com_danielecattaneo_windowsappsimporter_fileversion",
      @"com_danielecattaneo_windowsappsimporter_internalname",
      @"com_danielecattaneo_windowsappsimporter_legaltrademarks",
      @"com_danielecattaneo_windowsappsimporter_originalfilename",
      @"com_danielecattaneo_windowsappsimporter_privatebuild",
      @"com_danielecattaneo_windowsappsimporter_productname"]];
  });

  EIExeFile *f = [[EIExeFile alloc] initWithExeFileURL:url error:nil];
  if (!f)
    return NO;
  
  NSString *fmt = [NSString stringWithFormat:@"%d bit", [f bitness]];
  [attr setObject:fmt forKey:@"com_danielecattaneo_windowsappsimporter_binaryformat"];
  
  EIVersionInfo *vir = [f versionInfo];
  if (vir) {
    NSString *queryHeader = @"\\StringFileInfo\\*";
    NSArray *resSrch = [vir querySubNodesUnder:queryHeader error:NULL];
    
    if (resSrch) {
      for (NSString *node in resSrch) {
        NSString *vpath = [NSString stringWithFormat:@"%@\\%@", queryHeader, node];
        NSString *value = [vir queryStringValue:vpath error:NULL];
        if (!value || [@"" isEqual:value])
          continue;
        
        NSString *destkey;
        if ([node isEqual:@"Comments"])
          destkey = (NSString *)kMDItemComment;
        else if ([node isEqual:@"LegalCopyright"])
          destkey = (NSString *)kMDItemCopyright;
        else {
          NSString *test = [@"com_danielecattaneo_windowsappsimporter_" stringByAppendingString:[node lowercaseString]];
          if ([recognizedTags containsObject:test])
            destkey = test;
        }
        
        if (destkey)
          [attr setObject:value forKey:destkey];
      }
    }
  }
  
  return YES;
}


//==============================================================================
//
//  Get metadata attributes from document files
//
//  The purpose of this function is to extract useful information from the
//  file formats for your document, and set the values into the attribute
//  dictionary for Spotlight to include.
//
//==============================================================================

Boolean GetMetadataForFile(void *thisInterface, CFMutableDictionaryRef attributes, CFStringRef contentTypeUTI, CFStringRef pathToFile)
{
  Boolean res;
  
  @autoreleasepool {
    NSMutableDictionary *attr = (__bridge NSMutableDictionary *)(attributes);
    NSURL *url = [[NSURL alloc] initFileURLWithPath:(__bridge NSString * _Nonnull)(pathToFile)];
    res = EIMetadataForFile(url, attr);
  }
  return res;
}
