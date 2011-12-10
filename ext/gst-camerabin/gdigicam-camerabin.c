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
 * SECTION:gdigicam-camerabin
 * @short_description: GDigicam Descriptor for GStreamer camerabin
 * plugin
 *
 * GDigicam Camerabin provides the tools needed to easily create a
 * complete #GDigicamDescriptor which internally makes use of the
 * GStreamer camerabin plugin so it can be used straight away in a
 * #GDigicamManager.
 **/

#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gst/interfaces/photography.h>
#include <gst/video/video.h>

#include <config.h>

#include "gdigicam-camerabin.h"
#include "gdigicam-manager-private.h"
#include "gdigicam-debug.h"


/*****************************************/
/* Utiliy macros */
/*****************************************/

#define G_KEY_FILE_PATH CONFIG_DIR "/gdigicam-camerabin.conf"

#define GST_TAG_DATE_TIME_ORIGINAL          "date-time-original"
#define GST_TAG_DATE_TIME_MODIFIED          "date-time-modified"
#define GST_TAG_DEVICE_MAKE                 "device-make"
#define GST_TAG_DEVICE_MODEL                "device-model"
#define GST_TAG_CLASSIFICATION              "classification"
#define GST_TAG_GEO_LOCATION_COUNTRY        "geo-location-country"
#define GST_TAG_GEO_LOCATION_CITY           "geo-location-city"
#define GST_TAG_GEO_LOCATION_SUBLOCATION    "geo-location-sublocation"
#define GST_TAG_CAPTURE_ORIENTATION         "capture-orientation"

/* FIXME: This should be customizable somehow. */
#define G_DIGICAM_CAMERABIN_STILL_4X3_HIGH_WIDTH 2576;
#define G_DIGICAM_CAMERABIN_STILL_4X3_HIGH_HEIGHT 1936;
#define G_DIGICAM_CAMERABIN_STILL_4X3_MEDIUM_WIDTH 2048;
#define G_DIGICAM_CAMERABIN_STILL_4X3_MEDIUM_HEIGHT 1536;
#define G_DIGICAM_CAMERABIN_STILL_4X3_LOW_WIDTH 1280;
#define G_DIGICAM_CAMERABIN_STILL_4X3_LOW_HEIGHT 960;

#define G_DIGICAM_CAMERABIN_STILL_16X9_HIGH_WIDTH 2560;
#define G_DIGICAM_CAMERABIN_STILL_16X9_HIGH_HEIGHT 1440;
#define G_DIGICAM_CAMERABIN_STILL_16X9_MEDIUM_WIDTH 2560;
#define G_DIGICAM_CAMERABIN_STILL_16X9_MEDIUM_HEIGHT 1440;
#define G_DIGICAM_CAMERABIN_STILL_16X9_LOW_WIDTH 2560;
#define G_DIGICAM_CAMERABIN_STILL_16X9_LOW_HEIGHT 1440;


#define G_DIGICAM_CAMERABIN_VIDEO_4X3_HD_WIDTH 960;
#define G_DIGICAM_CAMERABIN_VIDEO_4X3_HD_HEIGHT 720;
#define G_DIGICAM_CAMERABIN_VIDEO_4X3_DVD_WIDTH 720;
#define G_DIGICAM_CAMERABIN_VIDEO_4X3_DVD_HEIGHT 576;

#define G_DIGICAM_CAMERABIN_VIDEO_4X3_HIGH_WIDTH 640;
#define G_DIGICAM_CAMERABIN_VIDEO_4X3_HIGH_HEIGHT 480;
#define G_DIGICAM_CAMERABIN_VIDEO_4X3_MEDIUM_WIDTH 640;
#define G_DIGICAM_CAMERABIN_VIDEO_4X3_MEDIUM_HEIGHT 480;
#define G_DIGICAM_CAMERABIN_VIDEO_4X3_LOW_WIDTH 320;
#define G_DIGICAM_CAMERABIN_VIDEO_4X3_LOW_HEIGHT 240;

#define G_DIGICAM_CAMERABIN_VIDEO_16X9_HD_WIDTH 1280;
#define G_DIGICAM_CAMERABIN_VIDEO_16X9_HD_HEIGHT 720;
#define G_DIGICAM_CAMERABIN_VIDEO_16X9_DVD_WIDTH 1024;
#define G_DIGICAM_CAMERABIN_VIDEO_16X9_DVD_HEIGHT 576;

#define G_DIGICAM_CAMERABIN_VIDEO_16X9_HIGH_WIDTH 848;
#define G_DIGICAM_CAMERABIN_VIDEO_16X9_HIGH_HEIGHT 480;
#define G_DIGICAM_CAMERABIN_VIDEO_16X9_MEDIUM_WIDTH 848;
#define G_DIGICAM_CAMERABIN_VIDEO_16X9_MEDIUM_HEIGHT 480;
#define G_DIGICAM_CAMERABIN_VIDEO_16X9_LOW_WIDTH 848;
#define G_DIGICAM_CAMERABIN_VIDEO_16X9_LOW_HEIGHT 480;


/* Viewfinder resolutions and fps for still picture. */
#define G_DIGICAM_CAMERABIN_VIEWFINDER_4X3_WIDTH 640;
#define G_DIGICAM_CAMERABIN_VIEWFINDER_4X3_HEIGHT 480;
#define G_DIGICAM_CAMERABIN_VIEWFINDER_4X3_FPS 2993;

#define G_DIGICAM_CAMERABIN_VIEWFINDER_16X9_WIDTH 800;
#define G_DIGICAM_CAMERABIN_VIEWFINDER_16X9_HEIGHT 450;
#define G_DIGICAM_CAMERABIN_VIEWFINDER_16X9_FPS 2988;

/* Framerate for video widescreen mode. */
#define G_DIGICAM_CAMERABIN_VIEWFINDER_16X9_VIDEO_FPS 3000;

/* Framerate for video 4x3 DVD (720x576)mode. */
#define G_DIGICAM_CAMERABIN_VIEWFINDER_4X3_DVD_VIDEO_FPS 3000;

/* Framerate for video 4x3 HD (960x720) mode. */
#define G_DIGICAM_CAMERABIN_VIEWFINDER_4X3_HD_VIDEO_FPS 3000;

/* Framerate for video 16x9 DVD (1024x576)mode. */
#define G_DIGICAM_CAMERABIN_VIEWFINDER_16X9_DVD_VIDEO_FPS 3000;

/* Framerate for video 16x9 HD (1280x720) mode. */
#define G_DIGICAM_CAMERABIN_VIEWFINDER_16X9_HD_VIDEO_FPS 2500;

#define G_DIGICAM_CAMERABIN_PHOTO_CAPTURE_START_MESSAGE "photo-capture-start"
#define G_DIGICAM_CAMERABIN_PHOTO_CAPTURE_PICTURE_GOT_MESSAGE "photo-capture-end"
#define G_DIGICAM_CAMERABIN_PHOTO_CAPTURE_END_MESSAGE   "image-captured"
#define G_DIGICAM_CAMERABIN_PHOTO_PREVIEW_MESSAGE       "preview-image"

#define G_DIGICAM_CAMERABIN_DEFAULT_COLORKEY 0x000010

typedef struct _PreviewHelper {
    GDigicamManager *mgr;
    GdkPixbuf *preview;
} PreviewHelper;


/**************************************************/
/* Camerabin operations implementation prototypes */
/**************************************************/

static gboolean _g_digicam_camerabin_set_mode (GDigicamManager *manager,
                                               gpointer user_data);
static gboolean _g_digicam_camerabin_set_flash_mode (GDigicamManager *manager,
                                                     gpointer user_data);
static gboolean _g_digicam_camerabin_set_focus_mode (GDigicamManager *manager,
                                                     gpointer user_data);
static gboolean _g_digicam_camerabin_set_focus_region_pattern (GDigicamManager *manager,
                                                               gpointer user_data);
static gboolean _g_digicam_camerabin_set_exposure_mode (GDigicamManager *manager,
                                                        gpointer user_data);
static gboolean _g_digicam_camerabin_set_exposure_comp (GDigicamManager *manager,
                                                        gpointer user_data);
static gboolean _g_digicam_camerabin_set_iso_sensitivity_mode (GDigicamManager *manager,
                                                               gpointer user_data);
static gboolean _g_digicam_camerabin_set_white_balance_mode (GDigicamManager *manager,
                                                             gpointer user_data);
static gboolean _g_digicam_camerabin_set_quality (GDigicamManager *manager,
                                                  gpointer user_data);
static gboolean _g_digicam_camerabin_set_aspect_ratio_resolution (GDigicamManager *manager,
                                                                  gpointer user_data);
static gboolean _g_digicam_camerabin_set_locks (GDigicamManager *manager,
                                                gpointer user_data);
static gboolean _g_digicam_camerabin_set_zoom (GDigicamManager *manager,
                                               gpointer user_data);
static gboolean _g_digicam_camerabin_set_audio (GDigicamManager *manager,
                                                gpointer user_data);
static gboolean _g_digicam_camerabin_set_preview_mode (GDigicamManager *manager,
                                                       gpointer user_data);
static gboolean _g_digicam_camerabin_get_still_picture (GDigicamManager *manager,
                                                        gpointer user_data);
static gboolean _g_digicam_camerabin_start_recording_video (GDigicamManager *manager,
                                                            gpointer user_data);
static gboolean _g_digicam_camerabin_pause_recording_video (GDigicamManager *manager,
                                                            gpointer user_data);
static gboolean _g_digicam_camerabin_finish_recording_video (GDigicamManager *manager,
                                                             gpointer user_data);
static void _g_digicam_camerabin_set_picture_metadata (GstElement *gst_camera_bin,
                                                       const GDigicamCamerabinMetadata *metadata);
static void _g_digicam_camerabin_set_video_metadata (GstElement *gst_camera_bin,
                                                     const GDigicamCamerabinMetadata *metadata);
static gboolean _g_digicam_camerabin_handle_bus_message (GDigicamManager *manager,
                                                         gpointer user_data);
static gboolean _g_digicam_camerabin_handle_sync_bus_message (GDigicamManager *manager,
							      gpointer user_data);


/**************************************************/
/* Camerabin utility method prototypes            */
/**************************************************/


static void _pixbuf_destroy (guchar *pixels, gpointer data);
static GdkPixbuf *_pixbuf_from_buffer (GDigicamManager *manager,
                                       GstBuffer *buff,
                                       gboolean has_alpha);
static gboolean _emit_preview_signal (gpointer user_data);
static gboolean _emit_capture_start_signal (gpointer user_data);
static gboolean _emit_capture_end_signal (gpointer user_data);
static gboolean _emit_picture_got_signal (gpointer user_data);
static GstCaps *_new_preview_caps (gint pre_w, gint pre_h);
static void _get_aspect_ratio_and_resolution (GDigicamMode mode,
                                              GDigicamAspectratio ar,
                                              GDigicamResolution res,
                                              gint *vf_w, gint *vf_h,
                                              gint *res_w, gint *res_h,
                                              gint *fps_n, gint *fps_d);
static void _get_still_aspect_ratio_and_resolution (GDigicamAspectratio ar,
                                                    GDigicamResolution res,
                                                    gint *vf_w, gint *vf_h,
                                                    gint *res_w, gint *res_h,
                                                    gint *fps_n, gint *fps_d);
static void _get_video_aspect_ratio_and_resolution (GDigicamAspectratio ar,
                                                    GDigicamResolution res,
                                                    gint *vf_w, gint *vf_h,
                                                    gint *res_w, gint *res_h,
                                                    gint *fps_n, gint *fps_d);


/*****************************/
/* Public abstract functions */
/*****************************/


/**
 * g_digicam_camerabin_descriptor_new:
 * @gst_camera_bin: The #GstElement described by this descriptor.
 *
 * Creates a #GDigicamDescriptor customized to deal with GStreamer
 * camerabins.
 *
 * Returns: A new #GDigicamDescriptor with the proper functions to
 * deal with a GStreamer camerabin
 **/
