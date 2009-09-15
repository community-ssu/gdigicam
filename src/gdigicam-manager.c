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
 * SECTION:gdigicam-manager
 * @short_description: The manager provides a common way of accessing
 * any GStreamer based digicam like bin
 *
 * The manager provides a common way of accessing any GStreamer based
 * digicam like bin. Although it holds some automatic
 * autoconfiguration pending on the passed GStreamer bin, it
 * relays in a custom set of external functions provided with the
 * bin which will translate effectively the common manager's API
 * to actions into the GStreamer bin.
 *
 * The bin may be provided with a descriptor structure holding
 * those custom functions and a set of flags signaling the
 * capabilities of the current bin.
 *
 * #GDigicamManager is the manager object to any digicam like
 * GStreamer bin.
 **/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <locale.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <glib.h>
#include <glib/gprintf.h>
/* #include <osso-log.h> */
#include <gst/gst.h>
#include <gst/interfaces/xoverlay.h>

#include "gdigicam-marshal.h"
#include "gdigicam-error.h"
#include "gdigicam-manager.h"
#include "gdigicam-manager-private.h"
#include "gdigicam-debug.h"

/* Access to local superclass */
#define SUPER(klass)   G_OBJECT_CLASS(klass)
#define GET_SUPER(obj) G_OBJECT_GET_CLASS(obj)

/* Access to local superclass */
#define super ((GObjectClass *) parent_class)

/* Easy way to access the object class */
static gpointer parent_class = NULL;

/* Property indices */
enum
{
    PROP_0,
    PROP_LAST_PROPERTY
};

/* Signal indices */
enum
{
    /* action signals */
    /* emit signals */
    FOCUS_DONE_SIGNAL,
    PICT_DONE_SIGNAL,
    CAPTURE_START_SIGNAL,
    CAPTURE_END_SIGNAL,
    PREVIEW_SIGNAL,
    PICTURE_GOT_SIGNAL,
    INTERNAL_ERROR_SIGNAL,
    LAST_SIGNAL
};

static guint manager_signals [LAST_SIGNAL] = { 0 };

#define MIN_ZOOM 1

static guint bus_callback_source_id = 0;

/***************************************/
/* Gobject support function prototypes */
/***************************************/

static void _g_digicam_manager_class_init (GDigicamManagerClass *class);
static void _g_digicam_manager_init (GDigicamManager *manager);
static void _g_digicam_manager_finalize (GObject *object);
static GDigicamManager* _g_digicam_manager_new (void);


/***********************************************/
/* Abstract function implementation prototypes */
/***********************************************/

static GDigicamDescriptor* _g_digicam_manager_descriptor_builder (GstElement *gst_bin);
static gboolean _g_digicam_manager_bus_callback (GstBus     *bus,
                                                 GstMessage *message,
                                                 gpointer    data);
static GstBusSyncReply _g_digicam_manager_sync_bus_callback (GstBus     *bus,
                                                             GstMessage *message,
                                                             gpointer    data);
static void _g_digicam_manager_cleanup_bin (GDigicamManagerPrivate   *priv);
static void _g_digicam_manager_cleanup_sink (GDigicamManagerPrivate   *priv, GError **error);
static void _g_digicam_manager_free_private (GDigicamManagerPrivate *priv);
static void _mapping_capabilities (GstCaps *caps, GDigicamDescriptor *descriptor);
gboolean _mapping_structure  (GQuark field_id, const GValue *value, gpointer user_data);
static gboolean _picture_done (GObject *camera, const gchar *filename, gpointer user_data);

/***************************************/
/* Public functions to manage G_OBJECT */
/***************************************/

G_DEFINE_TYPE (GDigicamManager,
               _g_digicam_manager,
               G_TYPE_OBJECT);


/*****************************/
/* Public abstract functions */
/*****************************/


/**
 * g_digicam_manager_new:
 *
 * Creates a new #GDigicamManager object.
 *
 * Returns: The newly created #GDigicamManager object
 **/
GDigicamManager*
g_digicam_manager_new (void)
{
    /* FIXME: Should we do this a singleton? */
/*     static GDigicamManager *instance = NULL; */

/*     if (instance == NULL) { */
/*         instance = _g_digicam_manager_new (); */
/*     } */
    GDigicamManager *instance = NULL;

/*     instance = g_object_new (G_DIGICAM_TYPE_MANAGER, NULL); */
    instance = _g_digicam_manager_new ();

    return instance;
}


/**
 * g_digicam_manager_set_gstreamer_bin:
 * @manager: A #GDigicamManager
 * @gst_bin: The #GstElement bin which controls de digicam.
 * @descriptor: The #GDigicamDescriptor defining the #GstElement, or
 *  #NULL if we want #GDigicamManager to provide a automatic one.
 * @error: A #GError to store the result of the operation.
 *
 * Sets the #GstElement bin with which interact with the digicam and
 * stablish the internal structure with stores the digicam
 * capabilities.
 *
 * Returns: #True if success, #False otherwise.
 **/
gboolean
g_digicam_manager_set_gstreamer_bin (GDigicamManager    *manager,
                                     GstElement         *gst_bin,
                                     GDigicamDescriptor *descriptor,
                                     GError            **error)
{
    GDigicamManagerPrivate *priv = NULL;
    gboolean result = FALSE;
    gchar *error_msg = NULL;
    GDigicamError error_code = G_DIGICAM_ERROR_FAILED;

    g_return_val_if_fail (G_DIGICAM_IS_MANAGER (manager), FALSE);
    g_return_val_if_fail (GST_IS_ELEMENT (gst_bin), FALSE);


    priv = G_DIGICAM_MANAGER_GET_PRIVATE (manager);

    /* Cleaning up previous sink and bin */
    _g_digicam_manager_cleanup_sink (priv, NULL);
    _g_digicam_manager_cleanup_bin (priv);

    priv->gst_bin = gst_bin;
    gst_object_ref (GST_OBJECT (priv->gst_bin));

    if (NULL != descriptor) {
        /* FIXME: We have to copy the descriptor */
        priv->descriptor = g_digicam_manager_descriptor_copy (descriptor);
    } else {
        priv->descriptor = _g_digicam_manager_descriptor_builder (priv->gst_bin);
        if (NULL == priv->descriptor) {
            error_code = G_DIGICAM_ERROR_FAILED;
            error_msg = g_strdup ("there was an error getting "
                                  "the gst_bin capabilities");
            goto cleanup;
        }
    }

    /* FIXME: Lots of things to set once we have the gst_bin */

    priv->gst_pipeline = gst_pipeline_new (NULL);
    if (NULL == priv->gst_pipeline) {
        error_code = G_DIGICAM_ERROR_FAILED;
        error_msg = g_strdup ("impossible to create the pipeline "
                              "for the gstreamer bin");
        goto cleanup;
    }

    if (!(gst_bin_add (GST_BIN (priv->gst_pipeline),
                       GST_ELEMENT (priv->gst_bin)))) {
        error_code = G_DIGICAM_ERROR_FAILED;
        error_msg = g_strdup ("there was an error adding the bin "
                              "to the pipeline");
        goto cleanup;
    }

    gst_object_ref (GST_OBJECT (priv->gst_bin));

    priv->gst_bus = gst_pipeline_get_bus (GST_PIPELINE (priv->gst_pipeline));
    bus_callback_source_id = gst_bus_add_watch (priv->gst_bus,
                                                _g_digicam_manager_bus_callback,
                                                manager);

    /* set the handler for the messages of the sink */
    gst_bus_set_sync_handler (priv->gst_bus,
			      _g_digicam_manager_sync_bus_callback,
			      manager);
    /* FIXME: This is a mess. We have to clean it up */

    /* if the descriptor has viewfinder capabilities
       and the bin has a sink, we store it */
    if ((priv->descriptor != NULL) &&
        (priv->descriptor->supported_features &
         G_DIGICAM_CAPABILITIES_VIEWFINDER)) {
	if (NULL == priv->descriptor->viewfinder_sink) {
            GstElement *gst_element = NULL;

            gst_element = gst_bin_get_by_interface (GST_BIN (gst_bin),
                                                    GST_TYPE_X_OVERLAY);
            if (NULL == gst_element) {
                error_code = G_DIGICAM_ERROR_FAILED;
                error_msg = g_strdup ("the gst_element has not viewfinder "
                                      "capabilities but the descriptor needs them");
                goto cleanup;
            }

            descriptor->viewfinder_sink = gst_element;
            /* FIXME: Is this not really needed? */
/*             gst_object_ref (GST_OBJECT (descriptor->viewfinder_sink)); */
        }
    }

    if (0 == g_signal_lookup ("img-done", G_OBJECT_TYPE (priv->gst_bin))) {
        G_DIGICAM_DEBUG ("GDigicam: The Gst-Element has not "
                         "\"img-done\" signal\n");
    } else {
        /* Connect the picture done callback */
        g_signal_connect (G_OBJECT (priv->gst_bin),
                          "img-done",
                          (GCallback) _picture_done,
                          (gpointer) manager);
    }

    result = TRUE;

cleanup:
    if ((NULL != error) && (NULL == *error)) {
        if ((!result) && (NULL != error_msg)) {
            g_digicam_set_error (error, error_code, error_msg);
        }
    }

    /* Free */
    if (NULL != error_msg) {
        g_free (error_msg);
    }
    if (!result) {
        _g_digicam_manager_cleanup_bin (priv);
    }

    return result;
}


/**
 * g_digicam_manager_get_gstreamer_bin:
 * @manager: A #GDigicamManager
 * @gst_bin: The #GstElement bin which controls de digicam.
 * @error: A #GError to store the result of the operation.
 *
 * Gets the GstElement bin with which interact with the digicam.
 *
 * Returns: #True if success, #FALSE otherwise.
 **/
gboolean
g_digicam_manager_get_gstreamer_bin (GDigicamManager *manager,
                                     GstElement     **gst_bin,
                                     GError         **error)
{
    GDigicamManagerPrivate *priv = NULL;
    GDigicamError error_code = G_DIGICAM_ERROR_FAILED;
    gchar *error_msg = NULL;
    gboolean result = FALSE;

    g_return_val_if_fail (G_DIGICAM_IS_MANAGER (manager), FALSE);
    g_return_val_if_fail (NULL != gst_bin, FALSE);
    priv = G_DIGICAM_MANAGER_GET_PRIVATE (manager);

    /* Check GStreamer bin */
    if (NULL == priv->gst_bin) {
        error_code = G_DIGICAM_ERROR_GSTREAMER_BIN_NOT_SET;
        error_msg = g_strdup ("imposible to get the digicam descriptor "
                              "since there is no GStreamer bin "
                              "from which to get it");
        *gst_bin = NULL;
        result = FALSE;
        goto error;
    }

    /* Take a new reference of the GStreamer bin */
    *gst_bin = priv->gst_bin;
    gst_object_ref (GST_OBJECT (*gst_bin));
    result = TRUE;

error:
    if ((NULL != error) && (NULL == *error)) {
        if ((!result) && (NULL != error_msg)) {
            g_digicam_set_error (error, error_code, error_msg);
        }
    }

    /* Free */
    if (NULL != error_msg) {
        g_free (error_msg);
    }

    return result;
}


/**
 * g_digicam_manager_query_capabilities:
 * @manager: A #GDigicamManager
 * @descriptor: A pointer in which to set a copy of the
 *  #GDigicamDescriptor from the #GDigicamManager that will be have to
 *  be freed with #g_digicam_manager_descriptor_free.
 * @error: A #GError to store the result of the operation.
 *
 * Gets the digicam descriptor from the #GDigicamManager object.
 *
 * Returns: #True if success, #False otherwise.
 **/
gboolean
g_digicam_manager_query_capabilities (GDigicamManager     *manager,
                                      GDigicamDescriptor **descriptor,
                                      GError             **error)
{
    GDigicamManagerPrivate *priv = NULL;
    gboolean result = FALSE;
    gchar *error_msg = NULL;
    GDigicamError error_code = G_DIGICAM_ERROR_FAILED;

    g_return_val_if_fail (G_DIGICAM_IS_MANAGER (manager), FALSE);
    g_return_val_if_fail (NULL != descriptor, FALSE);


    priv = G_DIGICAM_MANAGER_GET_PRIVATE (manager);

    /* Check GStreamer bin */
    if (NULL == priv->gst_bin) {
        error_code = G_DIGICAM_ERROR_GSTREAMER_BIN_NOT_SET;
        error_msg = g_strdup ("imposible to get the digicam descriptor "
                              "since there is no GStreamer bin "
                              "from which to get it");
        goto error;
    }

    /* Performs operation */
    *descriptor = g_digicam_manager_descriptor_copy (priv->descriptor);

error:
    if ((NULL != error) && (NULL == *error)) {
        if ((!result) && (NULL != error_msg)) {
            g_digicam_set_error (error, error_code, error_msg);
        }
    }

    /* Free */
    if (NULL != error_msg) {
        g_free (error_msg);
    }

    return result;
}


/**
 * g_digicam_manager_set_mode:
 * @manager: A #GDigicamManager
 * @mode: The mode to set in the #GDigicamManager
 * @error: A #GError to store the result of the operation.
 * @user_data: Data to be used with the customized set function
 *  provided by the user in the #GDigicamDescriptor.
 *
 * Sets the working mode in the #GDigicamManager object.
 *
 * Returns: #True if success, #False otherwise.
 **/
gboolean
g_digicam_manager_set_mode (GDigicamManager *manager,
                            GDigicamMode     mode,
                            GError         **error,
                            gpointer         user_data)
{
    GDigicamManagerPrivate *priv = NULL;
    gboolean result = FALSE;
    gchar *error_msg = NULL;
    GDigicamError error_code = G_DIGICAM_ERROR_FAILED;

    g_return_val_if_fail (G_DIGICAM_IS_MANAGER (manager), FALSE);


    priv = G_DIGICAM_MANAGER_GET_PRIVATE (manager);

    /* Check GStreamer bin */
    if (NULL == priv->gst_bin) {
        error_code = G_DIGICAM_ERROR_GSTREAMER_BIN_NOT_SET;
        error_msg = g_strdup ("imposible to set the mode "
                              "since there is no GStreamer bin "
                              "in which set it");
        goto error;
    }

    /* Check mode capabilities */
    if (!(priv->descriptor->supported_modes & mode)) {
        error_code = G_DIGICAM_ERROR_MODE_NOT_SUPPORTED;
        error_msg = g_strdup ("imposible to set the mode "
                              "since the GStreamer bin "
                              "has not this capability");
        goto error;
    }

    /* Avoid to set the same value */
    if (priv->mode == mode) {
        result = TRUE;
        goto error;
    }

    /* Check function handler */
    if (NULL == priv->descriptor->set_mode_func) {
        error_code = G_DIGICAM_ERROR_FAILED;
        error_msg = g_strdup ("error setting the mode "
                              "in the GStreamer bin since "
                              "there is not handler function");
        goto error;
    }

    /* Perform operation */
    G_DIGICAM_DEBUG ("GDigicam: Set mode operation started\n");
    result = priv->descriptor->set_mode_func (manager, user_data);

    /* Check operation result */
    if (!result) {
        error_code = G_DIGICAM_ERROR_FAILED;
        error_msg = g_strdup_printf ("internal error setting mode %i "
                                     "in the GStreamer bin",
                                     mode);
        goto error;
    }

    /* Setting internal value */
    /* FIXME: Maybe all of this is unnecessary and we are just
     * notified from GStreamer*/
    priv->mode = mode;

    /* Set the auto values for all configuration settigns */
    priv->iso_sensitivity_mode = G_DIGICAM_ISOSENSITIVITYMODE_AUTO;
    priv->iso_level = 0;
    priv->white_balance_mode = G_DIGICAM_ISOSENSITIVITYMODE_AUTO;
    priv->white_balance_level = 0;
    priv->exposure_compensation = 0.0;

    /* Mode specific conf should me manually set */
    priv->flash_mode = G_DIGICAM_FLASHMODE_NONE;
    priv->aspect_ratio = G_DIGICAM_ASPECTRATIO_NONE;
    priv->resolution = G_DIGICAM_RESOLUTION_NONE;

error:
    if ((NULL != error) && (NULL == *error)) {
        if ((!result) && (NULL != error_msg)) {
            g_digicam_set_error (error, error_code, error_msg);
	}
    }

    /* Free */
    if (NULL != error_msg) {
        g_free (error_msg);
    }

    return result;
}


/**
 * g_digicam_manager_get_mode:
 * @manager: A #GDigicamManager
 * @mode: A pointer in which to set the mode from the #GDigicamManager
 * @error: A #GError to store the result of the operation.
 *
 * Gets the working mode from the #GDigicamManager object.
 *
 * Returns: #True if success, #FALSE otherwise.
 **/
gboolean
g_digicam_manager_get_mode (GDigicamManager *manager,
                            GDigicamMode    *mode,
                            GError         **error)
{
    GDigicamManagerPrivate *priv = NULL;
    gboolean result = FALSE;
    gchar *error_msg = NULL;
    GDigicamError error_code = G_DIGICAM_ERROR_FAILED;

    g_return_val_if_fail (G_DIGICAM_IS_MANAGER (manager), FALSE);
    g_return_val_if_fail (NULL != mode, FALSE);


    priv = G_DIGICAM_MANAGER_GET_PRIVATE (manager);

    /* Check for GStreamer bin */
    if (NULL == priv->gst_bin) {
        error_code = G_DIGICAM_ERROR_GSTREAMER_BIN_NOT_SET;
        error_msg = g_strdup ("imposible to get the mode "
                              "since there is no GStreamer bin "
                              "from which to get it");
        goto error;
    }

    /* Check mode capabilities */
    if (G_DIGICAM_MODE_NONE == priv->descriptor->supported_modes) {
        error_code = G_DIGICAM_ERROR_MODE_NOT_SUPPORTED;
        error_msg = g_strdup ("imposible to get the mode "
                              "since the GStreamer bin "
                              "has not this capability");
        goto error;
    }

    /* Perform operation */
    *mode = priv->mode;

    result = TRUE;

error:
    if ((NULL != error) && (NULL == *error)) {
        if ((!result) && (NULL != error_msg)) {
            g_digicam_set_error (error, error_code, error_msg);
	}
    }

    /* Free */
    if (NULL != error_msg) {
        g_free (error_msg);
    }

    return result;
}


/**
 * g_digicam_manager_set_flash_mode:
 * @manager: A #GDigicamManager
 * @flash_mode: The flash mode to set in the #GDigicamManager
 * @error: A #GError to store the result of the operation.
 * @user_data: Data to be used with the customized set function
 *  provided by the user in the #GDigicamDescriptor.
 *
 * Sets the flash mode in the #GDigicamManager object.
 *
 * Returns: #True if success, #False otherwise.
 **/
