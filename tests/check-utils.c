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

#include <gtk/gtkmain.h>
#include <gst/gstbin.h>
#include <gdk/gdkx.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "check-utils.h"

#ifdef GDIGICAM_PLATFORM_GNOME
#include <gtk/gtkwindow.h>
#endif

#ifdef GDIGICAM_PLATFORM_MAEMO
#include <hildon/hildon-window.h>
#endif

/***********************************************/
/* Abstract function implementation prototypes */
/***********************************************/

static gboolean _dummy_manager_func (GDigicamManager *manager,
                                     gpointer user_data);


/*****************************/
/* Public abstract functions */
/*****************************/


/**
 * create_test_window:
 *
 * Creates a window of a fixed, well known size
 *
 * Returns: creates a #GtkWidget pointer to a main window.
 **/
GtkWidget *
create_test_window (void)
{
    GtkWidget *window = NULL;

#ifdef GDIGICAM_PLATFORM_MAEMO
    window = hildon_window_new ();
#else
    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
#if 0
    gtk_window_resize (GTK_WINDOW (window), TEST_WINDOW_WIDTH, TEST_WINDOW_HEIGHT);
#endif
#endif

    return window;
}


/**
 * show_test_window:
 * @window: a #GtkWidget pointer to a window.
 *
 * Wrapper to gtk_widget_show, that call to gtk_events_pending serving
 *  events.
 **/
void
show_test_window(GtkWidget * window)
{

    gtk_widget_show (window);

    while (gtk_events_pending ()) {
        gtk_main_iteration ();
    }
}


/**
 * show_all_test_window:
 * @window: a #GtkWidget pointer to a window.
 *
 * Wrapper to gtk_widget_show_all, that call to gtk_events_pending
 *  serving events.
 **/
void
show_all_test_window(GtkWidget * window)
{

    gtk_widget_show_all (window);

    while (gtk_events_pending ()) {
        gtk_main_iteration ();
    }
}


/**
 * get_test_window_id:
 * @window: a #GtkWidget pointer to a window.
 *
 * Wrapper to GDK_WINDOW_XID, to get window id.
 *
 * Returns: the window id.
 **/
gulong
get_test_window_id (GtkWidget * window)
{
    return GDK_WINDOW_XID (window->window);
}


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


/**
 * create_none_featured_descriptor:
 * @gst_element: the #GstElement this descriptor owns to
 *
 * Creates a #GdigicamDescriptor from a #GstElement with no
 *  capabilities.
 *
 * Returns: the #GDigicamDescriptor from the #GstElement or %NULL.
 **/
GDigicamDescriptor*
create_none_featured_descriptor (GstElement *gst_element)
{
    GDigicamDescriptor *descriptor = NULL;

    if (NULL != gst_element) {
        descriptor = g_digicam_manager_descriptor_new ();

        /* Name */
        descriptor->name = gst_element_get_name (gst_element);

        /* Viewfinder */
        gst_element = gst_bin_get_by_interface (GST_BIN (gst_element),
                                                GST_TYPE_X_OVERLAY);
        if (NULL != gst_element) {
            descriptor->viewfinder_sink = gst_element;
            descriptor->supported_features = descriptor->supported_features |
                G_DIGICAM_CAPABILITIES_VIEWFINDER;
        }
    }

    return descriptor;
}


/**
 * create_minimum_featured_descriptor:
 * @gst_element: the #GstElement this descriptor owns to
 *
 * Creates a #GdigicamDescriptor from a #GstElement with the minimum
 *  requested capabilities.
 *
 * Returns: the #GDigicamDescriptor from the #GstElement or %NULL.
 **/
GDigicamDescriptor*
create_minimum_featured_descriptor (GstElement *gst_element)
{
    GDigicamDescriptor *descriptor = NULL;

    descriptor = create_none_featured_descriptor (gst_element);

    if (NULL != descriptor) {
        /* Features */
        descriptor->supported_modes = descriptor->supported_modes |
            G_DIGICAM_MODE_STILL |
            G_DIGICAM_MODE_VIDEO;
        descriptor->set_mode_func =
            (GDigicamManagerFunc) _dummy_manager_func;
    }

    return descriptor;
}


/**
 * create_full_featured_descriptor:
 * @gst_element: the #GstElement this descriptor owns to
 *
 * Creates a #GdigicamDescriptor from a #GstElement with all the
 *  available capabilities.
 *
 * Returns: the #GDigicamDescriptor from the #GstElement or %NULL.
 **/
