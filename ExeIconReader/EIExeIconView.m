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

#import "EIExeIconView.h"
#import "EIExeFile.h"
#import "EIVersionInfo.h"
#import "EIMainWindowDelegate.h"


@implementation EIExeIconView


- (void)setImageFromExe:(NSURL *)exefile {
  NSImage *icoImage;
  NSError *err;
  
  exf = [[EIExeFile alloc] initWithExeFileURL:exefile error:&err];
  
  if (exf) {
    icoImage = [exf icon];
    [self setImage:icoImage];
  } else {
    [[self window] presentError:err];
  }
  
  [self sendAction:[self action] to:[self target]];
}


- (EIExeFile *)exeFile {
  return exf;
}


- (NSDragOperation)draggingEntered:(id <NSDraggingInfo>)sender {
  if ((NSDragOperationGeneric & [sender draggingSourceOperationMask]) == NSDragOperationGeneric)
    return NSDragOperationCopy;
  else
    return NSDragOperationNone;
}


- (BOOL)performDragOperation:(id <NSDraggingInfo>)sender {
  NSPasteboard *pboard;
  NSURL* finderUrl;

  pboard = [sender draggingPasteboard];
  finderUrl = [NSURL URLFromPasteboard:pboard];
  if (finderUrl == nil) return NO;

  [self setImageFromExe:finderUrl];
  
  return YES;
}


@end
