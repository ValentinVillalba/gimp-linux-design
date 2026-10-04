/* GIMP - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * gimpcriticaldialog.c
 * Copyright (C) 2018  Jehan <jehan@gimp.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

/*
 * This widget is particular that I want to be able to use it
 * internally but also from an alternate tool (gimp-debug-tool). It
 * means that the implementation must stay as generic glib/GTK+ as
 * possible.
 */

#include "config.h"

#include <string.h>

#include <gtk/gtk.h>
#include <gegl.h>

#ifdef PLATFORM_OSX
#import <Cocoa/Cocoa.h>
#endif

#ifdef G_OS_WIN32
#undef DATADIR
#include <dwmapi.h>
#include <gdk/gdkwin32.h>
#include <windows.h>

#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE
#define DWMWA_USE_IMMERSIVE_DARK_MODE 20
#endif
#endif

#include "gimpcriticaldialog.h"

#include "gimp-intl.h"
#include "gimp-version.h"


#define GIMP_CRITICAL_RESPONSE_CLIPBOARD 1
#define GIMP_CRITICAL_RESPONSE_RESTART   3

#define BUTTON1_TEXT _("Copy Diagnostic Information")

enum
{
  PROP_0,
  PROP_LAST_VERSION,
  PROP_RELEASE_DATE
};

static void     gimp_critical_dialog_constructed  (GObject      *object);
static void     gimp_critical_dialog_finalize     (GObject      *object);
static void     gimp_critical_dialog_set_property (GObject      *object,
                                                   guint         property_id,
                                                   const GValue *value,
                                                   GParamSpec   *pspec);
static void     gimp_critical_dialog_response     (GtkDialog    *dialog,
                                                   gint          response_id);

static void     gimp_critical_dialog_copy_info    (GimpCriticalDialog *dialog);
#if defined(G_OS_WIN32) || (defined(PLATFORM_OSX) && MAC_OS_X_VERSION_MIN_REQUIRED >= 101400)
static void     gimp_critical_dialog_realize      (GtkWidget          *widget,
                                                   GimpCriticalDialog *dialog);
#endif


G_DEFINE_TYPE (GimpCriticalDialog, gimp_critical_dialog, GTK_TYPE_DIALOG)

#define parent_class gimp_critical_dialog_parent_class


static void
gimp_critical_dialog_class_init (GimpCriticalDialogClass *klass)
{
  GObjectClass   *object_class = G_OBJECT_CLASS (klass);
  GtkDialogClass *dialog_class = GTK_DIALOG_CLASS (klass);

  object_class->constructed  = gimp_critical_dialog_constructed;
  object_class->finalize     = gimp_critical_dialog_finalize;
  object_class->set_property = gimp_critical_dialog_set_property;

  dialog_class->response = gimp_critical_dialog_response;

  g_object_class_install_property (object_class, PROP_LAST_VERSION,
                                   g_param_spec_string ("last-version",
                                                        NULL, NULL, NULL,
                                                        G_PARAM_WRITABLE |
                                                        G_PARAM_CONSTRUCT_ONLY));
  g_object_class_install_property (object_class, PROP_RELEASE_DATE,
                                   g_param_spec_string ("release-date",
                                                        NULL, NULL, NULL,
                                                        G_PARAM_WRITABLE |
                                                        G_PARAM_CONSTRUCT_ONLY));
}