gboolean
g_digicam_manager_set_flash_mode (GDigicamManager   *manager,
                                  GDigicamFlashmode  flash_mode,
                                  GError           **error,
                                  gpointer           user_data)
{
    GDigicamManagerPrivate *priv = NULL;
    gboolean result = FALSE;
    gchar *error_msg = NULL;
    GDigicamError error_code = G_DIGICAM_ERROR_FAILED;

    g_return_val_if_fail (G_DIGICAM_IS_MANAGER (manager), FALSE);


    priv = G_DIGICAM_MANAGER_GET_PRIVATE (manager);

    /* Check GStreamer bin */
    if (NULL == priv->gst_bin) {
        error_code = G_DIGICAM_ERROR_GSTREAMER_BIN_NOT_SET;
        error_msg = g_strdup ("imposible to set the flash mode "
                              "since there is no GStreamer bin "
                              "in which set it");
        goto error;
    }

    /* Check still picture mode */
    if (priv->mode != G_DIGICAM_MODE_STILL) {
        error_code = G_DIGICAM_ERROR_INVALID_MODE;
        error_msg = g_strdup ("imposible to set flash mode "
                              "since camera is not in still picture "
                              "mode, required for this operation.");
        goto error;
    }

    /* Check flash capability */
    if (!(priv->descriptor->supported_features &
          G_DIGICAM_CAPABILITIES_FLASH)) {
        error_code = G_DIGICAM_ERROR_FLASHMODE_NOT_SUPPORTED;
        error_msg = g_strdup ("imposible to set the flash mode "
                              "since the GStreamer bin "
                              "has not flash capability");
        goto error;
    }

    /* Check flash mode capability */
    if (!(priv->descriptor->supported_flash_modes &
          flash_mode)) {
        error_code = G_DIGICAM_ERROR_FLASHMODE_NOT_SUPPORTED;
        error_msg = g_strdup ("imposible to set the flash mode "
                              "since the GStreamer bin "
                              "has not this flash mode capability");
        goto error;
    }

    /* Avoid to set the same value */
    if (priv->flash_mode == flash_mode) {
        result = TRUE;
        goto error;
    }

    /* Check function handler */
    if (NULL == priv->descriptor->set_flash_mode_func) {
        error_code = G_DIGICAM_ERROR_FAILED;
        error_msg = g_strdup ("error setting the flash mode "
                              "in the GStreamer bin since "
                              "there is not handler function");
        goto error;
    }

    /* Perform operation */
    G_DIGICAM_DEBUG ("GDigicam: Set flash mode started\n");
    result = priv->descriptor->set_flash_mode_func (manager, user_data);

    /* Check operation result */
    if (!result) {
        error_code = G_DIGICAM_ERROR_FAILED;
        error_msg = g_strdup_printf ("internal error setting flash mode %i "
                                     "in the GStreamer bin",
                                     flash_mode);
        goto error;
    }

    /* Setting internal value */
    /* FIXME: Maybe all of this is unnecessary and we are just
     * notified from GStreamer*/
    priv->flash_mode = flash_mode;

error:
    if ((NULL != error) && (NULL == *error)) {
        if ((!result) && (NULL != error_msg)) {
            g_digicam_set_error (error, error_code, error_msg);
	}
    }

    /* Free */
    if (NULL != error_msg) {
        g_free (error_msg);
    }

    return result;
}


/**
 * g_digicam_manager_get_flash_mode:
 * @manager: A #GDigicamManager
 * @flash_mode: A pointer in which to set the flash mode from the
 * #GDigicamManager
 * @error: A #GError to store the result of the operation.
 *
 * Gets the flash mode from the #GDigicamManager object.
 *
 * Returns: #True if success, #FALSE otherwise.
 **/
gboolean
g_digicam_manager_get_flash_mode (GDigicamManager   *manager,
                                  GDigicamFlashmode *flash_mode,
                                  GError           **error)
{
    GDigicamManagerPrivate *priv = NULL;
    gboolean result = FALSE;
    gchar *error_msg = NULL;
    GDigicamError error_code = G_DIGICAM_ERROR_FAILED;

    g_return_val_if_fail (G_DIGICAM_IS_MANAGER (manager), FALSE);
    g_return_val_if_fail (NULL != flash_mode, FALSE);


    priv = G_DIGICAM_MANAGER_GET_PRIVATE (manager);

    /* Check for GStreamer bin */
    if (NULL == priv->gst_bin) {
        error_code = G_DIGICAM_ERROR_GSTREAMER_BIN_NOT_SET;
        error_msg = g_strdup ("imposible to get the flash mode "
                              "since there is no GStreamer bin "
                              "from which to get it");
        goto error;
    }

    /* Check still picture mode */
    if (priv->mode != G_DIGICAM_MODE_STILL) {
        error_code = G_DIGICAM_ERROR_INVALID_MODE;
        error_msg = g_strdup ("imposible to get the flash mode "
                              "since camera is not in still picture "
                              "mode, required for this operation.");
        goto error;
    }

    /* Check flash capability */
    if (!(priv->descriptor->supported_features &
          G_DIGICAM_CAPABILITIES_FLASH)) {
        error_code = G_DIGICAM_ERROR_FLASHMODE_NOT_SUPPORTED;
        error_msg = g_strdup ("imposible to get the flash mode "
                              "since the GStreamer bin has not "
                              "this capability");
        goto error;
    }

    /* Perform operation */
    *flash_mode = priv->flash_mode;

    result = TRUE;

error:
    if ((NULL != error) && (NULL == *error)) {
        if ((!result) && (NULL != error_msg)) {
            g_digicam_set_error (error, error_code, error_msg);
	}
    }

    /* Free */
    if (NULL != error_msg) {
        g_free (error_msg);
    }

    return result;
}

/*     gboolean g_digicam_manager_is_flash_ready (GDigicamManager   *manager, */
/*                                                gboolean          *flash_ready, */
/*                                                GError           **error); */

/**
 * g_digicam_manager_set_focus_mode:
 * @manager: A #GDigicamManager
 * @focus_mode: the #GDigicamFocusmode value to set.
 * @macro_enabled: wether macro focus is enabled or not.
 * @error: A #GError to store the result of the operation.
 * @user_data: Data to be used with the customized set function
 * provided by the user in the #GDigicamDescriptor.
 *
 * Sets the current focus mode and autofocus algorithm,
 * in case of automatic focus setting.
 *
 * Returns: #True if success, #False otherwise.
 **/
gboolean
g_digicam_manager_set_focus_mode (GDigicamManager   *manager,
                                  GDigicamFocusmode  focus_mode,
                                  gboolean           macro_enabled,
                                  GError           **error,
                                  gpointer           user_data)
{
    GDigicamManagerPrivate *priv = NULL;
    gboolean supported = TRUE;
    gboolean result = FALSE;
    gchar *error_msg = NULL;
    GDigicamError error_code = G_DIGICAM_ERROR_FAILED;

    g_return_val_if_fail (G_DIGICAM_IS_MANAGER (manager), FALSE);


    priv = G_DIGICAM_MANAGER_GET_PRIVATE (manager);

    /* Check GStreamer bin */
    if (NULL == priv->gst_bin) {
        error_code = G_DIGICAM_ERROR_GSTREAMER_BIN_NOT_SET;
        error_msg = g_strdup ("imposible to set focus mode"
                              "since there is no GStreamer bin "
                              "in which set it");
        goto error;
    }

    /* Check macro focus */
    if (macro_enabled) {
        if (!(priv->descriptor->supported_features &
              G_DIGICAM_CAPABILITIES_MACROFOCUS)) {
            error_code = G_DIGICAM_ERROR_FOCUSMODE_NOT_SUPPORTED;
            error_msg = g_strdup ("imposible to set macro focus "
                                  "since the GStreamer bin "
                                  "has not this capability.");
            goto error;
        }
    }

    /* Check focus capabilities  */
    switch (focus_mode) {
    case G_DIGICAM_FOCUSMODE_MANUAL:
        supported = (priv->descriptor->supported_features &
                     G_DIGICAM_CAPABILITIES_MANUALFOCUS);
        break;
    case G_DIGICAM_FOCUSMODE_AUTO:
    case G_DIGICAM_FOCUSMODE_FACE:
    case G_DIGICAM_FOCUSMODE_SMILE:
    case G_DIGICAM_FOCUSMODE_CENTROID:
        supported = (priv->descriptor->supported_features &
                     G_DIGICAM_CAPABILITIES_AUTOFOCUS);

        break;
    case G_DIGICAM_FOCUSMODE_CONTINUOUSAUTO:
    case G_DIGICAM_FOCUSMODE_CONTINUOUSCENTROID:
        supported = (priv->descriptor->supported_features &
                     G_DIGICAM_CAPABILITIES_CONTINUOUSAUTOFOCUS);

        break;
    default:
        break;
    }
    if (!supported) {
        error_code = G_DIGICAM_ERROR_FOCUSMODE_NOT_SUPPORTED;
        error_msg = g_strdup_printf ("imposible to set focus mode %i "
                                     "since the GStreamer bin "
                                     "has not this capability.",
                                     focus_mode);
        goto error;
    }

    /* Check locked focus */
    if (priv->locks & G_DIGICAM_LOCK_AUTOFOCUS) {
        error_code = G_DIGICAM_ERROR_AUTOFOCUS_LOCKED;
        error_msg = g_strdup ("imposible to set focus mode "
                              "since the autofocus is locked.");
        goto error;
    }

    /* Avoid to set the same value */
    if ((priv->focus_mode == focus_mode) &&
        (priv->is_macro_enabled == macro_enabled)) {
        result = TRUE;
        goto error;
    }

    /* Check function handler */
    if (NULL == priv->descriptor->set_focus_mode_func) {
        error_code = G_DIGICAM_ERROR_FAILED;
        error_msg = g_strdup ("error setting the focus mode "
                              "in the GStreamer bin since "
                              "there is not handler function");
        goto error;
    }

    /* Perform operation */
    G_DIGICAM_DEBUG ("GDigicam: Set focus mode started\n");
    result = priv->descriptor->set_focus_mode_func (manager, user_data);

    /* Check operation result */
    if (!result) {
        error_code = G_DIGICAM_ERROR_FAILED;
        error_msg = g_strdup_printf ("internal error setting focus mode %i "
                                     "in the GStreamer bin",
                                     focus_mode);
        goto error;
    }

    /* Setting internal value */
    /* FIXME: Maybe all of this is unnecessary and we are just
     * notified from GStreamer*/
    priv->focus_mode = focus_mode;
    priv->is_macro_enabled = macro_enabled;

error:
    if ((NULL != error) && (NULL == *error)) {
        if ((!result) && (NULL != error_msg)) {
            g_digicam_set_error (error, error_code, error_msg);
        }
    }

    /* Free */
    if (NULL != error_msg) {
        g_free(error_msg);
    }

    return result;
}

/**
 * g_digicam_manager_get_focus_mode:
 * @manager: A #GDigicamManager
 * @focus_mode: the #GDigicamFocusmode value to get.
 * @macro_enabled: wether macro focus is enabled or not.
 * @error: A #GError to store the result of the operation.
 *
 * Gets the current focus mode and autofocus algorithm,
 * in case of automatic focus setting.
 *
 * Returns: #True if success, #FALSE otherwise.
 **/
gboolean
g_digicam_manager_get_focus_mode (GDigicamManager   *manager,
                                  GDigicamFocusmode *focus_mode,
                                  gboolean          *macro_enabled,
                                  GError           **error)
{
    GDigicamManagerPrivate *priv = NULL;
    gboolean result = FALSE;
    gchar *error_msg = NULL;
    GDigicamError error_code = G_DIGICAM_ERROR_FAILED;

    g_return_val_if_fail (G_DIGICAM_IS_MANAGER (manager), FALSE);
    g_return_val_if_fail (NULL != focus_mode, FALSE);
    g_return_val_if_fail (NULL != macro_enabled, FALSE);


    priv = G_DIGICAM_MANAGER_GET_PRIVATE (manager);

    /* Check for GStreamer bin */
    if (NULL == priv->gst_bin) {
        error_code = G_DIGICAM_ERROR_GSTREAMER_BIN_NOT_SET;
        error_msg = g_strdup ("imposible to get the focus mode "
                              "since there is no GStreamer bin "
                              "from which to get it");
        goto error;
    }

    /* Check still picture mode */
    if (priv->mode != G_DIGICAM_MODE_STILL) {
        error_code = G_DIGICAM_ERROR_INVALID_MODE;
        error_msg = g_strdup ("imposible to get the focus mode "
                              "since camera is not in still picture "
                              "mode, required for this operation.");
        goto error;
    }

    /* Check flash capability */
    if (!(priv->descriptor->supported_features &
          (G_DIGICAM_CAPABILITIES_MANUALFOCUS |
           G_DIGICAM_CAPABILITIES_AUTOFOCUS |
           G_DIGICAM_CAPABILITIES_CONTINUOUSAUTOFOCUS))) {
        error_code = G_DIGICAM_ERROR_FOCUSMODE_NOT_SUPPORTED;
        error_msg = g_strdup ("imposible to get macro focus "
                              "since the GStreamer bin "
                              "has not this capability.");
        goto error;
    }

    /* Perform operation */
    *focus_mode = priv->focus_mode;
    *macro_enabled = priv->is_macro_enabled;

    result = TRUE;

error:
    if ((NULL != error) && (NULL == *error)) {
        if ((!result) && (NULL != error_msg)) {
            g_digicam_set_error (error, error_code, error_msg);
	}
    }

    /* Free */
    if (NULL != error_msg) {
        g_free (error_msg);
    }

    return result;
}

/**
 * g_digicam_manager_set_focus_region_pattern:
 * @manager: A #GDigicamManager
 * @focus_points: the #GDigicamFocuspoints points available.
 * @active_points: active points used to manually focus.
 * @error: A #GError to store the result of the operation.
 * @user_data: Data to be used with the customized set function
 *  provided by the user in the #GDigicamDescriptor.
 *
 * Sets an active region in the image matrix for setting
 * a focus pattern manually.
 *
 * Returns: #True if success, #False otherwise.
 **/
gboolean
g_digicam_manager_set_focus_region_pattern (GDigicamManager    *manager,
                                            GDigicamFocuspoints focus_points,
                                            guint64             active_points,
                                            GError            **error,
                                            gpointer            user_data)

{
    GDigicamManagerPrivate *priv = NULL;
    gboolean result = FALSE;
    gchar *error_msg = NULL;
    GDigicamError error_code = G_DIGICAM_ERROR_FAILED;

    g_return_val_if_fail (G_DIGICAM_IS_MANAGER (manager), FALSE);


    priv = G_DIGICAM_MANAGER_GET_PRIVATE (manager);

    /* Check GStreamer bin */
    if (NULL == priv->gst_bin) {
        error_code = G_DIGICAM_ERROR_GSTREAMER_BIN_NOT_SET;
        error_msg = g_strdup ("imposible to set focus region pattern "
                              "since there is no GStreamer bin "
                              "in which set it");
        goto error;
    }

    /* Check still picture mode */
    if (priv->mode != G_DIGICAM_MODE_STILL) {
        error_code = G_DIGICAM_ERROR_INVALID_MODE;
        error_msg = g_strdup ("imposible to set focus region pattern "
                              "since camera is not in still picture "
                              "mode, required for this operation.");
        goto error;
    }

    /* Check focus mode capabilities */
    if (!(priv->descriptor->supported_features &
          G_DIGICAM_CAPABILITIES_MANUALFOCUS)) {
        error_code = G_DIGICAM_ERROR_FOCUSMODE_NOT_SUPPORTED;
        error_msg = g_strdup ("imposible to set focus region pattern "
                              "since the GStreamer bin "
                              "has not this capability.");
        goto error;
    }

    /* Check current focus mode */
    if (priv->focus_mode != G_DIGICAM_CAPABILITIES_MANUALFOCUS) {
        error_code = G_DIGICAM_ERROR_INVALID_FOCUSMODE;
        error_msg = g_strdup ("imposible to set focus region pattern "
                              "since the focus mode is not set to manual");
        goto error;
    }

    /* Avoid to set the same value */
    if ((priv->focus_points == focus_points) &&
        (priv->active_points == active_points)) {
        result = TRUE;
        goto error;
    }

    /* Check function handler */
    if (NULL == priv->descriptor->set_focus_region_pattern_func) {
        error_code = G_DIGICAM_ERROR_FAILED;
        error_msg = g_strdup ("error setting the focus region pattern "
                              "in the GStreamer bin since "
                              "there is not handler function");
        goto error;
    }

    /* Perform operation */
    G_DIGICAM_DEBUG ("GDigicam: Set focus region pattern started\n");
    result = priv->descriptor->set_focus_region_pattern_func (manager,
                                                              user_data);

    /* Check operation result */
    if (!result) {
        error_code = G_DIGICAM_ERROR_FAILED;
        error_msg = g_strdup_printf ("internal error setting focus "
                                     "region pattern to %d focus points "
                                     "and %llu active points"
                                     "in the GStreamer bin",
                                     focus_points,
                                     active_points);
        goto error;
    }

    /* Setting internal value */
    /* FIXME: Maybe all of this is unnecessary and we are just
     * notified from GStreamer*/
    priv->focus_points = focus_points;
    priv->active_points = active_points;

error:
    if ((NULL != error) && (NULL == *error)) {
        if ((!result) && (NULL != error_msg)) {
            g_digicam_set_error (error, error_code, error_msg);
        }
    }

    /* Free */
    if (NULL != error_msg) {
        g_free(error_msg);
    }

    return result;
}

/**
 * g_digicam_manager_get_focus_region_pattern:
 * @manager: A #GDigicamManager
 * @focus_points: the #GDigicamFocuspoints focus points.
 * @active_points: the active focus matrix points.
 * @error: A #GError to store the result of the operation.
 *
 * Gets the current activated focus region pattern, used
 * with the manual focus capability.
 *
 * Returns: #True if success, #FALSE otherwise.
 **/
gboolean
g_digicam_manager_get_focus_region_pattern (GDigicamManager     *manager,
                                            GDigicamFocuspoints *focus_points,
                                            guint64             *active_points,
                                            GError             **error)
{
    GDigicamManagerPrivate *priv = NULL;
    gboolean result = FALSE;
    gchar *error_msg = NULL;
    GDigicamError error_code = G_DIGICAM_ERROR_FAILED;

    g_return_val_if_fail (G_DIGICAM_IS_MANAGER (manager), FALSE);
    g_return_val_if_fail (NULL != focus_points, FALSE);
    g_return_val_if_fail (NULL != active_points, FALSE);


    priv = G_DIGICAM_MANAGER_GET_PRIVATE (manager);

    /* Check GStreamer bin */
    if (NULL == priv->gst_bin) {
        error_code = G_DIGICAM_ERROR_GSTREAMER_BIN_NOT_SET;
        error_msg = g_strdup ("imposible to get focus region pattern "
                              "since there is no GStreamer bin "
                              "from which to get it");
        goto error;
    }

    /* Check still picture mode */
    if (priv->mode != G_DIGICAM_MODE_STILL) {
        error_code = G_DIGICAM_ERROR_INVALID_MODE;
        error_msg = g_strdup ("imposible to get focus region pattern "
                              "since camera is not in still picture "
                              "mode, required for this operation.");
        goto error;
    }

    /* Check focus mode capabilities */
    if (!(priv->descriptor->supported_features &
          G_DIGICAM_CAPABILITIES_MANUALFOCUS)) {
        error_code = G_DIGICAM_ERROR_FOCUSMODE_NOT_SUPPORTED;
        error_msg = g_strdup ("imposible to get focus region pattern "
                              "since the GStreamer bin "
                              "has not this capability.");
        goto error;
    }

    /* Check current focus mode */
    if (priv->focus_mode != G_DIGICAM_CAPABILITIES_MANUALFOCUS) {
        error_code = G_DIGICAM_ERROR_INVALID_FOCUSMODE;
        error_msg = g_strdup ("imposible to get focus region pattern "
                              "since the focus mode is not set to manual");
        goto error;
    }

    /* Perform operation */
    *focus_points = priv->focus_points;
    *active_points = priv->active_points;

    result = TRUE;

error:
    if ((NULL != error) && (NULL == *error)) {
        if ((!result) && (NULL != error_msg)) {
            g_digicam_set_error (error, error_code, error_msg);
	}
    }

    /* Free */
    if (NULL != error_msg) {
        g_free (error_msg);
    }

    return result;
}

/*     gboolean g_digicam_manager_get_focus_region_positions (GDigicamManager   *manager, */
/*                                                            GSList *slist, */
/*                                                            GError           **error); */
/*     gboolean g_digicam_manager_get_focus_mode_status (GDigicamManager   *manager, */
/*                                                       GDigicamFocusmodestatus *focus_mode_status, */
/*                                                       guint64 *region_status, */
/*                                                       GError           **error); */

/**
 * g_digicam_manager_set_exposure_mode:
 * @manager: A #GDigicamManager
 * @exposure_mode: The exposure mode to set in the #GDigicamManager
 * @conf: The exposure mode specific configuration to set automatically.
 * @error: A #GError to store the result of the operation.
 * @user_data: Data to be used with the customized set function
 *  provided by the user in the #GDigicamDescriptor.
 *
 * Sets the exposure mode in the #GDigicamManager object.
 *
 * Returns: #True if success, #False otherwise.
 **/
