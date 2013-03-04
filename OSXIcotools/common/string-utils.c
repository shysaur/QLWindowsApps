/* string-utils.c - Functions dealing with string contents.
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
#include <ctype.h>		/* C89 */
#include <stdarg.h>		/* Gnulib/C89 */
#include <stdio.h>		/* Gnulib/C89 */
#include <string.h>		/* Gnulib/C89 */
#include <stdlib.h>		/* Gnulib/C89 */
#include "xalloc.h"		/* Gnulib */
#include "xvasprintf.h"		/* Gnulib */
#include "common.h"		/* common */
#include "error.h"		/* common */
#include "string-utils.h"	/* common */

/**
 * Return a zero-based of a character in a string.
 */
/*inline*/ int
strindex(const char *str, char ch)
{
	char *pos = strchr(str, ch);
	return (pos == NULL ? -1 : pos - str);
}

/**
 * Remove trailing newline from a character, if
 * the last character is a newline.
 */
void
chomp(char *str)
{
	int len;

	len = strlen(str);
	if (len > 0 && str[len-1] == '\n')
		str[len-1] = '\0';
}

/**
 * Strip leading characters from a string, modifying the
 * string. What characters to strip are determined by the
 * function, which can be an isalpha, isalnum etc.
 *
 * @param line
 *   Line to strip characters from. This string will be modified.
 * @param check
 *   A function to be called for each character. If it returns
 *   a non-zero value, the character will be stripped.
 */
void
strip_leading(char *line, int (*check)(int))
{
	int start;
	int end;

	if (line[0] != '\0' && check(line[0])) {
		for (start = 1; line[start] != '\0' && check(line[start]); start++);
		for (end = start; line[end] != '\0'; end++);
		memmove(line, line+start, end-start+1);
	}
}

/**
 * Return the index of the first character of some type.
 * For this purpose a check function is used.
 *
 * @param line
 *   String to look for characters in.
 * @param check 
 *   A function to be called for each character. If it returns
 *   a non-zero value, we has found a character we were looking
 *   for, and char_index returns with the index of this character.
 */
int
char_index(const char *line, int (*check)(int))
{
	int c;
	for (c = 0; line[c] != '\0'; c++) {
		if (check(line[c]))
			return c;
	}
	return -1;
}

/**
 * Return true if the first string ends with the second.
 *
 * @param str
 *   String to look for end in.
 * @param end
 *   String to look for.
 */
bool
ends_with(const char *str, const char *end)
{
	int c;
	int diff = strlen(str) - strlen(end);

	if (diff < 0)
		return false;

	for (c = 0; end[c] != '\0'; c++) {
		if (end[c] != str[c+diff])
			return false;
	}

	return true;
}

/**
 * Return true if the first string ends with the second,
 * ignoring case.
 *
 * @param str
 *   String to look for end in.
 * @param end
 *   String to look for.
 */
bool
ends_with_nocase(const char *str, const char *end)
{
	int c;
	int diff = strlen(str) - strlen(end);

	if (diff < 0)
		return false;

	for (c = 0; end[c] != '\0'; c++) {
		if (tolower(end[c]) != tolower(str[c+diff]))
			return false;
	}

	return true;
}

/**
 * Return true if the first string starts with the second.
 *
 * @param str
 *   String to look for start in.
 * @param start
 *   String to look for.
 */
bool
starts_with(const char *str, const char *start)
{
	int c;
	for (c = 0; str[c] != '\0'; c++) {
		if (str[c] != start[c])
			break;
	}
	return (start[c] == '\0');
}

/**
 * Return true if the first string starts with the second,
 * ignoring case.
 *
 * @param str
 *   String to look for start in.
 * @param start
 *   String to look for.
 */
bool
starts_with_nocase(const char *str, const char *start)
{
	int c;
	for (c = 0; str[c] != '\0'; c++) {
		if (tolower(str[c]) != tolower(start[c]))
			break;
	}
	return (start[c] == '\0');
}

/**
 * Translate each occurence of `from' in `str' to `to'.
 *
 * @returns
 *   Number of changes made.
 */
int
translate_char(char *str, char from, char to)
{
	int changes = 0;
	int c;
	for (c = 0; str[c] != '\0'; c++) {
		if (str[c] == from) {
			str[c] = to;
			changes++;
		}
	}
	return changes;
}

/**
 * Convert all characters in the specified string using
 * some character-conversion function.
 */
void
str_convert(char *str, int (*convert)(int))
{
	int c;
	for (c = 0; str[c] != '\0'; c++)
		str[c] = convert(str[c]);
}

/**
 * Return true if the specified character is a Perl "word"
 * character (`\w'), that is: if it is an alphanumeric character
 * or `_'.
 */
int
iswordchar(int ch)
{
	return isalnum(ch) || ch == '_';
}

