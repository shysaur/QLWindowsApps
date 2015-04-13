/* EIVersioninfoTableDS.m
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

#import "EIVersioninfoTableDS.h"
#import "EIVersionInfo.h"


@implementation EIVersioninfoTableDS


- init {
  self = [super init];
  list = nil;
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


- (void)loadFromVersionInfo:(EIVersionInfo*)vir {
  EIVERSION_ERR error;
  [list release];
  list = [[NSMutableArray alloc] init];
  
  NSString *queryHeader = @"\\StringFileInfo\\*";
  NSArray *resSrch = [vir querySubNodesUnder:queryHeader error:&error];
  
  NSStringEncoding resEnc;
  if ([vir is16bit]) {
    resEnc = NSWindowsCP1252StringEncoding;
  } else {
    resEnc = NSUTF16LittleEndianStringEncoding;
  }
  
  for (int c=0; c<[resSrch count]; c++) {
    NSData* item = [vir queryValue:[NSString stringWithFormat:@"%@\\%@", queryHeader, [resSrch objectAtIndex:c]] error:&error];
    if (item) {
      NSString* temp = [[NSString alloc] initWithData:item encoding:resEnc];
      [list addObject:[NSString stringWithFormat:@"%@: %@", [resSrch objectAtIndex:c], temp]];
      [temp release];
    }
  }
}


- (void)loadFromEIExeFile:(EIExeFile*)exfile {
  [self loadFromVersionInfo:[exfile versionInfo]];
}


@end