GDigicamDescriptor*
create_full_featured_descriptor (GstElement *gst_element)
{
    GDigicamDescriptor *descriptor = NULL;

    descriptor = create_minimum_featured_descriptor (gst_element);

    if (NULL != descriptor) {
        /* Features */
        descriptor->supported_features = descriptor->supported_features |
            G_DIGICAM_CAPABILITIES_FLASH |
            G_DIGICAM_CAPABILITIES_AUTOFOCUS |
            G_DIGICAM_CAPABILITIES_CONTINUOUSAUTOFOCUS |
            G_DIGICAM_CAPABILITIES_MANUALFOCUS |
            G_DIGICAM_CAPABILITIES_MACROFOCUS |
            G_DIGICAM_CAPABILITIES_AUTOEXPOSURE |
            G_DIGICAM_CAPABILITIES_MANUALEXPOSURE |
            G_DIGICAM_CAPABILITIES_METERING |
            G_DIGICAM_CAPABILITIES_ASPECTRATIO |
            G_DIGICAM_CAPABILITIES_RESOLUTION |
            G_DIGICAM_CAPABILITIES_OPTICALZOOM |
            G_DIGICAM_CAPABILITIES_QUALITY |
            G_DIGICAM_CAPABILITIES_DIGITALZOOM |
            G_DIGICAM_CAPABILITIES_AUDIO |
            G_DIGICAM_CAPABILITIES_PREVIEW |
            G_DIGICAM_CAPABILITIES_MANUALISOSENSITIVITY |
            G_DIGICAM_CAPABILITIES_AUTOISOSENSITIVITY |
            G_DIGICAM_CAPABILITIES_AUTOWHITEBALANCE |
            G_DIGICAM_CAPABILITIES_MANUALWHITEBALANCE;
        descriptor->supported_flash_modes = descriptor->supported_flash_modes |
            G_DIGICAM_FLASHMODE_OFF |
            G_DIGICAM_FLASHMODE_ON |
            G_DIGICAM_FLASHMODE_AUTO |
            G_DIGICAM_FLASHMODE_REDEYEREDUCTION |
            G_DIGICAM_FLASHMODE_REDEYEREDUCTIONAUTO |
            G_DIGICAM_FLASHMODE_FILLIN;
        descriptor->set_flash_mode_func =
            (GDigicamManagerFunc) _dummy_manager_func;
        descriptor->supported_focus_modes = descriptor->supported_focus_modes |
            G_DIGICAM_FOCUSMODE_MANUAL |
            G_DIGICAM_FOCUSMODE_AUTO   |
            G_DIGICAM_FOCUSMODE_FACE   |
            G_DIGICAM_FOCUSMODE_SMILE  |
            G_DIGICAM_FOCUSMODE_CENTROID |
            G_DIGICAM_FOCUSMODE_CONTINUOUSAUTO |
            G_DIGICAM_FOCUSMODE_CONTINUOUSCENTROID;
        descriptor->set_focus_mode_func =
            (GDigicamManagerFunc) _dummy_manager_func;
        descriptor->set_focus_region_pattern_func =
            (GDigicamManagerFunc) _dummy_manager_func;
        descriptor->supported_exposure_modes = descriptor->supported_exposure_modes |
            G_DIGICAM_EXPOSUREMODE_MANUAL |
            G_DIGICAM_EXPOSUREMODE_AUTO |
            G_DIGICAM_EXPOSUREMODE_NIGHT |
            G_DIGICAM_EXPOSUREMODE_BACKLIGHT |
            G_DIGICAM_EXPOSUREMODE_SPOTLIGHT |
            G_DIGICAM_EXPOSUREMODE_SPORTS |
            G_DIGICAM_EXPOSUREMODE_SNOW |
            G_DIGICAM_EXPOSUREMODE_BEACH |
            G_DIGICAM_EXPOSUREMODE_LARGEAPERTURE |
            G_DIGICAM_EXPOSUREMODE_SMALLAPERTURE |
            G_DIGICAM_EXPOSUREMODE_PORTRAIT |
            G_DIGICAM_EXPOSUREMODE_NIGHTPORTRAIT;
        descriptor->set_exposure_mode_func =
            (GDigicamManagerFunc) _dummy_manager_func;
        descriptor->supported_metering_modes = descriptor->supported_metering_modes |
            G_DIGICAM_METERINGMODE_AVERAGE |
            G_DIGICAM_METERINGMODE_SPOT |
            G_DIGICAM_METERINGMODE_MATRIX;
        descriptor->set_metering_mode_func =
            (GDigicamManagerFunc) _dummy_manager_func;
        descriptor->supported_aspect_ratios = descriptor->supported_aspect_ratios |
            G_DIGICAM_ASPECTRATIO_4X3 |
            G_DIGICAM_ASPECTRATIO_16X9;
        descriptor->set_aspect_ratio_func =
            (GDigicamManagerFunc) _dummy_manager_func;
        descriptor->set_aspect_ratio_resolution_func =
            (GDigicamManagerFunc) _dummy_manager_func;
        descriptor->supported_resolutions = descriptor->supported_resolutions |
            G_DIGICAM_RESOLUTION_HIGH |
            G_DIGICAM_RESOLUTION_MEDIUM |
            G_DIGICAM_RESOLUTION_LOW;
        descriptor->set_resolution_func =
            (GDigicamManagerFunc) _dummy_manager_func;
	descriptor->set_iso_sensitivity_mode_func =
	    (GDigicamManagerFunc) _dummy_manager_func;
	descriptor->supported_white_balance_modes = descriptor->supported_white_balance_modes |
            G_DIGICAM_WHITEBALANCEMODE_MANUAL |
            G_DIGICAM_WHITEBALANCEMODE_AUTO   |
            G_DIGICAM_WHITEBALANCEMODE_SUNLIGHT |
            G_DIGICAM_WHITEBALANCEMODE_CLOUDY |
            G_DIGICAM_WHITEBALANCEMODE_SHADE   |
            G_DIGICAM_WHITEBALANCEMODE_TUNGSTEN |
            G_DIGICAM_WHITEBALANCEMODE_FLUORESCENT |
            G_DIGICAM_WHITEBALANCEMODE_INCANDESCENT |
            G_DIGICAM_WHITEBALANCEMODE_FLASH |
            G_DIGICAM_WHITEBALANCEMODE_SUNSET ;
        descriptor->set_white_balance_mode_func =
	    (GDigicamManagerFunc) _dummy_manager_func;
        descriptor->max_zoom_macro_disabled = MAX_ZOOM_MACRO_DISABLED;
        descriptor->max_zoom_macro_enabled = MAX_ZOOM_MACRO_ENABLED;
        descriptor->max_optical_zoom_macro_disabled =
            MAX_OPTICAL_ZOOM_MACRO_DISABLED;
        descriptor->max_optical_zoom_macro_enabled =
            MAX_OPTICAL_ZOOM_MACRO_ENABLED;
        descriptor->max_digital_zoom = MAX_DIGITAL_ZOOM;
        descriptor->set_zoom_func =
            (GDigicamManagerFunc) _dummy_manager_func;
        descriptor->set_locks_func =
            (GDigicamManagerFunc) _dummy_manager_func;
        descriptor->get_still_picture_func =
            (GDigicamManagerFunc) _dummy_manager_func;
        descriptor->start_recording_video_func =
            (GDigicamManagerFunc) _dummy_manager_func;
        descriptor->pause_recording_video_func =
            (GDigicamManagerFunc) _dummy_manager_func;
        descriptor->finish_recording_video_func =
            (GDigicamManagerFunc) _dummy_manager_func;
        descriptor->handle_sync_bus_message_func =
            (GDigicamManagerFunc) _dummy_manager_func;
        descriptor->set_exposure_comp_func =
            (GDigicamManagerFunc) _dummy_manager_func;
	descriptor->supported_qualities = descriptor->supported_qualities |
            G_DIGICAM_QUALITY_HIGH |
	    G_DIGICAM_QUALITY_LOW;
        descriptor->set_quality_func =
            (GDigicamManagerFunc) _dummy_manager_func;
	descriptor->supported_audio_states = descriptor->supported_audio_states |
	    G_DIGICAM_AUDIO_PLAYBACKON |
	    G_DIGICAM_AUDIO_PLAYBACKOFF |
	    G_DIGICAM_AUDIO_RECORDON    |
	    G_DIGICAM_AUDIO_RECORDOFF;
	descriptor->set_audio_func =
	    (GDigicamManagerFunc) _dummy_manager_func;
	descriptor->supported_preview_modes = descriptor->supported_preview_modes |
	    G_DIGICAM_PREVIEW_ON |
	    G_DIGICAM_PREVIEW_OFF;
	descriptor->set_preview_mode_func =
	    (GDigicamManagerFunc) _dummy_manager_func;
    }

    return descriptor;
}

/****************************************************************/
/* Private functions implementing the abstract public functions */
/****************************************************************/


/**
 * _dummy_manager_func:
 * @manager: the #GDigicamManager which calls this function.
 * @user_data: user data.
 *
 * Dummy #GDigicamManagerFunc to be used with a #GDigicamDescriptor.
 *
 * Returns: always %TRUE.
 **/
static gboolean
_dummy_manager_func (GDigicamManager *manager,
                     gpointer user_data)
{

    return TRUE;
}