GDigicamDescriptor *
g_digicam_camerabin_descriptor_new (const GstElement *gst_camera_bin)
{
    GDigicamDescriptor* descriptor = NULL;

    g_return_val_if_fail (NULL != gst_camera_bin, NULL);
    g_return_val_if_fail (GST_IS_ELEMENT (gst_camera_bin), NULL);

    descriptor = g_digicam_manager_descriptor_new ();
    descriptor->name = g_strdup ("GStreamer CameraBin");
    descriptor->set_mode_func = _g_digicam_camerabin_set_mode;
    descriptor->set_flash_mode_func = _g_digicam_camerabin_set_flash_mode;
    descriptor->set_focus_mode_func = _g_digicam_camerabin_set_focus_mode;
    descriptor->set_focus_region_pattern_func = _g_digicam_camerabin_set_focus_region_pattern;
    descriptor->set_exposure_mode_func = _g_digicam_camerabin_set_exposure_mode;
    descriptor->set_exposure_comp_func = _g_digicam_camerabin_set_exposure_comp;
    descriptor->set_iso_sensitivity_mode_func = _g_digicam_camerabin_set_iso_sensitivity_mode;
    descriptor->set_white_balance_mode_func = _g_digicam_camerabin_set_white_balance_mode;
    descriptor->set_metering_mode_func = NULL;
    descriptor->set_aspect_ratio_func = _g_digicam_camerabin_set_aspect_ratio_resolution;
    descriptor->set_aspect_ratio_resolution_func = _g_digicam_camerabin_set_aspect_ratio_resolution;
    descriptor->set_quality_func = _g_digicam_camerabin_set_quality;
    descriptor->set_resolution_func = _g_digicam_camerabin_set_aspect_ratio_resolution;
    descriptor->set_locks_func = _g_digicam_camerabin_set_locks;
    descriptor->set_zoom_func = _g_digicam_camerabin_set_zoom;
    descriptor->set_audio_func = _g_digicam_camerabin_set_audio;
    descriptor->set_preview_mode_func = _g_digicam_camerabin_set_preview_mode;
    descriptor->get_still_picture_func = _g_digicam_camerabin_get_still_picture;
    descriptor->start_recording_video_func = _g_digicam_camerabin_start_recording_video;
    descriptor->pause_recording_video_func = _g_digicam_camerabin_pause_recording_video;
    descriptor->finish_recording_video_func = _g_digicam_camerabin_finish_recording_video;
    descriptor->handle_bus_message_func = _g_digicam_camerabin_handle_bus_message;
    descriptor->handle_sync_bus_message_func = _g_digicam_camerabin_handle_sync_bus_message;
    g_object_get (G_OBJECT (gst_camera_bin), "vfsink", &descriptor->viewfinder_sink, NULL);

    return descriptor;
}


/**
 * g_digicam_camerabin_element_new:
 * @videosrc: name of a valid video source #GstElement.
 * @videoenc: name of a valid video encoder #GstElement.
 * @videomux: name of a valid video muxer #GstElement.
 * @audiosrc: name of a valid audio source #GstElement.
 * @audioenc: name of a valid audio encoder #GstElement.
 * @imageenc: name of a valid image encoder #GstElement.
 * @imagepp: name of a valid post processing #GstElement.
 * @ximagesink: name of a valid X image sink #GstElement.
 * @colorkey: output parameter to retrieve the colorkey.
 *
 * Creates a customized CameraBin #GstElement.
 *
 * Returns: A new and complete CameraBin #GstElement.
 **/
GstElement *
g_digicam_camerabin_element_new (const gchar *videosrc,
                                 const gchar *videoenc,
                                 const gchar *videomux,
                                 const gchar *audiosrc,
                                 const gchar *audioenc,
				 const gchar *imageenc,
				 const gchar *imagepp,
                                 const gchar *ximagesink,
                                 gint *colorkey)
{
    GstElement *gst_camera_bin = NULL;
    GstElement *vsrc = NULL;
    GstElement *venc = NULL;
    GstElement *vmux = NULL;
    GstElement *asrc = NULL;
    GstElement *aenc = NULL;
    GstElement *ienc = NULL;
    GstElement *ipp = NULL;
    GstElement *ximg = NULL;
    GstPad *pad = NULL;
    GstCaps *caps = NULL;
    GstStructure *gst_struct = NULL;
    gboolean use_config_file = FALSE;
    gchar *element = NULL;
    gint quality = 0;
    gint bitrate = 0;
    gint aenc_width = 0;
    gint aenc_depth = 0;
    gint aenc_rate = 0;
    gint aenc_channels = 0;
    GKeyFile *key_file = NULL;
    gint ximg_colorkey = 0;
    GstElement *aenc_bin = NULL;
    GstElement *capsfilter = NULL;

#ifdef USE_CONFIG_FILE
    key_file = g_key_file_new ();
    use_config_file = g_key_file_load_from_file (key_file,
                                                 G_KEY_FILE_PATH,
                                                 G_KEY_FILE_NONE,
                                                 NULL);
    if (use_config_file) {
	G_DIGICAM_DEBUG ("GDigicamCamerabin::g_digicam_camerabin_element_new: "
                         "config file exists.");
	use_config_file = g_key_file_get_boolean (key_file,
                                                  "global",
                                                  "useconfigfile",
                                                  NULL);
	if (use_config_file) {
	    G_DIGICAM_DEBUG ("GDigicamCamerabin::g_digicam_camerabin_element_new: "
                             "config file indicates to use file config.");
	} else {
	    G_DIGICAM_DEBUG ("GDigicamCamerabin::g_digicam_camerabin_element_new: "
                             "config file indicates to use application config.");
	}
    } else {
	G_DIGICAM_DEBUG ("GDigicamCamerabin::g_digicam_camerabin_element_new: "
                         "config file doesn't exist.");
    }
#endif

    /* Create a new instance of Camerabin component */
    gst_camera_bin = gst_element_factory_make ("camerabin", NULL);
    if (NULL == gst_camera_bin) {
        G_DIGICAM_DEBUG ("GDigicamCamerabin::g_digicam_camerabin_element_new: "
                         "GStreamer bin creation failed!!!");
        goto cleanup;
    }


    /* --------------------- videosrc --------------------- */

    if (use_config_file) {
	element = g_key_file_get_string (key_file,
					 "videosrc",
					 "element",
					 NULL);
        if (NULL != element) {
            G_DIGICAM_DEBUG ("GDigicamCamerabin::g_digicam_camerabin_element_new: "
                             "Using %s videosrc from config file.", element);
            vsrc = gst_element_factory_make (element, NULL);
            g_free (element);
        }
    } else {
	if (NULL != videosrc) {
	    G_DIGICAM_DEBUG ("GDigicamCamerabin::g_digicam_camerabin_element_new: "
                             "App specified %s as videosrc. Using it.",
                             videosrc);
	    vsrc = gst_element_factory_make (videosrc, NULL);
	} else {
	    G_DIGICAM_DEBUG ("GDigicamCamerabin::g_digicam_camerabin_element_new: "
                             "Application didn't specify any videosrc.");
	}
    }

    if (NULL != vsrc) {
	g_object_set (G_OBJECT (gst_camera_bin), "videosrc", vsrc, NULL);
    } else {
	G_DIGICAM_DEBUG ("GDigicamCamerabin::g_digicam_camerabin_element_new: "
                         "video source element creation failed!!!");
    }


    /* --------------------- videoenc --------------------- */

    if (use_config_file) {
	element = g_key_file_get_string (key_file,
					 "videoenc",
					 "element",
					 NULL);
	if (NULL != element) {
            G_DIGICAM_DEBUG ("GDigicamCamerabin::g_digicam_camerabin_element_new: "
                             "Using %s videoenc from config file.", element);
            venc = gst_element_factory_make (element, NULL);
            g_free (element);
        }
    } else {
	if (NULL != videoenc) {
	    G_DIGICAM_DEBUG ("GDigicamCamerabin::g_digicam_camerabin_element_new: "
                             "App specified %s as videoenc. Using it.",
                             videoenc);
	    venc = gst_element_factory_make (videoenc, NULL);
	} else {
	    G_DIGICAM_DEBUG ("GDigicamCamerabin::g_digicam_camerabin_element_new: "
                             "Application didn't specify any videoenc.");
	}
    }

    if (NULL != venc) {
	g_object_set (G_OBJECT (gst_camera_bin), "videoenc", venc, NULL);
    } else {
	G_DIGICAM_DEBUG ("GDigicamCamerabin::g_digicam_camerabin_element_new: "
                         "video encoder element creation failed!!!");
    }


    /* --------------------- videomux --------------------- */

    if (use_config_file) {
	element = g_key_file_get_string (key_file,
					 "videomux",
					 "element",
					 NULL);
	if (NULL != element) {
            G_DIGICAM_DEBUG ("GDigicamCamerabin::g_digicam_camerabin_element_new: "
                             "Using %s videomux from config file.", element);
            vmux = gst_element_factory_make (element, NULL);
            g_free (element);
        }
    } else {
	if (NULL != videomux) {
	    G_DIGICAM_DEBUG ("GDigicamCamerabin::g_digicam_camerabin_element_new: "
                             "App specified %s as videomux. Using it.",
                             videomux);
	    vmux = gst_element_factory_make (videomux, NULL);
	} else {
	    G_DIGICAM_DEBUG ("GDigicamCamerabin::g_digicam_camerabin_element_new: "
                             "Application didn't specify any videomux.");
	}
    }

    if (NULL != vmux) {
	g_object_set (G_OBJECT (gst_camera_bin), "videomux", vmux, NULL);
    } else {
	G_DIGICAM_DEBUG ("GDigicamCamerabin::g_digicam_camerabin_element_new: "
                         "video mux element creation failed!!!");
    }


    /* --------------------- audiosrc --------------------- */

    if (use_config_file) {
	element = g_key_file_get_string (key_file,
					 "audiosrc",
					 "element",
					 NULL);
	if (NULL != element) {
            G_DIGICAM_DEBUG ("GDigicamCamerabin::g_digicam_camerabin_element_new: "
                             "Using %s audiosrc from config file.", element);
            asrc = gst_element_factory_make (element, NULL);
            g_free (element);
        }
    } else {
	if (NULL != audiosrc) {
	    G_DIGICAM_DEBUG ("GDigicamCamerabin::g_digicam_camerabin_element_new: "
                             "App specified %s as audiosrc. Using it.", audiosrc);
	    asrc = gst_element_factory_make (audiosrc, NULL);
	} else {
	    G_DIGICAM_DEBUG ("GDigicamCamerabin::g_digicam_camerabin_element_new: "
                             "Application didn't specify any audiosrc.");
	}
    }

    if (NULL != asrc) {
	g_object_set (G_OBJECT (gst_camera_bin), "audiosrc", asrc, NULL);
    } else {
	G_DIGICAM_DEBUG ("GDigicamCamerabin::g_digicam_camerabin_element_new: "
                         "audio source element creation failed!!!");
    }


    /* --------------------- audioenc --------------------- */

    if (use_config_file) {
	element = g_key_file_get_string (key_file,
					 "audioenc",
					 "element",
					 NULL);
	if (NULL != element) {
            G_DIGICAM_DEBUG ("GDigicamCamerabin::g_digicam_camerabin_element_new: "
                             "Using %s audioenc from config file.", element);
            aenc = gst_element_factory_make (element, NULL);

            /* bitrate parameter */
            bitrate = g_key_file_get_integer (key_file,
                                              "audioenc",
                                              "bitrate",
                                              NULL);
            /* caps parameters */
            aenc_width = g_key_file_get_integer (key_file,
                                                 "audioenc",
                                                 "width",
                                                 NULL);
            aenc_depth = g_key_file_get_integer (key_file,
                                                 "audioenc",
                                                 "depth",
                                                 NULL);
            aenc_rate = g_key_file_get_integer (key_file,
                                                "audioenc",
                                                "rate",
                                                NULL);
            aenc_channels = g_key_file_get_integer (key_file,
                                                    "audioenc",
                                                    "channels",
                                                    NULL);

            g_free (element);
        }
    } else {
	if (NULL != audioenc) {
	    G_DIGICAM_DEBUG ("GDigicamCamerabin::g_digicam_camerabin_element_new: "
                             "App specified %s as audioenc. Using it.",
                             audioenc);
	    aenc = gst_element_factory_make (audioenc, NULL);
	    bitrate = 128000;
	    aenc_width = 16;
	    aenc_depth = 16;
	    aenc_rate = 48000;
	    aenc_channels = 1;
	} else {
	    G_DIGICAM_DEBUG ("GDigicamCamerabin::g_digicam_camerabin_element_new: "
                             "Application didn't specify any audioenc.");
	}
    }

    if (NULL != aenc) {
        if (0 != bitrate) {
            g_object_set (G_OBJECT (aenc), "bitrate", bitrate, NULL);
        }

	caps = gst_caps_new_empty ();
        gst_struct = gst_structure_empty_new ("audio/x-raw-int");

        if (0 != aenc_width) {
            gst_structure_set (gst_struct,
                               "width", G_TYPE_INT, aenc_width,
                               NULL);
        }

        if (0 != aenc_depth) {
            gst_structure_set (gst_struct,
                               "depth", G_TYPE_INT, aenc_depth,
                               NULL);
        }

        if (0 != aenc_rate) {
            gst_structure_set (gst_struct,
                               "rate", G_TYPE_INT, aenc_rate,
                               NULL);
        }

        if (0 != aenc_channels) {
            gst_structure_set (gst_struct,
                               "channels", G_TYPE_INT, aenc_channels,
                               NULL);
        }

        gst_caps_merge_structure (caps, gst_struct);

	aenc_bin = gst_bin_new ("aenc_bin");
	capsfilter = gst_element_factory_make ("capsfilter", NULL);
	g_object_set (capsfilter,
		      "caps",
		      caps,
		      NULL);
	gst_caps_unref (caps);

	gst_bin_add_many (GST_BIN (aenc_bin), capsfilter, aenc, NULL);
	gst_element_link_many (capsfilter, aenc, NULL);

	pad = gst_element_get_static_pad (capsfilter, "sink");
	gst_element_add_pad (aenc_bin, gst_ghost_pad_new ("sink", pad));
	gst_object_unref (pad);

	pad = gst_element_get_static_pad (aenc, "src");
	gst_element_add_pad (aenc_bin, gst_ghost_pad_new ("src", pad));
	gst_object_unref (pad);

	g_object_set (G_OBJECT (gst_camera_bin), "audioenc", aenc_bin, NULL);
    } else {
	G_DIGICAM_DEBUG ("GDigicamCamerabin::g_digicam_camerabin_element_new: "
                         "audio encoder element creation failed!!!");
    }


    /* --------------------- imageenc --------------------- */

    if (use_config_file) {
	element = g_key_file_get_string (key_file,
					 "imageenc",
					 "element",
					 NULL);
	if (NULL != element) {
            G_DIGICAM_DEBUG ("GDigicamCamerabin::g_digicam_camerabin_element_new: "
                             "Using %s imageenc from config file.", element);
            ienc = gst_element_factory_make (element, NULL);

            /*get quality value */
            quality = g_key_file_get_integer (key_file,
                                              "imageenc",
                                              "quality",
                                              NULL);

            g_free (element);
        }
    } else {
	if (NULL != imageenc) {
	    G_DIGICAM_DEBUG ("GDigicamCamerabin::g_digicam_camerabin_element_new: "
                             "App specified %s as imageenc. Using it.",
                             imageenc);
	    ienc = gst_element_factory_make (imageenc, NULL);
	    quality = 85;
	} else {
	    G_DIGICAM_DEBUG ("GDigicamCamerabin::g_digicam_camerabin_element_new: "
                             "Application didn't specify any imageenc.");
	}
    }

    if (NULL != ienc) {
        if (0 != quality) {
            if (NULL != g_object_class_find_property (G_OBJECT_GET_CLASS (ienc),
                                                      "quality")) {
                g_object_set (G_OBJECT (ienc), "quality", quality, NULL);
            } else {
                G_DIGICAM_DEBUG ("GDigicamCamerabin::g_digicam_camerabin_element_new: "
                                 "image encoder has not quality capabilities.");
            }
        } else {
            G_DIGICAM_DEBUG ("GDigicamCamerabin::g_digicam_camerabin_element_new: "
                             "quality not specified for image encoder.");
        }

	g_object_set (G_OBJECT (gst_camera_bin), "imageenc", ienc, NULL);
    } else {
	G_DIGICAM_DEBUG ("GDigicamCamerabin::g_digicam_camerabin_element_new: "
                         "image encoder element creation failed!!!");
    }


    /* --------------------- imagepp --------------------- */

    if (use_config_file) {
	element = g_key_file_get_string (key_file,
					 "imagepp",
					 "element",
					 NULL);
	if (NULL != element) {
            G_DIGICAM_DEBUG ("GDigicamCamerabin::g_digicam_camerabin_element_new: "
                             "Using %s imagepp from config file.", element);
            ipp = gst_element_factory_make (element, NULL);
            g_free (element);
        }
    } else {
	if (NULL != imagepp) {
	    G_DIGICAM_DEBUG ("GDigicamCamerabin::g_digicam_camerabin_element_new: "
                             "App specified %s as imagepp. Using it.",
                             imagepp);
	    ipp = gst_element_factory_make (imagepp, NULL);
	} else {
	    G_DIGICAM_DEBUG ("GDigicamCamerabin::g_digicam_camerabin_element_new: "
                             "Application didn't specify any imagepp.");
	}
    }

    if (NULL != ipp) {
	g_object_set (G_OBJECT (gst_camera_bin), "imagepp", ipp, NULL);
    } else {
	G_DIGICAM_DEBUG ("GDigicamCamerabin::g_digicam_camerabin_element_new: "
                         "image post processing element creation failed!!!");
    }


    /* --------------------- vfsink --------------------- */

    if (NULL != colorkey) {
        *colorkey = 0;
    }

    if (use_config_file) {
	element = g_key_file_get_string (key_file,
					 "vfsink",
					 "element",
					 NULL);
	if (NULL != element) {
            G_DIGICAM_DEBUG ("GDigicamCamerabin::g_digicam_camerabin_element_new: "
                             "Using %s vfsink from config file.", element);
            ximg = gst_element_factory_make (element, NULL);
            g_free (element);
        }
	ximg_colorkey = g_key_file_get_integer (key_file,
						"vfsink",
						"colorkey",
						NULL);
    } else {
	if (NULL != ximagesink) {
	    G_DIGICAM_DEBUG ("GDigicamCamerabin::g_digicam_camerabin_element_new: "
                             "App specified %s as vfsink. Using it.", ximagesink);
	    ximg = gst_element_factory_make (ximagesink, NULL);
	    ximg_colorkey = G_DIGICAM_CAMERABIN_DEFAULT_COLORKEY;
	} else {
	    G_DIGICAM_DEBUG ("GDigicamCamerabin::g_digicam_camerabin_element_new: "
                             "Application didn't specify any vfsink");
	}
    }

    if (NULL != ximg) {
	g_object_set (G_OBJECT (ximg),
		      "autopaint-colorkey", FALSE,
		      "force-aspect-ratio", TRUE,
		      "draw-borders", FALSE,
		      "colorkey", ximg_colorkey,
		      NULL);

	g_object_set (G_OBJECT (gst_camera_bin), "vfsink", ximg, NULL);

        /* Get colorkey if requested */
	if (NULL != colorkey) {
	    g_object_get (G_OBJECT (ximg),
			  "colorkey", colorkey,
			  NULL);
	}
    } else {
	G_DIGICAM_DEBUG ("GDigicamCamerabin::g_digicam_camerabin_element_new: "
                         "image viewfinder sink element creation failed!!!");
    }

cleanup:

    if (NULL != key_file) {
	g_key_file_free (key_file);
    }

    return gst_camera_bin;
}


