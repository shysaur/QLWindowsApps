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
#include "wrestool.h"
#include "win32.h"
#include "fileread.h"
#include "restypes.h"
//#include "common/common.h"

static bool decode_pe_resource_id (WinLibrary *, WinResource *, uint32_t);
static bool decode_ne_resource_id (WinLibrary *, WinResource *, uint16_t);
static WinResource *list_ne_type_resources (WinLibrary *, int *, wres_error *);
static WinResource *list_ne_name_resources (WinLibrary *, WinResource *, int *, wres_error *);
static WinResource *list_pe_resources (WinLibrary *, Win32ImageResourceDirectory *, int, int *, wres_error *);
static wres_error do_resources_recurs (WinLibrary *, WinResource *, WinResource *, WinResource *, WinResource *, char *, char *, char *, DoResourceCallback);
static char *get_resource_id_quoted (WinResource *);
static WinResource *find_with_resource_array(WinLibrary *, WinResource *, char *, wres_error *);

#define NE_TYPEINFO_NEXT(x) ((Win16NETypeInfo *)((uint8_t *)(x) + sizeof(Win16NETypeInfo) + \
						    ((Win16NETypeInfo *)x)->count * sizeof(Win16NENameInfo)))
#define NE_RESOURCE_NAME_IS_NUMERIC (0x8000)

bool compare_resource_id (WinResource *, char *);

/* what is each entry in this directory level for? type, name or language? */
#define WINRESOURCE_BY_LEVEL(x) ((x)==0 ? type_wr : ((x)==1 ? name_wr : lang_wr))

/* does the id of this entry match the specified id? */
#define LEVEL_MATCHES(x) (x == NULL || x ## _wr->id[0] == '\0' || compare_resource_id(x ## _wr, x))


/* do_resources:
 *   Do something for each resource matching type, name and lang.
 */

wres_error
do_resources (WinLibrary *fi, char *type, char *name, char *lang, DoResourceCallback cb)
{
	wres_error err;
	WinResource *type_wr;
	WinResource *name_wr;
	WinResource *lang_wr;

	type_wr = calloc(sizeof(WinResource), 3);
	name_wr = type_wr + 1;
	lang_wr = type_wr + 2;

	err = do_resources_recurs(fi, NULL, type_wr, name_wr, lang_wr, type, name, lang, cb);

	free(type_wr);
	return err;
}

static wres_error
do_resources_recurs (WinLibrary *fi, WinResource *base, WinResource *type_wr,
                          WinResource *name_wr, WinResource *lang_wr,
						  char *type, char *name, char *lang, DoResourceCallback cb)
{
	wres_error err;
	int c, rescnt;
	WinResource *wr;

	/* get a list of all resources at this level */
	wr = list_resources (fi, base, &rescnt, &err);
	if (wr == NULL)
		return err;

	/* process each resource listed */
	for (c = 0 ; c < rescnt ; c++) {
		/* (over)write the corresponding WinResource holder with the current */
		memcpy(WINRESOURCE_BY_LEVEL(wr[c].level), wr+c, sizeof(WinResource));

		/* go deeper unless there is something that does NOT match */
		if (LEVEL_MATCHES(type) && LEVEL_MATCHES(name) && LEVEL_MATCHES(lang)) {
			if (wr->is_directory) {
				err = do_resources_recurs (fi, wr+c, type_wr, name_wr, lang_wr, type, name, lang, cb);
				if (err != WRES_ERROR_NONE)
					return err;
			} else
				cb(fi, wr+c, type_wr, name_wr, lang_wr);
		}
	}

	/* since we're moving back one level after this, unset the
	 * WinResource holder used on this level */
	memset(WINRESOURCE_BY_LEVEL(wr[0].level), 0, sizeof(WinResource));
	return WRES_ERROR_NONE;
}