/**
 * Replace an occurence of `from' in `str' with `to'.
 * If `from' is longer than `to', then `str' must contain
 * enough space for the new string.
 *
 * Note: This implementation is slow and not optimized.
 *
 * @returns
 *   true if a replacement was made.
 */
bool
replace_str(char *str, const char *from, const char *to)
{
	char *pos;
	int s1;
	int s2;
	
	if ((pos = strstr(str, from)) == NULL)
		return false;

	s1 = strlen(from);
	s2 = strlen(to);
	memmove(pos+s2, pos+s1, strlen(pos)-s2+1);
	memcpy(pos, to, s2);
	return true;	
}

/**
 * Concatenate two file names into a new string.
 * If file is "." return a duplicate of file2.
 */
char *
cat_files(const char *file, const char *file2)
{
	if (strcmp(file, ".") == 0)
		return xstrdup(file2);
	return xasprintf("%s%s%s", file, (ends_with(file, "/") ? "" : "/"), file2);
}

/**
 * Return the substring of a string, putting it into a newly allocated
 * string.
 *
 * @param start
 *   Index of first character to copy. If this value is
 *   negative, the index is counted from the end of the string.
 *   E.g. -1 is before last non-null character.
 * @param end
 *   Index of first character not to copy. If this value is
 *   negative, the index is counted from the end of the string.
 *   E.g. -1 is before last non-null character.
 */
char *
substring(const char *buf, int start, int end)
{
	int len = strlen(buf);

	if (start < 0)
		start += len;
	if (end < 0)
		end += len;

	return strndup(buf+start, end-start);
}

/**
 * Strip all trailing characters listed in `stripchars'.
 *
 * @param str
 *   String to remove characters from. This string will be
 *   modified.
 */
int
string_strip_trailing(char *str, const char *stripchars)
{
	int a = strlen(str)-1;
	int b = a;

	while (a >= 0 && string_index_of_c(stripchars, str[a]) != -1)
		a--;
	str[a+1] = '\0';

	return b-a;
}

/**
 * Strip all trailing characters that are `stripchar'.
 *
 * @param str
 *   String to remove characters from. This string will be
 *   modified.
 */
int
string_strip_trailing_c(char *str, char stripchar)
{
	int a = strlen(str-1);
	int b = a;

	while (a >= 0 && str[a] == stripchar)
		a--;
	str[a+1] = '\0';

	return b-a;
}

/* XXX: document */
int
string_strip_leading(char *str, const char *stripchars)
{
	int a = 0;
	int b = 0;

	while (str[a] != '\0' && string_index_of_c(stripchars, str[a]) != -1)
		a++;
	while (str[a] != '\0')
		str[b++] = str[a++];
	str[b] = str[a];

	return a-b;
}

/* XXX: document */
int
string_strip_leading_c(char *str, char stripchar)
{
	int a = 0;
	int b = 0;

	while (str[a] != '\0' && str[a] == stripchar)
		a++;
	while (str[a] != '\0')
		str[b++] = str[a++];
	str[b] = str[a];

	return a-b;
}

/**
 * Return the index of the first character listed in `findchars'
 * in `str'.
 *
 * @param str
 *   The string to look for characters in.
 * @param findchars
 *   The characters to look for.
 */
int
string_index_of_any(const char *str, const char *findchars)
{
	int c;
	for (c = 0; str[c] != '\0'; c++) {
		if (string_index_of_c(findchars, str[c]) != -1)
			return c;
	}
	return -1;
}

int
word_get_index(const char *str, int pos)
{
	bool in_word = false;
	int words = 0;
	int c;

	for (c = 0; str[c] != '\0' && c < pos; c++) {
		if (!in_word && !isspace(str[c])) {
			in_word = true;
		}
		if (in_word && isspace(str[c])) {
			in_word = false;
			words++;
		}
	}

	return words;
}

char *
word_get(const char *str, int idx)
{
	bool in_word = false;
	int words = 0;
	int start = -1;
	int c;

	for (c = 0; str[c] != '\0'; c++) {
		if (!in_word && !isspace(str[c])) {
			words++;
			start = c;
			in_word = true;
		}
		if (in_word && isspace(str[c])) {
			if (words > idx)
				return substring(str, start, c);
			in_word = false;
		}
	}

	if (words-1 == idx)
		return substring(str, start, c);
	return NULL;
}

int
uintlen(uint64_t value)
{
	int len;
	for (len = 1; value >= 10; len++)
		value /= 10;
	return len;
}

/* Like dirname, but returns a newly allocated string.
 */
char *
xdirname(const char *path)
{
	char *pos = strrchr(path, '/');

	if (pos == NULL)
		return xstrdup(".");

	if (pos == path)
		return xstrdup("/");

	return substring(path, 0, pos-path);
}
