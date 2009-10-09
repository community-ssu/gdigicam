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

#include <stdlib.h>
#include <string.h>
/* #include <unistd.h> */
#include <check.h>
#include <gst/gst.h>
#include <gst/gstbin.h>
#include <gtk/gtk.h>
#include <gdk/gdkx.h>

#include "test_suites.h"
#include "check-utils.h"
#include "gdigicam-util.h"
#include "gdigicam-error.h"
#include "gdigicam-manager.h"

#include <libintl.h>

#define _(String) dgettext("gdigicam", String)

#define MIN_ZOOM 1
#define NORMAL_ZOOM_MACRO_DISABLED 8
#define NORMAL_ZOOM_MACRO_ENABLED 2
#define NORMAL_OPTICAL_ZOOM_MACRO_DISABLED 5
#define WHITE_BALANCE_TEST_VALUE 4
#define ISO_SENSITIVITY_TEST_VALUE 3

static GDigicamManager *no_featured_manager = NULL;
static GDigicamManager *minimum_featured_manager = NULL;
static GDigicamManager *full_featured_manager = NULL;
static GstElement *no_featured_camera_bin = NULL;
static GstElement *minimum_featured_camera_bin = NULL;
static GstElement *full_featured_camera_bin = NULL;
static GstElement *tmp_camera_bin = NULL;
static GDigicamDescriptor *no_featured_descriptor = NULL;
static GDigicamDescriptor *minimum_featured_descriptor = NULL;
static GDigicamDescriptor *full_featured_descriptor = NULL;
static GDigicamDescriptor *tmp_descriptor = NULL;
static GError *error = NULL;
static GtkWidget *window = NULL;

/* -------------------- Fixtures -------------------- */

static void
fx_setup_g_digicam (void)
{
    int argc = 0;
    char **argv = NULL;

    /* Initialize the GDigicam */
    g_digicam_init (&argc, &argv);

    /* Initialize gtk */
    gtk_init (&argc, &argv);
}

static void
fx_setup_default_managers (void)
{
    fx_setup_g_digicam ();

    no_featured_manager = g_digicam_manager_new ();
    no_featured_camera_bin = create_basic_gstreamer_element ("no featured camera bin");
    no_featured_descriptor = create_none_featured_descriptor (no_featured_camera_bin);

    minimum_featured_manager = g_digicam_manager_new ();
    minimum_featured_camera_bin = create_basic_gstreamer_element ("minimum featured camera bin");
    minimum_featured_descriptor = create_minimum_featured_descriptor (minimum_featured_camera_bin);

    full_featured_manager = g_digicam_manager_new ();
    full_featured_camera_bin = create_basic_gstreamer_element ("full featured camera bin");
    full_featured_descriptor = create_full_featured_descriptor (full_featured_camera_bin);
}

static void
fx_teardown_default_managers (void)
{
    /* Unref managers */
    if (NULL != no_featured_manager) {
        g_object_unref (G_OBJECT (no_featured_manager));
        no_featured_manager = NULL;
    }
    if (NULL != minimum_featured_manager) {
        g_object_unref (G_OBJECT (minimum_featured_manager));
        minimum_featured_manager = NULL;
    }
    if (NULL != full_featured_manager) {
        g_object_unref (G_OBJECT (full_featured_manager));
        full_featured_manager = NULL;
    }

    if (NULL != no_featured_camera_bin) {
        gst_object_unref (GST_OBJECT (no_featured_camera_bin));
        no_featured_camera_bin = NULL;
    }

    if (NULL != minimum_featured_camera_bin) {
        gst_object_unref (GST_OBJECT (minimum_featured_camera_bin));
        minimum_featured_camera_bin = NULL;
    }

    if (NULL != full_featured_camera_bin) {
        gst_object_unref (GST_OBJECT (full_featured_camera_bin));
        full_featured_camera_bin = NULL;
    }

    if (NULL != tmp_camera_bin) {
        gst_object_unref (GST_OBJECT (tmp_camera_bin));
        tmp_camera_bin = NULL;
    }

    if (NULL != no_featured_descriptor) {
	g_digicam_manager_descriptor_free (no_featured_descriptor);
        no_featured_descriptor = NULL;
    }

    if (NULL != minimum_featured_descriptor) {
	g_digicam_manager_descriptor_free (minimum_featured_descriptor);
        minimum_featured_descriptor = NULL;
    }

    if (NULL != full_featured_descriptor) {
	g_digicam_manager_descriptor_free (full_featured_descriptor);
        full_featured_descriptor = NULL;
    }

    if (NULL != tmp_descriptor) {
	g_digicam_manager_descriptor_free (tmp_descriptor);
        tmp_descriptor = NULL;
    }

    if (NULL != error) {
        g_error_free (error);
        error = NULL;
    }

    if (NULL != window) {
        gtk_widget_destroy (window);
/*         g_object_unref (G_OBJECT (window)); */
        window = NULL;
    }
}

/* -------------------- Test cases -------------------- */

/* ----- Test case for new -----*/

/**
 * Purpose: test #GDigicamManager class instantation.
 * Cases considered:
 *    - Create a #GDigicamManager implementator object.
 */
START_TEST (test_g_digicam_manager_new)
{
    /* Case 1 */
    no_featured_manager = g_digicam_manager_new ();

    /* Check that the gdigicam-manager object has been created properly */
    fail_if (!G_DIGICAM_IS_MANAGER (no_featured_manager),
             "g-digicam-manager: the returned object "
             "is not a GDigicamManager instance.");
}
END_TEST

/* ----- Test case for set/get_gstreamer_bin -----*/

/**
 * Purpose: test setting and getting limit gstreamer bin in a #GDigicamManager
 * Cases considered:
 *    - get camera bin from just created #GDigicamManager.
 */
START_TEST (test_set_get_gstreamer_bin_limit)
{
    GstElement *gotten_camera_bin = NULL;

    /* Test 1 */
    fail_if (g_digicam_manager_get_gstreamer_bin
             (no_featured_manager, &gotten_camera_bin, &error),
             "gdigicam-manager: an error has not happened.");
    fail_if (NULL != gotten_camera_bin,
             "gdigicam-manager: GDigicamManager object is not empty "
             "just after creation.");
    fail_if (NULL == error,
             "gdigicam-manager: error was not set.");
}
END_TEST

/**
 * Purpose: test setting and getting a valid gstreamer bin in a #GDigicamManager
 * Cases considered:
 *    - set/get camera bin to a #GDigicamManager.
 *    - set/get a different camera bin to a #GDigicamManager with a camera
 *      bin already set.
 *    - set/get the same camera bin to a #GDigicamManager with a camera
 *      bin already set.
 */
START_TEST (test_set_get_gstreamer_bin_regular)
{
    GstElement *gotten_camera_bin = NULL;

    /* Test 1 */
    fail_if (!g_digicam_manager_set_gstreamer_bin
             (no_featured_manager, no_featured_camera_bin,
              no_featured_descriptor, &error),
             "gdigicam-manager: an error has happened.");
    fail_if (NULL != error,
             "gdigicam-manager: error was set.");

    fail_if (!g_digicam_manager_get_gstreamer_bin
             (no_featured_manager, &gotten_camera_bin, &error),
             "gdigicam-manager: an error has happened.");
    fail_if (no_featured_camera_bin != gotten_camera_bin,
             "gdigicam-manager: internal camera bin differs "
             "from previously provided one.");
    fail_if (NULL != error,
             "gdigicam-manager: error was set.");
    gst_object_unref (GST_OBJECT (gotten_camera_bin));

    /* Test 2 */
    tmp_camera_bin = create_basic_gstreamer_element ("tmp no featured "
                                                     "camera bin");
    tmp_descriptor = create_none_featured_descriptor (tmp_camera_bin);
    fail_if (!g_digicam_manager_set_gstreamer_bin
             (no_featured_manager, tmp_camera_bin,
              tmp_descriptor, &error),
             "gdigicam-manager: an error has happened.");
    fail_if (NULL != error,
             "gdigicam-manager: error was set.");

    fail_if (!g_digicam_manager_get_gstreamer_bin
             (no_featured_manager, &gotten_camera_bin, &error),
             "gdigicam-manager: an error has happened.");
    fail_if (tmp_camera_bin != gotten_camera_bin,
             "gdigicam-manager: internal camera bin differs "
             "from previously provided one.");
    fail_if (NULL != error,
             "gdigicam-manager: error was set.");
    gst_object_unref (GST_OBJECT (gotten_camera_bin));

    /* Test 3 */
    fail_if (!g_digicam_manager_set_gstreamer_bin
             (no_featured_manager, tmp_camera_bin, tmp_descriptor, &error),
             "gdigicam-manager: an error has happened.");
    fail_if (NULL != error,
             "gdigicam-manager: error was set.");

    fail_if (!g_digicam_manager_get_gstreamer_bin
             (no_featured_manager, &gotten_camera_bin, &error),
             "gdigicam-manager: an error has happened.");
    fail_if (tmp_camera_bin != gotten_camera_bin,
             "gdigicam-manager: internal camera bin differs "
             "from previously provided one.");
    fail_if (NULL != error,
             "gdigicam-manager: error was set.");
    gst_object_unref (GST_OBJECT (gotten_camera_bin));
}
END_TEST

/**
 * Purpose: test setting and getting an invalid camera bin in a #GDigicamManager
 * Cases considered:
 *    - set/get a NULL camera bin to a #GDigicamManager.
 *      The internal camera bin remains the same.
 *    - set/get an invalid camera bin to a #GDigicamManager.
 *      The internal camera bin remains the same.
 *    - set a camera bin to a NULL #GDigicamManager. Parameters remain
 *      the same.
 *    - set a camera bin to an invalid #GDigicamManager.  Parameters
 *      remain the same.
 *    - get a camera bin from a NULL #GDigicamManager. Parameters remain
 *      the same.
 *    - get a camera bin from an invalid #GDigicamManager. Parameters
 *      remain the same.
 */
START_TEST (test_set_get_gstreamer_bin_invalid)
{
    GstElement *gotten_camera_bin = NULL;

    /* Test 1 */
    g_digicam_manager_set_gstreamer_bin (no_featured_manager,
                                         no_featured_camera_bin,
                                         no_featured_descriptor,
                                         NULL);
    fail_if (g_digicam_manager_set_gstreamer_bin
             (no_featured_manager, NULL, NULL, &error),
             "gdigicam-manager: there was no error.");
    fail_if (NULL != error,
             "gdigicam-manager: error was set.");

    fail_if (!g_digicam_manager_get_gstreamer_bin
             (no_featured_manager, &gotten_camera_bin, &error),
             "gdigicam-manager: an error has happened.");
    fail_if (no_featured_camera_bin != gotten_camera_bin,
             "gdigicam-manager: internal camera bin differs "
             "from previously provided one.");
    fail_if (NULL != error,
             "gdigicam-manager: error was set.");
    gst_object_unref (GST_OBJECT (gotten_camera_bin));
    gotten_camera_bin = NULL;

    /* Test 2 */
    window = create_test_window ();
    fail_if (g_digicam_manager_set_gstreamer_bin
             (no_featured_manager, GST_ELEMENT (window),
              no_featured_descriptor, &error),
             "gdigicam-manager: there was no error.");
    fail_if (NULL != error,
             "gdigicam-manager: error was set.");
    fail_if (!g_digicam_manager_get_gstreamer_bin
             (no_featured_manager, &gotten_camera_bin, &error),
             "gdigicam-manager: an error has happened.");
    fail_if (no_featured_camera_bin != gotten_camera_bin,
             "gdigicam-manager: internal camera bin differs "
             "from previously provided one.");
    fail_if (NULL != error,
             "gdigicam-manager: error was set.");
    gst_object_unref (GST_OBJECT (gotten_camera_bin));
    gotten_camera_bin = NULL;

    /* Test 3 */
    fail_if (g_digicam_manager_set_gstreamer_bin
             (NULL, no_featured_camera_bin, no_featured_descriptor, &error),
             "gdigicam-manager: there was no error.");
    fail_if (NULL != error,
             "gdigicam-manager: error has changed.");

    /* Test 4 */
    fail_if (g_digicam_manager_set_gstreamer_bin
             (G_DIGICAM_MANAGER (window),
              no_featured_camera_bin, no_featured_descriptor, &error),
             "gdigicam-manager: there was no error.");
    fail_if (NULL != error,
             "gdigicam-manager: error has changed.");

    /* Test 5 */
    fail_if (g_digicam_manager_get_gstreamer_bin
             (NULL, &gotten_camera_bin, &error),
             "gdigicam-manager: there was no error.");
    fail_if (NULL != gotten_camera_bin,
             "gdigicam-manager: obtained a strange camera bin "
             "from invalid object.");
    fail_if (NULL != error,
             "gdigicam-manager: error has changed.");
    gotten_camera_bin = NULL;

    /* Test 6 */
    fail_if (g_digicam_manager_get_gstreamer_bin
             (G_DIGICAM_MANAGER (window), &gotten_camera_bin, &error),
             "gdigicam-manager: there was no error from provided parameters.");
    fail_if (NULL != gotten_camera_bin,
             "gdigicam-manager: obtained a strange camera bin "
             "from invalid object.");
    fail_if (NULL != error,
             "gdigicam-manager: error has changed.");
    gotten_camera_bin = NULL;
}
END_TEST


/* ----- Test case for set/get_mode -----*/

/**
 * Purpose: test setting and getting limit modes in a #GDigicamManager
 * Cases considered:
 *    - set/get mode from just created #GDigicamManager.
 *    - set/get mode to a none featured #GDigicamManager.
 *    - get mode from a featured but not set #GDigicamManager.
 */
START_TEST (test_set_get_mode_limit)
{
    GDigicamMode gotten_mode = 0;

    /* Test 1 */
    fail_if (g_digicam_manager_set_mode (no_featured_manager,
                                          G_DIGICAM_MODE_VIDEO,
                                          &error,
                                          NULL),
             "gdigicam-manager: an error has not happened.");
    fail_if (NULL == error,
             "gdigicam-manager: error was not set.");

    fail_if (g_digicam_manager_get_mode
             (no_featured_manager, &gotten_mode, &error),
             "gdigicam-manager: an error has not happened.");
    fail_if (0 != gotten_mode,
             "gdigicam-manager: the mode was not the \"unset\" "
             "value, it was \"%d\".",
             gotten_mode);
    fail_if (NULL == error,
             "gdigicam-manager: error was not set.");
    gotten_mode = 0;

    /* Test 2 */
    g_digicam_manager_set_gstreamer_bin (no_featured_manager,
                                         no_featured_camera_bin,
                                         no_featured_descriptor,
                                         NULL);
    fail_if (g_digicam_manager_set_mode (no_featured_manager,
                                          G_DIGICAM_MODE_VIDEO,
                                          &error,
                                          NULL),
             "gdigicam-manager: an error has not happened.");
    fail_if (NULL == error,
             "gdigicam-manager: error was not set.");

    fail_if (g_digicam_manager_get_mode
             (no_featured_manager, &gotten_mode, &error),
             "gdigicam-manager: an error has not happened.");
    fail_if (0 != gotten_mode,
             "gdigicam-manager: the mode was not the \"unset\" "
             "value, it was \"%d\".",
             gotten_mode);
    fail_if (NULL == error,
             "gdigicam-manager: error was not set.");
    fail_if (g_error_matches (error,
                               G_DIGICAM_ERROR,
                               G_DIGICAM_ERROR_MODE_NOT_SUPPORTED),
             "gdigicam-manager: error is  \n\"%s\"\n instead of "
             "\"mode not supported\".",
             error->message);
    g_error_free (error);
    error = NULL;

    /* Test 3 */
    g_digicam_manager_set_gstreamer_bin (minimum_featured_manager,
                                         minimum_featured_camera_bin,
                                         minimum_featured_descriptor,
                                         NULL);
    fail_if (!g_digicam_manager_get_mode
             (minimum_featured_manager, &gotten_mode, &error),
             "gdigicam-manager: an error has happened.");
    fail_if (0 != gotten_mode,
             "gdigicam-manager: the mode was not the \"unset\" "
             "value, it was \"%d\".",
             gotten_mode);
    fail_if (NULL != error,
             "gdigicam-manager: error was set.");
}
END_TEST

/**
 * Purpose: test setting and getting a valid mode in a #GDigicamManager
 * Cases considered:
 *    - set/get mode to a featured #GDigicamManager.
 *    - set/get again another mode to a featured #GDigicamManager.
 */
START_TEST (test_set_get_mode_regular)
{
    GDigicamMode gotten_mode = 0;

    /* Test 1 */
    g_digicam_manager_set_gstreamer_bin (minimum_featured_manager,
                                         minimum_featured_camera_bin,
                                         minimum_featured_descriptor,
                                         NULL);
    fail_if (!g_digicam_manager_set_mode (minimum_featured_manager,
                                          G_DIGICAM_MODE_VIDEO,
                                          &error,
                                          NULL),
             "gdigicam-manager: an error has happened.");
    fail_if (NULL != error,
             "gdigicam-manager: error was set.");

    fail_if (!g_digicam_manager_get_mode
             (minimum_featured_manager, &gotten_mode, &error),
             "gdigicam-manager: an error has happened.");
    fail_if (G_DIGICAM_MODE_VIDEO != gotten_mode,
             "gdigicam-manager: the mode was not the previously provided.");
    fail_if (NULL != error,
             "gdigicam-manager: error was set.");
    gotten_mode = 0;

    /* Test 2 */
    fail_if (!g_digicam_manager_set_mode (minimum_featured_manager,
                                          G_DIGICAM_MODE_STILL,
                                          &error,
                                          NULL),
             "gdigicam-manager: an error has happened.");
    fail_if (NULL != error,
             "gdigicam-manager: error was not set.");

    fail_if (!g_digicam_manager_get_mode
             (minimum_featured_manager, &gotten_mode, &error),
             "gdigicam-manager: an error has happened.");
    fail_if (G_DIGICAM_MODE_STILL != gotten_mode,
             "gdigicam-manager: the mode was not the previously provided.");
    fail_if (NULL != error,
             "gdigicam-manager: error was set.");
}
END_TEST

/**
 * Purpose: test setting and getting a in an invalid #GDigicamManager
 * Cases considered:
 *    - set/get the mode to/from a NULL #GDigicamManager. Parameters
 *      remain the same.
 *    - set/get the mode to/from an invalid
 *      #GDigicamManager. Parameters remain the same.
 */
START_TEST (test_set_get_mode_invalid)
{
    GDigicamMode gotten_mode = 0;

    /* Test 1 */
    fail_if (g_digicam_manager_set_mode (NULL,
                                         G_DIGICAM_MODE_VIDEO,
                                         &error,
                                         NULL),
             "gdigicam-manager: there was no error.");
    fail_if (NULL != error,
             "gdigicam-manager: error has changed.");

    fail_if (g_digicam_manager_get_mode
             (NULL, &gotten_mode, &error),
             "gdigicam-manager: there was no error.");
    fail_if (0 != gotten_mode,
             "gdigicam-manager: the mode was not the \"unset\" "
             "value, it was \"%d\".",
             gotten_mode);
    fail_if (NULL != error,
             "gdigicam-manager: error has changed.");
    gotten_mode = 0;

    /* Test 2 */
    fail_if (g_digicam_manager_set_mode (G_DIGICAM_MANAGER (window),
                                         G_DIGICAM_MODE_VIDEO,
                                         &error,
                                         NULL),
             "gdigicam-manager: there was no error.");
    fail_if (NULL != error,
             "gdigicam-manager: error has changed.");

    fail_if (g_digicam_manager_get_mode
             (G_DIGICAM_MANAGER (window), &gotten_mode, &error),
             "gdigicam-manager: there was no error.");
    fail_if (0 != gotten_mode,
             "gdigicam-manager: the mode was not the \"unset\" "
             "value, it was \"%d\".",
             gotten_mode);
    fail_if (NULL != error,
             "gdigicam-manager: error has changed.");
    gotten_mode = 0;
}
END_TEST

/* ----- Test case for set/get_flash_mode -----*/

/**
 * Purpose: test setting and getting limit flash modes in a
 * #GDigicamManager
 * Cases considered:
 *    - set/get flash mode from just created #GDigicamManager.
 *    - set/get flash mode to a none featured #GDigicamManager.
 *    - set/get flash mode to a minimum featured #GDigicamManager.
 *    - get flash mode from a featured but not set #GDigicamManager.
 */
START_TEST (test_set_get_flash_mode_limit)
{
    GDigicamFlashmode gotten_flash_mode = 0;

    /* Test 1 */
    fail_if (g_digicam_manager_set_flash_mode (no_featured_manager,
                                                G_DIGICAM_FLASHMODE_OFF,
                                                &error,
                                                NULL),
             "gdigicam-manager: an error has not happened.");
    fail_if (NULL == error,
             "gdigicam-manager: error was not set.");
    fail_if (!g_error_matches (error,
                               G_DIGICAM_ERROR,
                               G_DIGICAM_ERROR_GSTREAMER_BIN_NOT_SET),
             "gdigicam-manager: error is  \n\"%s\"\n instead of "
             "\"flash mode not supported\".",
             error->message);
    g_error_free (error);
    error = NULL;

    fail_if (g_digicam_manager_get_flash_mode
             (no_featured_manager, &gotten_flash_mode, &error),
             "gdigicam-manager: an error has not happened.");
    fail_if (0 != gotten_flash_mode,
             "gdigicam-manager: the flash mode was not the \"unset\" "
             "value, it was \"%d\".",
             gotten_flash_mode);
    fail_if (NULL == error,
             "gdigicam-manager: error was not set.");
    fail_if (!g_error_matches (error,
                               G_DIGICAM_ERROR,
                               G_DIGICAM_ERROR_GSTREAMER_BIN_NOT_SET),
             "gdigicam-manager: error is  \n\"%s\"\n instead of "
             "\"flash mode not supported\".",
             error->message);
    g_error_free (error);
    error = NULL;
    gotten_flash_mode = 0;

    /* Test 2 */
    g_digicam_manager_set_gstreamer_bin (no_featured_manager,
                                         no_featured_camera_bin,
                                         no_featured_descriptor,
                                         NULL);
    fail_if (g_digicam_manager_set_flash_mode (no_featured_manager,
                                                G_DIGICAM_FLASHMODE_ON,
                                                &error,
                                                NULL),
             "gdigicam-manager: an error has not happened.");
    fail_if (NULL == error,
             "gdigicam-manager: error was not set.");
    fail_if (g_error_matches (error,
                               G_DIGICAM_ERROR,
                               G_DIGICAM_ERROR_FLASHMODE_NOT_SUPPORTED),
             "gdigicam-manager: error is  \n\"%s\"\n instead of "
             "\"flash mode not supported\".",
             error->message);
    g_error_free (error);
    error = NULL;

    fail_if (g_digicam_manager_get_flash_mode
             (no_featured_manager, &gotten_flash_mode, &error),
             "gdigicam-manager: an error has not happened.");
    fail_if (0 != gotten_flash_mode,
             "gdigicam-manager: the flash mode was not the \"unset\" "
             "value, it was \"%d\".",
             gotten_flash_mode);
    fail_if (NULL == error,
             "gdigicam-manager: error was not set.");
    fail_if (!g_error_matches (error,
                               G_DIGICAM_ERROR,
                               G_DIGICAM_ERROR_INVALID_MODE),
             "gdigicam-manager: error is  \n\"%s\"\n instead of "
             "\"flash mode not supported\".",
             error->message);
    g_error_free (error);
    error = NULL;
    gotten_flash_mode = 0;

    /* Test 3 */
    g_digicam_manager_set_gstreamer_bin (minimum_featured_manager,
                                         minimum_featured_camera_bin,
                                         minimum_featured_descriptor,
                                         NULL);
    fail_if (g_digicam_manager_set_flash_mode (minimum_featured_manager,
                                                G_DIGICAM_FLASHMODE_ON,
                                                &error,
                                                NULL),
             "gdigicam-manager: an error has not happened.");
    fail_if (NULL == error,
             "gdigicam-manager: error was not set.");
    fail_if (!g_error_matches (error,
                               G_DIGICAM_ERROR,
                               G_DIGICAM_ERROR_INVALID_MODE),
             "gdigicam-manager: error is  \n\"%s\"\n instead of "
             "\"flash mode not supported\".",
             error->message);
    g_error_free (error);
    error = NULL;

    fail_if (g_digicam_manager_get_flash_mode
             (minimum_featured_manager, &gotten_flash_mode, &error),
             "gdigicam-manager: an error has not happened.");
    fail_if (0 != gotten_flash_mode,
             "gdigicam-manager: the flash mode was not the \"unset\" "
             "value, it was \"%d\".",
             gotten_flash_mode);
    fail_if (NULL == error,
             "gdigicam-manager: error was not set.");
    fail_if (!g_error_matches (error,
                               G_DIGICAM_ERROR,
                               G_DIGICAM_ERROR_INVALID_MODE),
             "gdigicam-manager: error is  \n\"%s\"\n instead of "
             "\"flash mode not supported\".",
             error->message);
    g_error_free (error);
    error = NULL;
    gotten_flash_mode = 0;

    /* Test 4 */
    g_digicam_manager_set_gstreamer_bin (full_featured_manager,
                                         full_featured_camera_bin,
                                         full_featured_descriptor,
                                         NULL);
    g_digicam_manager_set_mode (full_featured_manager,
				G_DIGICAM_MODE_STILL,
				&error,
				NULL);
    fail_if (!g_digicam_manager_get_flash_mode
             (full_featured_manager, &gotten_flash_mode, &error),
             "gdigicam-manager: an error has happened.");
    fail_if (0 != gotten_flash_mode,
             "gdigicam-manager: the mode was not the \"unset\" "
             "value, it was \"%d\".",
             gotten_flash_mode);
    fail_if (NULL != error,
             "gdigicam-manager: error was set.");
}
END_TEST

/**
 * Purpose: test setting and getting a valid flash mode in a
 * #GDigicamManager
 * Cases considered:
 *    - set/get flash mode to a featured #GDigicamManager.
 *    - set/get again another flash mode to a featured #GDigicamManager.
 */
START_TEST (test_set_get_flash_mode_regular)
{
    GDigicamFlashmode gotten_flash_mode = 0;

    /* Test 1 */
    g_digicam_manager_set_gstreamer_bin (full_featured_manager,
                                         full_featured_camera_bin,
                                         full_featured_descriptor,
                                         NULL);
    g_digicam_manager_set_mode (full_featured_manager,
				G_DIGICAM_MODE_STILL,
				&error,
				NULL);
    fail_if (!g_digicam_manager_set_flash_mode (full_featured_manager,
                                                G_DIGICAM_FLASHMODE_ON,
                                                &error,
                                                NULL),
             "gdigicam-manager: an error has happened.");
    fail_if (NULL != error,
             "gdigicam-manager: error was set.");

    fail_if (!g_digicam_manager_get_flash_mode
             (full_featured_manager, &gotten_flash_mode, &error),
             "gdigicam-manager: an error has happened.");
    fail_if (G_DIGICAM_FLASHMODE_ON != gotten_flash_mode,
             "gdigicam-manager: the flash mode was not "
             "the previously provided.");
    fail_if (NULL != error,
             "gdigicam-manager: error was set.");
    gotten_flash_mode = 0;

    /* Test 2 */
    fail_if (!g_digicam_manager_set_flash_mode (full_featured_manager,
                                                G_DIGICAM_FLASHMODE_REDEYEREDUCTION,
                                                &error,
                                                NULL),
             "gdigicam-manager: an error has happened.");
    fail_if (NULL != error,
             "gdigicam-manager: error was set.");

    fail_if (!g_digicam_manager_get_flash_mode
             (full_featured_manager, &gotten_flash_mode, &error),
             "gdigicam-manager: an error has happened.");
    fail_if (G_DIGICAM_FLASHMODE_REDEYEREDUCTION != gotten_flash_mode,
             "gdigicam-manager: the flash mode was not "
             "the previously provided.");
    fail_if (NULL != error,
             "gdigicam-manager: error was set.");
}
END_TEST

/**
 * Purpose: test setting and getting a flash mode in an invalid
 * #GDigicamManager
 * Cases considered:
 *    - set/get the flash mode to/from a NULL
 *      #GDigicamManager. Parameters remain the same.
 *    - set/get the flash mode to/from an invalid
 *      #GDigicamManager. Parameters remain the same.
 */
START_TEST (test_set_get_flash_mode_invalid)
{
    GDigicamFlashmode gotten_flash_mode = 0;

    /* Test 1 */
    fail_if (g_digicam_manager_set_flash_mode (NULL,
                                               G_DIGICAM_FLASHMODE_ON,
                                               &error,
                                               NULL),
             "gdigicam-manager: there was no error.");
    fail_if (NULL != error,
             "gdigicam-manager: error has changed.");

    fail_if (g_digicam_manager_get_flash_mode
             (NULL, &gotten_flash_mode, &error),
             "gdigicam-manager: there was no error.");
    fail_if (0 != gotten_flash_mode,
             "gdigicam-manager: the flash mode was not the \"unset\" "
             "value, it was \"%d\".",
             gotten_flash_mode);
    fail_if (NULL != error,
             "gdigicam-manager: error has changed.");
    gotten_flash_mode = 0;

    /* Test 2 */
    fail_if (g_digicam_manager_set_flash_mode (G_DIGICAM_MANAGER (window),
                                               G_DIGICAM_FLASHMODE_ON,
                                               &error,
                                               NULL),
             "gdigicam-manager: there was no error.");
    fail_if (NULL != error,
             "gdigicam-manager: error has changed.");

    fail_if (g_digicam_manager_get_flash_mode
             (G_DIGICAM_MANAGER (window), &gotten_flash_mode, &error),
             "gdigicam-manager: there was no error.");
    fail_if (0 != gotten_flash_mode,
             "gdigicam-manager: the flash mode was not the \"unset\" "
             "value, it was \"%d\".",
             gotten_flash_mode);
    fail_if (NULL != error,
             "gdigicam-manager: error has changed.");
    gotten_flash_mode = 0;
}
END_TEST

