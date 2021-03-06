/*
 * Copyright (C) 1996-2014 The Squid Software Foundation and contributors
 *
 * Squid software is distributed under GPLv2+ license and includes
 * contributions from numerous individuals and organizations.
 * Please see the COPYING and CONTRIBUTORS files for details.
 */

#ifndef _SQUID_HELPERS_BASIC_AUTH_MSNT_MSNTAUTH_H
#define _SQUID_HELPERS_BASIC_AUTH_MSNT_MSNTAUTH_H

extern int OpenConfigFile(void);
extern int QueryServers(char *, char *);
extern void Checktimer(void);
extern "C" void Check_forchange(int);
extern int Read_denyusers(void);
extern int Read_allowusers(void);
extern int Check_user(char *);
extern int QueryServers(char *, char *);
extern int Check_ifuserallowed(char *ConnectingUser);
extern void Check_forallowchange(void);

#endif /* _SQUID_HELPERS_BASIC_AUTH_MSNT_MSNTAUTH_H */
