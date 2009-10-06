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

#include <gst/gstbin.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "check-utils.h"

/***********************************************/
/* Abstract function implementation prototypes */
/***********************************************/


/*****************************/
/* Public abstract functions */
/*****************************/


/**
 * create_basic_gstreamer_element:
 * @name: name to set to the element
 *
 * Creates a #GstElement with basic GDigicam functionality.
 *
 * Returns: a #GstElement with basic GDigicam functionality or %NULL
 *  if not possible to do so.
 **/
GstElement *
create_basic_gstreamer_element (const gchar* name)
{
    GstElement* camera_bin = NULL;
    GstElement* src = NULL;
    GstElement* colorspace = NULL;
    GstElement* sink = NULL;

    /* Setting the camera bin */
    camera_bin = gst_bin_new (NULL);
    src = gst_element_factory_make ("videotestsrc", NULL);
    colorspace = gst_element_factory_make ("ffmpegcolorspace", NULL);
    sink = gst_element_factory_make (VIDEO_SINK, NULL);
    if (NULL != camera_bin && NULL != src &&
        NULL != colorspace && NULL != sink) {
        gst_bin_add_many (GST_BIN_CAST (camera_bin),
                          src, colorspace, sink,
                          NULL);
        gst_element_link_many (src, colorspace, sink, NULL);
        gst_element_set_name (camera_bin, name);
    } else {
        if (NULL != camera_bin) {
            gst_object_unref (GST_OBJECT (camera_bin));
        }
        if (NULL != src) {
            gst_object_unref (GST_OBJECT (src));
        }
        if (NULL != colorspace) {
            gst_object_unref (GST_OBJECT (colorspace));
        }
        if (NULL != sink) {
            gst_object_unref (GST_OBJECT (sink));
        }
    }

    return camera_bin;
}

/****************************************************************/
/* Private functions implementing the abstract public functions */
/****************************************************************/
