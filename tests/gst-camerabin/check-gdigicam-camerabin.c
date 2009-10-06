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

#include <check.h>

#include "check-utils.h"

#include "test_suites.h"
#include "gdigicam-util.h"
#include "gdigicam-camerabin.h"

static GstElement *minimum_camera_bin = NULL;
static GstElement *simple_camerabin = NULL;
static GstElement *full_camerabin = NULL;
static GDigicamDescriptor *descriptor = NULL;

/* -------------------- Fixtures -------------------- */

static void
fx_setup_g_digicam_camerabin (void)
{
    int argc = 0;
    char **argv = NULL;

    /* Initialize the GDigicam */
    g_digicam_init (&argc, &argv);
}

static void
fx_setup_default_gst_camerabins (void)
{
    fx_setup_g_digicam_camerabin ();

    minimum_camera_bin = create_basic_gstreamer_element ("minimum featured camera bin");

    simple_camerabin = g_digicam_camerabin_element_new ("videotestsrc",
                                                        NULL,
                                                        NULL,
                                                        NULL,
                                                        NULL,
							NULL,
                                                        NULL,
                                                        VIDEO_SINK,
                                                        NULL);

    full_camerabin = g_digicam_camerabin_element_new ("v4l2camsrc",
                                                      "jpegenc",
                                                      "avimux",
                                                      "pulsesrc",
                                                      "mulawenc",
                                                      "jpegenc",
						      "ipp",
                                                      VIDEO_SINK,
                                                      NULL);
}

static void
fx_teardown_default_camerabins (void)
{
    if (NULL != minimum_camera_bin) {
        gst_object_unref (GST_OBJECT (minimum_camera_bin));
        minimum_camera_bin = NULL;
    }

    if (NULL != simple_camerabin) {
        gst_object_unref (GST_OBJECT (simple_camerabin));
        simple_camerabin = NULL;
    }

    if (NULL != full_camerabin) {
        gst_object_unref (GST_OBJECT (full_camerabin));
        full_camerabin = NULL;
    }
}

/* -------------------- Test cases -------------------- */

/* ----- Test case for element_new -----*/

/**
 * Purpose: test the creation of a #GstElement CameraBin in the border line cases.
 * Cases considered:
 *    - Create a  #GstElement CameraBin with non existent elements.
 */
START_TEST (test_g_digicam_camerabin_element_new_limit)
{
    /* Case 1 */
    simple_camerabin = g_digicam_camerabin_element_new (TEST_STRING,
                                                        TEST_STRING,
                                                        TEST_STRING,
                                                        TEST_STRING,
                                                        TEST_STRING,
                                                        TEST_STRING,
                                                        TEST_STRING,
							TEST_STRING,
                                                        NULL);

    /* Check that the GstElement object has been created properly */
    fail_if (!GST_IS_ELEMENT (simple_camerabin),
             "g-digicam-camerabin: the returned camerabin"
             "is not a valid GstElement.");

    gst_object_unref (GST_OBJECT (simple_camerabin));
    simple_camerabin = NULL;
}
END_TEST


/**
 * Purpose: test the creation of a #GstElement CameraBin in the regular cases.
 * Cases considered:
 *    - Create a  #GstElement CameraBin with regular elements.
 */
START_TEST (test_g_digicam_camerabin_element_new_regular)
{
    /* Case 1 */
    simple_camerabin = g_digicam_camerabin_element_new ("videotestsrc",
                                                        NULL,
                                                        NULL,
                                                        NULL,
                                                        NULL,
                                                        NULL,
							NULL,
                                                        VIDEO_SINK,
                                                        NULL);

    /* Check that the GstElement object has been created properly */
    fail_if (!GST_IS_ELEMENT (simple_camerabin),
             "g-digicam-camerabin: the returned camerabin"
             "is not a valid GstElement.");

    gst_object_unref (GST_OBJECT (simple_camerabin));
    simple_camerabin = NULL;
}
END_TEST

/* ----- Test case for descriptor_new -----*/

/**
 * Purpose: test the customized #GDigicamDescriptor for the GStreamer
 * CameraBin plugin with invalid CameraBin #GstElement.
 * Cases considered:
 *    - Create a new customized #GDigicamDescriptor with a wrong object.
 *    - Create a new customized #GDigicamDescriptor with a NULL pointer.
 */