/* ----- Test case for set/get_iso_sensitivity_mode -----*/

/**
 * Purpose: test setting and getting limit iso sensitivity modes in a
 * #GDigicamManager
 * Cases considered:
 *    - set/get iso sensitivity mode from just created #GDigicamManager.
 *    - set/get iso sensitivity mode to a none featured #GDigicamManager.
 *    - set/get iso sensitivity mode to a minimum featured #GDigicamManager.
 *    - get iso sensitivity mode from a featured but not set #GDigicamManager.
 */
START_TEST (test_set_get_iso_sensitivity_mode_limit)
{
    GDigicamIsosensitivitymode gotten_iso_sensitivity_mode = 0;
    guint gotten_level = 0;

    /* Test 1 */
    fail_if (g_digicam_manager_set_iso_sensitivity_mode (no_featured_manager,
                                                        G_DIGICAM_ISOSENSITIVITYMODE_AUTO,
                                                        3,
                                                        &error,
                                                        NULL),
             "gdigicam-manager: an error has not happened.");
    fail_if (NULL == error,
             "gdigicam-manager: error was not set.");
    fail_if (!g_error_matches (error,
                               G_DIGICAM_ERROR,
                               G_DIGICAM_ERROR_GSTREAMER_BIN_NOT_SET),
             "gdigicam-manager: error is  \n\"%s\"\n instead of "
             "\"exposure mode not supported\".",
             error->message);
    g_error_free (error);
    error = NULL;

    fail_if (g_digicam_manager_get_iso_sensitivity_mode
             (no_featured_manager,
              &gotten_iso_sensitivity_mode,
              &gotten_level,
              &error),
             "gdigicam-manager: an error has not happened.");
    fail_if (0 != gotten_iso_sensitivity_mode,
             "gdigicam-manager: the exposure mode was not the \"unset\" "
             "value, it was \"%d\".",
             gotten_iso_sensitivity_mode);
    fail_if (0 != gotten_level,
             "gdigicam-manager: the compensation was not the \"unset\" "
             "value, it was \"%d\".",
             gotten_level);
    fail_if (NULL == error,
             "gdigicam-manager: error was not set.");
    fail_if (!g_error_matches (error,
                               G_DIGICAM_ERROR,
                               G_DIGICAM_ERROR_GSTREAMER_BIN_NOT_SET),
             "gdigicam-manager: error is  \n\"%s\"\n instead of "
             "\"exposure mode not supported\".",
             error->message);
    g_error_free (error);
    error = NULL;
    gotten_iso_sensitivity_mode = 0;
    gotten_level = 0;

    /* Test 2 */
    g_digicam_manager_set_gstreamer_bin (no_featured_manager,
                                         no_featured_camera_bin,
                                         no_featured_descriptor,
                                         NULL);
    fail_if (g_digicam_manager_set_iso_sensitivity_mode (no_featured_manager,
                                                        G_DIGICAM_ISOSENSITIVITYMODE_AUTO,
                                                        3,
                                                        &error,
                                                        NULL),
             "gdigicam-manager: an error has not happened.");
    fail_if (NULL == error,
             "gdigicam-manager: error was not set.");
    fail_if (!g_error_matches (error,
                               G_DIGICAM_ERROR,
                               G_DIGICAM_ERROR_ISOSENSITIVITYMODE_NOT_SUPPORTED),
             "gdigicam-manager: error is  \n\"%s\"\n instead of "
             "\"white balance mode not supported\".",
             error->message);
    g_error_free (error);
    error = NULL;

    fail_if (g_digicam_manager_get_iso_sensitivity_mode
             (no_featured_manager,
              &gotten_iso_sensitivity_mode,
              &gotten_level,
              &error),
             "gdigicam-manager: an error has not happened.");
    fail_if (0 != gotten_iso_sensitivity_mode,
             "gdigicam-manager: the white balance mode was not the \"unset\" "
             "value, it was \"%d\".",
             gotten_iso_sensitivity_mode);
    fail_if (0 != gotten_level,
             "gdigicam-manager: the compensation was not the \"unset\" "
             "value, it was \"%d\".",
             gotten_level);
    fail_if (NULL == error,
             "gdigicam-manager: error was not set.");
    fail_if (!g_error_matches (error,
                               G_DIGICAM_ERROR,
                               G_DIGICAM_ERROR_ISOSENSITIVITYMODE_NOT_SUPPORTED),
             "gdigicam-manager: error is  \n\"%s\"\n instead of "
             "\"white balance mode not supported\".",
             error->message);
    g_error_free (error);
    error = NULL;
    gotten_iso_sensitivity_mode = 0;
    gotten_level = 0;

    /* Test 3 */
    g_digicam_manager_set_gstreamer_bin (minimum_featured_manager,
                                         minimum_featured_camera_bin,
                                         minimum_featured_descriptor,
                                         NULL);
    fail_if (g_digicam_manager_set_iso_sensitivity_mode (minimum_featured_manager,
                                                        G_DIGICAM_ISOSENSITIVITYMODE_AUTO,
                                                        3,
                                                        &error,
                                                        NULL),
             "gdigicam-manager: an error has not happened.");
    fail_if (NULL == error,
             "gdigicam-manager: error was not set.");
    fail_if (!g_error_matches (error,
                               G_DIGICAM_ERROR,
                               G_DIGICAM_ERROR_ISOSENSITIVITYMODE_NOT_SUPPORTED),
             "gdigicam-manager: error is  \n\"%s\"\n instead of "
             "\"exposure mode not supported\".",
             error->message);
    g_error_free (error);
    error = NULL;

    fail_if (g_digicam_manager_get_iso_sensitivity_mode
             (minimum_featured_manager,
              &gotten_iso_sensitivity_mode,
              &gotten_level,
              &error),
             "gdigicam-manager: an error has not happened.");
    fail_if (0 != gotten_iso_sensitivity_mode,
             "gdigicam-manager: the white balance mode was not the \"unset\" "
             "value, it was \"%d\".",
             gotten_iso_sensitivity_mode);
    fail_if (0 != gotten_level,
             "gdigicam-manager: the compensation was not the \"unset\" "
             "value, it was \"%d\".",
             gotten_level);
    fail_if (NULL == error,
             "gdigicam-manager: error was not set.");
    fail_if (!g_error_matches (error,
                               G_DIGICAM_ERROR,
                               G_DIGICAM_ERROR_ISOSENSITIVITYMODE_NOT_SUPPORTED),
             "gdigicam-manager: error is  \n\"%s\"\n instead of "
             "\"white balance mode not supported\".",
             error->message);
    g_error_free (error);
    error = NULL;
    gotten_iso_sensitivity_mode = 0;
    gotten_level = 0;

    /* Test 4 */
    g_digicam_manager_set_gstreamer_bin (full_featured_manager,
                                         full_featured_camera_bin,
                                         full_featured_descriptor,
                                         NULL);
    fail_if (!g_digicam_manager_get_iso_sensitivity_mode
             (full_featured_manager,
              &gotten_iso_sensitivity_mode,
              &gotten_level,
              &error),
             "gdigicam-manager: an error has happened.");
    fail_if (0 != gotten_iso_sensitivity_mode,
             "gdigicam-manager: the white balance mode was not the \"unset\" "
             "value, it was \"%d\".",
             gotten_iso_sensitivity_mode);
    fail_if (0 != gotten_level,
             "gdigicam-manager: the compensation was not the \"unset\" "
             "value, it was \"%d\".",
             gotten_level);
    fail_if (NULL != error,
             "gdigicam-manager: error was set.");
}
END_TEST

/**
 * Purpose: test setting and getting a valid iso sensitivity mode in a
 * #GDigicamManager
 * Cases considered:
 *    - set/get iso sensitivity mode to a featured #GDigicamManager.
 *    - set/get again another iso sensitivity mode to a featured #GDigicamManager.
 */
START_TEST (test_set_get_iso_sensitivity_mode_regular)
{
    GDigicamIsosensitivitymode gotten_iso_sensitivity_mode = 0;
    guint gotten_level = 0;

    /* Test 1 */
    g_digicam_manager_set_gstreamer_bin (full_featured_manager,
                                         full_featured_camera_bin,
                                         full_featured_descriptor,
                                         NULL);
    fail_if (!g_digicam_manager_set_iso_sensitivity_mode (full_featured_manager,
                                                   G_DIGICAM_ISOSENSITIVITYMODE_MANUAL,
                                                   ISO_SENSITIVITY_TEST_VALUE,
                                                   &error,
                                                   NULL),
             "gdigicam-manager: an error has happened.");
    fail_if (NULL != error,
             "gdigicam-manager: error was set.");

    fail_if (!g_digicam_manager_get_iso_sensitivity_mode
             (full_featured_manager,
              &gotten_iso_sensitivity_mode,
              &gotten_level,
              &error),
             "gdigicam-manager: an error has happened.");
    fail_if (G_DIGICAM_ISOSENSITIVITYMODE_MANUAL != gotten_iso_sensitivity_mode,
             "gdigicam-manager: the iso_sensitivity mode was not "
             "the previously provided.");
    fail_if (ISO_SENSITIVITY_TEST_VALUE != gotten_level,
             "gdigicam-manager: the level was not the \"unset\" "
             "value, it was \"%d\".",
             gotten_level);
    fail_if (NULL != error,
             "gdigicam-manager: error was set.");
    gotten_iso_sensitivity_mode = 0;
    gotten_level = 0;

    /* Test 2 */
    fail_if (!g_digicam_manager_set_iso_sensitivity_mode (full_featured_manager,
                                                   G_DIGICAM_ISOSENSITIVITYMODE_AUTO,
                                                   ISO_SENSITIVITY_TEST_VALUE,
                                                   &error,
                                                   NULL),
             "gdigicam-manager: an error has happened.");
    fail_if (NULL != error,
             "gdigicam-manager: error was set.");

    fail_if (!g_digicam_manager_get_iso_sensitivity_mode
             (full_featured_manager,
              &gotten_iso_sensitivity_mode,
              &gotten_level,
              &error),
             "gdigicam-manager: an error has happened.");
    fail_if (G_DIGICAM_ISOSENSITIVITYMODE_AUTO != gotten_iso_sensitivity_mode,
             "gdigicam-manager: the iso_sensitivity mode was not "
             "the previously provided.");
    /* TODO: check why this case is failing. Can we set the value in
     * Automode ? */
    fail_if (ISO_SENSITIVITY_TEST_VALUE != gotten_level,
             "gdigicam-manager: the level was not "
             "the previously provided.");
    fail_if (NULL != error,
             "gdigicam-manager: error was set.");
}
END_TEST

/**
 * Purpose: test setting and getting a white balance mode in an invalid
 * #GDigicamManager
 * Cases considered:
 *    - set/get the iso sensitivity mode to/from a NULL
 *      #GDigicamManager. Parameters remain the same.
 *    - set/get the white balance mode to/from an invalid
 *      #GDigicamManager. Parameters remain the same.
 */
START_TEST (test_set_get_iso_sensitivity_mode_invalid)
{
    GDigicamIsosensitivitymode gotten_iso_sensitivity_mode = 0;
    guint gotten_level = 0;

    /* Test 1 */
    fail_if (g_digicam_manager_set_iso_sensitivity_mode (NULL,
                                                  G_DIGICAM_ISOSENSITIVITYMODE_AUTO,
                                                  3,
                                                  &error,
                                                  NULL),
             "gdigicam-manager: there was no error.");
    fail_if (NULL != error,
             "gdigicam-manager: error has changed.");

    fail_if (g_digicam_manager_get_iso_sensitivity_mode
             (NULL, &gotten_iso_sensitivity_mode, &gotten_level, &error),
             "gdigicam-manager: there was no error.");
    fail_if (0 != gotten_iso_sensitivity_mode,
             "gdigicam-manager: the iso_sensitivity mode was not the \"unset\" "
             "value, it was \"%d\".",
             gotten_iso_sensitivity_mode);
    fail_if (0 != gotten_level,
             "gdigicam-manager: the level was not the \"unset\" "
             "value, it was \"%d\".",
             gotten_level);
    fail_if (NULL != error,
             "gdigicam-manager: error has changed.");
    gotten_iso_sensitivity_mode = 0;
    gotten_level = 0;

    /* Test 2 */
    fail_if (g_digicam_manager_set_iso_sensitivity_mode (G_DIGICAM_MANAGER (window),
                                                  G_DIGICAM_ISOSENSITIVITYMODE_AUTO,
                                                  3,
                                                  &error,
                                                  NULL),
             "gdigicam-manager: there was no error.");
    fail_if (NULL != error,
             "gdigicam-manager: error has changed.");

    fail_if (g_digicam_manager_get_iso_sensitivity_mode
             (G_DIGICAM_MANAGER (window),
              &gotten_iso_sensitivity_mode,
              &gotten_level,
              &error),
             "gdigicam-manager: there was no error.");
    fail_if (0 != gotten_iso_sensitivity_mode,
             "gdigicam-manager: the iso_sensitivity mode was not the \"unset\" "
             "value, it was \"%d\".",
             gotten_iso_sensitivity_mode);
    fail_if (0 != gotten_level,
             "gdigicam-manager: the level was not the \"unset\" "
             "value, it was \"%d\".",
             gotten_level);
    fail_if (NULL != error,
             "gdigicam-manager: error has changed.");
    gotten_iso_sensitivity_mode = 0;
}
END_TEST

/* ----- Test case for set/get_white_balance_mode -----*/

/**
 * Purpose: test setting and getting limit white balance modes in a
 * #GDigicamManager
 * Cases considered:
 *    - set/get white balance mode from just created #GDigicamManager.
 *    - set/get white balance mode to a none featured #GDigicamManager.
 *    - set/get white balance mode to a minimum featured #GDigicamManager.
 *    - get white balance mode from a featured but not set #GDigicamManager.
 */
START_TEST (test_set_get_white_balance_mode_limit)
{
    GDigicamWhitebalancemode gotten_white_balance_mode = 0;
    guint gotten_level = 0;

    /* Test 1 */
    fail_if (g_digicam_manager_set_white_balance_mode (no_featured_manager,
                                                        G_DIGICAM_WHITEBALANCEMODE_AUTO,
                                                        3,
                                                        &error,
                                                        NULL),
             "gdigicam-manager: an error has not happened.");
    fail_if (NULL == error,
             "gdigicam-manager: error was not set.");
    fail_if (!g_error_matches (error,
                               G_DIGICAM_ERROR,
                               G_DIGICAM_ERROR_GSTREAMER_BIN_NOT_SET),
             "gdigicam-manager: error is  \n\"%s\"\n instead of "
             "\"exposure mode not supported\".",
             error->message);
    g_error_free (error);
    error = NULL;

    fail_if (g_digicam_manager_get_white_balance_mode
             (no_featured_manager,
              &gotten_white_balance_mode,
              &gotten_level,
              &error),
             "gdigicam-manager: an error has not happened.");
    fail_if (0 != gotten_white_balance_mode,
             "gdigicam-manager: the exposure mode was not the \"unset\" "
             "value, it was \"%d\".",
             gotten_white_balance_mode);
    fail_if (0 != gotten_level,
             "gdigicam-manager: the compensation was not the \"unset\" "
             "value, it was \"%d\".",
             gotten_level);
    fail_if (NULL == error,
             "gdigicam-manager: error was not set.");
    fail_if (!g_error_matches (error,
                               G_DIGICAM_ERROR,
                               G_DIGICAM_ERROR_GSTREAMER_BIN_NOT_SET),
             "gdigicam-manager: error is  \n\"%s\"\n instead of "
             "\"exposure mode not supported\".",
             error->message);
    g_error_free (error);
    error = NULL;
    gotten_white_balance_mode = 0;
    gotten_level = 0;

    /* Test 2 */
    g_digicam_manager_set_gstreamer_bin (no_featured_manager,
                                         no_featured_camera_bin,
                                         no_featured_descriptor,
                                         NULL);
    fail_if (g_digicam_manager_set_white_balance_mode (no_featured_manager,
                                                        G_DIGICAM_WHITEBALANCEMODE_AUTO,
                                                        3,
                                                        &error,
                                                        NULL),
             "gdigicam-manager: an error has not happened.");
    fail_if (NULL == error,
             "gdigicam-manager: error was not set.");
    fail_if (!g_error_matches (error,
                               G_DIGICAM_ERROR,
                               G_DIGICAM_ERROR_WHITEBALANCEMODE_NOT_SUPPORTED),
             "gdigicam-manager: error is  \n\"%s\"\n instead of "
             "\"white balance mode not supported\".",
             error->message);
    g_error_free (error);
    error = NULL;

    fail_if (g_digicam_manager_get_white_balance_mode
             (no_featured_manager,
              &gotten_white_balance_mode,
              &gotten_level,
              &error),
             "gdigicam-manager: an error has not happened.");
    fail_if (0 != gotten_white_balance_mode,
             "gdigicam-manager: the white balance mode was not the \"unset\" "
             "value, it was \"%d\".",
             gotten_white_balance_mode);
    fail_if (0 != gotten_level,
             "gdigicam-manager: the compensation was not the \"unset\" "
             "value, it was \"%d\".",
             gotten_level);
    fail_if (NULL == error,
             "gdigicam-manager: error was not set.");
    fail_if (!g_error_matches (error,
                               G_DIGICAM_ERROR,
                               G_DIGICAM_ERROR_WHITEBALANCEMODE_NOT_SUPPORTED),
             "gdigicam-manager: error is  \n\"%s\"\n instead of "
             "\"white balance mode not supported\".",
             error->message);
    g_error_free (error);
    error = NULL;
    gotten_white_balance_mode = 0;
    gotten_level = 0;

    /* Test 3 */
    g_digicam_manager_set_gstreamer_bin (minimum_featured_manager,
                                         minimum_featured_camera_bin,
                                         minimum_featured_descriptor,
                                         NULL);
    fail_if (g_digicam_manager_set_white_balance_mode (minimum_featured_manager,
                                                        G_DIGICAM_WHITEBALANCEMODE_AUTO,
                                                        3,
                                                        &error,
                                                        NULL),
             "gdigicam-manager: an error has not happened.");
    fail_if (NULL == error,
             "gdigicam-manager: error was not set.");
    fail_if (!g_error_matches (error,
                               G_DIGICAM_ERROR,
                               G_DIGICAM_ERROR_WHITEBALANCEMODE_NOT_SUPPORTED),
             "gdigicam-manager: error is  \n\"%s\"\n instead of "
             "\"exposure mode not supported\".",
             error->message);
    g_error_free (error);
    error = NULL;

    fail_if (g_digicam_manager_get_white_balance_mode
             (minimum_featured_manager,
              &gotten_white_balance_mode,
              &gotten_level,
              &error),
             "gdigicam-manager: an error has not happened.");
    fail_if (0 != gotten_white_balance_mode,
             "gdigicam-manager: the white balance mode was not the \"unset\" "
             "value, it was \"%d\".",
             gotten_white_balance_mode);
    fail_if (0 != gotten_level,
             "gdigicam-manager: the compensation was not the \"unset\" "
             "value, it was \"%d\".",
             gotten_level);
    fail_if (NULL == error,
             "gdigicam-manager: error was not set.");
    fail_if (!g_error_matches (error,
                               G_DIGICAM_ERROR,
                               G_DIGICAM_ERROR_WHITEBALANCEMODE_NOT_SUPPORTED),
             "gdigicam-manager: error is  \n\"%s\"\n instead of "
             "\"white balance mode not supported\".",
             error->message);
    g_error_free (error);
    error = NULL;
    gotten_white_balance_mode = 0;
    gotten_level = 0;

    /* Test 4 */
    g_digicam_manager_set_gstreamer_bin (full_featured_manager,
                                         full_featured_camera_bin,
                                         full_featured_descriptor,
                                         NULL);
    fail_if (!g_digicam_manager_get_white_balance_mode
             (full_featured_manager,
              &gotten_white_balance_mode,
              &gotten_level,
              &error),
             "gdigicam-manager: an error has happened.");
    fail_if (0 != gotten_white_balance_mode,
             "gdigicam-manager: the white balance mode was not the \"unset\" "
             "value, it was \"%d\".",
             gotten_white_balance_mode);
    fail_if (0 != gotten_level,
             "gdigicam-manager: the compensation was not the \"unset\" "
             "value, it was \"%d\".",
             gotten_level);
    fail_if (NULL != error,
             "gdigicam-manager: error was set.");
}
END_TEST

/**
 * Purpose: test setting and getting a valid white balance mode in a
 * #GDigicamManager
 * Cases considered:
 *    - set/get white balance mode to a featured #GDigicamManager.
 *    - set/get again another whiet balance mode to a featured #GDigicamManager.
 */
START_TEST (test_set_get_white_balance_mode_regular)
{
    GDigicamWhitebalancemode gotten_white_balance_mode = 0;
    guint gotten_level = 0;

    /* Test 1 */
    g_digicam_manager_set_gstreamer_bin (full_featured_manager,
                                         full_featured_camera_bin,
                                         full_featured_descriptor,
                                         NULL);
    fail_if (!g_digicam_manager_set_white_balance_mode (full_featured_manager,
                                                   G_DIGICAM_WHITEBALANCEMODE_MANUAL,
						   WHITE_BALANCE_TEST_VALUE,
                                                   &error,
                                                   NULL),
             "gdigicam-manager: an error has happened.");
    fail_if (NULL != error,
             "gdigicam-manager: error was set.");

    fail_if (!g_digicam_manager_get_white_balance_mode
             (full_featured_manager,
              &gotten_white_balance_mode,
              &gotten_level,
              &error),
             "gdigicam-manager: an error has happened.");
    fail_if (G_DIGICAM_WHITEBALANCEMODE_MANUAL != gotten_white_balance_mode,
             "gdigicam-manager: the white_balance mode was not "
             "the previously provided.");
    fail_if (WHITE_BALANCE_TEST_VALUE != gotten_level,
             "gdigicam-manager: the level was not the \"unset\" "
             "value, it was \"%d\".",
             gotten_level);
    fail_if (NULL != error,
             "gdigicam-manager: error was set.");
    gotten_white_balance_mode = 0;
    gotten_level = 0;

    /* Test 2 */
    fail_if (!g_digicam_manager_set_white_balance_mode (full_featured_manager,
                                                   G_DIGICAM_WHITEBALANCEMODE_AUTO,
                                                   WHITE_BALANCE_TEST_VALUE,
                                                   &error,
                                                   NULL),
             "gdigicam-manager: an error has happened.");
    fail_if (NULL != error,
             "gdigicam-manager: error was set.");

    fail_if (!g_digicam_manager_get_white_balance_mode
             (full_featured_manager,
              &gotten_white_balance_mode,
              &gotten_level,
              &error),
             "gdigicam-manager: an error has happened.");
    fail_if (G_DIGICAM_WHITEBALANCEMODE_AUTO != gotten_white_balance_mode,
             "gdigicam-manager: the white_balance mode was not "
             "the previously provided.");
    /* TODO: this test case is failing. Can we actually set a value
     * when white balance mode is auto ?*/
    fail_if (WHITE_BALANCE_TEST_VALUE != gotten_level,
             "gdigicam-manager: the level was not "
             "the previously provided.");
    fail_if (NULL != error,
             "gdigicam-manager: error was set.");
}
END_TEST

/**
 * Purpose: test setting and getting a white balance mode in an invalid
 * #GDigicamManager
 * Cases considered:
 *    - set/get the white balance mode to/from a NULL
 *      #GDigicamManager. Parameters remain the same.
 *    - set/get the white balance mode to/from an invalid
 *      #GDigicamManager. Parameters remain the same.
 */
START_TEST (test_set_get_white_balance_mode_invalid)
{
    GDigicamWhitebalancemode gotten_white_balance_mode = 0;
    guint gotten_level = 0;

    /* Test 1 */
    fail_if (g_digicam_manager_set_white_balance_mode (NULL,
                                                  G_DIGICAM_WHITEBALANCEMODE_AUTO,
                                                  3,
                                                  &error,
                                                  NULL),
             "gdigicam-manager: there was no error.");
    fail_if (NULL != error,
             "gdigicam-manager: error has changed.");

    fail_if (g_digicam_manager_get_white_balance_mode
             (NULL, &gotten_white_balance_mode, &gotten_level, &error),
             "gdigicam-manager: there was no error.");
    fail_if (0 != gotten_white_balance_mode,
             "gdigicam-manager: the white_balance mode was not the \"unset\" "
             "value, it was \"%d\".",
             gotten_white_balance_mode);
    fail_if (0 != gotten_level,
             "gdigicam-manager: the level was not the \"unset\" "
             "value, it was \"%d\".",
             gotten_level);
    fail_if (NULL != error,
             "gdigicam-manager: error has changed.");
    gotten_white_balance_mode = 0;
    gotten_level = 0;

    /* Test 2 */
    fail_if (g_digicam_manager_set_white_balance_mode (G_DIGICAM_MANAGER (window),
                                                  G_DIGICAM_WHITEBALANCEMODE_AUTO,
                                                  3,
                                                  &error,
                                                  NULL),
             "gdigicam-manager: there was no error.");
    fail_if (NULL != error,
             "gdigicam-manager: error has changed.");

    fail_if (g_digicam_manager_get_white_balance_mode
             (G_DIGICAM_MANAGER (window),
              &gotten_white_balance_mode,
              &gotten_level,
              &error),
             "gdigicam-manager: there was no error.");
    fail_if (0 != gotten_white_balance_mode,
             "gdigicam-manager: the white_balance mode was not the \"unset\" "
             "value, it was \"%d\".",
             gotten_white_balance_mode);
    fail_if (0 != gotten_level,
             "gdigicam-manager: the level was not the \"unset\" "
             "value, it was \"%d\".",
             gotten_level);
    fail_if (NULL != error,
             "gdigicam-manager: error has changed.");
    gotten_white_balance_mode = 0;
}
END_TEST

/* ----- Test case for set/get_exposure_mode -----*/

/**
 * Purpose: test setting and getting limit exposure modes in a
 * #GDigicamManager
 * Cases considered:
 *    - set/get exposure mode from just created #GDigicamManager.
 *    - set/get exposure mode to a none featured #GDigicamManager.
 *    - set/get exposure mode to a minimum featured #GDigicamManager.
 *    - get exposure mode from a featured but not set #GDigicamManager.
 */
START_TEST (test_set_get_exposure_mode_limit)
{
    GDigicamExposuremode gotten_exposure_mode = 0;

    /* Test 1 */
    fail_if (g_digicam_manager_set_exposure_mode (no_featured_manager,
                                                  G_DIGICAM_EXPOSUREMODE_AUTO,
                                                  NULL,
                                                  &error,
                                                  NULL),
             "gdigicam-manager: an error has not happened.");
    fail_if (NULL == error,
             "gdigicam-manager: error was not set.");
    fail_if (!g_error_matches (error,
                               G_DIGICAM_ERROR,
                               G_DIGICAM_ERROR_GSTREAMER_BIN_NOT_SET),
             "gdigicam-manager: error is  \n\"%s\"\n instead of "
             "\"exposure mode not supported\".",
             error->message);
    g_error_free (error);
    error = NULL;

    fail_if (g_digicam_manager_get_exposure_mode
             (no_featured_manager,
              &gotten_exposure_mode,
              &error),
             "gdigicam-manager: an error has not happened.");
    fail_if (0 != gotten_exposure_mode,
             "gdigicam-manager: the exposure mode was not the \"unset\" "
             "value, it was \"%d\".",
             gotten_exposure_mode);
    fail_if (NULL == error,
             "gdigicam-manager: error was not set.");
    fail_if (!g_error_matches (error,
                               G_DIGICAM_ERROR,
                               G_DIGICAM_ERROR_GSTREAMER_BIN_NOT_SET),
             "gdigicam-manager: error is  \n\"%s\"\n instead of "
             "\"exposure mode not supported\".",
             error->message);
    g_error_free (error);
    error = NULL;
    gotten_exposure_mode = 0;

    /* Test 2 */
    g_digicam_manager_set_gstreamer_bin (no_featured_manager,
                                         no_featured_camera_bin,
                                         no_featured_descriptor,
                                         NULL);
    fail_if (g_digicam_manager_set_exposure_mode (no_featured_manager,
                                                  G_DIGICAM_EXPOSUREMODE_AUTO,
                                                  NULL,
                                                  &error,
                                                  NULL),
             "gdigicam-manager: an error has not happened.");
    fail_if (NULL == error,
             "gdigicam-manager: error was not set.");
    fail_if (!g_error_matches (error,
                               G_DIGICAM_ERROR,
                               G_DIGICAM_ERROR_EXPOSUREMODE_NOT_SUPPORTED),
             "gdigicam-manager: error is  \n\"%s\"\n instead of "
             "\"exposure mode not supported\".",
             error->message);
    g_error_free (error);
    error = NULL;

    fail_if (g_digicam_manager_get_exposure_mode
             (no_featured_manager,
              &gotten_exposure_mode,
              &error),
             "gdigicam-manager: an error has not happened.");
    fail_if (0 != gotten_exposure_mode,
             "gdigicam-manager: the exposure mode was not the \"unset\" "
             "value, it was \"%d\".",
             gotten_exposure_mode);
    fail_if (NULL == error,
             "gdigicam-manager: error was not set.");
    fail_if (!g_error_matches (error,
                               G_DIGICAM_ERROR,
                               G_DIGICAM_ERROR_EXPOSUREMODE_NOT_SUPPORTED),
             "gdigicam-manager: error is  \n\"%s\"\n instead of "
             "\"exposure mode not supported\".",
             error->message);
    g_error_free (error);
    error = NULL;
    gotten_exposure_mode = 0;

    /* Test 3 */
    g_digicam_manager_set_gstreamer_bin (minimum_featured_manager,
                                         minimum_featured_camera_bin,
                                         minimum_featured_descriptor,
                                         NULL);
    fail_if (g_digicam_manager_set_exposure_mode (minimum_featured_manager,
                                                  G_DIGICAM_EXPOSUREMODE_AUTO,
                                                  NULL,
                                                  &error,
                                                  NULL),
             "gdigicam-manager: an error has not happened.");
    fail_if (NULL == error,
             "gdigicam-manager: error was not set.");
    fail_if (!g_error_matches (error,
                               G_DIGICAM_ERROR,
                               G_DIGICAM_ERROR_EXPOSUREMODE_NOT_SUPPORTED),
             "gdigicam-manager: error is  \n\"%s\"\n instead of "
             "\"exposure mode not supported\".",
             error->message);
    g_error_free (error);
    error = NULL;

    fail_if (g_digicam_manager_get_exposure_mode
             (minimum_featured_manager,
              &gotten_exposure_mode,
              &error),
             "gdigicam-manager: an error has not happened.");
    fail_if (0 != gotten_exposure_mode,
             "gdigicam-manager: the exposure mode was not the \"unset\" "
             "value, it was \"%d\".",
             gotten_exposure_mode);
    fail_if (NULL == error,
             "gdigicam-manager: error was not set.");
    fail_if (!g_error_matches (error,
                               G_DIGICAM_ERROR,
                               G_DIGICAM_ERROR_EXPOSUREMODE_NOT_SUPPORTED),
             "gdigicam-manager: error is  \n\"%s\"\n instead of "
             "\"exposure mode not supported\".",
             error->message);
    g_error_free (error);
    error = NULL;
    gotten_exposure_mode = 0;

    /* Test 4 */
    g_digicam_manager_set_gstreamer_bin (full_featured_manager,
                                         full_featured_camera_bin,
                                         full_featured_descriptor,
                                         NULL);
    fail_if (!g_digicam_manager_get_exposure_mode
             (full_featured_manager,
              &gotten_exposure_mode,
              &error),
             "gdigicam-manager: an error has happened.");
    fail_if (0 != gotten_exposure_mode,
             "gdigicam-manager: the exposure mode was not the \"unset\" "
             "value, it was \"%d\".",
             gotten_exposure_mode);
    fail_if (NULL != error,
             "gdigicam-manager: error was set.");
}
END_TEST