gboolean
g_digicam_manager_set_exposure_mode (GDigicamManager            *manager,
                                     GDigicamExposuremode       exposure_mode,
                                     const GDigicamExposureconf *conf,
                                     GError                     **error,
                                     gpointer                   user_data)
{
    GDigicamManagerPrivate *priv = NULL;
    gboolean result = FALSE;
    gchar *error_msg = NULL;
    GDigicamError error_code = G_DIGICAM_ERROR_FAILED;

    g_return_val_if_fail (G_DIGICAM_IS_MANAGER (manager), FALSE);
    priv = G_DIGICAM_MANAGER_GET_PRIVATE (manager);

    /* Check GStreamer bin */
    if (NULL == priv->gst_bin) {
        error_code = G_DIGICAM_ERROR_GSTREAMER_BIN_NOT_SET;
        error_msg = g_strdup ("imposible to set the exposure mode "
                              "since there is no GStreamer bin "
                              "in which set it");
        goto error;
    }

    /* Check exposure capabilities */
    if (!(priv->descriptor->supported_features &
          G_DIGICAM_CAPABILITIES_AUTOEXPOSURE)) {
        error_code = G_DIGICAM_ERROR_EXPOSUREMODE_NOT_SUPPORTED;
        error_msg = g_strdup ("imposible to set "
                              "the exposure mode "
                              "since the GStreamer bin "
                              "has not this capability");
        goto error;
    }

/*     /\* Check exposure mode values *\/ */
/*     if (priv->mode == G_DIGICAM_MODE_VIDEO) { */
/*         switch (exposure_mode) { */
/*         case G_DIGICAM_EXPOSUREMODE_AUTO: */
/*         case G_DIGICAM_EXPOSUREMODE_NIGHT: */
/*             break; */
/*         default: */
/*             error_code = G_DIGICAM_ERROR_EXPOSUREMODE_NOT_SUPPORTED; */
/*             error_msg = g_strdup ("imposible to set " */
/*                                   "the exposure mode " */
/*                                   "since the GStreamer bin " */
/*                                   "is in Video mode"); */
/*             goto error; */
/*         } */
/*     } */

    /* Check function handler */
    if (NULL == priv->descriptor->set_exposure_mode_func) {
        error_code = G_DIGICAM_ERROR_FAILED;
        error_msg = g_strdup ("error setting the exposure mode "
                              "in the GStreamer bin since "
                              "there is not handler function");
        goto error;
    }

    /* Perform operation */
    G_DIGICAM_DEBUG ("GDigicam: Set exposure mode started\n");
    result = priv->descriptor->set_exposure_mode_func (manager, user_data);

    /* Check operation result */
    if (!result) {
        error_code = G_DIGICAM_ERROR_FAILED;
        error_msg = g_strdup_printf ("internal error setting the "
                                     "%d exposure mode "
                                     "in the GStreamer bin",
                                     exposure_mode);
        goto error;
    }

    /* Setting internal value */
    /* FIXME: Maybe all of this is unnecessary and we are just
     * notified from GStreamer*/
    priv->exposure_mode = exposure_mode;

    /* Set the associated values for specific exposure mode */
    if (conf != NULL) {
        priv->iso_sensitivity_mode = conf->iso_sensitivity_mode;
        priv->iso_level = conf->iso_level;
        priv->white_balance_mode = conf->white_balance_mode;
        priv->white_balance_level = conf->white_balance_level;
        priv->exposure_compensation = conf->white_balance_level;
        priv->flash_mode = conf->flash_mode;
    }

error:
    if ((NULL != error) && (NULL == *error)) {
        if ((!result) && (NULL != error_msg)) {
            g_digicam_set_error (error, error_code, error_msg);
        }
    }

    /* Free */
    if (NULL != error_msg) {
        g_free(error_msg);
    }

    return result;
}


/**
 * g_digicam_manager_get_exposure_mode:
 * @manager: A #GDigicamManager
 * @exposure_mode: A pointer in which to set the exposure mode from
 *  the #GDigicamManager
 * @error: A #GError to store the result of the operation.
 *
 * Gets the exposure mode from the #GDigicamManager object.
 *
 * Returns: #True if success, #FALSE otherwise.
 **/
gboolean
g_digicam_manager_get_exposure_mode (GDigicamManager      *manager,
                                     GDigicamExposuremode *exposure_mode,
                                     GError              **error)
{
    GDigicamManagerPrivate *priv = NULL;
    gboolean result = FALSE;
    gchar *error_msg = NULL;
    GDigicamError error_code = G_DIGICAM_ERROR_FAILED;

    g_return_val_if_fail (G_DIGICAM_IS_MANAGER (manager), FALSE);
    g_return_val_if_fail (NULL != exposure_mode, FALSE);
    priv = G_DIGICAM_MANAGER_GET_PRIVATE (manager);

    /* Check GStreamer bin */
    if (NULL == priv->gst_bin) {
        error_code = G_DIGICAM_ERROR_GSTREAMER_BIN_NOT_SET;
        error_msg = g_strdup ("imposible to get the exposure mode "
                              "since there is no GStreamer bin "
                              "in which set it");
        goto error;
    }

    /* Check exposure capabilities */
    if (!(priv->descriptor->supported_features &
          (G_DIGICAM_CAPABILITIES_AUTOEXPOSURE |
           G_DIGICAM_CAPABILITIES_MANUALEXPOSURE))) {
        error_code = G_DIGICAM_ERROR_EXPOSUREMODE_NOT_SUPPORTED;
        error_msg = g_strdup ("imposible to get "
                              "the exposure mode "
                              "since the GStreamer bin "
                              "has not this capability");
        goto error;
    }

    /* Perform operation */
    *exposure_mode = priv->exposure_mode;

    result = TRUE;

error:
    if ((NULL != error) && (NULL == *error)) {
        if ((!result) && (NULL != error_msg)) {
            g_digicam_set_error (error, error_code, error_msg);
	}
    }

    /* Free */
    if (NULL != error_msg) {
        g_free (error_msg);
    }

    return result;
}


/**
 * g_digicam_manager_set_exposure_comp:
 * @manager: A #GDigicamManager
 * @exposure_comp: The exposure compensation value to set in the
 *  #GDigicamManager
 * @error: A #GError to store the result of the operation.
 * @user_data: Data to be used with the customized set function
 *  provided by the user in the #GDigicamDescriptor.
 *
 * Sets the exposure compensation value in the #GDigicamManager object.
 *
 * Returns: #True if success, #False otherwise.
 **/
gboolean
g_digicam_manager_set_exposure_comp (GDigicamManager *manager,
                                     gdouble          exposure_comp,
                                     GError         **error,
                                     gpointer         user_data)
{
    GDigicamManagerPrivate *priv = NULL;
    gboolean result = FALSE;
    gchar *error_msg = NULL;
    GDigicamError error_code = G_DIGICAM_ERROR_FAILED;

    g_return_val_if_fail (G_DIGICAM_IS_MANAGER (manager), FALSE);
    priv = G_DIGICAM_MANAGER_GET_PRIVATE (manager);

    /* Check GStreamer bin */
    if (NULL == priv->gst_bin) {
        error_code = G_DIGICAM_ERROR_GSTREAMER_BIN_NOT_SET;
        error_msg = g_strdup ("imposible to set the exposure compensation "
                              "since there is no GStreamer bin "
                              "in which set it");
        goto error;
    }

    /* Avoid to set the same value */
    if (priv->exposure_compensation == exposure_comp) {
        result = TRUE;
        goto error;
    }

    /* Check function handler */
    if (NULL == priv->descriptor->set_exposure_comp_func) {
        error_code = G_DIGICAM_ERROR_FAILED;
        error_msg = g_strdup ("error setting the exposure compensation "
                              "in the GStreamer bin since "
                              "there is not handler function");
        goto error;
    }

    /* Perform operation */
    G_DIGICAM_DEBUG ("GDigicam: Set exposure compensation started\n");
    result = priv->descriptor->set_exposure_comp_func (manager, user_data);

    /* Check operation result */
    if (!result) {
        error_code = G_DIGICAM_ERROR_FAILED;
        error_msg = g_strdup_printf ("internal error setting the "
                                     "%f as exposure compensation "
                                     "in the GStreamer bin",
                                     exposure_comp);
        goto error;
    }

    /* Setting internal value */
    /* FIXME: Maybe all of this is unnecessary and we are just
     * notified from GStreamer*/
    priv->exposure_compensation = exposure_comp;

error:
    if ((NULL != error) && (NULL == *error)) {
        if ((!result) && (NULL != error_msg)) {
            g_digicam_set_error (error, error_code, error_msg);
        }
    }

    /* Free */
    if (NULL != error_msg) {
        g_free(error_msg);
    }

    return result;
}


/**
 * g_digicam_manager_get_exposure_comp:
 * @manager: A #GDigicamManager
 * @exposure_comp: A pointer in which to set the exposure compensation
 * value.
 * @error: A #GError to store the result of the operation.
 *
 * Gets the exposure compensation from the #GDigicamManager object.
 *
 * Returns: #True if success, #FALSE otherwise.
 **/
gboolean
g_digicam_manager_get_exposure_comp (GDigicamManager      *manager,
                                     gdouble              *exposure_comp,
                                     GError              **error)
{
    GDigicamManagerPrivate *priv = NULL;
    gboolean result = FALSE;
    gchar *error_msg = NULL;
    GDigicamError error_code = G_DIGICAM_ERROR_FAILED;

    g_return_val_if_fail (G_DIGICAM_IS_MANAGER (manager), FALSE);
    g_return_val_if_fail (NULL != exposure_comp, FALSE);
    priv = G_DIGICAM_MANAGER_GET_PRIVATE (manager);

    /* Check GStreamer bin */
    if (NULL == priv->gst_bin) {
        error_code = G_DIGICAM_ERROR_GSTREAMER_BIN_NOT_SET;
        error_msg = g_strdup ("imposible to get the exposure compensation "
                              "since there is no GStreamer bin "
                              "in which set it");
        goto error;
    }

    /* Check exposure capabilities */
    if (!(priv->descriptor->supported_features &
          (G_DIGICAM_CAPABILITIES_AUTOEXPOSURE |
           G_DIGICAM_CAPABILITIES_MANUALEXPOSURE))) {
        error_code = G_DIGICAM_ERROR_EXPOSUREMODE_NOT_SUPPORTED;
        error_msg = g_strdup ("imposible to get "
                              "the exposure compensation "
                              "since the GStreamer bin "
                              "has not this capability");
        goto error;
    }

    /* Perform operation */
    *exposure_comp = priv->exposure_compensation;

    result = TRUE;

error:
    if ((NULL != error) && (NULL == *error)) {
        if ((!result) && (NULL != error_msg)) {
            g_digicam_set_error (error, error_code, error_msg);
	}
    }

    /* Free */
    if (NULL != error_msg) {
        g_free (error_msg);
    }

    return result;
}


/**
 * g_digicam_manager_set_iso_sensitivity_mode:
 * @manager: A #GDigicamManager
 * @iso_sensitivity_mode: The ISO sensitivity mode to set in the #GDigicamManager
 * @iso_level: used when #G_DIGICAM_ISOSENSITIVITYMODE_MANUAL is set.
 * @error: A #GError to store the result of the operation.
 * @user_data: Data to be used with the customized set function
 *  provided by the user in the #GDigicamDescriptor.
 *
 * Sets the ISO sensitivity mode in the #GDigicamManager object.
 *
 * Returns: #True if success, #False otherwise.
 **/
gboolean
g_digicam_manager_set_iso_sensitivity_mode (GDigicamManager           *manager,
                                            GDigicamIsosensitivitymode iso_sensitivity_mode,
                                            guint                      iso_level,
                                            GError                   **error,
                                            gpointer                   user_data)
{
    GDigicamManagerPrivate *priv = NULL;
    gboolean result = FALSE;
    gchar *error_msg = NULL;
    GDigicamError error_code = G_DIGICAM_ERROR_FAILED;

    g_return_val_if_fail (G_DIGICAM_IS_MANAGER (manager), FALSE);


    priv = G_DIGICAM_MANAGER_GET_PRIVATE (manager);

    /* Check for GStreamer bin */
    if (NULL == priv->gst_bin) {
        error_code = G_DIGICAM_ERROR_GSTREAMER_BIN_NOT_SET;
        error_msg = g_strdup ("imposible to set iso sensitivity mode"
                              "since there is no GStreamer bin "
                              "in which set it");
        goto error;
    }

    /* FIXME: Not sure if we should check this */
/*     /\* Check still picture mode *\/ */
/*     if (priv->mode != G_DIGICAM_MODE_STILL) { */
/*         error_code = G_DIGICAM_ERROR_INVALID_MODE; */
/*         error_msg = g_strdup ("imposible to set the iso sensitivity " */
/*                               "since camera is not in still picture " */
/*                               "mode, required for this operation."); */
/*         goto error; */
/*     } */

    /* Check iso sensitivity capabilities */
    if (iso_sensitivity_mode == G_DIGICAM_ISOSENSITIVITYMODE_MANUAL) {
        if (!(priv->descriptor->supported_features &
              G_DIGICAM_CAPABILITIES_MANUALISOSENSITIVITY)) {
            error_code = G_DIGICAM_ERROR_ISOSENSITIVITYMODE_NOT_SUPPORTED;
            error_msg = g_strdup ("imposible to set the iso sensitivity "
                                  "since the GStreamer bin "
                                  "has not this capability");
            goto error;
        }
    } else {
        if (!(priv->descriptor->supported_features &
              G_DIGICAM_CAPABILITIES_AUTOISOSENSITIVITY)) {
            error_code = G_DIGICAM_ERROR_ISOSENSITIVITYMODE_NOT_SUPPORTED;
            error_msg = g_strdup ("imposible to set the iso sensitivity "
                                  "since the GStreamer bin "
                                  "has not this capability");
            goto error;
        }
    }

    /* Avoid to set the same value */
    if (iso_sensitivity_mode == priv->iso_sensitivity_mode) {
        result = TRUE;
        if (iso_sensitivity_mode !=
            G_DIGICAM_ISOSENSITIVITYMODE_MANUAL) {
            result = TRUE;
            goto error;
        }
        if (iso_level == priv->iso_level) {
            goto error;
        }
    }

    /* Check function handler */
    if (NULL == priv->descriptor->set_iso_sensitivity_mode_func) {
        error_code = G_DIGICAM_ERROR_FAILED;
        error_msg = g_strdup ("error setting the iso sensitivity mode "
                              "in the GStreamer bin since "
                              "there is not handler function");
        goto error;
    }

    /* Perform operation */
    G_DIGICAM_DEBUG ("GDigicam: Set iso sensitivity mode started\n");
    result = priv->descriptor->set_iso_sensitivity_mode_func (manager, user_data);

    /* Check operation result */
    if (!result) {
        error_code = G_DIGICAM_ERROR_FAILED;
        error_msg = g_strdup_printf ("internal error setting the "
                                     "%d iso sensitivity mode and the "
                                     "%d iso level"
                                     "in the GStreamer bin",
                                     iso_sensitivity_mode,
                                     iso_level);
        goto error;
    }

    /* Setting internal value */
    /* FIXME: Maybe all of this is unnecessary and we are just
     * notified from GStreamer*/
    priv->iso_sensitivity_mode = iso_sensitivity_mode;
    priv->iso_level = iso_level;

error:
    if ((NULL != error) && (NULL == *error)) {
        if ((!result) && (NULL != error_msg)) {
            g_digicam_set_error (error, error_code, error_msg);
        }
    }

    /* Free */
    if (NULL != error_msg) {
        g_free(error_msg);
    }

    return result;
}

/**
 * g_digicam_manager_get_iso_sensitivity_mode:
 * @manager: A #GDigicamManager
 * @iso_sensitivity_mode: A pointer in which to set the ISO sensitivity mode from the
 * @iso_level: used when #G_DIGICAM_ISOSENSITIVITYMODE_MANUAL is set.
 * #GDigicamManager
 * @error: A #GError to store the result of the operation.
 *
 * Gets the ISO sensitivity mode from the #GDigicamManager object.
 *
 * Returns: #True if success, #FALSE otherwise.
 **/
gboolean
g_digicam_manager_get_iso_sensitivity_mode (GDigicamManager            *manager,
                                            GDigicamIsosensitivitymode *iso_sensitivity_mode,
                                            guint                      *iso_level,
                                            GError                    **error)
{
    GDigicamManagerPrivate *priv = NULL;
    gboolean result = FALSE;
    gchar *error_msg = NULL;
    GDigicamError error_code = G_DIGICAM_ERROR_FAILED;

    g_return_val_if_fail (G_DIGICAM_IS_MANAGER (manager), FALSE);
    g_return_val_if_fail (NULL != iso_sensitivity_mode, FALSE);
    g_return_val_if_fail (NULL != iso_level, FALSE);


    priv = G_DIGICAM_MANAGER_GET_PRIVATE (manager);

    /* Check for GStreamer bin */
    if (NULL == priv->gst_bin) {
        error_code = G_DIGICAM_ERROR_GSTREAMER_BIN_NOT_SET;
        error_msg = g_strdup ("imposible to get the iso sensitivity "
                              "since there is no GStreamer bin "
                              "from which to get it");
        goto error;
    }

    /* FIXME: Not sure if we should check this */
/*     /\* Check still picture mode *\/ */
/*     if (priv->mode != G_DIGICAM_MODE_STILL) { */
/*         error_code = G_DIGICAM_ERROR_INVALID_MODE; */
/*         error_msg = g_strdup ("imposible to get the iso sensitivity " */
/*                               "since camera is not in still picture " */
/*                               "mode, required for this operation."); */
/*         goto error; */
/*     } */

    /* Check iso sensitivity capabilities */
    if (!(priv->descriptor->supported_features &
          (G_DIGICAM_CAPABILITIES_AUTOISOSENSITIVITY |
           G_DIGICAM_CAPABILITIES_MANUALISOSENSITIVITY))) {
        error_code = G_DIGICAM_ERROR_ISOSENSITIVITYMODE_NOT_SUPPORTED;
        error_msg = g_strdup ("imposible to get the iso sensitivity "
                              "since the GStreamer bin "
                              "has not this capability.");
        goto error;
    }

    /* Perform operation */
    *iso_sensitivity_mode = priv->iso_sensitivity_mode;
    if (*iso_sensitivity_mode ==
        G_DIGICAM_ISOSENSITIVITYMODE_MANUAL) {
        *iso_level = priv->iso_level;
    }

    result = TRUE;

error:
    if ((NULL != error) && (NULL == *error)) {
        if ((!result) && (NULL != error_msg)) {
            g_digicam_set_error (error, error_code, error_msg);
	}
    }

    /* Free */
    if (NULL != error_msg) {
        g_free (error_msg);
    }

    return result;
}


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


/**
 * g_digicam_manager_set_white_balance_mode:
 * @manager: A #GDigicamManager
 * @white_balance_mode: The white balance mode to set in the #GDigicamManager
 * @white_balance_level: used when #G_DIGICAM_WHITEBALANCEMODE_MANUAL is set.
 * @error: A #GError to store the result of the operation.
 * @user_data: Data to be used with the customized set function
 *  provided by the user in the #GDigicamDescriptor.
 *
 * Sets the white balance mode in the #GDigicamManager object.
 *
 * Returns: #True if success, #False otherwise.
 **/
