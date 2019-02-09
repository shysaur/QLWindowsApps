/* extract.c - Extract icon and cursor files from PE and NE files
 * 
 * Copyright (C) 1998 Oskar Liljeblad
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

#include <config.h>
#include "gettext.h"			/* Gnulib */
#define _(s) gettext(s)
#define N_(s) gettext_noop(s)
#include "xalloc.h"			/* Gnulib */
#include "common/log.h"
#include "common/intutil.h"
#include "win32.h"
#include "win32-endian.h"
#include "fileread.h"
#include "extract.h"
#include "dirname.h"
#include "restypes.h"
#include "restable.h"
#include "fileread.h"


#define SET_IF_NULL(x,def) ((x) = ((x) == NULL ? (def) : (x)))

#define STRIP_RES_ID_FORMAT(x) (x != NULL && (x[0] == '-' || x[0] == '+') ? ++x : x)


static void *extract_group_icon_cursor_resource(WinLibrary *, WinResource *, char *, size_t *, bool, wres_error *);
static void *extract_bitmap_resource(WinLibrary *, WinResource *, size_t *, wres_error *);


/* extract_resource:
 *   Extract a resource, returning pointer to data.
 */
void *
extract_resource (WinLibrary *fi, WinResource *wr, size_t *size,
                  bool *free_it, char *type, char *lang, bool raw, wres_error *err)
{
	char *str;
	int32_t intval;
	
	/* just return pointer to data if raw */
	if (raw) {
		*free_it = false;
		return get_resource_entry(fi, wr, size, err);
	}

	/* find out how to extract */
	str = type;
	if (str != NULL && parse_int32(STRIP_RES_ID_FORMAT(str), &intval)) {
		if (intval == (int) RT_BITMAP) {
			*free_it = true;
			return extract_bitmap_resource(fi, wr, size, err);
		}
		if (intval == (int) RT_GROUP_ICON) {
			*free_it = true;
			return extract_group_icon_cursor_resource(fi, wr, lang, size, true, err);
		}
		if (intval == (int) RT_GROUP_CURSOR) {
			*free_it = true;
			return extract_group_icon_cursor_resource(fi, wr, lang, size, false, err);
		}
		if (intval == (int) RT_VERSION) {
			*free_it = false;
			return get_resource_entry(fi, wr, size, err); //no conversion for versioninfo
		}
	}

	if (err) *err = WRES_ERROR_UNSUPPRESTYPE;
	return NULL;
}

/* extract_group_icon_resource:
 *   Create a complete RT_GROUP_ICON resource, that can be written to
 *   an `.ico' file without modifications. Returns an allocated
 *   memory block that should be freed with free() once used.
 *
 *   `root' is the offset in file that specifies the resource.
 *   `base' is the offset that string pointers are calculated from.
 *   `ressize' should point to an integer variable where the size of
 *   the returned memory block will be placed.
 *   `is_icon' indicates whether resource to be extracted is icon
 *   or cursor group.
 */