/**
 * Purpose: test setting and getting a valid exposure mode in a
 * #GDigicamManager
 * Cases considered:
 *    - set/get exposure mode to a featured #GDigicamManager.
 *    - set/get again another exposure mode to a featured #GDigicamManager.
 */
START_TEST (test_set_get_exposure_mode_regular)
{
    GDigicamExposuremode gotten_exposure_mode = 0;

    /* Test 1 */
    g_digicam_manager_set_gstreamer_bin (full_featured_manager,
                                         full_featured_camera_bin,
                                         full_featured_descriptor,
                                         NULL);
    fail_if (!g_digicam_manager_set_exposure_mode (full_featured_manager,
                                                   G_DIGICAM_EXPOSUREMODE_MANUAL,
                                                   NULL,
                                                   &error,
                                                   NULL),
             "gdigicam-manager: an error has happened.");
    fail_if (NULL != error,
             "gdigicam-manager: error was set.");

    fail_if (!g_digicam_manager_get_exposure_mode
             (full_featured_manager,
              &gotten_exposure_mode,
              &error),
             "gdigicam-manager: an error has happened.");
    fail_if (G_DIGICAM_EXPOSUREMODE_MANUAL != gotten_exposure_mode,
             "gdigicam-manager: the exposure mode was not "
             "the previously provided.");
    fail_if (NULL != error,
             "gdigicam-manager: error was set.");
    gotten_exposure_mode = 0;

    /* Test 2 */
    fail_if (!g_digicam_manager_set_exposure_mode (full_featured_manager,
                                                   G_DIGICAM_EXPOSUREMODE_AUTO,
                                                   NULL,
                                                   &error,
                                                   NULL),
             "gdigicam-manager: an error has happened.");
    fail_if (NULL != error,
             "gdigicam-manager: error was set.");

    fail_if (!g_digicam_manager_get_exposure_mode
             (full_featured_manager,
              &gotten_exposure_mode,
              &error),
             "gdigicam-manager: an error has happened.");
    fail_if (G_DIGICAM_EXPOSUREMODE_AUTO != gotten_exposure_mode,
             "gdigicam-manager: the exposure mode was not "
             "the previously provided.");
    fail_if (NULL != error,
             "gdigicam-manager: error was set.");
}
END_TEST

/**
 * Purpose: test setting and getting a exposure mode in an invalid
 * #GDigicamManager
 * Cases considered:
 *    - set/get the exposure mode to/from a NULL
 *      #GDigicamManager. Parameters remain the same.
 *    - set/get the exposure mode to/from an invalid
 *      #GDigicamManager. Parameters remain the same.
 */
START_TEST (test_set_get_exposure_mode_invalid)
{
    GDigicamExposuremode gotten_exposure_mode = 0;

    /* Test 1 */
    fail_if (g_digicam_manager_set_exposure_mode (NULL,
                                                  G_DIGICAM_EXPOSUREMODE_AUTO,
                                                  NULL,
                                                  &error,
                                                  NULL),
             "gdigicam-manager: there was no error.");
    fail_if (NULL != error,
             "gdigicam-manager: error has changed.");

    fail_if (g_digicam_manager_get_exposure_mode
             (NULL, &gotten_exposure_mode, &error),
             "gdigicam-manager: there was no error.");
    fail_if (0 != gotten_exposure_mode,
             "gdigicam-manager: the exposure mode was not the \"unset\" "
             "value, it was \"%d\".",
             gotten_exposure_mode);
    fail_if (NULL != error,
             "gdigicam-manager: error has changed.");
    gotten_exposure_mode = 0;

    /* Test 2 */
    fail_if (g_digicam_manager_set_exposure_mode (G_DIGICAM_MANAGER (window),
                                                  G_DIGICAM_EXPOSUREMODE_AUTO,
                                                  NULL,
                                                  &error,
                                                  NULL),
             "gdigicam-manager: there was no error.");
    fail_if (NULL != error,
             "gdigicam-manager: error has changed.");

    fail_if (g_digicam_manager_get_exposure_mode
             (G_DIGICAM_MANAGER (window),
              &gotten_exposure_mode,
              &error),
             "gdigicam-manager: there was no error.");
    fail_if (0 != gotten_exposure_mode,
             "gdigicam-manager: the exposure mode was not the \"unset\" "
             "value, it was \"%d\".",
             gotten_exposure_mode);
    fail_if (NULL != error,
             "gdigicam-manager: error has changed.");
    gotten_exposure_mode = 0;
}
END_TEST

/* ----- Test case for set/get_metering_mode -----*/

/**
 * Purpose: test setting and getting limit metering modes in a
 * #GDigicamManager
 * Cases considered:
 *    - set/get metering mode from just created #GDigicamManager.
 *    - set/get metering mode to a none featured #GDigicamManager.
 *    - set/get metering mode to a minimum featured #GDigicamManager.
 *    - get metering mode from a featured but not set #GDigicamManager.
 */
START_TEST (test_set_get_metering_mode_limit)
{
    GDigicamMeteringmode gotten_metering_mode = 0;

    /* Test 1 */
    fail_if (g_digicam_manager_set_metering_mode (no_featured_manager,
                                                   G_DIGICAM_METERINGMODE_AVERAGE,
                                                   &error,
                                                   NULL),
             "gdigicam-manager: an error has not happened.");
    fail_if (NULL == error,
             "gdigicam-manager: error was not set.");
    fail_if (!g_error_matches (error,
                               G_DIGICAM_ERROR,
                               G_DIGICAM_ERROR_GSTREAMER_BIN_NOT_SET),
             "gdigicam-manager: error is  \n\"%s\"\n instead of "
             "\"metering mode not supported\".",
             error->message);
    g_error_free (error);
    error = NULL;

    fail_if (g_digicam_manager_get_metering_mode
             (no_featured_manager, &gotten_metering_mode, &error),
             "gdigicam-manager: an error has not happened.");
    fail_if (0 != gotten_metering_mode,
             "gdigicam-manager: the metering mode was not the \"unset\" "
             "value, it was \"%d\".",
             gotten_metering_mode);
    fail_if (NULL == error,
             "gdigicam-manager: error was not set.");
    fail_if (!g_error_matches (error,
                               G_DIGICAM_ERROR,
                               G_DIGICAM_ERROR_GSTREAMER_BIN_NOT_SET),
             "gdigicam-manager: error is  \n\"%s\"\n instead of "
             "\"metering mode not supported\".",
             error->message);
    g_error_free (error);
    error = NULL;
    gotten_metering_mode = 0;

    /* Test 2 */
    g_digicam_manager_set_gstreamer_bin (no_featured_manager,
                                         no_featured_camera_bin,
                                         no_featured_descriptor,
                                         NULL);
    fail_if (g_digicam_manager_set_metering_mode (no_featured_manager,
                                                   G_DIGICAM_METERINGMODE_AVERAGE,
                                                   &error,
                                                   NULL),
             "gdigicam-manager: an error has not happened.");
    fail_if (NULL == error,
             "gdigicam-manager: error was not set.");
    fail_if (!g_error_matches (error,
                               G_DIGICAM_ERROR,
                               G_DIGICAM_ERROR_METERINGMODE_NOT_SUPPORTED),
             "gdigicam-manager: error is  \n\"%s\"\n instead of "
             "\"metering mode not supported\".",
             error->message);
    g_error_free (error);
    error = NULL;

    fail_if (g_digicam_manager_get_metering_mode
             (no_featured_manager, &gotten_metering_mode, &error),
             "gdigicam-manager: an error has not happened.");
    fail_if (0 != gotten_metering_mode,
             "gdigicam-manager: the metering mode was not the \"unset\" "
             "value, it was \"%d\".",
             gotten_metering_mode);
    fail_if (NULL == error,
             "gdigicam-manager: error was not set.");
    fail_if (!g_error_matches (error,
                               G_DIGICAM_ERROR,
                               G_DIGICAM_ERROR_METERINGMODE_NOT_SUPPORTED),
             "gdigicam-manager: error is  \n\"%s\"\n instead of "
             "\"metering mode not supported\".",
             error->message);
    g_error_free (error);
    error = NULL;
    gotten_metering_mode = 0;

    /* Test 3 */
    g_digicam_manager_set_gstreamer_bin (minimum_featured_manager,
                                         minimum_featured_camera_bin,
                                         minimum_featured_descriptor,
                                         NULL);
    fail_if (g_digicam_manager_set_metering_mode (minimum_featured_manager,
                                                   G_DIGICAM_METERINGMODE_AVERAGE,
                                                   &error,
                                                   NULL),
             "gdigicam-manager: an error has not happened.");
    fail_if (NULL == error,
             "gdigicam-manager: error was not set.");
    fail_if (!g_error_matches (error,
                               G_DIGICAM_ERROR,
                               G_DIGICAM_ERROR_METERINGMODE_NOT_SUPPORTED),
             "gdigicam-manager: error is  \n\"%s\"\n instead of "
             "\"metering mode not supported\".",
             error->message);
    g_error_free (error);
    error = NULL;

    fail_if (g_digicam_manager_get_metering_mode
             (minimum_featured_manager, &gotten_metering_mode, &error),
             "gdigicam-manager: an error has not happened.");
    fail_if (0 != gotten_metering_mode,
             "gdigicam-manager: the metering mode was not the \"unset\" "
             "value, it was \"%d\".",
             gotten_metering_mode);
    fail_if (NULL == error,
             "gdigicam-manager: error was not set.");
    fail_if (!g_error_matches (error,
                               G_DIGICAM_ERROR,
                               G_DIGICAM_ERROR_METERINGMODE_NOT_SUPPORTED),
             "gdigicam-manager: error is  \n\"%s\"\n instead of "
             "\"metering mode not supported\".",
             error->message);
    g_error_free (error);
    error = NULL;
    gotten_metering_mode = 0;

    /* Test 4 */
    g_digicam_manager_set_gstreamer_bin (full_featured_manager,
                                         full_featured_camera_bin,
                                         full_featured_descriptor,
                                         NULL);
    fail_if (!g_digicam_manager_get_metering_mode
             (full_featured_manager, &gotten_metering_mode, &error),
             "gdigicam-manager: an error has happened.");
    fail_if (0 != gotten_metering_mode,
             "gdigicam-manager: the mode was not the \"unset\" "
             "value, it was \"%d\".",
             gotten_metering_mode);
    fail_if (NULL != error,
             "gdigicam-manager: error was set.");
}
END_TEST

/**
 * Purpose: test setting and getting a valid metering mode in a
 * #GDigicamManager
 * Cases considered:
 *    - set/get metering mode to a featured #GDigicamManager.
 *    - set/get again another metering mode to a featured #GDigicamManager.
 */
START_TEST (test_set_get_metering_mode_regular)
{
    GDigicamMeteringmode gotten_metering_mode = 0;


    /* Test 1 */
    g_digicam_manager_set_gstreamer_bin (full_featured_manager,
                                         full_featured_camera_bin,
                                         full_featured_descriptor,
                                         NULL);
    fail_if (!g_digicam_manager_set_metering_mode (full_featured_manager,
                                         G_DIGICAM_METERINGMODE_AVERAGE,
                                         &error,
                                         NULL),
             "gdigicam-manager: an error has happened.");
    fail_if (NULL != error,
             "gdigicam-manager: error was set.");

    fail_if (!g_digicam_manager_get_metering_mode
             (full_featured_manager, &gotten_metering_mode, &error),
             "gdigicam-manager: an error has happened.");
    fail_if (G_DIGICAM_METERINGMODE_AVERAGE != gotten_metering_mode,
             "gdigicam-manager: the metering mode was not "
             "the previously provided.");
    fail_if (NULL != error,
             "gdigicam-manager: error was set.");
    gotten_metering_mode = 0;

    /* Test 2 */
    fail_if (!g_digicam_manager_set_metering_mode (full_featured_manager,
                                                   G_DIGICAM_METERINGMODE_SPOT,
                                                   &error,
                                                   NULL),
             "gdigicam-manager: an error has happened.");
    fail_if (NULL != error,
             "gdigicam-manager: error was set.");

    fail_if (!g_digicam_manager_get_metering_mode
             (full_featured_manager, &gotten_metering_mode, &error),
             "gdigicam-manager: an error has happened.");
    fail_if (G_DIGICAM_METERINGMODE_SPOT != gotten_metering_mode,
             "gdigicam-manager: the metering mode was not "
             "the previously provided.");
    fail_if (NULL != error,
             "gdigicam-manager: error was set.");
}
END_TEST

/**
 * Purpose: test setting and getting a metering mode in an invalid
 * #GDigicamManager
 * Cases considered:
 *    - set/get the metering mode to/from a NULL
 *      #GDigicamManager. Parameters remain the same.
 *    - set/get the metering mode to/from an invalid
 *      #GDigicamManager. Parameters remain the same.
 */
START_TEST (test_set_get_metering_mode_invalid)
{
    GDigicamMeteringmode gotten_metering_mode = 0;

    /* Test 1 */
    fail_if (g_digicam_manager_set_metering_mode (NULL,
                                                  G_DIGICAM_METERINGMODE_AVERAGE,
                                                  &error,
                                                  NULL),
             "gdigicam-manager: there was no error.");
    fail_if (NULL != error,
             "gdigicam-manager: error has changed.");

    fail_if (g_digicam_manager_get_metering_mode
             (NULL, &gotten_metering_mode, &error),
             "gdigicam-manager: there was no error.");
    fail_if (0 != gotten_metering_mode,
             "gdigicam-manager: the metering mode was not the \"unset\" "
             "value, it was \"%d\".",
             gotten_metering_mode);
    fail_if (NULL != error,
             "gdigicam-manager: error has changed.");
    gotten_metering_mode = 0;

    /* Test 2 */
    fail_if (g_digicam_manager_set_metering_mode (G_DIGICAM_MANAGER (window),
                                                  G_DIGICAM_METERINGMODE_AVERAGE,
                                                  &error,
                                                  NULL),
             "gdigicam-manager: there was no error.");
    fail_if (NULL != error,
             "gdigicam-manager: error has changed.");

    fail_if (g_digicam_manager_get_metering_mode
             (G_DIGICAM_MANAGER (window), &gotten_metering_mode, &error),
             "gdigicam-manager: there was no error.");
    fail_if (0 != gotten_metering_mode,
             "gdigicam-manager: the metering mode was not the \"unset\" "
             "value, it was \"%d\".",
             gotten_metering_mode);
    fail_if (NULL != error,
             "gdigicam-manager: error has changed.");
    gotten_metering_mode = 0;
}
END_TEST

/* ----- Test case for set/get_aspect_ratio -----*/


/**
 * Purpose: test setting and getting a valid aspect ratio in a
 * #GDigicamManager
 * Cases considered:
 *    - set/get aspect ratio to a featured #GDigicamManager.
 *    - set/get again another aspect ratio to a featured #GDigicamManager.
 */
START_TEST (test_set_get_aspect_ratio_regular)
{
    GDigicamAspectratio gotten_aspect_ratio = 0;

    /* Test 1 */
    g_digicam_manager_set_gstreamer_bin (full_featured_manager,
                                         full_featured_camera_bin,
                                         full_featured_descriptor,
                                         NULL);
    fail_if (!g_digicam_manager_set_aspect_ratio (full_featured_manager,
                                                  G_DIGICAM_ASPECTRATIO_4X3,
                                                  &error,
                                                  NULL),
             "gdigicam-manager: an error has happened.");

    fail_if (!g_digicam_manager_get_aspect_ratio
             (full_featured_manager, &gotten_aspect_ratio, &error),
             "gdigicam-manager: an error has happened.");
    fail_if (G_DIGICAM_ASPECTRATIO_4X3 != gotten_aspect_ratio,
             "gdigicam-manager: the aspect ratio was not "
             "the previously provided.");
    gotten_aspect_ratio = 0;

    /* Test 2 */
    fail_if (!g_digicam_manager_set_aspect_ratio (full_featured_manager,
                                                  G_DIGICAM_ASPECTRATIO_16X9,
                                                  &error,
                                                  NULL),
             "gdigicam-manager: an error has happened.");

    fail_if (!g_digicam_manager_get_aspect_ratio
             (full_featured_manager, &gotten_aspect_ratio, &error),
             "gdigicam-manager: an error has happened.");
    fail_if (G_DIGICAM_ASPECTRATIO_16X9 != gotten_aspect_ratio,
             "gdigicam-manager: the aspect ratio was not "
             "the previously provided.");
}
END_TEST

/**
 * Purpose: test setting and getting an aspect ratio in an invalid
 * #GDigicamManager
 * Cases considered:
 *    - set/get the aspect ratio to/from a NULL
 *      #GDigicamManager. Parameters remain the same.
 *    - set/get the aspect ratio to/from an invalid
 *      #GDigicamManager. Parameters remain the same.
 */
START_TEST (test_set_get_aspect_ratio_invalid)
{
    GDigicamAspectratio gotten_aspect_ratio = 0;

    /* Test 1 */
    fail_if (g_digicam_manager_set_aspect_ratio (NULL,
                                                 G_DIGICAM_ASPECTRATIO_4X3,
                                                 &error,
                                                 NULL),
             "gdigicam-manager: there was no error.");
    fail_if (NULL != error,
             "gdigicam-manager: error has changed.");

    fail_if (g_digicam_manager_get_aspect_ratio
             (NULL, &gotten_aspect_ratio, &error),
             "gdigicam-manager: there was no error.");
    fail_if (0 != gotten_aspect_ratio,
             "gdigicam-manager: the aspect ratio was not the \"unset\" "
             "value, it was \"%d\".",
             gotten_aspect_ratio);
    fail_if (NULL != error,
             "gdigicam-manager: error has changed.");
    gotten_aspect_ratio = 0;

    /* Test 2 */
    fail_if (g_digicam_manager_set_aspect_ratio (G_DIGICAM_MANAGER (window),
                                                 G_DIGICAM_ASPECTRATIO_4X3,
                                                 &error,
                                                 NULL),
             "gdigicam-manager: there was no error.");
    fail_if (NULL != error,
             "gdigicam-manager: error has changed.");

    fail_if (g_digicam_manager_get_aspect_ratio
             (G_DIGICAM_MANAGER (window), &gotten_aspect_ratio, &error),
             "gdigicam-manager: there was no error.");
    fail_if (0 != gotten_aspect_ratio,
             "gdigicam-manager: the aspect ratio was not the \"unset\" "
             "value, it was \"%d\".",
             gotten_aspect_ratio);
    fail_if (NULL != error,
             "gdigicam-manager: error has changed.");
    gotten_aspect_ratio = 0;
}
END_TEST

/* ----- Test case for set/get_resolution -----*/

/**
 * Purpose: test setting and getting limit resolutions in a
 * #GDigicamManager
 * Cases considered:
 *    - set/get resolution from just created #GDigicamManager.
 *    - set/get resolution to a none featured #GDigicamManager.
 *    - set/get resolution to a minimum featured #GDigicamManager.
 *    - get resolution from a featured but not set #GDigicamManager.
 */
START_TEST (test_set_get_resolution_limit)
{
    GDigicamResolution gotten_resolution = 0;

    /* Test 1 */
    fail_if (g_digicam_manager_set_resolution (no_featured_manager,
                                                G_DIGICAM_RESOLUTION_HIGH,
                                                &error,
                                                NULL),
             "gdigicam-manager: an error has not happened.");
    fail_if (NULL == error,
             "gdigicam-manager: error was not set.");
    fail_if (!g_error_matches (error,
                               G_DIGICAM_ERROR,
                               G_DIGICAM_ERROR_GSTREAMER_BIN_NOT_SET),
             "gdigicam-manager: error is  \n\"%s\"\n instead of "
             "\"bin not set\".",
             error->message);
    g_error_free (error);
    error = NULL;

    fail_if (g_digicam_manager_get_resolution
             (no_featured_manager, &gotten_resolution, &error),
             "gdigicam-manager: an error has not happened.");
    fail_if (0 != gotten_resolution,
             "gdigicam-manager: the resolution was not the \"unset\" "
             "value, it was \"%d\".",
             gotten_resolution);
    fail_if (NULL == error,
             "gdigicam-manager: error was not set.");
    fail_if (!g_error_matches (error,
                               G_DIGICAM_ERROR,
                               G_DIGICAM_ERROR_GSTREAMER_BIN_NOT_SET),
             "gdigicam-manager: error is  \n\"%s\"\n instead of "
             "\"bin not set\".",
             error->message);
    g_error_free (error);
    error = NULL;
    gotten_resolution = 0;

    /* Test 2 */
    g_digicam_manager_set_gstreamer_bin (no_featured_manager,
                                         no_featured_camera_bin,
                                         no_featured_descriptor,
                                         NULL);
    fail_if (g_digicam_manager_set_resolution (no_featured_manager,
                                                G_DIGICAM_RESOLUTION_HIGH,
                                                &error,
                                                NULL),
             "gdigicam-manager: an error has not happened.");
    fail_if (NULL == error,
             "gdigicam-manager: error was not set.");
    fail_if (!g_error_matches (error,
                               G_DIGICAM_ERROR,
                               G_DIGICAM_ERROR_RESOLUTION_NOT_SUPPORTED),
             "gdigicam-manager: error is  \n\"%s\"\n instead of "
             "\"resolution not supported\".",
             error->message);
    g_error_free (error);
    error = NULL;

    fail_if (g_digicam_manager_get_resolution
             (no_featured_manager, &gotten_resolution, &error),
             "gdigicam-manager: an error has not happened.");
    fail_if (0 != gotten_resolution,
             "gdigicam-manager: the resolution was not the \"unset\" "
             "value, it was \"%d\".",
             gotten_resolution);
    fail_if (NULL == error,
             "gdigicam-manager: error was not set.");
    fail_if (!g_error_matches (error,
                               G_DIGICAM_ERROR,
                               G_DIGICAM_ERROR_RESOLUTION_NOT_SUPPORTED),
             "gdigicam-manager: error is  \n\"%s\"\n instead of "
             "\"resolution not supported\".",
             error->message);
    g_error_free (error);
    error = NULL;

    /* Test 3 */
    g_digicam_manager_set_gstreamer_bin (minimum_featured_manager,
                                         minimum_featured_camera_bin,
                                         minimum_featured_descriptor,
                                         NULL);
    fail_if (g_digicam_manager_set_resolution (minimum_featured_manager,
                                                G_DIGICAM_RESOLUTION_HIGH,
                                                &error,
                                                NULL),
             "gdigicam-manager: an error has not happened.");
    fail_if (NULL == error,
             "gdigicam-manager: error was not set.");
    fail_if (!g_error_matches (error,
                               G_DIGICAM_ERROR,
                               G_DIGICAM_ERROR_RESOLUTION_NOT_SUPPORTED),
             "gdigicam-manager: error is  \n\"%s\"\n instead of "
             "\"resolution not supported\".",
             error->message);
    g_error_free (error);
    error = NULL;

    fail_if (g_digicam_manager_get_resolution
             (minimum_featured_manager, &gotten_resolution, &error),
             "gdigicam-manager: an error has not happened.");
    fail_if (0 != gotten_resolution,
             "gdigicam-manager: the resolution was not the \"unset\" "
             "value, it was \"%d\".",
             gotten_resolution);
    fail_if (NULL == error,
             "gdigicam-manager: error was not set.");
    fail_if (!g_error_matches (error,
                               G_DIGICAM_ERROR,
                               G_DIGICAM_ERROR_RESOLUTION_NOT_SUPPORTED),
             "gdigicam-manager: error is  \n\"%s\"\n instead of "
             "\"resolution not supported\".",
             error->message);
    g_error_free (error);
    error = NULL;

    /* Test 4 */
    g_digicam_manager_set_gstreamer_bin (full_featured_manager,
                                         full_featured_camera_bin,
                                         full_featured_descriptor,
                                         NULL);
    fail_if (!g_digicam_manager_get_resolution
             (full_featured_manager, &gotten_resolution, &error),
             "gdigicam-manager: an error has happened.");
    fail_if (0 != gotten_resolution,
             "gdigicam-manager: the resolution was not the \"unset\" "
             "value, it was \"%d\".",
             gotten_resolution);
    fail_if (NULL != error,
             "gdigicam-manager: error was set.");
}
END_TEST

/**
 * Purpose: test setting and getting a valid resolution in a
 * #GDigicamManager
 * Cases considered:
 *    - set/get resolution to a featured #GDigicamManager.
 *    - set/get again another resolution to a featured #GDigicamManager.
 */
START_TEST (test_set_get_resolution_regular)
{
    GDigicamResolution gotten_resolution = 0;

    /* Test 1 */
    g_digicam_manager_set_gstreamer_bin (full_featured_manager,
                                         full_featured_camera_bin,
                                         full_featured_descriptor,
                                         NULL);
    fail_if (!g_digicam_manager_set_resolution (full_featured_manager,
                                                G_DIGICAM_RESOLUTION_HIGH,
                                                &error,
                                                NULL),
             "gdigicam-manager: an error has happened.");
    fail_if (NULL != error,
             "gdigicam-manager: error was set.");

    fail_if (!g_digicam_manager_get_resolution
             (full_featured_manager, &gotten_resolution, &error),
             "gdigicam-manager: an error has happened.");
    fail_if (G_DIGICAM_RESOLUTION_HIGH != gotten_resolution,
             "gdigicam-manager: the resolution was not "
             "the previously provided.");
    fail_if (NULL != error,
             "gdigicam-manager: error was set.");
    gotten_resolution = 0;

    /* Test 2 */
    fail_if (!g_digicam_manager_set_resolution (full_featured_manager,
                                                G_DIGICAM_RESOLUTION_LOW,
                                                &error,
                                                NULL),
             "gdigicam-manager: an error has happened.");
    fail_if (NULL != error,
             "gdigicam-manager: error was set.");

    fail_if (!g_digicam_manager_get_resolution
             (full_featured_manager, &gotten_resolution, &error),
             "gdigicam-manager: an error has happened.");
    fail_if (G_DIGICAM_RESOLUTION_LOW != gotten_resolution,
             "gdigicam-manager: the resolution was not "
             "the previously provided.");
    fail_if (NULL != error,
             "gdigicam-manager: error was set.");
}
END_TEST

/**
 * Purpose: test setting and getting an resolution in an invalid
 * #GDigicamManager
 * Cases considered:
 *    - set/get the resolution to/from a NULL
 *      #GDigicamManager. Parameters remain the same.
 *    - set/get the resolution to/from an invalid
 *      #GDigicamManager. Parameters remain the same.
 */
