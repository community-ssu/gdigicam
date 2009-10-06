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

#ifndef __G_DIGICAM_ERROR_H__
#define __G_DIGICAM_ERROR_H__

#ifdef __cplusplus
extern "C" {
#endif

    G_BEGIN_DECLS

    /**
     * G_DIGICAM_ERROR:
     *
     * #GQuark identifier for #GDigicamError.
     *
     * Returns: the #GDigicamError #GQuark.
     */
#define G_DIGICAM_ERROR g_digicam_error_quark ()

    /**
     * GDigicamError:
     * @G_DIGICAM_ERROR_FAILED: Something didn't work, don't know why,
     *  probably unrecoverable so there's no point having a more specific
     *  errno.
     * @G_DIGICAM_ERROR_GSTREAMER_BIN_NOT_SET: There isn't any
     *  GStreamer Digicam bin set.
     * @G_DIGICAM_ERROR_VIEWFINDER_NOT_SUPPORTED: The GStreamer Digicam
     *  has not viewfinder capability.
     * @G_DIGICAM_ERROR_MODE_NOT_SUPPORTED: The mode is not supported by
     *  the device.
     * @G_DIGICAM_ERROR_INVALID_MODE: The operation is invalid for this
     *  mode.
     * @G_DIGICAM_ERROR_FLASHMODE_NOT_SUPPORTED: The GStreamer Digicam
     *  has not this flash mode capability.
     * @G_DIGICAM_ERROR_FOCUSMODE_NOT_SUPPORTED: The GStreamer Digicam
     *  has not this focus mode capability.
     * @G_DIGICAM_ERROR_INVALID_FOCUSMODE: The operation is invalid for
     *  this focus mode.
     * @G_DIGICAM_ERROR_AUTOFOCUS_LOCKED: The GStreamer Digicam has the
     *  autofocus locked.
     * @G_DIGICAM_ERROR_EXPOSUREMODE_NOT_SUPPORTED: The GStreamer
     *  Digicam has not this exposure mode capability.
     * @G_DIGICAM_ERROR_ISOSENSITIVITYMODE_NOT_SUPPORTED: The GStreamer
     *  Digicam has not this iso sensitivity mode capability.
     * @G_DIGICAM_ERROR_WHITEBALANCEMODE_NOT_SUPPORTED: The GStreamer
     *  Digicam has not this white balance mode capability.
     * @G_DIGICAM_ERROR_METERINGMODE_NOT_SUPPORTED: The GStreamer
     *  Digicam has not this metering mode capability.
     * @G_DIGICAM_ERROR_ASPECTRATIO_NOT_SUPPORTED: The GStreamer
     *  Digicam has not this aspect ratio capability.
     * @G_DIGICAM_ERROR_QUALITY_NOT_SUPPORTED: These quality setting was
     *  impossible to perform.
     * @G_DIGICAM_ERROR_RESOLUTION_NOT_SUPPORTED: The GStreamer
     *  Digicam has not this resolution capability.
     * @G_DIGICAM_ERROR_LOCK_NOT_POSSIBLE: These lock combination was
     *  impossible to perform.
     * @G_DIGICAM_ERROR_ZOOM_NOT_SUPPORTED: The GStreamer Digicam has
     *  not zoom capabilities.
     * @G_DIGICAM_ERROR_ZOOM_OUT_OF_RANGE: The zoom value is out of
     *  range.
     * @G_DIGICAM_ERROR_AUDIO_NOT_SUPPORTED: These audio setting was
     *  impossible to perform.
     * @G_DIGICAM_ERROR_PREVIEW_NOT_SUPPORTED: The preview operations are
     *  not supported.
     *
     * Indicates the type of #GError.
     */
    typedef enum {                             /*< skip >*/
        G_DIGICAM_ERROR_FAILED = 0,
        G_DIGICAM_ERROR_GSTREAMER_BIN_NOT_SET,
        G_DIGICAM_ERROR_VIEWFINDER_NOT_SUPPORTED,
        G_DIGICAM_ERROR_MODE_NOT_SUPPORTED,
        G_DIGICAM_ERROR_INVALID_MODE,
        G_DIGICAM_ERROR_FLASHMODE_NOT_SUPPORTED,
        G_DIGICAM_ERROR_FOCUSMODE_NOT_SUPPORTED,
        G_DIGICAM_ERROR_INVALID_FOCUSMODE,
        G_DIGICAM_ERROR_AUTOFOCUS_LOCKED,
        G_DIGICAM_ERROR_EXPOSUREMODE_NOT_SUPPORTED,
        G_DIGICAM_ERROR_ISOSENSITIVITYMODE_NOT_SUPPORTED,
        G_DIGICAM_ERROR_WHITEBALANCEMODE_NOT_SUPPORTED,
        G_DIGICAM_ERROR_METERINGMODE_NOT_SUPPORTED,
        G_DIGICAM_ERROR_ASPECTRATIO_NOT_SUPPORTED,
        G_DIGICAM_ERROR_QUALITY_NOT_SUPPORTED,
        G_DIGICAM_ERROR_RESOLUTION_NOT_SUPPORTED,
        G_DIGICAM_ERROR_LOCK_NOT_POSSIBLE,
        G_DIGICAM_ERROR_ZOOM_NOT_SUPPORTED,
        G_DIGICAM_ERROR_ZOOM_OUT_OF_RANGE,
        G_DIGICAM_ERROR_AUDIO_NOT_SUPPORTED,
        G_DIGICAM_ERROR_PREVIEW_NOT_SUPPORTED,
    } GDigicamError;


    GQuark g_digicam_error_quark (void);

    GError* g_digicam_error_new (GDigicamError error_id,
                                 const gchar *str,
                                 ...);

    void g_digicam_set_error (GError **orig_error,
                              GDigicamError error_id,
                              const gchar *str,
                              ...);

    G_END_DECLS

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* __G_DIGICAM_ERROR_H__ */
