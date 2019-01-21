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
#include <sys/mman.h>
#include "gettext.h"			/* Gnulib */
#define _(s) gettext(s)
#define N_(s) gettext_noop(s)
#include "fileread.h"
#include "win32.h"
#include "xalloc.h"		/* Gnulib */
#include "minmax.h"		/* Gnulib */


static off_t calc_vma_size (WinLibrary *);
static wres_error load_ne_library(WinLibrary *);
static wres_error load_pe_library(WinLibrary *);


/* Check whether access to a PE_SECTIONS is allowed */
#define BAD_PE_SECTIONS(fi, module) ( 											 \
    BAD_POINTER(fi, PE_HEADER(module)->optional_header) || 						 \
    BAD_POINTER(fi, PE_HEADER(module)->file_header.number_of_sections) || 		 \
	BAD_OFFSET(fi, ((void*)PE_SECTIONS(module)), sizeof(Win32ImageSectionHeader) \
            * PE_HEADER(module)->file_header.number_of_sections))

#define RETURN_IF_BAD_PE_SECTIONS(fi, ret, module) do { \
    	if (BAD_PE_SECTIONS(fi, module)) { 			    \
    		return (ret); 								\
		} 												\
	} while(0)


/* check_offset:
 *   Check if a chunk of data (determined by offset and size)
 *   is within the bounds of the WinLibrary file.
 *   Usually not called directly.
 */
bool
check_offset(const char *memory, size_t total_size, const char *name, const void *offset, size_t size)
{
	const char* memory_end = memory + total_size;
	const char* block = (const char*) offset;
	const char* block_end = block + size;

	/*debug("check_offset: size=%x vs %x offset=%x size=%x\n",
		need_size, total_size, (char *) offset - memory, size);*/

	if (((memory > memory_end) || (block > block_end))
		|| (block < memory) || (block >= memory_end) || (block_end > memory_end))
		return false;

	return true;
}



/* load_library:
 *
 * Read header and get resource directory offset in a Windows library
 * (AKA module).
 */
wres_error load_library(WinLibrary *fi)
{
	fseek(fi->file, 0, SEEK_END);
	fi->total_size = ftello(fi->file);
	fseek(fi->file, 0, SEEK_SET);
	if (fi->total_size == -1)
		return -errno;
	if (fi->total_size == 0)
		return WRES_ERROR_WRONGFORMAT;
	
	/* read all of file */
	fi->memory = mmap(NULL, fi->total_size, PROT_READ, MAP_FILE | MAP_PRIVATE, fileno(fi->file), 0);
	if (fi->memory == MAP_FAILED)
		return -errno;
	
	/* check for DOS header signature `MZ' */
	RETURN_IF_BAD_POINTER(fi, WRES_ERROR_WRONGFORMAT, MZ_HEADER(fi->memory)->magic);
	if (MZ_HEADER(fi->memory)->magic == IMAGE_DOS_SIGNATURE) {
		RETURN_IF_BAD_POINTER(fi, WRES_ERROR_WRONGFORMAT, *MZ_HEADER(fi->memory));
		DOSImageHeader *mz_header = MZ_HEADER(fi->memory);

		RETURN_IF_BAD_POINTER(fi, WRES_ERROR_WRONGFORMAT, mz_header->lfanew);
		if (mz_header->lfanew < sizeof (DOSImageHeader))
			return WRES_ERROR_WRONGFORMAT;

		/* falls through */
	}

	/* check for OS2/Win16 header signature `NE' */
	RETURN_IF_BAD_POINTER(fi, WRES_ERROR_WRONGFORMAT, NE_HEADER(fi->memory)->magic);
	if (NE_HEADER(fi->memory)->magic == IMAGE_OS2_SIGNATURE) {
		RETURN_IF_BAD_POINTER(fi, WRES_ERROR_WRONGFORMAT, *NE_HEADER(fi->memory));
		return load_ne_library(fi);
	}

	/* check for NT header signature `PE' */
	RETURN_IF_BAD_POINTER(fi, WRES_ERROR_WRONGFORMAT, PE_HEADER(fi->memory)->signature);
	if (PE_HEADER(fi->memory)->signature == IMAGE_NT_SIGNATURE) {
		RETURN_IF_BAD_POINTER(fi, WRES_ERROR_WRONGFORMAT, *PE_HEADER(fi->memory));
		return load_pe_library(fi);
	}

	/* other (unknown) header signature was found */
	return WRES_ERROR_WRONGFORMAT;
}


/* calc_vma_size:
 *   Calculate the total amount of memory needed for a 32-bit Windows
 *   module. Returns -1 if file was too small.
 */