START_TEST (test_set_get_resolution_invalid)
{
    GDigicamResolution gotten_resolution = 0;

    /* Test 1 */
    fail_if (g_digicam_manager_set_resolution (NULL,
                                               G_DIGICAM_RESOLUTION_HIGH,
                                               &error,
                                               NULL),
             "gdigicam-manager: there was no error.");
    fail_if (NULL != error,
             "gdigicam-manager: error has changed.");

    fail_if (g_digicam_manager_get_resolution
             (NULL, &gotten_resolution, &error),
             "gdigicam-manager: there was no error.");
    fail_if (0 != gotten_resolution,
             "gdigicam-manager: the resolution was not the \"unset\" "
             "value, it was \"%d\".",
             gotten_resolution);
    fail_if (NULL != error,
             "gdigicam-manager: error has changed.");
    gotten_resolution = 0;

    /* Test 2 */
    fail_if (g_digicam_manager_set_resolution (G_DIGICAM_MANAGER (window),
                                               G_DIGICAM_RESOLUTION_HIGH,
                                               &error,
                                               NULL),
             "gdigicam-manager: there was no error.");
    fail_if (NULL != error,
             "gdigicam-manager: error has changed.");

    fail_if (g_digicam_manager_get_resolution
             (G_DIGICAM_MANAGER (window), &gotten_resolution, &error),
             "gdigicam-manager: there was no error.");
    fail_if (0 != gotten_resolution,
             "gdigicam-manager: the resolution was not the \"unset\" "
             "value, it was \"%d\".",
             gotten_resolution);
    fail_if (NULL != error,
             "gdigicam-manager: error has changed.");
    gotten_resolution = 0;
}
END_TEST

/* ----- Test case for set/get_zoom -----*/

/**
 * Purpose: test setting and getting limit zooms in a
 * #GDigicamManager
 * Cases considered:
 *    - get zoom from a featured but not set #GDigicamManager.
 *    - set/get maximum optical zoom to a featured #GDigicamManager with macro disabled.
 *    - set/get maximum zoom to a featured #GDigicamManager.
 *    - set/get minimum zoom to a featured #GDigicamManager.
 *    - set/get maximum optical zoom to a featured #GDigicamManager with macro enabled.
 */
START_TEST (test_set_get_zoom_limit)
{
    gdouble gotten_zoom = 0;
    gboolean gotten_digital = TRUE;

    /* Test 1 */
    g_digicam_manager_set_gstreamer_bin (full_featured_manager,
                                         full_featured_camera_bin,
                                         full_featured_descriptor,
                                         NULL);
    if (g_digicam_manager_get_zoom
        (full_featured_manager, &gotten_zoom, &gotten_digital, &error)) {
        fail_if (MIN_ZOOM != gotten_zoom,
                 "gdigicam-manager: the zoom was not the \"default\" "
                 "value, it was \"%d\".",
                 gotten_zoom);
        fail_if (FALSE != gotten_digital,
                 "gdigicam-manager: the digital should be false, "
                 "but it is true.");
    } else {
        fail_if (NULL == error,
                 "gdigicam-manager: an error has happened.");
        fail_if (NULL != error,
                 "gdigicam-manager: call has failed with this \n\"%s\"\n "
                 "error message and it should have been succesfull.",
                 error->message);
    }
    g_error_free (error);
    error = NULL;
    gotten_zoom = 0;
    gotten_digital = TRUE;

    /* Test 2 */
    if (g_digicam_manager_set_zoom (full_featured_manager,
                                    MAX_OPTICAL_ZOOM_MACRO_DISABLED,
                                    &gotten_digital,
                                    &error,
                                    NULL)) {
        fail_if (TRUE != gotten_digital,
                 "gdigicam-manager: the digital should be true "
                 "but it is false.");
    } else {
        fail_if (NULL == error,
                 "gdigicam-manager: an error has happened.");
        fail_if (NULL != error,
                 "gdigicam-manager: call has failed with this \n\"%s\"\n "
                 "error message and it should have been succesfull.",
                 error->message);
    }
    g_error_free (error);
    error = NULL;
    gotten_digital = TRUE;

    if (g_digicam_manager_get_zoom
        (full_featured_manager, &gotten_zoom, &gotten_digital, &error)) {
        fail_if (MAX_OPTICAL_ZOOM_MACRO_DISABLED != gotten_zoom,
                 "gdigicam-manager: the zoom was not the stablished "
                 "before, it was \"%d\".",
                 gotten_zoom);
        fail_if (TRUE != gotten_digital,
                 "gdigicam-manager: the digital should be true "
                 "but it is false.");
    } else {
        fail_if (NULL == error,
                 "gdigicam-manager: an error has happened.");
        fail_if (NULL != error,
                 "gdigicam-manager: call has failed with this \n\"%s\"\n "
                 "error message and it should have been succesfull.",
                 error->message);
    }
    g_error_free (error);
    error = NULL;
    gotten_zoom = 0;
    gotten_digital = FALSE;

    /* Test 3 */
    if (g_digicam_manager_set_zoom (full_featured_manager,
                                    MAX_ZOOM_MACRO_DISABLED,
                                    &gotten_digital,
                                    &error,
                                    NULL)) {
        fail_if (TRUE != gotten_digital,
                 "gdigicam-manager: the digital should be true "
                 "but it is false.");
    } else {
        fail_if (NULL == error,
                 "gdigicam-manager: an error has happened.");
        fail_if (NULL != error,
                 "gdigicam-manager: call has failed with this \n\"%s\"\n "
                 "error message and it should have been succesfull.",
                 error->message);
    }
    g_error_free (error);
    error = NULL;
    gotten_digital = FALSE;

    if (g_digicam_manager_get_zoom
        (full_featured_manager, &gotten_zoom, &gotten_digital, &error)) {
        fail_if (MAX_ZOOM_MACRO_DISABLED != gotten_zoom,
                 "gdigicam-manager: the zoom was not the stablished "
                 "before, it was \"%d\".",
                 gotten_zoom);
        fail_if (TRUE != gotten_digital,
                 "gdigicam-manager: the digital should be true but it is false.");
    } else {
        fail_if (NULL == error,
                 "gdigicam-manager: an error has happened.");
        fail_if (NULL != error,
                 "gdigicam-manager: call has failed with this \n\"%s\"\n "
                 "error message and it should have been succesfull.",
                 error->message);
    }
    g_error_free (error);
    error = NULL;
    gotten_zoom = 0;
    gotten_digital = TRUE;

    /* Test 4 */
    if (g_digicam_manager_set_zoom (full_featured_manager,
                                    MIN_ZOOM,
                                    &gotten_digital,
                                    &error,
                                    NULL)) {
        fail_if (TRUE != gotten_digital,
                 "gdigicam-manager: the digital should be true but it is false.");
    } else {
        fail_if (NULL == error,
                 "gdigicam-manager: an error has happened.");
        fail_if (NULL != error,
                 "gdigicam-manager: call has failed with this \n\"%s\"\n "
                 "error message and it should have been succesfull.",
                 error->message);
    }
    g_error_free (error);
    error = NULL;
    gotten_digital = TRUE;

    if (g_digicam_manager_get_zoom
        (full_featured_manager, &gotten_zoom, &gotten_digital, &error)) {
        fail_if (MIN_ZOOM != gotten_zoom,
                 "gdigicam-manager: the zoom was not the stablished "
                 "before, it was \"%d\".",
                 gotten_zoom);
        fail_if (TRUE != gotten_digital,
                 "gdigicam-manager: the digital should be true "
                 "but it is false.");
    } else {
        fail_if (NULL == error,
                 "gdigicam-manager: an error has happened.");
        fail_if (NULL != error,
                 "gdigicam-manager: call has failed with this \n\"%s\"\n "
                 "error message and it should have been succesfull.",
                 error->message);
    }
    g_error_free (error);
    error = NULL;
    gotten_zoom = 0;
    gotten_digital = TRUE;

    /* Test 5 */
    g_digicam_manager_set_mode (full_featured_manager,
                                G_DIGICAM_MODE_STILL,
                                NULL,
                                NULL);
    g_digicam_manager_set_focus_mode (full_featured_manager,
                                      G_DIGICAM_FOCUSMODE_AUTO,
                                      TRUE,
                                      NULL,
                                      NULL);
    if (g_digicam_manager_set_zoom (full_featured_manager,
                                    MAX_OPTICAL_ZOOM_MACRO_ENABLED,
                                    &gotten_digital,
                                    &error,
                                    NULL)) {
        fail_if (TRUE != gotten_digital,
                 "gdigicam-manager: the digital should be true "
                 "but it is false.");
    } else {
        fail_if (NULL == error,
                 "gdigicam-manager: an error has happened.");
        fail_if (NULL != error,
                 "gdigicam-manager: call has failed with this \n\"%s\"\n "
                 "error message and it should have been succesfull.",
                 error->message);
    }
    g_error_free (error);
    error = NULL;
    gotten_digital = TRUE;

    if (g_digicam_manager_get_zoom
        (full_featured_manager, &gotten_zoom, &gotten_digital, &error)) {
        fail_if (MAX_OPTICAL_ZOOM_MACRO_ENABLED != gotten_zoom,
                 "gdigicam-manager: the zoom was not the stablished "
                 "before, it was \"%d\".",
                 gotten_zoom);
        fail_if (TRUE != gotten_digital,
                 "gdigicam-manager: the digital should be true "
                 "but it is false.");
    } else {
        fail_if (NULL == error,
                 "gdigicam-manager: an error has happened.");
        fail_if (NULL != error,
                 "gdigicam-manager: call has failed with this \n\"%s\"\n "
                 "error message and it should have been succesfull.",
                 error->message);
    }
    g_error_free (error);
    error = NULL;
}
END_TEST

/**
 * Purpose: test setting and getting a valid zoom in a
 * #GDigicamManager
 * Cases considered:
 *    - set/get zoom to a featured #GDigicamManager with macro disabled.
 *    - set/get zoom to a featured #GDigicamManager with macro enabled.
 *    - set/get again another zoom to a featured #GDigicamManager with macro disabled.
 */
START_TEST (test_set_get_zoom_regular)
{
    gdouble gotten_zoom = 0;
    gboolean gotten_digital = FALSE;

    /* Test 1 */
    g_digicam_manager_set_gstreamer_bin (full_featured_manager,
                                         full_featured_camera_bin,
                                         full_featured_descriptor,
                                         NULL);
    if (g_digicam_manager_set_zoom (full_featured_manager,
                                    NORMAL_ZOOM_MACRO_DISABLED,
                                    &gotten_digital,
                                    &error,
                                    NULL)) {
        fail_if (TRUE != gotten_digital,
                 "gdigicam-manager: the digital should be true "
                 "but it is false.");
    } else {
        fail_if (NULL == error,
                 "gdigicam-manager: an error has happened.");
        fail_if (NULL != error,
                 "gdigicam-manager: call has failed with this \n\"%s\"\n "
                 "error message and it should have been succesfull.",
                 error->message);
    }
    g_error_free (error);
    error = NULL;
    gotten_digital = FALSE;

    if (g_digicam_manager_get_zoom
        (full_featured_manager, &gotten_zoom, &gotten_digital, &error)) {
        fail_if (NORMAL_ZOOM_MACRO_DISABLED != gotten_zoom,
                 "gdigicam-manager: the zoom was not the stablished "
                 "before, it was \"%d\".",
                 gotten_zoom);
        fail_if (TRUE != gotten_digital,
                 "gdigicam-manager: the digital should be true "
                 "but it is false.");
    } else {
        fail_if (NULL == error,
                 "gdigicam-manager: an error has happened.");
        fail_if (NULL != error,
                 "gdigicam-manager: call has failed with this \n\"%s\"\n "
                 "error message and it should have been succesfull.",
                 error->message);
    }
    g_error_free (error);
    error = NULL;
    gotten_zoom = 0;
    gotten_digital = TRUE;

    /* Test 2 */
    g_digicam_manager_set_mode (full_featured_manager,
                                G_DIGICAM_MODE_STILL,
                                NULL,
                                NULL);
    g_digicam_manager_set_focus_mode (full_featured_manager,
                                      G_DIGICAM_FOCUSMODE_AUTO,
                                      TRUE,
                                      NULL,
                                      NULL);
    if (g_digicam_manager_set_zoom (full_featured_manager,
                                    NORMAL_ZOOM_MACRO_ENABLED,
                                    &gotten_digital,
                                    &error,
                                    NULL)) {
        fail_if (TRUE != gotten_digital,
                 "gdigicam-manager: the digital should be true "
                 "but it is false.");
    } else {
        fail_if (NULL == error,
                 "gdigicam-manager: an error has happened.");
        fail_if (NULL != error,
                 "gdigicam-manager: call has failed with this \n\"%s\"\n "
                 "error message and it should have been succesfull.",
                 error->message);
    }
    g_error_free (error);
    error = NULL;
    gotten_digital = TRUE;

    if (g_digicam_manager_get_zoom
        (full_featured_manager, &gotten_zoom, &gotten_digital, &error)) {
        fail_if (NORMAL_ZOOM_MACRO_ENABLED != gotten_zoom,
                 "gdigicam-manager: the zoom was not the stablished "
                 "before, it was \"%d\".",
                 gotten_zoom);
        fail_if (TRUE != gotten_digital,
                 "gdigicam-manager: the digital should be true "
                 "but it is false.");
    } else {
        fail_if (NULL == error,
                 "gdigicam-manager: an error has happened.");
        fail_if (NULL != error,
                 "gdigicam-manager: call has failed with this \n\"%s\"\n "
                 "error message and it should have been succesfull.",
                 error->message);
    }
    g_error_free (error);
    error = NULL;
    gotten_zoom = 0;
    gotten_digital = TRUE;

    /* Test 3 */
    g_digicam_manager_set_focus_mode (full_featured_manager,
                                      G_DIGICAM_FOCUSMODE_AUTO,
                                      FALSE,
                                      NULL,
                                      NULL);
    if (g_digicam_manager_set_zoom (full_featured_manager,
                                    NORMAL_OPTICAL_ZOOM_MACRO_DISABLED,
                                    &gotten_digital,
                                    &error,
                                    NULL)) {
        fail_if (TRUE != gotten_digital,
                 "gdigicam-manager: the digital should be true "
                 "but it is false.");
    } else {
        fail_if (NULL == error,
                 "gdigicam-manager: an error has happened.");
        fail_if (NULL != error,
                 "gdigicam-manager: call has failed with this \n\"%s\"\n "
                 "error message and it should have been succesfull.",
                 error->message);
    }
    g_error_free (error);
    error = NULL;
    gotten_digital = TRUE;

    if (g_digicam_manager_get_zoom
        (full_featured_manager, &gotten_zoom, &gotten_digital, &error)) {
        fail_if (NORMAL_OPTICAL_ZOOM_MACRO_DISABLED != gotten_zoom,
                 "gdigicam-manager: the zoom was not the stablished "
                 "before, it was \"%d\".",
                 gotten_zoom);
        fail_if (TRUE != gotten_digital,
                 "gdigicam-manager: the digital should be true "
                 "but it is false.");
    } else {
        fail_if (NULL == error,
                 "gdigicam-manager: an error has happened.");
        fail_if (NULL != error,
                 "gdigicam-manager: call has failed with this \n\"%s\"\n "
                 "error message and it should have been succesfull.",
                 error->message);
    }
    g_error_free (error);
    error = NULL;
}
END_TEST

/**
 * Purpose: test setting and getting an zoom in an invalid
 * #GDigicamManager
 * Cases considered:
 *    - set/get the zoom to/from a NULL
 *      #GDigicamManager. Parameters remain the same.
 *    - set/get the zoom to/from an invalid
 *      #GDigicamManager. Parameters remain the same.
 *    - set/get zoom from just created #GDigicamManager.
 *    - set/get zoom to a none featured #GDigicamManager.
 *    - set/get zoom to a minimum featured #GDigicamManager.
 *    - set/get an underranged zoom to/from a full featured
 *    #GDigicamManager. Parameters remain the same.
 *    - set/get an overranged zoom to/from a full featured
 *    #GDigicamManager with macro disabled. Parameters remain the
 *    same.
 *    - set/get an overranged zoom to/from a full featured
 *    #GDigicamManager with macro enabled. Parameters remain the same.
 */
START_TEST (test_set_get_zoom_invalid)
{
    gdouble gotten_zoom = 0;
    gboolean gotten_digital = FALSE;

    /* Test 1 */
    fail_if (g_digicam_manager_set_zoom (NULL,
                                         NORMAL_OPTICAL_ZOOM_MACRO_DISABLED,
                                         &gotten_digital,
                                         &error,
                                         NULL),
             "gdigicam-manager: there was no error.");
    fail_if (NULL != error,
             "gdigicam-manager: error has changed.");
    fail_if (FALSE != gotten_digital,
             "gdigicam-manager: the digital should be false but it is true.");

    fail_if (g_digicam_manager_get_zoom
             (NULL, &gotten_zoom, &gotten_digital, &error),
             "gdigicam-manager: there was no error.");
    fail_if (NULL != error,
             "gdigicam-manager: error has changed.");
    fail_if (0 != gotten_zoom,
             "gdigicam-manager: the zoom was not the \"unset\" "
             "value, it was \"%d\".",
             gotten_zoom);
    fail_if (FALSE != gotten_digital,
             "gdigicam-manager: the digital should be false but it is true.");

    /* Test 2 */
    fail_if (g_digicam_manager_set_zoom (G_DIGICAM_MANAGER (window),
                                         NORMAL_OPTICAL_ZOOM_MACRO_DISABLED,
                                         &gotten_digital,
                                         &error,
                                         NULL),
             "gdigicam-manager: there was no error.");
    fail_if (NULL != error,
             "gdigicam-manager: error has changed.");
    fail_if (FALSE != gotten_digital,
             "gdigicam-manager: the digital should be false but it is true.");

    fail_if (g_digicam_manager_get_zoom
             (NULL, &gotten_zoom, &gotten_digital, &error),
             "gdigicam-manager: there was no error.");
    fail_if (NULL != error,
             "gdigicam-manager: error has changed.");
    fail_if (0 != gotten_zoom,
             "gdigicam-manager: the zoom was not the \"unset\" "
             "value, it was \"%d\".",
             gotten_zoom);
    fail_if (FALSE != gotten_digital,
             "gdigicam-manager: the digital should be false but it is true.");

    /* Test 3 */
    fail_if (g_digicam_manager_set_zoom (no_featured_manager,
                                         NORMAL_OPTICAL_ZOOM_MACRO_DISABLED,
                                         &gotten_digital,
                                         &error,
                                         NULL),
             "gdigicam-manager: the call was succesfull but it shouldn't.");
    fail_if (NULL == error,
             "gdigicam-manager: an error has happened.");
    fail_if (FALSE != gotten_digital,
             "gdigicam-manager: the digital was changed.");
    fail_if (!g_error_matches (error,
                               G_DIGICAM_ERROR,
                               G_DIGICAM_ERROR_GSTREAMER_BIN_NOT_SET),
             "gdigicam-manager: error is  \n\"%s\"\n instead of "
             "\"bin not set\".",
             error->message);
    g_error_free (error);
    error = NULL;
    gotten_digital = FALSE;

    fail_if (g_digicam_manager_get_zoom
             (no_featured_manager, &gotten_zoom, &gotten_digital, &error),
             "gdigicam-manager: the call was succesfull but it shouldn't.");
    fail_if (NULL == error,
             "gdigicam-manager: an error has happened.");
    fail_if (0 != gotten_zoom,
             "gdigicam-manager: the zoom was not the \"unset\" "
             "value, it was \"%d\".",
             gotten_zoom);
    fail_if (FALSE != gotten_digital,
             "gdigicam-manager: the digital was changed.");
    fail_if (!g_error_matches (error,
                               G_DIGICAM_ERROR,
                               G_DIGICAM_ERROR_GSTREAMER_BIN_NOT_SET),
             "gdigicam-manager: error is  \n\"%s\"\n instead of "
             "\"bin not set\".",
             error->message);
    g_error_free (error);
    error = NULL;
    gotten_zoom = 0;
    gotten_digital = FALSE;

    /* Test 4 */
    g_digicam_manager_set_gstreamer_bin (no_featured_manager,
                                         no_featured_camera_bin,
                                         no_featured_descriptor,
                                         NULL);
    fail_if (g_digicam_manager_set_zoom (no_featured_manager,
                                         NORMAL_OPTICAL_ZOOM_MACRO_DISABLED,
                                         &gotten_digital,
                                         &error,
                                         NULL),
             "gdigicam-manager: the call was succesfull but it shouldn't.");
    fail_if (NULL == error,
             "gdigicam-manager: an error has happened.");
    fail_if (FALSE != gotten_digital,
             "gdigicam-manager: the digital was changed.");
    fail_if (!g_error_matches (error,
                               G_DIGICAM_ERROR,
                               G_DIGICAM_ERROR_ZOOM_NOT_SUPPORTED),
             "gdigicam-manager: error is  \n\"%s\"\n instead of "
             "\"zoom not supported\".",
             error->message);
    g_error_free (error);
    error = NULL;
    gotten_digital = FALSE;

    fail_if (g_digicam_manager_get_zoom
             (no_featured_manager, &gotten_zoom, &gotten_digital, &error),
             "gdigicam-manager: the call was succesfull but it shouldn't.");
    fail_if (NULL == error,
             "gdigicam-manager: an error has happened.");
    fail_if (0 != gotten_zoom,
             "gdigicam-manager: the zoom was not the \"unset\" "
             "value, it was \"%d\".",
             gotten_zoom);
    fail_if (FALSE != gotten_digital,
             "gdigicam-manager: the digital was changed.");
    fail_if (!g_error_matches (error,
                               G_DIGICAM_ERROR,
                               G_DIGICAM_ERROR_ZOOM_NOT_SUPPORTED),
             "gdigicam-manager: error is  \n\"%s\"\n instead of "
             "\"zoom not supported\".",
             error->message);
    g_error_free (error);
    error = NULL;
    gotten_zoom = 0;
    gotten_digital = FALSE;

    /* Test 5 */
    g_digicam_manager_set_gstreamer_bin (minimum_featured_manager,
                                         minimum_featured_camera_bin,
                                         minimum_featured_descriptor,
                                         NULL);
    fail_if (g_digicam_manager_set_zoom (minimum_featured_manager,
                                         NORMAL_OPTICAL_ZOOM_MACRO_DISABLED,
                                         &gotten_digital,
                                         &error,
                                         NULL),
             "gdigicam-manager: the call was succesfull but it shouldn't.");
    fail_if (NULL == error,
             "gdigicam-manager: an error has happened.");
    fail_if (FALSE != gotten_digital,
             "gdigicam-manager: the digital was changed.");
    fail_if (!g_error_matches (error,
                               G_DIGICAM_ERROR,
                               G_DIGICAM_ERROR_ZOOM_NOT_SUPPORTED),
             "gdigicam-manager: error is  \n\"%s\"\n instead of "
             "\"zoom not supported\".",
             error->message);
    g_error_free (error);
    error = NULL;
    gotten_digital = FALSE;

    fail_if (g_digicam_manager_get_zoom
             (minimum_featured_manager, &gotten_zoom, &gotten_digital, &error),
             "gdigicam-manager: the call was succesfull but it shouldn't.");
    fail_if (NULL == error,
             "gdigicam-manager: an error has happened.");
    fail_if (0 != gotten_zoom,
             "gdigicam-manager: the zoom was not the \"unset\" "
             "value, it was \"%d\".",
             gotten_zoom);
    fail_if (FALSE != gotten_digital,
             "gdigicam-manager: the digital was changed.");
    fail_if (!g_error_matches (error,
                               G_DIGICAM_ERROR,
                               G_DIGICAM_ERROR_ZOOM_NOT_SUPPORTED),
             "gdigicam-manager: error is  \n\"%s\"\n instead of "
             "\"zoom not supported\".",
             error->message);
    g_error_free (error);
    error = NULL;
    gotten_zoom = 0;
    gotten_digital = TRUE;

    /* Test 6 */
    g_digicam_manager_set_gstreamer_bin (full_featured_manager,
                                         full_featured_camera_bin,
                                         full_featured_descriptor,
                                         NULL);
    fail_if (g_digicam_manager_set_zoom (full_featured_manager,
                                         MIN_ZOOM - 1,
                                         &gotten_digital,
                                         &error,
                                         NULL),
             "gdigicam-manager: the call was succesfull but it shouldn't.");
    fail_if (NULL == error,
             "gdigicam-manager: an error has happened.");
    fail_if (TRUE != gotten_digital,
             "gdigicam-manager: the digital was changed.");
    fail_if (!g_error_matches (error,
                               G_DIGICAM_ERROR,
                               G_DIGICAM_ERROR_ZOOM_OUT_OF_RANGE),
             "gdigicam-manager: error is  \n\"%s\"\n instead of "
             "\"zoom our of range\".",
             error->message);
    g_error_free (error);
    error = NULL;
    gotten_digital = TRUE;

    if (g_digicam_manager_get_zoom
        (full_featured_manager, &gotten_zoom, &gotten_digital, &error)) {
        fail_if (MIN_ZOOM != gotten_zoom,
                 "gdigicam-manager: the zoom was not the \"default\" "
                 "value, it was \"%d\".",
                 gotten_zoom);
        fail_if (FALSE != gotten_digital,
                 "gdigicam-manager: the digital should be false, "
                 "but it is true.");
    } else {
        fail_if (NULL == error,
                 "gdigicam-manager: an error has happened.");
        fail_if (NULL != error,
                 "gdigicam-manager: call has failed with this \n\"%s\"\n "
                 "error message and it should have been succesfull.",
                 error->message);
    }
    g_error_free (error);
    error = NULL;
    gotten_zoom = 0;
    gotten_digital = TRUE;

    /* Test 7 */
    fail_if (g_digicam_manager_set_zoom (full_featured_manager,
                                         MAX_ZOOM_MACRO_DISABLED + 1,
                                         &gotten_digital,
                                         &error,
                                         NULL),
             "gdigicam-manager: the call was succesfull but it shouldn't.");
    fail_if (NULL == error,
             "gdigicam-manager: an error has happened.");
    fail_if (TRUE != gotten_digital,
             "gdigicam-manager: the digital was changed.");
    fail_if (!g_error_matches (error,
                               G_DIGICAM_ERROR,
                               G_DIGICAM_ERROR_ZOOM_OUT_OF_RANGE),
             "gdigicam-manager: error is  \n\"%s\"\n instead of "
             "\"zoom our of range\".",
             error->message);
    g_error_free (error);
    error = NULL;
    gotten_digital = TRUE;

    if (g_digicam_manager_get_zoom
        (full_featured_manager, &gotten_zoom, &gotten_digital, &error)) {
        fail_if (MIN_ZOOM != gotten_zoom,
                 "gdigicam-manager: the zoom was not the \"default\" "
                 "value, it was \"%d\".",
                 gotten_zoom);
        fail_if (FALSE != gotten_digital,
                 "gdigicam-manager: the digital should be false, "
                 "but it is true.");
    } else {
        fail_if (NULL == error,
                 "gdigicam-manager: an error has happened.");
        fail_if (NULL != error,
                 "gdigicam-manager: call has failed with this \n\"%s\"\n "
                 "error message and it should have been succesfull.",
                 error->message);
    }
    g_error_free (error);
    error = NULL;
    gotten_zoom = 0;
    gotten_digital = TRUE;

    /* Test 8 */
    g_digicam_manager_set_mode (full_featured_manager,
                                G_DIGICAM_MODE_STILL,
                                NULL,
                                NULL);
    g_digicam_manager_set_focus_mode (full_featured_manager,
                                      G_DIGICAM_FOCUSMODE_AUTO,
                                      TRUE,
                                      NULL,
                                      NULL);
    fail_if (g_digicam_manager_set_zoom (full_featured_manager,
                                         MAX_ZOOM_MACRO_ENABLED + 1,
                                         &gotten_digital,
                                         &error,
                                         NULL),
             "gdigicam-manager: the call was succesfull but it shouldn't.");
    fail_if (NULL == error,
             "gdigicam-manager: an error has happened.");
    fail_if (TRUE != gotten_digital,
             "gdigicam-manager: the digital was changed.");
    fail_if (!g_error_matches (error,
                               G_DIGICAM_ERROR,
                               G_DIGICAM_ERROR_ZOOM_OUT_OF_RANGE),
             "gdigicam-manager: error is  \n\"%s\"\n instead of "
             "\"zoom our of range\".",
             error->message);
    g_error_free (error);
    error = NULL;
    gotten_digital = TRUE;

    if (g_digicam_manager_get_zoom
        (full_featured_manager, &gotten_zoom, &gotten_digital, &error)) {
        fail_if (MIN_ZOOM != gotten_zoom,
                 "gdigicam-manager: the zoom was not the \"default\" "
                 "value, it was \"%d\".",
                 gotten_zoom);
        fail_if (FALSE != gotten_digital,
                 "gdigicam-manager: the digital should be false, "
                 "but it is true.");
    } else {
        fail_if (NULL == error,
                 "gdigicam-manager: an error has happened.");
        fail_if (NULL != error,
                 "gdigicam-manager: call has failed with this \n\"%s\"\n "
                 "error message and it should have been succesfull.",
                 error->message);
    }
    g_error_free (error);

}
END_TEST



/* ----- Test case for query_capabilities -----*/


/**
 * Purpose: Query capabilites in a #GDigicamManager
 * Cases considered:
 *    - get capabilities from just created #GDigicamManager.
 *    - get capabilites to a none featured #GDigicamManager.
 *    - get capabilities to a minimum featured #GDigicamManager.
 *    - get capabilities from a featured but not set #GDigicamManager.
 */
START_TEST (test_query_capabilities_limit)
{
    GDigicamDescriptor *descriptor = NULL;

    /* Test 1 */
    fail_if (g_digicam_manager_query_capabilities (no_featured_manager,
			                           &descriptor,
						   &error),
            "gdigicam-manager: an error has not happend");
    fail_if (error == NULL, "an error has not happend");
    fail_if (descriptor != NULL, "descriptor is not NULL");
    if (error) {
        g_error_free (error);
        error = NULL;
    }


    /* Test 2 */
    fail_if (g_digicam_manager_query_capabilities (minimum_featured_manager,
			                           &descriptor,
						   &error),
            "gdigicam-manager: an error has not happend");
    fail_if (error == NULL, "an error has not happend");
    fail_if (descriptor != NULL, "descriptor is not NULL");
    if (error) {
        g_error_free (error);
        error = NULL;
    }


    /* Test 3 */
    fail_if (g_digicam_manager_query_capabilities (full_featured_manager,
			                           &descriptor,
						   &error),
            "gdigicam-manager: an error has not happend");
    fail_if (error == NULL, "an error has not happend");
    fail_if (descriptor != NULL, "descriptor is not NULL");
    if (error) {
        g_error_free (error);
        error = NULL;
    }
}
END_TEST