gboolean
g_digicam_manager_set_white_balance_mode (GDigicamManager         *manager,
                                          GDigicamWhitebalancemode white_balance_mode,
                                          guint                    white_balance_level,
                                          GError                 **error,
                                          gpointer                 user_data)
{
    GDigicamManagerPrivate *priv = NULL;
    gboolean result = FALSE;
    gchar *error_msg = NULL;
    GDigicamError error_code = G_DIGICAM_ERROR_FAILED;

    g_return_val_if_fail (G_DIGICAM_IS_MANAGER (manager), FALSE);


    priv = G_DIGICAM_MANAGER_GET_PRIVATE (manager);

    /* Check GStreamer bin */
    if (NULL == priv->gst_bin) {
        error_code = G_DIGICAM_ERROR_GSTREAMER_BIN_NOT_SET;
        error_msg = g_strdup ("imposible to set the white balance mode "
                              "since there is no GStreamer bin "
                              "in which set it");
        goto error;
    }

    /* Check white balance capability */
    if (white_balance_mode == G_DIGICAM_WHITEBALANCEMODE_AUTO) {
        if (!(priv->descriptor->supported_features &
              G_DIGICAM_CAPABILITIES_AUTOWHITEBALANCE)) {
            error_code = G_DIGICAM_ERROR_WHITEBALANCEMODE_NOT_SUPPORTED;
            error_msg = g_strdup ("imposible to set the white balance mode "
                                  "since the GStreamer bin "
                                  "has not this capability");
            goto error;
        }
    } else {
        if (!(priv->descriptor->supported_features &
              G_DIGICAM_CAPABILITIES_MANUALWHITEBALANCE)) {
            error_code = G_DIGICAM_ERROR_WHITEBALANCEMODE_NOT_SUPPORTED;
            error_msg = g_strdup ("imposible to set the white balance mode "
                                  "since the GStreamer bin "
                                  "has not this capability");
            goto error;
        }
    }

    /* Check white balance mode capabilities */
    if (!(priv->descriptor->supported_white_balance_modes &
          white_balance_mode)) {
        error_code = G_DIGICAM_ERROR_WHITEBALANCEMODE_NOT_SUPPORTED;
        error_msg = g_strdup ("imposible to set the white balance mode "
                              "since the GStreamer bin "
                              "has not this capability");
        goto error;
    }

    /* Avoid to set the same value */
    if (white_balance_mode == priv->white_balance_mode) {
        result = TRUE;
        if (white_balance_mode !=
            G_DIGICAM_WHITEBALANCEMODE_MANUAL) {
            goto error;
        }
        if (white_balance_level == priv->white_balance_level) {
            goto error;
        }
    }

    /* Check function handler */
    if (NULL == priv->descriptor->set_white_balance_mode_func) {
        error_code = G_DIGICAM_ERROR_FAILED;
        error_msg = g_strdup ("error setting the white balance mode "
                              "in the GStreamer bin since "
                              "there is not handler function");
        goto error;
    }

    /* Perform operation */
    G_DIGICAM_DEBUG ("GDigicam: Set white balance mode started\n");
    result = priv->descriptor->set_white_balance_mode_func (manager, user_data);

    /* Check operation result */
    if (!result) {
        error_code = G_DIGICAM_ERROR_FAILED;
        error_msg = g_strdup_printf ("internal error setting the "
                                     "%d white balance mode and the "
                                     "%d white balance level"
                                     "in the GStreamer bin",
                                     white_balance_mode,
                                     white_balance_level);
        goto error;
    }

    /* Setting internal value */
    /* FIXME: Maybe all of this is unnecessary and we are just
     * notified from GStreamer*/
    priv->white_balance_mode = white_balance_mode;
    priv->white_balance_level = white_balance_level;

error:
    if ((NULL != error) && (NULL == *error)) {
        if ((!result) && (NULL != error_msg)) {
            g_digicam_set_error (error, error_code, error_msg);
        }
    }

    /* Free */
    if (NULL != error_msg) {
        g_free(error_msg);
    }

    return result;
}


/**
 * g_digicam_manager_get_white_balance_mode:
 * @manager: A #GDigicamManager
 * @white_balance_mode: A pointer in which to set the white balance mode from the
 * @white_balance_level: Manual value for white balance.
 * #GDigicamManager
 * @error: A #GError to store the result of the operation.
 *
 * Gets the white balance mode from the #GDigicamManager object.
 *
 * Returns: #True if success, #FALSE otherwise.
 **/
gboolean
g_digicam_manager_get_white_balance_mode (GDigicamManager          *manager,
                                          GDigicamWhitebalancemode *white_balance_mode,
                                          guint                    *white_balance_level,
                                          GError                  **error)
{
    GDigicamManagerPrivate *priv = NULL;
    gboolean result = FALSE;
    gchar *error_msg = NULL;
    GDigicamError error_code = G_DIGICAM_ERROR_FAILED;

    g_return_val_if_fail (G_DIGICAM_IS_MANAGER (manager), FALSE);
    g_return_val_if_fail (NULL != white_balance_mode, FALSE);
    g_return_val_if_fail (NULL != white_balance_level, FALSE);


    priv = G_DIGICAM_MANAGER_GET_PRIVATE (manager);

    /* Check GStreamer bin */
    if (NULL == priv->gst_bin) {
        error_code = G_DIGICAM_ERROR_GSTREAMER_BIN_NOT_SET;
        error_msg = g_strdup ("imposible to get the white balance mode "
                              "since there is no GStreamer bin "
                              "from which to get it");
        goto error;
    }

    /* Check white balance capabilities */
    if (!(priv->descriptor->supported_features &
          (G_DIGICAM_CAPABILITIES_AUTOWHITEBALANCE |
           G_DIGICAM_CAPABILITIES_MANUALWHITEBALANCE))) {
        error_code = G_DIGICAM_ERROR_WHITEBALANCEMODE_NOT_SUPPORTED;
        error_msg = g_strdup ("imposible to get the white balance mode "
                              "since the GStreamer bin "
                              "has not this capability");
        goto error;
    }

    /* Perform operation */
    *white_balance_mode = priv->white_balance_mode;
    if (*white_balance_mode ==
        G_DIGICAM_WHITEBALANCEMODE_MANUAL) {
        *white_balance_level = priv->white_balance_level;
    }

    result = TRUE;

error:
    if ((NULL != error) && (NULL == *error)) {
        if ((!result) && (NULL != error_msg)) {
            g_digicam_set_error (error, error_code, error_msg);
	}
    }

    /* Free */
    if (NULL != error_msg) {
        g_free (error_msg);
    }

    return result;
}


/**
 * g_digicam_manager_set_metering_mode:
 * @manager: A #GDigicamManager
 * @metering_mode: the #GDigicamMeteringmode value to set.
 * @error: A #GError to store the result of the operation.
 * @user_data: Data to be used with the customized set function
 * provided by the user in the #GDigicamDescriptor.
 *
 * Sets the current metering mode and autometering algorithm,
 * in case of automatic metering setting.
 *
 * Returns: #True if success, #False otherwise.
 **/
gboolean
g_digicam_manager_set_metering_mode (GDigicamManager     *manager,
                                     GDigicamMeteringmode metering_mode,
                                     GError             **error,
                                     gpointer             user_data)
{
    GDigicamManagerPrivate *priv = NULL;
    gboolean result = FALSE;
    gchar *error_msg = NULL;
    GDigicamError error_code = G_DIGICAM_ERROR_FAILED;

    g_return_val_if_fail (G_DIGICAM_IS_MANAGER (manager), FALSE);


    priv = G_DIGICAM_MANAGER_GET_PRIVATE (manager);

    /* Check GStreamer bin */
    if (NULL == priv->gst_bin) {
        error_code = G_DIGICAM_ERROR_GSTREAMER_BIN_NOT_SET;
        error_msg = g_strdup ("imposible to set metering mode"
                              "since there is no GStreamer bin "
                              "in which set it");
        goto error;
    }

    /* FIXME: Not sure if we should check this */
/*     /\* Check still picture mode *\/ */
/*     if (priv->mode != G_DIGICAM_MODE_STILL) { */
/*         error_code = G_DIGICAM_ERROR_INVALID_MODE; */
/*         error_msg = g_strdup ("imposible to set metering mode " */
/*                               "since camera is not in still picture " */
/*                               "mode, required for this operation."); */
/*         goto error; */
/*     } */

    /* Check metering mode capability */
    if (!(priv->descriptor->supported_features &
          G_DIGICAM_CAPABILITIES_METERING)) {
        error_code = G_DIGICAM_ERROR_METERINGMODE_NOT_SUPPORTED;
        error_msg = g_strdup ("imposible to set a metering mode "
                              "since the GStreamer bin "
                              "has not this capability.");
        goto error;
    }

    /* Check metering capabilities  */
    if (!(priv->descriptor->supported_metering_modes &
          metering_mode)) {
        error_code = G_DIGICAM_ERROR_METERINGMODE_NOT_SUPPORTED;
        error_msg = g_strdup_printf ("imposible to set metering mode %i "
                                     "since the GStreamer bin "
                                     "has not this capability.",
                                     metering_mode);
        goto error;
    }

    /* Avoid to set the same value */
    if (metering_mode == priv->metering_mode) {
        result = TRUE;
        goto error;
    }

    /* Check function handler */
    if (NULL == priv->descriptor->set_metering_mode_func) {
        error_code = G_DIGICAM_ERROR_FAILED;
        error_msg = g_strdup ("error setting the metering mode "
                              "in the GStreamer bin since "
                              "there is not handler function");
        goto error;
    }

    /* Perform operation */
    G_DIGICAM_DEBUG ("GDigicam: Set metering mode started\n");
    result = priv->descriptor->set_metering_mode_func (manager, user_data);

    /* Check operation result */
    if (!result) {
        error_code = G_DIGICAM_ERROR_FAILED;
        error_msg = g_strdup_printf ("internal error setting the "
                                     "%d metering mode"
                                     "in the GStreamer bin",
                                     metering_mode);
        goto error;
    }

    /* Setting internal value */
    /* FIXME: Maybe all of this is unnecessary and we are just
     * notified from GStreamer*/
    priv->metering_mode = metering_mode;

error:
    if ((NULL != error) && (NULL == *error)) {
        if ((!result) && (NULL != error_msg)) {
            g_digicam_set_error (error, error_code, error_msg);
        }
    }

    /* Free */
    if (NULL != error_msg) {
        g_free(error_msg);
    }

    return result;
}


/**
 * g_digicam_manager_get_metering_mode:
 * @manager: A #GDigicamManager
 * @metering_mode: A pointer in which to set the metering mode from
 * the #GDigicamManager
 * @error: A #GError to store the result of the operation.
 *
 * Gets the metering mode from the #GDigicamManager object.
 *
 * Returns: #True if success, #FALSE otherwise.
 **/
gboolean
g_digicam_manager_get_metering_mode (GDigicamManager      *manager,
                                     GDigicamMeteringmode *metering_mode,
                                     GError              **error)
{
    GDigicamManagerPrivate *priv = NULL;
    gboolean result = FALSE;
    gchar *error_msg = NULL;
    GDigicamError error_code = G_DIGICAM_ERROR_FAILED;

    g_return_val_if_fail (G_DIGICAM_IS_MANAGER (manager), FALSE);
    g_return_val_if_fail (NULL != metering_mode, FALSE);


    priv = G_DIGICAM_MANAGER_GET_PRIVATE (manager);

    /* Check GStreamer bin */
    if (NULL == priv->gst_bin) {
        error_code = G_DIGICAM_ERROR_GSTREAMER_BIN_NOT_SET;
        error_msg = g_strdup ("imposible to get the metering mode"
                              "since there is no GStreamer bin "
                              "in which set it");
        goto error;
    }

    /* FIXME: Not sure if we should check this */
/*     /\* Check still picture mode *\/ */
/*     if (priv->mode != G_DIGICAM_MODE_STILL) { */
/*         error_code = G_DIGICAM_ERROR_INVALID_MODE; */
/*         error_msg = g_strdup ("imposible to get the metering mode " */
/*                               "since camera is not in still picture " */
/*                               "mode, required for this operation."); */
/*         goto error; */
/*     } */

    /* Check metering mode capability */
    if (!(priv->descriptor->supported_features &
          G_DIGICAM_CAPABILITIES_METERING)) {
        error_code = G_DIGICAM_ERROR_METERINGMODE_NOT_SUPPORTED;
        error_msg = g_strdup ("imposible to get the metering mode "
                              "since the GStreamer bin "
                              "has not this capability.");
        goto error;
    }

    /* Perform operation */
    *metering_mode = priv->metering_mode;

    result = TRUE;

error:
    if ((NULL != error) && (NULL == *error)) {
        if ((!result) && (NULL != error_msg)) {
            g_digicam_set_error (error, error_code, error_msg);
	}
    }

    /* Free */
    if (NULL != error_msg) {
        g_free (error_msg);
    }

    return result;
}

/**
 * g_digicam_manager_set_aspect_ratio_resolution:
 * @manager: A #GDigicamManager
 * @aspect_ratio: The aspect ratio to set in the #GDigicamManager
 * @resolution: The resolution ratio to set in the #GDigicamManager
 * @error: A #GError to store the result of the operation.
 * @user_data: Data to be used with the customized set function
 *  provided by the user in the #GDigicamDescriptor.
 *
 * Sets the aspect ratio and resolution in the #GDigicamManager object.
 *
 * Returns: #True if success, #False otherwise.
 **/
gboolean
g_digicam_manager_set_aspect_ratio_resolution (GDigicamManager    *manager,
					       GDigicamAspectratio aspect_ratio,
					       GDigicamResolution  resolution,
					       GError            **error,
					       gpointer            user_data)
{
    GDigicamManagerPrivate *priv = NULL;
    gboolean result = FALSE;
    gchar *error_msg = NULL;
    GDigicamError error_code = G_DIGICAM_ERROR_FAILED;

    g_return_val_if_fail (G_DIGICAM_IS_MANAGER (manager), FALSE);


    priv = G_DIGICAM_MANAGER_GET_PRIVATE (manager);

    /* Check GStreamer bin */
    if (NULL == priv->gst_bin) {
        error_code = G_DIGICAM_ERROR_GSTREAMER_BIN_NOT_SET;
        error_msg = g_strdup ("imposible to set aspect ratio "
                              "and resolution "
                              "since there is no GStreamer bin "
                              "in which set it");
        goto error;
    }

    /* Check aspect ratio capability */
    if (!(priv->descriptor->supported_features &
          G_DIGICAM_CAPABILITIES_ASPECTRATIO)) {
        error_code = G_DIGICAM_ERROR_ASPECTRATIO_NOT_SUPPORTED;
        error_msg = g_strdup ("imposible to set aspect ratio "
                              "since the GStreamer bin "
                              "has not this capability");
        goto error;
    }

    /* Check resolution capability */
    if (!(priv->descriptor->supported_features &
          G_DIGICAM_CAPABILITIES_RESOLUTION)) {
        error_code = G_DIGICAM_ERROR_RESOLUTION_NOT_SUPPORTED;
        error_msg = g_strdup ("imposible to set the resolution "
                              "since the GStreamer bin "
                              "has not this capability");
        goto error;
    }

    /* Check aspect ratio capabilities */
    if (!(priv->descriptor->supported_aspect_ratios &
          aspect_ratio)) {
        error_code = G_DIGICAM_ERROR_ASPECTRATIO_NOT_SUPPORTED;
        error_msg = g_strdup_printf ("imposible to set aspect ratio %d "
                                     "since the GStreamer bin "
                                     "has not this capability.",
                                     aspect_ratio);
        goto error;
    }

    /* Avoid to set the same value */
    if (aspect_ratio == priv->aspect_ratio &&
	resolution == priv->resolution ) {
        result = TRUE;
        goto error;
    }

    /* Check function handler */
    if (NULL == priv->descriptor->set_aspect_ratio_resolution_func) {
        error_code = G_DIGICAM_ERROR_FAILED;
        error_msg = g_strdup ("error setting the aspect ratio and resolution "
                              "in the GStreamer bin since "
                              "there is not handler function");
        goto error;
    }

    /* Perform operation */
    G_DIGICAM_DEBUG ("GDigicam: Set aspect ratio and resolution started\n");
    result = priv->descriptor->set_aspect_ratio_resolution_func (manager, user_data);

    /* Check operation result */
    if (!result) {
        error_code = G_DIGICAM_ERROR_FAILED;
        error_msg = g_strdup_printf ("internal error setting the "
                                     "%d aspect ratio "
				     "and %d resolution "
                                     "in the GStreamer bin",
                                     aspect_ratio, resolution);
        goto error;
    }

    /* Setting internal value */
    /* FIXME: Maybe all of this is unnecessary and we are just
     * notified from GStreamer*/
    priv->aspect_ratio = aspect_ratio;
    priv->resolution = resolution;

error:
    if ((NULL != error) && (NULL == *error)) {
        if ((!result) && (NULL != error_msg)) {
            g_digicam_set_error (error, error_code, error_msg);
        }
    }

    /* Free */
    if (NULL != error_msg) {
        g_free(error_msg);
    }

    return result;
}

/**
 * g_digicam_manager_set_aspect_ratio:
 * @manager: A #GDigicamManager
 * @aspect_ratio: The aspect ratio to set in the #GDigicamManager
 * @error: A #GError to store the result of the operation.
 * @user_data: Data to be used with the customized set function
 *  provided by the user in the #GDigicamDescriptor.
 *
 * Sets the aspect ratio in the #GDigicamManager object.
 *
 * Returns: #True if success, #False otherwise.
 **/
gboolean
g_digicam_manager_set_aspect_ratio (GDigicamManager    *manager,
                                    GDigicamAspectratio aspect_ratio,
                                    GError            **error,
                                    gpointer            user_data)
{
    GDigicamManagerPrivate *priv = NULL;
    gboolean result = FALSE;
    gchar *error_msg = NULL;
    GDigicamError error_code = G_DIGICAM_ERROR_FAILED;

    g_return_val_if_fail (G_DIGICAM_IS_MANAGER (manager), FALSE);


    priv = G_DIGICAM_MANAGER_GET_PRIVATE (manager);

    /* Check GStreamer bin */
    if (NULL == priv->gst_bin) {
        error_code = G_DIGICAM_ERROR_GSTREAMER_BIN_NOT_SET;
        error_msg = g_strdup ("imposible to set aspect ratio "
                              "since there is no GStreamer bin "
                              "in which set it");
        goto error;
    }

    /* Check aspect ratio capability */
    if (!(priv->descriptor->supported_features &
          G_DIGICAM_CAPABILITIES_ASPECTRATIO)) {
        error_code = G_DIGICAM_ERROR_ASPECTRATIO_NOT_SUPPORTED;
        error_msg = g_strdup ("imposible to set aspect ratio "
                              "since the GStreamer bin "
                              "has not this capability");
        goto error;
    }

    /* Check aspect ratio capabilities */
    if (!(priv->descriptor->supported_aspect_ratios &
          aspect_ratio)) {
        error_code = G_DIGICAM_ERROR_ASPECTRATIO_NOT_SUPPORTED;
        error_msg = g_strdup_printf ("imposible to set aspect ratio %d "
                                     "since the GStreamer bin "
                                     "has not this capability.",
                                     aspect_ratio);
        goto error;
    }

    /* Avoid to set the same value */
    if (aspect_ratio == priv->aspect_ratio) {
        result = TRUE;
        goto error;
    }

    /* Check function handler */
    if (NULL == priv->descriptor->set_aspect_ratio_func) {
        error_code = G_DIGICAM_ERROR_FAILED;
        error_msg = g_strdup ("error setting the aspect ratio "
                              "in the GStreamer bin since "
                              "there is not handler function");
        goto error;
    }

    /* Perform operation */
    G_DIGICAM_DEBUG ("GDigicam: Set aspect ratio started\n");
    result = priv->descriptor->set_aspect_ratio_func (manager, user_data);

    /* Check operation result */
    if (!result) {
        error_code = G_DIGICAM_ERROR_FAILED;
        error_msg = g_strdup_printf ("internal error setting the "
                                     "%d aspect ratio"
                                     "in the GStreamer bin",
                                     aspect_ratio);
        goto error;
    }

    /* Setting internal value */
    /* FIXME: Maybe all of this is unnecessary and we are just
     * notified from GStreamer*/
    priv->aspect_ratio = aspect_ratio;

error:
    if ((NULL != error) && (NULL == *error)) {
        if ((!result) && (NULL != error_msg)) {
            g_digicam_set_error (error, error_code, error_msg);
        }
    }

    /* Free */
    if (NULL != error_msg) {
        g_free(error_msg);
    }

    return result;
}


