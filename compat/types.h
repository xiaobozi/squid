/*
 * Copyright (C) 1996-2014 The Squid Software Foundation and contributors
 *
 * Squid software is distributed under GPLv2+ license and includes
 * contributions from numerous individuals and organizations.
 * Please see the COPYING and CONTRIBUTORS files for details.
 */

#ifndef SQUID_TYPES_H
#define SQUID_TYPES_H

/*
 * Here are defined several known-width types, obtained via autoconf
 * from system locations or various attempts. This is just a convenience
 * header to include which takes care of proper preprocessor stuff
 *
 * This file is only intended to be included via compat/compat.h, do
 * not include directly.
 */

/* This should be in synch with what we have in acinclude.m4 */
#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#if HAVE_LINUX_TYPES_H
#include <linux/types.h>
#endif
#if HAVE_STDLIB_H
#include <stdlib.h>
#endif
#if HAVE_STDDEF_H
#include <stddef.h>
#endif
#if HAVE_INTTYPES_H
#include <inttypes.h>
#endif
#if HAVE_SYS_BITYPES_H
#include <sys/bitypes.h>
#endif
#if HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif
#if HAVE_NETINET_IN_SYSTM_H
/* Several OS require types declared by in_systm.h without including it themselves. */
#include <netinet/in_systm.h>
#endif

/******************************************************/
/* Typedefs for missing entries on a system           */
/******************************************************/

/*
 * ISO C99 Standard printf() macros for 64 bit integers
 * On some 64 bit platform, HP Tru64 is one, for printf must be used
 * "%lx" instead of "%llx"
 */
#ifndef PRId64
#if _SQUID_WINDOWS_
#define PRId64 "I64d"
#elif SIZEOF_INT64_T > SIZEOF_LONG
#define PRId64 "lld"
#else
#define PRId64 "ld"
#endif
#endif

#ifndef PRIu64
#if _SQUID_WINDOWS_
#define PRIu64 "I64u"
#elif SIZEOF_INT64_T > SIZEOF_LONG
#define PRIu64 "llu"
#else
#define PRIu64 "lu"
#endif
#endif

#ifndef PRIX64
#if _SQUID_WINDOWS_
#define PRIX64 "I64X"
#elif SIZEOF_INT64_T > SIZEOF_LONG
#define PRIX64 "llX"
#else
#define PRIX64 "lX"
#endif
#endif

#ifndef PRIuSIZE
// NP: configure checks for support of %zu and defines where possible
#if SIZEOF_SIZE_T == 4 && _SQUID_MINGW_
#define PRIuSIZE "I32u"
#elif SIZEOF_SIZE_T == 4
#define PRIuSIZE "u"
#elif SIZEOF_SIZE_T == 8 && _SQUID_MINGW_
#define PRIuSIZE "I64u"
#elif SIZEOF_SIZE_T == 8
#define PRIuSIZE "lu"
#else
#error size_t is not 32-bit or 64-bit
#endif
#endif /* PRIuSIZE */

#ifndef HAVE_MODE_T
typedef unsigned short mode_t;
#endif

#ifndef HAVE_FD_MASK
typedef unsigned long fd_mask;
#endif

#ifndef HAVE_SOCKLEN_T
typedef int socklen_t;
#endif

#ifndef HAVE_MTYP_T
typedef long mtyp_t;
#endif

#ifndef NULL
#if defined(__cplusplus) && HAVE_NULLPTR
#define NULL nullptr
#else
#define NULL 0
#endif
#endif

#endif /* SQUID_TYPES_H */
