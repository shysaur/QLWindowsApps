/* restable.c - Decoding PE and NE resource tables
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

#include "restable.h"
#include <config.h>
#include <inttypes.h>		/* ? */
#include "gettext.h"		/* Gnulib */
#define _(s) gettext(s)
#define N_(s) gettext_noop(s)
#include "common/intutil.h"
#include "xalloc.h"		/* Gnulib */
#include "minmax.h"		/* Gnulib */
#include "common/error.h"
#include "wrestool.h"
#include "win32.h"
#include "fileread.h"
#include "restypes.h"
#include "common/common.h"

static bool decode_pe_resource_id (WinLibrary *, WinResource *, uint32_t);
static bool decode_ne_resource_id (WinLibrary *, WinResource *, uint16_t);
static WinResource *list_ne_type_resources (WinLibrary *, int *);
static WinResource *list_ne_name_resources (WinLibrary *, WinResource *, int *);
static WinResource *list_pe_resources (WinLibrary *, Win32ImageResourceDirectory *, int, int *);
static int calc_vma_size (WinLibrary *);
static void do_resources_recurs (WinLibrary *, WinResource *, WinResource *, WinResource *, WinResource *, char *, char *, char *, DoResourceCallback);
static char *get_resource_id_quoted (WinResource *);
static WinResource *find_with_resource_array(WinLibrary *, WinResource *, char *);
//static WinResource *list_resources (WinLibrary *fi, WinResource *res, int *count);
//static bool compare_resource_id (WinResource *wr, char *id);

/* do_resources:
 *   Do something for each resource matching type, name and lang.
 */

void
do_resources (WinLibrary *fi, char *type, char *name, char *lang, DoResourceCallback cb)
{
	WinResource *type_wr;
	WinResource *name_wr;
	WinResource *lang_wr;

	type_wr = malloc(sizeof(WinResource)*3);
	name_wr = type_wr + 1;
	lang_wr = type_wr + 2;
	memset(type_wr, 0, sizeof(WinResource)*3);

	do_resources_recurs(fi, NULL, type_wr, name_wr, lang_wr, type, name, lang, cb);

	free(type_wr);
}

static void
do_resources_recurs (WinLibrary *fi, WinResource *base, WinResource *type_wr,
                          WinResource *name_wr, WinResource *lang_wr,
						  char *type, char *name, char *lang, DoResourceCallback cb)
{
	int c, rescnt;
	WinResource *wr;

	/* get a list of all resources at this level */
	wr = list_resources (fi, base, &rescnt);
	if (wr == NULL)
		return;

	/* process each resource listed */
	for (c = 0 ; c < rescnt ; c++) {
		/* (over)write the corresponding WinResource holder with the current */
		memcpy(WINRESOURCE_BY_LEVEL(wr[c].level), wr+c, sizeof(WinResource));

		/* go deeper unless there is something that does NOT match */
		if (LEVEL_MATCHES(type) && LEVEL_MATCHES(name) && LEVEL_MATCHES(lang)) {
			if (wr->is_directory)
				do_resources_recurs (fi, wr+c, type_wr, name_wr, lang_wr, type, name, lang, cb);
			else
				cb(fi, wr+c, type_wr, name_wr, lang_wr);
		}
	}

	/* since we're moving back one level after this, unset the
	 * WinResource holder used on this level */
	memset(WINRESOURCE_BY_LEVEL(wr[0].level), 0, sizeof(WinResource));
}



void
print_resources_callback (WinLibrary *fi, WinResource *wr,
                          WinResource *type_wr, WinResource *name_wr,
						  WinResource *lang_wr)
{
	char *type, *offset;
	int32_t id, size;

	/* get named resource type if possible */
	type = NULL;
	if (parse_int32(type_wr->id, &id))
		type = res_type_id_to_string(id);

	/* get offset and size info on resource */
	offset = get_resource_entry(fi, wr, &size);
	if (offset == NULL)
		return;

	printf(_("--type=%s --name=%s%s%s [%s%s%soffset=0x%x size=%d]\n"),
	  get_resource_id_quoted(type_wr),
	  get_resource_id_quoted(name_wr),
	  (lang_wr->id[0] != '\0' ? _(" --language=") : ""),
	  get_resource_id_quoted(lang_wr),
	  (type != NULL ? "type=" : ""),
	  (type != NULL ? type : ""),
	  (type != NULL ? " " : ""),
	  (uint32_t) (offset - fi->memory), size);
}

