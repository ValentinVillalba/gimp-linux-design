/* GIMP - The GNU Image Manipulation Program
 * Copyright (C) 2009 Martin Nordholts
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

#include <string.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef _WIN32
#define STRICT
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <io.h>
#define close _close
#endif

#include <gegl.h>
#include <gtk/gtk.h>

#include "libgimpbase/gimpbase.h"

#include "widgets/widgets-types.h"

#include "widgets/gimpuimanager.h"

#include "operations/gimp-operation-config.h"
#include "operations/gimplevelsconfig.h"

#include "core/gimp.h"
#include "core/gimpchannel.h"
#include "core/gimpchannel-select.h"
#include "core/gimpdrawable.h"
#include "core/gimpdrawable-filters.h"
#include "core/gimpdrawablefilter.h"
#include "core/gimpcontainer.h"
#include "core/gimpfilter.h"
#include "core/gimppickable.h"
#include "core/gimpprojection.h"
#include "core/gimpgrid.h"
#include "core/gimpgrouplayer.h"
#include "core/gimpguide.h"
#include "core/gimpimage.h"
#include "core/gimpimage-undo.h"
#include "core/gimpimage-grid.h"
#include "core/gimpimage-guides.h"
#include "core/gimpimage-sample-points.h"
#include "core/gimplayer.h"
#include "core/gimplayermask.h"
#include "core/gimplayer-new.h"
#include "core/gimpsamplepoint.h"
#include "core/gimpselection.h"

#include "path/gimpanchor.h"
#include "path/gimpbezierstroke.h"
#include "path/gimppath.h"

#include "plug-in/gimppluginmanager-file.h"

#include "file/file-open.h"
#include "file/file-save.h"

#include "tests.h"

#include "gimp-app-test-utils.h"


/* we continue to use LEGACY layers for testing, so we can use the
 * same test image for all tests, including loading
 * files/gimp-2-6-file.xcf which can't have any non-LEGACY modes
 */

#define GIMP_MAINIMAGE_WIDTH            100
#define GIMP_MAINIMAGE_HEIGHT           90
#define GIMP_MAINIMAGE_TYPE             GIMP_RGB
#define GIMP_MAINIMAGE_PRECISION        GIMP_PRECISION_U8_NON_LINEAR

#define GIMP_MAINIMAGE_LAYER1_NAME      "layer1"
#define GIMP_MAINIMAGE_LAYER1_WIDTH     50
#define GIMP_MAINIMAGE_LAYER1_HEIGHT    51
#define GIMP_MAINIMAGE_LAYER1_FORMAT    babl_format ("R'G'B'A u8")
#define GIMP_MAINIMAGE_LAYER1_OPACITY   GIMP_OPACITY_OPAQUE
#define GIMP_MAINIMAGE_LAYER1_MODE      GIMP_LAYER_MODE_NORMAL_LEGACY

#define GIMP_MAINIMAGE_LAYER2_NAME      "layer2"
#define GIMP_MAINIMAGE_LAYER2_WIDTH     25
#define GIMP_MAINIMAGE_LAYER2_HEIGHT    251
#define GIMP_MAINIMAGE_LAYER2_FORMAT    babl_format ("R'G'B' u8")
#define GIMP_MAINIMAGE_LAYER2_OPACITY   GIMP_OPACITY_TRANSPARENT
#define GIMP_MAINIMAGE_LAYER2_MODE      GIMP_LAYER_MODE_MULTIPLY_LEGACY

#define GIMP_MAINIMAGE_GROUP1_NAME      "group1"

#define GIMP_MAINIMAGE_LAYER3_NAME      "layer3"

#define GIMP_MAINIMAGE_LAYER4_NAME      "layer4"

#define GIMP_MAINIMAGE_GROUP2_NAME      "group2"

#define GIMP_MAINIMAGE_LAYER5_NAME      "layer5"

#define GIMP_MAINIMAGE_VGUIDE1_POS      42
#define GIMP_MAINIMAGE_VGUIDE2_POS      82
#define GIMP_MAINIMAGE_HGUIDE1_POS      3
#define GIMP_MAINIMAGE_HGUIDE2_POS      4

#define GIMP_MAINIMAGE_SAMPLEPOINT1_X   10
#define GIMP_MAINIMAGE_SAMPLEPOINT1_Y   12
#define GIMP_MAINIMAGE_SAMPLEPOINT2_X   41
#define GIMP_MAINIMAGE_SAMPLEPOINT2_Y   49

#define GIMP_MAINIMAGE_RESOLUTIONX      400
#define GIMP_MAINIMAGE_RESOLUTIONY      410

#define GIMP_MAINIMAGE_PARASITE_NAME    "test-parasite"
#define GIMP_MAINIMAGE_PARASITE_DATA    "foo"
#define GIMP_MAINIMAGE_PARASITE_SIZE    4                /* 'f' 'o' 'o' '\0' */

#define GIMP_MAINIMAGE_COMMENT          "Created with code from "\
                                        "app/tests/test-xcf.c in the GIMP "\
                                        "source tree, i.e. it was not created "\
                                        "manually and may thus look weird if "\
                                        "opened and inspected in GIMP."

#define GIMP_MAINIMAGE_UNIT             gimp_unit_pica ()

#define GIMP_MAINIMAGE_GRIDXSPACING     25.0
#define GIMP_MAINIMAGE_GRIDYSPACING     27.0

#define GIMP_MAINIMAGE_CHANNEL1_NAME    "channel1"
#define GIMP_MAINIMAGE_CHANNEL1_WIDTH   GIMP_MAINIMAGE_WIDTH
#define GIMP_MAINIMAGE_CHANNEL1_HEIGHT  GIMP_MAINIMAGE_HEIGHT
#define GIMP_MAINIMAGE_CHANNEL1_COLOR   { 1.0, 0.0, 1.0, 1.0 }

#define GIMP_MAINIMAGE_SELECTION_X      5
#define GIMP_MAINIMAGE_SELECTION_Y      6
#define GIMP_MAINIMAGE_SELECTION_W      7
#define GIMP_MAINIMAGE_SELECTION_H      8

#define GIMP_MAINIMAGE_PATH1_NAME       "vectors1"
#define GIMP_MAINIMAGE_PATH1_COORDS     { { 11.0, 12.0, /* pad zeroes */ },\
                                          { 21.0, 22.0, /* pad zeroes */ },\
                                          { 31.0, 32.0, /* pad zeroes */ }, }

