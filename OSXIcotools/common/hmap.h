/* hmap.h - A hash map data structure
 *
 * Copyright (C) 2004 Oskar Liljeblad
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

#ifndef COMMON_HMAP_H
#define COMMON_HMAP_H

#include <stdbool.h>		/* Gnulib/C99/POSIX */
#include <stdint.h>		/* Gnulib/C99/POSIX */
#include <stdlib.h>		/* Gnulib/C89 */
#include "comparison.h"		/* common */

typedef struct _HMap HMap;
typedef struct _HMapIterator HMapIterator;

typedef uint32_t (*hash_fn_t)(const void *key);

struct _HMapIterator {
    bool (*has_next)(HMapIterator *it);
    void *(*next)(HMapIterator *it);

    /* Private data follow */
    void *p0;
    uint32_t p1;
    void *p2;
    void *p3;
};

uint32_t strhash(const char *str);
uint32_t strcasehash(const char *str);
#define hmap_is_empty(m) ((hmap_size(m) == 0)
HMap *hmap_new(void);
void hmap_free(HMap *map);
void *hmap_get(HMap *map, const void *key);
void *hmap_put(HMap *map, void *key, void *value);
bool hmap_contains_key(HMap *map, const void *key);
void *hmap_remove(HMap *map, const void *key);
void hmap_iterator(HMap *map, HMapIterator *it);
void hmap_foreach_key(HMap *map, void (*iterator)());
void hmap_foreach_value(HMap *map, void (*iterator)());
void hmap_clear(HMap *map);
size_t hmap_size(HMap *map);
void hmap_set_hash_fn(HMap *map, hash_fn_t hash);
void hmap_set_compare_fn(HMap *map, comparison_fn_t compare);

#endif