/* return the resource id quoted if it's a string, otherwise just return it */
static char *
get_resource_id_quoted (WinResource *wr)
{
	static char tmp[WINRES_ID_MAXLEN+2];

	if (wr->numeric_id || wr->id[0] == '\0')
		return wr->id;

	sprintf(tmp, "'%s'", wr->id);
	return tmp;
}

/*static*/ bool
compare_resource_id (WinResource *wr, char *id)
{
  if (*id == 0) return true; //Empty string is wildcard for disabling comparison
  
	if (wr->numeric_id) {
		int32_t cmp1, cmp2;
		if (id[0] == '+')
			return false;
		if (id[0] == '-')
			id++;
		if (!parse_int32(wr->id, &cmp1) || !parse_int32(id, &cmp2) || cmp1 != cmp2)
			return false;
	} else {
		if (id[0] == '-')
			return false;
		if (id[0] == '+')
			id++;
		if (strcmp(wr->id, id))
			return false;
	}

	return true;
}

static bool
decode_pe_resource_id (WinLibrary *fi, WinResource *wr, uint32_t value)
{
	if (value & IMAGE_RESOURCE_NAME_IS_STRING) {	/* numeric id */
		int c, len;
		uint16_t *mem = (uint16_t *)
		  (fi->first_resource + (value & ~IMAGE_RESOURCE_NAME_IS_STRING));

		/* copy each char of the string, and terminate it */
		RETURN_IF_BAD_POINTER(false, *mem);
		len = mem[0];
		RETURN_IF_BAD_OFFSET(false, &mem[1], sizeof(uint16_t) * len);

		len = MIN(mem[0], WINRES_ID_MAXLEN);
		for (c = 0 ; c < len ; c++)
			wr->id[c] = mem[c+1] & 0x00FF;
		wr->id[len] = '\0';
	} else {					/* Unicode string id */
		/* translate id into a string */
		snprintf(wr->id, WINRES_ID_MAXLEN, "%d", value);
	}

	wr->numeric_id = (value & IMAGE_RESOURCE_NAME_IS_STRING ? false:true);
	return true;
}
 
void *
get_resource_entry (WinLibrary *fi, WinResource *wr, int *size)
{
	if (fi->is_PE_binary) {
		Win32ImageResourceDataEntry *dataent;

		dataent = (Win32ImageResourceDataEntry *) wr->children;
		RETURN_IF_BAD_POINTER(NULL, *dataent);
		*size = dataent->size;
		RETURN_IF_BAD_OFFSET(NULL, fi->memory + dataent->offset_to_data, *size);

		return fi->memory + dataent->offset_to_data;
	} else {
		Win16NENameInfo *nameinfo;
		int sizeshift;

		nameinfo = (Win16NENameInfo *) wr->children;
		sizeshift = *((uint16_t *) fi->first_resource - 1);
		*size = nameinfo->length << sizeshift;
		RETURN_IF_BAD_OFFSET(NULL, fi->memory + (nameinfo->offset << sizeshift), *size);

		return fi->memory + (nameinfo->offset << sizeshift);
	}
}