void
print_resources_callback (WinLibrary *fi, WinResource *wr,
                          WinResource *type_wr, WinResource *name_wr,
						  WinResource *lang_wr)
{
	const char *type, *offset;
	int32_t id, size;

	/* get named resource type if possible */
	type = NULL;
	if (parse_int32(type_wr->id, &id))
		type = res_type_id_to_string(id);

	/* get offset and size info on resource */
	offset = get_resource_entry(fi, wr, &size, NULL);
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
	/* Empty string is wildcard for disabling comparison */
	if (*id == 0) return true;
  
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
		RETURN_IF_BAD_POINTER(fi, false, *mem);
		len = mem[0];
		RETURN_IF_BAD_OFFSET(fi, false, &mem[1], sizeof(uint16_t) * len);

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
get_resource_entry (WinLibrary *fi, WinResource *wr, int *size, wres_error *err)
{
	if (fi->binary_type == PE_BINARY || fi->binary_type == PEPLUS_BINARY) {
		Win32ImageResourceDataEntry *dataent;

		dataent = (Win32ImageResourceDataEntry *) wr->children;
		RET_NULL_AND_SET_ERR_IF_BAD_POINTER(fi, err, *dataent);
		*size = dataent->size;
		RET_NULL_AND_SET_ERR_IF_BAD_OFFSET(fi, err, fi->memory + dataent->offset_to_data, *size);

		return fi->memory + dataent->offset_to_data;
	} else {
		Win16NENameInfo *nameinfo;
		int sizeshift;

		nameinfo = (Win16NENameInfo *) wr->children;
		sizeshift = *((uint16_t *) fi->first_resource - 1);
		*size = nameinfo->length << sizeshift;
		RET_NULL_AND_SET_ERR_IF_BAD_OFFSET(fi, err, fi->memory + (nameinfo->offset << sizeshift), *size);

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
		RETURN_IF_BAD_POINTER(fi, false, *mem);
		len = mem[0];
		RETURN_IF_BAD_OFFSET(fi, false, &mem[1], sizeof(char) * len);
		memcpy(wr->id, &mem[1], len);
		wr->id[len] = '\0';
	}

	wr->numeric_id = (value & NE_RESOURCE_NAME_IS_NUMERIC ? true:false);
	return true;
}

static WinResource *
list_pe_resources (WinLibrary *fi, Win32ImageResourceDirectory *pe_res, int level, int *count, wres_error *err)
{
	WinResource *wr;
	int c, rescnt;
	Win32ImageResourceDirectoryEntry *dirent
	  = (Win32ImageResourceDirectoryEntry *) (pe_res + 1);

	/* count number of `type' resources */
	RET_NULL_AND_SET_ERR_IF_BAD_POINTER(fi, err, *dirent);
	rescnt = pe_res->number_of_named_entries + pe_res->number_of_id_entries;
	*count = rescnt;

	/* allocate WinResource's */
	wr = xmalloc(sizeof(WinResource) * rescnt);

	/* fill in the WinResource's */
	for (c = 0 ; c < rescnt ; c++) {
		RET_NULL_AND_SET_ERR_IF_BAD_POINTER(fi, err, dirent[c]);
		wr[c].this = pe_res;
		wr[c].level = level;
		wr[c].is_directory = (dirent[c].u2.s.data_is_directory);
		wr[c].children = fi->first_resource + dirent[c].u2.s.offset_to_directory;

		/* fill in wr->id, wr->numeric_id */
		if (!decode_pe_resource_id (fi, wr + c, dirent[c].u1.name)) {
			if (err) *err = WRES_ERROR_PREMATUREEND;
			return NULL;
		}
	}

	return wr;
}

static WinResource *
list_ne_name_resources (WinLibrary *fi, WinResource *typeres, int *count, wres_error *err)
{
	int c, rescnt;
	WinResource *wr;
	Win16NETypeInfo *typeinfo = (Win16NETypeInfo *) typeres->this;
	Win16NENameInfo *nameinfo = (Win16NENameInfo *) typeres->children;

	/* count number of `type' resources */
	RET_NULL_AND_SET_ERR_IF_BAD_POINTER(fi, err, typeinfo->count);
	*count = rescnt = typeinfo->count;

	/* allocate WinResource's */
	wr = xmalloc(sizeof(WinResource) * rescnt);

	/* fill in the WinResource's */
	for (c = 0 ; c < rescnt ; c++) {
		RET_NULL_AND_SET_ERR_IF_BAD_POINTER(fi, err, nameinfo[c]);
		wr[c].this = nameinfo+c;
		wr[c].is_directory = false;
		wr[c].children = nameinfo+c;
		wr[c].level = 1;

		/* fill in wr->id, wr->numeric_id */
		if (!decode_ne_resource_id (fi, wr + c, (nameinfo+c)->id)) {
			if (err) *err = WRES_ERROR_PREMATUREEND;
			return NULL;
		}
	}

	return wr;
}

static WinResource *
list_ne_type_resources(WinLibrary *fi, int *count, wres_error *err)
{
	int c, rescnt;
	WinResource *wr;
	Win16NETypeInfo *typeinfo;

	/* count number of `type' resources */
	typeinfo = (Win16NETypeInfo *) fi->first_resource;
	RET_NULL_AND_SET_ERR_IF_BAD_POINTER(fi, err, *typeinfo);
	
	for (rescnt = 0 ; typeinfo->type_id != 0 ; rescnt++) {
		if (((char *) NE_TYPEINFO_NEXT(typeinfo))+sizeof(uint16_t) > fi->memory + fi->total_size) {
			if (err) *err = WRES_ERROR_INVALIDRESTABLE;
		    return NULL;
		}
		
		typeinfo = NE_TYPEINFO_NEXT(typeinfo);
		RET_NULL_AND_SET_ERR_IF_BAD_POINTER(fi, err, *typeinfo);
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
		if (!decode_ne_resource_id(fi, wr + c, typeinfo->type_id)) {
			if (err) *err = WRES_ERROR_PREMATUREEND;
			return NULL;
		}

		typeinfo = NE_TYPEINFO_NEXT(typeinfo);
	}

	return wr;
}

/* list_resources:
 *   Return an array of WinResource's in the current
 *   resource level specified by res.
 */
/*static*/ WinResource *
list_resources(WinLibrary *fi, WinResource *res, int *count, wres_error *err)
{
	if (res != NULL && !res->is_directory)
		return NULL;

	if (fi->binary_type == PE_BINARY || fi->binary_type == PEPLUS_BINARY) {
		return list_pe_resources(fi, (Win32ImageResourceDirectory *)
				 (res == NULL ? fi->first_resource : res->children),
				 (res == NULL ? 0 : res->level+1),
				 count, err);
	} else {
		return (res == NULL
				? list_ne_type_resources(fi, count, err)
				: list_ne_name_resources(fi, res, count, err));
	}
}


static WinResource *
find_with_resource_array(WinLibrary *fi, WinResource *wr, char *id, wres_error *err)
{
	int c, rescnt;
	WinResource *return_wr;

	wr = list_resources(fi, wr, &rescnt, err);
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

	if (err) *err = WRES_ERROR_RESNOTFOUND;
	return NULL;
}

WinResource *
find_resource (WinLibrary *fi, char *type, char *name, char *language, int *level, wres_error *err)
{
	WinResource *wr, *old_wr;

	*level = 0;
	if (type == NULL) {
		if (err) *err = WRES_ERROR_INVALIDPARAM;
		return NULL;
	}
	wr = find_with_resource_array(fi, NULL, type, err);
	if (wr == NULL || !wr->is_directory)
		return wr;

	*level = 1;
	if (name == NULL)
		return wr;
	old_wr = wr;
	wr = find_with_resource_array(fi, old_wr, name, err);
	free(old_wr);
	if (wr == NULL || !wr->is_directory)
		return wr;

	*level = 2;
	if (language == NULL)
		return wr;
	old_wr = wr;
	wr = find_with_resource_array(fi, old_wr, language, err);
	free(old_wr);
	return wr;
}
