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

#ifndef __CHECK_UTILS_H__
#define __CHECK_UTILS_H__

#include <gtk/gtkwidget.h>
#include <gst/gst.h>
#include <gst/interfaces/xoverlay.h>

/* Define sources and sinks according to
 * running environment
 * NOTE: If you want to run the application
 * in ARM scratchbox, you have to change these*/
#ifdef __arm__
/* The device by default supports only
 * vl4l2src for camera and xvimagesink
 * for screen */
#define VIDEO_SINK "xvimagesink"
#else
#ifdef GDIGICAM_PLATFORM_MAEMO
/* These are for the X86 SDK. Xephyr doesn't
 * support XVideo extension, so the application
 * must use ximagesink. The video source depends
 * on driver of your Video4Linux device so this
 * may have to be changed */
#define VIDEO_SINK "ximagesink"
#else
#define VIDEO_SINK "xvimagesink"
#endif
#endif

#include "gdigicam-manager.h"

#define TEST_WINDOW_WIDTH  100
#define TEST_WINDOW_HEIGHT 100

#define MAX_ZOOM_MACRO_DISABLED 9
#define MAX_ZOOM_MACRO_ENABLED 3
#define MAX_OPTICAL_ZOOM_MACRO_DISABLED 6
#define MAX_OPTICAL_ZOOM_MACRO_ENABLED 3
#define MAX_DIGITAL_ZOOM 3

GtkWidget * create_test_window (void);
void show_test_window (GtkWidget * window);
void show_all_test_window (GtkWidget * window);
gulong get_test_window_id (GtkWidget *window);
GstElement * create_basic_gstreamer_element (const gchar* name);
GDigicamDescriptor* create_none_featured_descriptor (GstElement *gst_element);
GDigicamDescriptor* create_minimum_featured_descriptor (GstElement *gst_element);
GDigicamDescriptor* create_full_featured_descriptor (GstElement *gst_element);

#endif /* __CHECK_UTILS_H__ */
