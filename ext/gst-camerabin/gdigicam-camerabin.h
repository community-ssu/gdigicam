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

#ifndef __GDIGICAM_CAMERABIN_H__
#define __GDIGICAM_CAMERABIN_H__

#include <gst/gst.h>
#include <glib-object.h>

#include "gdigicam-manager.h"

#ifdef __cplusplus
extern "C" {
#endif

    G_BEGIN_DECLS


    /**
     * GDigicamCamerabinMetadata:
     * @model: Device model.
     * @make: Device maker.
     * @author: Content author.
     * @unique_id: Unique id for a device composed of device wlan mac.
     * @country_name: Country name given the current geolocation
     * coordinates.
     * @city_name: Town/city name given the current geolocation
     * coordinates.
     * @suburb_name: Suburb name given the current geolocation
     * coordinates.
     * @longitude: Geolocation's longitude parameter.
     * @latitude: Geolocation's latitude parameter.
     * @altitude: Geolocation's altitude parameter.
     * @orientation: Content orientation.
     *
     * Data structure with metadata information to be used during the
     * capture operation.
     */
    typedef struct  {
        gchar *model;
        gchar *make;
        gchar *author;
        gchar *unique_id;
        gchar *country_name;
        gchar *city_name;
        gchar *suburb_name;
        gdouble longitude;
        gdouble latitude;
        gdouble altitude;
        guint orientation;
    } GDigicamCamerabinMetadata;

    /*****************************************/
    /* Public "CAMERABIN" operation helpers */
    /*****************************************/

    /**
     * GDigicamCamerabinModeHelper:
     * @mode: A #GDigicamMode indicating the camera mode; still
     * picture or video.
     *
     * Data structure with helper data to be used during "set_mode"
     * operation for 'camerabin'.
     */
    typedef struct  {
        GDigicamMode mode;
    } GDigicamCamerabinModeHelper;

/**
 * GDigicamCamerabinFlashModeHelper:
 * @flash_mode: A #GDigicamFlashmode indicating the flash mode.
 *
 * Data structure with helper data to be used during "set_flash_mode"
 * operation for 'camerabin'.
 */
    typedef struct  {
        GDigicamFlashmode flash_mode;
    } GDigicamCamerabinFlashModeHelper;

/**
 * GDigicamCamerabinFocusModeHelper:
 * @focus_mode: A #GDigicamFocusmode indicating the focus mode.
 * @macro_enabled: Wether macro mode is enabled or not.
 *
 * Data structure with helper data to be used during "set_focus_mode"
 * operation for 'camerabin'.
 */
    typedef struct  {
        GDigicamFocusmode focus_mode;
        gboolean macro_enabled;
    } GDigicamCamerabinFocusModeHelper;

/**
 * GDigicamCamerabinFocusRegionPatternHelper:
 * @focus_points: A #GDigicamFocuspoints indicating the focus points
 * array shape.
 * @active_points: The active points in the focus points array.
 *
 * Data structure with helper data and functions to be used
 * during "set_focus_region_pattern" operation for 'camerabin'.
 */
    typedef struct  {
        GDigicamFocuspoints focus_points;
        guint64 active_points;
    } GDigicamCamerabinFocusRegionPatternHelper;

/**
 * GDigicamCamerabinExposureModeHelper:
 * @exposure_mode: A #GDigicamExposuremode indicating the exposure
 * mode.
 *
 * Data structure with helper data to be used during
 * "set_exposure_mode" operation for 'camerabin'.
 */
    typedef struct  {
        GDigicamExposuremode exposure_mode;
    } GDigicamCamerabinExposureModeHelper;

/**
 * GDigicamCamerabinExposureCompHelper:
 * @exposure_comp: The exposure compenastion level.
 *
 * Data structure with helper data to be used during
 * "set_exposure_comp" operation for 'camerabin'.
 */
    typedef struct  {
        gdouble exposure_comp;
    } GDigicamCamerabinExposureCompHelper;

/**
 * GDigicamCamerabinIsoSensitivityHelper
 * @iso_sensitivity_mode: A #GDigicamIsosensitivitymode indicating the
 * ISO mode.
 * @iso_value: The ISO level in case of auto mode.
 *
 * Data structure with helper data to be used during
 * "set_iso_sensitivity_mode" operation for 'camerabin'.
 */
    typedef struct {
        GDigicamIsosensitivitymode iso_sensitivity_mode;
        guint iso_value;
    } GDigicamCamerabinIsoSensitivityHelper;

/**
 * GDigicamCamerabinWhitebalanceModeHelper:
 * @wb_mode: A #GDigicamWhitebalanceMode indicating the white balance
 * mode.
 *
 * Data structure with helper data and functions to be used during
 * "set_white_balance_mode" operation for 'camerabin'.
 */
    typedef struct {
        GDigicamWhitebalancemode wb_mode;
    } GDigicamCamerabinWhitebalanceModeHelper;

/**
 * GDigicamCamerabinQualityHelper:
 * @quality: A #GDigicamQuality indicating the quality level.
 *
 * Data structure with helper data to be used during "set_quality"
 * operation for 'camerabin'.
 */
    typedef struct  {
        GDigicamQuality quality;
    } GDigicamCamerabinQualityHelper;

/**
 * GDigicamCamerabinAspectRatioResolutionHelper:
 * @aspect_ratio: A #GDigicamAspectratio indicating the aspect_ratio.
 * @resolution: A #GDigicamResolution indicating the resolution.
 * @preview_mode: A #GDigicamPreview indicating the preview mode.
 *
 * Data structure with helper data to be used during
 * "set_aspect_ratio" and "set_resolution" operation for 'camerabin'.
 */
    typedef struct  {
        GDigicamAspectratio aspect_ratio;
        GDigicamResolution resolution;
        GDigicamPreview preview_mode;
    } GDigicamCamerabinAspectRatioResolutionHelper;

/**
 * GDigicamCamerabinLocksHelper:
 * @locks: A #GDigicamLock indicating the locks.
 *
 * Data structure with helper data to be used during "set_locks"
 * operation for 'camerabin'.
 */
    typedef struct  {
        GDigicamLock locks;
    } GDigicamCamerabinLocksHelper;

/**
 * GDigicamCamerabinZoomHelper:
 * @value: The zoom value.
 *
 * Data structure with helper data to be used during "set_zoom"
 * operation for 'camerabin'.
 */
    typedef struct  {
        gdouble value;
    } GDigicamCamerabinZoomHelper;

/**
 * GDigicamCamerabinPictureHelper:
 * @file_path: Filename in which store the taken picture.
 * @metadata: A #GDigicamCamerabinMetadata with the metadata
 * information to add to the taken picture.
 *
 * Data structure with helper data and functions to be used
 * during "set_focus_region_pattern" operation for 'camerabin'.
 */
    typedef struct  {
        const gchar *file_path;
        GDigicamCamerabinMetadata *metadata;
    } GDigicamCamerabinPictureHelper;

/**
 * GDigicamCamerabinVideoHelper:
 * @file_path: Filename in which store the recorded video.
 * @metadata: A #GDigicamCamerabinMetadata with the metadata
 * information to add to the recorded video.
 * @resume: If used in pause/resume function, wether it is pause or
 * resume.
 * @audio: A #GDigicamAudio indicating the audio mode.
 *
 * Data structure with helper data and functions to be used
 * during "start/pause/stop_recording_video" operation for 'camerabin'.
 */
    typedef struct  {
        const gchar *file_path;
        GDigicamCamerabinMetadata *metadata;
        gboolean resume;
        GDigicamAudio audio;
    } GDigicamCamerabinVideoHelper;

/**
 * GDigicamCamerabinPreviewHelper:
 * @mode: A #GDigicamPreview indicating the preview mode.
 *
 * Data structure with helper data and functions to be used during the
 * preview related operations for 'camerabin'.
 */
    typedef struct  {
        GDigicamPreview mode;
    } GDigicamCamerabinPreviewHelper;

    /********************/
    /* Public functions */
    /********************/

    GDigicamDescriptor *g_digicam_camerabin_descriptor_new (const GstElement *gst_camera_bin);
    GstElement *g_digicam_camerabin_element_new (const gchar *videosrc,
                                                 const gchar *videoenc,
                                                 const gchar *videomux,
                                                 const gchar *audiosrc,
                                                 const gchar *audioenc,
                                                 const gchar *imageenc,
						 const gchar *imagepp,
                                                 const gchar *ximagesink,
                                                 gint *colorkey);

    G_END_DECLS

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* __GDIGICAM_CAMERABIN_H__ */
