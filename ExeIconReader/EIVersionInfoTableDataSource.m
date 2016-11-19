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

#import "EIVersionInfoTableDataSource.h"
#import "EIVersionInfo.h"


@implementation EIVersionInfoTableDataSource


- init {
  self = [super init];
  list = nil;
  return self;
}


- (NSInteger)numberOfRowsInTableView:(NSTableView *)aTableView {
  return [list count];
}


- (id)tableView:(NSTableView *)aTableView objectValueForTableColumn:(NSTableColumn *)aTableColumn row:(NSInteger)rowIndex {
  return [list objectAtIndex:rowIndex];
}


- (void)loadFromVersionInfo:(EIVersionInfo*)vir {
  NSArray *resSrch;
  NSString *node, *keyPath, *value;
  
  list = [[NSMutableArray alloc] init];
  
  resSrch = [vir querySubNodesUnder:@"\\StringFileInfo\\*" error:NULL];
  
  for (node in resSrch) {
    keyPath = [NSString stringWithFormat:@"\\StringFileInfo\\*\\%@", node];
    value = [vir queryStringValue:keyPath error:NULL];
    if (value)
      [list addObject:[NSString stringWithFormat:@"%@: %@", node, value]];
  }
}


- (void)loadFromEIExeFile:(EIExeFile*)exfile {
  [self loadFromVersionInfo:[exfile versionInfo]];
}


@end
