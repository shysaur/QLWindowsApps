/* EIMainWindowDelegate.m
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

#import "EIMainWindowDelegate.h"
#import "EIExeFile.h"
#import "EIExeIconView.h"
#import "EIVersionInfoTableDataSource.h"


@implementation EIMainWindowDelegate


- (BOOL)windowShouldClose:(id)sender {
  [app terminate:self];
  return YES;
}


- (void)takeExeFileValueFrom:(id)sender {
  EIVersionInfoTableDataSource *ds;
  EIExeFile *exf;
  NSWorkspace *ws;
  
  exf = [sender exeFile];

  ws = [NSWorkspace sharedWorkspace];
  [ws setIcon:[imageView image] forFile:[[exf url] path] options:0];
  [NSApp setApplicationIconImage:[imageView image]];
  
  ds = [tableView dataSource];
  [ds loadFromEIExeFile:exf];
  [tableView reloadData];
}


@end
