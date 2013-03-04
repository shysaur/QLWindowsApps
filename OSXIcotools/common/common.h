/* common.h - Common definitions.
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

#ifndef COMMON_COMMON_H
#define COMMON_COMMON_H

#define SWAP(a,b)	({ typeof(a) t = a; a = b; b = t; })

#ifndef min
#define min(a,b)	({ typeof(a) c = a; typeof(b) d = b; MIN(c,d); })
#endif

#ifndef max
#define max(a,b)	({ typeof(a) c = a; typeof(b) d = b; MAX(c,d); })
#endif

#define RETURN_IF_DIFFERENT(i1,i2) \
	{ \
		typeof(i1) a = i1; \
		typeof(i2) b = i2; \
		if (a != b) \
			return a-b; \
	}

#endif
