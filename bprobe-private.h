/* bprobe private header file.
 *
 * Copyright (C) 2006  Behdad Esfahbod <behdad@gnome.org>

 * bprobe is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * See the GNU Library General Public License for more details.
 */
#ifndef _BPROBE_PRIVATE_H
#define _BPROBE_PRIVATE_H

/* The public interface */

#define BPROBE_DEBUG(status)	BEGIN { bprobe_debug = (status); }

#define BPROBE_PROBE		BPROBE_ATTRIBUTE (visibility("default"))
#define BPROBE_SUPER(func)	BPROBE_FUNC_GET(&func, BPROBE_STRINGIZE(func), 1)
#define BPROBE_FUNC(type, name)	BPROBE_FUNC_GET(type, name, 0)

#define BPROBE_BEGIN		BPROBE_UNIQUE_ATTRIBUTED_FUNC (constructor, begin)
#define BPROBE_END		BPROBE_UNIQUE_ATTRIBUTED_FUNC (destructor,  end)
#ifdef BPROBE
#define BPROBE_MAIN		BPROBE_ATTRIBUTE (unused) static int bprobe_main
#else
#define BPROBE_MAIN		BPROBE_PROBE int main
#endif

#define BPROBE_LOG		bprobe_log
#define BPROBE_DIE		bprobe_die
#define BPROBE_LOG_FUNC		bprobe_log_n ("(%s) ", __func__), bprobe_log_q
#define BPROBE_DIE_FUNC		bprobe_log_n ("(%s) ", __func__), bprobe_die_q

#define BPROBE_STRINGIZE(x)	BPROBE_STRINGIZE0(x)
#define BPROBE_JOIN(x,y)	BPROBE_JOIN0(x, y)


/* Internal macro voodoo */

#define BPROBE_STRINGIZE0(x)	#x
#define BPROBE_JOIN0(x,y)	x##y

#define BPROBE_FUNC_GET(prototype, symstring, err)			\
	({								\
	  typedef __typeof__((prototype)) _bprobe_type;			\
	  static _bprobe_type _bprobe_sym_orig = NULL;			\
	  const char _bprobe_func[] = (symstring);			\
	  if (!_bprobe_sym_orig) {					\
	    if (err && bprobe_debug >= 3 &&				\
	        strcmp (__func__, _bprobe_func))			\
	      LOG_FUNC ("bprobe WARNING:\n"				\
			"Looked up symbol '%s' is different\n"		\
			"from enclosing function name '%s'.\n"		\
			"This may not be what you mean.\n",		\
		   _bprobe_func, __PRETTY_FUNCTION__);			\
	    _bprobe_sym_orig = dlsym (RTLD_NEXT, _bprobe_func);		\
	    if (!_bprobe_sym_orig) {					\
	      _bprobe_sym_orig = (_bprobe_type) bprobe_sym_not_found;	\
	      if (bprobe_debug >= 1)					\
	        LOG_FUNC ("bprobe CRITICAL:\n"				\
	      		  "Probe symbol '%s' not found\n"		\
			  "A function returning zero substituted.\n",	\
			  _bprobe_func);				\
	    }								\
	  }								\
	_bprobe_sym_orig;})

#define BPROBE_ATTRIBUTE(attr)						\
 	__attribute__((attr))

#define BPROBE_UNIQUE_ATTRIBUTED_FUNC(attr, name)			\
	BPROBE_ATTRIBUTE (attr) static void				\
	BPROBE_JOIN(_bprobe_##name##_at_line_,__LINE__) (void)



/* Probes should only export symbols marked as PROBE */

#pragma GCC visibility push(hidden)



/* Headers */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
/* Has got to work a bit hard to get RTLD_NEXT defined, as
 * user headers may already have imported dlfcn.h,
 * But it seems to be harmless so far...
 */
#undef _GNU_SOURCE
#define _GNU_SOURCE 1
#undef __USE_GNU
#define __USE_GNU 1
#undef _DLFCN_H
#include <dlfcn.h>


/* Prototypes */

BPROBE_ATTRIBUTE (unused)
static int bprobe_debug = 1;

BPROBE_ATTRIBUTE (unused)
static int bprobe_sym_not_found(void);

BPROBE_ATTRIBUTE (unused)
static void bprobe_log   (const char *fmt, ...);
BPROBE_ATTRIBUTE (unused)
static void bprobe_log_n (const char *fmt, ...);
BPROBE_ATTRIBUTE (unused)
static void bprobe_log_q (const char *fmt, ...);

BPROBE_ATTRIBUTE (unused)
BPROBE_ATTRIBUTE (noreturn)
static void bprobe_die   (const char *fmt, ...);
BPROBE_ATTRIBUTE (unused)
BPROBE_ATTRIBUTE (noreturn)
static void bprobe_die_q (const char *fmt, ...);






/* Function implementations */


#include <stdarg.h>

#define BPROBE_ARGLIST		va_list ap
#define BPROBE_PROMPT		fprintf (stderr, "BP| ")
#define BPROBE_PRINTF		va_start (ap, fmt); vfprintf (stderr, fmt, ap)
#define BPROBE_NEWLINE		fprintf (stderr, "\n")
#define BPROBE_EXIT		exit (125)
static void bprobe_log   (const char *fmt, ...) {
  BPROBE_ARGLIST; BPROBE_PROMPT; BPROBE_PRINTF; BPROBE_NEWLINE;
}
static void bprobe_log_n (const char *fmt, ...) {
  BPROBE_ARGLIST; BPROBE_PROMPT; BPROBE_PRINTF;
}
static void bprobe_log_q (const char *fmt, ...) {
  BPROBE_ARGLIST; BPROBE_PRINTF; BPROBE_NEWLINE;
}

static void bprobe_die   (const char *fmt, ...) {
  BPROBE_ARGLIST; BPROBE_PROMPT; BPROBE_PRINTF; BPROBE_NEWLINE; BPROBE_EXIT;
}
static void bprobe_die_q (const char *fmt, ...) {
  BPROBE_ARGLIST; BPROBE_PRINTF; BPROBE_NEWLINE; BPROBE_EXIT;
}
#undef BPROBE_ARGLIST
#undef BPROBE_PROMPT
#undef BPROBE_PRINTF
#undef BPROBE_NEWLINE
#undef BPROBE_EXIT

static int
bprobe_sym_not_found (void)
{
  if (bprobe_debug >= 2)
    LOG ("bprobe ERROR: symbol not found.");
  return 0;
}





/* Metadata gathering stuff */
#ifndef BPROBE_GATHER_METADATA


/* When compiling, ignore metadata.
 */
#define BPROBE_METADATA(token, value)

/* We really really really require gcc when compiling...
 */
#ifndef __GNUC__
#error You need gcc to run bprobe
#endif


#else /* BPROBE_GATHER_METADATA */


/* Export metadata sandwitched in magic tokens, to extract them
 * from the preprocessor output.
 */
#define BPROBE_MAGIC_BEGIN(token)	{{token{X
#define BPROBE_MAGIC_END(token)		X}token}}
#define BPROBE_METADATA(token, value)	BPROBE_MAGIC_BEGIN(token){value}BPROBE_MAGIC_END(token)

#ifndef BPROBE
#define _PKG_CONFIG_CFLAGS		_PKG_CONFIG
#endif

/* A symbol we check in the output to make sure preprocessor did
 * its job...
 */
BPROBE_JOIN (BPROBE_MA, GIC_TOKEN)


#endif /* BPROBE_GATHER_METADATA */


#endif /* BPROBE_PRIVATE_H */
