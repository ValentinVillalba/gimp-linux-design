/* GIMP - The GNU Image Manipulation Program
 * Copyright (C) 1995-1997 Spencer Kimball and Peter Mattis
 *
 * file-remote.c
 * Copyright (C) 2014  Michael Natterer <mitch@gimp.org>
 *
 * Based on: URI plug-in, GIO/GVfs backend
 * Copyright (C) 2008  Sven Neumann <sven@gimp.org>
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

#include "config.h"

#include <gegl.h>
#include <gio/gio.h>
#include "libgimpbase/gimpbase.h"
#include "core/core-types.h"
#include "file-remote.h"
#include "gimp-intl.h"

/* Remote transport is deliberately absent in this native-path-only fork.
 * Retain the internal entry points so stale callers receive a clear error.
 */
static void
file_remote_disabled (GError **error)
{
  g_set_error_literal (error, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED,
                       _("Remote file transfer is disabled in this editor."));
}

gboolean
file_remote_mount_file (Gimp *gimp, GFile *file,
                        GimpProgress *progress, GError **error)
{
  file_remote_disabled (error);
  return FALSE;
}

GFile *
file_remote_download_image (Gimp *gimp, GFile *file,
                            GimpProgress *progress, GError **error)
{
  file_remote_disabled (error);
  return NULL;
}

GFile *
file_remote_upload_image_prepare (Gimp *gimp, GFile *file,
                                  GimpProgress *progress, GError **error)
{
  file_remote_disabled (error);
  return NULL;
}

gboolean
file_remote_upload_image_finish (Gimp *gimp, GFile *file, GFile *local_file,
                                 GimpProgress *progress, GError **error)
{
  file_remote_disabled (error);
  return FALSE;
}
