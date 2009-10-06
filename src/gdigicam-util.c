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

#include "gdigicam-util.h"
#include <gst/gst.h>

/**
 * SECTION:gdigicam-util
 * @short_description: Utility functions for GDigicam.
 *
 * Various Utility functions for GDigicam.
 */

/**
 * g_digicam_init:
 * @argc: pointer to the argument list count
 * @argv: pointer to the argument list vector
 *
 * Utility function to call gst_init().
 */
void
g_digicam_init (int    *argc,
                char ***argv)
{
    static gboolean gst_is_initialized = FALSE;

    if (!gst_is_initialized) {
        gst_init (argc, argv);

        gst_is_initialized = TRUE;
    }

    return;
}