/* ----- Test case for start/stop the viewfinder -----*/


/**
 * Purpose: test starting/stopping viewfinder in a #GDigicamManager
 *
 * Cases considered:
 *    - start video streaming in viewfinder.
 *    - stop video streaming in viewfinder.
 **/
START_TEST (test_start_stop_viewfinder_regular)
{
    gulong xwindow_id = 0;

    window = create_test_window ();
    show_test_window (window);
    xwindow_id = get_test_window_id (window);

    /* Test 1 */

    g_digicam_manager_set_gstreamer_bin (full_featured_manager,
                                         full_featured_camera_bin,
                                         full_featured_descriptor,
                                         NULL);
    fail_if (!g_digicam_manager_play_bin (full_featured_manager,
                                          xwindow_id,
                                          &error),
             "gdigicam-manager: error has happened. "
	     "Unable to start the viewfinder");
    if (error != NULL) {
        fail_if (error != NULL,
                 "gdigicam-manager: error is \n\"%s\"\n instead of succes.",
                 error->message);
        g_error_free (error);
        error = NULL;
    }

    /* Test 2 */
    fail_if (!g_digicam_manager_stop_bin (full_featured_manager,
                                          &error),
             "gdigicam-manager: error has happened. "
	     "Unable to stop the viewfinder");

    if (error != NULL) {
        fail_if (error != NULL,
                 "gdigicam-manager: error is \n\"%s\"\n instead of success.",
                 error->message);
        g_error_free (error);
        error = NULL;
    }
}
END_TEST


/**
 * Purpose: test starting/stopping viewfinder with invalid values in
 * a #GDigicamManager
 *
 * Cases considered:
 *    - start video streaming in viewfinder.
 *    - stop video streaming in viewfinder.
 **/
START_TEST (test_start_stop_viewfinder_invalid)
{
    gulong xwindow_id = 0;

    window = create_test_window ();
    show_test_window (window);
    xwindow_id = get_test_window_id (window);

    /* Test 1 */
    fail_if (g_digicam_manager_play_bin (NULL,
                                         xwindow_id,
                                         &error),
            "gdigicam-manager: video streaming started "
            "with invalid digicam");

    /* Test 2 */
    fail_if (g_digicam_manager_stop_bin (NULL,
                                         &error),
            "gdigicam-manager: video streaming stopped "
            "with invalid digicam");
}
END_TEST



/* ----- Test case for play/pause/stop -----*/


/**
 * Purpose: test starting/pausing/stopping video recording in a#GDigicamManager
 * Cases considered:
 *    - start video recording.
 *    - pause video recording.
 *    - resume video recording.
 *    - stop video redording.
 */
START_TEST (test_video_recording_regular)
{

    g_digicam_manager_set_gstreamer_bin (full_featured_manager,
                                         full_featured_camera_bin,
                                         full_featured_descriptor,
                                         NULL);
    window = create_test_window ();
    show_test_window (window);
    g_digicam_manager_set_mode (full_featured_manager,
				G_DIGICAM_MODE_VIDEO,
				&error,
				NULL);
    /* Test 1 */
    fail_if (!g_digicam_manager_start_recording_video (full_featured_manager,
						       &error,
						       NULL),
             "gdigicam-manager: error starting video capture.");
    g_error_free (error);
    error = NULL;

    /* Test 2 */
    fail_if (!g_digicam_manager_pause_recording_video (full_featured_manager,
						       FALSE,
						       &error,
						       NULL),
             "gdigicam-manager: error pausing video capture.");
    g_error_free (error);
    error = NULL;

    /* Test 3 */
    fail_if (!g_digicam_manager_pause_recording_video (full_featured_manager,
						       TRUE,
						       &error,
						       NULL),
             "gdigicam-manager: error resuming video capture.");
    g_error_free (error);
    error = NULL;

    /* Test 3 */
    fail_if (!g_digicam_manager_finish_recording_video (full_featured_manager,
							&error,
							NULL),
             "gdigicam-manager: error finishing video capture.");
    g_error_free (error);
    error = NULL;
}
END_TEST

/**
 * Purpose: test starting/pausing/stopping video recording with invalid values in a #GDigicamManager
 * Cases considered:
 *    - star/pause/stop video capture with an invalid gdigicam-manager
 *    - start/pause/stop video capture with a camera with not video feature
 *    - start/pause/stop video capture with a gdigicam-manager not in video mode
 */
START_TEST (test_video_recording_invalid)
{
    /* Test 1 */
    fail_if (g_digicam_manager_start_recording_video (NULL,
						      &error,
						      NULL),
             "gdigicam-manager: video capture started with "
             "invalid gdigicam-manager.");

    fail_if (g_digicam_manager_pause_recording_video (NULL,
						      FALSE,
						      &error,
						      NULL),
             "gdigicam-manager: video pause paused with "
             "invalid gdigicam-manager.");

    fail_if (g_digicam_manager_pause_recording_video (NULL,
						      TRUE,
						      &error,
						      NULL),
             "gdigicam-manager: video resumed with "
             "invalid gdigicam-manager.");

    fail_if (g_digicam_manager_finish_recording_video (NULL,
						       &error,
						       NULL),
             "gdigicam-manager: video finished with "
             "invalid gdigicam-manager.");

    /* Test 2 */
    g_digicam_manager_set_gstreamer_bin (no_featured_manager,
                                         no_featured_camera_bin,
                                         no_featured_descriptor,
                                         NULL);


    fail_if (g_digicam_manager_start_recording_video (no_featured_manager,
						      &error,
						      NULL),
             "gdigicam-manager: video capture started with no capable bin.");

    fail_if (g_digicam_manager_pause_recording_video (no_featured_manager,
						      FALSE,
						      &error,
						      NULL),
             "gdigicam-manager: video pause paused with no capable bin.");

    fail_if (g_digicam_manager_pause_recording_video (no_featured_manager,
						      TRUE,
						      &error,
						      NULL),
             "gdigicam-manager: video resumed with no capable bin.");

    fail_if (g_digicam_manager_finish_recording_video (no_featured_manager,
						       &error,
						       NULL),
             "gdigicam-manager: video finished with no capable bin.");

    /* Test 3 */

    g_digicam_manager_set_gstreamer_bin (full_featured_manager,
                                         full_featured_camera_bin,
                                         full_featured_descriptor,
                                         NULL);
    window = create_test_window ();
    show_test_window (window);
    g_digicam_manager_set_mode (full_featured_manager,
				G_DIGICAM_MODE_STILL,
				&error,
				NULL);

    fail_if (g_digicam_manager_start_recording_video (full_featured_manager,
						      &error,
						      NULL),
             "gdigicam-manager: video capture started "
             "with bin not in video mode.");
    fail_if (NULL == error,
             "gdigicam-manager: error was not set.");
    fail_if (!g_error_matches (error,
                               G_DIGICAM_ERROR,
                               G_DIGICAM_ERROR_INVALID_MODE),
             "gdigicam-manager: error is \n\"%s\"\n instead of invalid mode.",
             error->message);
    g_error_free (error);
    error = NULL;

    fail_if (g_digicam_manager_pause_recording_video (full_featured_manager,
						      FALSE,
						      &error,
						      NULL),
             "gdigicam-manager: video pause paused "
             "with bin not in video mode.");
    fail_if (NULL == error,
             "gdigicam-manager: error was not set.");
    fail_if (!g_error_matches (error,
                               G_DIGICAM_ERROR,
                               G_DIGICAM_ERROR_INVALID_MODE),
             "gdigicam-manager: error is  \n\"%s\"\n instead of invalid mode.",
             error->message);
    g_error_free (error);
    error = NULL;

    fail_if (g_digicam_manager_pause_recording_video (full_featured_manager,
						      TRUE,
						      &error,
						      NULL),
             "gdigicam-manager: video resumed with bin not in video mode.");
    fail_if (NULL == error,
             "gdigicam-manager: error was not set.");
    fail_if (!g_error_matches (error,
                               G_DIGICAM_ERROR,
                               G_DIGICAM_ERROR_INVALID_MODE),
             "gdigicam-manager: error is  \n\"%s\"\n instead of invalid mode.",
             error->message);
    g_error_free (error);
    error = NULL;

    fail_if (g_digicam_manager_finish_recording_video (full_featured_manager,
						       &error,
						       NULL),
             "gdigicam-manager: video finished with bin not in video mode.");
    fail_if (NULL == error,
             "gdigicam-manager: error was not set.");
    fail_if (!g_error_matches (error,
                               G_DIGICAM_ERROR,
                               G_DIGICAM_ERROR_INVALID_MODE),
             "gdigicam-manager: error is  \n\"%s\"\n instead of invalid mode.",
             error->message);
    g_error_free (error);
    error = NULL;
}
END_TEST



/* FIXME: Not implemented yet since it is still too soon to test the
 * incomplete API */

/* ----- Test case for set_get_focus_mode -----*/

/**
 * Purpose: test set_get_focus_mode operation in a#GDigicamManager
 * Cases considered:
 *    - set G_DIGICAM_FOCUSMODE_MANUAL focus mode and macro enabled.
 *    - set G_DIGICAM_FOCUSMODE_MANUAL focus mode and macro disabled.
 *    - set G_DIGICAM_FOCUSMODE_AUTO focus mode and macro enabled.
 *    - set G_DIGICAM_FOCUSMODE_AUTO focus mode and macro disabled.
 *    - set G_DIGICAM_FOCUSMODE_FACE focus mode and macro enabled.
 *    - set G_DIGICAM_FOCUSMODE_FACE focus mode and macro disabled.
 *    - set G_DIGICAM_FOCUSMODE_SMILE focus mode and macro enabled.
 *    - set G_DIGICAM_FOCUSMODE_SMILE focus mode and macro disabled.
 *    - set G_DIGICAM_FOCUSMODE_CENTROID focus mode and macro enabled.
 *    - set G_DIGICAM_FOCUSMODE_CENTROID focus mode and macro disabled.
 *    - set G_DIGICAM_FOCUSMODE_CONTINUOUSAUTO focus mode and macro enabled.
 *    - set G_DIGICAM_FOCUSMODE_CONTINUOUSAUTO focus mode and macro disabled.
 *    - set G_DIGICAM_FOCUSMODE_CONTINUOUSCENTROID focus mode and macro enabled.
 *    - set G_DIGICAM_FOCUSMODE_CONTINUOUSCENTROID focus mode and macro disabled.
 */
START_TEST (test_set_get_focus_mode_regular)
{
    GDigicamFocusmode focus_mode;
    gboolean macro_enabled;
    gboolean result;

    g_digicam_manager_set_gstreamer_bin (full_featured_manager,
                                         full_featured_camera_bin,
                                         full_featured_descriptor,
                                         NULL);
    window = create_test_window ();
    show_test_window (window);
    g_digicam_manager_set_mode (full_featured_manager,
				G_DIGICAM_MODE_STILL,
				&error,
				NULL);
    if (error != NULL)
        g_error_free (error);
    error = NULL;

    /* Test 1 */
    result = g_digicam_manager_set_focus_mode (full_featured_manager,
                                               G_DIGICAM_FOCUSMODE_MANUAL,
                                               TRUE,
                                               &error,
                                               NULL);
    if (error != NULL) {
        fail_if (error != NULL,
                 "gdigicam-manager: error is \n\"%s\"\n instead of succes.",
                 error->message);
        g_error_free (error);
        error = NULL;
    }
    fail_if (!result,
             "gdigicam-manager: error setting focus mode to %i macro enabled.",
             G_DIGICAM_FOCUSMODE_MANUAL);
    g_digicam_manager_get_focus_mode (full_featured_manager,
                                      &focus_mode,
                                      &macro_enabled,
                                      &error);
    fail_if (focus_mode != G_DIGICAM_FOCUSMODE_MANUAL,
             "gdigicam-manager: retrieved focus mode is %i and it should be %i.",
             focus_mode,
             G_DIGICAM_FOCUSMODE_MANUAL);
    fail_if (!macro_enabled,
             "gdigicam-manager: macro is not enabled, and it should be.");

    /* Test 2 */
    result = g_digicam_manager_set_focus_mode (full_featured_manager,
                                               G_DIGICAM_FOCUSMODE_MANUAL,
                                               FALSE,
                                               &error,
                                               NULL);
    if (error != NULL) {
        fail_if (error != NULL,
                 "gdigicam-manager: error is \n\"%s\"\n instead of succes.",
                 error->message);
        g_error_free (error);
        error = NULL;
    }
    fail_if (!result,
             "gdigicam-manager: error setting focus mode to %i macro disabled.",
             G_DIGICAM_FOCUSMODE_MANUAL);
    g_digicam_manager_get_focus_mode (full_featured_manager,
                                      &focus_mode,
                                      &macro_enabled,
                                      &error);
    fail_if (focus_mode != G_DIGICAM_FOCUSMODE_MANUAL,
             "gdigicam-manager: retrieved focus mode is %i and it should be %i.",
             focus_mode,
             G_DIGICAM_FOCUSMODE_MANUAL);
    fail_if (macro_enabled,
             "gdigicam-manager: macro is enabled, and it should not be.");

    /* Test 3 */
    result = g_digicam_manager_set_focus_mode (full_featured_manager,
                                               G_DIGICAM_FOCUSMODE_AUTO,
                                               TRUE,
                                               &error,
                                               NULL);
    if (error != NULL) {
        fail_if (error != NULL,
                 "gdigicam-manager: error is \n\"%s\"\n instead of succes.",
                 error->message);
        g_error_free (error);
        error = NULL;
    }
    fail_if (!result,
             "gdigicam-manager: error setting focus mode to %i macro enabled.",
             G_DIGICAM_FOCUSMODE_MANUAL);
    g_digicam_manager_get_focus_mode (full_featured_manager,
                                      &focus_mode,
                                      &macro_enabled,
                                      &error);
    fail_if (focus_mode != G_DIGICAM_FOCUSMODE_AUTO,
             "gdigicam-manager: retrieved focus mode is %i and it should be %i.",
             focus_mode,
             G_DIGICAM_FOCUSMODE_AUTO);
    fail_if (!macro_enabled,
             "gdigicam-manager: macro is not enabled, and it should be.");

    /* Test 4 */
    result = g_digicam_manager_set_focus_mode (full_featured_manager,
                                               G_DIGICAM_FOCUSMODE_AUTO,
                                               FALSE,
                                               &error,
                                               NULL);
    if (error != NULL) {
        fail_if (error != NULL,
                 "gdigicam-manager: error is \n\"%s\"\n instead of succes.",
                 error->message);
        g_error_free (error);
        error = NULL;
    }
    fail_if (!result,
             "gdigicam-manager: error setting focus mode to %i macro disabled.",
             G_DIGICAM_FOCUSMODE_AUTO);
    g_digicam_manager_get_focus_mode (full_featured_manager,
                                      &focus_mode,
                                      &macro_enabled,
                                      &error);
    fail_if (focus_mode != G_DIGICAM_FOCUSMODE_AUTO,
             "gdigicam-manager: retrieved focus mode is %i and it should be %i.",
             focus_mode,
             G_DIGICAM_FOCUSMODE_AUTO);
    fail_if (macro_enabled,
             "gdigicam-manager: macro is enabled, and it should not be.");

    /* Test 5 */
    result = g_digicam_manager_set_focus_mode (full_featured_manager,
                                               G_DIGICAM_FOCUSMODE_SMILE,
                                               TRUE,
                                               &error,
                                               NULL);
    if (error != NULL) {
        fail_if (error != NULL,
                 "gdigicam-manager: error is \n\"%s\"\n instead of succes.",
                 error->message);
        g_error_free (error);
        error = NULL;
    }
    fail_if (!result,
             "gdigicam-manager: error setting focus mode to %i macro enabled.",
             G_DIGICAM_FOCUSMODE_SMILE);
    g_digicam_manager_get_focus_mode (full_featured_manager,
                                      &focus_mode,
                                      &macro_enabled,
                                      &error);
    fail_if (focus_mode != G_DIGICAM_FOCUSMODE_SMILE,
             "gdigicam-manager: retrieved focus mode is %i and it should be %i.",
             focus_mode,
             G_DIGICAM_FOCUSMODE_SMILE);
    fail_if (!macro_enabled,
             "gdigicam-manager: macro is not enabled, and it should be.");

    /* Test 6 */
    result = g_digicam_manager_set_focus_mode (full_featured_manager,
                                               G_DIGICAM_FOCUSMODE_SMILE,
                                               FALSE,
                                               &error,
                                               NULL);
    if (error != NULL) {
        fail_if (error != NULL,
                 "gdigicam-manager: error is \n\"%s\"\n instead of succes.",
                 error->message);
        g_error_free (error);
        error = NULL;
    }
    fail_if (!result,
             "gdigicam-manager: error setting focus mode to %i macro disabled.",
             G_DIGICAM_FOCUSMODE_SMILE);
    g_digicam_manager_get_focus_mode (full_featured_manager,
                                      &focus_mode,
                                      &macro_enabled,
                                      &error);
    fail_if (focus_mode != G_DIGICAM_FOCUSMODE_SMILE,
             "gdigicam-manager: retrieved focus mode is %i and it should be %i.",
             focus_mode,
             G_DIGICAM_FOCUSMODE_SMILE);
    fail_if (macro_enabled,
             "gdigicam-manager: macro is enabled, and it should not be.");

    /* Test 7 */
    result = g_digicam_manager_set_focus_mode (full_featured_manager,
                                               G_DIGICAM_FOCUSMODE_CENTROID,
                                               TRUE,
                                               &error,
                                               NULL);
    if (error != NULL) {
        fail_if (error != NULL,
                 "gdigicam-manager: error is \n\"%s\"\n instead of succes.",
                 error->message);
        g_error_free (error);
        error = NULL;
    }
    fail_if (!result,
             "gdigicam-manager: error setting focus mode to %i macro enabled.",
             G_DIGICAM_FOCUSMODE_CENTROID);
    g_digicam_manager_get_focus_mode (full_featured_manager,
                                      &focus_mode,
                                      &macro_enabled,
                                      &error);
    fail_if (focus_mode != G_DIGICAM_FOCUSMODE_CENTROID,
             "gdigicam-manager: retrieved focus mode is %i and it should be %i.",
             focus_mode,
             G_DIGICAM_FOCUSMODE_CENTROID);
    fail_if (!macro_enabled,
             "gdigicam-manager: macro is not enabled, and it should be.");

    /* Test 8 */
    result = g_digicam_manager_set_focus_mode (full_featured_manager,
                                               G_DIGICAM_FOCUSMODE_CENTROID,
                                               FALSE,
                                               &error,
                                               NULL);
    if (error != NULL) {
        fail_if (error != NULL,
                 "gdigicam-manager: error is \n\"%s\"\n instead of succes.",
                 error->message);
        g_error_free (error);
        error = NULL;
    }
    fail_if (!result,
             "gdigicam-manager: error setting focus mode to %i macro disabled.",
             G_DIGICAM_FOCUSMODE_CENTROID);
    g_digicam_manager_get_focus_mode (full_featured_manager,
                                      &focus_mode,
                                      &macro_enabled,
                                      &error);
    fail_if (focus_mode != G_DIGICAM_FOCUSMODE_CENTROID,
             "gdigicam-manager: retrieved focus mode is %i and it should be %i.",
             focus_mode,
             G_DIGICAM_FOCUSMODE_CENTROID);
    fail_if (macro_enabled,
             "gdigicam-manager: macro is enabled, and it should not be.");

    /* Test 9 */
    result = g_digicam_manager_set_focus_mode (full_featured_manager,
                                               G_DIGICAM_FOCUSMODE_CONTINUOUSAUTO,
                                               TRUE,
                                               &error,
                                               NULL);
    if (error != NULL) {
        fail_if (error != NULL,
                 "gdigicam-manager: error is \n\"%s\"\n instead of succes.",
                 error->message);
        g_error_free (error);
        error = NULL;
    }
    fail_if (!result,
             "gdigicam-manager: error setting focus mode to %i macro enabled.",
             G_DIGICAM_FOCUSMODE_CONTINUOUSAUTO);
    g_digicam_manager_get_focus_mode (full_featured_manager,
                                      &focus_mode,
                                      &macro_enabled,
                                      &error);
    fail_if (focus_mode != G_DIGICAM_FOCUSMODE_CONTINUOUSAUTO,
             "gdigicam-manager: retrieved focus mode is %i and it should be %i.",
             focus_mode,
             G_DIGICAM_FOCUSMODE_CONTINUOUSAUTO);
    fail_if (!macro_enabled,
             "gdigicam-manager: macro is not enabled, and it should be.");

    /* Test 10 */
    result = g_digicam_manager_set_focus_mode (full_featured_manager,
                                               G_DIGICAM_FOCUSMODE_CONTINUOUSAUTO,
                                               FALSE,
                                               &error,
                                               NULL);
    if (error != NULL) {
        fail_if (error != NULL,
                 "gdigicam-manager: error is \n\"%s\"\n instead of succes.",
                 error->message);
        g_error_free (error);
        error = NULL;
    }
    fail_if (!result,
             "gdigicam-manager: error setting focus mode to %i macro disabled.",
             G_DIGICAM_FOCUSMODE_CONTINUOUSAUTO);
    g_digicam_manager_get_focus_mode (full_featured_manager,
                                      &focus_mode,
                                      &macro_enabled,
                                      &error);
    fail_if (focus_mode != G_DIGICAM_FOCUSMODE_CONTINUOUSAUTO,
             "gdigicam-manager: retrieved focus mode is %i and it should be %i.",
             focus_mode,
             G_DIGICAM_FOCUSMODE_CONTINUOUSAUTO);
    fail_if (macro_enabled,
             "gdigicam-manager: macro is enabled, and it should not be.");

    /* Test 11 */
    result = g_digicam_manager_set_focus_mode (full_featured_manager,
                                               G_DIGICAM_FOCUSMODE_CONTINUOUSCENTROID,
                                               TRUE,
                                               &error,
                                               NULL);
    if (error != NULL) {
        fail_if (error != NULL,
                 "gdigicam-manager: error is \n\"%s\"\n instead of succes.",
                 error->message);
        g_error_free (error);
        error = NULL;
    }
    fail_if (!result,
             "gdigicam-manager: error setting focus mode to %i macro enabled.",
             G_DIGICAM_FOCUSMODE_CONTINUOUSCENTROID);
    g_digicam_manager_get_focus_mode (full_featured_manager,
                                      &focus_mode,
                                      &macro_enabled,
                                      &error);
    fail_if (focus_mode != G_DIGICAM_FOCUSMODE_CONTINUOUSCENTROID,
             "gdigicam-manager: retrieved focus mode is %i and it should be %i.",
             focus_mode,
             G_DIGICAM_FOCUSMODE_CONTINUOUSCENTROID);
    fail_if (!macro_enabled,
             "gdigicam-manager: macro is not enabled, and it should be.");

    /* Test 12 */
    result = g_digicam_manager_set_focus_mode (full_featured_manager,
                                               G_DIGICAM_FOCUSMODE_CONTINUOUSCENTROID,
                                               FALSE,
                                               &error,
                                               NULL);
    if (error != NULL) {
        fail_if (error != NULL,
                 "gdigicam-manager: error is \n\"%s\"\n instead of succes.",
                 error->message);
        g_error_free (error);
        error = NULL;
    }
    fail_if (!result,
             "gdigicam-manager: error setting focus mode to %i macro disabled.",
             G_DIGICAM_FOCUSMODE_CONTINUOUSCENTROID);
    g_digicam_manager_get_focus_mode (full_featured_manager,
                                      &focus_mode,
                                      &macro_enabled,
                                      &error);
    fail_if (focus_mode != G_DIGICAM_FOCUSMODE_CONTINUOUSCENTROID,
             "gdigicam-manager: retrieved focus mode is %i and it should be %i.",
             focus_mode,
             G_DIGICAM_FOCUSMODE_CONTINUOUSCENTROID);
    fail_if (macro_enabled,
             "gdigicam-manager: macro is enabled, and it should not be.");
}
END_TEST

/**
 * Purpose: test set_get_focus_mode operation with invalid values in a #GDigicamManager
 * Cases considered:
 *    - using an invalid gdigicam-manager
 *    - using a camera with not focus mode setting feature
 *    - using a gdigicam-manager not in still mode
 */
START_TEST (test_set_get_focus_mode_invalid)
{
    gboolean result;

    /* Test 1 */
    fail_if (g_digicam_manager_set_focus_mode (NULL,
                                               G_DIGICAM_FOCUSMODE_MANUAL,
                                               TRUE,
                                               &error,
                                               NULL),
             "gdigicam-manager: setting focus mode with invalid gdigicam-manager.");


    /* Test 2 */
    g_digicam_manager_set_gstreamer_bin (minimum_featured_manager,
                                         minimum_featured_camera_bin,
                                         minimum_featured_descriptor,
                                         NULL);

    fail_if (!g_digicam_manager_set_mode (minimum_featured_manager,
                                          G_DIGICAM_MODE_STILL,
                                          &error,
                                          NULL),
             "gdigicam-manager: still mode could not be set.");

    if (error != NULL)
        g_error_free (error);
    error = NULL;

    result = g_digicam_manager_set_focus_mode (minimum_featured_manager,
                                               G_DIGICAM_FOCUSMODE_MANUAL,
                                               TRUE,
                                               &error,
                                               NULL);
    if (error != NULL) {
        fail_if (!g_error_matches (error,
                                   G_DIGICAM_ERROR,
                                   G_DIGICAM_ERROR_FOCUSMODE_NOT_SUPPORTED),
                 "gdigicam-manager: error is %i instead of %i.",
                 error->code,
                 G_DIGICAM_ERROR_FOCUSMODE_NOT_SUPPORTED);
        g_error_free (error);
        error = NULL;
    } else {
        fail_if (error == NULL,
                 "gdigicam-manager: there is not error returned.");
    }
    fail_if (result,
             "gdigicam-manager: setting focus mode with no capable bin.");



    /* Test 3 */

    g_digicam_manager_set_gstreamer_bin (full_featured_manager,
                                         full_featured_camera_bin,
                                         full_featured_descriptor,
                                         NULL);
    window = create_test_window ();
    show_test_window (window);
    fail_if (!g_digicam_manager_set_mode (minimum_featured_manager,
                                          G_DIGICAM_MODE_VIDEO,
                                          &error,
                                          NULL),
             "gdigicam-manager: still mode could not be set.");

    if (error != NULL)
        g_error_free (error);
    error = NULL;

    result = g_digicam_manager_set_focus_mode (full_featured_manager,
                                               G_DIGICAM_FOCUSMODE_MANUAL,
                                               TRUE,
                                               &error,
                                               NULL);
    fail_if (!result, "gdigicam-manager: error has occured");

    if (error != NULL) {
        fail_if (!g_error_matches (error,
                                   G_DIGICAM_ERROR,
                                   G_DIGICAM_ERROR_INVALID_MODE),
                 "gdigicam-manager: error is  \n\"%s\"\n instead of %i.",
                 error->message,
                 G_DIGICAM_ERROR_INVALID_MODE);
        g_error_free (error);
        error = NULL;
    }

}
END_TEST



/* ----- Test case for set_get_focus_region_pattern -----*/

/**
 * Purpose: test set_get_focus_region_pattern operation in a#GDigicamManager
 * Cases considered:
 *    - set G_DIGICAM_FOCUSPOINTS_ONE focus region pattern.
 *    - set G_DIGICAM_FOCUSPOINTS_THREE_3X1 focus region pattern.
 *    - set G_DIGICAM_FOCUSPOINTS_FIVE_CROSS focus region pattern.
 *    - set G_DIGICAM_FOCUSPOINTS_SEVEN_CROSS focus region pattern.
 *    - set G_DIGICAM_FOCUSPOINTS_NINE_SQUARE focus region pattern.
 *    - set G_DIGICAM_FOCUSPOINTS_ELEVEN_CROSS focus region pattern.
 *    - set G_DIGICAM_FOCUSPOINTS_TWELVE_3X4 focus region pattern.
 *    - set G_DIGICAM_FOCUSPOINTS_TWELVE_4X3 focus region pattern.
 *    - set G_DIGICAM_FOCUSPOINTS_SIXTEEN_SQUARE focus region pattern.
 *    - set G_DIGICAM_FOCUSPOINTS_CUSTOM focus region pattern and infinite active points.
 */
