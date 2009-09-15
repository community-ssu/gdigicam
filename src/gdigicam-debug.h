/*
 * This file is part of GDigicam
 *
 * Copyright (C) 2008-2009 Nokia Corporation and/or its subsidiary(-ies).
 *
 * Contact: Alexander Bokovoy <alexander.bokovoy@nokia.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * version 2.1 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 */


/**
 * SECTION:gdigicam-debug
 * @short_description: Debug functions for GDigicam.
 *
 * Various debug functions for GDigicam.
 */

#ifndef __GDIGICAM_DEBUG_H__
#define __GDIGICAM_DEBUG_H__

#ifdef __cplusplus
extern "C" {
#endif

    G_BEGIN_DECLS

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#ifdef GDIGICAM_PERFORMANCE
#include <sys/time.h>
#endif

#ifdef GDIGICAM_PLATFORM_MAEMO
#include <osso-log.h>

/**
 * G_DIGICAM_DEBUG:
 * @args...: variable arguments following the format as in printf
 * function.
 *
 * Macro function to send debug messages via the chosen platform
 * message logging engine.
 */
#define G_DIGICAM_DEBUG(args...) ULOG_DEBUG(args)

/**
 * G_DIGICAM_WARN:
 * @args...: variable arguments following the format as in printf
 * function.
 *
 * Macro function to send warning messages via the chosen platform
 * message logging engine.
 */
#define G_DIGICAM_WARN(args...) ULOG_WARN(args)

/**
 * G_DIGICAM_ERR:
 * @args...: variable arguments following the format as in printf
 * function.
 *
 * Macro function to send error messages via the chosen platform
 * message logging engine.
 */
#define G_DIGICAM_ERR(args...) ULOG_ERR(args)

#else

/**
 * G_DIGICAM_DEBUG:
 * @args...: variable arguments following the format as in printf
 * function.
 *
 * Macro function to send debug messages via the chosen platform
 * message logging engine.
 */
#define G_DIGICAM_DEBUG(args...) g_debug(args)

/**
 * G_DIGICAM_WARN:
 * @args...: variable arguments following the format as in printf
 * function.
 *
 * Macro function to send warning messages via the chosen platform
 * message logging engine.
 */
#define G_DIGICAM_WARN(args...) g_warning(args)

/**
 * G_DIGICAM_ERR:
 * @args...: variable arguments following the format as in printf
 * function.
 *
 * Macro function to send error messages via the chosen platform
 * message logging engine.
 */
#define G_DIGICAM_ERR(args...) g_error(args)

#endif

#ifdef GDIGICAM_PERFORMANCE
#define TSTAMP(name) do {                                 \
    struct timeval tm;                                    \
    gettimeofday (&tm, NULL);                             \
    printf("PERFORMANCE: %s %d.%06d\n", #name, (int) tm.tv_sec, (int) tm.tv_usec);    \
    } while (0)

#else
#define TSTAMP(name)

#endif

    G_END_DECLS

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