static void
gimp_critical_dialog_init (GimpCriticalDialog *dialog)
{
  GtkWidget      *container;
  PangoAttrList  *attrs;
  PangoAttribute *attr;

  gtk_window_set_role (GTK_WINDOW (dialog), "gimp-critical");

  gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_CLOSE);
  gtk_window_set_resizable (GTK_WINDOW (dialog), TRUE);
  gtk_window_set_position (GTK_WINDOW (dialog), GTK_WIN_POS_CENTER);
  gtk_window_set_titlebar (GTK_WINDOW (dialog), NULL);

  dialog->main_vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 6);
  container         = gtk_dialog_get_content_area (GTK_DIALOG (dialog));
  gtk_container_set_border_width (GTK_CONTAINER (container), 12);
  gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (dialog))),
                      dialog->main_vbox, TRUE, TRUE, 0);
  gtk_widget_set_visible (dialog->main_vbox, TRUE);

  /* The error label. */
  dialog->top_label = gtk_label_new (NULL);
  gtk_widget_set_halign (dialog->top_label, GTK_ALIGN_START);
  gtk_label_set_ellipsize (GTK_LABEL (dialog->top_label), PANGO_ELLIPSIZE_END);
  gtk_label_set_selectable (GTK_LABEL (dialog->top_label), TRUE);
  gtk_box_pack_start (GTK_BOX (dialog->main_vbox), dialog->top_label,
                      FALSE, FALSE, 0);

  attrs = pango_attr_list_new ();
  attr  = pango_attr_weight_new (PANGO_WEIGHT_SEMIBOLD);
  pango_attr_list_insert (attrs, attr);
  gtk_label_set_attributes (GTK_LABEL (dialog->top_label), attrs);
  pango_attr_list_unref (attrs);

  gtk_widget_set_visible (dialog->top_label, TRUE);

  dialog->center_label = gtk_label_new (NULL);

  gtk_widget_set_halign (dialog->center_label, GTK_ALIGN_START);
  gtk_label_set_selectable (GTK_LABEL (dialog->center_label), TRUE);
  gtk_box_pack_start (GTK_BOX (dialog->main_vbox), dialog->center_label,
                      FALSE, FALSE, 0);
  gtk_widget_set_visible (dialog->center_label, TRUE);

  dialog->bottom_label = gtk_label_new (NULL);
  gtk_widget_set_halign (dialog->bottom_label, GTK_ALIGN_START);
  gtk_box_pack_start (GTK_BOX (dialog->main_vbox), dialog->bottom_label, FALSE, FALSE, 0);

  attrs = pango_attr_list_new ();
  attr  = pango_attr_style_new (PANGO_STYLE_ITALIC);
  pango_attr_list_insert (attrs, attr);
  gtk_label_set_attributes (GTK_LABEL (dialog->bottom_label), attrs);
  pango_attr_list_unref (attrs);
  gtk_widget_set_visible (dialog->bottom_label, TRUE);

  dialog->pid      = 0;
  dialog->program  = NULL;
}

static void
gimp_critical_dialog_constructed (GObject *object)
{
  GimpCriticalDialog *dialog = GIMP_CRITICAL_DIALOG (object);
  GtkWidget          *scrolled;
  GtkTextBuffer      *buffer;
  gchar              *version;
  gchar              *text;

  gtk_window_set_icon_name (GTK_WINDOW (dialog), "dialog-error");

  /* Bug details for developers. */
  scrolled = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolled),
                                       GTK_SHADOW_IN);
  gtk_widget_set_size_request (scrolled, -1, 200);

  gtk_box_pack_start (GTK_BOX (dialog->main_vbox), scrolled, TRUE, TRUE, 6);
  gtk_widget_set_visible (scrolled, TRUE);
  gtk_dialog_add_buttons (GTK_DIALOG (dialog),
                          BUTTON1_TEXT, GIMP_CRITICAL_RESPONSE_CLIPBOARD,
                          _("_Close"), GTK_RESPONSE_CLOSE,
                          NULL);
  gtk_label_set_text (GTK_LABEL (dialog->center_label),
                      _("This error may have left GIMP in an inconsistent state. "
                        "Save your work to a separate file and restart GIMP."));
  gtk_label_set_text (GTK_LABEL (dialog->bottom_label),
                      _("Copy the diagnostic information to keep a local record "
                        "of the error and the steps that caused it."));

  buffer = gtk_text_buffer_new (NULL);
  version = gimp_version (TRUE, FALSE);
  text = g_strdup_printf ("<!-- %s -->\n\n\n```\n%s\n```",
                          _("Copy-paste this whole debug data to report to developers"),
                          version);
  gtk_text_buffer_set_text (buffer, text, -1);
  g_free (version);
  g_free (text);

  dialog->details = gtk_text_view_new_with_buffer (buffer);
  g_object_unref (buffer);
  gtk_text_view_set_editable (GTK_TEXT_VIEW (dialog->details), FALSE);
  gtk_widget_set_visible (dialog->details, TRUE);
  gtk_container_add (GTK_CONTAINER (scrolled), dialog->details);
}

