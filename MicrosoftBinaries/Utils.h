/* Utils.h
 *
 * Copyright (C) 2016 Daniele Cattaneo
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

#import <Foundation/Foundation.h>


/* Undocumented thumbnail properties
 * Credit to @Marginal */

extern const CFStringRef kQLThumbnailPropertyIconFlavorKey_10_5;
extern const CFStringRef kQLThumbnailPropertyIconFlavorKey_10_15;

typedef NS_ENUM(NSInteger, QLThumbnailIconFlavor)
{
    kQLThumbnailIconPlainFlavor     = 0,
    kQLThumbnailIconShadowFlavor    = 1,
    kQLThumbnailIconBookFlavor      = 2,
    kQLThumbnailIconMovieFlavor     = 3,
    kQLThumbnailIconAddressFlavor   = 4,
    kQLThumbnailIconImageFlavor     = 5,
    kQLThumbnailIconGlossFlavor     = 6,
    kQLThumbnailIconSlideFlavor     = 7,
    kQLThumbnailIconSquareFlavor    = 8,
    kQLThumbnailIconBorderFlavor    = 9,
    // = 10,
    kQLThumbnailIconCalendarFlavor  = 11,
    kQLThumbnailIconPatternFlavor   = 12,
};


NSUserDefaults *QWAUserDefaults(void);


