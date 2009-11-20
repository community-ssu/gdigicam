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

#ifndef __G_DIGICAM_MANAGER_PRIVATE_H__
#define __G_DIGICAM_MANAGER_PRIVATE_H__

#include <glib-object.h>
#include <gst/gst.h>

#ifdef __cplusplus
extern "C" {
#endif

    G_BEGIN_DECLS

    typedef struct _GDigicamManagerPrivate GDigicamManagerPrivate;

#define G_DIGICAM_MANAGER_GET_PRIVATE(obj)                              \
    (G_TYPE_INSTANCE_GET_PRIVATE ((obj),                                \
                                  G_DIGICAM_TYPE_MANAGER, GDigicamManagerPrivate))


    /**
     * GDigicamManagerPrivate:
     *
     * The #GDigicamManagerPrivate structure contains the private data
     * of a #GDigicamManager.
     */
    struct _GDigicamManagerPrivate
    {
/*         GtkWidget* video_window; */

        GstElement *gst_bin;
        GstElement *gst_pipeline;
	gulong xwindow_id;

/*         gchar *saving_location; */
/*         gchar *video_saving_location; */
/*         gchar *photo_base_filename; */
/*         gchar *video_base_filename; */
/*         guint32 photo_file_counter; */
/*         guint32 video_file_counter; */

/*         gulong photo_handler_signal_id; */

/*         gboolean is_recording; */
/*         gboolean pipeline_is_playing; */
/*         gchar *photo_filename; */

/*         gint num_webcam_devices; */
/*         gchar *device_name; */
/*         CheeseWebcamDevice *webcam_devices; */

        GDigicamDescriptor *descriptor;
        GDigicamMode mode;
        GDigicamFlashmode flash_mode;
        gboolean is_flash_ready;
/*         GDigicamAperturemode aperture_mode; */
        GDigicamFocusmode focus_mode;
        gboolean is_macro_enabled;
        GDigicamFocuspoints focus_points;
        guint64 active_points;
        GSList *focus_region_positions;
        GDigicamExposuremode exposure_mode;
        gdouble exposure_compensation;
        GDigicamIsosensitivitymode iso_sensitivity_mode;
        guint iso_level;
/*         GDigicamAperturemode  aperture_mode; */
/*         GDigicamShutterspeedmode shutter_speed_mode; */
        GDigicamWhitebalancemode white_balance_mode;
        guint white_balance_level;
        GDigicamMeteringmode metering_mode;
        GDigicamAspectratio aspect_ratio;
/*         GDigicamColorfilter  color_filter; */
        GDigicamQuality quality;
        GDigicamResolution resolution;
/*         GDigicamVideostabilization video_stabilization; */
        GDigicamLock locks;
        gdouble zoom;
        gboolean digital_zoom;
        GDigicamAudio audio;
        GDigicamPreview preview_mode;
	GMutex *capture_lock;
    };

    /* Protected functions */
    void _g_digicam_manager_set_capture_lock (GDigicamManager *manager);
    void _g_digicam_manager_release_capture_lock (GDigicamManager *manager);
    gboolean _g_digicam_manager_is_valid_flag (GDigicamManager *manager,
                                               guint32 flag,
                                               guint32 low, guint32 high);
    gboolean _g_digicam_manager_is_valid_enum (GDigicamManager *manager,
                                               guint32 flag,
                                               guint32 low, guint32 high);

    G_END_DECLS

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* __G_DIGICAM_MANAGER_PRIVATE_H__ */
