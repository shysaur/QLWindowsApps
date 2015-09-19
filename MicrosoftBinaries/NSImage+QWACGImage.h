//
//  NSImage+QWACGImage.h
//  QLWindowsApps
//
//  Created by Daniele Cattaneo on 19/09/15.
//
//

#import <Cocoa/Cocoa.h>


@interface NSImage (QWACGImage)


- (CGImageRef)qwa_CGImageForProposedRect:(NSRect *)rect;


@end