/****************************************************************/
/* Private functions implementing the abstract public functions */
/****************************************************************/


/**
 * _g_digicam_camerabin_set_mode:
 * @manager: A #GDigicamManager.
 * @user_data: A #GDigicamCamerabinModeHelper.
 *
 * Implementation of "set_mode" GDigicam operation specifically for
 * the "camerabin" GStreamer bin.
 *
 * Returns: #FALSE if invalid input arguments are received or the
 * operation fails, #TRUE otherwise.
 **/
static gboolean
_g_digicam_camerabin_set_mode (GDigicamManager *manager,
                               gpointer user_data)
{
    GDigicamCamerabinModeHelper *helper = NULL;
    GError *error = NULL;
    GstElement *bin = NULL;
    gboolean result;

    helper = (GDigicamCamerabinModeHelper *) user_data;

    G_DIGICAM_DEBUG ("GDigicamCamerabin: Setting operation mode  ...\n");

    TSTAMP (gst-before-mode-changed);

    /* Get "camerabin" Gstreamer bin  */
    result = g_digicam_manager_get_gstreamer_bin (manager,
                                                  &bin,
                                                  &error);

    /* Check errors */
    if (!result) {
        if (NULL != error) {
            G_DIGICAM_DEBUG ("GDigicamCamerabin: %s", error->message);
        }
        goto free;
    }


    /* FIXME: Pending of camerabin integration */
    switch (helper->mode) {
    case G_DIGICAM_MODE_STILL:
        g_object_set (bin, "mode", 0, NULL);
        break;
    case G_DIGICAM_MODE_VIDEO:
        g_object_set (bin, "mode", 1, NULL);
        break;
    default:
        g_assert_not_reached ();
    }

    TSTAMP (gst-after-mode-changed);

    /* free */
free:
    if (NULL != bin) {
        gst_object_unref (bin);
    }
    if (NULL != error) {
        g_error_free (error);
    }

    return result;
}


/**
 * _g_digicam_camerabin_set_flash_mode:
 * @manager: A #GDigicamManager.
 * @user_data: A #GDigicamCamerabinFlashModeHelper.
 *
 * Implementation of "set_flash_mode" GDigicam operation specifically
 * for the "camerabin" GStreamer bin.
 *
 * Returns: #FALSE if invalid input arguments are received or the
 * operation fails, #TRUE otherwise.
 **/
static gboolean
_g_digicam_camerabin_set_flash_mode (GDigicamManager *manager,
                                     gpointer user_data)
{
    GDigicamCamerabinFlashModeHelper *helper = NULL;
    GError *error = NULL;
    GstElement *bin = NULL;
    GstFlashMode value;
    gboolean result;

    helper = (GDigicamCamerabinFlashModeHelper *) user_data;

    G_DIGICAM_DEBUG ("GDigicamCamerabin: Setting flash mode  ...\n");

    /* Get "camerabin" Gstreamer bin  */
    result = g_digicam_manager_get_gstreamer_bin (manager,
                                                  &bin,
                                                  &error);


    /* Check errors */
    if (!result) {
        if (NULL != error) {
            G_DIGICAM_DEBUG ("GDigicamCamerabin: %s", error->message);
        }
        goto free;
    }

    switch (helper->flash_mode) {
    case G_DIGICAM_FLASHMODE_AUTO:
        /* FIXME: the AUTO scene mode is unimplemented in camerabin side.*/
        value = GST_PHOTOGRAPHY_FLASH_MODE_AUTO;
        break;
    case G_DIGICAM_FLASHMODE_OFF:
        value = GST_PHOTOGRAPHY_FLASH_MODE_OFF;
        break;
    case G_DIGICAM_FLASHMODE_ON:
        value = GST_PHOTOGRAPHY_FLASH_MODE_ON;
        break;
    case G_DIGICAM_FLASHMODE_REDEYEREDUCTION:
        value = GST_PHOTOGRAPHY_FLASH_MODE_RED_EYE;
        break;
    default:
        g_assert_not_reached ();
    }

    /* Establish new flash mode */
    result = gst_photography_set_flash_mode (GST_PHOTOGRAPHY (bin),
                                             value);

    /* free */
free:
    if (NULL != bin) {
        gst_object_unref (bin);
    }
    if (NULL != error) {
        g_error_free (error);
    }

    return result;
}


/**
 * _g_digicam_camerabin_set_focus_mode:
 * @manager: A #GDigicamManager.
 * @user_data: A #GDigicamCamerabinFocusModeHelper.
 *
 * Implementation of "set_focus_mode" GDigicam operation specifically
 * for the "camerabin" GStreamer bin.
 *
 * Returns: #FALSE if invalid input arguments are received or the
 * operation fails, #TRUE otherwise.
 **/
static gboolean
_g_digicam_camerabin_set_focus_mode (GDigicamManager *manager,
                                     gpointer user_data)
{
    GDigicamCamerabinFocusModeHelper *helper = NULL;
    GstElement *bin = NULL;
    GError *error = NULL;
    gboolean result;

    helper = (GDigicamCamerabinFocusModeHelper *) user_data;

    G_DIGICAM_DEBUG ("GDigicamCamerabin: Setting focus mode");

    /* Get "camerabin" Gstreamer bin  */
    result = g_digicam_manager_get_gstreamer_bin (manager,
                                                  &bin,
                                                  &error);

    /* Check errors */
    if (!result) {
        if (NULL != error) {
            G_DIGICAM_DEBUG ("GDigicamCamerabin: %s", error->message);
        }
        goto free;
    }

    /* TODO: Specific implementation to set the focus mode */

    /* free */
free:
    if (NULL != bin) {
        gst_object_unref (bin);
    }
    if (NULL != error) {
        g_error_free (error);
    }

    return result;
}


/**
 * _g_digicam_camerabin_set_focus_region_pattern:
 * @manager: A #GDigicamManager.
 * @user_data: A #GDigicamCamerabinFocusRegionPatternHelper.
 *
 * Implementation of "set_focus_region_pattern" GDigicam operation
 * specifically for the "camerabin" GStreamer bin.
 *
 * Returns: #FALSE if invalid input arguments are received or the
 * operation fails, #TRUE otherwise.
 **/
