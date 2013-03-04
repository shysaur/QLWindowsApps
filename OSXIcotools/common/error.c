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
#include "progname.h"	/* Gnulib */
#include "error.h"

struct MessageHeader {
	struct MessageHeader *old;
	char *message;
};

void (*program_termination_hook)(void) = NULL;
static char *error_message = NULL;
static struct MessageHeader *message_header = NULL;

static inline const char *
get_message_header(void)
{
	if (message_header != NULL)
		return message_header->message;
	return program_name;
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
 * Free all global memory allocated by the error facilities
 * provided here.
 */
void
free_error(void)
{
	struct MessageHeader *hdr;

	for (hdr = message_header; hdr != NULL; hdr = hdr->old) {
		free(hdr->message);
		free(hdr);
	}
	if (error_message != NULL)
		free(error_message);
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
	if (program_termination_hook != NULL)
		program_termination_hook();
	fprintf(stderr, _("\
An internal error has occured. Please report this error by sending the\n\
output below to %s.\n\
\n\
Program: %s\n\
Version: %s\n\
Error: "), PACKAGE_BUGREPORT, program_name, VERSION);
	vfprintf(stderr, msg, ap);
	va_end(ap);

	free_error();
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
	if (program_termination_hook != NULL)
		program_termination_hook();
	v_warn(msg, ap);
	va_end(ap);

	free_error();
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
	if (program_termination_hook != NULL)
		program_termination_hook();
	v_warn_errno(msg, ap);
	va_end(ap);

	free_error();
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

/**
 * Set the current message header.
 */
void
set_message_header(const char *msg, ...)
{
	va_list ap;
	struct MessageHeader *hdr;

	hdr = malloc(sizeof(struct MessageHeader));
	if (hdr == NULL)
		xalloc_die();
	hdr->old = message_header;
	va_start(ap, msg);
	if (vasprintf(&hdr->message, msg, ap) < 0)
		xalloc_die();
	message_header = hdr;
	va_end(ap);
}

/**
 * Restore the message header to the default.
 */
void
restore_message_header(void)
{
	if (message_header != NULL) {
		struct MessageHeader *old;
		old = message_header->old;
		free(message_header->message);
		free(message_header);
		message_header = old;
	}
}

/**
 * Set a global error message.
 */
void
set_error(const char *format, ...)
{
	va_list ap;

	if (error_message != NULL)
		free(error_message);

	if (format != NULL) {
		va_start(ap, format);
		if (vasprintf(&error_message, format, ap) < 0)
			xalloc_die();
		va_end(ap);
	} else {
		error_message = NULL;
	}
}

/**
 * Return the global error message.
 * The returned value cannot be modified and should never
 * be freed.
 */
const char *
get_error(void)
{
	return error_message;
}

/**
 * Remove and return the global error message.
 * The returned value may be modified and should
 * be freed when no longer needed.
 */
char *
remove_error(void)
{
	char *msg = error_message;
	error_message = NULL;
	return msg;
}

/**
 * Die printing the current error message.
 */
void
die_error(void)
{
	if (program_termination_hook != NULL)
		program_termination_hook();
	warn(error_message);
	free_error();
	exit(1);
}