/**
 * g_digicam_manager_get_aspect_ratio:
 * @manager: A #GDigicamManager
 * @aspect_ratio: A pointer in which to set the aspect ratio from the
 *  #GDigicamManager
 * @error: A #GError to store the result of the operation.
 *
 * Gets the aspect_ratio mode from the #GDigicamManager object.
 *
 * Returns: #True if success, #FALSE otherwise.
 **/
gboolean
g_digicam_manager_get_aspect_ratio (GDigicamManager     *manager,
                                    GDigicamAspectratio *aspect_ratio,
                                    GError             **error)
{
    GDigicamManagerPrivate *priv = NULL;
    gboolean result = FALSE;
    gchar *error_msg = NULL;
    GDigicamError error_code = G_DIGICAM_ERROR_FAILED;

    g_return_val_if_fail (G_DIGICAM_IS_MANAGER (manager), FALSE);
    g_return_val_if_fail (NULL != aspect_ratio, FALSE);


    priv = G_DIGICAM_MANAGER_GET_PRIVATE (manager);

    /* Check GStreamer bin */
    if (NULL == priv->gst_bin) {
        error_code = G_DIGICAM_ERROR_GSTREAMER_BIN_NOT_SET;
        error_msg = g_strdup ("imposible to get the aspect ratio "
                              "since there is no GStreamer bin "
                              "in which set it");
        goto error;
    }

    /* Check aspect ratio capability */
    if (!(priv->descriptor->supported_features &
          G_DIGICAM_CAPABILITIES_ASPECTRATIO)) {
        error_code = G_DIGICAM_ERROR_ASPECTRATIO_NOT_SUPPORTED;
        error_msg = g_strdup ("imposible to get the aspect ratio "
                              "since the GStreamer bin "
                              "has not this capability");
        goto error;
    }

    /* Performs operation */
    *aspect_ratio = priv->aspect_ratio;

    result = TRUE;

error:
    if ((NULL != error) && (NULL == *error)) {
        if ((!result) && (NULL != error_msg)) {
            g_digicam_set_error (error, error_code, error_msg);
	}
    }

    /* Free */
    if (NULL != error_msg) {
        g_free (error_msg);
    }

    return result;
}


/*     gboolean g_digicam_manager_set_color_filter (GDigicamManager   *manager, */
/*                                                  GDigicamColorfilter  color_filter, */
/*                                                  GError           **error); */
/*     gboolean g_digicam_manager_get_color_filter (GDigicamManager   *manager, */
/*                                                  GDigicamColorfilter *color_filter, */
/*                                                  GError           **error); */


/**
 * g_digicam_manager_set_quality:
 * @manager: A #GDigicamManager
 * @quality: The quality to set in the #GDigicamManager
 * @error: A #GError to store the result of the operation.
 * @user_data: Data to be used with the customized set function
 *  provided by the user in the #GDigicamDescriptor.
 *
 * Sets the quality in the #GDigicamManager object.
 *
 * Returns: #True if success, #False otherwise.
 **/
gboolean
g_digicam_manager_set_quality (GDigicamManager *manager,
                               GDigicamQuality  quality,
                               GError         **error,
                               gpointer         user_data)
{
    GDigicamManagerPrivate *priv = NULL;
    gboolean result = FALSE;
    gchar *error_msg = NULL;
    GDigicamError error_code = G_DIGICAM_ERROR_FAILED;

    g_return_val_if_fail (G_DIGICAM_IS_MANAGER (manager), FALSE);


    priv = G_DIGICAM_MANAGER_GET_PRIVATE (manager);

    /* Check GStreamer bin */
    if (NULL == priv->gst_bin) {
        error_code = G_DIGICAM_ERROR_GSTREAMER_BIN_NOT_SET;
        error_msg = g_strdup ("imposible to set the quality "
                              "since there is no GStreamer bin "
                              "in which set it");
        goto error;
    }

    /* Check quality capability */
    if (!(priv->descriptor->supported_features &
          G_DIGICAM_CAPABILITIES_QUALITY)) {
        error_code = G_DIGICAM_ERROR_QUALITY_NOT_SUPPORTED;
        error_msg = g_strdup ("imposible to set the quality "
                              "since the GStreamer bin "
                              "has not this capability");
        goto error;
    }

    /* Check quality capabilities */
    if (!(priv->descriptor->supported_qualities &
          quality)) {
        error_code = G_DIGICAM_ERROR_QUALITY_NOT_SUPPORTED;
        error_msg = g_strdup_printf ("imposible to set quality %d "
                                     "since the GStreamer bin "
                                     "has not this capability.",
                                     quality);
        goto error;
    }

    /* Avoid to set the same value */
    if (quality == priv->quality) {
        result = TRUE;
        goto error;
    }

    /* Check function handler */
    if (NULL == priv->descriptor->set_quality_func) {
        error_code = G_DIGICAM_ERROR_FAILED;
        error_msg = g_strdup ("error setting the quality "
                              "in the GStreamer bin since "
                              "there is not handler function");
        goto error;
    }

    /* Perform operation */
    G_DIGICAM_DEBUG ("GDigicam: Set quality started\n");
    result = priv->descriptor->set_quality_func (manager, user_data);

    /* Check operation result */
    if (!result) {
        error_code = G_DIGICAM_ERROR_FAILED;
        error_msg = g_strdup_printf ("internal error setting the "
                                     "%d quality"
                                     "in the GStreamer bin",
                                     quality);
        goto error;
    }

    /* Setting internal value */
    /* FIXME: Maybe all of this is unnecessary and we are just
     * notified from GStreamer*/
    priv->quality = quality;

error:
    if ((NULL != error) && (NULL == *error)) {
        if ((!result) && (NULL != error_msg)) {
            g_digicam_set_error (error, error_code, error_msg);
        }
    }

    /* Free */
    if (NULL != error_msg) {
        g_free(error_msg);
    }

    return result;
}

/**
 * g_digicam_manager_get_quality:
 * @manager: A #GDigicamManager
 * @quality: A pointer in which to get the quality from the
 * #GDigicamManager
 * @error: A #GError to store the result of the operation.
 *
 * Gets the quality from the #GDigicamManager object.
 *
 * Returns: #True if success, #FALSE otherwise.
 **/
gboolean
g_digicam_manager_get_quality (GDigicamManager *manager,
                               GDigicamQuality *quality,
                               GError         **error)
{
    GDigicamManagerPrivate *priv = NULL;
    gboolean result = FALSE;
    gchar *error_msg = NULL;
    GDigicamError error_code = G_DIGICAM_ERROR_FAILED;

    g_return_val_if_fail (G_DIGICAM_IS_MANAGER (manager), FALSE);
    g_return_val_if_fail (NULL != quality, FALSE);


    priv = G_DIGICAM_MANAGER_GET_PRIVATE (manager);

    /* Check GStreamer bin */
    if (NULL == priv->gst_bin) {
        error_code = G_DIGICAM_ERROR_GSTREAMER_BIN_NOT_SET;
        error_msg = g_strdup ("imposible to get the quality "
                              "since there is no GStreamer bin "
                              "from which to get it");
        goto error;
    }

    /* Check quality capability */
    if (!(priv->descriptor->supported_features &
          G_DIGICAM_CAPABILITIES_QUALITY)) {
        error_code = G_DIGICAM_ERROR_QUALITY_NOT_SUPPORTED;
        error_msg = g_strdup ("imposible to get the quality "
                              "since the GStreamer bin "
                              "has not this capability");
        goto error;
    }

    /* Performs operation */
    *quality = priv->quality;

    result = TRUE;

error:
    if ((NULL != error) && (NULL == *error)) {
        if ((!result) && (NULL != error_msg)) {
            g_digicam_set_error (error, error_code, error_msg);
	}
    }

    /* Free */
    if (NULL != error_msg) {
        g_free (error_msg);
    }

    return result;
}


/**
 * g_digicam_manager_set_resolution:
 * @manager: A #GDigicamManager
 * @resolution: The resolution to set in the #GDigicamManager
 * @error: A #GError to store the result of the operation.
 * @user_data: Data to be used with the customized set function
 *  provided by the user in the #GDigicamDescriptor.
 *
 * Sets the resolution in the #GDigicamManager object.
 *
 * Returns: #True if success, #False otherwise.
 **/
gboolean
g_digicam_manager_set_resolution (GDigicamManager   *manager,
                                  GDigicamResolution resolution,
                                  GError           **error,
                                  gpointer           user_data)
{
    GDigicamManagerPrivate *priv = NULL;
    gboolean result = FALSE;
    gchar *error_msg = NULL;
    GDigicamError error_code = G_DIGICAM_ERROR_FAILED;

    g_return_val_if_fail (G_DIGICAM_IS_MANAGER (manager), FALSE);


    priv = G_DIGICAM_MANAGER_GET_PRIVATE (manager);

    /* Check GStreamer bin */
    if (NULL == priv->gst_bin) {
        error_code = G_DIGICAM_ERROR_GSTREAMER_BIN_NOT_SET;
        error_msg = g_strdup ("imposible to set resolution parameters "
                              "since there is no GStreamer bin "
                              "in which set them");
        goto error;
    }

    /* Check resolution capability */
    if (!(priv->descriptor->supported_features &
          G_DIGICAM_CAPABILITIES_RESOLUTION)) {
        error_code = G_DIGICAM_ERROR_RESOLUTION_NOT_SUPPORTED;
        error_msg = g_strdup ("imposible to set the resolution "
                              "since the GStreamer bin "
                              "has not this capability");
        goto error;
    }

    /* Check resolution capabilities */
    if (!(priv->descriptor->supported_resolutions &
          resolution)) {
        error_code = G_DIGICAM_ERROR_RESOLUTION_NOT_SUPPORTED;
        error_msg = g_strdup_printf ("imposible to set resolution %d "
                                     "since the GStreamer bin "
                                     "has not this capability.",
                                     resolution);
        goto error;
    }

    /* Avoid to set the same value */
    if (resolution == priv->resolution) {
        result = TRUE;
        goto error;
    }

    /* Check function handler */
    if (NULL == priv->descriptor->set_resolution_func) {
        error_code = G_DIGICAM_ERROR_FAILED;
        error_msg = g_strdup ("error setting the resolution "
                              "in the GStreamer bin since "
                              "there is not handler function");
        goto error;
    }

    /* Perform operation */
    G_DIGICAM_DEBUG ("GDigicam: Set quality started\n");
    result = priv->descriptor->set_resolution_func (manager, user_data);

    /* Check operation result */
    if (!result) {
        error_code = G_DIGICAM_ERROR_FAILED;
        error_msg = g_strdup_printf ("internal error setting the "
                                     "%d resolution"
                                     "in the GStreamer bin",
                                     resolution);
        goto error;
    }

    /* Setting internal value */
    /* FIXME: Maybe all of this is unnecessary and we are just
     * notified from GStreamer*/
    priv->resolution = resolution;

error:
    if ((NULL != error) && (NULL == *error)) {
        if ((!result) && (NULL != error_msg)) {
            g_digicam_set_error (error, error_code, error_msg);
        }
    }

    /* Free */
    if (NULL != error_msg) {
        g_free(error_msg);
    }

    return result;
}


/**
 * g_digicam_manager_get_resolution:
 * @manager: A #GDigicamManager
 * @resolution: A pointer in which to set the resolution from the
 * #GDigicamManager
 * @error: A #GError to store the result of the operation.
 *
 * Gets the resolution from the #GDigicamManager object.
 *
 * Returns: #True if success, #False otherwise.
 **/
gboolean
g_digicam_manager_get_resolution (GDigicamManager   *manager,
                                  GDigicamResolution *resolution,
                                  GError           **error)
{
    GDigicamManagerPrivate *priv = NULL;
    gboolean result = FALSE;
    gchar *error_msg = NULL;
    GDigicamError error_code = G_DIGICAM_ERROR_FAILED;

    g_return_val_if_fail (G_DIGICAM_IS_MANAGER (manager), FALSE);
    g_return_val_if_fail (NULL != resolution, FALSE);


    priv = G_DIGICAM_MANAGER_GET_PRIVATE (manager);

    /* Check GStreamer bin */
    if (NULL == priv->gst_bin) {
        error_code = G_DIGICAM_ERROR_GSTREAMER_BIN_NOT_SET;
        error_msg = g_strdup ("imposible to get the resolution "
                              "since there is no GStreamer bin "
                              "from which to get it");
        goto error;
    }

    /* Check resolution capability */
    if (!(priv->descriptor->supported_features &
          G_DIGICAM_CAPABILITIES_RESOLUTION)) {
        error_code = G_DIGICAM_ERROR_RESOLUTION_NOT_SUPPORTED;
        error_msg = g_strdup ("imposible to get the resolution "
                              "since the GStreamer bin "
                              "has not this capability");
        goto error;
    }

    /* Performs operation */
    *resolution = priv->resolution;

    result = TRUE;

error:
    if ((NULL != error) && (NULL == *error)) {
        if ((!result) && (NULL != error_msg)) {
            g_digicam_set_error (error, error_code, error_msg);
	}
    }

    /* Free */
    if (NULL != error_msg) {
        g_free (error_msg);
    }

    return result;
}


/*     gboolean g_digicam_manager_set_video_stabilization (GDigicamManager   *manager, */
/*                                                         GDigicamVideostabilization  video_stabilization, */
/*                                                         GError           **error); */
/*     gboolean g_digicam_manager_get_video_stabilization (GDigicamManager   *manager, */
/*                                                         GDigicamVideostabilization *video_stabilization, */
/*                                                         GError           **error); */


/**
 * g_digicam_manager_set_locks:
 * @manager:  A #GDigicamManager
 * @locks: A #GDigicamLock flag specifying which locks to set.
 * @error: A #GError to store the error condition.
 * @user_data: Data to be used with the customized set function
 *  provided by the user in the #GDigicamDescriptor.
 *
 * Sets the locks in the #GDigicamManager object.
 *
 * Returns: #True if success, #False otherwise.
 **/
gboolean
g_digicam_manager_set_locks (GDigicamManager *manager,
                             GDigicamLock     locks,
                             GError         **error,
                             gpointer         user_data)
{
    GDigicamManagerPrivate *priv = NULL;
    gboolean result = FALSE;
    gchar *error_msg = NULL;
    GDigicamError error_code = G_DIGICAM_ERROR_FAILED;

    g_return_val_if_fail (G_DIGICAM_IS_MANAGER (manager), FALSE);


    priv = G_DIGICAM_MANAGER_GET_PRIVATE (manager);

    /* Check GStreamer bin */
    if (NULL == priv->gst_bin) {
        error_code = G_DIGICAM_ERROR_GSTREAMER_BIN_NOT_SET;
        error_msg = g_strdup ("imposible to set auto locks "
                              "since there is no GStreamer bin "
                              "in which set them");
        goto error;
    }

    /* Check current capabilities and modes */
    if (locks & G_DIGICAM_LOCK_AUTOFOCUS) {
        switch (priv->focus_mode) {
        case G_DIGICAM_FOCUSMODE_MANUAL:
            error_code = G_DIGICAM_ERROR_LOCK_NOT_POSSIBLE;
            error_msg = g_strdup ("imposible to set the autofocus lock "
                                  "since the GStreamer bin "
                                  "is in manual focus mode.");
            goto error;
        case G_DIGICAM_FOCUSMODE_AUTO:
        case G_DIGICAM_FOCUSMODE_FACE:
        case G_DIGICAM_FOCUSMODE_SMILE:
        case G_DIGICAM_FOCUSMODE_CENTROID:
        case G_DIGICAM_FOCUSMODE_CONTINUOUSAUTO:
        case G_DIGICAM_FOCUSMODE_CONTINUOUSCENTROID:
            break;
        default:
            error_code = G_DIGICAM_ERROR_LOCK_NOT_POSSIBLE;
            error_msg = g_strdup ("imposible to set the autofocus lock "
                                  "since the GStreamer bin "
                                  "is in an invalid focus mode.");
            goto error;
        }
    }

    if (locks & G_DIGICAM_LOCK_AUTOEXPOSURE) {
        switch (priv->exposure_mode) {
        case G_DIGICAM_EXPOSUREMODE_AUTO:
            break;
        case G_DIGICAM_EXPOSUREMODE_MANUAL:
        case G_DIGICAM_EXPOSUREMODE_NIGHT:
        case G_DIGICAM_EXPOSUREMODE_BACKLIGHT:
        case G_DIGICAM_EXPOSUREMODE_SPOTLIGHT:
        case G_DIGICAM_EXPOSUREMODE_SPORTS:
        case G_DIGICAM_EXPOSUREMODE_SNOW:
        case G_DIGICAM_EXPOSUREMODE_BEACH:
        case G_DIGICAM_EXPOSUREMODE_LARGEAPERTURE:
        case G_DIGICAM_EXPOSUREMODE_SMALLAPERTURE:
        case G_DIGICAM_EXPOSUREMODE_PORTRAIT:
        case G_DIGICAM_EXPOSUREMODE_NIGHTPORTRAIT:
        case G_DIGICAM_EXPOSUREMODE_LANDSCAPE:
            error_code = G_DIGICAM_ERROR_LOCK_NOT_POSSIBLE;
            error_msg = g_strdup ("imposible to set the autoexposure lock "
                                  "since the GStreamer bin "
                                  "is not in auto exposure mode.");
            goto error;
        default:
            error_code = G_DIGICAM_ERROR_LOCK_NOT_POSSIBLE;
            error_msg = g_strdup ("imposible to set the autoexposure lock "
                                  "since the GStreamer bin "
                                  "is in an invalid exposure mode.");
            goto error;
        }
    }

    if (locks & G_DIGICAM_LOCK_AUTOWHITEBALANCE) {
        switch (priv->white_balance_mode) {
        case G_DIGICAM_WHITEBALANCEMODE_AUTO:
            break;
        case G_DIGICAM_WHITEBALANCEMODE_MANUAL:
        case G_DIGICAM_WHITEBALANCEMODE_SUNLIGHT:
        case G_DIGICAM_WHITEBALANCEMODE_CLOUDY:
        case G_DIGICAM_WHITEBALANCEMODE_SHADE:
        case G_DIGICAM_WHITEBALANCEMODE_TUNGSTEN:
        case G_DIGICAM_WHITEBALANCEMODE_FLUORESCENT:
        case G_DIGICAM_WHITEBALANCEMODE_INCANDESCENT:
        case G_DIGICAM_WHITEBALANCEMODE_FLASH:
        case G_DIGICAM_WHITEBALANCEMODE_SUNSET:
            error_code = G_DIGICAM_ERROR_LOCK_NOT_POSSIBLE;
            error_msg = g_strdup ("imposible to set the autowhitebalance lock "
                                  "since the GStreamer bin "
                                  "is not in auto white balance mode.");
            goto error;
        default:
            error_code = G_DIGICAM_ERROR_LOCK_NOT_POSSIBLE;
            error_msg = g_strdup ("imposible to set the autowhitebalance lock "
                                  "since the GStreamer bin "
                                  "is in an invalid white balance mode.");
            goto error;
        }
    }

    /* Avoid to set the same value */
    if (priv->locks == locks) {
        result = TRUE;
        goto error;
    }

    /* Check function handler */
    if (NULL == priv->descriptor->set_locks_func) {
        error_code = G_DIGICAM_ERROR_FAILED;
        error_msg = g_strdup ("error setting the resolution "
                              "in the GStreamer bin since "
                              "there is not handler function");
        goto error;
    }

    /* Perform operation */
    G_DIGICAM_DEBUG ("GDigicam: Set locks operation started\n");
    result = priv->descriptor->set_locks_func (manager, user_data);

    /* Check operation result */
    if (!result) {
        error_code = G_DIGICAM_ERROR_FAILED;
        error_msg = g_strdup ("internal error setting the locks "
                              "in the GStreamer bin");
        goto error;
    }

    /* Setting internal value */
    /* FIXME: Maybe all of this is unnecessary and we are just
     * notified from GStreamer*/
    priv->locks = locks;

error:
    if ((NULL != error) && (NULL == *error)) {
        if ((!result) && (NULL != error_msg)) {
            g_digicam_set_error (error, error_code, error_msg);
        }
    }

    /* Free */
    if (NULL != error_msg) {
        g_free(error_msg);
    }

    return result;
}