static gboolean
_g_digicam_camerabin_set_focus_region_pattern (GDigicamManager *manager,
                                               gpointer user_data)
{
    GDigicamCamerabinFocusRegionPatternHelper *helper = NULL;
    GstElement *bin = NULL;
    GError *error = NULL;
    gboolean result;

    helper = (GDigicamCamerabinFocusRegionPatternHelper *) user_data;

    G_DIGICAM_DEBUG ("GDigicamCamerabin: Setting focus region pattern");

    /* Get "camerabin" Gstreamer bin  */
    result = g_digicam_manager_get_gstreamer_bin (manager,
                                                  &bin,
                                                  &error);

    /* Check errors */
    if (!result) {
        if (NULL != error) {
            G_DIGICAM_DEBUG ("GDigicamCamerabin: %s", error->message);
        }
        goto free;
    }

    /* TODO: Specific implementation to set the focus region pattern */

    /* free */
free:
    if (NULL != bin) {
        gst_object_unref (bin);
    }
    if (NULL != error) {
        g_error_free (error);
    }

    return result;
}


/**
 * _g_digicam_camerabin_set_exposure_mode:
 * @manager: A #GDigicamManager.
 * @user_data: A #GDigicamCamerabinExposureModeHelper.
 *
 * Implementation of "set_exposure_mode" GDigicam operation
 * specifically for the "camerabin" GStreamer bin.
 *
 * Returns: #FALSE if invalid input arguments are received or the
 * operation fails, #TRUE otherwise.
 **/
static gboolean
_g_digicam_camerabin_set_exposure_mode (GDigicamManager *manager,
                                        gpointer user_data)
{
    GDigicamCamerabinExposureModeHelper *helper = NULL;
    GstElement *bin = NULL;
    GstSceneMode value;
    GDigicamFocusmode focus_mode;
    gboolean macro_enabled;
    GError *error = NULL;
    gboolean result;
    GDigicamMode mode;

    helper = (GDigicamCamerabinExposureModeHelper *) user_data;

    TSTAMP (gst-scene-mode);

    /* Get "camerabin" Gstreamer bin  */
    result = g_digicam_manager_get_gstreamer_bin (manager,
                                                  &bin,
                                                  &error);

    /* Check errors */
    if (!result) {
        if (NULL != error) {
            G_DIGICAM_DEBUG ("GDigicamCamerabin: %s", error->message);
        }
        goto free;
    }

    result = g_digicam_manager_get_mode (manager,
                                         &mode,
                                         &error);

    /* Check errors */
    if (!result) {
        if (NULL != error) {
            G_DIGICAM_DEBUG ("GDigicamCamerabin: %s", error->message);
        }
        goto free;
    }

    if (mode == G_DIGICAM_MODE_VIDEO) {
	if (helper->exposure_mode == G_DIGICAM_EXPOSUREMODE_NIGHT) {
	    value = GST_PHOTOGRAPHY_SCENE_MODE_NIGHT;
	} else {
	    value = GST_PHOTOGRAPHY_SCENE_MODE_AUTO;
	}
    } else {
	switch (helper->exposure_mode) {
	case G_DIGICAM_EXPOSUREMODE_AUTO:
	    if (NULL != error) {
		g_error_free (error);
		error = NULL;
	    }

	    result = g_digicam_manager_get_focus_mode (manager,
						       &focus_mode,
						       &macro_enabled,
						       &error);

	    /* Check errors */
	    if (!result) {
		if (NULL != error) {
		    G_DIGICAM_DEBUG ("GDigicamCamerabin: %s", error->message);
		}
		goto free;
	    }

	    if (macro_enabled) {
		value = GST_PHOTOGRAPHY_SCENE_MODE_CLOSEUP;
	    } else {
		value = GST_PHOTOGRAPHY_SCENE_MODE_AUTO;
	    }
	    break;
	case G_DIGICAM_EXPOSUREMODE_LANDSCAPE:
	    value = GST_PHOTOGRAPHY_SCENE_MODE_LANDSCAPE;
	    break;
	case G_DIGICAM_EXPOSUREMODE_NIGHT:
	    value = GST_PHOTOGRAPHY_SCENE_MODE_NIGHT;
	    break;
	case G_DIGICAM_EXPOSUREMODE_PORTRAIT:
	    value = GST_PHOTOGRAPHY_SCENE_MODE_PORTRAIT;
	    break;
	case G_DIGICAM_EXPOSUREMODE_SPORTS:
	    value = GST_PHOTOGRAPHY_SCENE_MODE_SPORT;
	    break;
	default:
	    g_assert_not_reached ();
	}
    }

    G_DIGICAM_DEBUG ("GDigicamCamerabin: Setting new scene mode to %i\n",
                     value);

    TSTAMP (gst-before-scenemode);

    result = gst_photography_set_scene_mode (GST_PHOTOGRAPHY (bin),
                                             value);

    TSTAMP (gst-after-scenemode);

    /* free */
free:
    if (NULL != bin) {
        gst_object_unref (bin);
    }
    if (NULL != error) {
        g_error_free (error);
    }

    return result;
}


/**
 * _g_digicam_camerabin_set_exposure_comp:
 * @manager: A #GDigicamManager.
 * @user_data: A #GDigicamCamerabinExposureCompHelper.
 *
 * Implementation of "set_exposure_comp" GDigicam operation
 * specifically for the "camerabin" GStreamer bin.
 *
 * Returns: #FALSE if invalid input arguments are received or the
 * operation fails, #TRUE otherwise.
 **/
static gboolean
_g_digicam_camerabin_set_exposure_comp (GDigicamManager *manager,
                                        gpointer user_data)
{
    GDigicamCamerabinExposureCompHelper *helper = NULL;
    GstElement *bin = NULL;
    GError *error = NULL;
    gboolean result;

    helper = (GDigicamCamerabinExposureCompHelper *) user_data;

    /* Get "camerabin" Gstreamer bin  */
    result = g_digicam_manager_get_gstreamer_bin (manager,
                                                  &bin,
                                                  &error);

    /* Check errors */
    if (!result) {
        if (NULL != error) {
            G_DIGICAM_DEBUG ("GDigicamCamerabin: %s", error->message);
        }
        goto free;
    }

    G_DIGICAM_DEBUG ("GDigicamCamerabin: Setting new exposure value to %f\n",
                     helper->exposure_comp);

    result = gst_photography_set_ev_compensation (GST_PHOTOGRAPHY (bin),
                                                  helper->exposure_comp);

    /* free */
free:
    if (NULL != bin) {
        gst_object_unref (bin);
    }
    if (NULL != error) {
        g_error_free (error);
    }

    return result;
}


/**
 * _g_digicam_camerabin_set_iso_sensitivity_mode:
 * @manager: A #GDigicamManager.
 * @user_data: A #GDigicamCamerabinIsoSensitivityHelper.
 *
 * Implementation of "set_iso_sensitivity" GDigicam operation
 * specifically for the "camerabin" GStreamer bin.
 *
 * Returns: #FALSE if invalid input arguments are received or the
 * operation fails, #TRUE otherwise.
 **/
static gboolean
_g_digicam_camerabin_set_iso_sensitivity_mode (GDigicamManager *manager,
                                               gpointer user_data)
{
    GDigicamCamerabinIsoSensitivityHelper *helper = NULL;
    GstElement *bin = NULL;
    GError *error = NULL;
    gboolean result;

    helper = (GDigicamCamerabinIsoSensitivityHelper *) user_data;

    G_DIGICAM_DEBUG ("GDigicamCamerabin: Setting new iso value to %i\n",
                     helper->iso_value);

    /* Get "camerabin" Gstreamer bin  */
    result = g_digicam_manager_get_gstreamer_bin (manager,
                                                  &bin,
                                                  &error);

    /* Check errors */
    if (!result) {
        if (NULL != error) {
            G_DIGICAM_DEBUG ("GDigicamCamerabin: %s", error->message);
        }
        goto free;
    }

    /* Establish new iso value */
    /* FIXME: what about the auto iso value ? */
    result = gst_photography_set_iso_speed (GST_PHOTOGRAPHY (bin),
                                            helper->iso_value);
    /* free */
free:
    if (NULL != bin) {
        gst_object_unref (bin);
    }
    if (NULL != error) {
        g_error_free (error);
    }

    return result;
}


/**
 * _g_digicam_camerabin_set_white_balance_mode:
 * @manager: A #GDigicamManager.
 * @user_data: A #GDigicamCamerabinWhitebalanceModeHelper.
 *
 * Implementation of "set_white_balance_mode" GDigicam operation
 * specifically for the "camerabin" GStreamer bin.
 *
 * Returns: #FALSE if invalid input arguments are received or the
 * operation fails, #TRUE otherwise.
 **/
static gboolean
_g_digicam_camerabin_set_white_balance_mode (GDigicamManager *manager,
                                             gpointer user_data)
{
    GDigicamCamerabinWhitebalanceModeHelper *helper = NULL;
    GstElement *bin = NULL;
    GError *error = NULL;
    GstWhiteBalanceMode value;
    gboolean result;

    helper = (GDigicamCamerabinWhitebalanceModeHelper *) user_data;

    G_DIGICAM_DEBUG ("GDigicamCamerabin: Setting new white balance "
                     "value to %i\n",
                     helper->wb_mode);

    /* Get "camerabin" Gstreamer bin  */
    result = g_digicam_manager_get_gstreamer_bin (manager,
                                                  &bin,
                                                  &error);

    /* Check errors */
    if (!result) {
        if (NULL != error) {
            G_DIGICAM_DEBUG ("GDigicamCamerabin: %s", error->message);
        }
        goto free;
    }

    /* Mapping values */
    switch (helper->wb_mode) {
    case G_DIGICAM_WHITEBALANCEMODE_AUTO:
        value = GST_PHOTOGRAPHY_WB_MODE_AUTO;
        break;
    case G_DIGICAM_WHITEBALANCEMODE_SUNLIGHT:
        value = GST_PHOTOGRAPHY_WB_MODE_DAYLIGHT;
        break;
    case G_DIGICAM_WHITEBALANCEMODE_CLOUDY:
        value = GST_PHOTOGRAPHY_WB_MODE_CLOUDY;
        break;
    case G_DIGICAM_WHITEBALANCEMODE_TUNGSTEN:
        value = GST_PHOTOGRAPHY_WB_MODE_TUNGSTEN;
        break;
    case G_DIGICAM_WHITEBALANCEMODE_FLUORESCENT:
        value = GST_PHOTOGRAPHY_WB_MODE_FLUORESCENT;
        break;
    case G_DIGICAM_WHITEBALANCEMODE_FLASH:
        value = GST_PHOTOGRAPHY_WB_MODE_SUNSET;
        break;
    default:
        g_assert_not_reached ();
    }


    /* Establish new white balance value  */
    result = gst_photography_set_white_balance_mode (GST_PHOTOGRAPHY (bin),
                                                     value);

    /* free */
free:
    if (NULL != bin) {
        gst_object_unref (bin);
    }
    if (NULL != error) {
        g_error_free (error);
    }

    return result;
}


/**
 * _g_digicam_camerabin_set_quality:
 * @manager: A #GDigicamManager.
 * @user_data: A #GDigicamCamerabinQualityHelper.
 *
 * Implementation of "set_quality" GDigicam operation specifically for
 * the "camerabin" GStreamer bin.
 *
 * Returns: #FALSE if invalid input arguments are received or the
 * operation fails, #TRUE otherwise.
 **/
static gboolean
_g_digicam_camerabin_set_quality (GDigicamManager *manager,
                                  gpointer user_data)
{
    GstElement *bin = NULL;
    GDigicamCamerabinQualityHelper *helper = NULL;
    GError *error = NULL;
    gboolean result;

    helper = (GDigicamCamerabinQualityHelper *) user_data;

    G_DIGICAM_DEBUG ("GDigicamCamerabin: Setting new quality \n");

    /* Get "camerabin" Gstreamer bin  */
    result = g_digicam_manager_get_gstreamer_bin (manager,
                                                  &bin,
                                                  &error);

    /* Check errors */
    if (!result) {
        if (NULL != error) {
            G_DIGICAM_DEBUG ("GDigicamCamerabin: %s", error->message);
        }
        goto free;
    }

    /* FIXME: once camera bin has support for quality, use proper API. */
    /* Establish new quality value  */
/*     result = gst_photography_set_quality (GST_PHOTOGRAPHY (bin), */
/*                                           value); */

    /* free */
free:
    if (NULL != bin) {
        gst_object_unref (bin);
    }
    if (NULL != error) {
        g_error_free (error);
    }

    return result;
}