START_TEST (test_set_get_focus_region_pattern_regular)
{
    GDigicamFocuspoints focus_points;
    guint64 active_points;
    guint64 infinite = 0;
    gboolean result;

    g_digicam_manager_set_gstreamer_bin (full_featured_manager,
                                         full_featured_camera_bin,
                                         full_featured_descriptor,
                                         NULL);
    window = create_test_window ();
    show_test_window (window);
    g_digicam_manager_set_mode (full_featured_manager,
				G_DIGICAM_MODE_STILL,
				&error,
				NULL);
    if (error != NULL)
        g_error_free (error);
    error = NULL;

    g_digicam_manager_set_focus_mode (full_featured_manager,
                                      G_DIGICAM_CAPABILITIES_MANUALFOCUS,
		                      TRUE,
		                      &error,
		                      NULL);
    if (error != NULL)
        g_error_free (error);
    error = NULL;


    /* Test 1 */
    result = g_digicam_manager_set_focus_region_pattern (full_featured_manager,
                                                         G_DIGICAM_FOCUSPOINTS_ONE,
                                                         0,
                                                         &error,
                                                         NULL);
    if (error != NULL) {
        fail_if (error != NULL,
                 "gdigicam-manager: error is \n\"%s\"\n instead of succes.",
                 error->message);
        g_error_free (error);
        error = NULL;
    }
    fail_if (!result,
             "gdigicam-manager: error setting focus region pattern to %i.",
             G_DIGICAM_FOCUSPOINTS_ONE);
    g_digicam_manager_get_focus_region_pattern (full_featured_manager,
                                                &focus_points,
                                                &active_points,
                                                &error);
    fail_if (focus_points != G_DIGICAM_FOCUSPOINTS_ONE,
             "gdigicam-manager: retrieved focus points is %i and it should be %i.",
             focus_points,
             G_DIGICAM_FOCUSPOINTS_ONE);


    /* Test 2 */
    result = g_digicam_manager_set_focus_region_pattern (full_featured_manager,
                                                         G_DIGICAM_FOCUSPOINTS_THREE_3X1,
                                                         0,
                                                         &error,
                                                         NULL);
    if (error != NULL) {
        fail_if (error != NULL,
                 "gdigicam-manager: error is \n\"%s\"\n instead of succes.",
                 error->message);
        g_error_free (error);
        error = NULL;
    }
    fail_if (!result,
             "gdigicam-manager: error setting focus region pattern to %i.",
             G_DIGICAM_FOCUSPOINTS_THREE_3X1);
    g_digicam_manager_get_focus_region_pattern (full_featured_manager,
                                                &focus_points,
                                                &active_points,
                                                &error);
    fail_if (focus_points != G_DIGICAM_FOCUSPOINTS_THREE_3X1,
             "gdigicam-manager: retrieved focus points is %i and it should be %i.",
             focus_points,
             G_DIGICAM_FOCUSPOINTS_THREE_3X1);

    /* Test 3 */
    result = g_digicam_manager_set_focus_region_pattern (full_featured_manager,
                                                         G_DIGICAM_FOCUSPOINTS_FIVE_CROSS,
                                                         0,
                                                         &error,
                                                         NULL);
    if (error != NULL) {
        fail_if (error != NULL,
                 "gdigicam-manager: error is \n\"%s\"\n instead of succes.",
                 error->message);
        g_error_free (error);
        error = NULL;
    }
    fail_if (!result,
             "gdigicam-manager: error setting focus region pattern to %i.",
             G_DIGICAM_FOCUSPOINTS_FIVE_CROSS);
    g_digicam_manager_get_focus_region_pattern (full_featured_manager,
                                                &focus_points,
                                                &active_points,
                                                &error);
    fail_if (focus_points != G_DIGICAM_FOCUSPOINTS_FIVE_CROSS,
             "gdigicam-manager: retrieved focus points is %i and it should be %i.",
             focus_points,
             G_DIGICAM_FOCUSPOINTS_FIVE_CROSS);

    /* Test 4 */
    result = g_digicam_manager_set_focus_region_pattern (full_featured_manager,
                                                         G_DIGICAM_FOCUSPOINTS_SEVEN_CROSS,
                                                         0,
                                                         &error,
                                                         NULL);
    if (error != NULL) {
        fail_if (error != NULL,
                 "gdigicam-manager: error is \n\"%s\"\n instead of succes.",
                 error->message);
        g_error_free (error);
        error = NULL;
    }
    fail_if (!result,
             "gdigicam-manager: error setting focus region pattern to %i.",
             G_DIGICAM_FOCUSPOINTS_SEVEN_CROSS);
    g_digicam_manager_get_focus_region_pattern (full_featured_manager,
                                                &focus_points,
                                                &active_points,
                                                &error);
    fail_if (focus_points != G_DIGICAM_FOCUSPOINTS_SEVEN_CROSS,
             "gdigicam-manager: retrieved focus points is %i and it should be %i.",
             focus_points,
             G_DIGICAM_FOCUSPOINTS_SEVEN_CROSS);

    /* Test 5 */
    result = g_digicam_manager_set_focus_region_pattern (full_featured_manager,
                                                         G_DIGICAM_FOCUSPOINTS_NINE_SQUARE,
                                                         0,
                                                         &error,
                                                         NULL);
    if (error != NULL) {
        fail_if (error != NULL,
                 "gdigicam-manager: error is \n\"%s\"\n instead of succes.",
                 error->message);
        g_error_free (error);
        error = NULL;
    }
    fail_if (!result,
             "gdigicam-manager: error setting focus region pattern to %i.",
             G_DIGICAM_FOCUSPOINTS_NINE_SQUARE);
    g_digicam_manager_get_focus_region_pattern (full_featured_manager,
                                                &focus_points,
                                                &active_points,
                                                &error);
    fail_if (focus_points != G_DIGICAM_FOCUSPOINTS_NINE_SQUARE,
             "gdigicam-manager: retrieved focus points is %i and it should be %i.",
             focus_points,
             G_DIGICAM_FOCUSPOINTS_NINE_SQUARE);

    /* Test 5 */
    result = g_digicam_manager_set_focus_region_pattern (full_featured_manager,
                                                         G_DIGICAM_FOCUSPOINTS_NINE_SQUARE,
                                                         0,
                                                         &error,
                                                         NULL);
    if (error != NULL) {
        fail_if (error != NULL,
                 "gdigicam-manager: error is \n\"%s\"\n instead of succes.",
                 error->message);
        g_error_free (error);
        error = NULL;
    }
    fail_if (!result,
             "gdigicam-manager: error setting focus region pattern to %i.",
             G_DIGICAM_FOCUSPOINTS_NINE_SQUARE);
    g_digicam_manager_get_focus_region_pattern (full_featured_manager,
                                                &focus_points,
                                                &active_points,
                                                &error);
    fail_if (focus_points != G_DIGICAM_FOCUSPOINTS_NINE_SQUARE,
             "gdigicam-manager: retrieved focus points is %i and it should be %i.",
             focus_points,
             G_DIGICAM_FOCUSPOINTS_NINE_SQUARE);

    /* Test 6 */
    result = g_digicam_manager_set_focus_region_pattern (full_featured_manager,
                                                         G_DIGICAM_FOCUSPOINTS_ELEVEN_CROSS,
                                                         0,
                                                         &error,
                                                         NULL);
    if (error != NULL) {
        fail_if (error != NULL,
                 "gdigicam-manager: error is \n\"%s\"\n instead of succes.",
                 error->message);
        g_error_free (error);
        error = NULL;
    }
    fail_if (!result,
             "gdigicam-manager: error setting focus region pattern to %i.",
             G_DIGICAM_FOCUSPOINTS_ELEVEN_CROSS);
    g_digicam_manager_get_focus_region_pattern (full_featured_manager,
                                                &focus_points,
                                                &active_points,
                                                &error);
    fail_if (focus_points != G_DIGICAM_FOCUSPOINTS_ELEVEN_CROSS,
             "gdigicam-manager: retrieved focus points is %i and it should be %i.",
             focus_points,
             G_DIGICAM_FOCUSPOINTS_ELEVEN_CROSS);

    /* Test 7 */
    result = g_digicam_manager_set_focus_region_pattern (full_featured_manager,
                                                         G_DIGICAM_FOCUSPOINTS_TWELVE_3X4,
                                                         0,
                                                         &error,
                                                         NULL);
    if (error != NULL) {
        fail_if (error != NULL,
                 "gdigicam-manager: error is \n\"%s\"\n instead of succes.",
                 error->message);
        g_error_free (error);
        error = NULL;
    }
    fail_if (!result,
             "gdigicam-manager: error setting focus region pattern to %i.",
             G_DIGICAM_FOCUSPOINTS_TWELVE_3X4);
    g_digicam_manager_get_focus_region_pattern (full_featured_manager,
                                                &focus_points,
                                                &active_points,
                                                &error);
    fail_if (focus_points != G_DIGICAM_FOCUSPOINTS_TWELVE_3X4,
             "gdigicam-manager: retrieved focus points is %i and it should be %i.",
             focus_points,
             G_DIGICAM_FOCUSPOINTS_TWELVE_3X4);

    /* Test 8 */
    result = g_digicam_manager_set_focus_region_pattern (full_featured_manager,
                                                         G_DIGICAM_FOCUSPOINTS_TWELVE_4X3,
                                                         0,
                                                         &error,
                                                         NULL);
    if (error != NULL) {
        fail_if (error != NULL,
                 "gdigicam-manager: error is \n\"%s\"\n instead of succes.",
                 error->message);
        g_error_free (error);
        error = NULL;
    }
    fail_if (!result,
             "gdigicam-manager: error setting focus region pattern to %i.",
             G_DIGICAM_FOCUSPOINTS_TWELVE_4X3);
    g_digicam_manager_get_focus_region_pattern (full_featured_manager,
                                                &focus_points,
                                                &active_points,
                                                &error);
    fail_if (focus_points != G_DIGICAM_FOCUSPOINTS_TWELVE_4X3,
             "gdigicam-manager: retrieved focus points is %i and it should be %i.",
             focus_points,
             G_DIGICAM_FOCUSPOINTS_TWELVE_4X3);


    /* Test 9 */
    result = g_digicam_manager_set_focus_region_pattern (full_featured_manager,
                                                         G_DIGICAM_FOCUSPOINTS_SIXTEEN_SQUARE,
                                                         0,
                                                         &error,
                                                         NULL);
    if (error != NULL) {
        fail_if (error != NULL,
                 "gdigicam-manager: error is \n\"%s\"\n instead of succes.",
                 error->message);
        g_error_free (error);
        error = NULL;
    }
    fail_if (!result,
             "gdigicam-manager: error setting focus region pattern to %i.",
             G_DIGICAM_FOCUSPOINTS_SIXTEEN_SQUARE);
    g_digicam_manager_get_focus_region_pattern (full_featured_manager,
                                                &focus_points,
                                                &active_points,
                                                &error);
    fail_if (focus_points != G_DIGICAM_FOCUSPOINTS_SIXTEEN_SQUARE,
             "gdigicam-manager: retrieved focus points is %i and it should be %i.",
             focus_points,
             G_DIGICAM_FOCUSPOINTS_SIXTEEN_SQUARE);

    /* Test 10 */
    result = g_digicam_manager_set_focus_region_pattern (full_featured_manager,
                                                         G_DIGICAM_FOCUSPOINTS_CUSTOM,
                                                         infinite,
                                                         &error,
                                                         NULL);
    if (error != NULL) {
        fail_if (error != NULL,
                 "gdigicam-manager: error is \n\"%s\"\n instead of succes.",
                 error->message);
        g_error_free (error);
        error = NULL;
    }
    fail_if (!result,
             "gdigicam-manager: error setting focus region pattern to %i.",
             G_DIGICAM_FOCUSPOINTS_CUSTOM);
    g_digicam_manager_get_focus_region_pattern (full_featured_manager,
                                                &focus_points,
                                                &active_points,
                                                &error);
    fail_if (focus_points != G_DIGICAM_FOCUSPOINTS_CUSTOM,
             "gdigicam-manager: retrieved focus points is %i and it should be %i.",
             focus_points,
             G_DIGICAM_FOCUSPOINTS_CUSTOM);
    fail_if (active_points != infinite,
             "gdigicam-manager: retrieved active points is %i and it should be %i.",
             active_points,
             infinite);

}
END_TEST

/**
 * Purpose: test set_get_focus_region_pattern with invalid values in a #GDigicamManager
 * Cases considered:
 *    - using an invalid gdigicam-manager
 *    - using a camera with not focus region pattern setting feature
 */
START_TEST (test_set_get_focus_region_pattern_invalid)
{
    gboolean result;

    /* Test 1 */
    fail_if (g_digicam_manager_set_focus_region_pattern (NULL,
                                                         G_DIGICAM_FOCUSPOINTS_ONE,
                                                         0,
                                                         &error,
                                                         NULL),
             "gdigicam-manager: setting focus region_pattern with invalid gdigicam-manager.");


    /* Test 2 */
    g_digicam_manager_set_gstreamer_bin (minimum_featured_manager,
                                         minimum_featured_camera_bin,
                                         minimum_featured_descriptor,
                                         NULL);

    fail_if (!g_digicam_manager_set_mode (minimum_featured_manager,
                                          G_DIGICAM_MODE_STILL,
                                          &error,
                                          NULL),
             "gdigicam-manager: still mode could not be set.");

    if (error != NULL)
        g_error_free (error);
    error = NULL;

    result = g_digicam_manager_set_focus_region_pattern (minimum_featured_manager,
                                                         G_DIGICAM_FOCUSPOINTS_ONE,
                                                         0,
                                                         &error,
                                                         NULL);
    if (error != NULL) {
        fail_if (!g_error_matches (error,
                                   G_DIGICAM_ERROR,
                                   G_DIGICAM_ERROR_FOCUSMODE_NOT_SUPPORTED),
                 "gdigicam-manager: error is %i instead of %i.",
                 error->code,
                 G_DIGICAM_ERROR_FOCUSMODE_NOT_SUPPORTED);
        g_error_free (error);
        error = NULL;
    } else {
        fail_if (error == NULL,
                 "gdigicam-manager: there is not error returned.");
    }
    fail_if (result,
             "gdigicam-manager: setting focus region  with no capable bin.");




}
END_TEST


/* ----- Test case for capture_still_picture -----*/

/**
 * Purpose: test capture_still_picture operation in a#GDigicamManager
 * Cases considered:
 *    - capture still picture.
 */
START_TEST (test_capture_still_picture_regular)
{
    gboolean result;

    g_digicam_manager_set_gstreamer_bin (full_featured_manager,
                                         full_featured_camera_bin,
                                         full_featured_descriptor,
                                         NULL);
    window = create_test_window ();
    show_test_window (window);
    g_digicam_manager_set_mode (full_featured_manager,
				G_DIGICAM_MODE_STILL,
				&error,
				NULL);
    if (error != NULL)
        g_error_free (error);
    error = NULL;

    /* Test 1 */
    result = g_digicam_manager_capture_still_picture (full_featured_manager,
                                                      "file.jpg",
                                                      &error,
                                                      NULL);
    if (error != NULL) {
        fail_if (error != NULL,
                 "gdigicam-manager: error is \n\"%s\"\n instead of succes.",
                 error->message);
        g_error_free (error);
        error = NULL;
    }
    fail_if (!result,
             "gdigicam-manager: error capturing still picture.");
}
END_TEST

/**
 * Purpose: test capture_still_picture with invalid values in a #GDigicamManager
 * Cases considered:
 *    - using an invalid gdigicam-manager
 *    - using a gdigicam-manager not in still mode
 */
START_TEST (test_capture_still_picture_invalid)
{
    gboolean result;

    /* Test 1 */
    fail_if (g_digicam_manager_capture_still_picture (NULL,
                                                      "file.jpg",
                                                      &error,
                                                      NULL),
             "gdigicam-manager: setting focus region_pattern with invalid gdigicam-manager.");



    /* Test 2 */
    g_digicam_manager_set_gstreamer_bin (full_featured_manager,
                                         full_featured_camera_bin,
                                         full_featured_descriptor,
                                         NULL);
    window = create_test_window ();
    show_test_window (window);
    fail_if (!g_digicam_manager_set_mode (full_featured_manager,
                                          G_DIGICAM_MODE_VIDEO,
                                          &error,
                                          NULL),
             "gdigicam-manager: still mode could not be set.");

    if (error != NULL)
        g_error_free (error);
    error = NULL;

    result = g_digicam_manager_capture_still_picture (full_featured_manager,
                                                      "file.jpg",
                                                      &error,
                                                      NULL);
    if (error != NULL) {
        fail_if (!g_error_matches (error,
                                   G_DIGICAM_ERROR,
                                   G_DIGICAM_ERROR_INVALID_MODE),
                 "gdigicam-manager: error is  \n\"%s\"\n instead of %i.",
                 error->message,
                 G_DIGICAM_ERROR_INVALID_MODE);
        g_error_free (error);
        error = NULL;
    } else {
        fail_if (error == NULL,
                 "gdigicam-manager: there is not error returned.");
    }
    fail_if (result,
             "gdigicam-manager: capture still picture with a bin without that capability.");



}
END_TEST


/* ----- Test case for set aspect_ratio_resolution -----*/

/**
 * Purpose: test setting and getting limit resolutions and aspect ratios in a
 * #GDigicamManager
 * Cases considered:
 *    - set/get resolution from just created #GDigicamManager.
 *    - set/get resolution to a none featured #GDigicamManager.
 *    - set/get resolution to a minimum featured #GDigicamManager.
 *    - get resolution from a featured but not set #GDigicamManager.
 */
START_TEST (test_set_aspect_ratio_resolution_limit)
{
    GDigicamResolution gotten_resolution = 0;
    GDigicamAspectratio gotten_aspect_ratio = 0;

    /* Test 1 */
    fail_if (g_digicam_manager_set_aspect_ratio_resolution (no_featured_manager,
							     G_DIGICAM_ASPECTRATIO_4X3,
							     G_DIGICAM_RESOLUTION_HIGH,
							     &error,
							     NULL),
             "gdigicam-manager: an error has not happened.");
    fail_if (NULL == error,
             "gdigicam-manager: error was not set.");
    fail_if (!g_error_matches (error,
                               G_DIGICAM_ERROR,
                               G_DIGICAM_ERROR_GSTREAMER_BIN_NOT_SET),
             "gdigicam-manager: error is  \n\"%s\"\n instead of "
             "\"bin not set\".",
             error->message);
    g_error_free (error);
    error = NULL;

    fail_if (g_digicam_manager_get_resolution
             (no_featured_manager, &gotten_resolution, &error),
             "gdigicam-manager: an error has not happened.");
    fail_if (0 != gotten_resolution,
             "gdigicam-manager: the resolution was not the \"unset\" "
             "value, it was \"%d\".",
             gotten_resolution);
    fail_if (NULL == error,
             "gdigicam-manager: error was not set.");
    fail_if (!g_error_matches (error,
                               G_DIGICAM_ERROR,
                               G_DIGICAM_ERROR_GSTREAMER_BIN_NOT_SET),
             "gdigicam-manager: error is  \n\"%s\"\n instead of "
             "\"bin not set\".",
             error->message);
    g_error_free (error);
    error = NULL;
    gotten_resolution = 0;

    fail_if (g_digicam_manager_get_aspect_ratio
             (no_featured_manager, &gotten_aspect_ratio, &error),
             "gdigicam-manager: an error has not happened.");
    fail_if (0 != gotten_aspect_ratio,
             "gdigicam-manager: the aspect ratio was not the \"unset\" "
             "value, it was \"%d\".",
             gotten_aspect_ratio);
    fail_if (NULL == error,
             "gdigicam-manager: error was not set.");
    fail_if (!g_error_matches (error,
                               G_DIGICAM_ERROR,
                               G_DIGICAM_ERROR_GSTREAMER_BIN_NOT_SET),
             "gdigicam-manager: error is  \n\"%s\"\n instead of "
             "\"bin not set\".",
             error->message);
    g_error_free (error);
    error = NULL;
    gotten_aspect_ratio = 0;


    /* Test 2 */
    g_digicam_manager_set_gstreamer_bin (no_featured_manager,
                                         no_featured_camera_bin,
                                         no_featured_descriptor,
                                         NULL);
    fail_if (g_digicam_manager_set_aspect_ratio_resolution (no_featured_manager,
							     G_DIGICAM_ASPECTRATIO_4X3,
							     G_DIGICAM_RESOLUTION_HIGH,
							     &error,
                                                NULL),
             "gdigicam-manager: an error has not happened.");
    fail_if (NULL == error,
             "gdigicam-manager: error was not set.");
    fail_if (!g_error_matches (error,
                               G_DIGICAM_ERROR,
                               G_DIGICAM_ERROR_ASPECTRATIO_NOT_SUPPORTED),
             "gdigicam-manager: error is  \n\"%s\"\n instead of "
             "\"aspect ratio not supported\".",
             error->message);
    g_error_free (error);
    error = NULL;

    fail_if (g_digicam_manager_get_resolution
             (no_featured_manager, &gotten_resolution, &error),
             "gdigicam-manager: an error has not happened.");
    fail_if (0 != gotten_resolution,
             "gdigicam-manager: the resolution was not the \"unset\" "
             "value, it was \"%d\".",
             gotten_resolution);
    fail_if (NULL == error,
             "gdigicam-manager: error was not set.");
    fail_if (!g_error_matches (error,
                               G_DIGICAM_ERROR,
                               G_DIGICAM_ERROR_RESOLUTION_NOT_SUPPORTED),
             "gdigicam-manager: error is  \n\"%s\"\n instead of "
             "\"aspect ratio not supported\".",
             error->message);
    g_error_free (error);
    error = NULL;

    fail_if (g_digicam_manager_get_aspect_ratio
             (no_featured_manager, &gotten_aspect_ratio, &error),
             "gdigicam-manager: an error has not happened.");
    fail_if (0 != gotten_aspect_ratio,
             "gdigicam-manager: the aspect ratio was not the \"unset\" "
             "value, it was \"%d\".",
             gotten_aspect_ratio);
    fail_if (NULL == error,
             "gdigicam-manager: error was not set.");
    fail_if (!g_error_matches (error,
                               G_DIGICAM_ERROR,
                               G_DIGICAM_ERROR_ASPECTRATIO_NOT_SUPPORTED),
             "gdigicam-manager: error is  \n\"%s\"\n instead of "
             "\"aspect ratio not supported\".",
             error->message);
    g_error_free (error);
    error = NULL;

    /* Test 3 */
    g_digicam_manager_set_gstreamer_bin (minimum_featured_manager,
                                         minimum_featured_camera_bin,
                                         minimum_featured_descriptor,
                                         NULL);
    fail_if (g_digicam_manager_set_aspect_ratio_resolution (minimum_featured_manager,
							     G_DIGICAM_ASPECTRATIO_4X3,
							     G_DIGICAM_RESOLUTION_HIGH,
							     &error,
							     NULL),
             "gdigicam-manager: an error has not happened.");
    fail_if (NULL == error,
             "gdigicam-manager: error was not set.");
    fail_if (!g_error_matches (error,
                               G_DIGICAM_ERROR,
                               G_DIGICAM_ERROR_ASPECTRATIO_NOT_SUPPORTED),
             "gdigicam-manager: error is  \n\"%s\"\n instead of "
             "\"aspect ratio not supported\".",
             error->message);
    g_error_free (error);
    error = NULL;

    fail_if (g_digicam_manager_get_resolution
             (minimum_featured_manager, &gotten_resolution, &error),
             "gdigicam-manager: an error has not happened.");
    fail_if (0 != gotten_resolution,
             "gdigicam-manager: the resolution was not the \"unset\" "
             "value, it was \"%d\".",
             gotten_resolution);
    fail_if (NULL == error,
             "gdigicam-manager: error was not set.");
    fail_if (!g_error_matches (error,
                               G_DIGICAM_ERROR,
                               G_DIGICAM_ERROR_RESOLUTION_NOT_SUPPORTED),
             "gdigicam-manager: error is  \n\"%s\"\n instead of "
             "\"resolution not supported\".",
             error->message);
    g_error_free (error);
    error = NULL;

    fail_if (g_digicam_manager_get_aspect_ratio
             (minimum_featured_manager, &gotten_aspect_ratio, &error),
             "gdigicam-manager: an error has not happened.");
    fail_if (0 != gotten_aspect_ratio,
             "gdigicam-manager: the aspect ratio was not the \"unset\" "
             "value, it was \"%d\".",
             gotten_aspect_ratio);
    fail_if (NULL == error,
             "gdigicam-manager: error was not set.");
    fail_if (!g_error_matches (error,
                               G_DIGICAM_ERROR,
                               G_DIGICAM_ERROR_ASPECTRATIO_NOT_SUPPORTED),
             "gdigicam-manager: error is  \n\"%s\"\n instead of "
             "\"aspect ratio not supported\".",
             error->message);
    g_error_free (error);
    error = NULL;

    /* Test 4 */
    g_digicam_manager_set_gstreamer_bin (full_featured_manager,
                                         full_featured_camera_bin,
                                         full_featured_descriptor,
                                         NULL);
    fail_if (!g_digicam_manager_get_resolution
             (full_featured_manager, &gotten_resolution, &error),
             "gdigicam-manager: an error has happened.");
    fail_if (0 != gotten_resolution,
             "gdigicam-manager: the resolution was not the \"unset\" "
             "value, it was \"%d\".",
             gotten_resolution);
    fail_if (NULL != error,
             "gdigicam-manager: error was set.");

    fail_if (!g_digicam_manager_get_aspect_ratio
             (full_featured_manager, &gotten_aspect_ratio, &error),
             "gdigicam-manager: an error has happened.");
    fail_if (0 != gotten_aspect_ratio,
             "gdigicam-manager: the aspect ratio was not the \"unset\" "
             "value, it was \"%d\".",
             gotten_aspect_ratio);
    fail_if (NULL != error,
             "gdigicam-manager: error was set.");
}
END_TEST

/**
 * Purpose: test setting and getting a valid resolution and aspect ratio in a
 * #GDigicamManager
 * Cases considered:
 *    - set/get resolution and aspect ratio to a featured #GDigicamManager.
 *    - set/get again another resolution and aspect ratio to a featured #GDigicamManager.
 */
START_TEST (test_set_aspect_ratio_resolution_regular)
{
    GDigicamResolution gotten_resolution = 0;
    GDigicamAspectratio gotten_aspect_ratio = 0;

    /* Test 1 */
    g_digicam_manager_set_gstreamer_bin (full_featured_manager,
                                         full_featured_camera_bin,
                                         full_featured_descriptor,
                                         NULL);
    fail_if (!g_digicam_manager_set_aspect_ratio_resolution (full_featured_manager,
							     G_DIGICAM_ASPECTRATIO_4X3,
							     G_DIGICAM_RESOLUTION_HIGH,
							     &error,
							     NULL),
             "gdigicam-manager: an error has happened.");
    fail_if (NULL != error,
             "gdigicam-manager: error was set.");

    fail_if (!g_digicam_manager_get_resolution
             (full_featured_manager, &gotten_resolution, &error),
             "gdigicam-manager: an error has happened.");
    fail_if (G_DIGICAM_RESOLUTION_HIGH != gotten_resolution,
             "gdigicam-manager: the resolution was not "
             "the previously provided.");
    fail_if (NULL != error,
             "gdigicam-manager: error was set.");
    gotten_resolution = 0;

    fail_if (!g_digicam_manager_get_aspect_ratio
             (full_featured_manager, &gotten_aspect_ratio, &error),
             "gdigicam-manager: an error has happened.");
    fail_if (G_DIGICAM_ASPECTRATIO_4X3 != gotten_aspect_ratio,
             "gdigicam-manager: the aspect ratio was not "
             "the previously provided.");
    fail_if (NULL != error,
             "gdigicam-manager: error was set.");
    gotten_aspect_ratio = 0;

    /* Test 2 */
    fail_if (!g_digicam_manager_set_aspect_ratio_resolution (full_featured_manager,
							     G_DIGICAM_ASPECTRATIO_16X9,
							     G_DIGICAM_RESOLUTION_LOW,
							     &error,
							     NULL),
             "gdigicam-manager: an error has happened.");
    fail_if (NULL != error,
             "gdigicam-manager: error was set.");

    fail_if (!g_digicam_manager_get_resolution
             (full_featured_manager, &gotten_resolution, &error),
             "gdigicam-manager: an error has happened.");
    fail_if (G_DIGICAM_RESOLUTION_LOW != gotten_resolution,
             "gdigicam-manager: the resolution was not "
             "the previously provided.");
    fail_if (NULL != error,
             "gdigicam-manager: error was set.");

    fail_if (!g_digicam_manager_get_aspect_ratio
             (full_featured_manager, &gotten_aspect_ratio, &error),
             "gdigicam-manager: an error has happened.");
    fail_if (G_DIGICAM_ASPECTRATIO_16X9 != gotten_aspect_ratio,
             "gdigicam-manager: the aspect ratio was not "
             "the previously provided.");
    fail_if (NULL != error,
             "gdigicam-manager: error was set.");
    gotten_aspect_ratio = 0;
}
END_TEST

/**
 * Purpose: test setting and getting a resolution and aspect ratio in an invalid
 * #GDigicamManager
 * Cases considered:
 *    - set/get the resolution and aspect ratio to/from a NULL
 *      #GDigicamManager. Parameters remain the same.
 *    - set/get the resolution and aspect ratio to/from an invalid
 *      #GDigicamManager. Parameters remain the same.
 */