static bool
decode_ne_resource_id (WinLibrary *fi, WinResource *wr, uint16_t value)
{
	if (value & NE_RESOURCE_NAME_IS_NUMERIC) {		/* numeric id */
		/* translate id into a string */
		snprintf(wr->id, WINRES_ID_MAXLEN, "%d", value & ~NE_RESOURCE_NAME_IS_NUMERIC);
	} else {					/* ASCII string id */
		int len;
		char *mem = (char *) NE_HEADER(fi->memory)
		                     + NE_HEADER(fi->memory)->rsrctab
		                     + value;

		/* copy each char of the string, and terminate it */
		RETURN_IF_BAD_POINTER(false, *mem);
		len = mem[0];
		RETURN_IF_BAD_OFFSET(false, &mem[1], sizeof(char) * len);
		memcpy(wr->id, &mem[1], len);
		wr->id[len] = '\0';
	}

	wr->numeric_id = (value & NE_RESOURCE_NAME_IS_NUMERIC ? true:false);
	return true;
}

static WinResource *
list_pe_resources (WinLibrary *fi, Win32ImageResourceDirectory *pe_res, int level, int *count)
{
	WinResource *wr;
	int c, rescnt;
	Win32ImageResourceDirectoryEntry *dirent
	  = (Win32ImageResourceDirectoryEntry *) (pe_res + 1);

	/* count number of `type' resources */
	RETURN_IF_BAD_POINTER(NULL, *dirent);
	rescnt = pe_res->number_of_named_entries + pe_res->number_of_id_entries;
	*count = rescnt;

	/* allocate WinResource's */
	wr = xmalloc(sizeof(WinResource) * rescnt);

	/* fill in the WinResource's */
	for (c = 0 ; c < rescnt ; c++) {
		RETURN_IF_BAD_POINTER(NULL, dirent[c]);
		wr[c].this = pe_res;
		wr[c].level = level;
		wr[c].is_directory = (dirent[c].u2.s.data_is_directory);
		wr[c].children = fi->first_resource + dirent[c].u2.s.offset_to_directory;

		/* fill in wr->id, wr->numeric_id */
		if (!decode_pe_resource_id (fi, wr + c, dirent[c].u1.name))
			return NULL;
	}

	return wr;
}

static WinResource *
list_ne_name_resources (WinLibrary *fi, WinResource *typeres, int *count)
{
	int c, rescnt;
	WinResource *wr;
	Win16NETypeInfo *typeinfo = (Win16NETypeInfo *) typeres->this;
	Win16NENameInfo *nameinfo = (Win16NENameInfo *) typeres->children;

	/* count number of `type' resources */
	RETURN_IF_BAD_POINTER(NULL, typeinfo->count);
	*count = rescnt = typeinfo->count;

	/* allocate WinResource's */
	wr = xmalloc(sizeof(WinResource) * rescnt);

	/* fill in the WinResource's */
	for (c = 0 ; c < rescnt ; c++) {
		RETURN_IF_BAD_POINTER(NULL, nameinfo[c]);
		wr[c].this = nameinfo+c;
		wr[c].is_directory = false;
		wr[c].children = nameinfo+c;
		wr[c].level = 1;

		/* fill in wr->id, wr->numeric_id */
		if (!decode_ne_resource_id (fi, wr + c, (nameinfo+c)->id))
			return NULL;
	}

	return wr;
}

static WinResource *
list_ne_type_resources (WinLibrary *fi, int *count)
{
	int c, rescnt;
	WinResource *wr;
	Win16NETypeInfo *typeinfo;

	/* count number of `type' resources */
	typeinfo = (Win16NETypeInfo *) fi->first_resource;
	RETURN_IF_BAD_POINTER(NULL, *typeinfo);
	for (rescnt = 0 ; typeinfo->type_id != 0 ; rescnt++) {
		if (((char *) NE_TYPEINFO_NEXT(typeinfo))+sizeof(uint16_t) > fi->memory + fi->total_size) {
		    warn(_("%s: resource table invalid, ignoring remaining entries"), fi->name);
		    break;
		}
		typeinfo = NE_TYPEINFO_NEXT(typeinfo);
		RETURN_IF_BAD_POINTER(NULL, *typeinfo);
	}
	*count = rescnt;

	/* allocate WinResource's */
	wr = xmalloc(sizeof(WinResource) * rescnt);

	/* fill in the WinResource's */
	typeinfo = (Win16NETypeInfo *) fi->first_resource;
	for (c = 0 ; c < rescnt ; c++) {
		wr[c].this = typeinfo;
		wr[c].is_directory = (typeinfo->count != 0);
		wr[c].children = typeinfo+1;
		wr[c].level = 0;

		/* fill in wr->id, wr->numeric_id */
		if (!decode_ne_resource_id (fi, wr + c, typeinfo->type_id))
			return NULL;

		typeinfo = NE_TYPEINFO_NEXT(typeinfo);
	}

	return wr;
}

