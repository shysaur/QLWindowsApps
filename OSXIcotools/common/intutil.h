/* intutil.c - Integer utility functions.
 *
 * Copyright (C) 2001 Oskar Liljeblad
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

#ifndef COMMON_INTUTIL_H
#define COMMON_INTUTIL_H

#include <stdbool.h>	/* Gnulib/C99/POSIX */
#include <stdint.h>	/* Gnulib/C99/POSIX */

char *uint64_str(uint64_t value);
char *uint32_str(uint32_t value);
char *uint16_str(uint16_t value);
char *uint8_str(uint8_t value);
char *int64_str(int64_t value);
char *int32_str(int32_t value);
char *int16_str(int16_t value);
char *int8_str(int8_t value);

bool parse_int8(const char *instr, int8_t *outint);
bool parse_int16(const char *instr, int16_t *outint);
bool parse_int32(const char *instr, int32_t *outint);
bool parse_int64(const char *instr, int64_t *outint);
bool parse_uint8(const char *instr, uint8_t *outint);
bool parse_uint16(const char *instr, uint16_t *outint);
bool parse_uint32(const char *instr, uint32_t *outint);
bool parse_uint64(const char *instr, uint64_t *outint);

#endif