/**
 * _g_digicam_camerabin_set_aspect_ratio_resolution:
 * @manager: A #GDigicamManager.
 * @user_data: A #GDigicamCamerabinAspectRatioResolutionHelper.
 *
 * Implementation of "set_resolution" and "set_aspect_ratio" GDigicam
 * operations specifically for the "camerabin" GStreamer bin.
 *
 * Returns: #FALSE if invalid input arguments are received or the
 * operation fails, #TRUE otherwise.
 **/
static gboolean
_g_digicam_camerabin_set_aspect_ratio_resolution (GDigicamManager *manager,
                                                  gpointer user_data)
{
    GDigicamCamerabinAspectRatioResolutionHelper *helper = NULL;
    GstElement *bin = NULL;
    GstCaps *preview_caps = NULL;
    GDigicamMode mode;
    GError *error = NULL;
    gint vf_w, vf_h;
    gint res_w, res_h;
    gint fps_n, fps_d;
    gboolean enabled;
    gboolean result;

    helper = (GDigicamCamerabinAspectRatioResolutionHelper *) user_data;

    G_DIGICAM_DEBUG ("GDigicamCamerabin: Setting new aspect ratio "
                     "and resolution \n");

    TSTAMP (gst-resolution);

    /* Get "camerabin" Gstreamer bin  */
    result = g_digicam_manager_get_gstreamer_bin (manager,
                                                  &bin,
                                                  &error);

    /* Check errors */
    if (!result) {
        if (NULL != error) {
            G_DIGICAM_DEBUG ("GDigicamCamerabin: %s", error->message);
        }
        goto free;
    }

    /* Get mode to set specific resolution and aspect ratio*/
    result = g_digicam_manager_get_mode (manager,
                                         &mode,
                                         &error);

    /* Check errors */
    if (!result) {
        if (NULL != error) {
            G_DIGICAM_DEBUG ("GDigicamCamerabin: %s", error->message);
        }
        goto free;
    }


    TSTAMP (gst-before-res-changed);

    /* Get resolution specific values depending on the camera mode */
    _get_aspect_ratio_and_resolution (mode,
                                      helper->aspect_ratio,
                                      helper->resolution,
                                      &vf_w, &vf_h,
                                      &res_w, &res_h,
                                      &fps_n, &fps_d);

    /* Set Image Capturing settings  */
    if (G_DIGICAM_MODE_STILL == mode) {
 	G_DIGICAM_DEBUG ("GDigicamCamerabin: Setting capture resolution "
                         "to %dx%d\n", res_w, res_h);

        /* Capture resolution */
 	g_signal_emit_by_name (bin,
 			       "user-image-res",
 			       res_w, res_h,
 			       0);
    }

    G_DIGICAM_DEBUG ("GDigicamCamerabin: Setting new recording/viewfinder "
                     "resolution and fps: %dx%d at %d/%d fps \n",
                     vf_w, vf_h, fps_n, fps_d);

    /* Sete Viewfinder and Recording settings */
    g_signal_emit_by_name (bin,
                           "user-res-fps",
                           vf_w, vf_h,
                           fps_n, fps_d,
                           0);

    /* Preview size will be the same as viewfinder size */
    g_digicam_manager_preview_enabled (manager, &enabled, NULL);
    if (enabled) {
        preview_caps = _new_preview_caps (vf_w, vf_h);
 	g_object_set (G_OBJECT (bin),
 		      "preview-caps", preview_caps,
 		      NULL);
    }


    TSTAMP (gst-after-res-changed);

    /* free */
free:
    if (NULL != bin) {
        gst_object_unref (bin);
    }
    if (NULL != preview_caps) {
        gst_caps_unref (preview_caps);
    }
    if (NULL != error) {
        g_error_free (error);
    }

    return result;
}


/**
 * _g_digicam_camerabin_set_locks:
 * @manager: A #GDigicamManager.
 * @user_data: A #GDigicamCamerabinLocksHelper helper data
 * structure.
 *
 * Implementation of "set_focus_mode" GDigicam operation
 * specifically for the "camerabin" GStreamer bin.
 *
 * Returns: FALSE if invalid input arguments are received or
 * the operation fails, it returns TRUE otherwise.
 **/
static gboolean
_g_digicam_camerabin_set_locks (GDigicamManager *manager,
                                gpointer user_data)
{
    GDigicamCamerabinLocksHelper *helper = NULL;
    GstElement *bin = NULL;
    GError *error = NULL;
    gboolean result;

    helper = (GDigicamCamerabinLocksHelper *) user_data;

    /* Get "camerabin" Gstreamer bin  */
    result = g_digicam_manager_get_gstreamer_bin (manager,
                                                  &bin,
                                                  &error);

    /* Check errors */
    if (!result) {
        if (NULL != error) {
            G_DIGICAM_DEBUG ("GDigicamCamerabin: %s", error->message);
        }
        goto free;
    }

    if (helper->locks & G_DIGICAM_LOCK_AUTOFOCUS) {
        TSTAMP (before-gst-af);
        G_DIGICAM_DEBUG ("GDigicamCamerabin: Emit autofocus to camerabin\n");
	gst_photography_set_autofocus (GST_PHOTOGRAPHY (bin), TRUE);
    } else {
        G_DIGICAM_DEBUG ("GDigicamCamerabin: Stop autofocus to camerabin\n");
	gst_photography_set_autofocus (GST_PHOTOGRAPHY (bin), FALSE);
    }

    /* free */
free:
    if (NULL != bin) {
        gst_object_unref (bin);
    }
    if (NULL != error) {
        g_error_free (error);
    }

    return result;
}


/**
 * _g_digicam_camerabin_set_zoom:
 * @manager: A #GDigicamManager.
 * @user_data: A #GDigicamCamerabinZoomHelper.
 *
 * Implementation of "set_zoom" GDigicam operation specifically
 * for the "camerabin" GStreamer bin.
 *
 * Returns: #FALSE if invalid input arguments are received or the
 * operation fails, #TRUE otherwise.
 **/
static gboolean
_g_digicam_camerabin_set_zoom (GDigicamManager *manager,
                               gpointer user_data)
{
    GDigicamCamerabinZoomHelper *helper = NULL;
    GstElement *bin = NULL;
    GError *error = NULL;
    gint value;
    gboolean result;

    helper = (GDigicamCamerabinZoomHelper *) user_data;

    G_DIGICAM_DEBUG ("GDigicamCamerabin: Setting new zoom value to %f\n",
                     helper->value);

    /* Get "camerabin" Gstreamer bin  */
    result = g_digicam_manager_get_gstreamer_bin (manager,
                                                  &bin,
                                                  &error);

    /* Check errors */
    if (!result) {
        if (NULL != error) {
            G_DIGICAM_DEBUG ("GDigicamCamerabin: %s", error->message);
        }
        goto free;
    }

    /* Establish new zoom value */
    /* FIXME: What about digital zoom */
    value = 100 * helper->value;
    g_object_set (bin, "zoom", value, NULL);

    /* free */
free:
    if (NULL != bin) {
        gst_object_unref (bin);
    }
    if (NULL != error) {
        g_error_free (error);
    }

    return result;
}


/**
 * _g_digicam_camerabin_set_audio:
 * @manager: A #GDigicamManager.
 * @user_data: A #GDigicamCamerabinVideoHelper.
 *
 * Implementation of "set_audio" GDigicam operation specifically for
 * the "camerabin" GStreamer bin.
 *
 * Returns: #FALSE if invalid input arguments are received or the
 * operation fails, #TRUE otherwise.
 **/
static gboolean
_g_digicam_camerabin_set_audio (GDigicamManager *manager,
                                gpointer user_data)
{
    GDigicamCamerabinVideoHelper *helper = NULL;
    GstElement *bin = NULL;
    gboolean audio_mute;
    GError *error = NULL;
    gboolean result;

    helper = (GDigicamCamerabinVideoHelper *) user_data;

    G_DIGICAM_DEBUG ("GDigicamCamerabin: Setting new mute value to %d\n",
                     helper->audio);

    /* Get "camerabin" Gstreamer bin  */
    result = g_digicam_manager_get_gstreamer_bin (manager,
                                                  &bin,
                                                  &error);

    /* Check errors */
    if (!result) {
        if (NULL != error) {
            G_DIGICAM_DEBUG ("GDigicamCamerabin: %s", error->message);
        }
        goto free;
    }

    /* Establish new mute value */
    if (helper->audio & G_DIGICAM_AUDIO_RECORDOFF) {
    	audio_mute = TRUE;
    } else {
    	audio_mute = FALSE;
    }

    g_object_set (bin, "mute", audio_mute, NULL);

    /* free */
free:
    if (NULL != bin) {
        gst_object_unref (bin);
    }
    if (NULL != error) {
        g_error_free (error);
    }

    return result;
}



/**
 * _g_digicam_camerabin_set_preview_mode:
 * @manager: A #GDigicamManager.
 * @user_data: A #GDigicamCamerabinAspectRatioResolutionHelper.
 *
 * Implementation of "set_preview_mode" GDigicam operation
 * specifically for the "camerabin" GStreamer bin.
 *
 * Returns: #FALSE if invalid input arguments are received or the
 * operation fails, #TRUE otherwise.
 **/
static gboolean
_g_digicam_camerabin_set_preview_mode (GDigicamManager *manager,
                                       gpointer user_data)
{
    GDigicamCamerabinPreviewHelper *helper = NULL;
    GstElement *bin = NULL;
    GstCaps *caps = NULL;
    GError *error = NULL;
    GDigicamMode mode;
    GDigicamAspectratio ar;
    GDigicamResolution res;
    gint vf_w, vf_h;
    gint res_w, res_h;
    gint fps_n, fps_d;
    gboolean result;

    helper = (GDigicamCamerabinPreviewHelper *) user_data;

    G_DIGICAM_DEBUG ("GDigicamCamerabin: Setting new preview value to %d\n",
                     helper->mode);

    /* Get "camerabin" Gstreamer bin  */
    result = g_digicam_manager_get_gstreamer_bin (manager,
                                                  &bin,
                                                  &error);
    /* Check errors */
    if (!result) {
        if (NULL != error) {
            G_DIGICAM_DEBUG ("GDigicamCamerabin: %s", error->message);
        }
        goto free;
    }

    /* Get operation mode */
    result = g_digicam_manager_get_mode (manager,
                                         &mode,
                                         &error);
    /* Check errors */
    if (!result) {
        if (NULL != error) {
            G_DIGICAM_DEBUG ("GDigicamCamerabin: %s", error->message);
        }
        goto free;
    }

    /* Get resoluton */
    result = g_digicam_manager_get_resolution (manager, &res, &error);

    /* Check errors */
    if (!result) {
        if (NULL != error) {
            G_DIGICAM_DEBUG ("GDigicamCamerabin: %s", error->message);
        }
        goto free;
    }

    /* Get aspect ratio  */
    result = g_digicam_manager_get_aspect_ratio (manager, &ar, &error);

    /* Check errors */
    if (!result) {
        if (NULL != error) {
            G_DIGICAM_DEBUG ("GDigicamCamerabin: %s", error->message);
        }
        goto free;
    }

    /* Get resolution specific values depending on the camera mode */
    _get_aspect_ratio_and_resolution (mode,
                                      ar, res,
                                      &vf_w, &vf_h,
                                      &res_w, &res_h,
                                      &fps_n, &fps_d);

    /* Establish new preview mode value */
    if (helper->mode & G_DIGICAM_PREVIEW_ON) {
        caps = _new_preview_caps (vf_w, vf_h);
	g_object_set (G_OBJECT (bin),
		      "preview-caps", caps,
		      NULL);
    } else if (helper->mode & G_DIGICAM_PREVIEW_OFF) {
	g_object_set (G_OBJECT (bin),
		      "preview-caps", NULL,
		      NULL);
    } else {
        G_DIGICAM_ERR ("GDigicamCamerabin: invalid preview mode %d received.",
                       helper->mode);
        goto free;
    }

    /* free */
free:
    if (NULL != bin) {
        gst_object_unref (bin);
    }
    if (NULL != caps) {
        gst_caps_unref (caps);
    }
    if (NULL != error) {
        g_error_free (error);
    }

    return result;
}

/**
 * _g_digicam_camerabin_get_still_picture:
 * @manager: A #GDigicamManager.
 * @user_data: A #GDigicamCamerabinPictureHelper.
 *
 * Implementation of "get_still_picture" GDigicam operation
 * specifically for the "camerabin" GStreamer bin.
 *
 * Returns: #FALSE if invalid input arguments are received or the
 * operation fails, #TRUE otherwise.
 **/
