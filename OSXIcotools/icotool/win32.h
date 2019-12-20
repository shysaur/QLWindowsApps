/* win32.h - Common definitions for win32 objects
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

#ifndef WIN32_H
#define WIN32_H

#include <stdint.h>	/* POSIX/Gnulib */

#define PACKED __attribute__ ((packed))
#pragma pack(1)

typedef struct {
    uint8_t width;
    uint8_t height;
    uint8_t color_count;
    uint8_t reserved;
} Win32IconResDir;

typedef struct {
    uint16_t width;
    uint16_t height;
} Win32CursorDir;

typedef struct PACKED {
    union {
		Win32IconResDir icon;
		Win32CursorDir cursor;
    } res_info;
    uint16_t plane_count;
    uint16_t bit_count;
    uint32_t bytes_in_res;
    uint16_t res_id;
} Win32CursorIconDirEntry;

typedef struct PACKED {
    uint16_t reserved;
    uint16_t type;
    uint16_t count;
    Win32CursorIconDirEntry entries[0];
} Win32CursorIconDir;

typedef struct {
    uint8_t width;
    uint8_t height;
    uint8_t color_count;
    uint8_t reserved;
    uint16_t hotspot_x;		/* hotspot_x for cursors, planes for icons */
    uint16_t hotspot_y;		/* hotspot_y for cursors, bit_count for icons */
    uint32_t dib_size;
    uint32_t dib_offset;
} Win32CursorIconFileDirEntry;

typedef struct {
    uint16_t reserved;
    uint16_t type;
    uint16_t count;
    Win32CursorIconFileDirEntry entries[0];
} Win32CursorIconFileDir;

typedef struct {
    uint32_t size;
    int32_t width;
    int32_t height;
    uint16_t planes;
    uint16_t bit_count;
    uint32_t compression;
    uint32_t size_image;
    int32_t x_pels_per_meter;
    int32_t y_pels_per_meter;
    uint32_t clr_used;
    uint32_t clr_important;
} Win32BitmapInfoHeader;

/* compression */
#define BI_RGB           0
#define BI_RLE8          1
#define BI_RLE4          2
#define BI_BITFIELDS     3
#define BI_JPEG          4
#define BI_PNG           5

typedef struct {
    uint8_t blue;
    uint8_t green;
    uint8_t red;
    uint8_t reserved;
} Win32RGBQuad;

#define IMAGE_NUMBEROF_DIRECTORY_ENTRIES 16
#define IMAGE_SIZEOF_SHORT_NAME 8

#define	IMAGE_RESOURCE_NAME_IS_STRING		0x80000000
#define	IMAGE_RESOURCE_DATA_IS_DIRECTORY	0x80000000

#define PE_HEADER(module) \
    ((Win32ImageNTHeaders*)((uint8_t *)(module) + \
    	(((DOSImageHeader*)(module))->lfanew)))

#define MZ_HEADER(x)	((DOSImageHeader *)(x))
#define NE_HEADER(x)	((OS2ImageHeader *)PE_HEADER(x))

#define PE_SECTIONS(module) \
    ((Win32ImageSectionHeader *)((uint8_t *) &PE_HEADER(module)->optional_header + \
                           PE_HEADER(module)->file_header.size_of_optional_header))

#define IMAGE_DOS_SIGNATURE    0x5A4D     /* MZ */
#define IMAGE_OS2_SIGNATURE    0x454E     /* NE */
#define IMAGE_OS2_SIGNATURE_LE 0x454C     /* LE */
#define IMAGE_OS2_SIGNATURE_LX 0x584C     /* LX */
#define IMAGE_VXD_SIGNATURE    0x454C     /* LE */
#define IMAGE_NT_SIGNATURE     0x00004550 /* PE00 */

#define OPTIONAL_MAGIC_PE32    0x010b
#define OPTIONAL_MAGIC_PE32_64 0x020b

#define IMAGE_SCN_CNT_CODE			0x00000020
#define IMAGE_SCN_CNT_INITIALIZED_DATA		0x00000040
#define IMAGE_SCN_CNT_UNINITIALIZED_DATA	0x00000080

