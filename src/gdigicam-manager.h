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

#ifndef __G_DIGICAM_MANAGER_H__
#define __G_DIGICAM_MANAGER_H__

#include <glib-object.h>
#include <gst/gst.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#ifdef __cplusplus
extern "C" {
#endif

    G_BEGIN_DECLS

#define G_DIGICAM_TYPE_MANAGER _g_digicam_manager_get_type()

#define G_DIGICAM_MANAGER(obj)                                          \
    (G_TYPE_CHECK_INSTANCE_CAST((obj),                                  \
                                G_DIGICAM_TYPE_MANAGER, GDigicamManager))

#define G_DIGICAM_MANAGER_CLASS(klass)                                  \
    (G_TYPE_CHECK_CLASS_CAST((klass),                                   \
                             G_DIGICAM_TYPE_MANAGER, GDigicamManagerClass))

#define G_DIGICAM_IS_MANAGER(obj)                               \
    (G_TYPE_CHECK_INSTANCE_TYPE((obj),                          \
                                G_DIGICAM_TYPE_MANAGER))

#define G_DIGICAM_IS_MANAGER_CLASS(klass)               \
    (G_TYPE_CHECK_CLASS_TYPE((klass),                   \
                             G_DIGICAM_TYPE_MANAGER))

#define G_DIGICAM_MANAGER_GET_CLASS(obj)                                \
    (G_TYPE_INSTANCE_GET_CLASS ((obj),                                  \
                                G_DIGICAM_TYPE_MANAGER, GDigicamManagerClass))


    typedef struct _GDigicamDescriptor GDigicamDescriptor;
    typedef struct _GDigicamFocuspointposition GDigicamFocuspointposition;
    typedef struct _GDigicamManager GDigicamManager;
    typedef struct _GDigicamManagerClass GDigicamManagerClass;

    /**
     * GDigicamManagerFunc:
     * @manager: A #GDigicamManager
     * @user_data: User data.
     *
     * This is a generic function to be used with every _set public
     * function in GDigicam.
     *
     * Returns: TRUE if succesful, FALSE otherwise.
     **/
    typedef gboolean (*GDigicamManagerFunc) (GDigicamManager *manager,
                                             gpointer user_data);

/* This G_DIGICAM_CAPABILITIES can not be done with GFlagsValue,
 * because its size is longer than a guint -> 0xFFFF */

/**
 * G_DIGICAM_CAPABILITIES_VIEWFINDER:
 *
 * Flag to set in the supported_modes field of a #GDigicamDescriptor
 * meaning that the digicam like #GstElement it owns to has viewfinder
 * capabilities.
 */
#define G_DIGICAM_CAPABILITIES_VIEWFINDER           ((guint32) 0x00000001)

/**
 * G_DIGICAM_CAPABILITIES_FLASH:
 *
 * Flag to set in the supported_modes field of a #GDigicamDescriptor
 * meaning that the digicam like #GstElement it owns to has flash
 * capabilities.
 */
#define G_DIGICAM_CAPABILITIES_FLASH                ((guint32) 0x00000002)

/**
 * G_DIGICAM_CAPABILITIES_AUTOFOCUS:
 *
 * Flag to set in the supported_modes field of a #GDigicamDescriptor
 * meaning that the digicam like #GstElement it owns to has auto focus
 * capabilities.
 */
#define G_DIGICAM_CAPABILITIES_AUTOFOCUS                ((guint32) 0x00000004)

/**
 * G_DIGICAM_CAPABILITIES_CONTINUOUSAUTOFOCUS:
 *
 * Flag to set in the supported_modes field of a #GDigicamDescriptor
 * meaning that the digicam like #GstElement it owns to has continous
 * auto focus capabilities.
 */
#define G_DIGICAM_CAPABILITIES_CONTINUOUSAUTOFOCUS                ((guint32) 0x00000008)

/**
 * G_DIGICAM_CAPABILITIES_MANUALFOCUS:
 *
 * Flag to set in the supported_modes field of a #GDigicamDescriptor
 * meaning that the digicam like #GstElement it owns to has manual
 * focus capabilities.
 */
#define G_DIGICAM_CAPABILITIES_MANUALFOCUS            ((guint32) 0x00000010)

/**
 * G_DIGICAM_CAPABILITIES_MACROFOCUS:
 *
 * Flag to set in the supported_modes field of a #GDigicamDescriptor
 * meaning that the digicam like #GstElement it owns to has macro
 * focus capabilities.
 */
#define G_DIGICAM_CAPABILITIES_MACROFOCUS  ((guint32) 0x00000020)

/**
 * G_DIGICAM_CAPABILITIES_AUTOEXPOSURE:
 *
 * Flag to set in the supported_modes field of a #GDigicamDescriptor
 * meaning that the digicam like #GstElement it owns to has auto exposure
 * capabilities.
 */
#define G_DIGICAM_CAPABILITIES_AUTOEXPOSURE          ((guint32) 0x00000040)

/**
 * G_DIGICAM_CAPABILITIES_MANUALEXPOSURE:
 *
 * Flag to set in the supported_modes field of a #GDigicamDescriptor
 * meaning that the digicam like #GstElement it owns to has manual
 * exposure capabilities.
 */
#define G_DIGICAM_CAPABILITIES_MANUALEXPOSURE           ((guint32) 0x00000080)

/**
 * G_DIGICAM_CAPABILITIES_AUTOISOSENSITIVITY:
 *
 * Flag to set in the supported_modes field of a #GDigicamDescriptor
 * meaning that the digicam like #GstElement it owns to has automatic
 * iso sensitivity capabilities.
 */
#define G_DIGICAM_CAPABILITIES_AUTOISOSENSITIVITY         ((guint32) 0x00000100)

/**
 * G_DIGICAM_CAPABILITIES_MANUALISOSENSITIVITY:
 *
 * Flag to set in the supported_modes field of a #GDigicamDescriptor
 * meaning that the digicam like #GstElement it owns to has manual
 * iso sensitivity capabilities.
 */
#define G_DIGICAM_CAPABILITIES_MANUALISOSENSITIVITY       ((guint32) 0x00000200)

/* #define G_DIGICAM_CAPABILITIES_AUTOAPERTURE   ((guint32) 0x00000400) */

/* #define G_DIGICAM_CAPABILITIES_MANUALAPERTURE ((guint32) 0x00000800) */

/* #define G_DIGICAM_CAPABILITIES_AUTOSHUTTERSPEED         ((guint32) 0x00001000) */

/* #define G_DIGICAM_CAPABILITIES_MANUALSHUTTERSPEED       ((guint32) 0x00002000) */

/**
 * G_DIGICAM_CAPABILITIES_AUTOWHITEBALANCE:
 *
 * Flag to set in the supported_modes field of a #GDigicamDescriptor
 * meaning that the digicam like #GstElement it owns to has automatic
 * white balance capabilities.
 */
#define G_DIGICAM_CAPABILITIES_AUTOWHITEBALANCE     ((guint32) 0x00004000)

/**
 * G_DIGICAM_CAPABILITIES_MANUALWHITEBALANCE:
 *
 * Flag to set in the supported_modes field of a #GDigicamDescriptor
 * meaning that the digicam like #GstElement it owns to has manual
 * white balance capabilities.
 */
#define G_DIGICAM_CAPABILITIES_MANUALWHITEBALANCE   ((guint32) 0x00008000)

/**
 * G_DIGICAM_CAPABILITIES_METERING:
 *
 * Flag to set in the metering_modes field of a #GDigicamDescriptor
 * meaning that the digicam like #GstElement it owns to has metering
 * mode capabilities.
 */
#define G_DIGICAM_CAPABILITIES_METERING     ((guint32) 0x00010000)

/**
 * G_DIGICAM_CAPABILITIES_ASPECTRATIO:
 *
 * Flag to set in the supported_modes field of a #GDigicamDescriptor
 * meaning that the digicam like #GstElement it owns to has aspect
 * ratio capabilities.
 */
#define G_DIGICAM_CAPABILITIES_ASPECTRATIO   ((guint32) 0x00020000)

/* #define G_DIGICAM_CAPABILITIES_COLORFILTER             ((guint32) 0x00040000) */

/**
 * G_DIGICAM_CAPABILITIES_QUALITY:
 *
 * Flag to set in the supported_qualities field of a #GDigicamDescriptor
 * meaning that the digicam like #GstElement it owns to have the quality
 * capabilities.
 */
#define G_DIGICAM_CAPABILITIES_QUALITY          ((guint32) 0x00080000)

/**
 * G_DIGICAM_CAPABILITIES_RESOLUTION:
 *
 * Flag to set in the supported_modes field of a #GDigicamDescriptor
 * meaning that the digicam like #GstElement it owns to has resolution
 * capabilities.
 */
#define G_DIGICAM_CAPABILITIES_RESOLUTION          ((guint32) 0x00100000)


/**
 * G_DIGICAM_CAPABILITIES_OPTICALZOOM:
 *
 * Flag to set in the supported_modes field of a #GDigicamDescriptor
 * meaning that the digicam like #GstElement it owns to has optical
 * zoom capability.
 */
#define G_DIGICAM_CAPABILITIES_OPTICALZOOM              ((guint32) 0x00200000)


/**
 * G_DIGICAM_CAPABILITIES_DIGITALZOOM:
 *
 * Flag to set in the supported_modes field of a #GDigicamDescriptor
 * meaning that the digicam like #GstElement it owns to has digital
 * zoom capability.
 */
#define G_DIGICAM_CAPABILITIES_DIGITALZOOM           ((guint32) 0x00400000)

/* #define G_DIGICAM_CAPABILITIES_BRIGHTNESS          ((guint32) 0x00800000) */

/* #define G_DIGICAM_CAPABILITIES_CONTRAST          ((guint32) 0x01000000) */

/* #define G_DIGICAM_CAPABILITIES_GAMMA           ((guint32) 0x02000000) */

/* #define G_DIGICAM_CAPABILITIES_VIDEOSTABILIZATION             ((guint32) 0x04000000) */


/**
 * G_DIGICAM_CAPABILITIES_AUDIO:
 *
 * Flag to set in the supported_modes field of a #GDigicamDescriptor
 * meaning that the digicam like #GstElement it owns to has audio
 * capabilities.
 */
#define G_DIGICAM_CAPABILITIES_AUDIO           ((guint32) 0x00800000)


/**
 * G_DIGICAM_CAPABILITIES_PREVIEW:
 *
 * Flag to set in the supported_modes field of a #GDigicamDescriptor
 * meaning that the digicam like #GstElement it owns to has preview
 * capabilities.
 */
#define G_DIGICAM_CAPABILITIES_PREVIEW         ((guint32) 0x01000000)


/**
 * G_DIGICAM_CAPABILITIES_VIEWFINDER_NAME:
 *
 * The name of the viewfinder #GstElement.
 */
#define G_DIGICAM_CAPABILITIES_VIEWFINDER_NAME      "viewfinder"

/* To be removed? */
/* #define G_DIGICAM_CAPABILITIES_PHOTO_NAME           "photo" */

    /**
     * GDigicamMode:
     * @G_DIGICAM_MODE_NONE: No mode set.
     * @G_DIGICAM_MODE_STILL: Mode set to still picture.
     * @G_DIGICAM_MODE_VIDEO: Mode set to video.
     * @G_DIGICAM_MODE_N: Ceiling and number of modes.
     *
     * Indicates the camera mode; still picture or video.
     */
    typedef enum {
        G_DIGICAM_MODE_NONE  = 0,

        G_DIGICAM_MODE_STILL = 1 << 0,
        G_DIGICAM_MODE_VIDEO = 1 << 1,

        G_DIGICAM_MODE_N     = (1 << 1)+1
    } GDigicamMode;


    /**
     * GDigicamFlashmode:
     * @G_DIGICAM_FLASHMODE_NONE: No flash mode set.
     * @G_DIGICAM_FLASHMODE_OFF: Flash mode is off, so it won't ever be
     * triggered.
     * @G_DIGICAM_FLASHMODE_ON: Flash mode is on, so it will always be
     * triggered.
     * @G_DIGICAM_FLASHMODE_AUTO: Flash mode set tu auto. This means
     * flash will be triggered depending on the light level.
     * @G_DIGICAM_FLASHMODE_REDEYEREDUCTION: Flash mode set to red eyes
     * reduction, so it will always be triggered in this mode.
     * @G_DIGICAM_FLASHMODE_REDEYEREDUCTIONAUTO: Flash mode set to do
     * red eyes reduction automatically when it is needed.
     * @G_DIGICAM_FLASHMODE_FILLIN: Flash mode set to fill-in dimly lit
     * areas.
     * @G_DIGICAM_FLASHMODE_N: Ceiling and number of flash modes.
     *
     * Indicates the way in which the digicam flash will behave.
     */
    typedef enum {
        G_DIGICAM_FLASHMODE_NONE                = 0,

        G_DIGICAM_FLASHMODE_OFF                 = 1 << 0,
        G_DIGICAM_FLASHMODE_ON                  = 1 << 1,
        G_DIGICAM_FLASHMODE_AUTO                = 1 << 2,
        G_DIGICAM_FLASHMODE_REDEYEREDUCTION     = 1 << 3,
        G_DIGICAM_FLASHMODE_REDEYEREDUCTIONAUTO = 1 << 4,
        G_DIGICAM_FLASHMODE_FILLIN              = 1 << 5,

        G_DIGICAM_FLASHMODE_N                   = (1 << 5)+1
    } GDigicamFlashmode;


    /**
     * GDigicamFocusmode:
     * @G_DIGICAM_FOCUSMODE_NONE: No focus mode set.
     * @G_DIGICAM_FOCUSMODE_MANUAL: Focus mode set to manual.
     * @G_DIGICAM_FOCUSMODE_AUTO: Focus mode set to one-shot auto.
     * @G_DIGICAM_FOCUSMODE_FACE: Focus mode set to face auto recognition.
     * @G_DIGICAM_FOCUSMODE_SMILE: Focus mode set to smile auto recognition.
     * @G_DIGICAM_FOCUSMODE_CENTROID: Focus mode set to centroid one-shot
     * auto.
     * @G_DIGICAM_FOCUSMODE_CONTINUOUSAUTO: Focus mode set to continous
     * centroid auto.
     * @G_DIGICAM_FOCUSMODE_CONTINUOUSCENTROID: Focus mode set to
     * continuous centroid.
     * @G_DIGICAM_FOCUSMODE_N: Ceiling and number of focus modes.
     *
     * Indicates the way in which the digicam focus mode will behave.
     */
    typedef enum {
        G_DIGICAM_FOCUSMODE_NONE               = 0,

        G_DIGICAM_FOCUSMODE_MANUAL             = 1 << 0,
        G_DIGICAM_FOCUSMODE_AUTO               = 1 << 1,
        G_DIGICAM_FOCUSMODE_FACE               = 1 << 2,
        G_DIGICAM_FOCUSMODE_SMILE              = 1 << 3,
        G_DIGICAM_FOCUSMODE_CENTROID           = 1 << 4,
        G_DIGICAM_FOCUSMODE_CONTINUOUSAUTO     = 1 << 5,
        G_DIGICAM_FOCUSMODE_CONTINUOUSCENTROID = 1 << 6,

        G_DIGICAM_FOCUSMODE_N                  = (1 << 6)+1
    } GDigicamFocusmode;


    /**
     * GDigicamExposuremode:
     * @G_DIGICAM_EXPOSUREMODE_NONE: No exposure mode set.
     * @G_DIGICAM_EXPOSUREMODE_MANUAL: Exposure mode set to manual.
     * @G_DIGICAM_EXPOSUREMODE_AUTO: Exposure mode set to auto.
     * @G_DIGICAM_EXPOSUREMODE_NIGHT: Exposure mode set to night.
     * @G_DIGICAM_EXPOSUREMODE_BACKLIGHT: Exposure mode set to backlight.
     * @G_DIGICAM_EXPOSUREMODE_SPOTLIGHT: Exposure mode set to spotlight.
     * @G_DIGICAM_EXPOSUREMODE_SPORTS: Exposure mode set to sports.
     * @G_DIGICAM_EXPOSUREMODE_SNOW: Exposure mode set to snow.
     * @G_DIGICAM_EXPOSUREMODE_BEACH: Exposure mode set to beach.
     * @G_DIGICAM_EXPOSUREMODE_LARGEAPERTURE: Exposure mode set to large
     * aperture.
     * @G_DIGICAM_EXPOSUREMODE_SMALLAPERTURE: Exposure mode set to small
     * aperture.
     * @G_DIGICAM_EXPOSUREMODE_PORTRAIT: Exposure mode set to portrait.
     * @G_DIGICAM_EXPOSUREMODE_NIGHTPORTRAIT: Exposure mode set to night
     * @G_DIGICAM_EXPOSUREMODE_LANDSCAPE: Exposure mode set to landscape
     * time portrait.
     * @G_DIGICAM_EXPOSUREMODE_N: Ceiling and number of exposure modes.
     *
     * Indicates the camera exposure mode.
     */
    typedef enum {
        G_DIGICAM_EXPOSUREMODE_NONE          = 0,

        G_DIGICAM_EXPOSUREMODE_MANUAL        = 1 << 0,
        G_DIGICAM_EXPOSUREMODE_AUTO          = 1 << 1,
        G_DIGICAM_EXPOSUREMODE_NIGHT         = 1 << 2,
        G_DIGICAM_EXPOSUREMODE_BACKLIGHT     = 1 << 3,
        G_DIGICAM_EXPOSUREMODE_SPOTLIGHT     = 1 << 4,
        G_DIGICAM_EXPOSUREMODE_SPORTS        = 1 << 5,
        G_DIGICAM_EXPOSUREMODE_SNOW          = 1 << 6,
        G_DIGICAM_EXPOSUREMODE_BEACH         = 1 << 7,
        G_DIGICAM_EXPOSUREMODE_LARGEAPERTURE = 1 << 8,
        G_DIGICAM_EXPOSUREMODE_SMALLAPERTURE = 1 << 9,
        G_DIGICAM_EXPOSUREMODE_PORTRAIT      = 1 << 10,
        G_DIGICAM_EXPOSUREMODE_NIGHTPORTRAIT = 1 << 11,
        G_DIGICAM_EXPOSUREMODE_LANDSCAPE     = 1 << 12,

        G_DIGICAM_EXPOSUREMODE_N             = (1 << 12)+1
    } GDigicamExposuremode;


    /**
     * GDigicamIsosensitivitymode:
     * @G_DIGICAM_ISOSENSITIVITYMODE_NONE: No ISO mode set.
     * @G_DIGICAM_ISOSENSITIVITYMODE_MANUAL: ISO mode set to manual
     * sensitivity.
     * @G_DIGICAM_ISOSENSITIVITYMODE_AUTO: ISO mode set to auto
     * sensitivity.
     * @G_DIGICAM_ISOSENSITIVITYMODE_N: Ceiling and number of ISO
     * modes.
     *
     * Indicates the ISO sensitivity mode.
     */
    typedef enum {
        G_DIGICAM_ISOSENSITIVITYMODE_NONE   = 0,

        G_DIGICAM_ISOSENSITIVITYMODE_MANUAL = 1 << 0,
        G_DIGICAM_ISOSENSITIVITYMODE_AUTO   = 1 << 1,

        G_DIGICAM_ISOSENSITIVITYMODE_N      = (1 << 1)+1
    } GDigicamIsosensitivitymode;


    /**
     * GDigicamWhitebalancemode:
     * @G_DIGICAM_WHITEBALANCEMODE_NONE: No white balance mode set.
     * @G_DIGICAM_WHITEBALANCEMODE_MANUAL: White balance mode is set
     * to manual.
     * @G_DIGICAM_WHITEBALANCEMODE_AUTO: White balance mode is set to
     * auto.
     * @G_DIGICAM_WHITEBALANCEMODE_SUNLIGHT: White balance mode is set to
     * sunlight.
     * @G_DIGICAM_WHITEBALANCEMODE_CLOUDY: White balance mode is set to
     * cloudy.
     * @G_DIGICAM_WHITEBALANCEMODE_SHADE: White balance mode is set to
     * shade.
     * @G_DIGICAM_WHITEBALANCEMODE_TUNGSTEN: White balance mode is set to
     * tungsten.
     * @G_DIGICAM_WHITEBALANCEMODE_FLUORESCENT: White balance mode is set
     * to fluorescent.
     * @G_DIGICAM_WHITEBALANCEMODE_INCANDESCENT: White balance mode is set
     * to incandescent.
     * @G_DIGICAM_WHITEBALANCEMODE_FLASH: White balance mode is set to
     * flash.
     * @G_DIGICAM_WHITEBALANCEMODE_SUNSET: White balance mode is set to
     * sunset.
     * @G_DIGICAM_WHITEBALANCEMODE_NONE: Ceiling and number of white
     * balance modes.
     *
     * Indicates the white balance mode.
     */
    typedef enum {
        G_DIGICAM_WHITEBALANCEMODE_NONE         = 0,

        G_DIGICAM_WHITEBALANCEMODE_MANUAL       = 1 << 0,
        G_DIGICAM_WHITEBALANCEMODE_AUTO         = 1 << 1,
        G_DIGICAM_WHITEBALANCEMODE_SUNLIGHT     = 1 << 2,
        G_DIGICAM_WHITEBALANCEMODE_CLOUDY       = 1 << 3,
        G_DIGICAM_WHITEBALANCEMODE_SHADE        = 1 << 4,
        G_DIGICAM_WHITEBALANCEMODE_TUNGSTEN     = 1 << 5,
        G_DIGICAM_WHITEBALANCEMODE_FLUORESCENT  = 1 << 6,
        G_DIGICAM_WHITEBALANCEMODE_INCANDESCENT = 1 << 7,
        G_DIGICAM_WHITEBALANCEMODE_FLASH        = 1 << 8,
        G_DIGICAM_WHITEBALANCEMODE_SUNSET       = 1 << 9,

        G_DIGICAM_WHITEBALANCEMODE_N            = (1 << 9)+1
    } GDigicamWhitebalancemode;


    /**
     * GDigicamAperturemode:
     * @G_DIGICAM_APERTUREMODE_INVALID: Invalid and bottom limit of
     * aperture modes.
     * @G_DIGICAM_APERTUREMODE_MANUAL: Aperture mode set to manual.
     * @G_DIGICAM_APERTUREMODE_AUTO: Aperture mode set to auto.
     * @G_DIGICAM_APERTUREMODE_N: Ceiling and number of aperture
     * modes.
     *
     * Indicates the way in which the digicam aperture will behave.
     */
    typedef enum {
        G_DIGICAM_APERTUREMODE_INVALID = -1,

        G_DIGICAM_APERTUREMODE_MANUAL,
        G_DIGICAM_APERTUREMODE_AUTO,

        G_DIGICAM_APERTUREMODE_N
    } GDigicamAperturemode;


    /**
     * GDigicamMeteringmode:
     * @G_DIGICAM_METERINGMODE_NONE: No metering mode set.
     * @G_DIGICAM_METERINGMODE_AVERAGE: Metering mode set to center
     * weighted average.
     * @G_DIGICAM_METERINGMODE_SPOT: Metering mode set to spot.
     * @G_DIGICAM_METERINGMODE_MATRIX: Metering mode set to matrix.
     * @G_DIGICAM_METERINGMODE_N: Ceiling and number of metering
     * modes.
     *
     * Indicates the metering mode for exposure.
     */
    typedef enum {
        G_DIGICAM_METERINGMODE_NONE = 0,

        G_DIGICAM_METERINGMODE_AVERAGE = 1 << 0,
        G_DIGICAM_METERINGMODE_SPOT    = 1 << 1,
        G_DIGICAM_METERINGMODE_MATRIX  = 1 << 2,

        G_DIGICAM_METERINGMODE_N       = (1 << 2)+1
    } GDigicamMeteringmode;


    /**
     * GDigicamShutterspeedmode:
     * @G_DIGICAM_SHUTTERSPEEDMODE_INVALID: Invalid and bottom limit
     * of shutter speed modes.
     * @G_DIGICAM_SHUTTERSPEEDMODE_MANUAL: Shutter speed mode set to
     * manual.
     * @G_DIGICAM_SHUTTERSPEEDMODE_AUTO: Shutter speed mode set to auto.
     * @G_DIGICAM_SHUTTERSPEEDMODE_N: Ceiling and number of shutter
     * speed modes.
     *
     * Indicates the shutter speed mode.
     */
    typedef enum {
        G_DIGICAM_SHUTTERSPEEDMODE_INVALID = -1,

        G_DIGICAM_SHUTTERSPEEDMODE_MANUAL,
        G_DIGICAM_SHUTTERSPEEDMODE_AUTO,

        G_DIGICAM_SHUTTERSPEEDMODE_N
    } GDigicamShutterspeedmode;


    /**
     * GDigicamAspectratio:
     * @G_DIGICAM_ASPECTRATIO_NONE: No aspect ratio set.
     * @G_DIGICAM_ASPECTRATIO_4X3: Aspect ratio set to 4x3.
     * @G_DIGICAM_ASPECTRATIO_16X9: Aspect ratio set to 16x9.
     * @G_DIGICAM_ASPECTRATIO_N: Ceiling and number of aspect ratios.
     *
     * Indicates the aspect ratio.
     */
    typedef enum {
        G_DIGICAM_ASPECTRATIO_NONE = 0,

        G_DIGICAM_ASPECTRATIO_4X3  = 1 << 0,
        G_DIGICAM_ASPECTRATIO_16X9 = 1 << 1,

        G_DIGICAM_ASPECTRATIO_N    = (1 << 1)+1
    } GDigicamAspectratio;


    /**
     * GDigicamColorfilter:
     * @G_DIGICAM_COLORFILTER_NONE: No color filter set.
     * @G_DIGICAM_COLORFILTER_NORMAL: Color filter not set.
     * @G_DIGICAM_COLORFILTER_SEPIA: Color filter set to sepia.
     * @G_DIGICAM_COLORFILTER_VIVID: Color filter set to vivid.
     * @G_DIGICAM_COLORFILTER_BW: Color filter set to black & white.
     * @G_DIGICAM_COLORFILTER_N: Ceiling and number of color filters.
     *
     * Indicates the color filter to use when capturing.
     */
    typedef enum {
        G_DIGICAM_COLORFILTER_NONE   = 0,

        G_DIGICAM_COLORFILTER_NORMAL = 1 << 0,
        G_DIGICAM_COLORFILTER_SEPIA  = 1 << 1,
        G_DIGICAM_COLORFILTER_VIVID  = 1 << 2,
        G_DIGICAM_COLORFILTER_BW     = 1 << 3,

        G_DIGICAM_COLORFILTER_N      = (1 << 3)+1
    } GDigicamColorfilter;


    /**
     * GDigicamQuality:
     * @G_DIGICAM_QUALITY_NONE: No quality set.
     * @G_DIGICAM_QUALITY_HIGH: Quality set to high.
     * @G_DIGICAM_QUALITY_LOW: Quality set to low.
     * @G_DIGICAM_QUALITY_N: Ceiling and number of qualities.
     *
     * Indicates the capturing quality level.
     */
    typedef enum {
        G_DIGICAM_QUALITY_NONE = 0,

        G_DIGICAM_QUALITY_HIGH = 1 << 0,
        G_DIGICAM_QUALITY_LOW  = 1 << 1,

        G_DIGICAM_QUALITY_N  = (1 << 1)+1
    } GDigicamQuality;


    /**
     * GDigicamResolution:
     * @G_DIGICAM_RESOLUTION_NONE: No resolution set.
     * @G_DIGICAM_RESOLUTION_HIGH: Resolution set to high.
     * @G_DIGICAM_RESOLUTION_MEDIUM: Resolution set to medium.
     * @G_DIGICAM_RESOLUTION_LOW: Resolution set to low.
     * @G_DIGICAM_RESOLUTION_N: Ceiling and number of resolutions.
     *
     * Indicates the capturing resolution.
     */
    typedef enum {
        G_DIGICAM_RESOLUTION_NONE   = 0,

        G_DIGICAM_RESOLUTION_HIGH   = 1 << 0,
        G_DIGICAM_RESOLUTION_MEDIUM = 1 << 1,
        G_DIGICAM_RESOLUTION_LOW    = 1 << 2,

        G_DIGICAM_RESOLUTION_N      = (1 << 2)+1
    } GDigicamResolution;


    /**
     * GDigicamVideostabilization:
     * @G_DIGICAM_VIDEOSTABILIZATION_INVALID: Invalid and bottom limit
     * of video stabilization modes.
     * @G_DIGICAM_VIDEOSTABILIZATION_OFF: Video stabilization off.
     * @G_DIGICAM_VIDEOSTABILIZATION_ON: Video stabilization on.
     * @G_DIGICAM_VIDEOSTABILIZATION_N: Ceiling and number of video
     * stabilization modes.
     *
     * Indicates the video stabilization mode.
     */
    typedef enum {
        G_DIGICAM_VIDEOSTABILIZATION_INVALID = -1,

        G_DIGICAM_VIDEOSTABILIZATION_OFF,
        G_DIGICAM_VIDEOSTABILIZATION_ON,

        G_DIGICAM_VIDEOSTABILIZATION_N
    } GDigicamVideostabilization;


    /**
     * GDigicamLock:
     * @G_DIGICAM_LOCK_AUTOFOCUS_NONE: No lock for autofocus set.
     * @G_DIGICAM_LOCK_AUTOFOCUS: Lock for the automatic focus.
     * @G_DIGICAM_LOCK_AUTOEXPOSURE: Lock for the automatic exposure
     * settings.
     * @G_DIGICAM_LOCK_AUTOWHITEBALANCE: Lock for the automatic white
     * balance.
     * @G_DIGICAM_LOCK_AUTOFOCUS_N: Ceiling and number of autofocus
     * locks.
     *
     * GDigicam locks.
     */
    typedef enum {
        G_DIGICAM_LOCK_AUTOFOCUS_NONE   = 0,

        G_DIGICAM_LOCK_AUTOFOCUS        = 1 << 0,
        G_DIGICAM_LOCK_AUTOEXPOSURE     = 1 << 1,
        G_DIGICAM_LOCK_AUTOWHITEBALANCE = 1 << 2,

        G_DIGICAM_LOCK_N                = 1 << 3
    } GDigicamLock;


    /**
     * GDigicamFocuspoints:
     * @G_DIGICAM_FOCUSPOINTS_NONE: No focus points set.
     * @G_DIGICAM_FOCUSPOINTS_ONE: Just a single focus point placed in
     * the center.
     * @G_DIGICAM_FOCUSPOINTS_THREE_3X1: Three focus points in a
     * horizontal line.
     * @G_DIGICAM_FOCUSPOINTS_FIVE_CROSS: Five focus points shaping a cross.
     * @G_DIGICAM_FOCUSPOINTS_SEVEN_CROSS: Seven focus points shaping a
     * cross.
     * @G_DIGICAM_FOCUSPOINTS_NINE_SQUARE: Nine focues points drawing a
     * square.
     * @G_DIGICAM_FOCUSPOINTS_ELEVEN_CROSS: Eleven focus points shaping
     * a cross.
     * @G_DIGICAM_FOCUSPOINTS_TWELVE_3X4: Twelve focus points; three
     * horizontal, four vertical.
     * @G_DIGICAM_FOCUSPOINTS_TWELVE_4X3: Twelve focus points; three
     * vertical, four horizontal.
     * @G_DIGICAM_FOCUSPOINTS_SIXTEEN_SQUARE: Sixteen focus points
     * drawing a square.
     * @G_DIGICAM_FOCUSPOINTS_CUSTOM: Custom; let's the user select the
     * focus points
     * @G_DIGICAM_FOCUSPOINTS_N: Ceiling and number of focus points
     * configurations.
     *
     * GDigicam focus points.
     */
    typedef enum {
        G_DIGICAM_FOCUSPOINTS_NONE           = 0,

        G_DIGICAM_FOCUSPOINTS_ONE            = 1 << 0,
        G_DIGICAM_FOCUSPOINTS_THREE_3X1      = 1 << 1,
        G_DIGICAM_FOCUSPOINTS_FIVE_CROSS     = 1 << 2,
        G_DIGICAM_FOCUSPOINTS_SEVEN_CROSS    = 1 << 3,
        G_DIGICAM_FOCUSPOINTS_NINE_SQUARE    = 1 << 4,
        G_DIGICAM_FOCUSPOINTS_ELEVEN_CROSS   = 1 << 5,
        G_DIGICAM_FOCUSPOINTS_TWELVE_3X4     = 1 << 6,
        G_DIGICAM_FOCUSPOINTS_TWELVE_4X3     = 1 << 7,
        G_DIGICAM_FOCUSPOINTS_SIXTEEN_SQUARE = 1 << 8,
        G_DIGICAM_FOCUSPOINTS_CUSTOM         = 1 << 9,

        G_DIGICAM_FOCUSPOINTS_N              = (1 << 9)+1
    } GDigicamFocuspoints;


    /**
     * GDigicamFocusmodestatus:
     * @G_DIGICAM_FOCUSMODESTATUS_INVALID: Invalid and bottom limit of
     * focus mode statuses.
     * @G_DIGICAM_FOCUSMODESTATUS_OFF: Indicates that the manual focus
     * mode is set so the focus status is not available.
     * @G_DIGICAM_FOCUSMODESTATUS_REQUEST: Indicates that the focus
     * request is currently in progress.
     * @G_DIGICAM_FOCUSMODESTATUS_REACHED: Indicates that the focus has
     * been reached.
     * @G_DIGICAM_FOCUSMODESTATUS_UNABLETOREACH: Indicates that the camera
     * was unable to reach the focus.
     * @G_DIGICAM_FOCUSMODESTATUS_LOST: Indicates that the focus has been
     * lost.
     * @G_DIGICAM_FOCUSMODESTATUS_N: Ceiling and number of focus mode
     * statuses.
     *
     * Indicates the status of the focus mode.
     */
    typedef enum {
        G_DIGICAM_FOCUSMODESTATUS_INVALID = -1,

        G_DIGICAM_FOCUSMODESTATUS_OFF,
        G_DIGICAM_FOCUSMODESTATUS_REQUEST,
        G_DIGICAM_FOCUSMODESTATUS_REACHED,
        G_DIGICAM_FOCUSMODESTATUS_UNABLETOREACH,
        G_DIGICAM_FOCUSMODESTATUS_LOST,

        G_DIGICAM_FOCUSMODESTATUS_N
    } GDigicamFocusmodestatus;


    /**
     * GDigicamAutoexposurestatus:
     * @G_DIGICAM_AUTOEXPOSURESTATUS_INVALID: Invalid and bottom limit
     * of autoexposure statuses.
     * @G_DIGICAM_AUTOEXPOSURESTATUS_SUCCESS: Auto exposure has been
     * successfully locked.
     * @G_DIGICAM_AUTOEXPOSURESTATUS_UNDEREXPOSURE: Auto exposure has
     * been locked, but the photo will we underexposed in the current
     * lighting conditions. Consider changing manually the exposure
     * settings or freeing the lock and trying the locking again.
     * @G_DIGICAM_AUTOEXPOSURESTATUS_OVEREXPOSURE: Auto exposure has
     * been locked, but the photo will we overexposed in the current
     * lighting conditions. Consider changing manually the exposure
     * settings or freeing the lock and trying the locking again.
     * @G_DIGICAM_AUTOEXPOSURESTATUS_N: Ceiling and number of
     * autoexposure statuses.
     *
     * Indicates the status of the auto exposure.
     */
    typedef enum {
        G_DIGICAM_AUTOEXPOSURESTATUS_INVALID = -1,

        G_DIGICAM_AUTOEXPOSURESTATUS_SUCCESS,
        G_DIGICAM_AUTOEXPOSURESTATUS_UNDEREXPOSURE,
        G_DIGICAM_AUTOEXPOSURESTATUS_OVEREXPOSURE,

        G_DIGICAM_AUTOEXPOSURESTATUS_N
    } GDigicamAutoexposurestatus;


    /**
     * GDigicamAudio:
     * @G_DIGICAM_AUDIO_NONE: No audio settings.
     * @G_DIGICAM_AUDIO_PLAYBACKON: Audio playback is on.
     * @G_DIGICAM_AUDIO_PLAYBACKOFF: Audio playback is off.
     * @G_DIGICAM_AUDIO_RECORDON: Audio recording is on.
     * @G_DIGICAM_AUDIO_RECORDOFF: Audio recording is off.
     * @G_DIGICAM_AUDIO_N: Ceiling and number of audio settings.
     *
     * GDigicam audio states.
     */
    typedef enum {
        G_DIGICAM_AUDIO_NONE        = 0,

        G_DIGICAM_AUDIO_PLAYBACKON  = 1 << 0,
        G_DIGICAM_AUDIO_PLAYBACKOFF = 1 << 1,
        G_DIGICAM_AUDIO_RECORDON    = 1 << 2,
        G_DIGICAM_AUDIO_RECORDOFF   = 1 << 3,

        G_DIGICAM_AUDIO_N           = 1 << 4
    } GDigicamAudio;


    /**
     * GDigicamPreview:
     * @G_DIGICAM_PREVIEW_NONE: No preview settings.
     * @G_DIGICAM_PREVIEW_ON: Preview is enabled.
     * @G_DIGICAM_PREVIEW_OFF: Preview is disabled.
     * @G_DIGICAM_PREVIEW_N: Ceiling and number of preview settings.
     *
     * GDigicam audio states.
     */
    typedef enum {
        G_DIGICAM_PREVIEW_NONE      = 0,

        G_DIGICAM_PREVIEW_ON        = 1 << 0,
        G_DIGICAM_PREVIEW_OFF       = 1 << 1,

        G_DIGICAM_PREVIEW_N         = (1 << 1)+1
    } GDigicamPreview;

    /**
     * GDigicamDescriptor:
     * @name: The name of the digicam like #GstElement it owns to.
     * @viewfinder_sink: the a #GstElement reference to the viewfinder
     * sink of the digicam like #GstElement, if any.
     * @supported_features: mask holding the features of the digicam
     * like #GstElement.
     * @supported_modes: mask holding the modes supported by the digicam
     * like #GstElement composed with #GDigicamMode values.
     * @set_mode_func: custom #GDigicamManagerFunc like function to
     * change the #GDigicamMode of the digicam like #GstElement.
     * @supported_flash_modes: mask holding the flash modes supported
     * by the digicam like #GstElement composed with
     * #GDigicamFlashmode values.
     * @set_flash_mode_func: custom #GDigicamManagerFunc like function to
     * change the #GDigicamFlashmode of the digicam like #GstElement.
     * @supported_focus_modes: mask holding the focus modes supported
     * by the digicam like #GstElement composed with
     * #GDigicamFocusmode values.
     * @set_focus_mode_func: custom #GDigicamManagerFunc like function
     * to change the #GDigicamFocusmode of the digicam like
     * #GstElement.
     * @set_focus_region_pattern_func: custom #GDigicamManagerFunc
     * like function to set the focus points mode from
     * #GDigicamFocuspoints values a mask pointing the active points.
     * @supported_exposure_modes: mask holding the exposure modes
     * supported by the digicam like #GstElement composed with
     * #GDigicamExposuremode values.
     * @set_exposure_mode_func: custom #GDigicamManagerFunc like
     * function to change the #GDigicamExposuremode of the digicam
     * like #GstElement.
     * @set_exposure_comp_func: custom #GDigicamManagerFunc like
     * function to change the exposure compensation of the digicam
     * like #GstElement.
     * @supported_metering_modes: mask holding the metering modes
     * supported by the digicam like #GstElement composed with
     * #GDigicamMeteringmode values.
     * @set_metering_mode_func: custom #GDigicamManagerFunc like
     * function to change the #GDigicamMeteringmode of the digicam
     * like #GstElement.
     * @supported_aspect_ratios: mask holding the aspect ratio modes
     * supported by the digicam like #GstElement composed with
     * #GDigicamAspectratio values.
     * @set_aspect_ratio_func: custom #GDigicamManagerFunc like
     * function to change the #GDigicamAspectratio of the digicam like
     * #GstElement.
     * @supported_qualities: mask holding the qualities supported by
     * the digicam like #GstElement composed with #GDigicamQualities
     * values.
     * @set_quality_func: custom #GDigicamManagerFunc like function to
     * change the #GDigicamQualities of the digicam like #GstElement.
     * @supported_resolutions: mask holding the resolution modes
     * supported by the digicam like #GstElement composed with
     * #GDigicamResolution values.
     * @set_resolution_func: custom #GDigicamManagerFunc like
     * function to change the #GDigicamResolution of the digicam like
     * #GstElement.
     * @set_aspect_ratio_resolution_func: custom #GDigicamManagerFunc like
     * function to change the #GDigicamResolution and #GDigicamAspectRatio 
     * of the digicam like #GstElement.
     * @max_zoom_macro_disabled: The maximum overall zoom value with
     * the macro mode disabled.
     * @max_zoom_macro_enabled: The maximum overall zoom value with
     * the macro mode enabled.
     * @max_optical_zoom_macro_disabled: The maximum optical zoom
     * value with the macro mode disabled.
     * @max_optical_zoom_macro_enabled: The maximum optical zoom value
     * with the macro mode enabled.
     * @max_digital_zoom: The maximum digital zoom value.
     * @set_zoom_func: custom #GDigicamManagerFunc like function to
     * change the zoom value of the digicam like #GstElement.
     * @set_iso_sensitivity_mode_func: custom #GDigicamManagerFunc like function to
     * change the iso sensitivity value of the digicam like #GstElement.
     * @supported_iso_sensitivity_modes: mask holding the iso sensitivity modes
     * supported by the digicam like #GstElement composed with
     * @set_white_balance_mode_func: custom #GDigicamManagerFunc like function to
     * change the white balance value of the digicam like #GstElement.
     * @supported_white_balance_modes: mask holding the iso white balance modes
     * supported by the digicam like #GstElement composed with
     * @supported_audio_states: mask holding the audio states
     * supported by the digicam like #GstElement composed with
     * @set_audio_func: custom #GDigicamManagerFunc like function to
     * change the audio status of the digicam like #GstElement.
     * @set_locks_func: custom #GDigicamManagerFunc like function to
     * change the locks flags of the digicam like #GstElement.
     * @get_still_picture_func: custom #GDigicamManagerFunc like
     * function to grab still picture using the digicam like
     * #GstElement.
     * @start_recording_video_func: custom #GDigicamManagerFunc like
     * function to start a new video recording using the digicam like
     * #GstElement.
     * @pause_recording_video_func: custom #GDigicamManagerFunc like
     * function to pause the recording in progress using the digicam like
     * #GstElement.
     * @finish_recording_video_func: custom #GDigicamManagerFunc like
     * function to stop the recording in progress using the digicam like
     * #GstElement.
     * @handle_bus_message: custom #GDigicamManagerFunc to
     * handle the bus messages emitted by the bin
     * @handle_syncbus_message: custom #GDigicamManagerFunc to
     * handle the sync bus messages emitted by the bin
     *
     * The #GDigicamDescriptor structure contains the capabilities of
     * the camera.
     */
    struct _GDigicamDescriptor
    {
        gchar *name;
        GstElement *viewfinder_sink;
/*         guint32 max_width; */
/*         guint32 max_height; */
/*         guint orientation; */
        guint32 supported_features;
        guint supported_modes;
        GDigicamManagerFunc set_mode_func;
        guint supported_flash_modes;
        GDigicamManagerFunc set_flash_mode_func;
        guint supported_focus_modes;
        GDigicamManagerFunc set_focus_mode_func;
        GDigicamManagerFunc set_focus_region_pattern_func;
        guint supported_exposure_modes;
        GDigicamManagerFunc set_exposure_mode_func;
        GDigicamManagerFunc set_exposure_comp_func;
        guint supported_iso_sensitivity_modes;
        GDigicamManagerFunc set_iso_sensitivity_mode_func;
        guint supported_white_balance_modes;
        GDigicamManagerFunc set_white_balance_mode_func;
        guint supported_metering_modes;
        GDigicamManagerFunc set_metering_mode_func;
        guint supported_aspect_ratios;
        GDigicamManagerFunc set_aspect_ratio_func;
/*         guint supported_color_filters; */
        guint supported_qualities;
	GDigicamManagerFunc set_quality_func;
        guint supported_resolutions;
        GDigicamManagerFunc set_resolution_func;
        GDigicamManagerFunc set_aspect_ratio_resolution_func;
        GDigicamManagerFunc set_locks_func;
        gdouble max_zoom_macro_disabled;
        gdouble max_zoom_macro_enabled;
        gdouble max_optical_zoom_macro_disabled;
        gdouble max_optical_zoom_macro_enabled;
        gdouble max_digital_zoom;
        GDigicamManagerFunc set_zoom_func;
        guint supported_audio_states;
        GDigicamManagerFunc set_audio_func;
        guint supported_preview_modes;
        GDigicamManagerFunc set_preview_mode_func;
        GDigicamManagerFunc get_still_picture_func;
        GDigicamManagerFunc start_recording_video_func;
        GDigicamManagerFunc pause_recording_video_func;
        GDigicamManagerFunc finish_recording_video_func;
        GDigicamManagerFunc handle_bus_message_func;
        GDigicamManagerFunc handle_sync_bus_message_func;
/*         gdouble min_focus_distance_macro_disabled; */
/*         gdouble min_focus_distance_macro_enabled; */
/*         guint min_gamma; */
/*         guint max_gamma; */
    };


    /**
     * GDigicamFocuspointposition:
     * @left: left coordinate of the focus point.
     * @top: top coordinate of the focus point.
     * @width: width of the focus point.
     * @height: height of the focus point.
     *
     * The #GDigicamFocuspointposition structure contains the position
     * of a single focus point.
     */
    struct _GDigicamFocuspointposition {
        guint left;
        guint top;
        guint width;
        guint height;
    };

    /**
     * GDigicamExposureconf:
     *
     * The #GDigicamExposureconf structure contains configuration
     * associated to a specific #GDigicamExposuremode.
     */
    typedef struct _GDigicamExposureconf
    {
        GDigicamFlashmode flash_mode;
        gdouble exposure_compensation;
        GDigicamIsosensitivitymode iso_sensitivity_mode;
        guint iso_level;
        GDigicamWhitebalancemode white_balance_mode;
        guint white_balance_level;
    } GDigicamExposureconf;

    /**
     * GDigicamManager:
     *
     * The #GDigicamManager structure contains only private data and
     * should not be accessed directly.
     */
    struct _GDigicamManager
    {
        /*< private >*/
        GObject parent_instance;
    };


    /**
     * GDigicamManagerClass:
     *
     * Base class for #GDigicamManager.
     */
    struct _GDigicamManagerClass
    {
        /*< private >*/
        GObjectClass parent_class;

/* #if 0 */
/*         void (* rotation)  (GDigicamManager *manager, gint degrees); */
/*         void (* flash_ready)  (GDigicamManager *manager); */
/*         void (* exposure_status)  (GDigicamManager *manager, */
/*                                    GDigicamAutoexposurestatus auto_exposure_status); */
/*         void (* white_balance_locked)  (GDigicamManager *manager); */
/* #endif */
        void (*focus_done) (GDigicamManager *manager,
                           GDigicamFocusmodestatus status);
        gboolean (*pict_done) (GDigicamManager *manager,
                               GString *filename);
	void (*capture_start) (GDigicamManager *manager);

	void (*capture_end) (GDigicamManager *manager);

	void (*picture_got) (GDigicamManager *manager);

	void (*image_preview) (GDigicamManager *manager, GdkPixbuf *value);

	void (*internal_error) (GDigicamManager *manager);
    };


    /********************************/
    /* GObject Management functions */
    /********************************/

    GType _g_digicam_manager_get_type (void);


    /********************/
    /* Public functions */
    /********************/

    GDigicamManager* g_digicam_manager_new (void);
    gboolean g_digicam_manager_set_gstreamer_bin (GDigicamManager   *manager,
                                                  GstElement        *gst_bin,
                                                  GDigicamDescriptor *descriptor,
                                                  GError           **error);
    gboolean g_digicam_manager_get_gstreamer_bin (GDigicamManager   *manager,
                                                  GstElement       **gst_bin,
                                                  GError           **error);
    gboolean g_digicam_manager_query_capabilities (GDigicamManager     *manager,
                                                   GDigicamDescriptor **descriptor,
                                                   GError             **error);
    gboolean g_digicam_manager_set_mode (GDigicamManager         *manager,
                                         GDigicamMode             mode,
                                         GError                 **error,
                                         gpointer user_data);
    gboolean g_digicam_manager_get_mode (GDigicamManager         *manager,
                                         GDigicamMode            *mode,
                                         GError                 **error);
    gboolean g_digicam_manager_set_flash_mode (GDigicamManager   *manager,
                                               GDigicamFlashmode  flash_mode,
                                               GError           **error,
                                               gpointer user_data);
    gboolean g_digicam_manager_get_flash_mode (GDigicamManager   *manager,
                                               GDigicamFlashmode *flash_mode,
                                               GError                 **error);
/*     gboolean g_digicam_manager_is_flash_ready (GDigicamManager   *manager, */
/*                                                gboolean          *flash_ready, */
/*                                                GError           **error); */
    gboolean g_digicam_manager_set_focus_mode (GDigicamManager   *manager,
                                               GDigicamFocusmode  focus_mode,
                                               gboolean macro_enabled,
                                               GError           **error,
                                               gpointer user_data);
    gboolean g_digicam_manager_get_focus_mode (GDigicamManager   *manager,
                                               GDigicamFocusmode *focus_mode,
                                               gboolean *macro_enabled,
                                               GError           **error);
    gboolean g_digicam_manager_set_focus_region_pattern (GDigicamManager   *manager,
                                                         GDigicamFocuspoints focus_points,
                                                         guint64 active_points,
                                                         GError           **error,
                                                         gpointer user_data);
    gboolean g_digicam_manager_get_focus_region_pattern (GDigicamManager   *manager,
                                                         GDigicamFocuspoints *focus_points,
                                                         guint64 *active_points,
                                                         GError           **error);
/*     gboolean g_digicam_manager_get_focus_region_positions (GDigicamManager   *manager, */
/*                                                            GSList *slist, */
/*                                                            GError           **error); */
/*     gboolean g_digicam_manager_get_focus_mode_status (GDigicamManager   *manager, */
/*                                                       GDigicamFocusmodestatus *focus_mode_status, */
/*                                                       guint64 *region_status, */
/*                                                       GError           **error); */
    gboolean g_digicam_manager_set_exposure_mode (GDigicamManager       *manager,
                                                  GDigicamExposuremode   exposure_mode,
                                                  const GDigicamExposureconf   *conf,
                                                  GError               **error,
                                                  gpointer               user_data);
    gboolean g_digicam_manager_get_exposure_mode (GDigicamManager       *manager,
                                                  GDigicamExposuremode  *exposure_mode,
                                                  GError               **error);
    gboolean g_digicam_manager_set_exposure_comp (GDigicamManager *manager,
                                                  gdouble          exposure_comp,
                                                  GError         **error,
                                                  gpointer         user_data);
    gboolean g_digicam_manager_get_exposure_comp (GDigicamManager *manager,
                                                  gdouble         *exposure_comp,
                                                  GError         **error);
    gboolean g_digicam_manager_set_iso_sensitivity_mode (GDigicamManager   *manager,
                                                         GDigicamIsosensitivitymode  iso_sensitivity_mode,
                                                         guint iso_level,
                                                         GError           **error,
                                                         gpointer user_data);
    gboolean g_digicam_manager_get_iso_sensitivity_mode (GDigicamManager   *manager,
                                                         GDigicamIsosensitivitymode *iso_sensitivity_mode,
                                                         guint *iso_level,
                                                         GError           **error);
/*     gboolean g_digicam_manager_set_aperture_mode (GDigicamManager   *manager, */
/*                                                   GDigicamAperturemode  aperture_mode, */
/*                                                   guint manual_settings, */
/*                                                   GError           **error); */
/*     gboolean g_digicam_manager_get_aperture_mode (GDigicamManager   *manager, */
/*                                                   GDigicamAperturemode *aperture_mode, */
/*                                                   guint *manual_settings, */
/*                                                   GError           **error); */
/*     gboolean g_digicam_manager_set_shutter_speed_mode (GDigicamManager   *manager, */
/*                                                        GDigicamShutterspeedmode  shutter_speed_mode, */
/*                                                        guint manual_settings, */
/*                                                        GError           **error); */
/*     gboolean g_digicam_manager_get_shutter_speed_mode (GDigicamManager   *manager, */
/*                                                        GDigicamShutterspeedmode *shutter_speed_mode, */
/*                                                        guint *manual_settings, */
/*                                                        GError           **error); */
    gboolean g_digicam_manager_set_white_balance_mode (GDigicamManager   *manager,
                                                       GDigicamWhitebalancemode  white_balance_mode,
                                                       guint white_balance_level,
                                                       GError           **error,
                                                       gpointer user_data);
    gboolean g_digicam_manager_get_white_balance_mode (GDigicamManager   *manager,
                                                       GDigicamWhitebalancemode *white_balance_mode,
                                                       guint *white_balance_level,
                                                       GError           **error);
    gboolean g_digicam_manager_set_metering_mode (GDigicamManager   *manager,
                                                  GDigicamMeteringmode  metering_mode,
                                                  GError           **error,
                                                  gpointer user_data);
    gboolean g_digicam_manager_get_metering_mode (GDigicamManager   *manager,
                                                  GDigicamMeteringmode *metering_mode,
                                                  GError           **error);
    gboolean g_digicam_manager_set_aspect_ratio (GDigicamManager   *manager,
                                                 GDigicamAspectratio  aspect_ratio,
                                                 GError           **error,
                                                 gpointer user_data);
    gboolean g_digicam_manager_get_aspect_ratio (GDigicamManager   *manager,
                                                 GDigicamAspectratio *aspect_ratio,
                                                 GError           **error);
/*     gboolean g_digicam_manager_set_color_filter (GDigicamManager   *manager, */
/*                                                  GDigicamColorfilter  color_filter, */
/*                                                  GError           **error); */
/*     gboolean g_digicam_manager_get_color_filter (GDigicamManager   *manager, */
/*                                                  GDigicamColorfilter *color_filter, */
/*                                                  GError           **error); */
    gboolean g_digicam_manager_set_quality (GDigicamManager   *manager,
                                            GDigicamQuality  quality,
                                            GError           **error,
                                            gpointer user_data);
    gboolean g_digicam_manager_get_quality (GDigicamManager   *manager,
                                            GDigicamQuality *quality,
                                            GError           **error);
    gboolean g_digicam_manager_set_resolution (GDigicamManager   *manager,
                                               GDigicamResolution  resolution,
                                               GError           **error,
                                               gpointer user_data);
    gboolean g_digicam_manager_get_resolution (GDigicamManager   *manager,
                                               GDigicamResolution *resolution,
                                               GError           **error);
    gboolean g_digicam_manager_set_aspect_ratio_resolution (GDigicamManager    *manager,
							    GDigicamAspectratio aspect_ratio,
							    GDigicamResolution  resolution,
							    GError            **error,
							    gpointer            user_data);
/*     gboolean g_digicam_manager_set_video_stabilization (GDigicamManager   *manager, */
/*                                                         GDigicamVideostabilization  video_stabilization, */
/*                                                         GError           **error); */
/*     gboolean g_digicam_manager_get_video_stabilization (GDigicamManager   *manager, */
/*                                                         GDigicamVideostabilization *video_stabilization, */
/*                                                         GError           **error); */
    gboolean g_digicam_manager_set_locks (GDigicamManager   *manager,
                                          GDigicamLock       locks,
                                          GError           **error,
                                          gpointer           user_data);
    gboolean g_digicam_manager_get_locks (GDigicamManager   *manager,
                                          GDigicamLock      *locks,
                                          GError           **error);
    gboolean g_digicam_manager_set_zoom (GDigicamManager   *manager,
                                         gdouble            zoom,
                                         gboolean          *digital,
                                         GError           **error,
                                         gpointer           user_data);
    gboolean g_digicam_manager_get_zoom (GDigicamManager   *manager,
                                         gdouble           *zoom,
                                         gboolean          *digital,
                                         GError           **error);
/*     gboolean g_digicam_manager_zoom_in (GDigicamManager   *manager, */
/*                                         GError           **error); */
/*     gboolean g_digicam_manager_zoom_out (GDigicamManager   *manager, */
/*                                          GError           **error); */
    gboolean g_digicam_manager_set_audio (GDigicamManager *manager,
                                          GDigicamAudio    audio,
                                          GError         **error,
                                          gpointer         user_data);
    gboolean g_digicam_manager_get_audio (GDigicamManager *manager,
                                          GDigicamAudio   *audio,
                                          GError         **error);
    gboolean g_digicam_manager_set_preview_mode (GDigicamManager  *manager,
                                                 GDigicamPreview   mode,
                                                 GError          **error,
                                                 gpointer          user_data);
    gboolean g_digicam_manager_get_preview_mode (GDigicamManager  *manager,
                                                 GDigicamPreview  *mode,
                                                 GError          **error);
    gboolean g_digicam_manager_preview_enabled (GDigicamManager  *manager,
                                                gboolean         *enabled,
                                                GError          **error);
/*     gboolean g_digicam_manager_query_focus_region_patterns (GDigicamManager    *manager, */
/*                                                             GDigicamFocuspoints *focus_point, */
/*                                                             guint64 *custom_points, */
/*                                                             GError            **error); */
/*     gboolean g_digicam_manager_query_supported_locks (GDigicamManager    *manager, */
/*                                                       GSList *slist, */
/*                                                       GError            **error); */
    gboolean g_digicam_manager_play_bin (GDigicamManager *manager,
					 gulong xwindow_id,
					 GError **error);
    gboolean g_digicam_manager_stop_bin (GDigicamManager *manager,
					 GError **error);
    gboolean g_digicam_manager_capture_still_picture (GDigicamManager *manager,
                                                      const gchar     *filename,
                                                      GError          **error,
                                                      gpointer user_data);
    gboolean g_digicam_manager_start_recording_video (GDigicamManager *manager,
                                                      GError **error,
						      gpointer user_data);
    gboolean g_digicam_manager_pause_recording_video (GDigicamManager *manager,
                                                      gboolean resume,
                                                      GError **error,
						      gpointer user_data);
    gboolean g_digicam_manager_finish_recording_video (GDigicamManager *manager,
                                                       GError **error,
						       gpointer user_data);
    GDigicamDescriptor* g_digicam_manager_descriptor_new (void);
    void g_digicam_manager_descriptor_free (GDigicamDescriptor *descriptor);
    GDigicamDescriptor* g_digicam_manager_descriptor_copy (const GDigicamDescriptor *orig_descriptor);

    G_END_DECLS

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* __G_DIGICAM_MANAGER_H__ */
