/* osxwres.m - wrestool bridge for Mac OS X / NeXTSTEP(untested) / GnuStep(untested)
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

#import <Cocoa/Cocoa.h>
#include "osxwres.h"
//#include "io-utils.h"
#include "restypes.h"
#include "restable.h"
#include "common/common.h"


NSData *
extract_resources_nsdata (WinLibrary *fi, WinResource *wr,
                            WinResource *type_wr, WinResource *name_wr,
                            WinResource *lang_wr)
{
	int size;
	bool free_it;
	void *memory;

	memory = extract_resource(fi, wr, &size, &free_it, type_wr->id, (lang_wr == NULL ? NULL : lang_wr->id), false);
  if (memory == NULL) return (NSData *)-1;
  NSData *icoData = [[NSData alloc] initWithBytes:memory length:size];
  if (free_it) free(memory);
  
	return icoData;
}


/* version of do_resources_recurs that stops when it finds the first non-directory matching item */

static NSData *
get_default_resource (WinLibrary *fi, WinResource *base, WinResource *type_wr,
                          WinResource *name_wr, WinResource *lang_wr,
						  char *type, char *name, char *lang)
{
	int c, rescnt;
	WinResource *wr;

	/* get a list of all resources at this level */
	wr = list_resources (fi, base, &rescnt);
	if (wr == NULL)
		return (NSData *)-1;

	/* process each resource listed */
	for (c = 0 ; c < rescnt ; c++) {
		/* (over)write the corresponding WinResource holder with the current */
    //WINRESOURCE_BY_LEVEL --> type_wr, lang_wr, name_wr
		memcpy(WINRESOURCE_BY_LEVEL(wr[c].level), wr+c, sizeof(WinResource));
    //NSLog(@"%s of level %d", wr[c].id, wr[c].level);
		/* go deeper unless there is something that does NOT match */
		if (LEVEL_MATCHES(type) && LEVEL_MATCHES(name) && LEVEL_MATCHES(lang)) {
			if (wr->is_directory) {
				void *temp = get_default_resource (fi, wr+c, type_wr, name_wr, lang_wr, type, name, lang);
        if (temp != NULL) {
          free(wr);
          return temp;
        }
      } else {
				NSData* temp = extract_resources_nsdata(fi, wr+c, type_wr, name_wr, lang_wr);
        free(wr);
        return temp;
      }
		}
	}

	/* since we're moving back one level after this, unset the
	 * WinResource holder used on this level */
  //Not really necessary, after all
	//memset(WINRESOURCE_BY_LEVEL(wr[0].level), 0, sizeof(WinResource));
  free(wr);
  return NO;
}


/* nsdata_default_icon:
 *   Extracts default (first) matching resource as a NSData object
 */

NSData *
nsdata_default_resource (WinLibrary *fi, char *type, char *name, char *lang, extract_error *err)
{
  WinResource *type_wr;
	WinResource *name_wr;
	WinResource *lang_wr;

	type_wr = malloc(sizeof(WinResource)*3);
	name_wr = type_wr + 1;
	lang_wr = type_wr + 2;
	memset(type_wr, 0, sizeof(WinResource)*3);
  
  NSData *temp = get_default_resource(fi, NULL, type_wr, name_wr, lang_wr, type, name, lang);
  
  *err = EXTR_NOERR;
  if (temp == (NSData *)-1)  //someday i'll change this
    *err = EXTR_FAIL;
  else if (temp == NULL)
    *err = EXTR_NOTFOUND;
  else
    [temp autorelease];

	free(type_wr);
  return temp;
}


