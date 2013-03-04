/* win32-endian.h - Correcting endian-ness of data loaded to memory
 *
 * Copyright (C) 1998 Oskar Liljeblad
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

#ifndef WIN32_ENDIAN_H
#define WIN32_ENDIAN_H

#include "win32.h"

#if WORDS_BIGENDIAN

void fix_win32_cursor_icon_file_dir_endian(Win32CursorIconFileDir *obj);
void fix_win32_bitmap_info_header_endian(Win32BitmapInfoHeader *obj);
void fix_win32_cursor_icon_file_dir_entry_endian(Win32CursorIconFileDirEntry *obj);
void fix_win32_image_data_directory(Win32ImageDataDirectory *obj);
void fix_os2_image_header_endian(OS2ImageHeader *obj);
void fix_win32_image_section_header(Win32ImageSectionHeader *obj);
void fix_win32_image_header_endian(Win32ImageNTHeaders *obj);

#else

#define fix_win32_bitmap_info_header_endian(x)
#define fix_win32_cursor_icon_file_dir_endian(x)
#define fix_win32_cursor_icon_file_dir_entry_endian(x)
#define fix_win32_image_data_directory(x)
#define fix_os2_image_header_endian(x)
#define fix_win32_image_section_header(x)
#define fix_win32_image_header_endian(x)

#endif /* WORDS_BIGENDIAN */

#endif /* WIN32_ENDIAN_H */
