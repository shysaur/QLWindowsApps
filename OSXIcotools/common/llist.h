/* llist.h - A linked list with a container object (unlike GList)
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

#ifndef COMMON_LLIST_H
#define COMMON_LLIST_H

#include <stdint.h>		/* Gnulib/C99/POSIX */
#include <stdbool.h>		/* Gnulib/C99/POSIX */

typedef struct _LList LList;
typedef struct _LNode LNode;
typedef struct _LListIterator LListIterator;

struct _LListIterator {
    bool (*has_next)(LListIterator *it);
    void *(*next)(LListIterator *it);
    void (*remove)(LListIterator *it);
    void *p0;
    void *p1;
};

LList *llist_new(void);
void llist_free(LList *list);
uint32_t llist_size(LList *list);
bool llist_is_empty(LList *list);
void llist_clear(LList *list);

#define llist_append	llist_add_last
#define llist_add		llist_add_last
#define llist_push		llist_add_last
#define llist_unshift	llist_add_first
void llist_add_at(LList *list, uint32_t index, void *object);
void llist_add_first(LList *list, void *object);
void llist_add_last(LList *list, void *object);
void llist_add_all(LList *list, LList *list2);

#define llist_pop		llist_remove_last
#define llist_shift		llist_remove_first
bool llist_remove(LList *list, void *object);
void *llist_remove_at(LList *list, uint32_t index);
void *llist_remove_first(LList *list);
void *llist_remove_last(LList *list);

#define llist_peek		llist_get_last
void *llist_get(LList *list, uint32_t index);
void *llist_get_first(LList *list);
void *llist_get_last(LList *list);

bool llist_contains(LList *list, void *object);
int32_t llist_index_of(LList *list, void *data);
int32_t llist_last_index_of(LList *list, void *data);
LList *llist_clone(LList *list);
void **llist_to_array(LList *list);
void **llist_to_null_terminated_array(LList *list);

void llist_iterate(LList *list, void (*iterator_func)());
void llist_iterator(LList *list, LListIterator *it);

void llist_reverse(LList *list);

LNode *llist_get_first_node(LList *list);
LNode *llist_get_last_node(LList *list);
LNode *lnode_next(LNode *node);
LNode *lnode_previous(LNode *node);
void lnode_remove(LList *list, LNode *node);
LNode *lnode_add_after(LList *list, LNode *node, void *data);
LNode *lnode_add_before(LList *list, LNode *node, void *data);
bool lnode_is_first(LNode *node);
bool lnode_is_last(LNode *node);
void *lnode_data(LNode *node);

#endif