START_TEST (test_set_aspect_ratio_resolution_invalid)
{
    GDigicamResolution gotten_resolution = 0;
    GDigicamAspectratio gotten_aspect_ratio = 0;

    /* Test 1 */
    fail_if (g_digicam_manager_set_aspect_ratio_resolution (NULL,
							    G_DIGICAM_ASPECTRATIO_4X3,
							    G_DIGICAM_RESOLUTION_HIGH,
							    &error,
							    NULL),
             "gdigicam-manager: there was no error.");
    fail_if (NULL != error,
             "gdigicam-manager: error has changed.");

    fail_if (g_digicam_manager_get_resolution
             (NULL, &gotten_resolution, &error),
             "gdigicam-manager: there was no error.");
    fail_if (0 != gotten_resolution,
             "gdigicam-manager: the resolution was not the \"unset\" "
             "value, it was \"%d\".",
             gotten_resolution);
    fail_if (NULL != error,
             "gdigicam-manager: error has changed.");
    gotten_resolution = 0;

    fail_if (g_digicam_manager_get_aspect_ratio
             (NULL, &gotten_aspect_ratio, &error),
             "gdigicam-manager: there was no error.");
    fail_if (0 != gotten_aspect_ratio,
             "gdigicam-manager: the aspect ratio was not the \"unset\" "
             "value, it was \"%d\".",
             gotten_resolution);
    fail_if (NULL != error,
             "gdigicam-manager: error has changed.");
    gotten_aspect_ratio = 0;

    /* Test 2 */
    fail_if (g_digicam_manager_set_aspect_ratio_resolution (G_DIGICAM_MANAGER (window),
							    G_DIGICAM_ASPECTRATIO_4X3,
							    G_DIGICAM_RESOLUTION_HIGH,
							    &error,
							    NULL),
             "gdigicam-manager: there was no error.");
    fail_if (NULL != error,
             "gdigicam-manager: error has changed.");

    fail_if (g_digicam_manager_get_resolution
             (G_DIGICAM_MANAGER (window), &gotten_resolution, &error),
             "gdigicam-manager: there was no error.");
    fail_if (0 != gotten_resolution,
             "gdigicam-manager: the resolution was not the \"unset\" "
             "value, it was \"%d\".",
             gotten_resolution);
    fail_if (NULL != error,
             "gdigicam-manager: error has changed.");
    gotten_resolution = 0;

    fail_if (g_digicam_manager_get_aspect_ratio
             (G_DIGICAM_MANAGER (window), &gotten_aspect_ratio, &error),
             "gdigicam-manager: there was no error.");
    fail_if (0 != gotten_aspect_ratio,
             "gdigicam-manager: the aspect ratio was not the \"unset\" "
             "value, it was \"%d\".",
             gotten_aspect_ratio);
    fail_if (NULL != error,
             "gdigicam-manager: error has changed.");
    gotten_aspect_ratio = 0;
}
END_TEST



/* ----- Test case for set/get_exposure_comp -----*/


/**
 * Purpose: test setting and getting limit exposure compensation in a
 * #GDigicamManager
 * Cases considered:
 *    - set/get exposure compensation from just created #GDigicamManager.
 *    - set/get exposure compensation to a none featured #GDigicamManager.
 *    - set/get exposure compensation to a minimum featured #GDigicamManager.
 *    - get exposure compensation from a featured but not set #GDigicamManager.
 */
START_TEST (test_set_get_exposure_comp_limit)
{
    /* TODO: What is the upper and lower limit ? Add case */

    gdouble gotten_comp = 0.0;

    /* Test 1 */
    fail_if (g_digicam_manager_set_exposure_comp (no_featured_manager,
                                                  2.0,
                                                  &error,
                                                  NULL),
             "gdigicam-manager: an error has not happened.");
    fail_if (NULL == error,
             "gdigicam-manager: error was not set.");
    fail_if (!g_error_matches (error,
                               G_DIGICAM_ERROR,
                               G_DIGICAM_ERROR_GSTREAMER_BIN_NOT_SET),
             "gdigicam-manager: error is  \n\"%s\"\n instead of "
             "\"exposure mode not supported\".",
             error->message);
    g_error_free (error);
    error = NULL;

    fail_if (g_digicam_manager_get_exposure_comp
             (no_featured_manager,
              &gotten_comp,
              &error),
             "gdigicam-manager: an error has not happened.");
    fail_if (0.0 != gotten_comp,
             "gdigicam-manager: the exposure mode was not the \"unset\" "
             "value, it was \"%f\".",
             gotten_comp);
    fail_if (NULL == error,
             "gdigicam-manager: error was not set.");
    fail_if (!g_error_matches (error,
                               G_DIGICAM_ERROR,
                               G_DIGICAM_ERROR_GSTREAMER_BIN_NOT_SET),
             "gdigicam-manager: error is  \n\"%s\"\n instead of "
             "\"exposure mode not supported\".",
             error->message);
    g_error_free (error);
    error = NULL;
    gotten_comp = 0.0;


    /* Test 2 */
    g_digicam_manager_set_gstreamer_bin (minimum_featured_manager,
                                         minimum_featured_camera_bin,
                                         minimum_featured_descriptor,
                                         NULL);
    fail_if (g_digicam_manager_set_exposure_comp (minimum_featured_manager,
                                                  2.0,
                                                  &error,
                                                  NULL),
             "gdigicam-manager: an error has not happened.");
    fail_if (NULL == error,
             "gdigicam-manager: error was not set.");
    fail_if (!g_error_matches (error,
                               G_DIGICAM_ERROR,
			       G_DIGICAM_ERROR_FAILED),
             "gdigicam-manager: error is  \n\"%s\"\n instead of "
             "\"exposure mode not supported\".",
             error->message);
    g_error_free (error);
    error = NULL;

    fail_if (g_digicam_manager_get_exposure_comp
             (minimum_featured_manager,
              &gotten_comp,
              &error),
             "gdigicam-manager: an error has not happened.");
    fail_if (0.0 != gotten_comp,
             "gdigicam-manager: the exposure compensation was not the \"unset\" "
             "value, it was \"%f\".",
             gotten_comp);
    fail_if (NULL == error,
             "gdigicam-manager: error was not set.");
    fail_if (!g_error_matches (error,
                               G_DIGICAM_ERROR,
			       G_DIGICAM_ERROR_EXPOSUREMODE_NOT_SUPPORTED),
             "gdigicam-manager: error is  \n\"%s\"\n instead of "
             "\"exposure compensation not supported\".",
             error->message);
    g_error_free (error);
    error = NULL;
    gotten_comp = 0;

    /* Test 3 */
    g_digicam_manager_set_gstreamer_bin (full_featured_manager,
                                         full_featured_camera_bin,
                                         full_featured_descriptor,
                                         NULL);
    fail_if (!g_digicam_manager_get_exposure_comp
             (full_featured_manager,
              &gotten_comp,
              &error),
             "gdigicam-manager: an error has happened.");
    fail_if (0.0 != gotten_comp,
             "gdigicam-manager: the white balance mode was not the \"unset\" "
             "value, it was \"%d\".",
             gotten_comp);
    fail_if (0.0 != gotten_comp,
             "gdigicam-manager: the compensation was not the \"unset\" "
             "value, it was \"%d\".",
             gotten_comp);
    fail_if (NULL != error,
             "gdigicam-manager: error was set.");

}
END_TEST


/**
 * Purpose: test setting and getting regular exposure compensation in a
 * #GDigicamManager
 * Cases considered:
 *    - set/get exposure compensation from a featured #GDigicamManager.
 */
START_TEST (test_set_get_exposure_comp_regular)
{
    gdouble gotten_comp = 0.0;

    g_digicam_manager_set_gstreamer_bin (full_featured_manager,
                                         full_featured_camera_bin,
                                         full_featured_descriptor,
                                         NULL);

    /* Test 1 */
    fail_if (!g_digicam_manager_set_exposure_comp (full_featured_manager,
                                                   2.0,
                                                   &error,
                                                   NULL),
             "gdigicam-manager: an error has happened.");
    fail_if (NULL != error,
             "gdigicam-manager: error was set.");
    g_error_free (error);
    error = NULL;

    fail_if (!g_digicam_manager_get_exposure_comp
             (full_featured_manager,
              &gotten_comp,
              &error),
             "gdigicam-manager: an error has happened.");
    fail_if (2.0 != gotten_comp,
             "gdigicam-manager: the exposure mode was not the \"unset\" "
             "value, it was \"%f\".",
             gotten_comp);
    fail_if (NULL != error,
             "gdigicam-manager: error was set.");
    g_error_free (error);
    error = NULL;

    /* Test 2 */
    fail_if (!g_digicam_manager_set_exposure_comp (full_featured_manager,
                                                   2.0,
                                                   &error,
                                                   NULL),
             "gdigicam-manager: an error has happened.");
    fail_if (NULL != error,
             "gdigicam-manager: error was set.");
    g_error_free (error);
    error = NULL;

}
END_TEST


/**
 * Purpose: test setting and getting regular exposure compensation in a
 * #GDigicamManager
 * Cases considered:
 *    - set/get exposure compensation from a NULL #GDigicamManager.
 */
START_TEST (test_set_get_exposure_comp_invalid)
{
    gdouble gotten_comp = 0.0;

    /* Test 1 */
    fail_if (g_digicam_manager_set_exposure_comp (NULL,
                                                  2.0,
                                                  &error,
                                                  NULL),
             "gdigicam-manager: an error has not happened.");

    fail_if (g_digicam_manager_get_exposure_comp
             (NULL,
              &gotten_comp,
              &error),
             "gdigicam-manager: an error has not happened.");
    fail_if (0.0 != gotten_comp,
             "gdigicam-manager: the exposure compensation was the \"set\" "
             "value, it was \"%f\".",
             gotten_comp);
    gotten_comp = 0.0;
}
END_TEST



/* ----- Test case for set/get_quality -----*/


/**
 * Purpose: test setting and getting quality in a #GDigicamManager
 * Cases considered:
 *    - set/get quality from just created #GDigicamManager.
 *    - set/get quality to a none featured #GDigicamManager.
 *    - set/get quality to a minimum featured #GDigicamManager.
 *    - get quality from a featured but not set #GDigicamManager.
 */
START_TEST (test_set_get_quality_limit)
{

    GDigicamQuality gotten_quality = 0;

    /* Test 1 */
    fail_if (g_digicam_manager_set_quality (no_featured_manager,
                                            G_DIGICAM_QUALITY_HIGH,
                                            &error,
                                            NULL),
             "gdigicam-manager: an error has not happened.");
    fail_if (NULL == error,
             "gdigicam-manager: error was not set.");
    fail_if (!g_error_matches (error,
                               G_DIGICAM_ERROR,
                               G_DIGICAM_ERROR_GSTREAMER_BIN_NOT_SET),
             "gdigicam-manager: error is  \n\"%s\"\n instead of "
             "\"quality not supported\".",
             error->message);
    g_error_free (error);
    error = NULL;

    fail_if (g_digicam_manager_get_quality
             (no_featured_manager,
              &gotten_quality,
              &error),
             "gdigicam-manager: an error has not happened.");
    fail_if (0 != gotten_quality,
             "gdigicam-manager: the quality was not the \"unset\" "
             "value, it was \"%d\".",
             gotten_quality);
    fail_if (NULL == error,
             "gdigicam-manager: error was not set.");
    fail_if (!g_error_matches (error,
                               G_DIGICAM_ERROR,
                               G_DIGICAM_ERROR_GSTREAMER_BIN_NOT_SET),
             "gdigicam-manager: error is  \n\"%s\"\n instead of "
             "\"quality not supported\".",
             error->message);
    g_error_free (error);
    error = NULL;
    gotten_quality = 0;


    /* Test 2 */
    g_digicam_manager_set_gstreamer_bin (minimum_featured_manager,
                                         minimum_featured_camera_bin,
                                         minimum_featured_descriptor,
                                         NULL);
    fail_if (g_digicam_manager_set_quality (minimum_featured_manager,
                                            G_DIGICAM_QUALITY_HIGH,
                                            &error,
                                            NULL),
             "gdigicam-manager: an error has not happened.");
    fail_if (NULL == error,
             "gdigicam-manager: error was not set.");
    fail_if (!g_error_matches (error,
                               G_DIGICAM_ERROR,
			       G_DIGICAM_ERROR_QUALITY_NOT_SUPPORTED),
             "gdigicam-manager: error is  \n\"%s\"\n instead of "
             "\"quality not supported\".",
             error->message);
    g_error_free (error);
    error = NULL;

    fail_if (g_digicam_manager_get_quality
             (minimum_featured_manager,
              &gotten_quality,
              &error),
             "gdigicam-manager: an error has not happened.");
    fail_if (0 != gotten_quality,
             "gdigicam-manager: the quality was not the \"unset\" "
             "value, it was \"%d\".",
             gotten_quality);
    fail_if (NULL == error,
             "gdigicam-manager: error was not set.");
    fail_if (!g_error_matches (error,
                               G_DIGICAM_ERROR,
			       G_DIGICAM_ERROR_QUALITY_NOT_SUPPORTED),
             "gdigicam-manager: error is  \n\"%s\"\n instead of "
             "\"quality not supported\".",
             error->message);
    g_error_free (error);
    error = NULL;
    gotten_quality = 0;

    /* Test 3 */
    g_digicam_manager_set_gstreamer_bin (full_featured_manager,
                                         full_featured_camera_bin,
                                         full_featured_descriptor,
                                         NULL);
    fail_if (!g_digicam_manager_get_quality
             (full_featured_manager,
              &gotten_quality,
              &error),
             "gdigicam-manager: an error has happened.");
    fail_if (0 != gotten_quality,
             "gdigicam-manager: the quality was not the \"unset\" "
             "value, it was \"%d\".",
             gotten_quality);
    fail_if (NULL != error,
             "gdigicam-manager: error was set.");

    fail_if (g_digicam_manager_set_quality
             (full_featured_manager,
              20,
              &error,
	      NULL),
             "gdigicam-manager: an error has not happened.");
    fail_if (NULL == error,
             "gdigicam-manager: error was not set.");
    fail_if (!g_error_matches (error,
                               G_DIGICAM_ERROR,
			       G_DIGICAM_ERROR_QUALITY_NOT_SUPPORTED),
             "gdigicam-manager: error is  \n\"%s\"\n instead of "
             "\"quality not supported\".",
             error->message);
    g_error_free (error);
    error = NULL;

}
END_TEST


/**
 * Purpose: test setting and getting regular quality in a
 * #GDigicamManager
 * Cases considered:
 *    - set/get quality from a featured #GDigicamManager.
 */
START_TEST (test_set_get_quality_regular)
{
    GDigicamQuality gotten_quality = 0;

    g_digicam_manager_set_gstreamer_bin (full_featured_manager,
                                         full_featured_camera_bin,
                                         full_featured_descriptor,
                                         NULL);

    /* Test 1 */
    fail_if (!g_digicam_manager_set_quality (full_featured_manager,
                                             G_DIGICAM_QUALITY_HIGH,
                                             &error,
                                             NULL),
             "gdigicam-manager: an error has happened.");
    fail_if (NULL != error,
             "gdigicam-manager: error was set.");
    g_error_free (error);
    error = NULL;

    fail_if (!g_digicam_manager_get_quality
             (full_featured_manager,
              &gotten_quality,
              &error),
             "gdigicam-manager: an error has happened.");
    fail_if (G_DIGICAM_QUALITY_HIGH != gotten_quality,
             "gdigicam-manager: the quality was not the \"unset\" "
             "value, it was \"%d\".",
             gotten_quality);
    fail_if (NULL != error,
             "gdigicam-manager: error was set.");
    g_error_free (error);
    error = NULL;

    /* Test 2 */
    fail_if (!g_digicam_manager_set_quality (full_featured_manager,
                                             G_DIGICAM_QUALITY_HIGH,
                                             &error,
                                             NULL),
             "gdigicam-manager: an error has happened.");
    fail_if (NULL != error,
             "gdigicam-manager: error was set.");
    g_error_free (error);
    error = NULL;

}
END_TEST


/**
 * Purpose: test setting and getting invalid quality in a
 * #GDigicamManager
 * Cases considered:
 *    - set/get quality from a NULL #GDigicamManager.
 */
START_TEST (test_set_get_quality_invalid)
{
    GDigicamQuality gotten_quality = 0;

    /* Test 1 */
    fail_if (g_digicam_manager_set_quality (NULL,
                                            G_DIGICAM_QUALITY_HIGH,
                                            &error,
                                            NULL),
             "gdigicam-manager: an error has not happened.");

    fail_if (g_digicam_manager_get_quality
             (NULL,
              &gotten_quality,
              &error),
             "gdigicam-manager: an error has not happened.");
    fail_if (0 != gotten_quality,
             "gdigicam-manager: the quality was the \"set\" "
             "value, it was \"%d\".",
             gotten_quality);
    gotten_quality = 0;
}
END_TEST



/* ----- Test case for set/get_locks -----*/


/**
 * Purpose: test setting and getting locks in a
 * #GDigicamManager
 * Cases considered:
 *    - set/get locks from just created #GDigicamManager.
 *    - set/get locks to a none featured #GDigicamManager.
 *    - set/get locks to a minimum featured #GDigicamManager.
 *    - get lock from a featured but not set #GDigicamManager.
 */
START_TEST (test_set_get_locks_limit)
{

    GDigicamLock gotten_lock = 0;

    /* Test 1 */
    fail_if (g_digicam_manager_set_locks (no_featured_manager,
                                          G_DIGICAM_QUALITY_HIGH,
                                          &error,
                                          NULL),
             "gdigicam-manager: an error has not happened.");
    fail_if (NULL == error,
             "gdigicam-manager: error was not set.");
    fail_if (!g_error_matches (error,
                               G_DIGICAM_ERROR,
                               G_DIGICAM_ERROR_GSTREAMER_BIN_NOT_SET),
             "gdigicam-manager: error is  \n\"%s\"\n instead of "
             "\"lock not supported\".",
             error->message);
    g_error_free (error);
    error = NULL;

    fail_if (g_digicam_manager_get_locks
             (no_featured_manager,
              &gotten_lock,
              &error),
             "gdigicam-manager: an error has not happened.");
    fail_if (0 != gotten_lock,
             "gdigicam-manager: the lock was not the \"unset\" "
             "value, it was \"%d\".",
             gotten_lock);
    fail_if (NULL == error,
             "gdigicam-manager: error was not set.");
    fail_if (!g_error_matches (error,
                               G_DIGICAM_ERROR,
                               G_DIGICAM_ERROR_GSTREAMER_BIN_NOT_SET),
             "gdigicam-manager: error is  \n\"%s\"\n instead of "
             "\"lock not supported\".",
             error->message);
    g_error_free (error);
    error = NULL;
    gotten_lock = 0;


    /* Test 2 */
    g_digicam_manager_set_gstreamer_bin (minimum_featured_manager,
                                         minimum_featured_camera_bin,
                                         minimum_featured_descriptor,
                                         NULL);
    g_digicam_manager_set_mode (minimum_featured_manager,
                                G_DIGICAM_MODE_STILL,
                                NULL,
                                NULL);
    fail_if (g_digicam_manager_set_locks (minimum_featured_manager,
                                          600,
                                          &error,
                                          NULL),
             "gdigicam-manager: an error has not happened.");
    fail_if (NULL == error,
             "gdigicam-manager: error was not set.");
    fail_if (!g_error_matches (error,
                               G_DIGICAM_ERROR,
		               G_DIGICAM_ERROR_FAILED),
             "gdigicam-manager: error is  \n\"%s\"\n instead of "
             "\"lock not supported\".",
             error->message);
    g_error_free (error);
    error = NULL;

    fail_if (!g_digicam_manager_get_locks
             (minimum_featured_manager,
              &gotten_lock,
              &error),
             "gdigicam-manager: an error has happened.");
    fail_if (600 == gotten_lock,
             "gdigicam-manager: the lock is the \"unset\" "
             "value, it is \"%d\".",
             gotten_lock);
    fail_if (NULL != error,
             "gdigicam-manager: error is set.");

    /* Test 3 */


    g_digicam_manager_set_gstreamer_bin (full_featured_manager,
                                         full_featured_camera_bin,
                                         full_featured_descriptor,
                                         NULL);
    g_digicam_manager_set_mode (full_featured_manager,
                                G_DIGICAM_MODE_STILL,
                                NULL,
                                NULL);
    fail_if (!g_digicam_manager_get_locks
             (full_featured_manager,
              &gotten_lock,
              &error),
             "gdigicam-manager: an error has happened.");
    fail_if (0 != gotten_lock,
             "gdigicam-manager: the lock was not the \"unset\" "
             "value, it was \"%d\".",
             gotten_lock);
    fail_if (NULL != error,
             "gdigicam-manager: error was set.");

    g_digicam_manager_set_focus_mode (full_featured_manager,
                                      G_DIGICAM_FOCUSMODE_MANUAL,
                                      TRUE,
                                      NULL,
                                      NULL);
    g_digicam_manager_set_exposure_mode (full_featured_manager,
                                         G_DIGICAM_EXPOSUREMODE_MANUAL,
                                         NULL,
                                         NULL,
                                         NULL);

    fail_if (g_digicam_manager_set_locks
             (full_featured_manager,
              G_DIGICAM_LOCK_AUTOFOCUS,
              &error,
	      NULL),
             "gdigicam-manager: an error has not happened.");
    fail_if (NULL == error,
             "gdigicam-manager: error was not set.");
    fail_if (!g_error_matches (error,
                               G_DIGICAM_ERROR,
		               G_DIGICAM_ERROR_LOCK_NOT_POSSIBLE),
             "gdigicam-manager: error is  \n\"%s\"\n instead of "
             "\"locks not supported\".",
             error->message);
    g_error_free (error);
    error = NULL;

}
END_TEST


/**
 * Purpose: test setting and getting locks in a #GDigicamManager
 * Cases considered:
 *    - set/get locks from a featured #GDigicamManager.
 */
START_TEST (test_set_get_locks_regular)
{
    GDigicamLock gotten_lock = 0;

    g_digicam_manager_set_gstreamer_bin (full_featured_manager,
                                         full_featured_camera_bin,
                                         full_featured_descriptor,
                                         NULL);
    g_digicam_manager_set_mode (full_featured_manager,
                                G_DIGICAM_MODE_STILL,
                                NULL,
                                NULL);
    g_digicam_manager_set_focus_mode (full_featured_manager,
                                      G_DIGICAM_FOCUSMODE_AUTO,
                                      TRUE,
                                      NULL,
                                      NULL);
    g_digicam_manager_set_exposure_mode (full_featured_manager,
                                         G_DIGICAM_EXPOSUREMODE_AUTO,
                                         NULL,
                                         NULL,
                                         NULL);
    /* Test 1 */
    fail_if (!g_digicam_manager_set_locks (full_featured_manager,
                                           G_DIGICAM_LOCK_AUTOFOCUS,
                                           &error,
                                           NULL),
             "gdigicam-manager: an error has happened.");
    fail_if (NULL != error,
             "gdigicam-manager: error was set.");
    g_error_free (error);
    error = NULL;

    fail_if (!g_digicam_manager_get_locks
             (full_featured_manager,
              &gotten_lock,
              &error),
             "gdigicam-manager: an error has happened.");
    fail_if (G_DIGICAM_LOCK_AUTOFOCUS != gotten_lock,
             "gdigicam-manager: the lock was not the \"unset\" "
             "value, it was \"%d\".",
             gotten_lock);
    fail_if (NULL != error,
             "gdigicam-manager: error was set.");
    g_error_free (error);
    error = NULL;
}
END_TEST


/**
 * Purpose: test setting and getting invalid locks in a
 * #GDigicamManager
 * Cases considered:
 *    - set/get locks from a NULL #GDigicamManager.
 */
START_TEST (test_set_get_locks_invalid)
{
    GDigicamLock gotten_lock = 0;

    /* Test 1 */
    fail_if (g_digicam_manager_set_locks (NULL,
                                         G_DIGICAM_FOCUSMODE_AUTO,
                                         &error,
                                         NULL),
             "gdigicam-manager: an error has not happened.");

    fail_if (g_digicam_manager_get_locks
             (NULL,
              &gotten_lock,
              &error),
             "gdigicam-manager: an error has not happened.");
    fail_if (0 != gotten_lock,
             "gdigicam-manager: the lock was the \"set\" "
             "value, it was \"%d\".",
             gotten_lock);
    gotten_lock = 0;
}
END_TEST



/* ----- Test case for set/get_audio -----*/


/**
 * Purpose: test setting and getting audio in a
 * #GDigicamManager
 * Cases considered:
 *    - set/get audio from just created #GDigicamManager.
 *    - set/get audio to a none featured #GDigicamManager.
 *    - set/get audio to a minimum featured #GDigicamManager.
 *    - get audio from a featured but not set #GDigicamManager.
 */
START_TEST (test_set_get_audio_limit)
{

    GDigicamAudio gotten_audio = 0;

    /* Test 1 */
    fail_if (g_digicam_manager_set_audio (no_featured_manager,
                                          G_DIGICAM_AUDIO_PLAYBACKON,
                                          &error,
                                          NULL),
             "gdigicam-manager: an error has not happened.");
    fail_if (NULL == error,
             "gdigicam-manager: error was not set.");
    fail_if (!g_error_matches (error,
                               G_DIGICAM_ERROR,
                               G_DIGICAM_ERROR_GSTREAMER_BIN_NOT_SET),
             "gdigicam-manager: error is  \n\"%s\"\n instead of "
             "\"audio not supported\".",
             error->message);
    g_error_free (error);
    error = NULL;

    fail_if (g_digicam_manager_get_audio
             (no_featured_manager,
              &gotten_audio,
              &error),
             "gdigicam-manager: an error has not happened.");
    fail_if (0 != gotten_audio,
             "gdigicam-manager: the audio was not the \"unset\" "
             "value, it was \"%d\".",
             gotten_audio);
    fail_if (NULL == error,
             "gdigicam-manager: error was not set.");
    fail_if (!g_error_matches (error,
                               G_DIGICAM_ERROR,
                               G_DIGICAM_ERROR_GSTREAMER_BIN_NOT_SET),
             "gdigicam-manager: error is  \n\"%s\"\n instead of "
             "\"audio not supported\".",
             error->message);
    g_error_free (error);
    error = NULL;
    gotten_audio = 0;


    /* Test 2 */
    g_digicam_manager_set_gstreamer_bin (minimum_featured_manager,
                                         minimum_featured_camera_bin,
                                         minimum_featured_descriptor,
                                         NULL);
    fail_if (g_digicam_manager_set_audio (minimum_featured_manager,
                                          G_DIGICAM_AUDIO_PLAYBACKOFF,
                                          &error,
                                          NULL),
             "gdigicam-manager: an error has not happened.");
    fail_if (NULL == error,
             "gdigicam-manager: error was not set.");
    fail_if (!g_error_matches (error,
                               G_DIGICAM_ERROR,
		               G_DIGICAM_ERROR_INVALID_MODE),
             "gdigicam-manager: error is  \n\"%s\"\n instead of "
             "\"audio not supported\".",
             error->message);
    g_error_free (error);
    error = NULL;

    fail_if (g_digicam_manager_get_audio
             (minimum_featured_manager,
              &gotten_audio,
              &error),
             "gdigicam-manager: an error has not happened.");
    fail_if (0 != gotten_audio,
             "gdigicam-manager: the audio is the \"unset\" "
             "value, it is \"%d\".",
             gotten_audio);
    fail_if (NULL == error,
             "gdigicam-manager: error is not set.");

    /* Test 3 */


    g_digicam_manager_set_gstreamer_bin (full_featured_manager,
                                         full_featured_camera_bin,
                                         full_featured_descriptor,
                                         NULL);
    fail_if (g_digicam_manager_get_audio
             (full_featured_manager,
              &gotten_audio,
              &error),
             "gdigicam-manager: an error has not happened.");
    fail_if (0 != gotten_audio,
             "gdigicam-manager: the audio was not the \"unset\" "
             "value, it was \"%d\".",
             gotten_audio);
    fail_if (NULL == error,
             "gdigicam-manager: error was not set.");
    if(error) {
        g_error_free(error);
        error = NULL;
    }

    g_digicam_manager_set_mode (full_featured_manager,
                                G_DIGICAM_MODE_VIDEO,
                                NULL,
                                NULL);
    gotten_audio = 6000;

    fail_if (g_digicam_manager_set_audio
             (full_featured_manager,
	      gotten_audio,
              &error,
	      NULL),
             "gdigicam-manager: an error has not happened.");
    fail_if (NULL == error,
             "gdigicam-manager: error was not set.");

}
END_TEST


/**
 * Purpose: test setting and getting audio in a
 * #GDigicamManager
 * Cases considered:
 *    - set/get audio from a featured #GDigicamManager.
 */
START_TEST (test_set_get_audio_regular)
{
    GDigicamAudio gotten_audio = 0;

    g_digicam_manager_set_gstreamer_bin (full_featured_manager,
                                         full_featured_camera_bin,
                                         full_featured_descriptor,
                                         NULL);
    g_digicam_manager_set_mode (full_featured_manager,
                                G_DIGICAM_MODE_VIDEO,
                                NULL,
                                NULL);
    /* Test 1 */
    fail_if (!g_digicam_manager_set_audio (full_featured_manager,
					   G_DIGICAM_AUDIO_PLAYBACKOFF,
                                           &error,
                                           NULL),
             "gdigicam-manager: an error has happened.");
    fail_if (NULL != error,
             "gdigicam-manager: error was set.");
    g_error_free (error);
    error = NULL;

    fail_if (!g_digicam_manager_get_audio
             (full_featured_manager,
              &gotten_audio,
              &error),
             "gdigicam-manager: an error has happened.");
    fail_if (G_DIGICAM_AUDIO_PLAYBACKOFF != gotten_audio,
             "gdigicam-manager: the audio was not the \"unset\" "
             "value, it was \"%d\".",
             gotten_audio);
    fail_if (NULL != error,
             "gdigicam-manager: error was set.");
    g_error_free (error);
    error = NULL;
}
END_TEST


/**
 * Purpose: test setting and getting invalid audio in a
 * #GDigicamManager
 * Cases considered:
 *    - set/get audio from a NULL #GDigicamManager.
 */
START_TEST (test_set_get_audio_invalid)
{
    GDigicamAudio gotten_audio = 0;

    /* Test 1 */
    fail_if (g_digicam_manager_set_audio (NULL,
					 G_DIGICAM_AUDIO_PLAYBACKON,
                                         &error,
                                         NULL),
             "gdigicam-manager: an error has not happened.");

    fail_if (g_digicam_manager_get_audio
             (NULL,
              &gotten_audio,
              &error),
             "gdigicam-manager: an error has not happened.");
    fail_if (0 != gotten_audio,
             "gdigicam-manager: the audio was the \"set\" "
             "value, it was \"%d\".",
             gotten_audio);
    gotten_audio = 0;
}
END_TEST



