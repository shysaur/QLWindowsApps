//
//  EIExeFile.m
//  EXEIconReader
//
//  Created by Daniele Cattaneo on 18/11/12.
//  Copyright 2012 __MyCompanyName__. All rights reserved.
//

#import "EIExeFile.h"
#include <stdlib.h>
#include "io-utils.h"
#include "osxwres.h"

@implementation EIExeFile

- initWithExeFileURL:(NSURL*)exeFile error:(BOOL*)err; {
  BOOL error;
  error = NO;
  
  /* initiate stuff */
  fl.file = NULL;
  fl.memory = NULL;

  /* get file size */
  sprintf(exename, "%s", [[exeFile path] UTF8String]);
  fl.name = exename;
  fl.total_size = (int)file_size(fl.name);
  if (fl.total_size == -1) {
    NSLog(@"%s total size = -1", fl.name);
    error = YES;
    goto cleanup;
  }
  if (fl.total_size == 0) {
    NSLog(@"%s: file has a size of 0", fl.name);
    error = YES;
    goto cleanup;
  }

  /* open file */
  fl.file = fopen(fl.name, "rb");
  if (fl.file == NULL) {
    NSLog(@"%s error opening file", fl.name);
    error = YES;
    goto cleanup;
  }
  
  /* read all of file */
  fl.memory = malloc(fl.total_size);
  if (fread(fl.memory, fl.total_size, 1, fl.file) != 1) {
    NSLog(@"%s error reading file contents", fl.name);
    error = YES;
    goto cleanup;
  }

  /* identify file and find resource table */
  if (!read_library (&fl)) {
    /* error reported by read_library */
    error = YES;
    goto cleanup;
  }
  
  cleanup:
  *err = error;
  return [super init];
}

- (NSImage*)getIconNSImage {
  extract_error err;
  NSData *icodata = nsdata_default_resource(&fl, "14", NULL, NULL, &err);
    
  if (err) {
    if (err == EXTR_NOTFOUND)
      NSLog(@"%s: suitable resource not found", fl.name);
    else
      NSLog(@"%s: error in extracting resource", fl.name);
    return nil;
  }
  
  NSImage *timg = [[[NSImage alloc] initWithData:icodata] autorelease];
  return timg;
}

- (NSData*)getVersionInfo {
  extract_error err;
  uint32_t sysLocale = [NSLocale windowsLocaleCodeFromLocaleIdentifier:[[NSLocale currentLocale] localeIdentifier]];
  
  char sysLocaleStr[64];
  sprintf(sysLocaleStr, "%d", sysLocale);
  //try with the current selected locale in the OS
  NSData *verdata = nsdata_default_resource(&fl, "16", NULL, sysLocaleStr, &err);
  if (err) {
    //if failure, try the en-US locale (the majority of apps, if they're not neutral, use this)
    verdata = nsdata_default_resource(&fl, "16", NULL, "1033", &err);
    if (err) {
      //else, pick the first locale we find, and go with it.
      verdata = nsdata_default_resource(&fl, "16", NULL, NULL, &err);
    }
  }
  
  if (err) {
    if (err == EXTR_NOTFOUND)
      NSLog(@"%s: suitable resource not found", fl.name);
    else
      NSLog(@"%s: error in extracting resource", fl.name);
    return nil;
  }
  return verdata;
}

- (BOOL)is16Bit {
  return !(fl.is_PE_binary);
}

- (void)dealloc {
  if (fl.file != NULL)
    fclose(fl.file);
  if (fl.memory != NULL)
    free(fl.memory);
  [super dealloc];
}

@end