static off_t
calc_vma_size (WinLibrary *fi)
{
    Win32ImageSectionHeader *seg;
    size_t c, segcount, size;

    size = 0;
    RETURN_IF_BAD_POINTER(fi, -1, PE_HEADER(fi->memory)->file_header.number_of_sections);
    segcount = PE_HEADER(fi->memory)->file_header.number_of_sections;

    /* If there are no segments, just process file like it is.
     * This is (probably) not the right thing to do, but problems
     * will be delt with later anyway.
     */
    if (segcount == 0)
    	return fi->total_size;

	RETURN_IF_BAD_PE_SECTIONS(fi, -1, fi->memory);
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


static wres_error load_ne_library(WinLibrary *fi)
{
	OS2ImageHeader *header = NE_HEADER(fi->memory);
	uint16_t *alignshift;

	RETURN_IF_BAD_POINTER(fi, WRES_ERROR_PREMATUREEND, header->rsrctab);
	RETURN_IF_BAD_POINTER(fi, WRES_ERROR_PREMATUREEND, header->restab);
	fi->binary_type = NE_BINARY;
	
	if (!(header->rsrctab >= header->restab)) {
		alignshift = (uint16_t *) ((uint8_t *) NE_HEADER(fi->memory) + header->rsrctab);
		fi->first_resource = ((uint8_t *) alignshift) + sizeof(uint16_t);
		RETURN_IF_BAD_POINTER(fi, WRES_ERROR_PREMATUREEND, *(Win16NETypeInfo *) fi->first_resource);
	} else {
		/* no resources */
		fi->first_resource = NULL;
	}

	return WRES_ERROR_NONE;
}


static wres_error load_pe_library(WinLibrary *fi)
{
	WinLibrary fi_new = *fi;
	
	/* allocate new memory */
	fi_new.total_size = calc_vma_size(fi);
	if (fi_new.total_size <= 0) /* calc_vma_size has reported error */
		return WRES_ERROR_WRONGFORMAT;

	fi_new.memory = mmap(NULL, fi_new.total_size, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
	if (fi_new.memory == MAP_FAILED)
		return -errno;

	wres_error err = WRES_ERROR_PREMATUREEND;
	
	/* relocate memory, start from last section */
	Win32ImageNTHeaders *pe_header = PE_HEADER(fi->memory);
	if (BAD_PE_SECTIONS(fi, fi->memory))
		goto fail;
	PE32plusImageNTHeaders *peplus_header = (PE32plusImageNTHeaders*)pe_header;
	IF_BAD_POINTER(fi, pe_header->optional_header.magic)
		goto fail;
	
	Win32ImageDataDirectory *dir;
	if (pe_header->optional_header.magic == OPTIONAL_MAGIC_PE32_64) {  /* PE32+ */
		/* find resource directory */
		fi_new.binary_type = PEPLUS_BINARY;
		IF_BAD_POINTER(fi, peplus_header->optional_header.data_directory[IMAGE_DIRECTORY_ENTRY_RESOURCE])
			goto fail;
		
		dir = peplus_header->optional_header.data_directory + IMAGE_DIRECTORY_ENTRY_RESOURCE;
	} else if (pe_header->optional_header.magic == OPTIONAL_MAGIC_PE32) {  /* PE32 */
		/* find resource directory */
		fi_new.binary_type = PE_BINARY;
		IF_BAD_POINTER(fi, pe_header->optional_header.data_directory[IMAGE_DIRECTORY_ENTRY_RESOURCE])
			goto fail;
		
		dir = pe_header->optional_header.data_directory + IMAGE_DIRECTORY_ENTRY_RESOURCE;
	} else {
		err = WRES_ERROR_WRONGFORMAT;
		goto fail;
	}
	
	if (dir->size > 0) {
		Win32ImageSectionHeader *pe_sections = PE_SECTIONS(fi->memory);
		/* we don't need to do OFFSET checking for the sections.
		 * calc_vma_size has already done that */
		for (int d = pe_header->file_header.number_of_sections - 1; d >= 0 ; d--) {
			Win32ImageSectionHeader *pe_sec = pe_sections + d;
			
			void *dest = fi_new.memory + pe_sec->virtual_address;
			off_t size = pe_sec->size_of_raw_data;
			off_t offset = pe_sec->pointer_to_raw_data;
			
			if (pe_sec->characteristics & IMAGE_SCN_CNT_UNINITIALIZED_DATA)
				continue;
			
			/* Protect against memory moves overwriting the section table */
            if ((uint8_t*)(fi->memory + pe_sec->virtual_address) <
            	(uint8_t*)(pe_sections + pe_header->file_header.number_of_sections)) {
                err = WRES_ERROR_INVALIDSECLAYOUT;
                goto fail;
            }
			
			/* do not load sections we are not interested in */
			if ((pe_sec->virtual_address < dir->virtual_address &&
				pe_sec->virtual_address + pe_sec->size_of_raw_data <= dir->virtual_address) ||
				(pe_sec->virtual_address >= dir->virtual_address + dir->size))
				continue;
			
			IF_BAD_OFFSET(&fi_new, dest, size)
				goto fail;
			IF_BAD_OFFSET(fi, fi->memory + offset, size)
				goto fail;
			
			void *res = mmap(dest, size,
				PROT_READ, MAP_FILE | MAP_FIXED | MAP_PRIVATE,
				fileno(fi_new.file), offset);
			if (res == MAP_FAILED) {
				/* As in PE files there is no requirement for sections in the file
				 * to be aligned in the same way as they are required to be aligned
				 * in memory, this code path will be hit very frequently */
				memcpy(dest, fi->memory + offset, size);
			}
		}

		fi_new.first_resource = ((uint8_t *)fi_new.memory) + dir->virtual_address;
	} else {
		/* no resources */
		fi_new.first_resource = NULL;
	}
	
	munmap(fi->memory, fi->total_size);
	*fi = fi_new;
	return WRES_ERROR_NONE;
	
fail:
	munmap(fi_new.memory, fi_new.total_size);
	return err;
}


void unload_library(WinLibrary *fi)
{
	munmap(fi->memory, fi->total_size);
}



