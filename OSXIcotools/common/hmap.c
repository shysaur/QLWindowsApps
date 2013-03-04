/* hmap.c - A hash map data structure
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

#include <config.h>
#include <stdlib.h>		/* Gnulib/C89 */
#include <string.h>		/* Gnulib/C89 */
#include <ctype.h>		/* C89 */
#include "xalloc.h"		/* Gnulib */
#include "comparison.h"		/* common */
#include "hmap.h"		/* common */

#define DEFAULT_CAPACITY    11
#define DEFAULT_LOAD_FACTOR 0.75F

typedef struct _HMapEntry HMapEntry;
typedef struct _HMapIteratorPriv HMapIteratorPriv;

struct _HMapEntry {
    void *key;
    void *value;
    HMapEntry *next;
};

struct _HMap {
    HMapEntry **buckets;
    size_t buckets_length;
    size_t threshold;
    float load_factor;
    size_t size;

    hash_fn_t hash;
    comparison_fn_t compare;
};

struct _HMapIteratorPriv {
    bool (*has_next)(HMapIterator *it);
    void *(*next)(HMapIterator *it);

    HMap *map;
    uint32_t index;
    HMapEntry *entry;
    HMapEntry *previous_entry;
};

uint32_t
strhash(const char *str)
{
    uint32_t hash = 0;

    for (; *str != '\0'; str++)
	hash = (hash << 5) - hash + *str;

    return hash;
}

uint32_t
strcasehash(const char *str)
{
    uint32_t hash = 0;

    for (; *str != '\0'; str++)
	hash = (hash << 5) - hash + tolower(*str);

    return hash;
}

static inline uint32_t
hmap_hash(HMap *map, const void *key)
{
    return (key == NULL ? 0 : map->hash(key) % map->buckets_length);
}

static void *
hmap_iterator_next(HMapIterator *it)
{
    HMapIteratorPriv *itp = (HMapIteratorPriv *) it;
    HMap *map = itp->map;
    void *data;

    if (itp->entry == NULL)
    	return NULL;

    data = itp->entry->value;
    itp->previous_entry = itp->entry;

    itp->entry = itp->entry->next;
    if (itp->entry == NULL) {
	uint32_t i;

    	i = itp->index+1;
	while (i < map->buckets_length && map->buckets[i] == NULL)
	    i++;
	itp->index = i;
	itp->entry = (i < map->buckets_length ? map->buckets[i] : NULL);
    }

    return data;
}

static bool
hmap_iterator_has_next(HMapIterator *it)
{
    HMapIteratorPriv *itp = (HMapIteratorPriv *) it;
    return itp->entry != NULL;
}

static inline void
hmap_rehash(HMap *map)
{	
    HMapEntry **old_buckets = map->buckets;
    uint32_t old_capacity = map->buckets_length;
    uint32_t i;

    map->buckets_length = (map->buckets_length * 2) + 1;
    map->threshold = (uint32_t) (map->buckets_length * map->load_factor);
    map->buckets = xmalloc(map->buckets_length * sizeof(HMapEntry *));
    memset(map->buckets, 0, map->buckets_length * sizeof(HMapEntry *));

    for (i = 0; i < old_capacity; i++) {
	HMapEntry *entry = old_buckets[i];
	while (entry != NULL) {
	    uint32_t index = hmap_hash(map, entry->key);
	    HMapEntry *dest = map->buckets[index];
	    HMapEntry *next;

	    if (dest != NULL) {
		    while (dest->next != NULL)
			    dest = dest->next;
		    dest->next = entry;
	    } else {
		    map->buckets[index] = entry;
	    }

	    next = entry->next;
	    entry->next = NULL;
	    entry = next;
	}
    }

    free(old_buckets);
}

void
hmap_set_compare_fn(HMap *map, comparison_fn_t compare)
{
    map->compare = compare;
}

void
hmap_set_hash_fn(HMap *map, hash_fn_t hash)
{
    map->hash = hash;
}

HMap *
hmap_new(void)
{
    HMap *map;

    map = xmalloc(sizeof(HMap));
    map->buckets_length = DEFAULT_CAPACITY;
    map->load_factor = DEFAULT_LOAD_FACTOR;
    map->buckets = xmalloc(map->buckets_length * sizeof(HMapEntry *));
    map->threshold = (uint32_t) (map->buckets_length * map->load_factor);
    map->size = 0;
    map->hash = (hash_fn_t) strhash;
    map->compare = (comparison_fn_t) strcmp;
    memset(map->buckets, 0, map->buckets_length * sizeof(HMapEntry *));

    return map;
}

void
hmap_free(HMap *map)
{
    if (map != NULL) {
	hmap_clear(map);
	free(map->buckets);
	free(map);
    }
}

#if 0
typedef int (*HMapCustomComparator*)(const void *key0, const void *key, void *data);

static HMapEntry *
hmap_get_entry_custom(HMap *map, const void *key, HMapCustomComparator *cmp, void *data)
{
    HMapEntry *entry = map->buckets[hmap_hash(map, key)];

    if (key == NULL) {
	for (; entry != NULL; entry = entry->next) {
	    if (entry->key == NULL)
		return entry;
	}
    } else {
	for (; entry != NULL; entry = entry->next) {
	    if (cmp(key, entry->key, data) == 0)
		return entry;
	}
    }

    return NULL;
}