static gboolean
_g_digicam_camerabin_get_still_picture (GDigicamManager *manager,
                                        gpointer user_data)
{
    GDigicamCamerabinPictureHelper *helper = NULL;
    GstElement *bin = NULL;
    GError *error = NULL;
    gboolean result;

    helper = (GDigicamCamerabinPictureHelper *) user_data;

    G_DIGICAM_DEBUG ("GDigicamCamerabin: Capturing still picture ...");

    /* Get "camerabin" Gstreamer bin  */
    result = g_digicam_manager_get_gstreamer_bin (manager,
                                                  &bin,
                                                  &error);

    /* Check errors */
    if (!result) {
        if (NULL != error) {
            G_DIGICAM_DEBUG ("GDigicamCamerabin: %s", error->message);
        }
        goto free;
    }


    /* Set application domain metadata */
    _g_digicam_camerabin_set_picture_metadata (bin, helper->metadata);


    /* take picture */
    g_object_set (bin, "filename", helper->file_path, NULL);
    TSTAMP (before-gst-capture);
    g_signal_emit_by_name (bin, "user-start", 0);
    g_signal_emit_by_name (bin, "user-stop", 0);
    TSTAMP (after-gst-capture);

    /* free */
free:
    if (NULL != bin) {
        gst_object_unref (bin);
    }
    if (NULL != error) {
        g_error_free (error);
    }

    return result;
}


/**
 * _gdigicam_camerabin_start_recording_video:
 * @manager: A #GDigicamManager.
 * @user_data: A #GDigicamCamerabinVideoHelper.
 *
 * Implementation of "start_recording_video" GDigicam operation
 * specifically for the "camerabin" GStreamer bin.
 *
 * Returns: #FALSE if invalid input arguments are received or the
 * operation fails, #TRUE otherwise.
 **/
static gboolean
_g_digicam_camerabin_start_recording_video (GDigicamManager *manager,
                                            gpointer user_data)
{
    GDigicamCamerabinVideoHelper *helper = NULL;
    GstElement *bin = NULL;
    GError *error = NULL;
    gboolean result;

    helper = (GDigicamCamerabinVideoHelper *) user_data;

    G_DIGICAM_DEBUG ("GDigicamCamerabin: Capturing video ...");

    /* Get "camerabin" Gstreamer bin  */
    result = g_digicam_manager_get_gstreamer_bin (manager,
                                                  &bin,
                                                  &error);

    /* Check errors */
    if (!result) {
        if (NULL != error) {
            G_DIGICAM_DEBUG ("GDigicamCamerabin: %s", error->message);
        }
        goto free;
    }


    /* Set application domain metadata */
    _g_digicam_camerabin_set_video_metadata (bin, helper->metadata);


    /* Start recording mode */
    g_object_set (bin, "filename", helper->file_path, NULL);
    TSTAMP (before-gst-video-capture);
    g_signal_emit_by_name(bin, "user-start", 0);
    TSTAMP (after-gst-video-capture);

    /* free */
free:
    if (NULL != bin) {
        gst_object_unref (bin);
    }
    if (NULL != error) {
        g_error_free (error);
    }

    return TRUE;
}


/**
 * _g_digicam_camerabin_pause_recording_video:
 * @manager: A #GDigicamManager.
 * @user_data: A #GDigicamCamerabinVideoHelper.
 *
 * Implementation of "pause_recording_video" GDigicam operation
 * specifically for the "camerabin" GStreamer bin.
 *
 * Returns: #FALSE if invalid input arguments are received or the
 * operation fails, #TRUE otherwise.
 **/
static gboolean
_g_digicam_camerabin_pause_recording_video (GDigicamManager *manager,
                                            gpointer user_data)
{
    GDigicamCamerabinVideoHelper *helper = NULL;
    GstElement *bin = NULL;
    GError *error = NULL;
    gboolean result;

    helper = (GDigicamCamerabinVideoHelper *) user_data;

    G_DIGICAM_DEBUG ("GDigicamCamerabin: Recording operation paused \n");

    /* Get "camerabin" Gstreamer bin  */
    result = g_digicam_manager_get_gstreamer_bin (manager,
                                                  &bin,
                                                  &error);

    /* Check errors */
    if (!result) {
        if (NULL != error) {
            G_DIGICAM_DEBUG ("GDigicamCamerabin: %s", error->message);
        }
        goto free;
    }

    /* Pause/Resume recording mode */
    if (helper->resume) {
        g_signal_emit_by_name (bin, "user-start", 0);
    } else {
        g_signal_emit_by_name (bin, "user-pause", 0);
    }

    /* free */
free:
    if (NULL != bin) {
        gst_object_unref (bin);
    }
    if (NULL != error) {
        g_error_free (error);
    }

    return result;
}


/**
 * _g_digicam_camerabin_finish_recording_video:
 * @manager: A #GDigicamManager.
 * @user_data: A #GDigicamCamerabinVideoHelper.
 *
 * Implementation of "finish_recording_video" GDigicam operation
 * specifically for the "camerabin" GStreamer bin.
 *
 * Returns: #FALSE if invalid input arguments are received or the
 * operation fails, #TRUE otherwise.
 **/
static gboolean
_g_digicam_camerabin_finish_recording_video (GDigicamManager *manager,
                                             gpointer user_data)
{
    GDigicamCamerabinVideoHelper *helper = NULL;
    GstElement *bin = NULL;
    GError *error = NULL;
    gboolean result;

    helper = (GDigicamCamerabinVideoHelper *) user_data;

    G_DIGICAM_DEBUG ("GDigicamCamerabin: Recording operation stopped \n");

    /* Get "camerabin" Gstreamer bin  */
    result = g_digicam_manager_get_gstreamer_bin (manager,
                                                  &bin,
                                                  &error);

    /* Check errors */
    if (!result) {
        if (NULL != error) {
            G_DIGICAM_DEBUG ("GDigicamCamerabin: %s", error->message);
        }
        goto free;
    }

    /* Stopping recording mode */
    g_signal_emit_by_name (bin, "user-stop", 0);
    TSTAMP (after-gst-capture-stop);

    /* free */
free:
    if (NULL != bin) {
        gst_object_unref (bin);
    }
    if (NULL != error) {
        g_error_free (error);
    }

    return result;
}


/**
 * _g_digicam_camerabin_set_picture_metadata:
 * @gst_camera_bin: A camerabin #GstElement.
 * @metadata: A #GDigicamCamerabinMetadata.
 *
 * Sets the metadata in pictures.
 **/
static void
_g_digicam_camerabin_set_picture_metadata (GstElement *gst_camera_bin,
                                           const GDigicamCamerabinMetadata *metadata)
{
    /* for more information about image metadata tags, see:
     * http://webcvs.freedesktop.org/gstreamer/gst-plugins-bad/tests/icles/metadata_editor.c
     * and for the mapping:
     * http://webcvs.freedesktop.org/gstreamer/gst-plugins-bad/ext/metadata/metadata_mapping.htm?view=co
     */

    /* TODO: These location tags in quotes have to be in sync with
     * other tags after GStreamer's headers are updated.
     * See NB#125831 */

    GstTagSetter *setter = NULL;
    GstTagList *list = NULL;
    GTimeVal time = { 0,0 };
    gchar *date_str = NULL;

    g_return_if_fail (GST_IS_ELEMENT (gst_camera_bin));
    g_return_if_fail (NULL != metadata);
    setter = GST_TAG_SETTER (gst_camera_bin);

    /* Modified time */
    g_get_current_time(&time);
    date_str = g_time_val_to_iso8601 (&time); /* this is UTC */

    ULOG_DEBUG ("GDigicamCamerabin::_g_digicam_camerabin_set_picture_metadata: "
                "Writing Picture Metadata: \n"
                "\n\tdate: %s"
                "\n\tmake: %s"
                "\n\tmodel: %s"
                "\n\tauthor: %s"
                "\n\tcountry: %s"
                "\n\tcity: %s"
                "\n\tsuburb: %s"
                "\n\taltitude: %f"
                "\n\tlatitude: %f"
                "\n\tlongtitude: %f"
                "\n\torientation: %d",
                date_str, metadata->make,
                metadata->model,
                metadata->author,
                metadata->country_name,
                metadata->city_name,
                metadata->suburb_name,
                metadata->altitude,
                metadata->latitude,
                metadata->longitude,
                metadata->orientation);

    /* Creating the tag list with the mandatory tags. */
    list = gst_tag_list_new ();
    gst_tag_list_add (list, GST_TAG_MERGE_APPEND,
                      GST_TAG_DATE_TIME_ORIGINAL, date_str,
                      GST_TAG_DATE_TIME_MODIFIED, date_str,
                      GST_TAG_DEVICE_MAKE, metadata->make,
                      GST_TAG_DEVICE_MODEL, metadata->model,
                      GST_TAG_CAPTURE_ORIENTATION, metadata->orientation,
                      NULL);

    /* Adding coordinates, just if set. */
    if (G_MAXDOUBLE != metadata->latitude &&
        G_MAXDOUBLE != metadata->longitude) {
        gst_tag_list_add (list, GST_TAG_MERGE_APPEND,
                          GST_TAG_GEO_LOCATION_LATITUDE, metadata->latitude,
                          GST_TAG_GEO_LOCATION_LONGITUDE, metadata->longitude,
                          NULL);
        if (G_MAXDOUBLE != metadata->altitude) {
            gst_tag_list_add (list, GST_TAG_MERGE_APPEND,
                              GST_TAG_GEO_LOCATION_ELEVATION, metadata->altitude,
                              NULL);
        }
    }

    /* Adding optional metadata tags. */
    if (NULL != metadata->author) {
        gst_tag_list_add (list, GST_TAG_MERGE_APPEND,
                          GST_TAG_COMPOSER, metadata->author,
                          NULL);
    }

    /* We should have all the geotags. */
    if (NULL != metadata->country_name) {
        gst_tag_list_add (list, GST_TAG_MERGE_APPEND,
                          GST_TAG_GEO_LOCATION_COUNTRY, metadata->country_name,
                          NULL);
        if (NULL != metadata->city_name) {
                gst_tag_list_add (list, GST_TAG_MERGE_APPEND,
                                  GST_TAG_GEO_LOCATION_CITY, metadata->city_name,
                                  NULL);
            }
        if (NULL != metadata->suburb_name) {
            gst_tag_list_add (list, GST_TAG_MERGE_APPEND,
                              GST_TAG_GEO_LOCATION_SUBLOCATION, metadata->suburb_name,
                              NULL);
        }
    }

    /* Set metadata tags. */
    gst_tag_setter_merge_tags (setter, list,
                               GST_TAG_MERGE_REPLACE_ALL);

    /* Free. */
    gst_tag_list_free (list);
    g_free (date_str);
}


/**
 * _g_digicam_camerabin_set_video_metadata:
 * @gst_camera_bin: A camerabin #GstElement.
 * @metadata: A #GDigicamCamerabinMetadata.
 *
 * Sets the metadata in videos.
 **/