/**
 * g_digicam_manager_get_locks:
 * @manager: A #GDigicamManager
 * @locks: A pointer in which to set the locks from the
 *  #GDigicamManager.
 * @error: A #GError to store the error condition.
 *
 * Sets the locks in the #GDigicamManager object.
 *
 * Returns: #True if success, #False otherwise.
 **/
gboolean
g_digicam_manager_get_locks (GDigicamManager *manager,
                             GDigicamLock    *locks,
                             GError         **error)
{
    GDigicamManagerPrivate *priv = NULL;
    gboolean result = FALSE;
    gchar *error_msg = NULL;
    GDigicamError error_code = G_DIGICAM_ERROR_FAILED;

    g_return_val_if_fail (G_DIGICAM_IS_MANAGER (manager), FALSE);
    g_return_val_if_fail (NULL != locks, FALSE);


    priv = G_DIGICAM_MANAGER_GET_PRIVATE (manager);

    /* Check GStreamer bin */
    if (NULL == priv->gst_bin) {
        error_code = G_DIGICAM_ERROR_GSTREAMER_BIN_NOT_SET;
        error_msg = g_strdup ("imposible to get auto locks "
                              "since there is no GStreamer bin "
                              "from which to get them");
        goto error;
    }

    /* Check still picture mode */
    if (priv->mode != G_DIGICAM_MODE_STILL) {
        error_code = G_DIGICAM_ERROR_INVALID_MODE;
        error_msg = g_strdup ("imposible to get auto locks "
                              "since camera is not in still picture "
                              "mode, required for this operation.");
        goto error;
    }

    /* Performs operation */
    *locks = priv->locks;

    result = TRUE;

error:
    if ((NULL != error) && (NULL == *error)) {
        if ((!result) && (NULL != error_msg)) {
            g_digicam_set_error (error, error_code, error_msg);
	}
    }

    /* Free */
    if (NULL != error_msg) {
        g_free (error_msg);
    }

    return result;
}


/**
 * g_digicam_manager_set_zoom:
 * @manager: A #GDigicamManager
 * @zoom: The zoom to set in the #GDigicamManager, from 1 to the
 *  maximum available.
 * @digital: A pointer in which to set if we are using digital zoom or
 *  not.
 * @error: A #GError to store the error condition.
 * @user_data: Data to be used with the customized set function
 *  provided by the user in the #GDigicamDescriptor.
 *
 * Sets the zoom in the #GDigicamManager object.
 *
 * Returns: #True if success, #False otherwise.
 **/
gboolean
g_digicam_manager_set_zoom (GDigicamManager *manager,
                            gdouble          zoom,
                            gboolean        *digital,
                            GError         **error,
                            gpointer         user_data)
{
    GDigicamManagerPrivate *priv = NULL;
    gdouble max_value;
    gdouble max_optical_value;
    gboolean result = FALSE;
    gchar *error_msg = NULL;
    GDigicamError error_code = G_DIGICAM_ERROR_FAILED;

    g_return_val_if_fail (G_DIGICAM_IS_MANAGER (manager), FALSE);
    g_return_val_if_fail (NULL != digital, FALSE);


    priv = G_DIGICAM_MANAGER_GET_PRIVATE (manager);

    /* Check GStreamer bin */
    if (NULL == priv->gst_bin) {
        error_code = G_DIGICAM_ERROR_GSTREAMER_BIN_NOT_SET;
        error_msg = g_strdup ("imposible to set the zoom "
                              "since there is no GStreamer bin "
                              "in which set it");
        goto error;
    }

    /* Check zoom capabilities */
    if (!(priv->descriptor->supported_features &
	  (G_DIGICAM_CAPABILITIES_OPTICALZOOM |
           G_DIGICAM_CAPABILITIES_DIGITALZOOM))) {
        error_code = G_DIGICAM_ERROR_ZOOM_NOT_SUPPORTED;
        error_msg = g_strdup ("imposible to set the zoom "
                              "since the GStreamer bin "
                              "has not this capability.");
        goto error;
    }

    /* Check optical/digical zoom values */
    if (priv->is_macro_enabled) {
        max_optical_value = priv->descriptor->max_optical_zoom_macro_enabled;
        if (priv->descriptor->supported_features &
            G_DIGICAM_CAPABILITIES_DIGITALZOOM) {
            max_value = priv->descriptor->max_zoom_macro_enabled;
        } else {
            max_value = priv->descriptor->max_optical_zoom_macro_enabled;
        }
    } else {
        max_optical_value = priv->descriptor->max_optical_zoom_macro_disabled;
        if (priv->descriptor->supported_features &
            G_DIGICAM_CAPABILITIES_DIGITALZOOM) {
            max_value = priv->descriptor->max_zoom_macro_disabled;
        } else {
            max_value = priv->descriptor->max_optical_zoom_macro_disabled;
        }
    }

    /* Check range values */
    if ((zoom < 1) || (zoom > max_value)) {
        error_code = G_DIGICAM_ERROR_ZOOM_OUT_OF_RANGE;
        error_msg = g_strdup ("imposible to set this zoom "
                              "value since it is out of range.");
        goto error;
    }

    /* Avoid to set the same value */
    if (priv->zoom == zoom) {
        result = TRUE;
        goto error;
    }

    /* Check function handler */
    if (NULL == priv->descriptor->set_zoom_func) {
        error_code = G_DIGICAM_ERROR_FAILED;
        error_msg = g_strdup_printf ("error setting zoom %f.2 "
                                     "in the GStreamer bin, "
                                     "there is not function handler",
                                     zoom);
        goto error;
    }

    /* Perform operation */
    G_DIGICAM_DEBUG ("GDigicam: Set zoom operation started\n");
    result = priv->descriptor->set_zoom_func (manager,
                                              user_data);

    /* Check operation result */
    if (!result) {
        error_code = G_DIGICAM_ERROR_FAILED;
        error_msg = g_strdup_printf ("internal error setting zoom %f.2 "
                                     "in the GStreamer bin",
                                     zoom);
        goto error;
    }

    /* Setting internal values */
    /* FIXME: Maybe all of this is unnecessary and we are just
     * notified from GStreamer*/
    if (zoom > max_optical_value) {
        *digital = TRUE;
    }

    priv->zoom = zoom;
    priv->digital_zoom = *digital;

error:
    if ((NULL != error) && (NULL == *error)) {
        if ((!result) && (NULL != error_msg)) {
            g_digicam_set_error (error, error_code, error_msg);
	}
    }

    /* Free */
    if (NULL != error_msg) {
        g_free (error_msg);
    }

    return result;
}


/**
 * g_digicam_manager_get_zoom:
 * @manager: A #GDigicamManager
 * @zoom: A pointer in which to set the zoom from the
 *  #GDigicamManager.
 * @digital: A pointer in which to set if we are using digital zoom or
 *  not.
 * @error: A #GError to store the result of the operation.
 *
 * Gets the zoom value from the #GDigicamManager object.
 *
 * Returns: #True if success, #FALSE otherwise.
 **/
gboolean
g_digicam_manager_get_zoom (GDigicamManager   *manager,
                            gdouble           *zoom,
                            gboolean          *digital,
                            GError           **error)
{
    GDigicamManagerPrivate *priv = NULL;
    gboolean result = FALSE;
    gchar *error_msg = NULL;
    GDigicamError error_code = G_DIGICAM_ERROR_FAILED;

    g_return_val_if_fail (G_DIGICAM_IS_MANAGER (manager), FALSE);
    g_return_val_if_fail (NULL != zoom, FALSE);
    g_return_val_if_fail (NULL != digital, FALSE);


    priv = G_DIGICAM_MANAGER_GET_PRIVATE (manager);

    /* Check for GStreamer bin */
    if (NULL == priv->gst_bin) {
        error_code = G_DIGICAM_ERROR_GSTREAMER_BIN_NOT_SET;
        error_msg = g_strdup ("imposible to get the zoom "
                              "since there is no GStreamer bin "
                              "from which to get it");
        goto error;
    }

    /* Check zoom capabilities */
    if (!(priv->descriptor->supported_features &
	  (G_DIGICAM_CAPABILITIES_OPTICALZOOM |
           G_DIGICAM_CAPABILITIES_DIGITALZOOM))) {
        error_code = G_DIGICAM_ERROR_ZOOM_NOT_SUPPORTED;
        error_msg = g_strdup ("imposible to get the zoom "
                              "since the GStreamer bin "
                              "has not these capabilities");
        goto error;
    }

    /* Perform operation */
    *zoom = priv->zoom;
    *digital = priv->digital_zoom;

    result = TRUE;

error:
    if ((NULL != error) && (NULL == *error)) {
        if ((!result) && (NULL != error_msg)) {
            g_digicam_set_error (error, error_code, error_msg);
	}
    }

    /* Free */
    if (NULL != error_msg) {
        g_free (error_msg);
    }

    return result;
}


/*     gboolean g_digicam_manager_zoom_in (GDigicamManager   *manager, */
/*                                         GError           **error); */
/*     gboolean g_digicam_manager_zoom_out (GDigicamManager   *manager, */
/*                                          GError           **error); */


/**
 * g_digicam_manager_set_audio:
 * @manager: A #GDigicamManager
 * @audio: The audio state to set.
 * @error: A #GError to store the result of the operation.
 * @user_data: Data to be used with the customized set function
 *  provided by the user in the #GDigicamDescriptor.
 *
 * Sets the audio status for the video recording operation.
 *
 * Returns: #True if success, #False otherwise.
 **/
gboolean
g_digicam_manager_set_audio (GDigicamManager *manager,
                             GDigicamAudio    audio,
                             GError         **error,
                             gpointer         user_data)
{
    GDigicamManagerPrivate *priv = NULL;
    gboolean result = FALSE;
    gchar *error_msg = NULL;
    GDigicamError error_code = G_DIGICAM_ERROR_FAILED;

    g_return_val_if_fail (G_DIGICAM_IS_MANAGER (manager), FALSE);
    g_return_val_if_fail (!((audio & G_DIGICAM_AUDIO_PLAYBACKON) &&
                            (audio & G_DIGICAM_AUDIO_PLAYBACKOFF)), FALSE);
    g_return_val_if_fail (!((audio & G_DIGICAM_AUDIO_RECORDON) &&
                            (audio & G_DIGICAM_AUDIO_RECORDOFF)), FALSE);


    priv = G_DIGICAM_MANAGER_GET_PRIVATE (manager);

    /* Check GStreamer bin */
    if (NULL == priv->gst_bin) {
        error_code = G_DIGICAM_ERROR_GSTREAMER_BIN_NOT_SET;
        error_msg = g_strdup ("imposible to set the audio state "
                              "since there is no GStreamer bin.");
        goto error;
    }

    /* Check video mode */
    if (priv->mode != G_DIGICAM_MODE_VIDEO) {
        error_code = G_DIGICAM_ERROR_INVALID_MODE;
        error_msg = g_strdup ("imposible to set the audio state "
                              "since camera is not in video "
                              "mode, required for this operation.");
        goto error;
    }

    /* Check audio capability */
    if (!(priv->descriptor->supported_features &
          G_DIGICAM_CAPABILITIES_AUDIO)) {
        error_code = G_DIGICAM_ERROR_AUDIO_NOT_SUPPORTED;
        error_msg = g_strdup ("imposible to set the audio state "
                              "since the GStreamer bin "
                              "has not this capability");
        goto error;
    }

    /* Check audio capabilities */
    if (!(priv->descriptor->supported_audio_states &
          audio)) {
        error_code = G_DIGICAM_ERROR_AUDIO_NOT_SUPPORTED;
        error_msg = g_strdup ("imposible to set the audio state "
                              "since the GStreamer bin "
                              "has not this capability.");
        goto error;
    }

    /* Check function handler */
    if (NULL == priv->descriptor->set_audio_func) {
        error_code = G_DIGICAM_ERROR_FAILED;
        error_msg = g_strdup ("error setting audio, "
                              "there is not function handler");
        goto error;
    }

    /* Performs operation */
    G_DIGICAM_DEBUG ("GDigicam: Setting audio operation started\n");
    result = priv->descriptor->set_audio_func (manager,
                                               user_data);

    /* Check operation result */
    if (!result) {
        error_code = G_DIGICAM_ERROR_FAILED;
        error_msg = g_strdup ("internal error setting the "
                              "audio state "
			      "in the GStreamer bin.");
        goto error;
    }

    priv->audio = audio;

error:
    if ((NULL != error) && (NULL == *error)) {
        if ((!result) && (NULL != error_msg)) {
            g_digicam_set_error (error, error_code, error_msg);
	}
    }

    /* Free */
    if (NULL != error_msg) {
        g_free (error_msg);
    }

    return result;
}


/**
 * g_digicam_manager_get_audio:
 * @manager: A #GDigicamManager
 * @audio: The audio state to get.
 * @error: A #GError to store the result of the operation.
 *
 * Gets the audio state for the video recording operation.
 *
 * Returns: #True if success, #FALSE otherwise.
 **/
gboolean
g_digicam_manager_get_audio (GDigicamManager *manager,
                             GDigicamAudio   *audio,
                             GError         **error)
{
    GDigicamManagerPrivate *priv = NULL;
    gboolean result = FALSE;
    gchar *error_msg = NULL;
    GDigicamError error_code = G_DIGICAM_ERROR_FAILED;

    g_return_val_if_fail (G_DIGICAM_IS_MANAGER (manager), FALSE);


    priv = G_DIGICAM_MANAGER_GET_PRIVATE (manager);

    /* Check GStreamer bin */
    if (NULL == priv->gst_bin) {
        error_code = G_DIGICAM_ERROR_GSTREAMER_BIN_NOT_SET;
        error_msg = g_strdup ("imposible to get the audio state "
                              "since there is no GStreamer bin.");
        goto error;
    }

    /* Check video mode */
    if (priv->mode != G_DIGICAM_MODE_VIDEO) {
        error_code = G_DIGICAM_ERROR_INVALID_MODE;
        error_msg = g_strdup ("imposible to get the audio state "
                              "since camera is not in video "
                              "mode, required for this operation.");
        goto error;
    }

    /* Check audio capability */
    if (!(priv->descriptor->supported_features &
          G_DIGICAM_CAPABILITIES_AUDIO)) {
        error_code = G_DIGICAM_ERROR_AUDIO_NOT_SUPPORTED;
        error_msg = g_strdup ("imposible to get the audio state "
                              "since the GStreamer bin "
                              "has not this capability");
        goto error;
    }

    *audio = priv->audio;

    result = TRUE;

error:
    if ((NULL != error) && (NULL == *error)) {
        if ((!result) && (NULL != error_msg)) {
            g_digicam_set_error (error, error_code, error_msg);
	}
    }

    /* Free */
    if (NULL != error_msg) {
        g_free (error_msg);
    }

    return result;
}

/**
 * g_digicam_manager_set_preview_mode:
 * @manager: A #GDigicamManager
 * @mode: The #GDigicamPreview mode to set.
 * @error: A #GError to store the result of the operation.
 * @user_data: Data to be used with the customized set function
 *  provided by the user in the #GDigicamDescriptor.
 *
 * Set the preview mode to the low level plugins, if the camera plugin
 * descriptor provides the required capability
 * (#G_DIGICAM_CAPABILITIES_PREVIEW) and the #GDigicamPreview is
 * valid.
 *
 * Returns: #True if success, #False otherwise.
 **/
gboolean
g_digicam_manager_set_preview_mode (GDigicamManager *manager,
                                    GDigicamPreview mode,
                                    GError          **error,
                                    gpointer        user_data)
{
    GDigicamManagerPrivate *priv = NULL;
    gboolean result = FALSE;
    gchar *error_msg = NULL;
    GDigicamError error_code = G_DIGICAM_ERROR_FAILED;

    g_return_val_if_fail (G_DIGICAM_IS_MANAGER (manager), FALSE);

    priv = G_DIGICAM_MANAGER_GET_PRIVATE (manager);

    /* Check GStreamer bin */
    if (NULL == priv->gst_bin) {
        error_code = G_DIGICAM_ERROR_GSTREAMER_BIN_NOT_SET;
        error_msg = g_strdup ("imposible to set the preview mode "
                              "since there is no GStreamer bin.");
        goto error;
    }

    /* Check preview capability */
    if (!(priv->descriptor->supported_features &
          G_DIGICAM_CAPABILITIES_PREVIEW)) {
        error_code = G_DIGICAM_ERROR_PREVIEW_NOT_SUPPORTED;
        error_msg = g_strdup ("imposible to set the preview mode"
                              "since the GStreamer bin "
                              "has not this capability");
        goto error;
    }

    /* Check preview available modes */
    if (!(priv->descriptor->supported_preview_modes &
          mode)) {
        error_code = G_DIGICAM_ERROR_PREVIEW_NOT_SUPPORTED;
        error_msg = g_strdup ("imposible to set the preview mode "
                              "since the GStreamer bin "
                              "has not this mode available.");
        goto error;
    }

    /* Check function handler */
    if (NULL == priv->descriptor->set_preview_mode_func) {
        error_code = G_DIGICAM_ERROR_FAILED;
        error_msg = g_strdup ("error setting the preview mode, "
                              "there is not function handler");
        goto error;
    }

    /* Avoid to set the same value */
    if (mode == priv->preview_mode) {
        result = TRUE;
        G_DIGICAM_DEBUG ("GDigicamManager: The new preview mode is the same "
                         "than the old one\n");
        goto error;
    }

    /* Performs operation */
    G_DIGICAM_DEBUG ("GDigicamManager: Settings the preview mode  ...\n");
    result = priv->descriptor->set_preview_mode_func (manager,
                                                      user_data);

    /* Check operation result */
    if (!result) {
        error_code = G_DIGICAM_ERROR_FAILED;
        error_msg = g_strdup ("internal error setting the "
                              "preview mode "
			      "in the GStreamer bin.");
        goto error;
    }

    priv->preview_mode = mode;

error:
    if ((NULL != error) && (NULL == *error)) {
        if ((!result) && (NULL != error_msg)) {
            g_digicam_set_error (error, error_code, error_msg);
	}
    }

    /* Free */
    if (NULL != error_msg) {
        g_free (error_msg);
    }

    return result;
}


/**
 * g_digicam_manager_get_preview_mode:
 * @manager: A #GDigicamManager
 * @mode: The activation of the preview features.
 * @error: A #GError to store the result of the operation.
 *
 * Gets the #GDigicamPreview mode from the #GDigicamManager object.
 *
 * Returns: #True if success, #FALSE otherwise.
 **/
gboolean
g_digicam_manager_get_preview_mode (GDigicamManager *manager,
                                    GDigicamPreview *mode,
                                    GError          **error)
{
    GDigicamManagerPrivate *priv = NULL;
    gboolean result = FALSE;
    gchar *error_msg = NULL;
    GDigicamError error_code = G_DIGICAM_ERROR_FAILED;

    g_return_val_if_fail (G_DIGICAM_IS_MANAGER (manager), FALSE);
    g_return_val_if_fail (NULL != mode, FALSE);

    priv = G_DIGICAM_MANAGER_GET_PRIVATE (manager);

    /* Check GStreamer bin */
    if (NULL == priv->gst_bin) {
        error_code = G_DIGICAM_ERROR_GSTREAMER_BIN_NOT_SET;
        error_msg = g_strdup ("imposible to get the preview mode "
                              "since there is no GStreamer bin");
        *mode = G_DIGICAM_PREVIEW_NONE;
        goto error;
    }

    /* Check preview capability */
    if (!(priv->descriptor->supported_features &
          G_DIGICAM_CAPABILITIES_PREVIEW)) {
        error_code = G_DIGICAM_ERROR_PREVIEW_NOT_SUPPORTED;
        error_msg = g_strdup ("imposible to perform the operation "
                              "since the GStreamer bin "
                              "has not this capability");
        *mode = G_DIGICAM_PREVIEW_NONE;
        goto error;
    }

    /* Performs operation */
    *mode = priv->preview_mode;
    result = TRUE;

error:
    if ((NULL != error) && (NULL == *error)) {
        if ((!result) && (NULL != error_msg)) {
            g_digicam_set_error (error, error_code, error_msg);
	}
    }

    /* Free */
    if (NULL != error_msg) {
        g_free (error_msg);
    }

    return result;
}


