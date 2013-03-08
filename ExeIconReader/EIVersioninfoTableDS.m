//
//  EIVersioninfoTableDS.m
//  EXEIconReader
//
//  Created by Daniele Cattaneo on 19/11/12.
//  Copyright 2012 __MyCompanyName__. All rights reserved.
//

#import "EIVersioninfoTableDS.h"
#import "EIVersionInfoReader.h"

@implementation EIVersioninfoTableDS

- init {
  self = [super init];
  list = [[NSMutableArray alloc] init];
  return self;
}

- (void)dealloc {
  [list release];
  [super dealloc];
}

- (NSInteger)numberOfRowsInTableView:(NSTableView *)aTableView {
  return [list count];
}

- (id)tableView:(NSTableView *)aTableView objectValueForTableColumn:(NSTableColumn *)aTableColumn row:(NSInteger)rowIndex {
  return [list objectAtIndex:rowIndex];
}

- (void)loadFromVersioninfo:(NSData*)verInfo is16Bit:(BOOL)bitness {
  EIVERSION_ERR error;
  [list release];
  list = [[NSMutableArray alloc] init];
  
  EIVersionInfoReader* vir = [[[EIVersionInfoReader alloc] initWithBlock:verInfo is16Bit:bitness] autorelease];
  NSData* transl = [vir queryValue:@"\\VarFileInfo\\Translation" error:&error];
  if (transl == nil) return;
  TRANSLATION def_transl = [vir bestLocaleWithError:&error];
  NSString* queryHeader = [NSString stringWithFormat:@"\\StringFileInfo\\%04X%04X", def_transl.lang, def_transl.coding ];
  
  NSArray *resSrch = [vir querySubNodesUnder:queryHeader error:&error];
  if (error) {
    //Sometimes the translation table does not reflect the actual translations available...
    queryHeader = @"\\StringFileInfo\\*";
    resSrch = [vir querySubNodesUnder:queryHeader error:&error];
  }
  
  NSStringEncoding resEnc = NSEncodingFromCodePage(def_transl.coding);  //Trusting the coding parameter...
  //Boy, the coding parameter can be trusted even *less* freqently than the language ID... How many VerInfo resources
  //out there have 1252 coding while they're just plain UTF16!!
  //I guess more people would be conforming to the version resource format if Microsoft publicized it better.
  
  for (int c=0; c<[resSrch count]; c++) {
    NSData* item = [vir queryValue:[NSString stringWithFormat:@"%@\\%@", queryHeader, [resSrch objectAtIndex:c]] error:&error];
    if (!error) {
      NSString* temp = [[NSString alloc] initWithData:item encoding:resEnc];
      [list addObject:[NSString stringWithFormat:@"%@: %@", [resSrch objectAtIndex:c], temp]];
      [temp release];
    }
  }
};

- (void)loadFromEIExeFile:(EIExeFile*)exfile {
  [self loadFromVersioninfo:[exfile getVersionInfo] is16Bit:[exfile is16Bit]];
}

@end
