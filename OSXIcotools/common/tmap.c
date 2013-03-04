/* tmap.c - A red-black tree map implementation.
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
#include <string.h>	/* C89 */
#include <stdlib.h>	/* C89 */
#include "xalloc.h"	/* Gnulib */
#include "tmap.h"

typedef struct _TMapNode TMapNode;
typedef struct _TMapIteratorPriv TMapIteratorPriv;

struct _TMapNode {
    void *key;
    void *value;
    bool red;
    TMapNode *left;
    TMapNode *right;
    TMapNode *parent;
};

struct _TMap {
    TMapNode *root;
    size_t size;
    union {
	comparison_fn_t simple;
	complex_comparison_fn_t complex;
    } comparator;
    bool complex; /* XXX: should really make this more general? */
    void *userdata;
};

struct _TMapIteratorPriv {
    bool (*has_next)(TMapIterator *it);
    void *(*next)(TMapIterator *it);

    TMapNode *n0; /* first node, inclusive */
    TMapNode *n1; /* last node, inclusive */
};

/* In the nil node, any field (including key and value) may be referenced
 * No field may be written, except `red', which may be overwritten with
 * the `false' value (for black).
 */
static TMapNode nil = { NULL, NULL, false, &nil, &nil, &nil };

static int
ptrcmp(const void *v0, const void *v1)
{
    return (int) (v0-v1);
}

static void
tmap_clear_nodes(TMapNode *node)
{
    if (node->left != &nil)
    	tmap_clear_nodes(node->left);
    if (node->right != &nil)
        tmap_clear_nodes(node->right);
    free(node);
}

TMap *
tmap_new(void)
{
    TMap *map = xmalloc(sizeof(TMap));

    map->root = &nil;
    map->size = 0;
    map->comparator.simple = ptrcmp;
    map->complex = false;

    return map;
}

void
tmap_free(TMap *map)
{
    if (map->root != &nil)
	tmap_clear_nodes(map->root);
    free(map);
}

void
tmap_set_compare_fn(TMap *map, comparison_fn_t comparator)
{
    map->comparator.simple = comparator;
    map->complex = false;
}

void
tmap_set_complex_compare_fn(TMap *map, complex_comparison_fn_t comparator, void *userdata)
{
    map->comparator.complex = comparator;
    map->complex = true;
    map->userdata = userdata;
}

size_t
tmap_size(TMap *map)
{
    return map->size;
}

static TMapNode *
tmap_first_node(TMap *map)
{
    TMapNode *node = map->root;
    while (node->left != &nil)
    	node = node->left;
    return node;
}

static TMapNode *
tmap_last_node(TMap *map)
{
    TMapNode *node = map->root;
    while (node->right != &nil)
    	node = node->right;
    return node;
}

static int
tmap_compare(TMap *map, const void *k1, const void *k2)
{
    if (map->complex)
	return map->comparator.complex(k1, k2, map->userdata);
    return map->comparator.simple(k1, k2);
}

static TMapNode *
tmap_get_node(TMap *map, const void *key)
{
    TMapNode *node = map->root;

    while (node != &nil) {
        int compare = tmap_compare(map, key, node->key);
	if (compare > 0) {
	    node = node->right;
	} else if (compare < 0) {
	    node = node->left;
	} else {
	    return node;
        }
    }

    return node;
}

bool
tmap_contains_key(TMap *map, const void *key)
{
    return tmap_get_node(map, key) != &nil;
}

void *
tmap_first_key(TMap *map)
{
    return tmap_first_node(map)->key;
}

void *
tmap_last_key(TMap *map)
{
    return tmap_last_node(map)->key;
}

void *
tmap_first_value(TMap *map)
{
    return tmap_first_node(map)->value;
}

void *
tmap_last_value(TMap *map)
{
    return tmap_last_node(map)->value;
}

void *
tmap_get(TMap *map, const void *key)
{
    return tmap_get_node(map, key)->value;
}

void
tmap_clear(TMap *map)
{
    if (map->root != &nil) {
        tmap_clear_nodes(map->root);
	map->root = &nil;
    }
    map->size = 0;
}

static void
tmap_rotate_left(TMap *map, TMapNode *node)
{
    TMapNode *child = node->right;

    node->right = child->left;
    if (child->left != &nil)
    	child->left->parent = node;

    child->parent = node->parent;
    if (node->parent != &nil) {
    	if (node == node->parent->left)
    	    node->parent->left = child;
	else
	    node->parent->right = child;
    } else {
    	map->root = child;
    }

    child->left = node;
    node->parent = child;
}

static void
tmap_rotate_right(TMap *map, TMapNode *node)
{
    TMapNode *child = node->left;

    node->left = child->right;
    if (child->right != &nil)
	child->right->parent = node;

    child->parent = node->parent;
    if (node->parent != &nil) {
    	if (node == node->parent->right)
    	    node->parent->right = child;
	else
	    node->parent->left = child;
    } else {
    	map->root = child;
    }

    child->right = node;
    node->parent = child;
}