/**
 * g_digicam_manager_preview_enabled:
 * @manager: A #GDigicamManager
 * @enabled: The activation of the preview features.
 * @error: A #GError to store the result of the operation.
 *
 * Wheather the preview features are enabled or not.
 *
 * Returns: #TRUE if preview is enabled, #FALSE otherwise, or in case
 * of invalid input arguments.
 **/
gboolean
g_digicam_manager_preview_enabled (GDigicamManager *manager,
                                   gboolean        *enabled,
                                   GError          **error)
{
    GDigicamManagerPrivate *priv = NULL;
    gboolean result = FALSE;
    gchar *error_msg = NULL;
    GDigicamError error_code = G_DIGICAM_ERROR_FAILED;
    gint enabled_modes;

    g_return_val_if_fail (G_DIGICAM_IS_MANAGER (manager), FALSE);
    g_return_val_if_fail (NULL != enabled, FALSE);

    priv = G_DIGICAM_MANAGER_GET_PRIVATE (manager);

    /* Check GStreamer bin */
    if (NULL == priv->gst_bin) {
        error_code = G_DIGICAM_ERROR_GSTREAMER_BIN_NOT_SET;
        error_msg = g_strdup ("imposible to get if the preview is enabled "
                              "since there is no GStreamer bin");
        *enabled = FALSE;
        goto error;
    }

    /* Check preview capability */
    if (!(priv->descriptor->supported_features &
          G_DIGICAM_CAPABILITIES_PREVIEW)) {
        error_code = G_DIGICAM_ERROR_PREVIEW_NOT_SUPPORTED;
        error_msg = g_strdup ("imposible to perform the operation "
                              "since the GStreamer bin "
                              "has not this capability");
        *enabled = FALSE;
        goto error;
    }

    /* Check preview mode */
    enabled_modes = G_DIGICAM_PREVIEW_ON;
    *enabled = (priv->preview_mode & enabled_modes);
    result = TRUE;

error:
    if ((NULL != error) && (NULL == *error)) {
        if ((!result) && (NULL != error_msg)) {
            g_digicam_set_error (error, error_code, error_msg);
	}
    }

    /* Free */
    if (NULL != error_msg) {
        g_free (error_msg);
    }

    return result;
}


/*     gboolean g_digicam_manager_query_focus_region_patterns (GDigicamManager    *manager, */
/*                                                             GDigicamFocuspoints *focus_point, */
/*                                                             guint64 *custom_points, */
/*                                                             GError            **error); */
/*     gboolean g_digicam_manager_query_supported_locks (GDigicamManager    *manager, */
/*                                                       GSList *slist, */
/*                                                       GError            **error); */


/**
 * g_digicam_manager_play_bin:
 * @manager: A #GDigicamManager
 * @xwindow_id: Identifier of the X window to play in.
 * @error: A #GError to store the result of the operation.
 *
 * Plays the bin.
 *
 * Returns: #True if success, #False otherwise.
 **/
gboolean
g_digicam_manager_play_bin (GDigicamManager *manager,
			    gulong           xwindow_id,
			    GError         **error)
{
    GDigicamManagerPrivate *priv = NULL;
    gboolean result = FALSE;
    gchar *error_msg = NULL;
    GDigicamError error_code = G_DIGICAM_ERROR_FAILED;

    g_return_val_if_fail (G_DIGICAM_IS_MANAGER (manager), FALSE);


    priv = G_DIGICAM_MANAGER_GET_PRIVATE (manager);

    /* Check GStreamer bin */
    if (NULL == priv->gst_bin) {
        error_code = G_DIGICAM_ERROR_GSTREAMER_BIN_NOT_SET;
        error_msg = g_strdup ("imposible to play the bin "
                              "since there is no GStreamer bin");
        goto error;
    }

    /* store the xwindow_id in the priv */
    priv->xwindow_id = xwindow_id;

    if ((priv->descriptor->supported_features &
         G_DIGICAM_CAPABILITIES_VIEWFINDER) &&
	(NULL != priv->descriptor->viewfinder_sink)) {
	gst_x_overlay_set_xwindow_id (GST_X_OVERLAY (priv->descriptor->viewfinder_sink),
				      priv->xwindow_id);
    }

    if (GST_STATE_CHANGE_FAILURE ==
        gst_element_set_state (priv->gst_pipeline, GST_STATE_PLAYING)) {
        error_code = G_DIGICAM_ERROR_FAILED;
        error_msg = g_strdup ("impossible to set the playing state "
                              "in the pipeline");
        goto error;
    }

    result = TRUE;

error:
    if ((NULL != error) && (NULL == *error)) {
        if ((!result) && (NULL != error_msg)) {
            g_digicam_set_error (error, error_code, error_msg);
	}
    }

    /* Free */
    if (NULL != error_msg) {
        g_free(error_msg);
    }

    return result;
}


/**
 * g_digicam_manager_stop_bin:
 * @manager: A #GDigicamManager
 * @error: A #GError to store the result of the operation.
 *
 * Stops the bin.
 *
 * Returns: #True if success, #False otherwise.
 **/
gboolean
g_digicam_manager_stop_bin (GDigicamManager *manager,
			    GError **error)
{
    GDigicamManagerPrivate *priv = NULL;
    gboolean result = FALSE;
    gchar *error_msg = NULL;
    GDigicamError error_code = G_DIGICAM_ERROR_FAILED;

    g_return_val_if_fail (G_DIGICAM_IS_MANAGER (manager), FALSE);


    priv = G_DIGICAM_MANAGER_GET_PRIVATE (manager);

    /* Check GStreamer bin */
    if (NULL == priv->gst_bin) {
        error_code = G_DIGICAM_ERROR_GSTREAMER_BIN_NOT_SET;
        error_msg = g_strdup ("imposible to stop the bin "
                              "since there is no GStreamer bin");
        goto error;
    }

    /* reset the xwindow_id in the priv */
    priv->xwindow_id = 0;

    if (GST_STATE_CHANGE_FAILURE ==
        gst_element_set_state (priv->gst_pipeline, GST_STATE_NULL)) {
        error_code = G_DIGICAM_ERROR_FAILED;
        error_msg = g_strdup ("impossible to set the stop "
                              "state in the pipeline");
        goto error;
    }

    result = TRUE;

error:
    if ((NULL != error) && (NULL == *error)) {
        if ((!result) && (NULL != error_msg)) {
            g_digicam_set_error (error, error_code, error_msg);
	}
    }

    /* Free */
    if (NULL != error_msg) {
        g_free(error_msg);
    }

    return result;
}


/**
 * g_digicam_manager_capture_still_picture:
 * @manager: A #GDigicamManager
 * @filename: The file name to store the still picture created.
 * @error: A #GError to store the result of the operation.
 * @user_data: Data to be used with the customized set function
 *  provided by the user in the #GDigicamDescriptor.
 *
 * Captures a still picture.
 *
 * Returns: #True if success, #False otherwise.
 **/
gboolean
g_digicam_manager_capture_still_picture (GDigicamManager *manager,
                                         const gchar     *filename,
                                         GError         **error,
                                         gpointer         user_data)
{
    GDigicamManagerPrivate *priv = NULL;
    gboolean result = FALSE;
    gchar *error_msg = NULL;
    GDigicamError error_code = G_DIGICAM_ERROR_FAILED;

    g_return_val_if_fail (G_DIGICAM_IS_MANAGER (manager), FALSE);


    priv = G_DIGICAM_MANAGER_GET_PRIVATE (manager);

    /* Check GStreamer bin */
    if (NULL == priv->gst_bin) {
        error_code = G_DIGICAM_ERROR_GSTREAMER_BIN_NOT_SET;
        error_msg = g_strdup ("imposible to start still picture capture "
                              "since there is no GStreamer bin.");
        goto error;
    }

    /* Check still picture mode */
    if (priv->mode != G_DIGICAM_MODE_STILL) {
        error_code = G_DIGICAM_ERROR_INVALID_MODE;
        error_msg = g_strdup ("imposible to start still picture capture "
                              "since camera is not in still picture "
                              "mode, required for this operation.");
        goto error;
    }

    /* Check function handler */
    if (NULL == priv->descriptor->get_still_picture_func) {
        error_code = G_DIGICAM_ERROR_FAILED;
        error_msg = g_strdup ("error starting still picture capture, "
                              "there is not function handler");
        goto error;
    }

    /* Release AutoFocus locks */
    priv->locks = 0;

    /* Performs operation */
    G_DIGICAM_DEBUG ("GDigicam: Capture still picture operation started\n");
    result = priv->descriptor->get_still_picture_func (manager,
                                                       user_data);

    /* Check operation result */
    if (!result) {
        error_code = G_DIGICAM_ERROR_FAILED;
        error_msg = g_strdup ("internal error starting still picture capture "
                              "in the GStreamer bin");
        goto error;
    }

error:
    if ((NULL != error) && (NULL == *error)) {
        if ((!result) && (NULL != error_msg)) {
            g_digicam_set_error (error, error_code, error_msg);
        }
    }

    /* Free */
    if (NULL != error_msg) {
        g_free (error_msg);
    }

    return result;
}


/**
 * g_digicam_manager_start_recording_video:
 * @manager: A #GDigicamManager
 * @error: A #GError to store the result of the operation.
 * @user_data: Data to be used with the customized set function
 *  provided by the user in the #GDigicamDescriptor.
 *
 * Starts the video recording.
 *
 * Returns: #True if success, #False otherwise.
 **/
gboolean
g_digicam_manager_start_recording_video (GDigicamManager *manager,
                                         GError         **error,
					 gpointer         user_data)
{
    GDigicamManagerPrivate *priv = NULL;
    gboolean result = FALSE;
    gchar *error_msg = NULL;
    GDigicamError error_code = G_DIGICAM_ERROR_FAILED;

    g_return_val_if_fail (G_DIGICAM_IS_MANAGER (manager), FALSE);


    priv = G_DIGICAM_MANAGER_GET_PRIVATE (manager);

    /* Check GStreamer bin */
    if (NULL == priv->gst_bin) {
        error_code = G_DIGICAM_ERROR_GSTREAMER_BIN_NOT_SET;
        error_msg = g_strdup ("imposible to start video recording "
                              "since there is no GStreamer bin.");
        goto error;
    }

    /* Check video mode */
    if (priv->mode != G_DIGICAM_MODE_VIDEO) {
        error_code = G_DIGICAM_ERROR_INVALID_MODE;
        error_msg = g_strdup ("imposible to start video recording "
                              "since camera is not in video "
                              "mode, required for this operation.");
        goto error;
    }

    /* Check function handler */
    if (NULL == priv->descriptor->start_recording_video_func) {
        error_code = G_DIGICAM_ERROR_FAILED;
        error_msg = g_strdup ("error starting video record, "
                              "there is not function handler");
        goto error;
    }

    /* Performs operation */
    G_DIGICAM_DEBUG ("GDigicam: Record video operation started\n");
    result = priv->descriptor->start_recording_video_func (manager,
                                                           user_data);

    /* Check operation result */
    if (!result) {
        error_code = G_DIGICAM_ERROR_FAILED;
        error_msg = g_strdup ("internal error starting video recording "
			      "in the GStreamer bin.");
        goto error;
    }

error:
    if ((NULL != error) && (NULL == *error)) {
        if ((!result) && (NULL != error_msg)) {
            g_digicam_set_error (error, error_code, error_msg);
	}
    }

    /* Free */
    if (NULL != error_msg) {
        g_free (error_msg);
    }

    return result;
}

/**
 * g_digicam_manager_pause_recording_video:
 * @manager: A #GDigicamManager
 * @resume: Whether to resume video recording or not.
 * @error: A #GError to store the result of the operation.
 * @user_data: Data to be used with the customized set function
 *  provided by the user in the #GDigicamDescriptor.
 *
 * Pauses the video recording.
 *
 * Returns: #True if success, #False otherwise.
 **/
gboolean
g_digicam_manager_pause_recording_video (GDigicamManager *manager,
                                         gboolean resume,
                                         GError **error,
					 gpointer user_data)
{
    GDigicamManagerPrivate *priv = NULL;
    gboolean result = FALSE;
    gchar *error_msg = NULL;
    GDigicamError error_code = G_DIGICAM_ERROR_FAILED;

    g_return_val_if_fail (G_DIGICAM_IS_MANAGER (manager), FALSE);


    priv = G_DIGICAM_MANAGER_GET_PRIVATE (manager);

    /* Check GStreamer bin */
    if (NULL == priv->gst_bin) {
        error_code = G_DIGICAM_ERROR_GSTREAMER_BIN_NOT_SET;
        error_msg = g_strdup ("imposible to pause video recording "
                              "since there is no GStreamer bin.");
        goto error;
    }

    /* Check video mode */
    if (priv->mode != G_DIGICAM_MODE_VIDEO) {
        error_code = G_DIGICAM_ERROR_INVALID_MODE;
        error_msg = g_strdup ("imposible to pause video recording "
                              "since camera is not in video "
                              "mode, required for this operation.");
        goto error;
    }

    /* Check function handler */
    if (NULL == priv->descriptor->pause_recording_video_func) {
        error_code = G_DIGICAM_ERROR_FAILED;
        error_msg = g_strdup ("error pausing video record, "
                              "there is not function handler");
        goto error;
    }

    /* Performs operation */
    G_DIGICAM_DEBUG ("GDigicam: Pause video operation started\n");
    result = priv->descriptor->pause_recording_video_func (manager,
                                                           user_data);

    /* Check operation result */
    if (!result) {
        error_code = G_DIGICAM_ERROR_FAILED;
        error_msg = g_strdup ("internal error pausing video recording "
			      "in the GStreamer bin.");
        goto error;
    }

error:
    if ((NULL != error) && (NULL == *error)) {
        if ((!result) && (NULL != error_msg)) {
            g_digicam_set_error (error, error_code, error_msg);
	}
    }

    /* Free */
    if (NULL != error_msg) {
        g_free (error_msg);
    }

    return result;
}

/**
 * g_digicam_manager_finish_recording_video:
 * @manager: A #GDigicamManager
 * @error: A #GError to store the result of the operation.
 * @user_data: Data to be used with the customized set function
 *  provided by the user in the #GDigicamDescriptor.
 *
 * Stops the video recording.
 *
 * Returns: #True if success, #False otherwise.
 **/
gboolean
g_digicam_manager_finish_recording_video (GDigicamManager *manager,
                                          GError         **error,
					  gpointer         user_data)
{
    GDigicamManagerPrivate *priv = NULL;
    gboolean result = FALSE;
    gchar *error_msg = NULL;
    GDigicamError error_code = G_DIGICAM_ERROR_FAILED;

    g_return_val_if_fail (G_DIGICAM_IS_MANAGER (manager), FALSE);


    priv = G_DIGICAM_MANAGER_GET_PRIVATE (manager);

    /* Check GStreamer bin */
    if (NULL == priv->gst_bin) {
        error_code = G_DIGICAM_ERROR_GSTREAMER_BIN_NOT_SET;
        error_msg = g_strdup ("imposible to finish video recording "
                              "since there is no GStreamer bin.");
        goto error;
    }

    /* Check video mode */
    if (priv->mode != G_DIGICAM_MODE_VIDEO) {
        error_code = G_DIGICAM_ERROR_INVALID_MODE;
        error_msg = g_strdup ("imposible to finish video recording "
                              "since camera is not in video "
                              "mode, required for this operation.");
        goto error;
    }

    /* Check function handler */
    if (NULL == priv->descriptor->finish_recording_video_func) {
        error_code = G_DIGICAM_ERROR_FAILED;
        error_msg = g_strdup ("error finishing video record, "
                              "there is not function handler");
        goto error;
    }

    /* Performs operation */
    G_DIGICAM_DEBUG ("GDigicam: Finish video operation started\n");
    result = priv->descriptor->finish_recording_video_func (manager,
                                                            user_data);

    /* Check operation result */
    if (!result) {
        error_code = G_DIGICAM_ERROR_FAILED;
        error_msg = g_strdup ("error finishing video recording "
			      "in the GStreamer bin.");
        goto error;
    }

    result = TRUE;

error:
    if ((NULL != error) && (NULL == *error)) {
        if ((!result) && (NULL != error_msg)) {
            g_digicam_set_error (error, error_code, error_msg);
	}
    }

    /* Free */
    if (NULL != error_msg) {
        g_free (error_msg);
    }

    return result;
}


/**
 * g_digicam_manager_descriptor_new:
 *
 * Creates an new and initialized #GDigicamDescriptor.
 *
 * Returns: the new and initialized #GDigicamDescriptor.
 **/
GDigicamDescriptor*
g_digicam_manager_descriptor_new (void)
{
    GDigicamDescriptor *descriptor = NULL;

    descriptor = g_new0 (GDigicamDescriptor, 1);

    return descriptor;
}


/**
 * g_digicam_manager_descriptor_free:
 * @descriptor: the #GDigicamDescritor to free.
 *
 * Frees a #GDigicamDescritor.
 **/
void
g_digicam_manager_descriptor_free (GDigicamDescriptor *descriptor)
{
    g_return_if_fail (NULL != descriptor);

    if (NULL != descriptor->name) {
        g_free (descriptor->name);
    }

    if (NULL != descriptor->viewfinder_sink) {
        gst_object_unref (GST_OBJECT (descriptor->viewfinder_sink));
    }

    g_free (descriptor);
}


/**
 * g_digicam_manager_descriptor_copy:
 * @orig_descriptor: the #GDigicamDescritor to copy.
 *
 * Duplicates a #GDigicamDescriptor.
 *
 * Returns: the duplicated #GDigicamDescriptor.
 **/
