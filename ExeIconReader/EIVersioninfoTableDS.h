//
//  EIVersioninfoTableDS.h
//  EXEIconReader
//
//  Created by Daniele Cattaneo on 19/11/12.
//  Copyright 2012 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "EIExeFile.h"


@interface EIVersioninfoTableDS : NSObject <NSTableViewDataSource> {
  NSMutableArray* list;
}

- init;
- (void)dealloc;

- (NSInteger)numberOfRowsInTableView:(NSTableView *)aTableView;
- (id)tableView:(NSTableView *)aTableView objectValueForTableColumn:(NSTableColumn *)aTableColumn row:(NSInteger)rowIndex;

- (void)loadFromVersioninfo:(NSData*)verInfo is16Bit:(BOOL)bitness;
- (void)loadFromEIExeFile:(EIExeFile*)exfile;

@end
