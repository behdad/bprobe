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

#ifdef  BPROBE
#define BPROBE_MAIN		BPROBE_ATTRIBUTE (unused) static int bprobe_main
#else
#define BPROBE_MAIN		BPROBE_PROBE int main
#endif

#define BPROBE_PID		(bprobe_pid ? bprobe_pid : (bprobe_pid = getpid()))
#define BPROBE_SPAWN_SYNC	BPROBE_SPAWN (1, 0)
#define BPROBE_SPAWN_ASYNC	BPROBE_SPAWN (0, 0)
#define BPROBE_SPAWN_ASYNC_WAIT	BPROBE_SPAWN (0, 1)
#define BPROBE_DUMP_CORE	bprobe_dump_core
#define BPROBE_STACK_TRACE	bprobe_stack_trace
#define BPROBE_ATTACH_DEBUGGER	bprobe_attach_debugger
#define BPROBE_BREAK_POINT	bprobe_break_point

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
	      BPROBE_LOG_FUNC ("bprobe WARNING:\n"			\
			"Looked up symbol '%s' is different\n"		\
			"from enclosing function name '%s'.\n"		\
			"This may not be what you mean.\n",		\
		   _bprobe_func, __PRETTY_FUNCTION__);			\
	    _bprobe_sym_orig = dlsym (RTLD_NEXT, _bprobe_func);		\
	    if (!_bprobe_sym_orig) {					\
	      _bprobe_sym_orig = (_bprobe_type) bprobe_sym_not_found;	\
	      if (bprobe_debug >= 1)					\
	        BPROBE_LOG_FUNC ("bprobe CRITICAL:\n"			\
	      		  "Probe symbol '%s' not found\n"		\
			  "A function returning zero substituted.\n",	\
			  _bprobe_func);				\
	    }								\
	  }								\
	_bprobe_sym_orig;})

#define BPROBE_ATTRIBUTE(attr)						\
 	__attribute__((attr))
#define BPROBE_ATTRIBUTE_UNUSED						\
	BPROBE_ATTRIBUTE (unused)
#define BPROBE_ATTRIBUTE_NORETURN					\
	BPROBE_ATTRIBUTE (noreturn)
#define BPROBE_ATTRIBUTE_PRINTF						\
	BPROBE_ATTRIBUTE (format (printf, 1, 2))

#define BPROBE_UNIQUE_ATTRIBUTED_FUNC(attr, name)			\
	BPROBE_ATTRIBUTE (attr) static void				\
	BPROBE_JOIN(_bprobe_##name##_at_line_,__LINE__) (void)


#define BPROBE_SPAWN(_sync, _wait)					\
	if (bprobe_pid = getpid (), (bprobe_child_pid = fork ()))	\
	  {								\
	    waitpid (bprobe_child_pid, NULL, 0);			\
	    if (_wait)							\
	      wait (NULL);						\
	  }								\
	else if (putenv ((char *) "LD_PRELOAD="),			\
		 (bprobe_child_pid = fork ()))				\
	  {								\
	    if (_sync)							\
	      waitpid (bprobe_child_pid, NULL, 0);			\
	    kill (getpid (), SIGKILL);					\
	  }								\
	else




/* Headers */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
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


/* Probes should only export symbols marked as PROBE */

#pragma GCC visibility push(hidden)


/* Prototypes */

BPROBE_ATTRIBUTE_UNUSED
static int bprobe_pid;

BPROBE_ATTRIBUTE_UNUSED
static int bprobe_child_pid;

BPROBE_ATTRIBUTE_UNUSED
static int bprobe_debug = 1;

BPROBE_ATTRIBUTE_UNUSED
static int bprobe_sym_not_found (void);

BPROBE_ATTRIBUTE_UNUSED
static void bprobe_dump_core (void);
BPROBE_ATTRIBUTE_UNUSED
static void bprobe_stack_trace (void);
BPROBE_ATTRIBUTE_UNUSED
static void bprobe_attach_debugger (void);
BPROBE_ATTRIBUTE_UNUSED
static void bprobe_break_point (void);

BPROBE_ATTRIBUTE_UNUSED
BPROBE_ATTRIBUTE_PRINTF
static void bprobe_log   (const char *fmt, ...);
BPROBE_ATTRIBUTE_UNUSED
BPROBE_ATTRIBUTE_PRINTF
static void bprobe_log_n (const char *fmt, ...);
BPROBE_ATTRIBUTE_UNUSED
BPROBE_ATTRIBUTE_PRINTF
static void bprobe_log_q (const char *fmt, ...);

BPROBE_ATTRIBUTE_UNUSED
BPROBE_ATTRIBUTE_PRINTF
BPROBE_ATTRIBUTE_NORETURN
static void bprobe_die   (const char *fmt, ...);
BPROBE_ATTRIBUTE_UNUSED
BPROBE_ATTRIBUTE_PRINTF
BPROBE_ATTRIBUTE_NORETURN
static void bprobe_die_q (const char *fmt, ...);






/* Function implementations */


#ifndef NULL
#define NULL ((void *)0)
#endif
#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif

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
    BPROBE_LOG ("bprobe ERROR: symbol not found.");
  return 0;
}

static void
bprobe_dump_core (void)
{
  BPROBE_SPAWN_SYNC
    {									
      char pid[10];
      snprintf (pid, sizeof (pid), "%d", BPROBE_PID);
      execlp ("gcore", "bprobe_dump_core", pid, NULL);
      if (bprobe_debug >= 1)
        BPROBE_DIE ("bprobe CRITICAL: running gcore failed.");
    }
}

static void
bprobe_stack_trace (void)
{
  BPROBE_SPAWN_SYNC
    {									
      char pid[10];
      snprintf (pid, sizeof (pid), "%d", BPROBE_PID);
      close (1);
      dup2 (2, 1);
      execlp ("gstack", "bprobe_stack_trace", pid, NULL);
      if (bprobe_debug >= 1)
        BPROBE_DIE ("bprobe CRITICAL: running gstack failed.");
    }
}

static void
bprobe_attach_debugger (void)
{
  BPROBE_SPAWN_ASYNC_WAIT
    {
      char pid[10];
      char exe[64];
      snprintf (pid, sizeof (pid), "%d", BPROBE_PID);
      snprintf (exe, sizeof (exe), "/proc/%s/exe", pid);
      execlp ("gdb", "bprobe_attach_debugger", "-nw", exe, pid, NULL);
      kill (bprobe_pid, SIGCHLD);
      if (bprobe_debug >= 1)
        BPROBE_DIE ("bprobe CRITICAL: running gdb failed.");
    }
}

static void
bprobe_break_point (void)
{
  raise (SIGTRAP);
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