#define GIMP_MAINIMAGE_PATH2_NAME       "vectors2"
#define GIMP_MAINIMAGE_PATH2_COORDS     { { 911.0, 912.0, /* pad zeroes */ },\
                                          { 921.0, 922.0, /* pad zeroes */ },\
                                          { 931.0, 932.0, /* pad zeroes */ }, }

#define ADD_TEST(function) \
  g_test_add_data_func ("/gimp-xcf/" #function, gimp, function);


GimpImage        * gimp_test_load_image                        (Gimp            *gimp,
                                                                GFile           *file);
static void        gimp_write_and_read_file                    (Gimp            *gimp,
                                                                gboolean         with_unusual_stuff,
                                                                gboolean         compat_paths,
                                                                gboolean         use_gimp_2_8_features);
static GimpImage * gimp_create_mainimage                       (Gimp            *gimp,
                                                                gboolean         with_unusual_stuff,
                                                                gboolean         compat_paths,
                                                                gboolean         use_gimp_2_8_features);
static void        gimp_assert_mainimage                       (GimpImage       *image,
                                                                gboolean         with_unusual_stuff,
                                                                gboolean         compat_paths,
                                                                gboolean         use_gimp_2_8_features);


/**
 * write_and_read_gimp_2_6_format:
 * @data:
 *
 * Do a write and read test on a file that could as well be
 * constructed with GIMP 2.6.
 **/
static void
write_and_read_gimp_2_6_format (gconstpointer data)
{
  Gimp *gimp = GIMP (data);

  gimp_write_and_read_file (gimp,
                            FALSE /*with_unusual_stuff*/,
                            FALSE /*compat_paths*/,
                            FALSE /*use_gimp_2_8_features*/);
}

/**
 * write_and_read_gimp_2_6_format_unusual:
 * @data:
 *
 * Do a write and read test on a file that could as well be
 * constructed with GIMP 2.6, and make it unusual, like compatible
 * paths and with a floating selection.
 **/
static void
write_and_read_gimp_2_6_format_unusual (gconstpointer data)
{
  Gimp *gimp = GIMP (data);

  gimp_write_and_read_file (gimp,
                            TRUE /*with_unusual_stuff*/,
                            TRUE /*compat_paths*/,
                            FALSE /*use_gimp_2_8_features*/);
}

/**
 * load_gimp_2_6_file:
 * @data:
 *
 * Loads a file created with GIMP 2.6 and makes sure it loaded as
 * expected.
 **/
static void
load_gimp_2_6_file (gconstpointer data)
{
  Gimp      *gimp = GIMP (data);
  GimpImage *image;
  gchar     *filename;
  GFile     *file;

  filename = g_build_filename (g_getenv ("GIMP_TESTING_ABS_TOP_SRCDIR"),
                               "app/tests/files/gimp-2-6-file.xcf",
                               NULL);
  file = g_file_new_for_path (filename);
  g_free (filename);

  image = gimp_test_load_image (gimp, file);

  /* The image file was constructed by running
   * gimp_write_and_read_file (FALSE, FALSE) in GIMP 2.6 by
   * copy-pasting the code to GIMP 2.6 and adapting it to changes in
   * the core API, so we can use gimp_assert_mainimage() to make sure
   * the file was loaded successfully.
   */
  gimp_assert_mainimage (image,
                         FALSE /*with_unusual_stuff*/,
                         FALSE /*compat_paths*/,
                         FALSE /*use_gimp_2_8_features*/);
}

/**
 * write_and_read_gimp_2_8_format:
 * @data:
 *
 * Writes an XCF file that uses GIMP 2.8 features such as layer
 * groups, then reads the file and make sure no relevant information
 * was lost.
 **/
static void
write_and_read_gimp_2_8_format (gconstpointer data)
{
  Gimp *gimp = GIMP (data);

  gimp_write_and_read_file (gimp,
                            FALSE /*with_unusual_stuff*/,
                            FALSE /*compat_paths*/,
                            TRUE /*use_gimp_2_8_features*/);
}

GimpImage *
gimp_test_load_image (Gimp  *gimp,
                      GFile *file)
{
  GimpPlugInProcedure *proc;
  GimpImage           *image;
  GimpPDBStatusType    unused;

  proc = gimp_plug_in_manager_file_procedure_find (gimp->plug_in_manager,
                                                   GIMP_FILE_PROCEDURE_GROUP_OPEN,
                                                   file,
                                                   NULL /*error*/);
  image = file_open_image (gimp,
                           gimp_get_user_context (gimp),
                           NULL /*progress*/,
                           file,
                           0, 0, /* vector width, height */
                           TRUE, /* vector keep ratio */
                           FALSE /*as_new*/,
                           proc,
                           GIMP_RUN_NONINTERACTIVE,
                           NULL, /* file_proc_handles_vector */
                           &unused /*status*/,
                           NULL /*mime_type*/,
                           NULL /*error*/);

  return image;
}

/**
 * gimp_write_and_read_file:
 *
 * Constructs the main test image and asserts its state, writes it to
 * a file, reads the image from the file, and asserts the state of the
 * loaded file. The function takes various parameters so the same
 * function can be used for different formats.
 **/
static void
gimp_write_and_read_file (Gimp     *gimp,
                          gboolean  with_unusual_stuff,
                          gboolean  compat_paths,
                          gboolean  use_gimp_2_8_features)
{
  GimpImage           *image;
  GimpImage           *loaded_image;
  GimpPlugInProcedure *proc;
  gchar               *filename = NULL;
  gint                 file_handle;
  GFile               *file;

  /* Create the image */
  image = gimp_create_mainimage (gimp,
                                 with_unusual_stuff,
                                 compat_paths,
                                 use_gimp_2_8_features);

  /* Assert valid state */
  gimp_assert_mainimage (image,
                         with_unusual_stuff,
                         compat_paths,
                         use_gimp_2_8_features);

  /* Write to file */
  file_handle = g_file_open_tmp ("gimp-test-XXXXXX.xcf", &filename, NULL);
  g_assert_true (file_handle != -1);
  close (file_handle);
  file = g_file_new_for_path (filename);
  g_free (filename);

  proc = gimp_plug_in_manager_file_procedure_find (image->gimp->plug_in_manager,
                                                   GIMP_FILE_PROCEDURE_GROUP_SAVE,
                                                   file,
                                                   NULL /*error*/);
  file_save (gimp,
             image,
             NULL /*progress*/,
             file,
             proc,
             GIMP_RUN_NONINTERACTIVE,
             FALSE /*change_saved_state*/,
             FALSE /*export_backward*/,
             FALSE /*export_forward*/,
             NULL /*error*/);

  /* Load from file */
  loaded_image = gimp_test_load_image (image->gimp, file);

  /* Assert on the loaded file. If success, it means that there is no
   * significant information loss when we wrote the image to a file
   * and loaded it again
   */
  gimp_assert_mainimage (loaded_image,
                         with_unusual_stuff,
                         compat_paths,
                         use_gimp_2_8_features);

  g_file_delete (file, NULL, NULL);
  g_object_unref (file);
}

/**
 * gimp_create_mainimage:
 *
 * Creates the main test image, i.e. the image that we use for most of
 * our XCF testing purposes.
 *
 * Returns: The #GimpImage
 **/
static GimpImage *
gimp_create_mainimage (Gimp     *gimp,
                       gboolean  with_unusual_stuff,
                       gboolean  compat_paths,
                       gboolean  use_gimp_2_8_features)
{
  GimpImage     *image          = NULL;
  GimpLayer     *layer          = NULL;
  GimpParasite  *parasite       = NULL;
  GimpGrid      *grid           = NULL;
  GimpChannel   *channel        = NULL;
  GeglColor     *channel_color  = gegl_color_new (NULL);
  GimpChannel   *selection      = NULL;
  GimpPath      *path           = NULL;
  GimpCoords     path1_coords[] = GIMP_MAINIMAGE_PATH1_COORDS;
  GimpCoords     path2_coords[] = GIMP_MAINIMAGE_PATH2_COORDS;
  GimpStroke    *stroke         = NULL;
  GimpLayerMask *layer_mask     = NULL;
  gdouble        rgb[4]         = GIMP_MAINIMAGE_CHANNEL1_COLOR;

  gegl_color_set_pixel (channel_color, babl_format ("R'G'B'A double"), &rgb);
  /* Image size and type */
  image = gimp_image_new (gimp,
                          GIMP_MAINIMAGE_WIDTH,
                          GIMP_MAINIMAGE_HEIGHT,
                          GIMP_MAINIMAGE_TYPE,
                          GIMP_MAINIMAGE_PRECISION);

  /* Layers */
  layer = gimp_layer_new (image,
                          GIMP_MAINIMAGE_LAYER1_WIDTH,
                          GIMP_MAINIMAGE_LAYER1_HEIGHT,
                          GIMP_MAINIMAGE_LAYER1_FORMAT,
                          GIMP_MAINIMAGE_LAYER1_NAME,
                          GIMP_MAINIMAGE_LAYER1_OPACITY,
                          GIMP_MAINIMAGE_LAYER1_MODE);
  gimp_image_add_layer (image,
                        layer,
                        NULL,
                        0,
                        FALSE/*push_undo*/);
  layer = gimp_layer_new (image,
                          GIMP_MAINIMAGE_LAYER2_WIDTH,
                          GIMP_MAINIMAGE_LAYER2_HEIGHT,
                          GIMP_MAINIMAGE_LAYER2_FORMAT,
                          GIMP_MAINIMAGE_LAYER2_NAME,
                          GIMP_MAINIMAGE_LAYER2_OPACITY,
                          GIMP_MAINIMAGE_LAYER2_MODE);
  gimp_image_add_layer (image,
                        layer,
                        NULL,
                        0,
                        FALSE /*push_undo*/);

  /* Layer mask */
  layer_mask = gimp_layer_create_mask (layer,
                                       GIMP_ADD_MASK_BLACK,
                                       NULL /*channel*/);
  gimp_layer_add_mask (layer,
                       layer_mask,
                       TRUE  /* edit_mask */,
                       FALSE /* push_undo */,
                       NULL  /* error     */);

  /* Image compression type
   *
   * We don't do any explicit test, only implicit when we read tile
   * data in other tests
   */

  /* Guides, note we add them in reversed order */
  gimp_image_add_hguide (image,
                         GIMP_MAINIMAGE_HGUIDE2_POS,
                         FALSE /*push_undo*/);
  gimp_image_add_hguide (image,
                         GIMP_MAINIMAGE_HGUIDE1_POS,
                         FALSE /*push_undo*/);
  gimp_image_add_vguide (image,
                         GIMP_MAINIMAGE_VGUIDE2_POS,
                         FALSE /*push_undo*/);
  gimp_image_add_vguide (image,
                         GIMP_MAINIMAGE_VGUIDE1_POS,
                         FALSE /*push_undo*/);


  /* Sample points */
  gimp_image_add_sample_point_at_pos (image,
                                      GIMP_MAINIMAGE_SAMPLEPOINT1_X,
                                      GIMP_MAINIMAGE_SAMPLEPOINT1_Y,
                                      FALSE /*push_undo*/);
  gimp_image_add_sample_point_at_pos (image,
                                      GIMP_MAINIMAGE_SAMPLEPOINT2_X,
                                      GIMP_MAINIMAGE_SAMPLEPOINT2_Y,
                                      FALSE /*push_undo*/);

  /* Tattoo
   * We don't bother testing this, not yet at least
   */

  /* Resolution */
  gimp_image_set_resolution (image,
                             GIMP_MAINIMAGE_RESOLUTIONX,
                             GIMP_MAINIMAGE_RESOLUTIONY);


  /* Parasites */
  parasite = gimp_parasite_new (GIMP_MAINIMAGE_PARASITE_NAME,
                                GIMP_PARASITE_PERSISTENT,
                                GIMP_MAINIMAGE_PARASITE_SIZE,
                                GIMP_MAINIMAGE_PARASITE_DATA);
  gimp_image_parasite_attach (image,
                              parasite, FALSE);
  gimp_parasite_free (parasite);
  parasite = gimp_parasite_new ("gimp-comment",
                                GIMP_PARASITE_PERSISTENT,
                                strlen (GIMP_MAINIMAGE_COMMENT) + 1,
                                GIMP_MAINIMAGE_COMMENT);
  gimp_image_parasite_attach (image, parasite, FALSE);
  gimp_parasite_free (parasite);


  /* Unit */
  gimp_image_set_unit (image,
                       GIMP_MAINIMAGE_UNIT);

  /* Grid */
  grid = g_object_new (GIMP_TYPE_GRID,
                       "xspacing", GIMP_MAINIMAGE_GRIDXSPACING,
                       "yspacing", GIMP_MAINIMAGE_GRIDYSPACING,
                       NULL);
  gimp_image_set_grid (image,
                       grid,
                       FALSE /*push_undo*/);
  g_object_unref (grid);

  /* Channel */
  channel = gimp_channel_new (image,
                              GIMP_MAINIMAGE_CHANNEL1_WIDTH,
                              GIMP_MAINIMAGE_CHANNEL1_HEIGHT,
                              GIMP_MAINIMAGE_CHANNEL1_NAME,
                              channel_color);
  g_object_unref (channel_color);
  gimp_image_add_channel (image,
                          channel,
                          NULL,
                          -1,
                          FALSE /*push_undo*/);

  /* Selection */
  selection = gimp_image_get_mask (image);
  gimp_channel_select_rectangle (selection,
                                 GIMP_MAINIMAGE_SELECTION_X,
                                 GIMP_MAINIMAGE_SELECTION_Y,
                                 GIMP_MAINIMAGE_SELECTION_W,
                                 GIMP_MAINIMAGE_SELECTION_H,
                                 GIMP_CHANNEL_OP_REPLACE,
                                 FALSE /*feather*/,
                                 0.0 /*feather_radius_x*/,
                                 0.0 /*feather_radius_y*/,
                                 FALSE /*push_undo*/);

  /* Path 1 */
  path = gimp_path_new (image,
                        GIMP_MAINIMAGE_PATH1_NAME);
  /* The XCF file can save paths in two kind of ways, one old way
   * and a new way. Parameterize the way so we can test both variants,
   * i.e. gimp_path_compat_is_compatible() must return both TRUE
   * and FALSE.
   */
  if (! compat_paths)
    {
      gimp_item_set_visible (GIMP_ITEM (path),
                             TRUE,
                             FALSE /*push_undo*/);
    }
  /* TODO: Add test for non-closed stroke. The order of the anchor
   * points changes for open strokes, so it's boring to test
   */
  stroke = gimp_bezier_stroke_new_from_coords (path1_coords,
                                               G_N_ELEMENTS (path1_coords),
                                               TRUE /*closed*/);
  gimp_path_stroke_add (path, stroke);
  gimp_image_add_path (image,
                       path,
                       NULL /*parent*/,
                       -1 /*position*/,
                       FALSE /*push_undo*/);

  /* Path 2 */
  path = gimp_path_new (image,
                        GIMP_MAINIMAGE_PATH2_NAME);

  stroke = gimp_bezier_stroke_new_from_coords (path2_coords,
                                               G_N_ELEMENTS (path2_coords),
                                               TRUE /*closed*/);
  gimp_path_stroke_add (path, stroke);
  gimp_image_add_path (image,
                       path,
                       NULL /*parent*/,
                       -1 /*position*/,
                       FALSE /*push_undo*/);

  /* Some of these things are pretty unusual, parameterize the
   * inclusion of this in the written file so we can do our test both
   * with and without
   */
  if (with_unusual_stuff)
    {
      GList *drawables;

      drawables = gimp_image_get_selected_drawables (image);

      /* Floating selection */
      gimp_selection_float (GIMP_SELECTION (gimp_image_get_mask (image)),
                            drawables,
                            gimp_get_user_context (gimp),
                            TRUE /*cut_image*/,
                            0 /*off_x*/,
                            0 /*off_y*/,
                            NULL /*error*/);
      g_list_free (drawables);
    }

  /* Adds stuff like layer groups */
  if (use_gimp_2_8_features)
    {
      GimpLayer *parent;

      /* Add a layer group and some layers:
       *
       *  group1
       *    layer3
       *    layer4
       *    group2
       *      layer5
       */

      /* group1 */
      layer = gimp_group_layer_new (image);
      gimp_object_set_name (GIMP_OBJECT (layer), GIMP_MAINIMAGE_GROUP1_NAME);
      gimp_image_add_layer (image,
                            layer,
                            NULL /*parent*/,
                            -1 /*position*/,
                            FALSE /*push_undo*/);
      parent = layer;

      /* layer3 */
      layer = gimp_layer_new (image,
                              GIMP_MAINIMAGE_LAYER1_WIDTH,
                              GIMP_MAINIMAGE_LAYER1_HEIGHT,
                              GIMP_MAINIMAGE_LAYER1_FORMAT,
                              GIMP_MAINIMAGE_LAYER3_NAME,
                              GIMP_MAINIMAGE_LAYER1_OPACITY,
                              GIMP_MAINIMAGE_LAYER1_MODE);
      gimp_image_add_layer (image,
                            layer,
                            parent,
                            -1 /*position*/,
                            FALSE /*push_undo*/);

      /* layer4 */
      layer = gimp_layer_new (image,
                              GIMP_MAINIMAGE_LAYER1_WIDTH,
                              GIMP_MAINIMAGE_LAYER1_HEIGHT,
                              GIMP_MAINIMAGE_LAYER1_FORMAT,
                              GIMP_MAINIMAGE_LAYER4_NAME,
                              GIMP_MAINIMAGE_LAYER1_OPACITY,
                              GIMP_MAINIMAGE_LAYER1_MODE);
      gimp_image_add_layer (image,
                            layer,
                            parent,
                            -1 /*position*/,
                            FALSE /*push_undo*/);

      /* group2 */
      layer = gimp_group_layer_new (image);
      gimp_object_set_name (GIMP_OBJECT (layer), GIMP_MAINIMAGE_GROUP2_NAME);
      gimp_image_add_layer (image,
                            layer,
                            parent,
                            -1 /*position*/,
                            FALSE /*push_undo*/);
      parent = layer;

      /* layer5 */
      layer = gimp_layer_new (image,
                              GIMP_MAINIMAGE_LAYER1_WIDTH,
                              GIMP_MAINIMAGE_LAYER1_HEIGHT,
                              GIMP_MAINIMAGE_LAYER1_FORMAT,
                              GIMP_MAINIMAGE_LAYER5_NAME,
                              GIMP_MAINIMAGE_LAYER1_OPACITY,
                              GIMP_MAINIMAGE_LAYER1_MODE);
      gimp_image_add_layer (image,
                            layer,
                            parent,
                            -1 /*position*/,
                            FALSE /*push_undo*/);
    }

  /* Todo, should be tested somehow:
   *
   * - Color maps
   * - Custom user units
   * - Text layers
   * - Layer parasites
   * - Channel parasites
   * - Different tile compression methods
   */

  return image;
}

static void
gimp_assert_path (GimpImage   *image,
                  const gchar *name,
                  GimpCoords   coords[],
                  gsize        coords_size,
                  gboolean     visible)
{
  GimpPath    *path           = NULL;
  GimpStroke  *stroke         = NULL;
  GArray      *control_points = NULL;
  gboolean     closed         = FALSE;
  gint         i              = 0;

  path = gimp_image_get_path_by_name (image, name);
  stroke = gimp_path_stroke_get_next (path, NULL);
  g_assert_true (stroke != NULL);
  control_points = gimp_stroke_control_points_get (stroke,
                                                   &closed);
  g_assert_true (closed);
  g_assert_cmpint (control_points->len,
                   ==,
                   coords_size);
  for (i = 0; i < control_points->len; i++)
    {
      g_assert_cmpint (coords[i].x,
                       ==,
                       g_array_index (control_points,
                                      GimpAnchor,
                                      i).position.x);
      g_assert_cmpint (coords[i].y,
                       ==,
                       g_array_index (control_points,
                                      GimpAnchor,
                                      i).position.y);
    }

  g_assert_true (gimp_item_get_visible (GIMP_ITEM (path)) ? TRUE : FALSE ==
                 visible ? TRUE : FALSE);
}

/**
 * gimp_assert_mainimage:
 * @image:
 *
 * Verifies that the passed #GimpImage contains all the information
 * that was put in it by gimp_create_mainimage().
 **/
static void
gimp_assert_mainimage (GimpImage *image,
                       gboolean   with_unusual_stuff,
                       gboolean   compat_paths,
                       gboolean   use_gimp_2_8_features)
{
  const GimpParasite *parasite               = NULL;
  gchar              *parasite_data          = NULL;
  guint32             parasite_size          = -1;
  GimpLayer          *layer                  = NULL;
  GList              *iter                   = NULL;
  GimpGuide          *guide                  = NULL;
  GimpSamplePoint    *sample_point           = NULL;
  gint                sample_point_x         = 0;
  gint                sample_point_y         = 0;
  gdouble             xres                   = 0.0;
  gdouble             yres                   = 0.0;
  GimpGrid           *grid                   = NULL;
  gdouble             xspacing               = 0.0;
  gdouble             yspacing               = 0.0;
  GimpChannel        *channel                = NULL;
  GeglColor          *actual_channel_color;
  gdouble             expected_rgb_color[4]  = GIMP_MAINIMAGE_CHANNEL1_COLOR;
  gdouble             rgb[4];
  GimpChannel        *selection              = NULL;
  gint                x                      = -1;
  gint                y                      = -1;
  gint                w                      = -1;
  gint                h                      = -1;
  GimpCoords          path1_coords[]         = GIMP_MAINIMAGE_PATH1_COORDS;
  GimpCoords          path2_coords[]         = GIMP_MAINIMAGE_PATH2_COORDS;

  /* Image size and type */
  g_assert_cmpint (gimp_image_get_width (image),
                   ==,
                   GIMP_MAINIMAGE_WIDTH);
  g_assert_cmpint (gimp_image_get_height (image),
                   ==,
                   GIMP_MAINIMAGE_HEIGHT);
  g_assert_cmpint (gimp_image_get_base_type (image),
                   ==,
                   GIMP_MAINIMAGE_TYPE);

  /* Layers */
  layer = gimp_image_get_layer_by_name (image,
                                        GIMP_MAINIMAGE_LAYER1_NAME);
  g_assert_cmpint (gimp_item_get_width (GIMP_ITEM (layer)),
                   ==,
                   GIMP_MAINIMAGE_LAYER1_WIDTH);
  g_assert_cmpint (gimp_item_get_height (GIMP_ITEM (layer)),
                   ==,
                   GIMP_MAINIMAGE_LAYER1_HEIGHT);
  g_assert_cmpstr (babl_get_name (gimp_drawable_get_format (GIMP_DRAWABLE (layer))),
                   ==,
                   babl_get_name (GIMP_MAINIMAGE_LAYER1_FORMAT));
  g_assert_cmpstr (gimp_object_get_name (GIMP_DRAWABLE (layer)),
                   ==,
                   GIMP_MAINIMAGE_LAYER1_NAME);
  g_assert_cmpfloat (gimp_layer_get_opacity (layer),
                     ==,
                     GIMP_MAINIMAGE_LAYER1_OPACITY);
  g_assert_cmpint (gimp_layer_get_mode (layer),
                   ==,
                   GIMP_MAINIMAGE_LAYER1_MODE);
  layer = gimp_image_get_layer_by_name (image,
                                        GIMP_MAINIMAGE_LAYER2_NAME);
  g_assert_cmpint (gimp_item_get_width (GIMP_ITEM (layer)),
                   ==,
                   GIMP_MAINIMAGE_LAYER2_WIDTH);
  g_assert_cmpint (gimp_item_get_height (GIMP_ITEM (layer)),
                   ==,
                   GIMP_MAINIMAGE_LAYER2_HEIGHT);
  g_assert_cmpstr (babl_get_name (gimp_drawable_get_format (GIMP_DRAWABLE (layer))),
                   ==,
                   babl_get_name (GIMP_MAINIMAGE_LAYER2_FORMAT));
  g_assert_cmpstr (gimp_object_get_name (GIMP_DRAWABLE (layer)),
                   ==,
                   GIMP_MAINIMAGE_LAYER2_NAME);
  g_assert_cmpfloat (gimp_layer_get_opacity (layer),
                     ==,
                     GIMP_MAINIMAGE_LAYER2_OPACITY);
  g_assert_cmpint (gimp_layer_get_mode (layer),
                   ==,
                   GIMP_MAINIMAGE_LAYER2_MODE);

  /* Guides, note that we rely on internal ordering */
  iter = gimp_image_get_guides (image);
  g_assert_true (iter != NULL);
  guide = iter->data;
  g_assert_cmpint (gimp_guide_get_position (guide),
                   ==,
                   GIMP_MAINIMAGE_VGUIDE1_POS);
  iter = g_list_next (iter);
  g_assert_true (iter != NULL);
  guide = iter->data;
  g_assert_cmpint (gimp_guide_get_position (guide),
                   ==,
                   GIMP_MAINIMAGE_VGUIDE2_POS);
  iter = g_list_next (iter);
  g_assert_true (iter != NULL);
  guide = iter->data;
  g_assert_cmpint (gimp_guide_get_position (guide),
                   ==,
                   GIMP_MAINIMAGE_HGUIDE1_POS);
  iter = g_list_next (iter);
  g_assert_true (iter != NULL);
  guide = iter->data;
  g_assert_cmpint (gimp_guide_get_position (guide),
                   ==,
                   GIMP_MAINIMAGE_HGUIDE2_POS);
  iter = g_list_next (iter);
  g_assert_true (iter == NULL);

  /* Sample points, we rely on the same ordering as when we added
   * them, although this ordering is not a necessity
   */
  iter = gimp_image_get_sample_points (image);
  g_assert_true (iter != NULL);
  sample_point = iter->data;
  gimp_sample_point_get_position (sample_point,
                                  &sample_point_x, &sample_point_y);
  g_assert_cmpint (sample_point_x,
                   ==,
                   GIMP_MAINIMAGE_SAMPLEPOINT1_X);
  g_assert_cmpint (sample_point_y,
                   ==,
                   GIMP_MAINIMAGE_SAMPLEPOINT1_Y);
  iter = g_list_next (iter);
  g_assert_true (iter != NULL);
  sample_point = iter->data;
  gimp_sample_point_get_position (sample_point,
                                  &sample_point_x, &sample_point_y);
  g_assert_cmpint (sample_point_x,
                   ==,
                   GIMP_MAINIMAGE_SAMPLEPOINT2_X);
  g_assert_cmpint (sample_point_y,
                   ==,
                   GIMP_MAINIMAGE_SAMPLEPOINT2_Y);
  iter = g_list_next (iter);
  g_assert_true (iter == NULL);

  /* Resolution */
  gimp_image_get_resolution (image, &xres, &yres);
  g_assert_cmpint (xres,
                   ==,
                   GIMP_MAINIMAGE_RESOLUTIONX);
  g_assert_cmpint (yres,
                   ==,
                   GIMP_MAINIMAGE_RESOLUTIONY);

  /* Parasites */
  parasite = gimp_image_parasite_find (image,
                                       GIMP_MAINIMAGE_PARASITE_NAME);
  parasite_data = (gchar *) gimp_parasite_get_data (parasite, &parasite_size);
  parasite_data = g_strndup (parasite_data, parasite_size);
  g_assert_cmpint (parasite_size,
                   ==,
                   GIMP_MAINIMAGE_PARASITE_SIZE);
  g_assert_cmpstr (parasite_data,
                   ==,
                   GIMP_MAINIMAGE_PARASITE_DATA);
  g_free (parasite_data);

  parasite = gimp_image_parasite_find (image,
                                       "gimp-comment");
  parasite_data = (gchar *) gimp_parasite_get_data (parasite, &parasite_size);
  parasite_data = g_strndup (parasite_data, parasite_size);
  g_assert_cmpint (parasite_size,
                   ==,
                   strlen (GIMP_MAINIMAGE_COMMENT) + 1);
  g_assert_cmpstr (parasite_data,
                   ==,
                   GIMP_MAINIMAGE_COMMENT);
  g_free (parasite_data);

  /* Unit */
  g_assert_true (gimp_image_get_unit (image) == GIMP_MAINIMAGE_UNIT);

  /* Grid */
  grid = gimp_image_get_grid (image);
  g_object_get (grid,
                "xspacing", &xspacing,
                "yspacing", &yspacing,
                NULL);
  g_assert_cmpint (xspacing,
                   ==,
                   GIMP_MAINIMAGE_GRIDXSPACING);
  g_assert_cmpint (yspacing,
                   ==,
                   GIMP_MAINIMAGE_GRIDYSPACING);


  /* Channel */
  channel = gimp_image_get_channel_by_name (image,
                                            GIMP_MAINIMAGE_CHANNEL1_NAME);
  actual_channel_color = gimp_channel_get_color (channel);
  g_assert_cmpint (gimp_item_get_width (GIMP_ITEM (channel)),
                   ==,
                   GIMP_MAINIMAGE_CHANNEL1_WIDTH);
  g_assert_cmpint (gimp_item_get_height (GIMP_ITEM (channel)),
                   ==,
                   GIMP_MAINIMAGE_CHANNEL1_HEIGHT);
  gegl_color_get_pixel (actual_channel_color, babl_format ("R'G'B'A double"), &rgb);
  g_assert_true (memcmp (&expected_rgb_color, &rgb, sizeof (expected_rgb_color)) == 0);

  /* Selection, if the image contains unusual stuff it contains a
   * floating select, and when floating a selection, the selection
   * mask is cleared, so don't test for the presence of the selection
   * mask in that case
   */
  if (! with_unusual_stuff)
    {
      selection = gimp_image_get_mask (image);
      gimp_item_bounds (GIMP_ITEM (selection), &x, &y, &w, &h);
      g_assert_cmpint (x,
                       ==,
                       GIMP_MAINIMAGE_SELECTION_X);
      g_assert_cmpint (y,
                       ==,
                       GIMP_MAINIMAGE_SELECTION_Y);
      g_assert_cmpint (w,
                       ==,
                       GIMP_MAINIMAGE_SELECTION_W);
      g_assert_cmpint (h,
                       ==,
                       GIMP_MAINIMAGE_SELECTION_H);
    }

  /* Path 1 */
  gimp_assert_path (image,
                    GIMP_MAINIMAGE_PATH1_NAME,
                    path1_coords,
                    G_N_ELEMENTS (path1_coords),
                    ! compat_paths /*visible*/);

  /* Path 2 (always visible FALSE) */
  gimp_assert_path (image,
                    GIMP_MAINIMAGE_PATH2_NAME,
                    path2_coords,
                    G_N_ELEMENTS (path2_coords),
                    FALSE /*visible*/);

  if (with_unusual_stuff)
    g_assert_true (gimp_image_get_floating_selection (image) != NULL);
  else /* if (! with_unusual_stuff) */
    g_assert_true (gimp_image_get_floating_selection (image) == NULL);

  if (use_gimp_2_8_features)
    {
      /* Only verify the parent relationships, the layer attributes
       * are tested above
       */
      GimpItem *group1 = GIMP_ITEM (gimp_image_get_layer_by_name (image, GIMP_MAINIMAGE_GROUP1_NAME));
      GimpItem *layer3 = GIMP_ITEM (gimp_image_get_layer_by_name (image, GIMP_MAINIMAGE_LAYER3_NAME));
      GimpItem *layer4 = GIMP_ITEM (gimp_image_get_layer_by_name (image, GIMP_MAINIMAGE_LAYER4_NAME));
      GimpItem *group2 = GIMP_ITEM (gimp_image_get_layer_by_name (image, GIMP_MAINIMAGE_GROUP2_NAME));
      GimpItem *layer5 = GIMP_ITEM (gimp_image_get_layer_by_name (image, GIMP_MAINIMAGE_LAYER5_NAME));

      g_assert_true (gimp_item_get_parent (group1) == NULL);
      g_assert_true (gimp_item_get_parent (layer3) == group1);
      g_assert_true (gimp_item_get_parent (layer4) == group1);
      g_assert_true (gimp_item_get_parent (group2) == group1);
      g_assert_true (gimp_item_get_parent (layer5) == group2);
    }
}


/**
 * main:
 * @argc:
 * @argv:
 *
 * These tests intend to
 *
 *  - Make sure that we are backwards compatible with files created by
 *    older version of GIMP, i.e. that we can load files from earlier
 *    version of GIMP
 *
 *  - Make sure that the information put into a #GimpImage is not lost
 *    when the #GimpImage is written to a file and then read again
 **/
static void
assert_adjustment_pixel (GimpImage *image,
                         gint       x,
                         gfloat     expected)
{
  gfloat pixel[4];

  gimp_test_run_mainloop_until_idle ();
  gimp_image_flush (image);
  gimp_pickable_flush (GIMP_PICKABLE (gimp_image_get_projection (image)));
  gegl_buffer_get (gimp_pickable_get_buffer (GIMP_PICKABLE (gimp_image_get_projection (image))),
                   GEGL_RECTANGLE (x, 2, 1, 1), 1.0, babl_format ("RGBA float"),
                   pixel, GEGL_AUTO_ROWSTRIDE, GEGL_ABYSS_NONE);
  g_assert_cmpfloat_with_epsilon (pixel[0], expected, 0.01);
  g_assert_cmpfloat_with_epsilon (pixel[1], expected, 0.01);
  g_assert_cmpfloat_with_epsilon (pixel[2], expected, 0.01);
  g_assert_cmpfloat_with_epsilon (pixel[3], 1.0, 0.01);
}

static void
adjustment_group_roundtrip (Gimp     *gimp,
                            gboolean  levels)
{
  GimpImage *image = gimp_image_new (gimp, 10, 10, GIMP_RGB,
                                    GIMP_PRECISION_FLOAT_LINEAR);
  GimpLayer *source = gimp_layer_new (image, 10, 10, babl_format ("RGBA float"),
                                     "Original", GIMP_OPACITY_OPAQUE,
                                     GIMP_LAYER_MODE_NORMAL);
  GimpLayer *group = gimp_group_layer_new (image);
  GeglColor *white = gegl_color_new ("white");
  const gchar *operation_name = levels ? "gimp:levels" : "gegl:invert-linear";
  GeglNode *operation = gegl_node_new_child (NULL, "operation", operation_name, NULL);
  GimpLevelsConfig *config = NULL;
  GimpDrawableFilter *filter;
  GimpLayerMask *mask;
  gfloat mask_row[10] = { 1.0, 0.5, 1.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 0.0 };

  gimp_image_add_layer (image, source, NULL, 0, FALSE);
  gegl_buffer_set_color (gimp_drawable_get_buffer (GIMP_DRAWABLE (source)), NULL, white);
  g_object_unref (white);
  gimp_object_set_name (GIMP_OBJECT (group), "Adjustment");
  gimp_layer_set_mode (group, GIMP_LAYER_MODE_PASS_THROUGH, FALSE);
  gimp_image_add_layer (image, group, NULL, 0, FALSE);
  filter = gimp_drawable_filter_new (GIMP_DRAWABLE (group),
                                     levels ? "Levels" : "Invert", operation, NULL);
  if (levels)
    {
      config = g_object_new (GIMP_TYPE_LEVELS_CONFIG, NULL);
      gegl_node_set (operation, "config", config, NULL);
    }
  gimp_drawable_filter_set_opacity (filter, 0.5);
  gimp_drawable_filter_apply (filter, NULL);
  g_assert_true (gimp_drawable_filter_commit (filter, TRUE, NULL, FALSE));
  gimp_drawable_filter_layer_mask_freeze (filter);
  if (levels)
    {
      /* Defaults preserve the image; changing the native setting stays editable. */
      assert_adjustment_pixel (image, 2, 1.0);
      g_object_set (config, "high-output", 0.0, NULL);
      gimp_operation_config_sync_node (G_OBJECT (config), operation);
      gimp_drawable_filter_apply (filter, NULL);
      g_object_unref (config);
    }
  g_object_unref (filter);
  g_object_unref (operation);
  assert_adjustment_pixel (image, 2, 0.5);
  mask = gimp_layer_create_mask (group, GIMP_ADD_MASK_WHITE, NULL);
  g_assert_nonnull (mask);
  g_assert_cmpint (gimp_item_get_width (GIMP_ITEM (mask)), ==, 10);
  g_assert_cmpint (gimp_item_get_height (GIMP_ITEM (mask)), ==, 10);
  g_assert_nonnull (gimp_layer_add_mask (group, mask, FALSE, TRUE, NULL));
  gegl_buffer_set (gimp_drawable_get_buffer (GIMP_DRAWABLE (mask)),
                   GEGL_RECTANGLE (0, 2, 10, 1), 0, babl_format ("Y float"),
                   mask_row, GEGL_AUTO_ROWSTRIDE);
  gimp_drawable_update (GIMP_DRAWABLE (mask), 0, 2, 10, 1);
  assert_adjustment_pixel (image, 2, 0.5);
  assert_adjustment_pixel (image, 1, 0.75);
  assert_adjustment_pixel (image, 7, 1.0);
  g_assert_true (gimp_image_undo (image));
  g_assert_null (gimp_layer_get_mask (group));
  assert_adjustment_pixel (image, 7, 0.5);
  g_assert_true (gimp_image_redo (image));
  g_assert_true (gimp_layer_get_mask (group) == mask);
  assert_adjustment_pixel (image, 1, 0.75);
  assert_adjustment_pixel (image, 7, 1.0);
  /* Paint after the projection has already been rendered. */
  mask_row[1] = 0.25;
  gegl_buffer_set (gimp_drawable_get_buffer (GIMP_DRAWABLE (mask)),
                   GEGL_RECTANGLE (1, 2, 1, 1), 0, babl_format ("Y float"),
                   &mask_row[1], GEGL_AUTO_ROWSTRIDE);
  gimp_drawable_update (GIMP_DRAWABLE (mask), 1, 2, 1, 1);
  assert_adjustment_pixel (image, 1, 0.875);

  for (gint compression = 0; compression < 2; compression++)
    {
      gchar *filename = NULL;
      gint fd = g_file_open_tmp ("gimp-adjustment-XXXXXX.xcf", &filename, NULL);
      GFile *file;
      GimpPlugInProcedure *proc;
      GimpImage *loaded;
      GimpLayer *loaded_group;
      GimpLayer *loaded_source;
      GimpDrawableFilter *loaded_filter;
      GimpLayerMask *loaded_mask;
      gchar *loaded_operation_name = NULL;
      GError *error = NULL;
      gfloat original[4];

      g_assert_cmpint (fd, !=, -1);
      close (fd);
      file = g_file_new_for_path (filename);
      g_free (filename);
      gimp_image_set_xcf_compression (image, compression);
      proc = gimp_plug_in_manager_file_procedure_find (gimp->plug_in_manager,
                 GIMP_FILE_PROCEDURE_GROUP_SAVE, file, &error);
      g_assert_no_error (error);
      g_assert_nonnull (proc);
      g_assert_cmpint (file_save (gimp, image, NULL, file, proc,
                                  GIMP_RUN_NONINTERACTIVE, FALSE, FALSE, FALSE, &error),
                       ==, GIMP_PDB_SUCCESS);
      g_assert_no_error (error);
      loaded = gimp_test_load_image (gimp, file);
      g_assert_nonnull (loaded);
      g_assert_cmpint (gimp_image_get_n_layers (loaded), ==, 2);
      loaded_group = GIMP_LAYER (gimp_container_get_child_by_index (gimp_image_get_layers (loaded), 0));
      loaded_source = GIMP_LAYER (gimp_container_get_child_by_index (gimp_image_get_layers (loaded), 1));
      g_assert_true (GIMP_IS_GROUP_LAYER (loaded_group));
      g_assert_cmpstr (gimp_object_get_name (loaded_group), ==, "Adjustment");
      g_assert_cmpint (gimp_layer_get_mode (loaded_group), ==, GIMP_LAYER_MODE_PASS_THROUGH);
      loaded_mask = gimp_layer_get_mask (loaded_group);
      g_assert_nonnull (loaded_mask);
      g_assert_cmpint (gimp_item_get_width (GIMP_ITEM (loaded_mask)), ==, 10);
      g_assert_cmpint (gimp_item_get_height (GIMP_ITEM (loaded_mask)), ==, 10);
      g_assert_cmpint (gimp_container_get_n_children (gimp_drawable_get_filters (GIMP_DRAWABLE (loaded_group))), ==, 1);
      loaded_filter = GIMP_DRAWABLE_FILTER (gimp_container_get_child_by_index (
                         gimp_drawable_get_filters (GIMP_DRAWABLE (loaded_group)), 0));
      g_assert_false (gimp_drawable_filter_get_temporary (loaded_filter));
      g_assert_cmpfloat_with_epsilon (gimp_drawable_filter_get_opacity (loaded_filter), 0.5, 0.001);
      gegl_node_get (gimp_drawable_filter_get_operation (loaded_filter), "operation", &loaded_operation_name, NULL);
      g_assert_cmpstr (loaded_operation_name, ==, operation_name);
      g_free (loaded_operation_name);
      gegl_buffer_get (gimp_drawable_get_buffer (GIMP_DRAWABLE (loaded_source)),
                       GEGL_RECTANGLE (2, 2, 1, 1), 1.0, babl_format ("RGBA float"),
                       original, GEGL_AUTO_ROWSTRIDE, GEGL_ABYSS_NONE);
      g_assert_cmpfloat_with_epsilon (original[0], 1.0, 0.01);
      assert_adjustment_pixel (loaded, 2, 0.5);
      assert_adjustment_pixel (loaded, 1, 0.875);
      assert_adjustment_pixel (loaded, 7, 1.0);
      gimp_filter_set_active (GIMP_FILTER (loaded_filter), FALSE);
      assert_adjustment_pixel (loaded, 2, 1.0);
      gimp_filter_set_active (GIMP_FILTER (loaded_filter), TRUE);
      assert_adjustment_pixel (loaded, 2, 0.5);
      assert_adjustment_pixel (loaded, 1, 0.875);
      assert_adjustment_pixel (loaded, 7, 1.0);
      if (levels)
        {
          GimpLevelsConfig *loaded_config = NULL;
          gdouble high_output;

          gegl_node_get (gimp_drawable_filter_get_operation (loaded_filter),
                         "config", &loaded_config, NULL);
          g_assert_true (GIMP_IS_LEVELS_CONFIG (loaded_config));
          g_object_get (loaded_config, "high-output", &high_output, NULL);
          g_assert_cmpfloat_with_epsilon (high_output, 0.0, 0.001);
          g_object_set (loaded_config, "high-output", 1.0, NULL);
          gimp_operation_config_sync_node (G_OBJECT (loaded_config),
                                           gimp_drawable_filter_get_operation (loaded_filter));
          gimp_drawable_filter_apply (loaded_filter, NULL);
          assert_adjustment_pixel (loaded, 1, 1.0);
          g_object_set (loaded_config, "high-output", 0.0, NULL);
          gimp_operation_config_sync_node (G_OBJECT (loaded_config),
                                           gimp_drawable_filter_get_operation (loaded_filter));
          gimp_drawable_filter_apply (loaded_filter, NULL);
          assert_adjustment_pixel (loaded, 1, 0.875);
          g_object_unref (loaded_config);
        }
      g_assert_true (g_file_delete (file, NULL, &error));
      g_assert_no_error (error);
      g_object_unref (file);
      g_object_unref (loaded);
    }
  g_object_unref (image);
}

static void
adjustment_group_xcf_roundtrip (gconstpointer data)
{
  adjustment_group_roundtrip (GIMP (data), FALSE);
}

static void
adjustment_levels_xcf_roundtrip (gconstpointer data)
{
  adjustment_group_roundtrip (GIMP (data), TRUE);
}

int
main (int    argc,
      char **argv)
{
  Gimp *gimp;
  int   result;

  g_test_init (&argc, &argv, NULL);

  gimp_test_utils_set_gimp3_directory ("GIMP_TESTING_ABS_TOP_SRCDIR",
                                       "app/tests/gimpdir");

  /* We share the same application instance across all tests. We need
   * the GUI variant for the file procs
   */
  gimp = gimp_init_for_testing ();

  /* Add tests */
  ADD_TEST (write_and_read_gimp_2_6_format);
  ADD_TEST (write_and_read_gimp_2_6_format_unusual);
  ADD_TEST (load_gimp_2_6_file);
  ADD_TEST (write_and_read_gimp_2_8_format);
  ADD_TEST (adjustment_group_xcf_roundtrip);
  ADD_TEST (adjustment_levels_xcf_roundtrip);

  /* Don't write files to the source dir */
  gimp_test_utils_set_gimp3_directory ("GIMP_TESTING_ABS_TOP_BUILDDIR",
                                       "app/tests/gimpdir-output");

  /* Run the tests */
  result = g_test_run ();

  /* Exit so we don't break script-fu plug-in wire */
  gimp_exit (gimp, TRUE);

  return result;
}
