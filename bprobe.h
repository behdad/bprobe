/* bprobe public header file.
 *
 * Include this file in your probes, after including all public headers.
 *
 * Copyright (C) 2006  Behdad Esfahbod <behdad@gnome.org>

 * bprobe is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * See the GNU Library General Public License for more details.
 */
#ifndef _BPROBE_H
#define _BPROBE_H




/******************
 *    Metadata    *
 ******************/


/* In all metadata variables (except for DEBUG),
 * $VAR variables will be expanded at compile time.
 *
 * Each metadata variable may be repeated more than once.
 * For some, like COMPILER, LANGUAGE, and DEBUG,
 * only the last occurance takes effect.
 */


/* Space-separated list of packages you want to add to both --cflags and --lib */
#define PKG_CONFIG(list)	BPROBE_METADATA (_PKG_CONFIG, list)

/* Space-separated list of packages you want to add to --cflags only.
 * When compiling into an standalone application, this behaves like
 * PKG_CONFIG.
 */
#define PKG_CONFIG_CFLAGS(list)	BPROBE_METADATA (_PKG_CONFIG_CFLAGS, list)

/* Space-separated list of packages you want to add to --libs only */
#define PKG_CONFIG_LIBS(list)	BPROBE_METADATA (_PKG_CONFIG_LIBS, list)


/* Arbitrary compiler flags */
#define COMPILER_FLAGS(list)	BPROBE_METADATA (_COMPILER_FLAGS, list)

/* Compiler to use, may be g++ for C++ probes for example */
#define COMPILER(name)		BPROBE_METADATA (_COMPILER, name)

/* Indicates language that the probe is written in, ie. C, C++ */
#define LANGUAGE(lang)		BPROBE_METADATA (_LANGUAGE, lang)


/* Tune debugging messages reported by bprobe.
 *
 * Zero turns off all messages by bprobe, including
 * critical warning.
 *
 * One is the default, you don't see anything other
 * than messages indicating that you definitely did
 * something wrong. (default)
 *
 * Two also prints out errors that are after-effects
 * of an already reported error.  Like erring every
 * time a missing symbol is called.
 *
 * Three turns on warnings on things that may
 * be an error on your side, but may as well be a 
 * valid but less common usage, like when you call
 * a SUPER(X) in a PROBE Y, where X != Y.
 */
#define DEBUG(status)		BPROBE_DEBUG(status)

/********************
 *  Writing Probes  *
 ********************/




/* Mark a function as a probe.
 * Should come right before the function definition/declaration.
 */
#define PROBE			BPROBE_PROBE

/* Accessor to call (or get) a function that is already
 * prototyped.  The symbol is resolved dynamically and
 * when the expression is evaluated, and it will NOT trigger
 * a run-time linker error.  Instead, if the symbol is
 * not found, a critical warning is printed and the
 * function 'int bprobe_sym_not_found (void)' is
 * returned, which is a function always returning 0.
 * You can compare the returned value of SUPER with this
 * symbol to check whether a failure has happened.
 *
 * This is most useful to call the default handler of a
 * probe, or call from other probes in the same bprobe module
 * when you do not want to trigger its probe.
 */
#define SUPER(func)		BPROBE_SUPER(func)

/* Accessor to call (or get) a function of specified type
 * (or type similar to the object passed as type) and given
 * name as a string.  If type is a function, it should
 * be prefixed by ampersand.  The type is only used to
 * cast the return value to the proper type.
 */
#define FUNC(type, name)	BPROBE_FUNC(type, name)


/* A block of code that should run on program startup.
 * There may be many of them.
 */
#define BEGIN			BPROBE_BEGIN

/* A block of code that should run on normal program termination.
 * There may be many of them.
 */
#define END			BPROBE_END

/* Use this instead of 'int main' if you want to define a
 * main function in your probe.  This function will be
 * ignored when compiling as a probe, but can also be used
 * to compile your probe as an standalone application.
 * For example, you may define a test suite for you probe
 * in this function.
 */
#define MAIN			BPROBE_MAIN


/* The pid of the main process being probed.  Can be
 * used inside spawned blocks too.
 */
#define PID			BPROBE_PID

/* Spawn the code in the following block in a separate
 * process and wait for it to finish.  The code in the
 * block must exit.
 */
#define SPAWN_SYNC		BPROBE_SPAWN_SYNC

/* Spawn the code in the following block in a separate
 * process and continue.  The code in the block must exit.
 */
#define SPAWN_ASYNC		BPROBE_SPAWN_ASYNC

/* Spawn the code in the following block in a separate
 * process and continue after receiving a signal.
 * The code must in the block must exit and must kill the
 * main process (accessible as BPROBE_PID) before exiting.
 */
#define SPAWN_ASYNC_WAIT	BPROBE_SPAWN_ASYNC_WAIT


/* Dump core and continue.
 */
#define DUMP_CORE()		BPROBE_DUMP_CORE ()

/* Dump an stack trace to stderr.
 */
#define STACK_TRACE()		BPROBE_STACK_TRACE ()

/* Attach a debugger.
 */
#define ATTACH_DEBUGGER()	BPROBE_ATTACH_DEBUGGER ()

/* Insert a break point if the process is bing debugged.
 */
#define BREAK_POINT()		BPROBE_BREAK_POINT ()


/* Log a message to stderr, printf style.
 */
#define LOG			BPROBE_LOG

/* Die with a message to stderr, printf style.
 */
#define DIE			BPROBE_DIE

/* Log a message to stderr, printf style.
 * Also prints the function name.
 */
#define LOG_FUNC		BPROBE_LOG_FUNC

/* Die with a message to stderr, printf style.
 * Also prints the function name.
 */
#define DIE_FUNC		BPROBE_DIE_FUNC



/* Make a string out of the expansion of a token.
 */
#define STRINGIZE(x)		BPROBE_STRINGIZE(x)

/* Join two tokens to form one.
 * Works when one of the arguments is __LINE__ too.
 */
#define JOIN(x,y)		BPROBE_JOIN(x)




#include "bprobe-private.h"
#endif /* BPROBE_H */