GDigicamDescriptor*
g_digicam_manager_descriptor_copy (const GDigicamDescriptor *orig_descriptor)
{
    GDigicamDescriptor *descriptor = NULL;

    descriptor = g_new0 (GDigicamDescriptor, 1);

    if (NULL != orig_descriptor->name) {
        descriptor->name = g_strdup (orig_descriptor->name);
    }
    if (NULL != orig_descriptor->viewfinder_sink) {
        descriptor->viewfinder_sink = orig_descriptor->viewfinder_sink;
        /* FIXME: Is this really needed? */
        gst_object_ref (GST_OBJECT (descriptor->viewfinder_sink));
    }
    descriptor->supported_features = orig_descriptor->supported_features;
    descriptor->supported_modes = orig_descriptor->supported_modes;
    descriptor->set_mode_func = orig_descriptor->set_mode_func;
    descriptor->supported_flash_modes = orig_descriptor->supported_flash_modes;
    descriptor->set_flash_mode_func = orig_descriptor->set_flash_mode_func;
    descriptor->supported_focus_modes = orig_descriptor->supported_focus_modes;
    descriptor->set_focus_mode_func = orig_descriptor->set_focus_mode_func;
    descriptor->set_focus_region_pattern_func = orig_descriptor->set_focus_region_pattern_func;
    descriptor->supported_exposure_modes = orig_descriptor->supported_exposure_modes;
    descriptor->set_exposure_mode_func = orig_descriptor->set_exposure_mode_func;
    descriptor->set_exposure_comp_func = orig_descriptor->set_exposure_comp_func;
    descriptor->supported_iso_sensitivity_modes = orig_descriptor->supported_iso_sensitivity_modes;
    descriptor->set_iso_sensitivity_mode_func = orig_descriptor->set_iso_sensitivity_mode_func;
    descriptor->supported_white_balance_modes = orig_descriptor->supported_white_balance_modes;
    descriptor->set_white_balance_mode_func = orig_descriptor->set_white_balance_mode_func;
    descriptor->supported_aspect_ratios = orig_descriptor->supported_metering_modes;
    descriptor->set_metering_mode_func = orig_descriptor->set_metering_mode_func;
    descriptor->supported_aspect_ratios = orig_descriptor->supported_aspect_ratios;
    descriptor->set_aspect_ratio_func = orig_descriptor->set_aspect_ratio_func;
    descriptor->set_aspect_ratio_resolution_func = orig_descriptor->set_aspect_ratio_resolution_func;
    descriptor->supported_qualities = orig_descriptor->supported_qualities;
    descriptor->set_quality_func = orig_descriptor->set_quality_func;
    descriptor->supported_resolutions = orig_descriptor->supported_resolutions;
    descriptor->set_resolution_func = orig_descriptor->set_resolution_func;
    descriptor->max_zoom_macro_disabled = orig_descriptor->max_zoom_macro_disabled;
    descriptor->max_zoom_macro_enabled = orig_descriptor->max_zoom_macro_enabled;
    descriptor->max_optical_zoom_macro_disabled = orig_descriptor->max_optical_zoom_macro_disabled;
    descriptor->max_optical_zoom_macro_enabled = orig_descriptor->max_optical_zoom_macro_enabled;
    descriptor->max_digital_zoom = orig_descriptor->max_digital_zoom;
    descriptor->set_zoom_func = orig_descriptor->set_zoom_func;
    descriptor->supported_audio_states = orig_descriptor->supported_audio_states;
    descriptor->set_audio_func = orig_descriptor->set_audio_func;
    descriptor->supported_preview_modes = orig_descriptor->supported_preview_modes;
    descriptor->set_preview_mode_func = orig_descriptor->set_preview_mode_func;
    descriptor->set_locks_func = orig_descriptor->set_locks_func;
    descriptor->get_still_picture_func = orig_descriptor->get_still_picture_func;
    descriptor->start_recording_video_func = orig_descriptor->start_recording_video_func;
    descriptor->pause_recording_video_func = orig_descriptor->pause_recording_video_func;
    descriptor->finish_recording_video_func = orig_descriptor->finish_recording_video_func;
    descriptor->handle_bus_message_func = orig_descriptor->handle_bus_message_func;
    descriptor->handle_sync_bus_message_func = orig_descriptor->handle_sync_bus_message_func;

    return descriptor;
}


/***************************************/
/* Protected visivility functions      */
/***************************************/

void
_g_digicam_manager_set_capture_lock (GDigicamManager *manager)
{
    GDigicamManagerPrivate *priv = NULL;

    g_assert (G_DIGICAM_IS_MANAGER (manager));
    priv = G_DIGICAM_MANAGER_GET_PRIVATE (manager);

    g_mutex_lock (priv->capture_lock);
}

void
_g_digicam_manager_release_capture_lock (GDigicamManager *manager)
{
    GDigicamManagerPrivate *priv = NULL;

    g_assert (G_DIGICAM_IS_MANAGER (manager));
    priv = G_DIGICAM_MANAGER_GET_PRIVATE (manager);

    g_mutex_unlock (priv->capture_lock);
}

gboolean
_g_digicam_manager_is_valid_flag (GDigicamManager *manager,
                                  guint32 flag,
                                  guint32 low,
                                  guint32 high)
{
    gboolean valid;
    guint32 n;

    g_assert (G_DIGICAM_IS_MANAGER (manager));

    /* Flags should be power of 2 */
    n = flag;
    valid = (n & (n-1)) == 0;

    /* It should be between the defined limits */
    valid = valid && ((low > flag) && (flag < high));

    return valid;
}

gboolean
_g_digicam_manager_is_valid_enum (GDigicamManager *manager,
                                  guint32 value,
                                  guint32 low,
                                  guint32 high)
{
    gboolean valid;

    g_assert (G_DIGICAM_IS_MANAGER (manager));

    /* It should be between the defined limits */
    valid = (low > value) && (value < high);

    return valid;
}

/***************************************/
/* Private functions to manage GObject */
/***************************************/

static void
_g_digicam_manager_class_init (GDigicamManagerClass *klass)
{
    GObjectClass *object_class = (GObjectClass*) klass;

    g_assert (G_DIGICAM_IS_MANAGER_CLASS (klass));

    parent_class = g_type_class_peek_parent (klass);

    g_type_class_add_private (klass, sizeof (GDigicamManagerPrivate));

    /* Override virtual functions */
    object_class->finalize = _g_digicam_manager_finalize;

    /**
     * GDigicamManager::focus-done:
     * @manager: the gdigicam manager
     * @status: the focus status
     *
     * Signal emited when the focus process is finished.
     */

    manager_signals[FOCUS_DONE_SIGNAL] =
        g_signal_new ("focus-done",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST,
                      G_STRUCT_OFFSET (GDigicamManagerClass, focus_done),
                      NULL, NULL,
                      g_cclosure_marshal_VOID__UINT,
                      G_TYPE_NONE, 1, G_TYPE_UINT);

    /**
     * GDigicamManager::pict-done:
     * @manager: the gdigicam manager
     * @filename: the name of the file just saved
     *
     * Signal emited when the picture has just been taken. To continue taking
     * pictures just update @filename and return TRUE, otherwise return FALSE.
     */

    manager_signals[PICT_DONE_SIGNAL] =
        g_signal_new ("pict-done",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST,
                      G_STRUCT_OFFSET (GDigicamManagerClass, pict_done),
                      NULL, NULL,
                      gdigicam_marshal_BOOLEAN__STRING,
                      G_TYPE_BOOLEAN, 1, G_TYPE_STRING);

    /**
     * GDigicamManager::capture-start:
     * @manager: the gdigicam manager
     *
     * Signal emited when the picture is going to be taken.
     */

    manager_signals[CAPTURE_START_SIGNAL] =
        g_signal_new ("capture-start",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST,
                      G_STRUCT_OFFSET (GDigicamManagerClass, capture_start),
                      NULL, NULL,
                      g_cclosure_marshal_VOID__VOID,
                      G_TYPE_NONE, 0);

    /**
     * GDigicamManager::capture-end:
     * @manager: the gdigicam manager
     *
     * Signal emited when the capture operation is finished.
     */

    manager_signals[CAPTURE_END_SIGNAL] =
        g_signal_new ("capture-end",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST,
                      G_STRUCT_OFFSET (GDigicamManagerClass, capture_end),
                      NULL, NULL,
                      g_cclosure_marshal_VOID__VOID,
                      G_TYPE_NONE, 0);

    /**
     * GDigicamManager::image-preview:
     * @manager: the gdigicam manager
     *
     * Signal emited when the image preview is generated.
     */

    manager_signals[PREVIEW_SIGNAL] =
        g_signal_new ("image-preview",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST,
                      G_STRUCT_OFFSET (GDigicamManagerClass, image_preview),
                      NULL, NULL,
                      g_cclosure_marshal_VOID__OBJECT,
                      G_TYPE_NONE, 1, GDK_TYPE_PIXBUF);

    /**
     * GDigicamManager::picture-got:
     * @manager: the gdigicam manager
     *
     * Signal emited during the capture, when the picture has just
     * been got.
     */

    manager_signals[PICTURE_GOT_SIGNAL] =
        g_signal_new ("picture-got",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST,
                      G_STRUCT_OFFSET (GDigicamManagerClass, picture_got),
                      NULL, NULL,
                      g_cclosure_marshal_VOID__VOID,
                      G_TYPE_NONE, 0);

    /**
     * GDigicamManager::internal-error:
     * @manager: the gdigicam manager
     *
     * Signal emited when an error is detected in the lower layers.
     */

    manager_signals[INTERNAL_ERROR_SIGNAL] =
        g_signal_new ("internal-error",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST,
                      G_STRUCT_OFFSET (GDigicamManagerClass, internal_error),
                      NULL, NULL,
                      g_cclosure_marshal_VOID__VOID,
                      G_TYPE_NONE, 0);
}

static void
_g_digicam_manager_init (GDigicamManager *manager)
{
    GDigicamManagerPrivate *priv = NULL;

    g_assert (G_DIGICAM_IS_MANAGER (manager));


    priv = G_DIGICAM_MANAGER_GET_PRIVATE (manager);
    g_assert (priv);

    priv->gst_bin = NULL;
    priv->gst_pipeline = NULL;
    priv->gst_bus = NULL;
    priv->xwindow_id = 0;
    priv->descriptor = NULL;
    priv->mode = G_DIGICAM_MODE_NONE;
    priv->flash_mode = G_DIGICAM_FLASHMODE_NONE;
    priv->is_flash_ready = FALSE;
    priv->focus_mode = G_DIGICAM_FOCUSMODE_NONE;
    priv->is_macro_enabled = FALSE;
    priv->focus_points = G_DIGICAM_FOCUSPOINTS_NONE;
    priv->active_points = G_DIGICAM_FOCUSPOINTS_NONE;
    priv->focus_region_positions = NULL;
    priv->exposure_mode = G_DIGICAM_EXPOSUREMODE_NONE;
    priv->exposure_compensation = 0;
    priv->iso_sensitivity_mode = G_DIGICAM_ISOSENSITIVITYMODE_NONE;
    priv->iso_level = 0;
    priv->white_balance_mode = G_DIGICAM_WHITEBALANCEMODE_NONE;
    priv->white_balance_level = 0;
    priv->metering_mode = G_DIGICAM_METERINGMODE_NONE;
    priv->aspect_ratio = G_DIGICAM_ASPECTRATIO_NONE;
    priv->quality = G_DIGICAM_QUALITY_NONE;
    priv->resolution = G_DIGICAM_RESOLUTION_NONE;
    priv->locks = G_DIGICAM_LOCK_AUTOFOCUS_NONE;
    priv->zoom = MIN_ZOOM;
    priv->digital_zoom = FALSE;
    priv->audio = G_DIGICAM_AUDIO_NONE;
    priv->preview_mode = G_DIGICAM_PREVIEW_NONE;
    priv->capture_lock = g_mutex_new ();
}

static void
_g_digicam_manager_finalize (GObject *object)
{
    GDigicamManager* manager = NULL;
    GDigicamManagerPrivate *priv = NULL;

    g_assert (G_DIGICAM_IS_MANAGER (object));
    manager = G_DIGICAM_MANAGER (object);

    priv = G_DIGICAM_MANAGER_GET_PRIVATE (manager);
    g_assert (priv);

    if (priv->capture_lock) {
        g_mutex_free (priv->capture_lock);
        priv->capture_lock = NULL;
    }

    _g_digicam_manager_free_private (priv);

    super->finalize (object);
}

static GDigicamManager*
_g_digicam_manager_new (void)
{
    return g_object_new (G_DIGICAM_TYPE_MANAGER, NULL);
}

/****************************************************************/
/* Private functions implementing the abstract public functions */
/****************************************************************/

static GDigicamDescriptor*
_g_digicam_manager_descriptor_builder (GstElement *gst_bin)
{
    GDigicamDescriptor *descriptor = NULL;
    GstCaps *input_caps = NULL;

    if (NULL != gst_bin) {
        GstElement *gst_element = NULL;

        descriptor = g_digicam_manager_descriptor_new ();

        /* Name */
        descriptor->name = gst_element_get_name (gst_bin);

        /* Viewfinder */
        gst_element = gst_bin_get_by_interface (GST_BIN (gst_bin),
                                                GST_TYPE_X_OVERLAY);
        if (NULL != gst_element) {
            descriptor->viewfinder_sink = gst_element;
            /* FIXME: Is this not really needed? */
/*             gst_object_ref (GST_OBJECT (descriptor->viewfinder_sink)); */
            descriptor->supported_features = descriptor->supported_features |
                G_DIGICAM_CAPABILITIES_VIEWFINDER;
        }

        /* Get device capabilities */
        g_object_get (G_OBJECT (gst_bin),
                      "inputcaps",
                      &input_caps,
                      NULL);

        /* Mapping device capabilities */
        if ((NULL != input_caps) &&
            GST_IS_CAPS (input_caps)) {
            _mapping_capabilities (input_caps, descriptor);
            gst_caps_unref (input_caps);
        }

        /* FIXME: Photo */

        /* FIXME: Video */
    }

    return descriptor;
}


static void
_mapping_capabilities (GstCaps *caps,
                       GDigicamDescriptor *descriptor)
{
    guint size;
    guint i;

    /* Get capabilities structures */
    size = gst_caps_get_size (caps);
    for (i = 0; i < size ; ++i) {
        GstStructure *st = NULL;

        /* maps single structure fields */
        st = gst_caps_get_structure(caps, i);
        g_return_if_fail (GST_IS_STRUCTURE(st));
        gst_structure_foreach (st,
                               _mapping_structure,
                               descriptor);
    }
}


gboolean
_mapping_structure  (GQuark field_id,
                     const GValue *value,
                     gpointer user_data)
{
    GDigicamDescriptor *descriptor = NULL;
    const gchar *field = NULL;
    guint32 feature = 0;

    g_return_val_if_fail (value != NULL, FALSE);
    g_return_val_if_fail (user_data != NULL, FALSE);
    descriptor = (GDigicamDescriptor *) user_data;
    field = g_quark_to_string (field_id);

    /* Mapping between structure and descriptor fields */
    if (!g_ascii_strcasecmp (field, G_DIGICAM_CAPABILITIES_VIEWFINDER_NAME)) {
        feature = G_DIGICAM_CAPABILITIES_VIEWFINDER;
    } else {
/*         if (!g_ascii_strcasecmp (field, G_DIGICAM_CAPABILITIES_PHOTO_NAME)) { */
/*             feature = G_DIGICAM_CAPABILITIES_PHOTO; */
/*         } */
    }

    /* Add suportes features to descriptor */
    descriptor->supported_features |= feature;

    return TRUE;
}


static gboolean
_g_digicam_manager_bus_callback (GstBus     *bus,
                                 GstMessage *message,
                                 gpointer    data)
{
    GDigicamManagerPrivate *priv = G_DIGICAM_MANAGER_GET_PRIVATE (data);

    switch (GST_MESSAGE_TYPE (message)) {
    case GST_MESSAGE_ERROR: {
        GError *err;
        gchar *debug;

        gst_message_parse_error (message, &err, &debug);
        G_DIGICAM_DEBUG ("GDigicamManager::_g_digicam_manager_bus_callback: "
                         "Error: %s.", err->message);
        g_error_free (err);
        g_free (debug);

	/* INTERNAL ERROR!!! STOP BIN!!! */
	g_digicam_manager_stop_bin (G_DIGICAM_MANAGER (data), NULL);
	/* Emit the internal error signal so any UI can handle it
         * properly. */
	g_signal_emit (G_OBJECT (data),
                       manager_signals [INTERNAL_ERROR_SIGNAL],
                       0);

        break;
    }
    case GST_MESSAGE_WARNING: {
        GError *err;
        gchar *debug;

        gst_message_parse_warning (message, &err, &debug);
        G_DIGICAM_WARN ("GDigicamManagerPrivate::_g_digicam_manager_bus_callback: "
                        "Warning: %s.", err->message);
        g_error_free (err);
        g_free (debug);
        break;
    }
    default:
	/* Nor error neither warning messages will be handled by the
         * plugin. */
	if (NULL != priv->descriptor->handle_bus_message_func) {
	    priv->descriptor->handle_bus_message_func (G_DIGICAM_MANAGER (data),
						       message);
	}
    }

    return TRUE;
}

static GstBusSyncReply
_g_digicam_manager_sync_bus_callback (GstBus     *bus,
				      GstMessage *message,
				      gpointer    data)
{

    GDigicamManagerPrivate *priv = G_DIGICAM_MANAGER_GET_PRIVATE (data);
    gboolean success = FALSE;

    if (GST_MESSAGE_TYPE (message) == GST_MESSAGE_ELEMENT) {
	if (NULL != priv->descriptor->handle_sync_bus_message_func) {
	    success = priv->descriptor->handle_sync_bus_message_func (G_DIGICAM_MANAGER (data),
								      message);
	}
    }

    if (success) {
	gst_message_unref (message);
	return (GST_BUS_DROP);
    } else {
	return (GST_BUS_PASS);
    }
}

static void
_g_digicam_manager_cleanup_bin (GDigicamManagerPrivate *priv)
{
    if (0 != bus_callback_source_id) {
        g_source_remove (bus_callback_source_id);
        bus_callback_source_id = 0;
    }

    if (GST_IS_BUS (priv->gst_bus)) {
        gst_bus_set_sync_handler (priv->gst_bus,
                                  NULL,
                                  NULL);
    }

    if (NULL != priv->descriptor) {
        g_digicam_manager_descriptor_free (priv->descriptor);
        priv->descriptor = NULL;
    }

    if (NULL != priv->gst_bus) {
        gst_object_unref (GST_OBJECT (priv->gst_bus));
        priv->gst_bus = NULL;
    }

    if (NULL != priv->gst_pipeline) {
        gst_element_set_state (priv->gst_pipeline, GST_STATE_NULL);
        /* FIXME: This blocks until the previous call has finished because
           it is async. We could remove this and wait for its completion
           notified by a message in the bus */
        gst_element_get_state (priv->gst_pipeline, NULL, NULL, GST_CLOCK_TIME_NONE);
        if (NULL != priv->gst_bin) {
            gst_bin_remove (GST_BIN (priv->gst_pipeline),
                            GST_ELEMENT (priv->gst_bin));
        }
        gst_object_unref (GST_OBJECT (priv->gst_pipeline));
        priv->gst_pipeline = NULL;
    }

    if (NULL != priv->gst_bin) {
        gst_object_unref (GST_OBJECT (priv->gst_bin));
        priv->gst_bin = NULL;
    }
}


static void
_g_digicam_manager_cleanup_sink (GDigicamManagerPrivate *priv,
                                 GError           **error)
{

    if (NULL != priv->gst_pipeline) {
        gst_element_set_state (priv->gst_pipeline, GST_STATE_NULL);
        /* FIXME: This blocks until the previous call has finished because
           it is async. We could remove this and wait for its completion
           notified by a message in the bus */
        gst_element_get_state (priv->gst_pipeline, NULL, NULL, GST_CLOCK_TIME_NONE);
    }

    if ((NULL != priv->descriptor) &&
        (priv->descriptor->supported_features &
         G_DIGICAM_CAPABILITIES_VIEWFINDER)) {
        gst_x_overlay_set_xwindow_id
            (GST_X_OVERLAY (priv->descriptor->viewfinder_sink),
             0);
    } else {
        if ((NULL != error) && (NULL == *error)) {
            g_digicam_set_error (error, G_DIGICAM_ERROR_VIEWFINDER_NOT_SUPPORTED,
                                 "imposible to add the viewfinder sink "
                                 "since the GStreamer bin has not this capability");
        }
    }
}


static void
_g_digicam_manager_free_private (GDigicamManagerPrivate *priv)
{
    _g_digicam_manager_cleanup_sink (priv, NULL);
    _g_digicam_manager_cleanup_bin (priv);
}


static gboolean
_picture_done (GObject *gst_element,
               const gchar *filename,
               gpointer user_data)
{
    gboolean result;
    GDigicamManagerPrivate *priv = NULL;
    GString *string = NULL;

    priv = G_DIGICAM_MANAGER_GET_PRIVATE (user_data);

    /* after the still picture has been taken, we need to remove
       the autofocus lock to allow a new one for the next picture */
    priv->locks = 0;

    string = g_string_new (filename);
    g_signal_emit (G_OBJECT (user_data), manager_signals [PICT_DONE_SIGNAL], 0,
		   string, &result);
    g_string_free (string, FALSE);

    return result;
}
