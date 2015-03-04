/**
 * @file nss-dbng.h
 * @brief Generic library macros.
 * @author Mikey Austin
 * @date 2015
 */

#ifndef NSS_DB_NG_H
#define NSS_DB_NG_H

#define _GNU_SOURCE

#ifdef HAVE_CONFIG_H
#include <config.h>
#else
#error You must use autotools to build this!
#endif

#include <nss.h>
#include <syslog.h>
#include <stdio.h>

/* Some syslog shortcuts */
#ifdef DEBUG
#  define NSS_DEBUG(msg, ...) syslog(LOG_DEBUG, (msg), ## __VA_ARGS__)
#else
#  define NSS_DEBUG(msg, ...)
#endif

#define NSS_ERROR(msg, ...) syslog(LOG_ERR, (msg), ## __VA_ARGS__)

#define DBNG_PASSWD     "passwd.db"
#define DBNG_PASSWD_UID "passwd_uid.db"
#define DBNG_SHADOW     "shadow.db"
#define DBNG_SHADOW_UID "shadow_uid.db"
#define DBNG_GROUP      "group.db"
#define DBNG_GROUP_UID  "group_uid.db"

#endif