/* Example usage:
 *  int
 *  map_strncmp(const void *key0, const void *key, void *data)
 *  {
 *      size_t n = *(size_t *) data;
 *      return strncmp(key0, key, n);
 *  }
 *  value = hmap_get_custom(map, unterminated_key, map_strncmp, &key_length);
 */

void *
hmap_get_custom(HMap *map, const void *key, HMapCustomComparator *cmp, void *data)
{
    HMapEntry *entry = hmap_get_entry_custom(map, key, cmp, data);
    return (entry != NULL ? entry->value : NULL);
}
#endif

static HMapEntry *
hmap_get_entry(HMap *map, const void *key)
{
    HMapEntry *entry = map->buckets[hmap_hash(map, key)];

    if (key == NULL) {
	for (; entry != NULL; entry = entry->next) {
	    if (entry->key == NULL)
		return entry;
	}
    } else {
	for (; entry != NULL; entry = entry->next) {
	    if (map->compare(key, entry->key) == 0)
		return entry;
	}
    }

    return NULL;
}

void *
hmap_get(HMap *map, const void *key)
{
    HMapEntry *entry = hmap_get_entry(map, key);
    return entry != NULL ? entry->value : NULL;
}

void *
hmap_put(HMap *map, void *key, void *value)
{
    HMapEntry *entry;
    uint32_t index;

    index = hmap_hash(map, key);
    if (key == NULL) {
	for (entry = map->buckets[index]; entry != NULL; entry = entry->next) {
	    if (entry->key == NULL) {
		void *old_value = entry->value;
		entry->value = value;
		return old_value;
	    }
	}
    } else {
	for (entry = map->buckets[index]; entry != NULL; entry = entry->next) {
	    if (map->compare(key, entry->key) == 0) {
		void *old_value = entry->value;
		entry->value = value;
		return old_value;
	    }
	}
    }

    map->size++;
    if (map->size > map->threshold) {
	hmap_rehash(map);
	index = hmap_hash(map, key);
    }

    entry = xmalloc(sizeof(HMapEntry));
    entry->key = key;
    entry->value = value;
    entry->next = map->buckets[index];
    map->buckets[index] = entry;

    return NULL;
}

void *
hmap_remove(HMap *map, const void *key)
{
    uint32_t index = hmap_hash(map, key);
    HMapEntry *entry;
    HMapEntry *last = NULL;

    if (key == NULL) {
	for (entry = map->buckets[index]; entry != NULL; entry = entry->next) {
	    if (entry->key == NULL) {
		void *value = entry->value;
		if (last == NULL)
		    map->buckets[index] = entry->next;
		else
		    last->next = entry->next;
		map->size--;
		free(entry);
		return value;
	    }
	    last = entry;
	}
    } else {
	for (entry = map->buckets[index]; entry != NULL; entry = entry->next) {
	    if (map->compare(key, entry->key) == 0) {
		void *value = entry->value;
		if (last == NULL)
		    map->buckets[index] = entry->next;
		else
		    last->next = entry->next;
		map->size--;
		free(entry);
		return value;
	    }
	    last = entry;
	}
    }

    return NULL;
}

void
hmap_iterator(HMap *map, HMapIterator *it)
{
    HMapIteratorPriv *itp = (HMapIteratorPriv *) it;
    uint32_t i;

    it->next = hmap_iterator_next;
    it->has_next = hmap_iterator_has_next;

    for (i = 0; i < map->buckets_length && map->buckets[i] == NULL; i++);
    itp->map = map;
    itp->index = i;
    itp->entry = (i < map->buckets_length ? map->buckets[i] : NULL);
    itp->previous_entry = NULL;
}

/* It is allowed to remove the current entry from the iterator callback
 * function. But no other entry.
 */
void
hmap_foreach_value(HMap *map, void (*iterator)())
{
    uint32_t c;

    for (c = 0; c < map->buckets_length; c++) {
	HMapEntry *entry;
	for (entry = map->buckets[c]; entry != NULL; ) {
	    HMapEntry *next = entry->next;
	    iterator(entry->value);
	    entry = next;
	}
    }
}

void
hmap_foreach_key(HMap *map, void (*iterator)())
{
    uint32_t c;

    for (c = 0; c < map->buckets_length; c++) {
	HMapEntry *entry;
	for (entry = map->buckets[c]; entry != NULL; ) {
	    HMapEntry *next = entry->next;
	    iterator(entry->key);
	    entry = next;
	}
    }
}

void
hmap_clear(HMap *map)
{
    uint32_t c;

    for (c = 0; c < map->buckets_length; c++) {
	HMapEntry *entry = map->buckets[c];
	while (entry != NULL) {
	    HMapEntry *next = entry->next;
	    free(entry);
	    entry = next;
	}
	map->buckets[c] = NULL;
    }

    map->size = 0;
}

size_t
hmap_size(HMap *map)
{
    return map->size;
}

bool
hmap_contains_key(HMap *map, const void *key)
{
    return hmap_get_entry(map, key) != NULL;
}