static void
tmap_delete_rebalance(TMap *map, TMapNode *node, TMapNode *parent)
{
    while (node != map->root && !node->red) {
        if (node == parent->left) {
	    TMapNode *sibling = parent->right;
	    if (sibling->red) {
		sibling->red = false;
		parent->red = true;
		tmap_rotate_left(map, parent);
		sibling = parent->right;
            }

            if (!sibling->left->red && !sibling->right->red) {
		sibling->red = true;
		node = parent;
		parent = parent->parent;
            } else {
		if (!sibling->right->red) {
	    	    sibling->left->red = false;
	    	    sibling->red = true;
	    	    tmap_rotate_right(map, sibling);
		    sibling = parent->right;
		}

		sibling->red = parent->red;
		parent->red = false;
		sibling->right->red = false;
		tmap_rotate_left(map, parent);
		node = map->root;
            }
        } else {
            TMapNode *sibling = parent->left;
            if (sibling->red) {
		sibling->red = false;
		parent->red = true;
		tmap_rotate_right(map, parent);
		sibling = parent->left;
            }

            if (!sibling->left->red && !sibling->right->red) {
		sibling->red = true;
		node = parent;
		parent = parent->parent;
            } else {
		if (!sibling->left->red) {
		    sibling->right->red = false;
		    sibling->red = true;
		    tmap_rotate_left(map, sibling);
		    sibling = parent->left;
		}

		sibling->red = parent->red;
		parent->red = false;
		sibling->left->red = false;
		tmap_rotate_right(map, parent);
		node = map->root;
            }
	}
    }

    node->red = false;
}

static void
tmap_remove_node(TMap *map, TMapNode *node)
{
    TMapNode *splice;
    TMapNode *child;
    TMapNode *parent;

    map->size--;

    if (node->left == &nil) {
	splice = node;
	child = node->right;
    } else if (node->right == &nil) {
	splice = node;
	child = node->left;
    } else {
	splice = node->left;
	while (splice->right != &nil)
	    splice = splice->right;
	child = splice->left;
	node->key = splice->key;
	node->value = splice->value;
    }

    parent = splice->parent;
    if (child != &nil)
	child->parent = parent;
    if (parent == &nil) {
        map->root = child;
	free(splice);
        return;
    }
    if (splice == parent->left)
	parent->left = child;
    else
	parent->right = child;

    if (!splice->red)
	tmap_delete_rebalance(map, child, parent);

    free(splice);
}

void *
tmap_remove(TMap *map, const void *key)
{
    TMapNode *node = tmap_get_node(map, key);
    if (node != &nil) {
    	void *value = node->value;
    	tmap_remove_node(map, node);
    	return value;
    }
    return NULL;
}

static void
tmap_insert_rebalance(TMap *map, TMapNode *node)
{
    while (node->parent->red && node->parent->parent != &nil) {
        if (node->parent == node->parent->parent->left) {
	    TMapNode *uncle = node->parent->parent->right;
            if (uncle->red) {
		node->parent->red = false;
		uncle->red = false;
		uncle->parent->red = true;
		node = uncle->parent;
            } else {
		if (node == node->parent->right) {
	    	    node = node->parent;
	    	    tmap_rotate_left(map, node);
		}
		node->parent->red = false;
		node->parent->parent->red = true;
		tmap_rotate_right(map, node->parent->parent);
            }
	} else {
	    TMapNode *uncle = node->parent->parent->left;
	    if (uncle->red) {
		node->parent->red = false;
		uncle->red = false;
		uncle->parent->red = true;
		node = uncle->parent;
            } else {
	    	if (node == node->parent->left) {
	    	    node = node->parent;
	    	    tmap_rotate_right(map, node);
		}
		node->parent->red = false;
		node->parent->parent->red = true;
		tmap_rotate_left(map, node->parent->parent);
            }
	}
    }

    map->root->red = false;
}

void *
tmap_put(TMap *map, void *key, void *value)
{
    TMapNode *node = map->root;
    TMapNode *parent = &nil;
    int compare = 0;

    while (node != &nil) {
    	parent = node;
    	compare = tmap_compare(map, key, node->key);
    	if (compare > 0) {
    	    node = node->right;
	} else if (compare < 0) {
	    node = node->left;
	} else {
	    void *old_value = node->value;
	    node->value = value;
	    return old_value;
	}
    }

    node = xmalloc(sizeof(TMapNode));
    node->key = key;
    node->value = value;
    node->red = true;
    node->parent = parent;
    node->left = &nil;
    node->right = &nil;

    map->size++;
    if (parent == &nil) {
        map->root = node;
        node->red = false;
    	return NULL;
    }
    if (compare > 0) {
    	parent->right = node;
    } else {
    	parent->left = node;
    }

    tmap_insert_rebalance(map, node);
    return NULL;
}

static TMapNode *
successor(TMapNode *node)
{
    TMapNode *parent;

    if (node->right != &nil) {
        node = node->right;
        while (node->left != &nil)
            node = node->left;
        return node;
    }

    parent = node->parent;
    while (node == parent->right) {
        node = parent;
	parent = parent->parent;
    }

    return parent;
}

