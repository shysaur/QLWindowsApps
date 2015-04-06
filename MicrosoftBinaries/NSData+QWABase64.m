/* NSData+QWABase64.m
 *
 * Copyright (C) 2015 Daniele Cattaneo
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

#import "NSData+QWABase64.h"


@implementation NSData (QWABase64)


- (NSString*)qwa_base64Encoding {
  const char *base64 = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  const uint8_t *data;
  char *bytes, *bp;
  uint8_t tb;
  NSInteger length, i, s, reslength;
  
  data = [self bytes];
  length = [self length];
  
  reslength = length * 4 / 3 + 8;
  bytes = bp = malloc(reslength);
  if (!bytes) return nil;
  
  s = 0;
  for (i=0; i<length; i++) {
    switch (s) {
      case 0:
        *(bp++) = base64[*data >> 2];
        tb = (*(data++) & 0b11) << 4;
        s++;
        break;
      case 1:
        *(bp++) = base64[(*data >> 4) | tb];
        tb = (*(data++) & 0b1111) << 2;
        s++;
        break;
      case 2:
        *(bp++) = base64[(*data >> 6) | tb];
        *(bp++) = base64[*(data++) & 0b111111];
        s = 0;
    }
  }
  if (s) {
    *(bp++) = base64[tb];
    for (i=s; i<3; i++)
      *(bp++) = '=';
  }
  reslength = bp - bytes;
  
  return [[[NSString alloc] initWithBytesNoCopy:bytes length:reslength encoding:NSASCIIStringEncoding freeWhenDone:YES] autorelease];
}


@end
