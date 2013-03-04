/* llist.c - A linked list with a container object (unlike GList)
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

#if HAVE_CONFIG_H
#include <config.h>
#endif
#include <sys/types.h>		/* POSIX */
#include <stdint.h>		/* Gnulib/POSIX */
#include <stdbool.h>		/* Gnulib/POSIX */
#include <stdlib.h>		/* Gnulib/C89 */
#include <gettext.h>		/* Gnulib/Gettext */
#define _(s) gettext(s)
#include "xalloc.h"		/* Gnulib */
#include "llist.h"		/* common */
#include "error.h"		/* common */

typedef struct _LListIteratorPriv LListIteratorPriv;

struct _LNode {
	void *data;
	LNode *next;
	LNode *previous;
};

struct _LList {
	LNode *first;
	LNode *last;
	uint32_t size;
};

struct _LListIteratorPriv {
    bool (*has_next)(LListIterator *it);
    void *(*next)(LListIterator *it);
    void (*remove)(LListIterator *it);
    LList *list;
    LNode *entry;
};

static inline void llist_add_last_entry(LList *list, LNode *entry);
static inline void llist_remove_entry(LList *list, LNode *entry);
static inline LNode *llist_get_entry(LList *list, uint32_t index);
static bool llist_iterator_has_next(LListIterator *it);
static void *llist_iterator_next(LListIterator *it);
static void llist_iterator_remove(LListIterator *it);
#if 0
static void llist_iterator_restart(Iterator *it);
static void *llist_iterator_previous(Iterator *it);
static void llist_iterator_add(Iterator *it, void *value);
#endif

LList *
llist_new(void)
{
	LList *list = xmalloc(sizeof(LList));
	
	list->first = NULL;
	list->last = NULL;
	list->size = 0;

	return list;
}

void
llist_free(LList *list)
{
	LNode *entry = list->first;

	while (entry != NULL) {
		LNode *next = entry->next;
		free(entry);
		entry = next;
	}

	free(list);
}

void *
llist_get_first(LList *list)
{
	return (list->size == 0 ? NULL : list->first->data);
}

void *
llist_get_last(LList *list)
{
	return (list->size == 0 ? NULL : list->last->data);
}

void *
llist_remove_first(LList *list)
{
	LNode *entry;
	void *data;

	if (list->size == 0)
		return NULL;

	list->size--;
	entry = list->first;
	data = entry->data;

	if (entry->next != NULL)
		entry->next->previous = NULL;
	else
		list->last = NULL;
	list->first = entry->next;
	free(entry);

	return data;
}

void *
llist_remove_last(LList *list)
{
	LNode *entry;
	void *data;

	if (list->size == 0)
		return NULL;

	list->size--;
	entry = list->last;
	data = entry->data;

	if (entry->previous != NULL)
		entry->previous->next = NULL;
	else
		list->first = NULL;
	list->last = entry->previous;
	free(entry);

	return data;
}

void
llist_add_first(LList *list, void *data)
{
	LNode *entry = xmalloc(sizeof(LNode));
	entry->data = data;

	if (list->size == 0) {
		entry->next = NULL;
		entry->previous = NULL;
		list->first = list->last = entry;
	} else {
		entry->next = list->first;
		entry->previous = NULL;
		list->first->previous = entry;
		list->first = entry;
	}
	list->size++;
}

void
llist_add_last(LList *list, void *data)
{
	LNode *entry = xmalloc(sizeof(LNode));
	entry->data = data;
	llist_add_last_entry(list, entry);
}

static inline void
llist_add_last_entry(LList *list, LNode *entry)
{
	if (list->size == 0) {
		entry->next = NULL;
		entry->previous = NULL;
		list->first = list->last = entry;
	} else {
		entry->next = NULL;
		entry->previous = list->last;
		list->last->next = entry;
		list->last = entry;
	}
	list->size++;
}

bool
llist_remove(LList *list, void *data)
{
	LNode *entry;

	for (entry = list->first; entry != NULL; entry = entry->next) {
		if (entry->data == data) {
			llist_remove_entry(list, entry);
			return true;
		}
	}

	return false;
}

static inline void
llist_remove_entry(LList *list, LNode *entry)
{
	if (list->size == 1) {
		list->first = list->last = NULL;
	} else if (entry == list->first) {
		list->first = entry->next;
		entry->next->previous = NULL;
	} else if (entry == list->last) {
		list->last = entry->previous;
		entry->previous->next = NULL;
	} else {
		entry->next->previous = entry->previous;
		entry->previous->next = entry->next;
	}
	list->size--;
	free(entry);
}

