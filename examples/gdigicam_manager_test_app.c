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

/* Includes */
#define _GNU_SOURCE
#include <stdlib.h>

#include <gtk/gtk.h>
#include <gst/gst.h>
#include <gst/gstbin.h>

/* #include <gdk/gdk.h> */
#include <gdk/gdkx.h>

#include "gdigicam-util.h"
#include "gdigicam-manager.h"
#include "gdigicam-debug.h"
#include <hildon/hildon-window.h>

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

static GtkWidget *mainwin = NULL;
static GtkWidget *hbox = NULL;
static GtkWidget *vbox = NULL;
static GtkWidget *event_box = NULL;
static GDigicamManager* digicam_manager = NULL;
static GstElement* camera_bin = NULL;

static gboolean _closing (GtkWidget *widget,
                          GdkEvent *event,
                          gpointer user_data)
{
    gtk_main_quit();

    G_DIGICAM_DEBUG ("GDigicam example: Closing...\n\n\n\n");

    return FALSE;
}

void _set_camerabin_button_clicked (GtkButton *button, gpointer data)
{
    GDigicamDescriptor *descriptor = NULL;

    /* FIXME: Create descriptor */
    /* In the meanwhile the #GDigicamManager will try to discover the
     * camera-bin features in its own */

    g_digicam_manager_set_gstreamer_bin (digicam_manager, camera_bin, descriptor, NULL);
}

void _unset_camerabin_button_clicked (GtkButton *button, gpointer data)
{
    /* As there is not unset method, this has no sense. */
/*   g_digicam_manager_unset_gstreamer_bin (digicam_manager, NULL); */
}

void _set_viewsink_button_clicked (GtkButton *button, gpointer data)
{
/*   g_digicam_manager_add_viewfinder_sink (digicam_manager, */
/*                                          GDK_WINDOW_XWINDOW (event_box->window), */
/*                                          NULL); */
}

void _unset_viewsink_button_clicked (GtkButton *button, gpointer data)
{
/*   g_digicam_manager_remove_viewfinder_sink (digicam_manager, NULL); */
}

int
main (int argc, char **argv)
{
    GtkWidget *set_camerabin_button = NULL;
    GtkWidget *unset_camerabin_button = NULL;
    GtkWidget *set_viewsink_button = NULL;
    GtkWidget *unset_viewsink_button = NULL;
/*     GdkColor black; */
    GstElement* src = NULL;
    GstElement* colorspace = NULL;
    GstElement* sink = NULL;
/*     GstPad *pad = NULL; */

/*     /\* Initialize threads environment. *\/ */
/*     g_thread_init (NULL); */
/*     gdk_threads_init (); */
/*     gdk_threads_enter (); */

    /* Initialize the GDigicam */
    g_digicam_init (&argc, &argv);

    /* Initialize i18n support */
    gtk_set_locale ();

    /* Initialize the GTK. */
    gtk_init(&argc, &argv);

    /* Create the main window */
#ifdef GDIGICAM_PLATFORM_MAEMO
    mainwin = hildon_window_new ();
#else
    mainwin = gtk_window_new (GTK_WINDOW_TOPLEVEL);
#endif

    /* Building UI widgets */
    vbox = gtk_vbox_new (FALSE, 2);
    hbox = gtk_hbox_new (TRUE, 4);
    event_box = gtk_event_box_new ();
    set_camerabin_button = gtk_button_new_with_label ("Set CameraBin");
    unset_camerabin_button = gtk_button_new_with_label ("Unset CameraBin");
    set_viewsink_button = gtk_button_new_with_label ("Set Viewsink");
    unset_viewsink_button = gtk_button_new_with_label ("Unset Viewsink");

    /* Setting eventbox background color to black */
/*     gdk_color_parse ("black", &black); */

/*     gtk_widget_modify_bg (GTK_WIDGET(vbox2), GTK_STATE_NORMAL, */
/*                           &black); */

/*     gtk_widget_modify_bg (GTK_WIDGET(event_box), GTK_STATE_NORMAL, */
/*                           &black); */

    /* Packing UI widgets */
    gtk_container_add (GTK_CONTAINER (mainwin), hbox);
    gtk_box_pack_start (GTK_BOX (hbox), event_box, TRUE, TRUE, 0);
    gtk_box_pack_start (GTK_BOX (hbox), vbox, FALSE, FALSE, 0);
    gtk_container_add (GTK_CONTAINER (vbox), set_camerabin_button);
    gtk_container_add (GTK_CONTAINER (vbox), unset_camerabin_button);
    gtk_container_add (GTK_CONTAINER (vbox), set_viewsink_button);
    gtk_container_add (GTK_CONTAINER (vbox), unset_viewsink_button);

    /* Setting the GDigicamManager instance */
    digicam_manager = g_digicam_manager_new ();

    /* Setting the camera bin */
    camera_bin = gst_bin_new (NULL);
    src = gst_element_factory_make ("videotestsrc", NULL);
    colorspace = gst_element_factory_make ("ffmpegcolorspace", NULL);
    sink = gst_element_factory_make (VIDEO_SINK, NULL);
    gst_bin_add_many (GST_BIN_CAST (camera_bin), src, colorspace, sink, NULL);
    gst_element_link_many (src, colorspace, sink, NULL);

/*     /\* add ghostpad *\/ */
/*     pad = gst_element_get_pad (sink, "sink"); */
/*     gst_element_add_pad (camera_bin, gst_ghost_pad_new ("sink", pad)); */
/*     gst_object_unref (GST_OBJECT (pad)); */

    /* Begin the main application */
    gtk_widget_show_all (mainwin);

    /* Connect signal to X in the upper corner */
    g_signal_connect (G_OBJECT (mainwin), "delete_event",
                      G_CALLBACK (_closing), NULL);

    g_signal_connect (G_OBJECT (set_camerabin_button), "clicked",
                      G_CALLBACK (_set_camerabin_button_clicked), NULL);

    g_signal_connect (G_OBJECT (unset_camerabin_button), "clicked",
                      G_CALLBACK (_unset_camerabin_button_clicked), NULL);

    g_signal_connect (G_OBJECT (set_viewsink_button), "clicked",
                      G_CALLBACK (_set_viewsink_button_clicked), NULL);

    g_signal_connect (G_OBJECT (unset_viewsink_button), "clicked",
                      G_CALLBACK (_unset_viewsink_button_clicked), NULL);

/*     g_signal_connect (G_OBJECT (video), "expose_event", */
/*                       G_CALLBACK (_video_widget_exposed), NULL); */

    gtk_main ();

    gdk_threads_leave ();

    /* Exit */
    return 0;
}
