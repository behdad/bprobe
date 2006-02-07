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

/* Space-separated list of packages you want to add to --cflags only */
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
 * Zero turns off all messages by bprobe, including
 * critical warning.
 * One is the normal, you don't see anything other
 * than messages indicating that you definitely did
 * something wrong. (default)
 * Two also prints out warnings on things that may
 * be an error on your side, but may as well be a 
 * valid but less common usage.
 */
#define DEBUG(status)		BPROBE_DEBUG(status)

/********************
 *  Writing Probes  *
 ********************/




/* Mark a function as a probe.
 * Should come just before the function return type.
 */
#define PROBE			BPROBE_PROBE

/* Accessor to call the default handler of a probe.
 * Means, a function not defined in the probe file.
 */
#define SUPER(func)		BPROBE_SUPER(func)

/* Accessor to get a function of specified type (or
 * type like the object passed as type) and given
 * name.  If type is a function, it should be prefixed
 * by ampersand.
 */
#define FUNC(type, name)	BPROBE_FUNC(type, name)


/* A block of code that should run on program startup.
 * There may be many of them.
 */
#define BEGIN			BPROBE_BEGIN

/* A block of code that should run on program shutdown.
 * There may be many of them.
 */
#define END			BPROBE_END



/* Log a message to stderr, printf style
 */
#define LOG			BPROBE_LOG

/* Die with a message to stderr, printf style
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



/* Make a string out of the expantion of a token.
 */
#define STRINGIZE(x)		BPROBE_STRINGIZE(x)

/* Joins two tokens to form one token.
 * Works when one of the arguments is __LINE__ too.
 */
#define JOIN(x,y)		BPROBE_JOIN(x)




#include "bprobe-private.h"
#endif /* BPROBE_H */