static inline LNode *
llist_get_entry(LList *list, uint32_t index)
{
	LNode *entry;

	if (index < list->size/2) {
		entry = list->first;
		while (index-- > 0)
			entry = entry->next;
	} else {
		entry = list->last;
		while (++index < list->size)
			entry = entry->previous;
	}

	return entry;
}

bool
llist_contains(LList *list, void *data)
{
	LNode *entry;

	for (entry = list->first; entry != NULL; entry = entry->next) {
		if (entry->data == data)
			return true;
	}

	return false;
}

uint32_t
llist_size(LList *list)
{
	return list->size;
}

void
llist_clear(LList *list)
{
	LNode *entry = list->first;

	while (entry != NULL) {
		LNode *next = entry->next;
		free(entry);
		entry = next;
	}

	list->first = list->last = NULL;
	list->size = 0;
}

void *
llist_get(LList *list, uint32_t index)
{
	LNode *entry;

	if (index >= list->size)
		return NULL;

	entry = llist_get_entry(list, index);
	return entry->data;
}

void *
llist_set(LList *list, uint32_t index, void *data)
{
	LNode *entry;
	void *old_data;

	if (index >= list->size)
		return NULL;

	entry = llist_get_entry(list, index);
	old_data = entry->data;
	entry->data = old_data;
	return old_data;
}

void
llist_add_all(LList *list, LList *list2)
{
    uint32_t c;
    for (c = 0; c < list2->size; c++)
    	llist_add(list, llist_get_entry(list2, c)->data);
}

void
llist_add_at(LList *list, uint32_t index, void *data)
{
	LNode *entry;
	
	if (index > list->size)
		return;

	entry = xmalloc(sizeof(LNode));
	entry->data = data;

	if (index < list->size) {
		LNode *after = llist_get_entry(list, index);
		entry->next = after;
		entry->previous = after->previous;
		if (after->previous == NULL)
			list->first = entry;
		else
			after->previous->next = entry;
		after->previous = entry;
		list->size++;
	} else {
		llist_add_last_entry(list, entry);
	}
}

void *
llist_remove_at(LList *list, uint32_t index)
{
	LNode *entry;
	void *data;

	if (index >= list->size)
		return NULL;

	entry = llist_get_entry(list, index);
	data = entry->data;
	llist_remove_entry(list, entry);
	return data;
}

int32_t
llist_index_of(LList *list, void *data)
{
	int32_t index = 0;
	LNode *entry;

	for (entry = list->first; entry != NULL; entry = entry->next) {
		if (entry->data == data)
			return index;
		index++;
	}

	return -1;
}

int32_t
llist_last_index_of(LList *list, void *data)
{
	int32_t index = list->size - 1;
	LNode *entry;

	for (entry = list->last; entry != NULL; entry = entry->previous) {
		if (entry->data == data)
			return index;
		index--;
	}

	return -1;
}

LList *
llist_clone(LList *list)
{
	LList *copy = xmalloc(sizeof(LList));
	LNode *entry;
	LNode *previous = NULL;

	copy->size = list->size;

	for (entry = list->first; entry != NULL; entry = entry->next) {
		LNode *new_entry = xmalloc(sizeof(LNode));
		new_entry->data = entry->data;
		new_entry->previous = previous;
		if (previous == NULL)
			copy->first = new_entry;
		else
			previous->next = new_entry;
		previous = new_entry;
	}

	copy->last = previous;
	if (previous == NULL)
		list->first = NULL;
	else
		previous->next = NULL;

	return copy;
}

void **
llist_to_array(LList *list)
{
	void **array = xmalloc(list->size * sizeof(void *));
	LNode *entry;
	uint32_t index = 0;

	for (entry = list->first; entry != NULL; entry = entry->next)
		array[index++] = entry->data;

	return array;
}

void **
llist_to_null_terminated_array(LList *list)
{
	void **array = xmalloc((list->size+1) * sizeof(void *));
	LNode *entry;
	uint32_t index = 0;

	for (entry = list->first; entry != NULL; entry = entry->next)
		array[index++] = entry->data;
	array[index] = NULL;

	return array;
}

bool
llist_is_empty(LList *list)
{
	return list->size == 0;
}

void
llist_iterate(LList *list, void (*iterator_func)())
{
	LNode *entry;
	for (entry = list->first; entry != NULL; entry = entry->next)
		iterator_func(entry->data);
}