static void
_g_digicam_camerabin_set_video_metadata (GstElement *gst_camera_bin,
                                         const GDigicamCamerabinMetadata *metadata)
{
    /* for more information about image metadata tags, see:
     * http://webcvs.freedesktop.org/gstreamer/gst-plugins-bad/tests/icles/metadata_editor.c
     * and for the mapping:
     * http://webcvs.freedesktop.org/gstreamer/gst-plugins-bad/ext/metadata/metadata_mapping.htm?view=co
     */

    /* TODO: These location tags in quotes have to be in sync with
     * other tags after GStreamer's headers are updated.
     * See NB#125831 */

    GstTagSetter *setter = NULL;
    GstTagList *list = NULL;
    gchar *geo_name = NULL;
    gchar *tmp = NULL;
    /*we should not set Date if hantro is used as its automatically set by it.
    If we set date and time, hantro extract only year from it and erase the automatic date */
    /*TODO: Verfiy aumatice date set. */
    /*GTimeVal time = { 0,0 };
    gchar *date_str = NULL;*/

    g_return_if_fail (GST_IS_ELEMENT (gst_camera_bin));
    g_return_if_fail (NULL != metadata);
    setter = GST_TAG_SETTER (gst_camera_bin);

    /* Modified time */
    /*g_get_current_time(&time);
    date_str = g_time_val_to_iso8601 (&time);*/ /* this is UTC */

    ULOG_DEBUG ("GDigicamCamerabin::_g_digicam_camerabin_set_video_metadata: "
                "Writing Video Metadata: \n"
                "\n\tauthor: %s"
                "\n\tcountry: %s"
                "\n\tcity: %s"
                "\n\tsuburb: %s"
                "\n\taltitude: %f"
                "\n\tlatitude: %f"
                "\n\tlongtitude: %f",
                metadata->author,
                metadata->country_name,
                metadata->city_name,
                metadata->suburb_name,
                metadata->altitude,
                metadata->latitude,
                metadata->longitude);

    /* Creating the tag list with the mandatory tags. */
    list = gst_tag_list_new ();
    gst_tag_list_add (list, GST_TAG_MERGE_APPEND,
                      GST_TAG_CLASSIFICATION, metadata->unique_id,
                      NULL);

    /* Adding coordinates, just if set. */
    if (G_MAXDOUBLE != metadata->latitude &&
        G_MAXDOUBLE != metadata->longitude) {
        gst_tag_list_add (list, GST_TAG_MERGE_APPEND,
                          GST_TAG_GEO_LOCATION_LATITUDE, metadata->latitude,
                          GST_TAG_GEO_LOCATION_LONGITUDE, metadata->longitude,
                          NULL);
        if (G_MAXDOUBLE != metadata->altitude) {
            gst_tag_list_add (list, GST_TAG_MERGE_APPEND,
                              GST_TAG_GEO_LOCATION_ELEVATION, metadata->altitude,
                              NULL);
        }
    }

    /* Adding optional metadata tags. */
    if (NULL != metadata->author) {
        gst_tag_list_add (list, GST_TAG_MERGE_APPEND,
                          GST_TAG_ARTIST, metadata->author,
                          NULL);
    }

    /* We should have all the geotags. */
    if (NULL != metadata->country_name) {
        geo_name = g_strdup (metadata->country_name);
        if (NULL != metadata->city_name) {
            tmp = g_strconcat (geo_name,
                               ",",
                               metadata->city_name,
                               NULL);
            g_free (geo_name);
            geo_name = tmp;
            tmp = NULL;
        }
        if (NULL != metadata->suburb_name) {
            tmp = g_strconcat (geo_name,
                               ",",
                               metadata->suburb_name,
                               NULL);
            g_free (geo_name);
            geo_name = tmp;
            tmp = NULL;
        }

        gst_tag_list_add (list, GST_TAG_MERGE_APPEND,
                          GST_TAG_GEO_LOCATION_NAME, geo_name,
                          NULL);
    }

    /* Set metadata tags. */
    gst_tag_setter_merge_tags (setter, list,
                               GST_TAG_MERGE_REPLACE_ALL);

    /* Free. */
    gst_tag_list_free (list);
    g_free (geo_name);
    g_free (tmp);
}


/**
 * _g_digicam_camerabin_handle_bus_message:
 * @manager: A #GDigicamManager.
 * @user_data: A #GstMessage.
 *
 * Function to handle custom gstreamer bus messages.
 *
 * Returns: #FALSE if invalid input arguments are received or the
 * operation fails, #TRUE otherwise.
 **/
static gboolean
_g_digicam_camerabin_handle_bus_message (GDigicamManager *manager,
					 gpointer user_data)
{
    const GstStructure *structure = NULL;
    gint status = GST_PHOTOGRAPHY_FOCUS_STATUS_NONE;
    const gchar *message_name = NULL;
    GstState old = 0;
    GstState new = 0;
    GstState pending = 0;
    GError *error = NULL;
    GDigicamMode mode;
    GstElement *bin = NULL;
    gboolean result = FALSE;

    switch (GST_MESSAGE_TYPE (GST_MESSAGE (user_data))) {
    case GST_MESSAGE_STATE_CHANGED:
        /* Don't care if it is not coming from camerabin itself */
	result = g_digicam_manager_get_gstreamer_bin (manager,
                                                      &bin,
                                                      &error);

        /* Check errors */
        if (!result) {
            if (NULL != error) {
                G_DIGICAM_DEBUG ("GDigicamCamerabin::_g_digicam_camerabin_handle_bus_message: "
                                 "%s", error->message);
            }
            goto free;
        }

	if (GST_ELEMENT (GST_MESSAGE_SRC (GST_MESSAGE (user_data))) == bin) {
	    gst_message_parse_state_changed (GST_MESSAGE (user_data),
					     &old,
					     &new,
					     &pending);
	    if (GST_STATE_PLAYING == new) {
		result = g_digicam_manager_get_mode (manager,
                                                     &mode,
                                                     &error);

                /* Check errors */
                if (!result) {
                    if (NULL != error) {
                        G_DIGICAM_DEBUG ("GDigicamCamerabin::_g_digicam_camerabin_handle_bus_message: "
                                         "%s", error->message);
                    }
                    goto free;
                }

		switch (mode) {
		case G_DIGICAM_MODE_STILL:
		    g_object_set (bin, "mode", 0, NULL);
		    break;
		case G_DIGICAM_MODE_VIDEO:
		    g_object_set (bin, "mode", 1, NULL);
		    break;
		default:
		    g_assert_not_reached ();
		}
	    }
	}
	break;
    case GST_MESSAGE_ELEMENT:
	structure = gst_message_get_structure (GST_MESSAGE (user_data));
	message_name = gst_structure_get_name (structure);

	/* autofocus message */
	if (g_strcmp0 (message_name, GST_PHOTOGRAPHY_AUTOFOCUS_DONE) == 0) {
	    gst_structure_get_int (structure, "status", &status);
	    switch (status) {
	    case GST_PHOTOGRAPHY_FOCUS_STATUS_FAIL:
		G_DIGICAM_DEBUG ("GDigicamCamerabin::_g_digicam_camerabin_handle_bus_message: "
                                 "Autofocus failed message received.");
		g_signal_emit_by_name (manager,
				       "focus-done",
				       G_DIGICAM_FOCUSMODESTATUS_UNABLETOREACH);
		break;
	    case GST_PHOTOGRAPHY_FOCUS_STATUS_SUCCESS:
		G_DIGICAM_DEBUG ("GDigicamCamerabin::_g_digicam_camerabin_handle_bus_message: "
                                 "Autofocus success message received.");
		g_signal_emit_by_name (manager,
				       "focus-done",
				       G_DIGICAM_FOCUSMODESTATUS_REACHED);
		break;
	    case GST_PHOTOGRAPHY_FOCUS_STATUS_NONE:
		G_DIGICAM_DEBUG ("GDigicamCamerabin::_g_digicam_camerabin_handle_bus_message: "
                                 "Autofocus none message received.");
		break;
	    case GST_PHOTOGRAPHY_FOCUS_STATUS_RUNNING:
		G_DIGICAM_DEBUG ("GDigicamCamerabin::_g_digicam_camerabin_handle_bus_message: "
                                 "Autofocus running message received.");
		break;
	    default:
		break;
	    }
	    return TRUE;
	}

	/* Shake risk message */
	if (g_strcmp0 (message_name, GST_PHOTOGRAPHY_SHAKE_RISK) == 0) {
	    return TRUE;
	}

	break;
    default:
	/* Not handling this message */
	break;
    }

    /* Free */
free:
    if (NULL != bin) {
        gst_object_unref (bin);
    }
    if (NULL != error) {
        g_error_free (error);
    }

    return TRUE;
}

/**
 * _g_digicam_camerabin_handle_sync_bus_message:
 * @manager: A #GDigicamManager.
 * @user_data: A #GstMessage.
 *
 * Function to handle custom gstreamer sync bus messages.
 *
 * Returns: #FALSE if invalid input arguments are received or the
 * message is not processed, #TRUE otherwise.
 **/
static gboolean
_g_digicam_camerabin_handle_sync_bus_message (GDigicamManager *manager,
					      gpointer user_data)
{
    const GstStructure *structure = NULL;
    const gchar *message_name = NULL;
    const GValue *value = NULL;
    PreviewHelper *helper = NULL;
    GstBuffer *buff = NULL;
    GdkPixbuf *preview = NULL;
    gboolean alpha;
    GstElement *bin = NULL;
    GError *error = NULL;
    gboolean result = FALSE;
    gboolean success = FALSE;

    structure = gst_message_get_structure (GST_MESSAGE (user_data));
    g_return_val_if_fail (structure != NULL, FALSE);
    message_name = gst_structure_get_name (structure);

    /* Don't care if it is not coming from camerabin itself */
    success = g_digicam_manager_get_gstreamer_bin (manager,
                                                   &bin,
                                                   &error);

    /* Check errors */
    if (!success) {
        if (NULL != error) {
            G_DIGICAM_DEBUG ("GDigicamCamerabin::_g_digicam_camerabin_handle_bus_message: "
                             "%s", error->message);
        }
        goto free;
    }

    if (GST_ELEMENT (GST_MESSAGE_SRC (GST_MESSAGE (user_data))) == bin) {
        if (g_strcmp0 (message_name, G_DIGICAM_CAMERABIN_PHOTO_CAPTURE_END_MESSAGE) == 0) {
            G_DIGICAM_DEBUG ("GDigicamCamerabin::_g_digicam_camerabin_handle_sync_bus_message: "
                             "Capture end message received.");
            TSTAMP (after-gst-next-shot);

            /* Release lock and inform capture was completed */
            _g_digicam_manager_release_capture_lock (manager);

            /* Emit a signal in the main loop */
            g_idle_add (_emit_capture_end_signal, manager);

            result = TRUE;
            goto free;
        } else if (g_strcmp0 (message_name, G_DIGICAM_CAMERABIN_PHOTO_PREVIEW_MESSAGE) == 0) {
            G_DIGICAM_DEBUG ("GDigicamCamerabin::_g_digicam_camerabin_handle_sync_bus_message: "
                             "Image preview message received.");
            TSTAMP (after-gst-snapshot);
            value = gst_structure_get_value (structure, "buffer");
            buff = gst_value_get_buffer (value);
            alpha = FALSE;

            /* Preview using the RGB row data from GstBuffer */
            preview = _pixbuf_from_buffer (manager, buff, alpha);

            /* FIXME: shouldn't we send the signal even if we don't have any data? */
            /* Send the acquired preview */
            if (NULL != preview) {
                helper = g_slice_new0 (PreviewHelper);
                helper->mgr = manager;
                helper->preview = preview;
                g_idle_add (_emit_preview_signal, helper);
            }

            result = TRUE;
            goto free;
        } else {
            G_DIGICAM_DEBUG ("GDigicamCamerabin::_g_digicam_camerabin_handle_sync_bus_message: "
                             "Unhandled sync DBus message "
                             "coming from camerabin.");
        }
    } else {
        if (g_strcmp0 (message_name, G_DIGICAM_CAMERABIN_PHOTO_CAPTURE_START_MESSAGE) == 0) {
            G_DIGICAM_DEBUG ("GDigicamCamerabin::_g_digicam_camerabin_handle_sync_bus_message: "
                             "Capture start message received.");

            /* Set lock and inform capture was started */
            _g_digicam_manager_set_capture_lock (manager);

            /* Emit a signal in the main loop */
            g_idle_add (_emit_capture_start_signal, manager);

            result = TRUE;
            goto free;
        } else if (g_strcmp0 (message_name, G_DIGICAM_CAMERABIN_PHOTO_CAPTURE_PICTURE_GOT_MESSAGE) == 0) {
            G_DIGICAM_DEBUG ("GDigicamCamerabin::_g_digicam_camerabin_handle_sync_bus_message: "
                             "Picture got message received.");

            /* Emit a signal in the main loop */
            g_idle_add (_emit_picture_got_signal, manager);

            result = TRUE;
            goto free;
        } else {
            G_DIGICAM_DEBUG ("GDigicamCamerabin::_g_digicam_camerabin_handle_sync_bus_message: "
                             "Unhandled sync DBus message "
                             "not coming from camerabin.");
        }
    }

    /* Free */
free:
    if (NULL != bin) {
        gst_object_unref (bin);
    }
    if (NULL != error) {
        g_error_free (error);
    }

    return result;
}


/*********************************/
/* Private utility functions     */
/*********************************/


static void
_pixbuf_destroy (guchar *pixels, gpointer data)
{
  gst_buffer_unref (GST_BUFFER (data));
}


