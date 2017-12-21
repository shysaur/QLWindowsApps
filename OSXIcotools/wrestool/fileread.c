/* fileread.c - Offset checking routines for file reading
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
#include <stdio.h>		/* C89 */
#include <stdlib.h>		/* C89 */
#include <string.h>		/* C89 */
#include <stdbool.h>		/* POSIX/Gnulib */
#include "gettext.h"			/* Gnulib */
#define _(s) gettext(s)
#define N_(s) gettext_noop(s)
#include "common/error.h"
#include "common/common.h"
#include "fileread.h"
#include "win32.h"
#include "xalloc.h"		/* Gnulib */
#include "minmax.h"		/* Gnulib */


static off_t calc_vma_size (WinLibrary *);


/* check_offset:
 *   Check if a chunk of data (determined by offset and size)
 *   is within the bounds of the WinLibrary file.
 *   Usually not called directly.
 */
bool
check_offset(char *memory, off_t total_size, char *name, void *offset, off_t size)
{
	off_t need_size = (off_t) ((char *) offset - memory + size);

	/*debug("check_offset: size=%x vs %x offset=%x size=%x\n",
		need_size, total_size, (char *) offset - memory, size);*/

	if (need_size < 0 || need_size > total_size) {
		warn(_("%s: premature end"), name);
		return false;
	}

	return true;
}


/* read_library:
 *
 * Read header and get resource directory offset in a Windows library
 * (AKA module).
 */