static void *
extract_group_icon_cursor_resource(WinLibrary *fi, WinResource *wr, char *lang,
                                   size_t *ressize, bool is_icon, wres_error *err)
{
	Win32CursorIconDir *icondir;
	Win32CursorIconFileDir *fileicondir;
	char *memory;
	int c, offset, skipped;
	size_t size;

	/* get resource data and size */
	icondir = (Win32CursorIconDir *) get_resource_entry(fi, wr, &size, err);
	if (icondir == NULL)
		return NULL;

	/* calculate total size of output file */
	RET_NULL_AND_SET_ERR_IF_BAD_POINTER(fi, err, icondir->count);
	skipped = 0;
	for (c = 0 ; c < icondir->count ; c++) {
		int level;
		size_t iconsize;
		char name[14];
		WinResource *fwr;

		RET_NULL_AND_SET_ERR_IF_BAD_POINTER(fi, err, icondir->entries[c]);
		/*printf("%d. bytes_in_res=%d width=%d height=%d planes=%d bit_count=%d\n", c,
			icondir->entries[c].bytes_in_res,
			(is_icon ? icondir->entries[c].res_info.icon.width : icondir->entries[c].res_info.cursor.width),
			(is_icon ? icondir->entries[c].res_info.icon.height : icondir->entries[c].res_info.cursor.height),
			icondir->entries[c].plane_count,
			icondir->entries[c].bit_count);*/
      
		/* find the corresponding icon resource */
		snprintf(name, sizeof(name)/sizeof(char), "-%d", icondir->entries[c].res_id);
		fwr = find_resource(fi, (is_icon ? "-3" : "-1"), name, "", &level, err);
		//The empty string tells find_resource to ignore the value of the language id. Some EXEs have GROUP_ICONS
		//with a different language ID than the ICONs themselves.
		if (fwr == NULL)
			return NULL;

		if (get_resource_entry(fi, fwr, &iconsize, err) != NULL) {
			if (iconsize == 0) {
				dbg_log(_("%s: icon resource `%s' is empty, skipping"), fi->name, name);
				skipped++;
				continue;
			}
			if (iconsize != icondir->entries[c].bytes_in_res) {
				dbg_log(_("%s: mismatch of size in icon resource `%s' and group (%d vs %d)"), fi->name, name, iconsize, icondir->entries[c].bytes_in_res);
			}
			size += iconsize < icondir->entries[c].bytes_in_res ? icondir->entries[c].bytes_in_res : iconsize;

			/* cursor resources have two additional WORDs that contain
			 * hotspot info */
			if (!is_icon)
				size -= sizeof(uint16_t)*2;
		} else {
			free(fwr);
			return NULL;
		}
		free(fwr);
	}
  
	offset = sizeof(Win32CursorIconFileDir) + (icondir->count-skipped) * sizeof(Win32CursorIconFileDirEntry);
	size += offset;
	*ressize = size;

	/* allocate that much memory */
	memory = xmalloc(size);
	fileicondir = (Win32CursorIconFileDir *) memory;

	/* transfer Win32CursorIconDir structure members */
	fileicondir->reserved = icondir->reserved;
	fileicondir->type = icondir->type;
	fileicondir->count = icondir->count - skipped;

	/* transfer each cursor/icon: Win32CursorIconDirEntry and data */
	skipped = 0;
	for (c = 0 ; c < icondir->count ; c++) {
		int level;
		char name[14];
		WinResource *fwr;
		char *data;
  
		/* find the corresponding icon resource */
		snprintf(name, sizeof(name)/sizeof(char), "-%d", icondir->entries[c].res_id);
		fwr = find_resource(fi, (is_icon ? "-3" : "-1"), name, "", &level, err);
		if (fwr == NULL)
			return NULL;

		/* get data and size of that resource */
		data = get_resource_entry(fi, fwr, &size, err);
		if (data == NULL)
			return NULL;

		if (size == 0) {
		    skipped++;
		    continue;
		}

		/* copy ICONDIRENTRY (not including last dwImageOffset) */
		memcpy(&fileicondir->entries[c-skipped], &icondir->entries[c],
			sizeof(Win32CursorIconFileDirEntry)-sizeof(uint32_t));

		if (size >= sizeof(Win32BitmapInfoHeader)) {
			const uint8_t pngh[8] = {137, 80, 78, 71, 13, 10, 26, 10};
			if (memcmp(data, pngh, 8) != 0) {
				/* don't trust the size specified in ICONDIRENTRY because there are
				 * some people who manage to get it wrong, believe it or not */
				Win32BitmapInfoHeader *bim = (Win32BitmapInfoHeader *)data;
				fileicondir->entries[c-skipped].width = bim->width;
				fileicondir->entries[c-skipped].height = bim->height / 2;
			} else {
				/* do not trust ICONDIRENTRY for PNG icons
				 * fixes cases in which big PNG icons were not prioritized
				 * over small DIB icons */
				fileicondir->entries[c-skipped].color_count = 0;
				fileicondir->entries[c-skipped].hotspot_y = 1;
				fileicondir->entries[c-skipped].hotspot_y = 32;
			}
		}
		
		/* special treatment for cursors */
		if (!is_icon) {
			fileicondir->entries[c-skipped].color_count = 0;
			fileicondir->entries[c-skipped].reserved = 0;
		}

		/* set image offset and increase it */
		fileicondir->entries[c-skipped].dib_offset = offset;

		/* transfer resource into file memory */
		if (size > icondir->entries[c-skipped].bytes_in_res)
			size = icondir->entries[c-skipped].bytes_in_res;
		if (is_icon) {
			/* Better to trust the resource itself. Fixes crash with ISCC.exe */
			memcpy(&memory[offset], data, size);
		} else if (size >= sizeof(uint16_t)*2) {
			fileicondir->entries[c-skipped].hotspot_x = ((uint16_t *) data)[0];
			fileicondir->entries[c-skipped].hotspot_y = ((uint16_t *) data)[1];
			memcpy(&memory[offset], data+sizeof(uint16_t)*2,
				   size-sizeof(uint16_t)*2);
			offset -= sizeof(uint16_t)*2;
		}

		/* increase the offset pointer */
		offset += icondir->entries[c].bytes_in_res;
		free(fwr);
	}

	return (void *) memory;
}