static GdkPixbuf *
_pixbuf_from_buffer (GDigicamManager *manager,
                     GstBuffer *buff,
                     gboolean has_alpha)
{
    GdkPixbuf *pix = NULL;
    GstVideoFormat fmt = GST_VIDEO_FORMAT_RGB;
    const guchar *data = NULL;
    GError *error = NULL;
    GDigicamMode mode;
    GDigicamAspectratio ar;
    GDigicamResolution res;
    gint vf_w, vf_h;
    gint res_w, res_h;
    gint fps_n, fps_d;
    gint minsize, buff_size, bytes_per_pixel;
    gint rowstride;
    gboolean result;


    /* Get mode to set specific resolution and aspect ratio*/
    result = g_digicam_manager_get_mode (manager,
                                         &mode,
                                         &error);
    /* Check errors */
    if (!result) {
        if (NULL != error) {
            G_DIGICAM_DEBUG ("GDigicamCamerabin: %s", error->message);
        }
        goto free;
    }

    /* Get resolution */
    result = g_digicam_manager_get_resolution (manager, &res, &error);

    /* Check errors */
    if (!result) {
        if (NULL != error) {
            G_DIGICAM_DEBUG ("GDigicamCamerabin: %s", error->message);
        }
        goto free;
    }

    /* Get aspect ratio  */
    result = g_digicam_manager_get_aspect_ratio (manager, &ar, &error);

    /* Check errors */
    if (!result) {
        if (NULL != error) {
            G_DIGICAM_DEBUG ("GDigicamCamerabin: %s", error->message);
        }
        goto free;
    }

    /* Get resolution specific values depending on the camera mode */
    _get_aspect_ratio_and_resolution (mode,
                                      ar, res,
                                      &vf_w, &vf_h,
                                      &res_w, &res_h,
                                      &fps_n, &fps_d);

    /* Build pixbuf */
    rowstride = gst_video_format_get_row_stride (fmt, 0, vf_w);
    if (has_alpha) {
        bytes_per_pixel = 4;
    } else {
        bytes_per_pixel = 3;
    }

    /* Last row needn't have row padding */
    minsize = (rowstride * (vf_h - 1)) + (bytes_per_pixel * vf_w);
    buff_size = GST_BUFFER_SIZE (buff);
    g_return_val_if_fail (buff_size >= minsize, NULL);

    /* Create pixbuf, ref it to keep data around as long as we use the
     * pixbuf */
    buff = gst_buffer_ref (buff);
    data = GST_BUFFER_DATA (buff);
    pix = gdk_pixbuf_new_from_data (data,
                                    GDK_COLORSPACE_RGB,
                                    has_alpha, 8, vf_w, vf_h,
                                    rowstride,
                                    _pixbuf_destroy, buff);

    G_DIGICAM_DEBUG ("GDigicamCamerabin: thumbail generated!!!");

free:
    return pix;
}


static gboolean
_emit_preview_signal (gpointer user_data)
{
    PreviewHelper *helper = NULL;

    helper = (PreviewHelper *) user_data;

    /* Emit image-preview signal */
    g_signal_emit_by_name (helper->mgr,
                           "image-preview", helper->preview,
                           0);

    /* Free */
    g_object_unref (helper->preview);
    g_slice_free (PreviewHelper, helper);

    return FALSE;
}

static gboolean
_emit_capture_start_signal (gpointer user_data)
{
    GDigicamManager *manager = NULL;

    manager = G_DIGICAM_MANAGER (user_data);

    /* Emit the capture-start signal */
    g_signal_emit_by_name (manager,
                           "capture-start",
                           0);

    return FALSE;
}

static gboolean
_emit_capture_end_signal (gpointer user_data)
{
    GDigicamManager *manager = NULL;

    manager = G_DIGICAM_MANAGER (user_data);

    /* Emit the capture-end signal */
    g_signal_emit_by_name (manager,
                           "capture-end",
                           0);

    return FALSE;
}


static gboolean
_emit_picture_got_signal (gpointer user_data)
{
    GDigicamManager *manager = NULL;

    manager = G_DIGICAM_MANAGER (user_data);

    /* Emit the picture-got signal */
    g_signal_emit_by_name (manager, "picture-got", 0);

    return FALSE;
}


static GstCaps *
_new_preview_caps (gint pre_w,
                   gint pre_h)
{
    GstCaps *caps = NULL;

    caps = gst_caps_new_simple ("video/x-raw-rgb",
                                "width", G_TYPE_INT, pre_w,
                                "height", G_TYPE_INT, pre_h,
                                "bpp", G_TYPE_INT, 24,
                                NULL);

    return caps;
}


static void
_get_aspect_ratio_and_resolution (GDigicamMode mode,
                                  GDigicamAspectratio ar,
                                  GDigicamResolution res,
                                  gint *vf_w, gint *vf_h,
                                  gint *res_w, gint *res_h,
                                  gint *fps_n, gint *fps_d)
{
    switch (mode) {
    case G_DIGICAM_MODE_STILL:
        _get_still_aspect_ratio_and_resolution (ar, res,
                                                vf_w, vf_h,
                                                res_w, res_h,
                                                fps_n, fps_d);
        break;
    case G_DIGICAM_MODE_VIDEO:
        _get_video_aspect_ratio_and_resolution (ar, res,
                                                vf_w, vf_h,
                                                res_w, res_h,
                                                fps_n, fps_d);
        break;
    default:
        g_assert_not_reached ();
    }
}


static void
_get_still_aspect_ratio_and_resolution (GDigicamAspectratio ar,
                                        GDigicamResolution res,
                                        gint *vf_w, gint *vf_h,
                                        gint *res_w, gint *res_h,
                                        gint *fps_n, gint *fps_d)
{
    switch (ar) {
    case G_DIGICAM_ASPECTRATIO_4X3:
        switch (res) {
        case G_DIGICAM_RESOLUTION_HIGH:
            *res_w = G_DIGICAM_CAMERABIN_STILL_4X3_HIGH_WIDTH;
            *res_h = G_DIGICAM_CAMERABIN_STILL_4X3_HIGH_HEIGHT;
            break;
        case G_DIGICAM_RESOLUTION_MEDIUM:
            *res_w = G_DIGICAM_CAMERABIN_STILL_4X3_MEDIUM_WIDTH;
            *res_h = G_DIGICAM_CAMERABIN_STILL_4X3_MEDIUM_HEIGHT;
            break;
        case G_DIGICAM_RESOLUTION_LOW:
            *res_w = G_DIGICAM_CAMERABIN_STILL_4X3_LOW_WIDTH;
            *res_h = G_DIGICAM_CAMERABIN_STILL_4X3_LOW_HEIGHT;
            break;
        default:
            g_assert_not_reached ();
        }
        *vf_w = G_DIGICAM_CAMERABIN_VIEWFINDER_4X3_WIDTH;
        *vf_h = G_DIGICAM_CAMERABIN_VIEWFINDER_4X3_HEIGHT;
        *fps_n = G_DIGICAM_CAMERABIN_VIEWFINDER_4X3_FPS;
        *fps_d = 100;
        break;
    case G_DIGICAM_ASPECTRATIO_16X9:
        switch (res) {
        case G_DIGICAM_RESOLUTION_HIGH:
            *res_w = G_DIGICAM_CAMERABIN_STILL_16X9_HIGH_WIDTH;
            *res_h = G_DIGICAM_CAMERABIN_STILL_16X9_HIGH_HEIGHT;
            break;
        case G_DIGICAM_RESOLUTION_MEDIUM:
            *res_w = G_DIGICAM_CAMERABIN_STILL_16X9_MEDIUM_WIDTH;
            *res_h = G_DIGICAM_CAMERABIN_STILL_16X9_MEDIUM_HEIGHT;
            break;
        case G_DIGICAM_RESOLUTION_LOW:
            *res_w = G_DIGICAM_CAMERABIN_STILL_16X9_LOW_WIDTH;
            *res_h = G_DIGICAM_CAMERABIN_STILL_16X9_LOW_HEIGHT;
            break;
        default:
            g_assert_not_reached ();
            }
        *vf_w = G_DIGICAM_CAMERABIN_VIEWFINDER_16X9_WIDTH;
        *vf_h = G_DIGICAM_CAMERABIN_VIEWFINDER_16X9_HEIGHT;
        *fps_n = G_DIGICAM_CAMERABIN_VIEWFINDER_16X9_FPS;
        *fps_d = 100;
        break;
    default:
        g_assert_not_reached ();
    }
}


static void
_get_video_aspect_ratio_and_resolution (GDigicamAspectratio ar,
                                        GDigicamResolution res,
                                        gint *vf_w, gint *vf_h,
                                        gint *res_w, gint *res_h,
                                        gint *fps_n, gint *fps_d)
{
    switch (ar) {
    case G_DIGICAM_ASPECTRATIO_4X3:
        switch (res) {
        case G_DIGICAM_RESOLUTION_HD:
            *vf_w = G_DIGICAM_CAMERABIN_VIDEO_4X3_HD_WIDTH;
            *vf_h = G_DIGICAM_CAMERABIN_VIDEO_4X3_HD_HEIGHT;
            break;
        case G_DIGICAM_RESOLUTION_DVD:
            *vf_w = G_DIGICAM_CAMERABIN_VIDEO_4X3_DVD_WIDTH;
            *vf_h = G_DIGICAM_CAMERABIN_VIDEO_4X3_DVD_HEIGHT;
            break;
        case G_DIGICAM_RESOLUTION_HIGH:
            *vf_w = G_DIGICAM_CAMERABIN_VIDEO_4X3_HIGH_WIDTH;
            *vf_h = G_DIGICAM_CAMERABIN_VIDEO_4X3_HIGH_HEIGHT;
            break;
        case G_DIGICAM_RESOLUTION_MEDIUM:
            *vf_w = G_DIGICAM_CAMERABIN_VIDEO_4X3_MEDIUM_WIDTH;
            *vf_h = G_DIGICAM_CAMERABIN_VIDEO_4X3_MEDIUM_HEIGHT;
            break;
        case G_DIGICAM_RESOLUTION_LOW:
            *vf_w = G_DIGICAM_CAMERABIN_VIDEO_4X3_LOW_WIDTH;
            *vf_h = G_DIGICAM_CAMERABIN_VIDEO_4X3_LOW_HEIGHT;
            break;
        default:
            g_assert_not_reached ();
        }
        if(res == G_DIGICAM_RESOLUTION_HD)
            *fps_n = G_DIGICAM_CAMERABIN_VIEWFINDER_4X3_HD_VIDEO_FPS
        else if(res == G_DIGICAM_RESOLUTION_DVD)
            *fps_n = G_DIGICAM_CAMERABIN_VIEWFINDER_4X3_DVD_VIDEO_FPS
        else
            *fps_n = G_DIGICAM_CAMERABIN_VIEWFINDER_4X3_FPS
        *fps_d = 100;
        break;
    case G_DIGICAM_ASPECTRATIO_16X9:
        switch (res) {
        case G_DIGICAM_RESOLUTION_HD:
            *vf_w = G_DIGICAM_CAMERABIN_VIDEO_16X9_HD_WIDTH;
            *vf_h = G_DIGICAM_CAMERABIN_VIDEO_16X9_HD_HEIGHT;
            break;
        case G_DIGICAM_RESOLUTION_DVD:
            *vf_w = G_DIGICAM_CAMERABIN_VIDEO_16X9_DVD_WIDTH;
            *vf_h = G_DIGICAM_CAMERABIN_VIDEO_16X9_DVD_HEIGHT;
            break;
        case G_DIGICAM_RESOLUTION_HIGH:
            *vf_w = G_DIGICAM_CAMERABIN_VIDEO_16X9_HIGH_WIDTH;
            *vf_h = G_DIGICAM_CAMERABIN_VIDEO_16X9_HIGH_HEIGHT;
            break;
        case G_DIGICAM_RESOLUTION_MEDIUM:
            *vf_w = G_DIGICAM_CAMERABIN_VIDEO_16X9_MEDIUM_WIDTH;
            *vf_h = G_DIGICAM_CAMERABIN_VIDEO_16X9_MEDIUM_HEIGHT;
            break;
        case G_DIGICAM_RESOLUTION_LOW:
            *vf_w = G_DIGICAM_CAMERABIN_VIDEO_16X9_LOW_WIDTH;
            *vf_h = G_DIGICAM_CAMERABIN_VIDEO_16X9_LOW_HEIGHT;
            break;
        default:
            g_assert_not_reached ();
        }
        if(res == G_DIGICAM_RESOLUTION_HD)
            *fps_n = G_DIGICAM_CAMERABIN_VIEWFINDER_16X9_HD_VIDEO_FPS
        else if(res == G_DIGICAM_RESOLUTION_DVD)
            *fps_n = G_DIGICAM_CAMERABIN_VIEWFINDER_16X9_DVD_VIDEO_FPS
        else
            *fps_n = G_DIGICAM_CAMERABIN_VIEWFINDER_16X9_VIDEO_FPS
        *fps_d = 100;
        break;
    default:
        g_assert_not_reached ();
    }
}
