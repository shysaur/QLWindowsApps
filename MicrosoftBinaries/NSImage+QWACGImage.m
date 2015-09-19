//
//  NSImage+QWACGImage.m
//  QLWindowsApps
//
//  Created by Daniele Cattaneo on 19/09/15.
//
//

#import "NSImage+QWACGImage.h"


NSComparisonResult QWACompareSizes(NSSize a, NSSize b) {
  CGFloat ad, bd;
  
  ad = a.height * a.height + a.width * a.width;
  bd = b.height * b.height + b.width * b.width;
  if (ad > bd)
    return NSOrderedDescending;
  else if (ad == bd)
    return NSOrderedSame;
  return NSOrderedAscending;
}


@implementation NSImage (QWACGImage)


- (CGImageRef)qwa_CGImageForProposedRect:(NSRect *)rect {
  NSArray *reps;
  NSImageRep *rep;
  NSSize maxrepsize;
  NSInteger i, maxrep;
  
  reps = [self representations];
  
  i = 0;
  maxrep = -1;
  maxrepsize = NSZeroSize;
  for (rep in reps) {
    if ([rep isKindOfClass:[NSBitmapImageRep class]]) {
      if (QWACompareSizes([rep size], maxrepsize) == NSOrderedDescending) {
        if (QWACompareSizes(rect->size, [rep size]) != NSOrderedAscending) {
          maxrepsize = [rep size];
          maxrep = i;
        }
      }
    }
    i++;
  }
  
  if (maxrep == -1)
    return NULL;
  
  rep = [reps objectAtIndex:maxrep];
  return [(NSBitmapImageRep*)rep CGImage];
}


@end