#define	IMAGE_DIRECTORY_ENTRY_EXPORT		0
#define	IMAGE_DIRECTORY_ENTRY_IMPORT		1
#define	IMAGE_DIRECTORY_ENTRY_RESOURCE		2
#define	IMAGE_DIRECTORY_ENTRY_EXCEPTION		3
#define	IMAGE_DIRECTORY_ENTRY_SECURITY		4
#define	IMAGE_DIRECTORY_ENTRY_BASERELOC		5
#define	IMAGE_DIRECTORY_ENTRY_DEBUG		6
#define	IMAGE_DIRECTORY_ENTRY_COPYRIGHT		7
#define	IMAGE_DIRECTORY_ENTRY_GLOBALPTR		8   /* (MIPS GP) */
#define	IMAGE_DIRECTORY_ENTRY_TLS		9
#define	IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG	10
#define	IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT	11
#define	IMAGE_DIRECTORY_ENTRY_IAT		12  /* Import Address Table */
#define	IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT	13
#define	IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR	14

#define RT_CURSOR        1
#define RT_BITMAP        2
#define RT_ICON          3
#define RT_MENU          4
#define RT_DIALOG        5
#define RT_STRING        6
#define RT_FONTDIR       7
#define RT_FONT          8
#define RT_ACCELERATOR   9
#define RT_RCDATA        10
#define RT_MESSAGELIST   11
#define RT_GROUP_CURSOR  12
#define RT_GROUP_ICON    14
#define RT_VERSION       16

typedef struct {
    union {
    	struct {
    	    #if BITFIELDS_BIGENDIAN
    	    unsigned name_is_string:1;
    	    unsigned name_offset:31;
    	    #else
    	    unsigned name_offset:31;
    	    unsigned name_is_string:1;
    	    #endif
    	} s1;
    	uint32_t name;
    	struct {
    	    #if WORDS_BIGENDIAN
    	    uint16_t __pad;
    	    uint16_t id;
    	    #else
    	    uint16_t id;
    	    uint16_t __pad;
    	    #endif
    	} s2;
    } u1;
    union {
    	uint32_t offset_to_data;
    	struct {
    	    #if BITFIELDS_BIGENDIAN
    	    unsigned data_is_directory:1;
    	    unsigned offset_to_directory:31;
    	    #else
    	    unsigned offset_to_directory:31;
    	    unsigned data_is_directory:1;
    	    #endif
    	} s;
    } u2;
} Win32ImageResourceDirectoryEntry;

typedef struct {
    uint16_t type_id;
    uint16_t count;
    uint32_t reserved;
} Win16NETypeInfo;

typedef struct {
    uint16_t offset;
    uint16_t length;
    uint16_t flags;
    uint16_t id;
    uint16_t handle;
    uint16_t usage;
} Win16NENameInfo;

typedef struct {
    uint16_t magic;
    uint8_t ver;
    uint8_t rev;
    uint16_t enttab;
    uint16_t cbenttab;
    int32_t crc;
    uint16_t flags;
    uint16_t autodata;
    uint16_t heap;
    uint16_t stack;
    uint32_t csip;
    uint32_t sssp;
    uint16_t cseg;
    uint16_t cmod;
    uint16_t cbnrestab;
    uint16_t segtab;
    uint16_t rsrctab;
    uint16_t restab;
    uint16_t modtab;
    uint16_t imptab;
    uint32_t nrestab;
    uint16_t cmovent;
    uint16_t align;
    uint16_t cres;
    uint8_t exetyp;
    uint8_t flagsothers;
    uint16_t fastload_offset;
    uint16_t fastload_length;
    uint16_t swaparea;
    uint16_t expver;
} OS2ImageHeader;

typedef struct {
    uint16_t magic;
    uint16_t cblp;
    uint16_t cp;
    uint16_t crlc;
    uint16_t cparhdr;
    uint16_t minalloc;
    uint16_t maxalloc;
    uint16_t ss;
    uint16_t sp;
    uint16_t csum;
    uint16_t ip;
    uint16_t cs;
    uint16_t lfarlc;
    uint16_t ovno;
    uint16_t res[4];
    uint16_t oemid;
    uint16_t oeminfo;
    uint16_t res2[10];
    uint32_t lfanew;
} DOSImageHeader;