/* Copied from app/widgets/gimpwidgets-utils.c, to reduce dependency
 * on internal GIMP procedures */
#if defined(G_OS_WIN32) || (defined(PLATFORM_OSX) && MAC_OS_X_VERSION_MIN_REQUIRED >= 101400)
static void
gimp_critical_dialog_realize (GtkWidget          *widget,
                              GimpCriticalDialog *dialog)
{
#ifdef G_OS_WIN32
  HWND             hwnd;
  GdkWindow       *window        = NULL;
#endif
  GtkStyleContext *style;
  GdkRGBA         *color         = NULL;
  gboolean         use_dark_mode = FALSE;

  /* Workaround if we don't have access to GimpGuiConfig.
   * If the background color is below the threshold, then we're
   * likely in dark mode.
   */
  style = gtk_widget_get_style_context (widget);
  gtk_style_context_get (style, gtk_style_context_get_state (style),
                         GTK_STYLE_PROPERTY_BACKGROUND_COLOR, &color,
                         NULL);
  if (color)
    {
      if (color->red < 0.5 && color->green < 0.5 && color->blue < 0.5)
        use_dark_mode = TRUE;
      gdk_rgba_free (color);
    }
#ifdef G_OS_WIN32
  window = gtk_widget_get_window (GTK_WIDGET (widget));
  if (window)
    {
      hwnd = (HWND) gdk_win32_window_get_handle (window);
      DwmSetWindowAttribute (hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE,
                              &use_dark_mode, sizeof (use_dark_mode));
    }
#elif defined(PLATFORM_OSX)
  if (use_dark_mode)
    [NSApp setAppearance:[NSAppearance appearanceNamed:NSAppearanceNameDarkAqua]];
  else
    [NSApp setAppearance:[NSAppearance appearanceNamed:NSAppearanceNameAqua]];
#endif
}
#endif


