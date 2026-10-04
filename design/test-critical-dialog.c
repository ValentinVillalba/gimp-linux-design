/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "config.h"
#include <string.h>
#include <gtk/gtk.h>
#include "widgets/gimpcriticaldialog.h"
#include "gimp-version.h"

/* Isolate the widget from version-report dependencies; test its buffer handling. */
gchar *gimp_version (gboolean verbose, gboolean localized)
{
  return g_strdup ("local-test-version");
}

static void check_dialog (const gchar *last_version, gboolean fatal)
{
  GtkWidget *widget = gimp_critical_dialog_new ("Local diagnostic", last_version, 0);
  GimpCriticalDialog *dialog = GIMP_CRITICAL_DIALOG (widget);
  GtkTextBuffer *buffer;
  GtkTextIter start, end;
  gchar *text, *clipboard;
  GtkWidget *actions;
  GList *children;
  guint count = 0;

  g_object_ref_sink (widget);
  gimp_critical_dialog_add (widget, "test error", "test trace", fatal, NULL, 0);
  actions = gtk_dialog_get_action_area (GTK_DIALOG (widget));
  children = gtk_container_get_children (GTK_CONTAINER (actions));
  for (GList *item = children; item; item = item->next)
    {
      gint response = gtk_dialog_get_response_for_widget (GTK_DIALOG (widget), item->data);
      g_assert_true (response == 1 || response == 3 || response == GTK_RESPONSE_CLOSE);
      count++;
    }
  g_list_free (children);
  g_assert_cmpuint (count, ==, fatal ? 3 : 2);
  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (dialog->details));
  gtk_text_buffer_get_bounds (buffer, &start, &end);
  text = gtk_text_buffer_get_text (buffer, &start, &end, FALSE);
  g_assert_nonnull (strstr (text, "local-test-version"));
  g_assert_nonnull (strstr (text, "test error"));
  g_assert_nonnull (strstr (text, "test trace"));
  gtk_dialog_response (GTK_DIALOG (widget), 1);
  clipboard = gtk_clipboard_wait_for_text (gtk_clipboard_get (GDK_SELECTION_CLIPBOARD));
  g_assert_cmpstr (clipboard, ==, text);
  g_free (clipboard);
  g_free (text);
  gtk_dialog_response (GTK_DIALOG (widget), GTK_RESPONSE_CLOSE);
  g_object_unref (widget);
}

int main (int argc, char **argv)
{
  gtk_init (&argc, &argv);
  check_dialog (NULL, FALSE);
  check_dialog ("99.0", FALSE);
  check_dialog (NULL, TRUE);
  check_dialog ("99.0", TRUE);
  g_print ("Local critical dialog: four cases passed\n");
  return 0;
}