#if 0
static TMapNode *
predecessor(TMapNode *node)
{
    TMapNode *parent;

    if (node->left != &nil) {
    	node = node->left;
	while (node->right != &nil)
            node = node->right;
	return node;
    }

    parent = node->parent;
    while (node == parent->left) {
    	node = parent;
    	parent = parent->parent;
    }

    return parent;
}
#endif

static void
tmap_foreach_nodes_key(TMapNode *node, void (*iterator)())
{
    if (node->left != &nil)
    	tmap_foreach_nodes_key(node->left, iterator);
    if (node->right != &nil)
        tmap_foreach_nodes_key(node->right, iterator);
    iterator(node->key);
}

static void
tmap_foreach_nodes_value(TMapNode *node, void (*iterator)())
{
    if (node->left != &nil)
    	tmap_foreach_nodes_value(node->left, iterator);
    if (node->right != &nil)
        tmap_foreach_nodes_value(node->right, iterator);
    iterator(node->value);
}

void
tmap_foreach_key(TMap *map, void (*iterator)())
{
    if (map->root != &nil)
	tmap_foreach_nodes_key(map->root, iterator);
}

void
tmap_foreach_value(TMap *map, void (*iterator)())
{
    if (map->root != &nil)
	tmap_foreach_nodes_value(map->root, iterator);
}

static bool
tmap_iterator_has_next(TMapIterator *it)
{
    TMapIteratorPriv *itp = (TMapIteratorPriv *) it;
    return itp->n0 != &nil;
}

static void *
tmap_iterator_next(TMapIterator *it)
{
    TMapIteratorPriv *itp = (TMapIteratorPriv *) it;
    void *data;

    if (itp->n0 == &nil)
	return NULL;
    data = itp->n0->value;
    itp->n0 = (itp->n0 == itp->n1 ? &nil : successor(itp->n0));
    return data;
}

void
tmap_iterator(TMap *map, TMapIterator *it)
{
    TMapIteratorPriv *itp = (TMapIteratorPriv *) it;

    itp->n0 = tmap_first_node(map);
    itp->n1 = &nil;
    itp->has_next = tmap_iterator_has_next;
    itp->next = tmap_iterator_next;
}

#ifdef ENABLE_TMAP_TESTING
#include <stdio.h>
static bool
tmap_node_verify(TMapNode *node, int *expect)
{
    if (node == &nil)
	return true;
    if (node->red) {
	if (node->left->red || node->right->red)
	    return false;
    }
    if (node->left == &nil && node->right == &nil) {
	int count = 0;
	TMapNode *x;
	for (x = node; x != &nil; x = x->parent) {
	    if (!x->red)
		count++;
	}
	if (*expect == -1) {
	    *expect = count;
	} else if (count != *expect) {
	    return false;
	}
    }
    return tmap_node_verify(node->left, expect) && tmap_node_verify(node->right, expect);
}

bool
tmap_verify(TMap *map)
{
    int expect = -1;
    return tmap_node_verify(map->root, &expect);
}


static void
tmap_dump_node(TMapNode *n, FILE *out, int i)
{
    int c;

    if (n == &nil)
	return;

    for (c=0;c<i;c++)
	fprintf(out, " ");
    fprintf(out, "N=(%s) %s\n", n->red ? "red" : "black", (char *) n->value);

    if (n->left != &nil) {
	for (c=0;c<i;c++)
	    fprintf(out, " ");
	fprintf(out, "L:\n");
	tmap_dump_node(n->left, out, i+1);
    }

    if (n->right != &nil) {
	for (c=0;c<i;c++)
	    fprintf(out, " ");
	fprintf(out, "R:\n");
	tmap_dump_node(n->right, out, i+1);
    }
}

void
tmap_dump(TMap *map, FILE *out)
{
    printf("VERIFIED: %d\n", tmap_verify(map));
    tmap_dump_node(map->root, out, 0);
}

#endif

bool
tmap_iterator_partial(TMap *map, TMapIterator *it, const void *match, comparison_fn_t comparator)
{
    TMapNode *n = map->root;
    TMapIteratorPriv *itp = (TMapIteratorPriv *) it;

    itp->has_next = tmap_iterator_has_next;
    itp->next = tmap_iterator_next;

    while (n != &nil) {
	int compare = comparator(match, n->key);
	if (compare > 0) {
	    n = n->right;
	} else if (compare < 0) {
	    n = n->left;
	} else {
	    TMapNode *nx;
	    TMapNode *nn;

	    nx = n;
	    for (nn = n->left; nn != &nil; ) {
		if (comparator(match, nn->key) == 0) {
		    nx = nn;
		    nn = nn->left;
		} else {
		    nn = nn->right;
		}
	    }
	    itp->n0 = nx;

	    nx = n;
	    for (nn = n->right; nn != &nil; ) {
		if (comparator(match, nn->key) == 0) {
		    nx = nn;
		    nn = nn->right;
		} else {
		    nn = nn->left;
		}
	    }
	    itp->n1 = nx;

	    //printf("match=[%s] n=[%s] n0=[%s] n1=[%s]\n", match, n->value, itp->n0->value, itp->n1->value);
	    
	    return true;
	}
    }

    itp->n0 = &nil;
    itp->n1 = &nil;

    return false;
}
