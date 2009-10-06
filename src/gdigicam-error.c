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
 * SECTION:gdigicam-error
 * @short_description: Methods to manage the operation errors in GDigicam.
 *
 * These tools bring together all the needed to handle the
 * #GDigicamError codes in order to identify and notify the possible
 * operation errors.
 */

#include <glib.h>

#include "gdigicam-error.h"

#define G_DIGICAM_ERROR_QUARK "gdigicam-error-quark"

static const gchar* error_msgs[] = {
    "Failed",
    "There isn't any GStreamer Digicam bin set",
    "The camera bin has not viewfinder capabilities",
    "The mode is not supported by the device",
    "The operation is invalid for this mode",
    "The camera bin has not this flash mode capability",
    "The camera bin has not this focus mode capability",
    "The operation is invalid for this focus mode",
    "The camera bin has the autofocus locked",
    "The camera bin has not this exposure mode capability",
    "The camera bin has not this iso sensitivity mode capability",
    "The camera bin has not this white balance mode capability",
    "The camera bin has not this metering mode capability",
    "The camera bin has not this aspect ratio capability",
    "The camera bin has not this quality capability",
    "The camera bin has not this resolution capability",
    "These lock combination was impossible to perform",
    "The camera bin has not zoom capabilities",
    "The zoom value is out of range",
    "The camera bin has not this audio state capability",
    "The camera bin has not this preview mode capability"
};

static const int n_error_msgs = sizeof (error_msgs)/sizeof (error_msgs[0]);


/**
 * g_digicam_error_quark:
 *
 * Creates a new #GQuark for error managing.
 *
 * Return value: the #GQuark
 */
GQuark
g_digicam_error_quark (void)
{
    static GQuark error_quark = 0;

    if (error_quark == 0)
    {
        error_quark = g_quark_from_static_string (G_DIGICAM_ERROR_QUARK);
    }

    return error_quark;
}

static const gchar*
_str_error (GDigicamError g_digicam_error)
{
    g_return_val_if_fail (g_digicam_error < n_error_msgs, NULL);

    return error_msgs[g_digicam_error];
}

static GError*
_error_new_valist (GDigicamError g_digicam_error, const gchar *str, va_list args)
{
    GError *error = NULL;
    gchar *orig = NULL;

    orig = g_strdup_vprintf (str, args);

    error = g_error_new (G_DIGICAM_ERROR, g_digicam_error, "%s: %s",
                         _str_error (g_digicam_error),
                         orig);

    g_free (orig);

    return error;
}

/**
 * g_digicam_error_new:
 * @error_id: the error id
 * @str: strings to be shown on error message
 * @Varargs: variable arguments following the format as in printf
 * function.
 *
 * Creates a new GDigicam #Gerror with the proper id and string
 *
 * Return value: the new GDigicam #GError
 */
GError*
g_digicam_error_new (GDigicamError error_id, const gchar *str, ...)
{
    GError* error = NULL;
    va_list args = NULL;

    va_start (args, str);
    error = _error_new_valist (error_id, str, args);
    va_end (args);

    return error;
}


/**
 * g_digicam_set_error:
 * @orig_error: the original #GError
 * @error_id: the error id to set
 * @str: strings to be shown on error message
 * @Varargs: variable arguments following the format as in printf
 * function.
 *
 * Sets or creates a gdigicam error with the proper id and string
 */
void
g_digicam_set_error (GError **orig_error, GDigicamError error_id, const gchar *str, ...)
{
    GError* error = NULL;
    va_list args;

    if (orig_error != NULL)
    {
        g_return_if_fail (*orig_error == NULL);

        va_start (args, str);
        error = _error_new_valist (error_id, str, args);
        va_end (args);

        *orig_error = error;
    }

    return;
}
