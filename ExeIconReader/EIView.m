/* EIView.m
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

#import "EIView.h"
#import "EIExeFile.h"
#import "EIVersionInfoReader.h"


@implementation EIView


- (void)setIconForExe:(NSURL *)exefile {
  NSImage *icoImage;
  
  EIExeFile *exf = [[EIExeFile alloc] initWithExeFileURL:exefile];
  if (exf) {
    icoImage = [exf icon];
    [self setImage:icoImage];
    
    MDItemRef mdirf = MDItemCreateWithURL(kCFAllocatorDefault, (CFURLRef)exefile);
    if (mdirf) {
      CFBooleanRef custico = MDItemCopyAttribute(mdirf, kMDItemFSHasCustomIcon);
      if (custico == kCFBooleanFalse || custico == NULL) {
        [[NSWorkspace sharedWorkspace] setIcon:icoImage forFile:[exefile path] options:0];
      }
    }
    [NSApp setApplicationIconImage: icoImage];
    
    [vinfods loadFromEIExeFile:exf];
    [tableView reloadData];
  }
  [exf release];
}


- (NSDragOperation)draggingEntered:(id <NSDraggingInfo>)sender {
  if ((NSDragOperationGeneric & [sender draggingSourceOperationMask]) == NSDragOperationGeneric)
    return NSDragOperationCopy;
  else
    return NSDragOperationNone;
}


- (BOOL)performDragOperation:(id <NSDraggingInfo>)sender {
	NSPasteboard *pboard = [sender draggingPasteboard];
  NSURL* finderUrl = [NSURL URLFromPasteboard:pboard];

  if (finderUrl == nil) return NO;

  [self setIconForExe:finderUrl];
  
  [self setNeedsDisplay:YES];    //redraw us with the new image
  return YES;
}


@end