/* list_resources:
 *   Return an array of WinResource's in the current
 *   resource level specified by res.
 */
/*static*/ WinResource *
list_resources (WinLibrary *fi, WinResource *res, int *count)
{
	if (res != NULL && !res->is_directory)
		return NULL;

	if (fi->is_PE_binary) {
		return list_pe_resources(fi, (Win32ImageResourceDirectory *)
				 (res == NULL ? fi->first_resource : res->children),
				 (res == NULL ? 0 : res->level+1),
				 count);
	} else {
		return (res == NULL
				? list_ne_type_resources(fi, count)
				: list_ne_name_resources(fi, res, count));
	}
}

/* read_library:
 *
 * Read header and get resource directory offset in a Windows library
 * (AKA module).
 */
bool
read_library (WinLibrary *fi)
{
	/* check for DOS header signature `MZ' */
	RETURN_IF_BAD_POINTER(false, MZ_HEADER(fi->memory)->magic);
	if (MZ_HEADER(fi->memory)->magic == IMAGE_DOS_SIGNATURE) {
		DOSImageHeader *mz_header = MZ_HEADER(fi->memory);

		RETURN_IF_BAD_POINTER(false, mz_header->lfanew);
		if (mz_header->lfanew < sizeof (DOSImageHeader)) {
			warn(_("%s: not a PE or NE library"), fi->name);
			return false;
		}

		/* falls through */
	}

	/* check for OS2 (Win16) header signature `NE' */
	RETURN_IF_BAD_POINTER(false, NE_HEADER(fi->memory)->magic);
	if (NE_HEADER(fi->memory)->magic == IMAGE_OS2_SIGNATURE) {
		OS2ImageHeader *header = NE_HEADER(fi->memory);
		uint16_t *alignshift;

		RETURN_IF_BAD_POINTER(false, header->rsrctab);
		RETURN_IF_BAD_POINTER(false, header->restab);
		if (header->rsrctab >= header->restab) {
			warn(_("%s: no resource directory found"), fi->name);
			return false;
		}

		fi->is_PE_binary = false;
		alignshift = (uint16_t *) ((uint8_t *) NE_HEADER(fi->memory) + header->rsrctab);
		fi->first_resource = ((uint8_t *) alignshift) + sizeof(uint16_t);
		RETURN_IF_BAD_POINTER(false, *(Win16NETypeInfo *) fi->first_resource);

		return true;
	}

	/* check for NT header signature `PE' */
	RETURN_IF_BAD_POINTER(false, PE_HEADER(fi->memory)->signature);
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
		RETURN_IF_BAD_POINTER(false, pe_header->file_header.number_of_sections);
    peplus_header = (PE32plusImageNTHeaders*)pe_header;
    
    /* we don't need to do OFFSET checking for the sections.
     * calc_vma_size has already done that */
    for (d = pe_header->file_header.number_of_sections - 1; d >= 0 ; d--) {
      pe_sec = PE_SECTIONS(fi->memory) + d;
      if (pe_sec->characteristics & IMAGE_SCN_CNT_UNINITIALIZED_DATA) continue;
      //if (pe_sec->virtual_address + pe_sec->size_of_raw_data > fi->total_size)
      RETURN_IF_BAD_OFFSET(0, fi->memory + pe_sec->virtual_address, pe_sec->size_of_raw_data);
      RETURN_IF_BAD_OFFSET(0, fi->memory + pe_sec->pointer_to_raw_data, pe_sec->size_of_raw_data);
      if (pe_sec->virtual_address != pe_sec->pointer_to_raw_data) {
        memmove(fi->memory + pe_sec->virtual_address,
            fi->memory + pe_sec->pointer_to_raw_data,
            pe_sec->size_of_raw_data);
      }
    }

    if (pe_header->optional_header.magic == 0x20B) {  //PE32+
      /* find resource directory */
      RETURN_IF_BAD_POINTER(false, peplus_header->optional_header.data_directory[IMAGE_DIRECTORY_ENTRY_RESOURCE]);
      dir = peplus_header->optional_header.data_directory + IMAGE_DIRECTORY_ENTRY_RESOURCE;
      if (dir->size == 0) {
        warn(_("%s: file contains no resources"), fi->name);
        return false;
      }
    } else {  //PE32
      /* find resource directory */
      RETURN_IF_BAD_POINTER(false, pe_header->optional_header.data_directory[IMAGE_DIRECTORY_ENTRY_RESOURCE]);
      dir = pe_header->optional_header.data_directory + IMAGE_DIRECTORY_ENTRY_RESOURCE;
      if (dir->size == 0) {
        warn(_("%s: file contains no resources"), fi->name);
        return false;
      }
    }

		fi->first_resource = ((uint8_t *) fi->memory) + dir->virtual_address;
		fi->is_PE_binary = true;
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
static int
calc_vma_size (WinLibrary *fi)
{
    Win32ImageSectionHeader *seg;
    int c, segcount, size;

    size = 0;
    RETURN_IF_BAD_POINTER(-1, PE_HEADER(fi->memory)->file_header.number_of_sections);
    segcount = PE_HEADER(fi->memory)->file_header.number_of_sections;

    /* If there are no segments, just process file like it is.
     * This is (probably) not the right thing to do, but problems
     * will be delt with later anyway.
     */
    if (segcount == 0)
    	return fi->total_size;

    seg = PE_SECTIONS(fi->memory);
    RETURN_IF_BAD_POINTER(-1, *seg);
    
    for (c = 0 ; c < segcount ; c++) {
    	RETURN_IF_BAD_POINTER(0, *seg);

    	size = MAX(size, seg->virtual_address + seg->size_of_raw_data);
		/* I have no idea what misc.virtual_size is for... */
    /* Pecoff 8.2 pag 24:  VirtualSize is The total size of the section when
    loaded into memory. If this value is greater than SizeOfRawData, the se-
    ction is zero-padded. This field is valid only for executable images and
    should be set to zero for object files. */
        size = MAX(size, seg->virtual_address + seg->misc.virtual_size);
        seg++;
    }

    return size;
}

static WinResource *
find_with_resource_array(WinLibrary *fi, WinResource *wr, char *id)
{
	int c, rescnt;
	WinResource *return_wr;

	wr = list_resources(fi, wr, &rescnt);
	if (wr == NULL)
		return NULL;

	for (c = 0 ; c < rescnt ; c++) {
		if (compare_resource_id (&wr[c], id)) {
			/* duplicate WinResource and return it */
			return_wr = xmalloc(sizeof(WinResource));
			memcpy(return_wr, &wr[c], sizeof(WinResource));

			/* free old WinResource */
			free(wr);
			return return_wr;
		}
	}

	return NULL;
}

WinResource *
find_resource (WinLibrary *fi, char *type, char *name, char *language, int *level)
{
	WinResource *wr;

	*level = 0;
	if (type == NULL)
		return NULL;
	wr = find_with_resource_array(fi, NULL, type);
	if (wr == NULL || !wr->is_directory)
		return wr;

	*level = 1;
	if (name == NULL)
		return wr;
  free(wr);
	wr = find_with_resource_array(fi, wr, name);
	if (wr == NULL || !wr->is_directory)
		return wr;

	*level = 2;
	if (language == NULL)
		return wr;
  free(wr);
	wr = find_with_resource_array(fi, wr, language);
	return wr;
}