static void
gimp_critical_dialog_finalize (GObject *object)
{
  GimpCriticalDialog *dialog = GIMP_CRITICAL_DIALOG (object);

  if (dialog->program)
    g_free (dialog->program);
  if (dialog->last_version)
    g_free (dialog->last_version);
  if (dialog->release_date)
    g_free (dialog->release_date);

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
gimp_critical_dialog_set_property (GObject      *object,
                                   guint         property_id,
                                   const GValue *value,
                                   GParamSpec   *pspec)
{
  GimpCriticalDialog *dialog = GIMP_CRITICAL_DIALOG (object);

  switch (property_id)
    {
    case PROP_LAST_VERSION:
      dialog->last_version = g_value_dup_string (value);
      break;
    case PROP_RELEASE_DATE:
      dialog->release_date = g_value_dup_string (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
gimp_critical_dialog_copy_info (GimpCriticalDialog *dialog)
{
  GtkClipboard *clipboard;

  clipboard = gtk_clipboard_get_for_display (gdk_display_get_default (),
                                             GDK_SELECTION_CLIPBOARD);
  if (clipboard)
    {
      GtkTextBuffer *buffer;
      gchar         *text;
      GtkTextIter    start;
      GtkTextIter    end;

      buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (dialog->details));
      gtk_text_buffer_get_iter_at_offset (buffer, &start, 0);
      gtk_text_buffer_get_iter_at_offset (buffer, &end, -1);
      text = gtk_text_buffer_get_text (buffer, &start, &end, FALSE);
      gtk_clipboard_set_text (clipboard, text, -1);
      g_free (text);
    }
}

static void
gimp_critical_dialog_response (GtkDialog *dialog,
                               gint       response_id)
{
  GimpCriticalDialog *critical = GIMP_CRITICAL_DIALOG (dialog);

  switch (response_id)
    {
    case GIMP_CRITICAL_RESPONSE_CLIPBOARD:
      gimp_critical_dialog_copy_info (critical);
      break;

    case GIMP_CRITICAL_RESPONSE_RESTART:
      {
        gchar *args[2] = { critical->program , NULL };

#ifndef G_OS_WIN32
        /* It is unneeded to kill the process on Win32. This was run
         * as an async call and the main process should already be
         * dead by now.
         */
        if (critical->pid > 0)
          kill ((pid_t ) critical->pid, SIGINT);
#endif
        if (critical->program)
          g_spawn_async (NULL, args, NULL, G_SPAWN_DEFAULT,
                         NULL, NULL, NULL, NULL);
      }
      /* Fall through. */
    case GTK_RESPONSE_DELETE_EVENT:
    case GTK_RESPONSE_CLOSE:
    default:
      gtk_widget_destroy (GTK_WIDGET (dialog));
      break;
    }
}

/*  public functions  */

GtkWidget *
gimp_critical_dialog_new (const gchar *title,
                          const gchar *last_version,
                          gint64       release_timestamp)
{
  GtkWidget *dialog;
  gchar     *date = NULL;

  g_return_val_if_fail (title != NULL, NULL);

  if (release_timestamp > 0)
    {
      GDateTime *datetime;

      datetime = g_date_time_new_from_unix_local (release_timestamp);
      date = g_date_time_format (datetime, "%x");
      g_date_time_unref (datetime);
    }

  dialog = g_object_new (GIMP_TYPE_CRITICAL_DIALOG,
                         "title",        title,
                         "last-version", last_version,
                         "release-date", date,
                         NULL);
  g_free (date);

#if defined(G_OS_WIN32) || (defined(PLATFORM_OSX) && MAC_OS_X_VERSION_MIN_REQUIRED >= 101400)
  g_signal_connect_object (dialog, "realize",
                           G_CALLBACK (gimp_critical_dialog_realize),
                           NULL, 0);
#endif

  return dialog;
}

void
gimp_critical_dialog_add (GtkWidget   *dialog,
                          const gchar *message,
                          const gchar *trace,
                          gboolean     is_fatal,
                          const gchar *program,
                          gint         pid)
{
  GimpCriticalDialog *critical;
  GtkTextBuffer      *buffer;
  GtkTextIter         end;
  gchar              *text;

  if (! GIMP_IS_CRITICAL_DIALOG (dialog) || ! message)
    {
      /* This is a bit hackish. We usually should use
       * g_return_if_fail(). But I don't want to end up in a critical
       * recursing loop if our code had bugs. We would crash GIMP with
       * a CRITICAL which would otherwise not have necessarily ended up
       * in a crash.
       */
      return;
    }
  critical = GIMP_CRITICAL_DIALOG (dialog);

  /* The user text, which should be localized. */
  if (is_fatal)
    {
      text = g_strdup_printf (_("GIMP crashed with a fatal error: %s"),
                              message);
    }
  else if (! gtk_label_get_text (GTK_LABEL (critical->top_label)) ||
           strlen (gtk_label_get_text (GTK_LABEL (critical->top_label))) == 0)
    {
      /* First error. Let's just display it. */
      text = g_strdup_printf (_("GIMP encountered an error: %s"),
                              message);
    }
  else
    {
      /* Let's not display all errors. They will be in the bug report
       * part anyway.
       */
      text = g_strdup_printf (_("GIMP encountered several critical errors!"));
    }
  gtk_label_set_text (GTK_LABEL (critical->top_label),
                      text);
  g_free (text);

  if (is_fatal)
    gtk_label_set_text (GTK_LABEL (critical->center_label),
                        _("Copy the diagnostic information before restarting GIMP. "
                          "Unsaved work may be available through recovery on restart."));

  /* The details text is untranslated on purpose. This is the message
   * meant to go to clipboard for the bug report. It has to be in
   * English.
   */
  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (critical->details));
  gtk_text_buffer_get_iter_at_offset (buffer, &end, -1);
  if (trace)
    text = g_strdup_printf ("\n> %s\n\nStack trace:\n```\n%s\n```", message, trace);
  else
    text = g_strdup_printf ("\n> %s\n", message);
  gtk_text_buffer_insert (buffer, &end, text, -1);
  g_free (text);

  /* Finally when encountering a fatal message, propose one more button
   * to restart GIMP.
   */
  if (is_fatal)
    {
      gtk_dialog_add_buttons (GTK_DIALOG (dialog),
                              _("_Restart GIMP"), GIMP_CRITICAL_RESPONSE_RESTART,
                              NULL);
      critical->program = g_strdup (program);
      critical->pid     = pid;
    }
}