/* extract_bitmap_resource:
 *   Create a complete RT_BITMAP resource, that can be written to
 *   an `.bmp' file without modifications. Returns an allocated
 *   memory block that should be freed with free() once used.
 *
 *   `ressize' should point to an integer variable where the size of
 *   the returned memory block will be placed.
 */
static void *
extract_bitmap_resource(WinLibrary *fi, WinResource *wr, size_t *ressize, wres_error *err)
{
    Win32BitmapInfoHeader info;
    uint8_t *result;
    uint8_t *resentry;
    uint32_t offbits;
    size_t size;

    resentry=(uint8_t *)(get_resource_entry(fi, wr, &size, err));
    if (!resentry)
		return NULL;

    /* Bitmap file consists of:
     * 1) File header (14 bytes)
     * 2) Bitmap header (40 bytes)
     * 3) Colormap (size depends on a few things)
     * 4) Pixels data
     *
     * parts 2-4 are present in the resource data, we need just
     * to add a file header, which contains file size and offset
     * from file beginning to pixels data.
     */

    /* Get the bitmap info */
    memcpy(&info,resentry,sizeof(info));
    fix_win32_bitmap_info_header_endian(&info);

    /* offbits - offset from file start to the beginning
     *           of the first pixel data */
    offbits = info.size+14;

    /* In 24-bit bitmaps there's no colormap
     * The size of an entry in colormap is 4
     */
    if (info.bit_count!=24) {

        /* 0 value of clr_used means that all possible color
        * entries are used */
       if (info.clr_used==0) {
           switch (info.bit_count) {
               case 1:    /* Monochrome bitmap */
                   offbits += 8;
                   break;
               case 4:    /* 16 colors bitmap */
                   offbits += 64;
                   break;
               case 8:    /* 256 colors bitmap */
                  offbits += 1024;
           }
       } else {
           offbits += 4 * info.clr_used;
       }
    }

    /* The file will consist of the resource data and
     * 14 bytes long file header */
    *ressize = 14+size;
    result = (uint8_t *)xmalloc(*ressize);

    /* Filling the file header with data */
    result[0] = 'B';   /* Magic char #1 */
    result[1] = 'M';   /* Magic char #2 */
    result[2] = (*ressize & 0x000000ff);      /* file size, little-endian */
    result[3] = (*ressize & 0x0000ff00)>>8;
    result[4] = (*ressize & 0x00ff0000)>>16;
    result[5] = (*ressize & 0xff000000)>>24;
    result[6] = 0; /* Reserved */
    result[7] = 0;
    result[8] = 0;
    result[9] = 0;
    result[10] = (offbits & 0x000000ff);  /* offset to pixels, little-endian */
    result[11] = (offbits & 0x0000ff00)>>8;
    result[12] = (offbits & 0x00ff0000)>>16;
    result[13] = (offbits & 0xff000000)>>24;

    /* The rest of the file is the resource entry */
    memcpy(result+14,resentry,size);

    return result;
}
