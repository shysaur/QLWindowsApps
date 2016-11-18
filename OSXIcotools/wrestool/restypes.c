/* restypes.c - Resource type exchange functions for wrestool
 *
 * Copyright (C) 1998 Oskar Liljeblad
 * Copyright (C) 2012, 2016 Daniele Cattaneo
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

#include "restypes.h"
#include "common/string-utils.h"
#include "wrestool.h"
#include "common/common.h"

static const char *res_types[] = {
    /* 0x01: */
    "cursor", "bitmap", "icon", "menu", "dialog", "string",
    "fontdir", "font", "accelerator", "rcdata", "messagelist",
    "group_cursor", NULL, "group_icon", NULL,
    /* the following are not defined in winbase.h, but found in wrc. */
    /* 0x10: */ 
    "version", "dlginclude", NULL, "plugplay", "vxd",
    "anicursor", "aniicon"
};
#define RES_TYPE_COUNT (sizeof(res_types)/sizeof(char *))

/* res_type_id_to_string:
 *   Translate a numeric resource type to it's corresponding string type.
 *   (For informative-ness.)
 */
const char *res_type_id_to_string (int id)
{
    if (id == 241)
		return "toolbar";
    if (id > 0 && id <= RES_TYPE_COUNT)
		return res_types[id-1];
    return NULL;
}

/* res_type_string_to_id:
 *   Translate a resource type string to integer.
 *   (Used to convert the --type option.)
 */
const char *res_type_string_to_id (char *type)
{
    static const char *res_type_ids[] = {
	"-1", "-2", "-3", "-4", "-5", "-6", "-7", "-8", "-9", "-10",
	"-11", "-12", NULL, "-14", NULL, "-16", "-17", NULL, "-19",
	"-20", "-21", "-22"
    };
    int c;

    if (type == NULL)
	return NULL;

    for (c = 0 ; c < RES_TYPE_COUNT ; c++) {
	if (res_types[c] != NULL && !strcasecmp(type, res_types[c]))
	    return res_type_ids[c];
    }

    return type;
}