typedef struct {
    uint16_t machine;
    uint16_t number_of_sections;
    uint32_t time_date_stamp;
    uint32_t pointer_to_symbol_table;
    uint32_t number_of_symbols;
    uint16_t size_of_optional_header;
    uint16_t characteristics;
} Win32ImageFileHeader;

typedef struct {
    uint32_t virtual_address;
    uint32_t size;
} Win32ImageDataDirectory;

typedef struct {
    uint16_t magic;
    uint8_t major_linker_version;
    uint8_t minor_linker_version;
    uint32_t size_of_code;
    uint32_t size_of_initialized_data;
    uint32_t size_of_uninitialized_data;
    uint32_t address_of_entry_point;
    uint32_t base_of_code;
    uint32_t base_of_data;
    uint32_t image_base;
    uint32_t section_alignment;
    uint32_t file_alignment;
    uint16_t  major_operating_system_version;
    uint16_t  minor_operating_system_version;
    uint16_t  major_image_version;
    uint16_t  minor_image_version;
    uint16_t  major_subsystem_version;
    uint16_t  minor_subsystem_version;
    uint32_t win32_version_value;
    uint32_t size_of_image;
    uint32_t size_of_headers;
    uint32_t checksum;
    uint16_t subsystem;
    uint16_t dll_characteristics;
    uint32_t size_of_stack_reserve;
    uint32_t size_of_stack_commit;
    uint32_t size_of_heap_reserve;
    uint32_t size_of_heap_commit;
    uint32_t loader_flags;
    uint32_t number_of_rva_and_sizes;
    Win32ImageDataDirectory data_directory[IMAGE_NUMBEROF_DIRECTORY_ENTRIES];
} Win32ImageOptionalHeader;

typedef struct {
    uint16_t magic;
    uint8_t major_linker_version;
    uint8_t minor_linker_version;
    uint32_t size_of_code;
    uint32_t size_of_initialized_data;
    uint32_t size_of_uninitialized_data;
    uint32_t address_of_entry_point;
    uint32_t base_of_code;
    uint64_t image_base;
    uint32_t section_alignment;
    uint32_t file_alignment;
    uint16_t  major_operating_system_version;
    uint16_t  minor_operating_system_version;
    uint16_t  major_image_version;
    uint16_t  minor_image_version;
    uint16_t  major_subsystem_version;
    uint16_t  minor_subsystem_version;
    uint32_t win32_version_value;
    uint32_t size_of_image;
    uint32_t size_of_headers;
    uint32_t checksum;
    uint16_t subsystem;
    uint16_t dll_characteristics;
    uint64_t size_of_stack_reserve;
    uint64_t size_of_stack_commit;
    uint64_t size_of_heap_reserve;
    uint64_t size_of_heap_commit;
    uint32_t loader_flags;
    uint32_t number_of_rva_and_sizes;
    Win32ImageDataDirectory data_directory[IMAGE_NUMBEROF_DIRECTORY_ENTRIES];
} PE32plusImageOptionalHeader;

typedef struct {
    uint32_t signature;
    Win32ImageFileHeader file_header;
    Win32ImageOptionalHeader optional_header;
} Win32ImageNTHeaders;

typedef struct {
    uint32_t signature;
    Win32ImageFileHeader file_header;
    PE32plusImageOptionalHeader optional_header;
} PE32plusImageNTHeaders;

typedef struct  {
    uint8_t name[IMAGE_SIZEOF_SHORT_NAME];
    union {
	uint32_t physical_address;
	uint32_t virtual_size;
    } misc;
    uint32_t virtual_address;
    uint32_t size_of_raw_data;
    uint32_t pointer_to_raw_data;
    uint32_t pointer_to_relocations;
    uint32_t pointer_to_linenumbers;
    uint16_t number_of_relocations;
    uint16_t number_of_linenumbers;
    uint32_t characteristics;
} Win32ImageSectionHeader;

typedef struct {
    uint32_t offset_to_data;
    uint32_t size;
    uint32_t code_page;
    uint32_t resource_handle;
} Win32ImageResourceDataEntry;

typedef struct {
    uint32_t characteristics;
    uint32_t time_date_stamp;
    uint16_t major_version;
    uint16_t minor_version;
    uint16_t number_of_named_entries;
    uint16_t number_of_id_entries;
} Win32ImageResourceDirectory;

#pragma pack()

#endif /* WIN32_H */