/* ----- Test case for set/get_preview_mode -----*/


/**
 * Purpose: test setting and getting preview mode in a
 * #GDigicamManager
 * Cases considered:
 *    - set/get preview mode from just created #GDigicamManager.
 *    - set/get preview mode to a none featured #GDigicamManager.
 *    - set/get preview mode to a minimum featured #GDigicamManager.
 *    - get preview mode from a featured but not set #GDigicamManager.
 */
START_TEST (test_set_preview_mode_limit)
{

    GDigicamPreview gotten_mode = G_DIGICAM_PREVIEW_NONE;

    /* Test 1 */
    fail_if (g_digicam_manager_set_preview_mode (no_featured_manager,
                                                 G_DIGICAM_PREVIEW_ON,
                                                 &error,
                                                 NULL),
             "gdigicam-manager: an error has not happened.");
    fail_if (NULL == error,
             "gdigicam-manager: error was not set.");
    fail_if (!g_error_matches (error,
                               G_DIGICAM_ERROR,
                               G_DIGICAM_ERROR_GSTREAMER_BIN_NOT_SET),
             "gdigicam-manager: error is  \n\"%s\"\n instead of "
             "\"bin not set\".",
             error->message);
    g_error_free (error);
    error = NULL;

    fail_if (g_digicam_manager_get_preview_mode (no_featured_manager,
                                                 &gotten_mode,
                                                 &error),
             "gdigicam-manager: an error has not happened.");
    fail_if (G_DIGICAM_PREVIEW_NONE != gotten_mode,
             "gdigicam-manager: the preview mode was not the \"unset\" "
             "value, it was \"%d\".",
             gotten_mode);
    fail_if (NULL == error,
             "gdigicam-manager: error was not set.");
    fail_if (!g_error_matches (error,
                               G_DIGICAM_ERROR,
                               G_DIGICAM_ERROR_GSTREAMER_BIN_NOT_SET),
             "gdigicam-manager: error is  \n\"%s\"\n instead of "
             "\"bin not set\".",
             error->message);
    g_error_free (error);
    error = NULL;
    gotten_mode = G_DIGICAM_PREVIEW_NONE;


    /* Test 2 */
    g_digicam_manager_set_gstreamer_bin (no_featured_manager,
                                         no_featured_camera_bin,
                                         no_featured_descriptor,
                                         NULL);
    fail_if (g_digicam_manager_set_preview_mode (no_featured_manager,
                                                 G_DIGICAM_PREVIEW_ON,
                                                 &error,
                                                 NULL),
             "gdigicam-manager: an error has not happened.");
    fail_if (NULL == error,
             "gdigicam-manager: error was not set.");
    fail_if (!g_error_matches (error,
                               G_DIGICAM_ERROR,
		               G_DIGICAM_ERROR_PREVIEW_NOT_SUPPORTED),
             "gdigicam-manager: error is  \n\"%s\"\n instead of "
             "\"preview not supported\".",
             error->message);
    g_error_free (error);
    error = NULL;

    fail_if (g_digicam_manager_get_preview_mode (no_featured_manager,
                                                 &gotten_mode,
                                                 &error),
             "gdigicam-manager: an error has not happened.");
    fail_if (G_DIGICAM_PREVIEW_NONE != gotten_mode,
             "gdigicam-manager: the preview mode was not the \"unset\" "
             "value, it is \"%d\".",
             gotten_mode);
    fail_if (NULL == error,
             "gdigicam-manager: error was not set.");
    fail_if (!g_error_matches (error,
                               G_DIGICAM_ERROR,
		               G_DIGICAM_ERROR_PREVIEW_NOT_SUPPORTED),
             "gdigicam-manager: error is  \n\"%s\"\n instead of "
             "\"preview not supported\".",
             error->message);
    g_error_free (error);
    error = NULL;
    gotten_mode = G_DIGICAM_PREVIEW_NONE;

    /* Test 3 */
    g_digicam_manager_set_gstreamer_bin (minimum_featured_manager,
                                         minimum_featured_camera_bin,
                                         minimum_featured_descriptor,
                                         NULL);
    fail_if (g_digicam_manager_set_preview_mode (minimum_featured_manager,
                                                 G_DIGICAM_PREVIEW_ON,
                                                 &error,
                                                 NULL),
             "gdigicam-manager: an error has not happened.");
    fail_if (NULL == error,
             "gdigicam-manager: error was not set.");
    fail_if (!g_error_matches (error,
                               G_DIGICAM_ERROR,
		               G_DIGICAM_ERROR_PREVIEW_NOT_SUPPORTED),
             "gdigicam-manager: error is  \n\"%s\"\n instead of "
             "\"preview not supported\".",
             error->message);
    g_error_free (error);
    error = NULL;

    fail_if (g_digicam_manager_get_preview_mode (minimum_featured_manager,
                                                 &gotten_mode,
                                                 &error),
             "gdigicam-manager: an error has not happened.");
    fail_if (G_DIGICAM_PREVIEW_NONE != gotten_mode,
             "gdigicam-manager: the preview mode was not the \"unset\" "
             "value, it is \"%d\".",
             gotten_mode);
    fail_if (NULL == error,
             "gdigicam-manager: error was not set.");
    fail_if (!g_error_matches (error,
                               G_DIGICAM_ERROR,
		               G_DIGICAM_ERROR_PREVIEW_NOT_SUPPORTED),
             "gdigicam-manager: error is  \n\"%s\"\n instead of "
             "\"preview not supported\".",
             error->message);
    g_error_free (error);
    error = NULL;
    gotten_mode = G_DIGICAM_PREVIEW_NONE;

    /* Test 4 */
    g_digicam_manager_set_gstreamer_bin (full_featured_manager,
                                         full_featured_camera_bin,
                                         full_featured_descriptor,
                                         NULL);
    fail_if (!g_digicam_manager_get_preview_mode (full_featured_manager,
                                                  &gotten_mode,
                                                  &error),
             "gdigicam-manager: an error has happened.");
    fail_if (G_DIGICAM_PREVIEW_NONE != gotten_mode,
             "gdigicam-manager: the preview mode was not the \"unset\" "
             "value, it was \"%d\".",
             gotten_mode);
    fail_if (NULL != error,
             "gdigicam-manager: error was set.");
    if (error != NULL) g_error_free (error);
    error = NULL;
}
END_TEST


/**
 * Purpose: test setting and getting preview mode in a
 * #GDigicamManager
 * Cases considered:
 *    - set/get #G_DIGICAM_PREVIEW_ON mode from a featured #GDigicamManager.
 *    - set/get #G_DIGICAM_PREVIEW_OFF mode from a featured #GDigicamManager.
 */
START_TEST (test_set_preview_mode_regular)
{
    GDigicamPreview gotten_mode = G_DIGICAM_PREVIEW_NONE;

    g_digicam_manager_set_gstreamer_bin (full_featured_manager,
                                         full_featured_camera_bin,
                                         full_featured_descriptor,
                                         NULL);
    g_digicam_manager_set_mode (full_featured_manager,
                                G_DIGICAM_MODE_STILL,
                                NULL,
                                NULL);
    /* Test 1 */
    fail_if (!g_digicam_manager_set_preview_mode (full_featured_manager,
                                                  G_DIGICAM_PREVIEW_ON,
                                                  &error,
                                                  NULL),
             "gdigicam-manager: an error has happened.");
    fail_if (NULL != error,
             "gdigicam-manager: error was set.");
    if (error != NULL) g_error_free (error);
    error = NULL;

    fail_if (!g_digicam_manager_get_preview_mode (full_featured_manager,
                                                  &gotten_mode,
                                                  &error),
             "gdigicam-manager: an error has happened.");
    fail_if (G_DIGICAM_PREVIEW_ON != gotten_mode,
             "gdigicam-manager: the preview mode was not the \"set\" "
             "value \"%d\", it was \"%d\".",
             G_DIGICAM_PREVIEW_ON,
             gotten_mode);
    fail_if (NULL != error,
             "gdigicam-manager: error was set.");
    if (error != NULL) g_error_free (error);
    error = NULL;
    gotten_mode = G_DIGICAM_PREVIEW_NONE;

    /* Test 2 */
    fail_if (!g_digicam_manager_set_preview_mode (full_featured_manager,
                                                  G_DIGICAM_PREVIEW_OFF,
                                                  &error,
                                                  NULL),
             "gdigicam-manager: an error has happened.");
    fail_if (NULL != error,
             "gdigicam-manager: error was set.");
    if (error != NULL) g_error_free (error);
    error = NULL;

    fail_if (!g_digicam_manager_get_preview_mode (full_featured_manager,
                                                  &gotten_mode,
                                                  &error),
             "gdigicam-manager: an error has happened.");
    fail_if (G_DIGICAM_PREVIEW_OFF != gotten_mode,
             "gdigicam-manager: the preview mode was not the \"set\" "
             "value \"%d\", it was \"%d\".",
             G_DIGICAM_PREVIEW_OFF,
             gotten_mode);
    fail_if (NULL != error,
             "gdigicam-manager: error was set.");
    if (error != NULL) g_error_free (error);
    error = NULL;
}
END_TEST


/**
 * Purpose: test setting and getting invalid values in a
 * #GDigicamManager
 * Cases considered:
 *    - set/get preview mode from a NULL #GDigicamManager.
 */
START_TEST (test_set_preview_mode_invalid)
{
    GDigicamPreview gotten_mode = G_DIGICAM_PREVIEW_NONE;

    /* Test 1 */
    fail_if (g_digicam_manager_set_preview_mode (NULL,
                                                 G_DIGICAM_PREVIEW_ON,
                                                 &error,
                                                 NULL),
             "gdigicam-manager: an error has not happened.");

    fail_if (g_digicam_manager_get_preview_mode (NULL,
                                                 &gotten_mode,
                                                 &error),
             "gdigicam-manager: an error has not happened.");
    fail_if (G_DIGICAM_PREVIEW_NONE != gotten_mode,
             "gdigicam-manager: the audio was the \"unset\" "
             "value, it was \"%d\".",
             gotten_mode);
    gotten_mode = 0;
}
END_TEST



/* ----- Test case for preview_enabled -----*/


/**
 * Purpose: test preview_enabled method in a #GDigicamManager.
 * Cases considered:
 *    - using a just created #GDigicamManager.
 *    - using a none featured #GDigicamManager.
 *    - using a minimum featured #GDigicamManager.
 *    - using a featured but not set #GDigicamManager.
 */
START_TEST (test_preview_enabled_limit)
{

    gboolean enabled = FALSE;

    /* Test 1 */
    fail_if (g_digicam_manager_preview_enabled (no_featured_manager,
                                                &enabled,
                                                &error),
             "gdigicam-manager: an error has not happened.");
    fail_if (NULL == error,
             "gdigicam-manager: error was not set.");
    fail_if (enabled,
             "gdigicam-manager: the preview mode was not the \"unset\" "
             "value, it was \"%d\".",
             enabled);
    fail_if (!g_error_matches (error,
                               G_DIGICAM_ERROR,
                               G_DIGICAM_ERROR_GSTREAMER_BIN_NOT_SET),
             "gdigicam-manager: error is  \n\"%s\"\n instead of "
             "\"bin not set\".",
             error->message);
    g_error_free (error);
    error = NULL;
    enabled = FALSE;

    /* Test 2 */
    g_digicam_manager_set_gstreamer_bin (no_featured_manager,
                                         no_featured_camera_bin,
                                         no_featured_descriptor,
                                         NULL);
    fail_if (g_digicam_manager_preview_enabled (no_featured_manager,
                                                &enabled,
                                                &error),
             "gdigicam-manager: an error has not happened.");
    fail_if (NULL == error,
             "gdigicam-manager: error was not set.");
    fail_if (enabled,
             "gdigicam-manager: the preview mode was not the \"unset\" "
             "value, it was \"%d\".",
             enabled);
    fail_if (!g_error_matches (error,
                               G_DIGICAM_ERROR,
		               G_DIGICAM_ERROR_PREVIEW_NOT_SUPPORTED),
             "gdigicam-manager: error is  \n\"%s\"\n instead of "
             "\"preview not supported\".",
             error->message);
    g_error_free (error);
    error = NULL;
    enabled = FALSE;

    /* Test 3 */
    g_digicam_manager_set_gstreamer_bin (minimum_featured_manager,
                                         minimum_featured_camera_bin,
                                         minimum_featured_descriptor,
                                         NULL);
    fail_if (g_digicam_manager_preview_enabled (minimum_featured_manager,
                                                &enabled,
                                                &error),
             "gdigicam-manager: an error has not happened.");
    fail_if (NULL == error,
             "gdigicam-manager: error was not set.");
    fail_if (enabled,
             "gdigicam-manager: the preview mode was not the \"unset\" "
             "value, it was \"%d\".",
             enabled);
    fail_if (!g_error_matches (error,
                               G_DIGICAM_ERROR,
		               G_DIGICAM_ERROR_PREVIEW_NOT_SUPPORTED),
             "gdigicam-manager: error is  \n\"%s\"\n instead of "
             "\"preview not supported\".",
             error->message);
    g_error_free (error);
    error = NULL;
    enabled = FALSE;

    /* Test 4 */
    g_digicam_manager_set_gstreamer_bin (full_featured_manager,
                                         full_featured_camera_bin,
                                         full_featured_descriptor,
                                         NULL);
    fail_if (!g_digicam_manager_preview_enabled (full_featured_manager,
                                                 &enabled,
                                                 &error),
             "gdigicam-manager: an error has happened.");
    fail_if (enabled,
             "gdigicam-manager: the preview mode was not the \"unset\" "
             "value, it was \"%d\".",
             enabled);
    fail_if (NULL != error,
             "gdigicam-manager: error was set.");
    if (error != NULL) g_error_free (error);
    error = NULL;
}
END_TEST


/**
 * Purpose: test preview_enabled method in a #GDigicamManager.
 * Cases considered:
 *    - set/get #G_DIGICAM_PREVIEW_ON mode from a featured #GDigicamManager.
 *    - set/get #G_DIGICAM_PREVIEW_OFF mode from a featured #GDigicamManager.
 */
START_TEST (test_preview_enabled_regular)
{
    gboolean enabled = FALSE;

    g_digicam_manager_set_gstreamer_bin (full_featured_manager,
                                         full_featured_camera_bin,
                                         full_featured_descriptor,
                                         NULL);
    g_digicam_manager_set_mode (full_featured_manager,
                                G_DIGICAM_MODE_STILL,
                                NULL,
                                NULL);
    /* Test 1 */
    fail_if (!g_digicam_manager_set_preview_mode (full_featured_manager,
                                                  G_DIGICAM_PREVIEW_ON,
                                                  &error,
                                                  NULL),
             "gdigicam-manager: an error has happened.");
    fail_if (NULL != error,
             "gdigicam-manager: error was set.");
    if (error != NULL) g_error_free (error);
    error = NULL;

    fail_if (!g_digicam_manager_preview_enabled (full_featured_manager,
                                                 &enabled,
                                                 &error),
             "gdigicam-manager: an error has happened.");
    fail_if (!enabled,
             "gdigicam-manager: the preview was enabled and "
             "it shouldn't.");
    fail_if (NULL != error,
             "gdigicam-manager: error was set.");
    if (error != NULL) g_error_free (error);
    error = NULL;
    enabled = FALSE;

    /* Test 2 */
    fail_if (!g_digicam_manager_set_preview_mode (full_featured_manager,
                                                  G_DIGICAM_PREVIEW_OFF,
                                                  &error,
                                                  NULL),
             "gdigicam-manager: an error has happened.");
    fail_if (NULL != error,
             "gdigicam-manager: error was set.");
    if (error != NULL) g_error_free (error);
    error = NULL;

    fail_if (!g_digicam_manager_preview_enabled (full_featured_manager,
                                                 &enabled,
                                                 &error),
             "gdigicam-manager: an error has happened.");
    fail_if (enabled,
             "gdigicam-manager: the preview was enabled and "
             "it shouldn't.");
    fail_if (NULL != error,
             "gdigicam-manager: error was set.");
    if (error != NULL) g_error_free (error);
    error = NULL;
}
END_TEST


/**
 * Purpose: test preview_enabled with invalid values in a
 * #GDigicamManager
 * Cases considered:
 *    - set preview mode from a NULL #GDigicamManager.
 *    - set and invalid preview mode in #GDigicamManager.
 */
START_TEST (test_preview_enabled_invalid)
{
    gboolean enabled = FALSE;

    /* Test 1 */
    fail_if (g_digicam_manager_set_preview_mode (NULL,
                                                 G_DIGICAM_PREVIEW_ON,
                                                 &error,
                                                 NULL),
             "gdigicam-manager: an error has not happened.");

    fail_if (g_digicam_manager_preview_enabled (NULL,
                                                &enabled,
                                                &error),
             "gdigicam-manager: an error has not happened.");
    fail_if (enabled,
             "gdigicam-manager: the preview was enabled and "
             "it shouldn't.");
    if (error != NULL) g_error_free (error);
    error = NULL;
    enabled = FALSE;

    /* Test 2 */
    g_digicam_manager_set_gstreamer_bin (full_featured_manager,
                                         full_featured_camera_bin,
                                         full_featured_descriptor,
                                         NULL);
    g_digicam_manager_set_mode (full_featured_manager,
                                G_DIGICAM_MODE_STILL,
                                NULL,
                                NULL);
    fail_if (g_digicam_manager_set_preview_mode (full_featured_manager,
                                                 G_DIGICAM_PREVIEW_NONE,
                                                 &error,
                                                 NULL),
             "gdigicam-manager: an error has not happened.");
    fail_if (NULL == error,
             "gdigicam-manager: error was not set.");
    if (error != NULL) g_error_free (error);
    error = NULL;
    enabled = FALSE;

    fail_if (!g_digicam_manager_preview_enabled (full_featured_manager,
                                                &enabled,
                                                &error),
             "gdigicam-manager: an error has happened.");
    fail_if (enabled,
             "gdigicam-manager: the preview was enabled and "
             "it shouldn't.");
}
END_TEST



/* ---------- Suite creation ---------- */

Suite *create_g_digicam_manager_suite (void)
{
    /* Create the suite */
    Suite *s = suite_create ("GDigicamManager");

    /* Create test cases */
    TCase *tc1 = tcase_create ("new");
    TCase *tc2 = tcase_create ("set_get_gstreamer_bin");
    TCase *tc6 = tcase_create ("test_set_get_mode");
    TCase *tc7 = tcase_create ("test_set_get_flash_mode");
    TCase *tc8 = tcase_create ("test_set_get_exposure_mode");
    TCase *tc9 = tcase_create ("test_set_get_metering_mode");
    TCase *tc10 = tcase_create ("test_set_get_aspect_ratio");
    TCase *tc11 = tcase_create ("test_set_get_resolution");
    TCase *tc12 = tcase_create ("test_set_get_zoom");
    TCase *tc13 = tcase_create ("test_video_recording");
    TCase *tc14 = tcase_create ("test_set_get_focus_mode");
    TCase *tc15 = tcase_create ("test_set_get_focus_region_pattern");
    TCase *tc16 = tcase_create ("test_capture_still_picture");
    TCase *tc17 = tcase_create ("test_set_get_white_balance_mode");
    TCase *tc18 = tcase_create ("test_set_get_iso_sensitivity_mode");
    TCase *tc19 = tcase_create ("test_start_stop_viewfinder");
    TCase *tc20 = tcase_create ("test_set_get_exposure_comp_mode");
    TCase *tc21 = tcase_create ("test_set_get_quality");
    TCase *tc22 = tcase_create ("test_set_get_locks");
    TCase *tc23 = tcase_create ("test_set_get_audio");
    TCase *tc24 = tcase_create ("test_query_capabilites");
    TCase *tc25 = tcase_create ("test_set_aspect_ratio_resolution");
    TCase *tc26 = tcase_create ("test_set_preview_mode");
    TCase *tc27 = tcase_create ("test_preview_enabled");

    /* Create test case for new and add it to the suite */
    tcase_add_checked_fixture (tc1, fx_setup_g_digicam, NULL);
    tcase_add_test (tc1, test_g_digicam_manager_new);
    suite_add_tcase (s, tc1);

    /* Create test case for set_get_gstreamer_bin and add it to the suite */
    tcase_add_checked_fixture (tc2,
                               fx_setup_default_managers,
                               fx_teardown_default_managers);
    tcase_add_test (tc2, test_set_get_gstreamer_bin_limit);
    tcase_add_test (tc2, test_set_get_gstreamer_bin_regular);
    tcase_add_test (tc2, test_set_get_gstreamer_bin_invalid);
    suite_add_tcase (s, tc2);

    /* Create test case for test_set_get_mode add it to the suite */
    tcase_add_checked_fixture (tc6,
                               fx_setup_default_managers,
                               fx_teardown_default_managers);
    tcase_add_test (tc6, test_set_get_mode_limit);
    tcase_add_test (tc6, test_set_get_mode_regular);
    tcase_add_test (tc6, test_set_get_mode_invalid);
    suite_add_tcase (s, tc6);

    /* Create test case for test_set_get_flash_mode and add it to the suite */
    tcase_add_checked_fixture (tc7,
                               fx_setup_default_managers,
                               fx_teardown_default_managers);
    tcase_add_test (tc7, test_set_get_flash_mode_limit);
    tcase_add_test (tc7, test_set_get_flash_mode_regular);
    tcase_add_test (tc7, test_set_get_flash_mode_invalid);
    suite_add_tcase (s, tc7);

    /* Create test case for test_set_get_exposure_mode and add it to the suite */
    tcase_add_checked_fixture (tc8,
                               fx_setup_default_managers,
                               fx_teardown_default_managers);
    tcase_add_test (tc8, test_set_get_exposure_mode_limit);
    tcase_add_test (tc8, test_set_get_exposure_mode_regular);
    tcase_add_test (tc8, test_set_get_exposure_mode_invalid);
    suite_add_tcase (s, tc8);

    /* Create test case for test_set_get_metering_mode and add it to the suite */
    tcase_add_checked_fixture (tc9,
                               fx_setup_default_managers,
                               fx_teardown_default_managers);
    tcase_add_test (tc9, test_set_get_metering_mode_limit);
    tcase_add_test (tc9, test_set_get_metering_mode_regular);
    tcase_add_test (tc9, test_set_get_metering_mode_invalid);
    suite_add_tcase (s, tc9);

    /* Create test case for test_set_get_aspect_ratio and add it to the suite */
    tcase_add_checked_fixture (tc10,
                               fx_setup_default_managers,
                               fx_teardown_default_managers);
    tcase_add_test (tc10, test_set_get_aspect_ratio_regular);
    tcase_add_test (tc10, test_set_get_aspect_ratio_invalid);
    suite_add_tcase (s, tc10);

    /* Create test case for test_set_get_resolution and add it to the suite */
    tcase_add_checked_fixture (tc11,
                               fx_setup_default_managers,
                               fx_teardown_default_managers);
    tcase_add_test (tc11, test_set_get_resolution_limit);
    tcase_add_test (tc11, test_set_get_resolution_regular);
    tcase_add_test (tc11, test_set_get_resolution_invalid);
    suite_add_tcase (s, tc11);

    /* Create test case for test_set_get_zoom and add it to the suite */
    tcase_add_checked_fixture (tc12,
                               fx_setup_default_managers,
                               fx_teardown_default_managers);
    tcase_add_test (tc12, test_set_get_zoom_limit);
    tcase_add_test (tc12, test_set_get_zoom_regular);
    tcase_add_test (tc12, test_set_get_zoom_invalid);
    suite_add_tcase (s, tc12);

    /* Create test case for test_video_recording and add it to the suite */
    tcase_add_checked_fixture(tc13, fx_setup_default_managers, fx_teardown_default_managers);
    tcase_add_test(tc13, test_video_recording_regular);
    tcase_add_test(tc13, test_video_recording_invalid);
    suite_add_tcase (s, tc13);

    /* Create test case for test_set_get_focus_mode and add it to the suite */
    tcase_add_checked_fixture(tc14, fx_setup_default_managers, fx_teardown_default_managers);
    tcase_add_test(tc14, test_set_get_focus_mode_regular);
    tcase_add_test(tc14, test_set_get_focus_mode_invalid);
    suite_add_tcase (s, tc14);

    /* Create test case for test_set_get_focus_region_pattern and add it to the suite */
    tcase_add_checked_fixture(tc15, fx_setup_default_managers, fx_teardown_default_managers);
    tcase_add_test(tc15, test_set_get_focus_region_pattern_regular);
    tcase_add_test(tc15, test_set_get_focus_region_pattern_invalid);
    suite_add_tcase (s, tc15);

    /* Create test case for test_capture_still_picture and add it to the suite */
    tcase_add_checked_fixture(tc16, fx_setup_default_managers, fx_teardown_default_managers);
    tcase_add_test(tc16, test_capture_still_picture_regular);
    tcase_add_test(tc16, test_capture_still_picture_invalid);
    suite_add_tcase (s, tc16);

    /* Create test case for test_set_get_white_balance_mode and add it to the suite */
    tcase_add_checked_fixture (tc17,
                               fx_setup_default_managers,
                               fx_teardown_default_managers);
    tcase_add_test (tc17, test_set_get_white_balance_mode_limit);
    tcase_add_test (tc17, test_set_get_white_balance_mode_regular);
    tcase_add_test (tc17, test_set_get_white_balance_mode_invalid);
    suite_add_tcase (s, tc17);

    /* Create test case for test_set_get_white_balance_mode and add it to the suite */
    tcase_add_checked_fixture (tc18,
                               fx_setup_default_managers,
                               fx_teardown_default_managers);
    tcase_add_test (tc18, test_set_get_iso_sensitivity_mode_limit);
    tcase_add_test (tc18, test_set_get_iso_sensitivity_mode_regular);
    tcase_add_test (tc18, test_set_get_iso_sensitivity_mode_invalid);
    suite_add_tcase (s, tc18);

    /* Create test case for test_start_stop_viewfinder and add it to the suite */
    tcase_add_checked_fixture (tc19,
                               fx_setup_default_managers,
                               fx_teardown_default_managers);
    tcase_add_test (tc19, test_start_stop_viewfinder_regular);
    tcase_add_test (tc19, test_start_stop_viewfinder_invalid);
    suite_add_tcase (s, tc19);


    /* Create test case for test_set_get_exposure_comp and add it to the suite */
    tcase_add_checked_fixture (tc20,
                               fx_setup_default_managers,
                               fx_teardown_default_managers);
    tcase_add_test (tc20, test_set_get_exposure_comp_limit);
    tcase_add_test (tc20, test_set_get_exposure_comp_regular);
    tcase_add_test (tc20, test_set_get_exposure_comp_invalid);
    suite_add_tcase (s, tc20);

    /* Create test case for test_set_get_quality and add it to the suite */
    tcase_add_checked_fixture (tc21,
                               fx_setup_default_managers,
                               fx_teardown_default_managers);
    tcase_add_test (tc21, test_set_get_quality_limit);
    tcase_add_test (tc21, test_set_get_quality_regular);
    tcase_add_test (tc21, test_set_get_quality_invalid);
    suite_add_tcase (s, tc21);

    /* Create test case for test_set_get_locks and add it to the suite */
    tcase_add_checked_fixture (tc22,
                               fx_setup_default_managers,
                               fx_teardown_default_managers);
    tcase_add_test (tc22, test_set_get_locks_limit);
    tcase_add_test (tc22, test_set_get_locks_regular);
    tcase_add_test (tc22, test_set_get_locks_invalid);
    suite_add_tcase (s, tc22);

    /* Create test case for test_set_get_audio and add it to the suite */
    tcase_add_checked_fixture (tc23,
                               fx_setup_default_managers,
                               fx_teardown_default_managers);
    tcase_add_test (tc23, test_set_get_audio_limit);
    tcase_add_test (tc23, test_set_get_audio_regular);
    tcase_add_test (tc23, test_set_get_audio_invalid);
    suite_add_tcase (s, tc23);

    /* Create test case for test_query_capabilities and add it to the suite */
    tcase_add_checked_fixture (tc24,
                               fx_setup_default_managers,
                               fx_teardown_default_managers);
    tcase_add_test (tc24, test_query_capabilities_limit);
    suite_add_tcase (s, tc24);

    /* Create test case for test_set_aspect_ratio_resolution and add it to the suite */
    tcase_add_checked_fixture (tc25,
                               fx_setup_default_managers,
                               fx_teardown_default_managers);
    tcase_add_test (tc25, test_set_aspect_ratio_resolution_limit);
    tcase_add_test (tc25, test_set_aspect_ratio_resolution_regular);
    tcase_add_test (tc25, test_set_aspect_ratio_resolution_invalid);
    suite_add_tcase (s, tc25);

    /* Create test case for test_set_preview_mode and add it to the suite */
    tcase_add_checked_fixture (tc26,
                               fx_setup_default_managers,
                               fx_teardown_default_managers);
    tcase_add_test (tc26, test_set_preview_mode_limit);
    tcase_add_test (tc26, test_set_preview_mode_regular);
    tcase_add_test (tc26, test_set_preview_mode_invalid);
    suite_add_tcase (s, tc26);

    /* Create test case for test_preview_enabled and add it to the suite */
    tcase_add_checked_fixture (tc27,
                               fx_setup_default_managers,
                               fx_teardown_default_managers);
    tcase_add_test (tc27, test_preview_enabled_limit);
    tcase_add_test (tc27, test_preview_enabled_regular);
    tcase_add_test (tc27, test_preview_enabled_invalid);
    suite_add_tcase (s, tc27);

    /* Return created suite */
    return s;
}