START_TEST (test_g_digicam_camerabin_descriptor_new_invalid)
{
    GstPad *gst_pad = NULL;

    /* Case 1 */
    gst_pad = gst_pad_new (NULL, GST_PAD_UNKNOWN);
    descriptor = g_digicam_camerabin_descriptor_new (GST_ELEMENT (gst_pad));

    /* Check that the descriptor object has been created properly */
    fail_if (NULL != descriptor,
             "g-digicam-camerabin: the returned descriptor"
             "is not NULL.");

    gst_object_unref (GST_OBJECT (gst_pad));


    /* Case 2 */
    descriptor = g_digicam_camerabin_descriptor_new (NULL);

    /* Check that the descriptor object has been created properly */
    fail_if (NULL != descriptor,
             "g-digicam-camerabin: the returned descriptor"
             "is not NULL.");
}
END_TEST

/**
 * Purpose: test the customized #GDigicamDescriptor for the GStreamer
 * CameraBin plugin with limit CameraBin #GstElement.
 * Cases considered:
 *    - Create a new customized #GDigicamDescriptor with a minimum,
 *      not-really, camera bin #GstElement.
 */
START_TEST (test_g_digicam_camerabin_descriptor_new_limit)
{
    GstElement *tmp_element = NULL;

    /* Case 1 */
    descriptor = g_digicam_camerabin_descriptor_new (minimum_camera_bin);

    /* Check that the descriptor has been created properly */
    fail_if (NULL == descriptor,
             "g-digicam-camerabin: the returned descriptor"
             "is NULL.");

    g_object_get (G_OBJECT (minimum_camera_bin), "vfsink", &tmp_element, NULL);

    /* Check that the descriptor has the same sink than the passed element */
    fail_if (tmp_element != descriptor->viewfinder_sink,
             "g-digicam-camerabin: the descriptor has a different sink than the "
             "passed element.");

    tmp_element = NULL;
    g_digicam_manager_descriptor_free (descriptor);
    descriptor = NULL;
}
END_TEST

/**
 * Purpose: test the customized #GDigicamDescriptor for the GStreamer
 * CameraBin plugin with regular CameraBin #GstElement.
 * Cases considered:
 *    - Create a new customized #GDigicamDescriptor with a simple camerabin.
 *    - Create a new customized #GDigicamDescriptor with a full camerabin.
 */
START_TEST (test_g_digicam_camerabin_descriptor_new_regular)
{
    GstElement *tmp_element = NULL;

    /* Case 1 */
    descriptor = g_digicam_camerabin_descriptor_new (simple_camerabin);

    /* Check that the descriptor has been created properly */
    fail_if (NULL == descriptor,
             "g-digicam-camerabin: the returned descriptor"
             "is NULL.");

    g_object_get (G_OBJECT (simple_camerabin), "vfsink", &tmp_element, NULL);

    /* Check that the descriptor has the same sink than the passed element */
    fail_if (tmp_element != descriptor->viewfinder_sink,
             "g-digicam-camerabin: the descriptor has a different sink than the "
             "passed element.");

    tmp_element = NULL;
    g_digicam_manager_descriptor_free (descriptor);
    descriptor = NULL;


    /* Case 2 */
    descriptor = g_digicam_camerabin_descriptor_new (full_camerabin);

    /* Check that the descriptor object has been created properly */
    fail_if (NULL == descriptor,
             "g-digicam-camerabin: the returned descriptor"
             "is NULL.");

    g_object_get (G_OBJECT (full_camerabin), "vfsink", &tmp_element, NULL);

    /* Check that the descriptor has the same sink than the passed element */
    fail_if (tmp_element != descriptor->viewfinder_sink,
             "g-digicam-camerabin: the descriptor has a different sink than the "
             "passed element.");

    tmp_element = NULL;
    g_digicam_manager_descriptor_free (descriptor);
    descriptor = NULL;
}
END_TEST

/* ---------- Suite creation ---------- */

Suite *create_g_digicam_camerabin_suite (void)
{
    /* Create the suite */
    Suite *s = suite_create ("GDigicamCamerabin");

    /* Create test cases */
    TCase *tc1 = tcase_create ("new");
    TCase *tc2 = tcase_create ("new");

    /* Create test case for element_new and add it to the suite */
    tcase_add_checked_fixture (tc1, fx_setup_g_digicam_camerabin, NULL);
    tcase_add_test (tc1, test_g_digicam_camerabin_element_new_limit);
    tcase_add_test (tc1, test_g_digicam_camerabin_element_new_regular);
    suite_add_tcase (s, tc1);

    /* Create test case for descriptor_new and add it to the suite */
    tcase_add_checked_fixture (tc2,
                               fx_setup_default_gst_camerabins,
                               fx_teardown_default_camerabins);
    tcase_add_test (tc2, test_g_digicam_camerabin_descriptor_new_invalid);
    tcase_add_test (tc2, test_g_digicam_camerabin_descriptor_new_limit);
    tcase_add_test (tc2, test_g_digicam_camerabin_descriptor_new_regular);
    suite_add_tcase (s, tc2);

    /* Return created suite */
    return s;
}