bool
read_library (WinLibrary *fi)
{
	fseek(fi->file, 0, SEEK_END);
	fi->total_size = ftello(fi->file);
	fseek(fi->file, 0, SEEK_SET);
	if (fi->total_size == -1) {
		fprintf(stderr, "%s total size = -1", fi->name);
		return false;
	}
	if (fi->total_size == 0) {
		fprintf(stderr, "%s: file has a size of 0", fi->name);
		return false;
	}
	
	/* read all of file */
	fi->memory = malloc(fi->total_size);
	if (fread(fi->memory, fi->total_size, 1, fi->file) != 1) {
		fprintf(stderr, "%s error reading file contents", fi->name);
		return false;
	}
	
	/* check for DOS header signature `MZ' */
	RETURN_IF_BAD_POINTER(fi, false, MZ_HEADER(fi->memory)->magic);
	if (MZ_HEADER(fi->memory)->magic == IMAGE_DOS_SIGNATURE) {
		DOSImageHeader *mz_header = MZ_HEADER(fi->memory);

		RETURN_IF_BAD_POINTER(fi, false, mz_header->lfanew);
		if (mz_header->lfanew < sizeof (DOSImageHeader)) {
			warn(_("%s: not a PE or NE library"), fi->name);
			return false;
		}

		/* falls through */
	}

	/* check for OS2 (Win16) header signature `NE' */
	RETURN_IF_BAD_POINTER(fi, false, NE_HEADER(fi->memory)->magic);
	if (NE_HEADER(fi->memory)->magic == IMAGE_OS2_SIGNATURE) {
		OS2ImageHeader *header = NE_HEADER(fi->memory);
		uint16_t *alignshift;

		RETURN_IF_BAD_POINTER(fi, false, header->rsrctab);
		RETURN_IF_BAD_POINTER(fi, false, header->restab);
		if (header->rsrctab >= header->restab) {
			warn(_("%s: no resource directory found"), fi->name);
			return false;
		}

		fi->binary_type = NE_BINARY;
		alignshift = (uint16_t *) ((uint8_t *) NE_HEADER(fi->memory) + header->rsrctab);
		fi->first_resource = ((uint8_t *) alignshift) + sizeof(uint16_t);
		RETURN_IF_BAD_POINTER(fi, false, *(Win16NETypeInfo *) fi->first_resource);

		return true;
	}

	/* check for NT header signature `PE' */
	RETURN_IF_BAD_POINTER(fi, false, PE_HEADER(fi->memory)->signature);
	if (PE_HEADER(fi->memory)->signature == IMAGE_NT_SIGNATURE) {
		Win32ImageSectionHeader *pe_sec;
		Win32ImageDataDirectory *dir;
		Win32ImageNTHeaders *pe_header;
		PE32plusImageNTHeaders *peplus_header;
		int d;

		/* allocate new memory */
		fi->total_size = calc_vma_size(fi);
		if (fi->total_size == 0) {
			/* calc_vma_size has reported error */
			return false;
		}
		fi->memory = xrealloc(fi->memory, fi->total_size);

		/* relocate memory, start from last section */
		pe_header = PE_HEADER(fi->memory);
		RETURN_IF_BAD_POINTER(fi, false, pe_header->file_header.number_of_sections);
		peplus_header = (PE32plusImageNTHeaders*)pe_header;
		
		/* we don't need to do OFFSET checking for the sections.
		 * calc_vma_size has already done that */
		for (d = pe_header->file_header.number_of_sections - 1; d >= 0 ; d--) {
			pe_sec = PE_SECTIONS(fi->memory) + d;
			if (pe_sec->characteristics & IMAGE_SCN_CNT_UNINITIALIZED_DATA) continue;
			//if (pe_sec->virtual_address + pe_sec->size_of_raw_data > fi->total_size)
			RETURN_IF_BAD_OFFSET(fi, 0, fi->memory + pe_sec->virtual_address, pe_sec->size_of_raw_data);
			RETURN_IF_BAD_OFFSET(fi, 0, fi->memory + pe_sec->pointer_to_raw_data, pe_sec->size_of_raw_data);
			if (pe_sec->virtual_address != pe_sec->pointer_to_raw_data) {
				memmove(fi->memory + pe_sec->virtual_address,
						fi->memory + pe_sec->pointer_to_raw_data,
						pe_sec->size_of_raw_data);
			}
		}

		if (pe_header->optional_header.magic == 0x20B) {  /* PE32+ */
			/* find resource directory */
			fi->binary_type = PEPLUS_BINARY;
			RETURN_IF_BAD_POINTER(fi, false, peplus_header->optional_header.data_directory[IMAGE_DIRECTORY_ENTRY_RESOURCE]);
			dir = peplus_header->optional_header.data_directory + IMAGE_DIRECTORY_ENTRY_RESOURCE;
			if (dir->size == 0) {
				warn(_("%s: file contains no resources"), fi->name);
				return false;
			}
		} else {  /* PE32 */
			/* find resource directory */
			fi->binary_type = PE_BINARY;
			RETURN_IF_BAD_POINTER(fi, false, pe_header->optional_header.data_directory[IMAGE_DIRECTORY_ENTRY_RESOURCE]);
			dir = pe_header->optional_header.data_directory + IMAGE_DIRECTORY_ENTRY_RESOURCE;
			if (dir->size == 0) {
				warn(_("%s: file contains no resources"), fi->name);
				return false;
			}
		}

		fi->first_resource = ((uint8_t *) fi->memory) + dir->virtual_address;
		return true;
	}

	/* other (unknown) header signature was found */
	warn(_("%s: not a PE or NE library"), fi->name);
	return false;
}

/* calc_vma_size:
 *   Calculate the total amount of memory needed for a 32-bit Windows
 *   module. Returns -1 if file was too small.
 */
static off_t
calc_vma_size (WinLibrary *fi)
{
    Win32ImageSectionHeader *seg;
    int c, segcount, size;

    size = 0;
    RETURN_IF_BAD_POINTER(fi, -1, PE_HEADER(fi->memory)->file_header.number_of_sections);
    segcount = PE_HEADER(fi->memory)->file_header.number_of_sections;

    /* If there are no segments, just process file like it is.
     * This is (probably) not the right thing to do, but problems
     * will be delt with later anyway.
     */
    if (segcount == 0)
    	return fi->total_size;

    seg = PE_SECTIONS(fi->memory);
    RETURN_IF_BAD_POINTER(fi, -1, *seg);
	
    for (c = 0 ; c < segcount ; c++) {
    	RETURN_IF_BAD_POINTER(fi, 0, *seg);

    	size = MAX(size, seg->virtual_address + seg->size_of_raw_data);
		/* Pecoff 8.2 pag 24:  VirtualSize is The total size of the section when
		 loaded into memory. If this value is greater than SizeOfRawData, the se-
		 ction is zero-padded. This field is valid only for executable images and
		 should be set to zero for object files. */
        size = MAX(size, seg->virtual_address + seg->misc.virtual_size);
        seg++;
    }

    return size;
}