void
llist_iterator(LList *list, LListIterator *it)
{
    LListIteratorPriv *itp = (LListIteratorPriv *) it;
    itp->list = list;
    itp->entry = list->first;
    itp->has_next = llist_iterator_has_next;
    itp->next = llist_iterator_next;
    itp->remove = llist_iterator_remove;
}

static bool
llist_iterator_has_next(LListIterator *it)
{
    LListIteratorPriv *itp = (LListIteratorPriv *) it;
    return itp->entry != NULL;
}

static void *
llist_iterator_next(LListIterator *it)
{
    LListIteratorPriv *itp = (LListIteratorPriv *) it;
    void *data = itp->entry->data;
    itp->entry = itp->entry->next;
    return data;
}

static void
llist_iterator_remove(LListIterator *it)
{
    LListIteratorPriv *itp = (LListIteratorPriv *) it;

    if (itp->list->first == itp->entry)
	internal_error(_("Called iterator_remove before first iterator_next"));
    //FIXME: we should probably check that this function isn't called
    // write between next entry.. this is probably done by lastReturned

    if (itp->entry == NULL) {
	llist_remove_entry(itp->list, itp->list->last);
    } else {
	llist_remove_entry(itp->list, itp->entry->previous);
    }
}

#if 0
static void *
llist_iterator_previous(LListIterator *it)
{
    LListIteratorPriv *itp = (LListIteratorPriv *) it;
    void *data;

    /* FIXME: not sure if this is right.
     * This will happen if the last next() returned the last
     * element in the list.
     */
    if (itp->entry == NULL)
	itp->entry = itp->list->last;
    else
	itp->entry = itp->entry->previous;
    data = itp->entry->data;
    return data;
}

static void
llist_iterator_restart(Iterator *it)
{
	LListIterator *listit = (LListIterator *) it;
	listit->entry = listit->list->first;
}

static void
llist_iterator_add(Iterator *it, void *value)
{
	LListIterator *listit = (LListIterator *) it;

	if (listit->entry == NULL) {
		llist_add_last(listit->list, value);
	} else {
		LNode *entry = xmalloc(sizeof(LNode));
		entry->previous = listit->entry->previous;
		entry->next = listit->entry;
		entry->data = value;
		if (entry->next != NULL)
			entry->next->previous = entry;
		else
			listit->list->last = entry;
		if (entry->previous != NULL)
			entry->previous->next = entry;
		else
			listit->list->first = entry;
		listit->list->size++;
		/*listit->entry = entry->next;		-- not necessary */
	}
}
#endif

void
llist_reverse(LList *list)
{
	LNode *head = list->first;
	LNode *tail = list->last;

	while (head != tail && head->previous != tail) {
		void *data = head->data;
		head->data = tail->data;
		tail->data = data;
		head = head->next;
		tail = tail->previous;
	}
}

LNode *
llist_get_first_node(LList *list)
{
	return list->first;
}

LNode *
llist_get_last_node(LList *list)
{
	return list->last;
}

LNode *
lnode_next(LNode *node)
{
	return node->next;
}

LNode *
lnode_previous(LNode *node)
{
	return node->previous;
}

void 
lnode_remove(LList *list, LNode *node)
{
	llist_remove_entry(list, node);
}

LNode *
lnode_add_after(LList *list, LNode *node, void *data)
{
	if (node->next == NULL) {
		llist_add_last(list, data);
		return list->last;
	} else {
		LNode *entry = xmalloc(sizeof(LNode));
		entry->previous = node;
		entry->next = node->next;
		entry->data = data;
		if (entry->next != NULL)
			entry->next->previous = entry;
		else
			list->last = entry;
		if (entry->previous != NULL)
			entry->previous->next = entry;
		else
			list->first = entry;
		list->size++;
		return entry;
	}
}

LNode *
lnode_add_before(LList *list, LNode *node, void *data)
{
	if (node->previous == NULL) {
		llist_add_first(list, data);
		return list->first;
	} else {
		LNode *entry = xmalloc(sizeof(LNode));
		entry->previous = node->previous;
		entry->next = node;
		entry->data = data;
		if (entry->next != NULL)
			entry->next->previous = entry;
		else
			list->last = entry;
		if (entry->previous != NULL)
			entry->previous->next = entry;
		else
			list->first = entry;
		list->size++;
		return entry;
	}
}

bool
lnode_is_first(LNode *node)
{
	return node->previous == NULL;
}

bool
lnode_is_last(LNode *node)
{
	return node->next == NULL;
}

void *
lnode_data(LNode *node)
{
	return node->data;
}
