/* win32-endian.c - Correcting endian-ness of data loaded to memory
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

#include <config.h>
#include "win32-endian.h"

#include <byteswap.h>			/* Gnulib */
#define BSWAP16(x)  	((x) = bswap_16(x))
#define BSWAP32(x)  	((x) = bswap_32(x))

#if WORDS_BIGENDIAN

void
fix_win32_cursor_icon_file_dir_endian(Win32CursorIconFileDir *obj)
{
    BSWAP16(obj->reserved);
    BSWAP16(obj->type);
    BSWAP16(obj->count);
}

void
fix_win32_bitmap_info_header_endian(Win32BitmapInfoHeader *obj)
{
    BSWAP32(obj->size);
    BSWAP32(obj->width);
    BSWAP32(obj->height);
    BSWAP16(obj->planes);
    BSWAP16(obj->bit_count);
    BSWAP32(obj->compression);
    BSWAP32(obj->size_image);
    BSWAP32(obj->x_pels_per_meter);
    BSWAP32(obj->y_pels_per_meter);
    BSWAP32(obj->clr_used);
    BSWAP32(obj->clr_important);
}

void
fix_win32_cursor_icon_file_dir_entry_endian(Win32CursorIconFileDirEntry *obj)
{
    BSWAP16(obj->hotspot_x);
    BSWAP16(obj->hotspot_y);
    BSWAP32(obj->dib_size);
    BSWAP32(obj->dib_offset);
}

void
fix_win32_image_section_header(Win32ImageSectionHeader *obj)
{
    BSWAP32(obj->misc.physical_address);
    BSWAP32(obj->virtual_address);
    BSWAP32(obj->size_of_raw_data);
    BSWAP32(obj->pointer_to_raw_data);
    BSWAP32(obj->pointer_to_relocations);
    BSWAP32(obj->pointer_to_linenumbers);
    BSWAP16(obj->number_of_relocations);
    BSWAP16(obj->number_of_linenumbers);
    BSWAP32(obj->characteristics);
}

void
fix_os2_image_header_endian(OS2ImageHeader *obj)
{
    BSWAP16(obj->magic);
    BSWAP16(obj->enttab);
    BSWAP16(obj->cbenttab);
    BSWAP32(obj->crc);
    BSWAP16(obj->flags);
    BSWAP16(obj->autodata);
    BSWAP16(obj->heap);
    BSWAP16(obj->stack);
    BSWAP32(obj->csip);
    BSWAP32(obj->sssp);
    BSWAP16(obj->cseg);
    BSWAP16(obj->cmod);
    BSWAP16(obj->cbnrestab);
    BSWAP16(obj->segtab);
    BSWAP16(obj->rsrctab);
    BSWAP16(obj->restab);
    BSWAP16(obj->modtab);
    BSWAP16(obj->imptab);
    BSWAP32(obj->nrestab);
    BSWAP16(obj->cmovent);
    BSWAP16(obj->align);
    BSWAP16(obj->cres);
    BSWAP16(obj->fastload_offset);
    BSWAP16(obj->fastload_length);
    BSWAP16(obj->swaparea);
    BSWAP16(obj->expver);
}

/* fix_win32_image_header_endian:
 * NOTE: This assumes that the optional header is always available.
 */
void
fix_win32_image_header_endian(Win32ImageNTHeaders *obj)
{
    BSWAP32(obj->signature);
    BSWAP16(obj->file_header.machine);
    BSWAP16(obj->file_header.number_of_sections);
    BSWAP32(obj->file_header.time_date_stamp);
    BSWAP32(obj->file_header.pointer_to_symbol_table);
    BSWAP32(obj->file_header.number_of_symbols);
    BSWAP16(obj->file_header.size_of_optional_header);
    BSWAP16(obj->file_header.characteristics);
    BSWAP16(obj->optional_header.magic);
    BSWAP32(obj->optional_header.size_of_code);
    BSWAP32(obj->optional_header.size_of_initialized_data);
    BSWAP32(obj->optional_header.size_of_uninitialized_data);
    BSWAP32(obj->optional_header.address_of_entry_point);
    BSWAP32(obj->optional_header.base_of_code);
    BSWAP32(obj->optional_header.base_of_data);
    BSWAP32(obj->optional_header.image_base);
    BSWAP32(obj->optional_header.section_alignment);
    BSWAP32(obj->optional_header.file_alignment);
    BSWAP16(obj->optional_header.major_operating_system_version);
    BSWAP16(obj->optional_header.minor_operating_system_version);
    BSWAP16(obj->optional_header.major_image_version);
    BSWAP16(obj->optional_header.minor_image_version);
    BSWAP16(obj->optional_header.major_subsystem_version);
    BSWAP16(obj->optional_header.minor_subsystem_version);
    BSWAP32(obj->optional_header.win32_version_value);
    BSWAP32(obj->optional_header.size_of_image);
    BSWAP32(obj->optional_header.size_of_headers);
    BSWAP32(obj->optional_header.checksum);
    BSWAP16(obj->optional_header.subsystem);
    BSWAP16(obj->optional_header.dll_characteristics);
    BSWAP32(obj->optional_header.size_of_stack_reserve);
    BSWAP32(obj->optional_header.size_of_stack_commit);
    BSWAP32(obj->optional_header.size_of_heap_reserve);
    BSWAP32(obj->optional_header.size_of_heap_commit);
    BSWAP32(obj->optional_header.loader_flags);
    BSWAP32(obj->optional_header.number_of_rva_and_sizes);
}

void
fix_win32_image_data_directory(Win32ImageDataDirectory *obj)
{
    BSWAP32(obj->virtual_address);
    BSWAP32(obj->size);
}

#endif /* WORDS_BIGENDIAN */
