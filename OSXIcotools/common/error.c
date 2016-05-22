/* error.c - Error-management and messaging routines.
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
#include <errno.h>	/* C89 */
#include <string.h>	/* Gnulib/C89 */
#include <stdarg.h>	/* Gnulib/C89 */
#include <stdlib.h>	/* Gnulib/C89 */
#include <stdio.h>	/* Gnulib/C89 */
#include "gettext.h"	/* Gnulib/Gettext */
#define _(s) gettext(s)
#include "xvasprintf.h"	/* Gnulib */
#include "xalloc.h"	/* Gnulib */
#include "error.h"


static inline const char *
get_message_header(void)
{
	return "OSXIcotools";
}

static void
v_warn(const char *msg, va_list ap)
{
	fprintf(stderr, "%s: ", get_message_header());
	if (msg != NULL)
		vfprintf(stderr, msg, ap);
	fprintf(stderr, "\n");
}

static void
v_warn_errno(const char *msg, va_list ap)
{
	fprintf(stderr, "%s: ", get_message_header());
	if (msg != NULL) {
		vfprintf(stderr, msg, ap);
		fprintf(stderr, ": ");
	}
	fprintf(stderr, "%s\n", strerror(errno));
}

/**
 * This function should be called when an internal error has
 * occured. It will display a more verbose message, asking
 * the user to mail the program author.
 *
 * @param msg
 *   Error message.
 */
void
internal_error(const char *msg, ...)
{
	va_list ap;

	va_start(ap, msg);
	fprintf(stderr, _("\
An internal error has occured. Please report this error by sending the\n\
output below to %s.\n\
\n\
Program: %s\n\
Version: %s\n\
Error: "), PACKAGE_BUGREPORT, "OSXIcotools", VERSION);
	vfprintf(stderr, msg, ap);
	va_end(ap);

	exit(1);
}

/**
 * Terminate the program with an error message.
 *
 * @param msg
 *   Error message.
 */
void
die(const char *msg, ...)
{
	va_list ap;
	
	va_start(ap, msg);
	v_warn(msg, ap);
	va_end(ap);

	exit(1);
}

/**
 * Terminate the program with an error message and
 * the current error in `errno'.
 *
 * @param msg
 *   Error message, or NULL if only the error in `errno'
 *   should be printed.
 */
void
die_errno(const char *msg, ...)
{
	va_list ap;
	
	va_start(ap, msg);
	v_warn_errno(msg, ap);
	va_end(ap);

	exit(1);
}

/**
 * Write a warning message to standard error.
 *
 * @param msg
 *   The message.
 */
void
warn(const char *msg, ...)
{
	va_list ap;
	
	va_start(ap, msg);
	v_warn(msg, ap);
	va_end(ap);
}


/**
 * Write an error message and the current error in `errno'.
 *
 * @param msg
 *   Error message, or NULL if only the error in `errno'
 *   should be printed.
 */
void
warn_errno(const char *msg, ...)
{
	va_list ap;
	
	va_start(ap, msg);
	v_warn_errno(msg, ap);
	va_end(ap);
}

