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

GstElement * create_basic_gstreamer_element (const gchar* name);

#endif /* __CHECK_UTILS_H__ */
