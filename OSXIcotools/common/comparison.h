/* Define the comparison_fn_t type from GNU Libc.
 *
 * Copyright (C) 2005 Oskar Liljeblad
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

#ifndef COMMON_COMPARISON_H
#define COMMON_COMPARISON_H

/* This file requires configure.ac additions (in the specified order):
 * AC_GNU_SOURCE
 * AC_CHECK_TYPES([comparison_fn_t])
 */

/* GNU Libc defines comparison_fn_t in stdlib.h if _GNU_SOURCE set. */
#include <stdlib.h>

#ifndef HAVE_COMPARISON_FN_T
typedef int (*comparison_fn_t)(const void *, const void *);
#endif

typedef int (*complex_comparison_fn_t)(const void *, const void *, void *);

#endif
