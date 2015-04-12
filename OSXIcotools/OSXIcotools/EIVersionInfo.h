/* EIVersionInfoReader.h - Class used to interpret VERSIONINFO structures
 *
 * Copyright (C) 2012-13 Daniele Cattaneo
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

#import <Cocoa/Cocoa.h>

typedef enum {
  EIV_NOERR = 0,
  EIV_UNKNOWNNODE = 1,
  EIV_WRONGFORMAT = 2,
  EIV_WRONGDATA = 3
} EIVERSION_ERR;


@interface EIVersionInfo : NSObject {
  NSData* gBlock;
  BOOL win16Block;
}

- (instancetype)initWithData:(NSData *)myBlock is16Bit:(BOOL)win16;
- (void)dealloc;

- (BOOL)is16bit;
- (NSData *)data;

- (NSData*)queryValue:(NSString*)subBlock error:(EIVERSION_ERR*)err;
- (NSArray*)querySubNodesUnder:(NSString*)subBlock error:(EIVERSION_ERR*)err;

@end


