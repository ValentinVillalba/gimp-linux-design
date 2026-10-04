/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "config.h"
#include <gio/gio.h>
#include <glib/gstdio.h>
#include "gimphelp.h"

int main (void)
{
  GimpHelpLocale *locale = gimp_help_locale_new ("en");
  GError *error = NULL;
  const gchar *remote[] = {"https://example.invalid/help.xml", "sftp://example.invalid/help.xml"};
  for (guint i = 0; i < G_N_ELEMENTS (remote); i++)
    {
      g_assert_false (gimp_help_locale_parse (locale, remote[i], GIMP_HELP_DEFAULT_DOMAIN, NULL, &error));
      g_assert_error (error, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED);
      g_clear_error (&error);
    }
  gchar *directory = g_dir_make_tmp ("gimp-local-help-XXXXXX", &error);
  g_assert_no_error (error);
  gchar *filename = g_build_filename (directory, "help index.xml", NULL);
  g_assert_true (g_file_set_contents (filename,
    "<gimp-help><help-item id=\"gimp-main\" ref=\"index.html\" title=\"Local manual\"/></gimp-help>", -1, &error));
  g_assert_no_error (error);
  gchar *uri = g_filename_to_uri (filename, NULL, &error);
  g_assert_no_error (error);
  g_assert_true (gimp_help_locale_parse (locale, uri, GIMP_HELP_DEFAULT_DOMAIN, NULL, &error));
  g_assert_no_error (error);
  g_assert_cmpstr (gimp_help_locale_map (locale, "gimp-main"), ==, "index.html");
  gimp_help_locale_free (locale);
  g_remove (filename);
  g_rmdir (directory);
  g_free (uri);
  g_free (filename);
  g_free (directory);
  g_print ("Local help: remote indexes rejected and native index parsed\n");
  return 0;
}
