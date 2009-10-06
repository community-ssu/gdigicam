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

#ifndef __G_DIGICAM_UTIL_H__
#define __G_DIGICAM_UTIL_H__

#include <glib.h>

#ifdef __cplusplus
extern "C" {
#endif

    G_BEGIN_DECLS

    void g_digicam_init (int    *argc,
                         char ***argv);

    G_END_DECLS

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* __G_DIGICAM_UTIL_H__ */
