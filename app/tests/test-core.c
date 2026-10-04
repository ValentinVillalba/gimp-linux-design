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

#include <gegl.h>
#include <gtk/gtk.h>

#include "widgets/widgets-types.h"

#include "widgets/gimpuimanager.h"

#include "core/gimp.h"
#include "core/gimpcontext.h"
#include "core/gimpgrouplayer.h"
#include "core/gimpimage.h"
#include "core/gimpimage-undo.h"
#include "core/gimplayer.h"
#include "core/gimplayer-new.h"

#include "operations/gimplevelsconfig.h"

#include "actions/layers-commands.h"

#include "tests.h"

#include "gimp-app-test-utils.h"


#define GIMP_TEST_IMAGE_SIZE 100

#define ADD_IMAGE_TEST(function) \
  g_test_add ("/gimp-core/" #function, \
              GimpTestFixture, \
              gimp, \
              gimp_test_image_setup, \
              function, \
              gimp_test_image_teardown);

#define ADD_TEST(function) \
  g_test_add ("/gimp-core/" #function, \
              GimpTestFixture, \
              gimp, \
              NULL, \
              function, \
              NULL);


typedef struct
{
  GimpImage *image;
} GimpTestFixture;


static void gimp_test_image_setup    (GimpTestFixture *fixture,
                                      gconstpointer    data);
static void gimp_test_image_teardown (GimpTestFixture *fixture,
                                      gconstpointer    data);


/**
 * gimp_test_image_setup:
 * @fixture:
 * @data:
 *
 * Test fixture setup for a single image.
 **/
static void
gimp_test_image_setup (GimpTestFixture *fixture,
                       gconstpointer    data)
{
  Gimp *gimp = GIMP (data);

  fixture->image = gimp_image_new (gimp,
                                   GIMP_TEST_IMAGE_SIZE,
                                   GIMP_TEST_IMAGE_SIZE,
                                   GIMP_RGB,
                                   GIMP_PRECISION_FLOAT_LINEAR);
}

/**
 * gimp_test_image_teardown:
 * @fixture:
 * @data:
 *
 * Test fixture teardown for a single image.
 **/
static void
gimp_test_image_teardown (GimpTestFixture *fixture,
                          gconstpointer    data)
{
  g_object_unref (fixture->image);
}

/**
 * rotate_non_overlapping:
 * @fixture:
 * @data:
 *
 * Super basic test that makes sure we can add a layer
 * and call gimp_item_rotate with center at (0, -10)
 * without triggering a failed assertion .
 **/
static void
rotate_non_overlapping (GimpTestFixture *fixture,
                        gconstpointer    data)
{
  Gimp        *gimp    = GIMP (data);
  GimpImage   *image   = fixture->image;
  GimpLayer   *layer;
  GimpContext *context = gimp_context_new (gimp, "Test", NULL /*template*/);
  gboolean     result;

  g_assert_cmpint (gimp_image_get_n_layers (image), ==, 0);

  layer = gimp_layer_new (image,
                          GIMP_TEST_IMAGE_SIZE,
                          GIMP_TEST_IMAGE_SIZE,
                          babl_format ("R'G'B'A u8"),
                          "Test Layer",
                          GIMP_OPACITY_OPAQUE,
                          GIMP_LAYER_MODE_NORMAL);

  g_assert_cmpint (GIMP_IS_LAYER (layer), ==, TRUE);

  result = gimp_image_add_layer (image,
                                 layer,
                                 GIMP_IMAGE_ACTIVE_PARENT,
                                 0,
                                 FALSE);

  gimp_item_rotate (GIMP_ITEM (layer), context, GIMP_ROTATE_DEGREES90, 0., -10., TRUE);

  g_assert_cmpint (result, ==, TRUE);
  g_assert_cmpint (gimp_image_get_n_layers (image), ==, 1);
  g_object_unref (context);
}

/**
 * add_layer:
 * @fixture:
 * @data:
 *
 * Super basic test that makes sure we can add a layer.
 **/
static void
add_layer (GimpTestFixture *fixture,
           gconstpointer    data)
{
  GimpImage *image = fixture->image;
  GimpLayer *layer;
  gboolean   result;

  g_assert_cmpint (gimp_image_get_n_layers (image), ==, 0);

  layer = gimp_layer_new (image,
                          GIMP_TEST_IMAGE_SIZE,
                          GIMP_TEST_IMAGE_SIZE,
                          babl_format ("R'G'B'A u8"),
                          "Test Layer",
                          GIMP_OPACITY_OPAQUE,
                          GIMP_LAYER_MODE_NORMAL);

  g_assert_cmpint (GIMP_IS_LAYER (layer), ==, TRUE);

  result = gimp_image_add_layer (image,
                                 layer,
                                 GIMP_IMAGE_ACTIVE_PARENT,
                                 0,
                                 FALSE);

  g_assert_cmpint (result, ==, TRUE);
  g_assert_cmpint (gimp_image_get_n_layers (image), ==, 1);
}

/**
 * remove_layer:
 * @fixture:
 * @data:
 *
 * Super basic test that makes sure we can remove a layer.
 **/
static void
remove_layer (GimpTestFixture *fixture,
              gconstpointer    data)
{
  GimpImage *image = fixture->image;
  GimpLayer *layer;
  gboolean   result;

  g_assert_cmpint (gimp_image_get_n_layers (image), ==, 0);

  layer = gimp_layer_new (image,
                          GIMP_TEST_IMAGE_SIZE,
                          GIMP_TEST_IMAGE_SIZE,
                          babl_format ("R'G'B'A u8"),
                          "Test Layer",
                          GIMP_OPACITY_OPAQUE,
                          GIMP_LAYER_MODE_NORMAL);

  g_assert_cmpint (GIMP_IS_LAYER (layer), ==, TRUE);

  result = gimp_image_add_layer (image,
                                 layer,
                                 GIMP_IMAGE_ACTIVE_PARENT,
                                 0,
                                 FALSE);

  g_assert_cmpint (result, ==, TRUE);
  g_assert_cmpint (gimp_image_get_n_layers (image), ==, 1);

  gimp_image_remove_layer (image,
                           layer,
                           FALSE,
                           NULL);

  g_assert_cmpint (gimp_image_get_n_layers (image), ==, 0);
}

/**
 * white_graypoint_in_red_levels:
 * @fixture:
 * @data:
 *
 * Makes sure the levels algorithm can handle when the graypoint is
 * white. It's easy to get a divide by zero problem when trying to
 * calculate what gamma will give a white graypoint.
 **/
static void
white_graypoint_in_red_levels (GimpTestFixture *fixture,
                               gconstpointer    data)
{
  GeglColor            *black   = gegl_color_new ("transparent");
  GeglColor            *gray    = gegl_color_new ("white");
  GeglColor            *white   = gegl_color_new ("white");
  GimpHistogramChannel  channel = GIMP_HISTOGRAM_RED;
  GimpLevelsConfig     *config;

  config = g_object_new (GIMP_TYPE_LEVELS_CONFIG, NULL);

  gimp_levels_config_adjust_by_colors (config,
                                       channel,
                                       NULL,
                                       black,
                                       gray,
                                       white);

  /* Make sure we didn't end up with an invalid gamma value */
  g_object_set (config,
                "gamma", config->gamma[channel],
                NULL);

  g_clear_object (&black);
  g_clear_object (&gray);
  g_clear_object (&white);
}

static void
adjustment_group_undo_redo (GimpTestFixture *fixture,
                            gconstpointer    data)
{
  Gimp        *gimp = GIMP (data);
  GimpImage   *image = fixture->image;
  GimpContext *context = gimp_get_user_context (gimp);
  GimpLayer   *adjustment;

  gimp_context_set_image (context, image);
  layers_new_adjustment_group_cmd_callback (NULL, NULL, gimp);
  g_assert_cmpint (gimp_image_get_n_layers (image), ==, 1);

  adjustment = gimp_image_get_selected_layers (image)->data;
  g_assert_true (GIMP_IS_GROUP_LAYER (adjustment));
  g_assert_cmpint (gimp_layer_get_mode (adjustment), ==,
                   GIMP_LAYER_MODE_PASS_THROUGH);
  g_assert_null (gimp_item_get_parent (GIMP_ITEM (adjustment)));
  g_assert_cmpint (gimp_item_get_index (GIMP_ITEM (adjustment)), ==, 0);

  g_assert_true (gimp_image_undo (image));
  g_assert_cmpint (gimp_image_get_n_layers (image), ==, 0);
  g_assert_true (gimp_image_redo (image));
  g_assert_cmpint (gimp_image_get_n_layers (image), ==, 1);
  gimp_context_set_image (context, NULL);
}

static void
adjustment_group_keeps_parent (GimpTestFixture *fixture,
                               gconstpointer    data)
{
  Gimp        *gimp = GIMP (data);
  GimpImage   *image = fixture->image;
  GimpContext *context = gimp_get_user_context (gimp);
  GimpLayer   *parent = gimp_group_layer_new (image);
  GimpLayer   *layer;
  GimpLayer   *adjustment;

  gimp_image_add_layer (image, parent, NULL, 0, FALSE);
  layer = gimp_layer_new (image, 10, 10, babl_format ("R'G'B'A u8"),
                          "Child", GIMP_OPACITY_OPAQUE,
                          GIMP_LAYER_MODE_NORMAL);
  gimp_image_add_layer (image, layer, parent, 0, FALSE);
  gimp_context_set_image (context, image);

  layers_new_adjustment_group_cmd_callback (NULL, NULL, gimp);
  adjustment = gimp_image_get_selected_layers (image)->data;
  g_assert_true (GIMP_IS_GROUP_LAYER (adjustment));
  g_assert_true (gimp_item_get_parent (GIMP_ITEM (adjustment)) == GIMP_ITEM (parent));
  g_assert_true (gimp_item_get_parent (GIMP_ITEM (layer)) == GIMP_ITEM (parent));
  g_assert_cmpint (gimp_item_get_index (GIMP_ITEM (adjustment)), ==, 0);
  g_assert_cmpint (gimp_item_get_index (GIMP_ITEM (layer)), ==, 1);
  g_assert_true (gimp_image_undo (image));
  g_assert_cmpint (gimp_item_get_index (GIMP_ITEM (layer)), ==, 0);
  gimp_context_set_image (context, NULL);
}

static GimpLayer *
group_test_layer (GimpImage *image,
                  GimpLayer *parent,
                  const char *name)
{
  GimpLayer *layer = gimp_layer_new (image, 10, 10,
                                     babl_format ("R'G'B'A u8"), name,
                                     GIMP_OPACITY_OPAQUE,
                                     GIMP_LAYER_MODE_NORMAL);
  gimp_image_add_layer (image, layer, parent, -1, FALSE);
  return layer;
}

static void
group_selected_order_undo (GimpTestFixture *fixture,
                           gconstpointer    data)
{
  Gimp        *gimp = GIMP (data);
  GimpImage   *image = fixture->image;
  GimpContext *context = gimp_get_user_context (gimp);
  GimpLayer   *bottom = group_test_layer (image, NULL, "Bottom");
  GimpLayer   *middle = group_test_layer (image, NULL, "Middle");
  GimpLayer   *top = group_test_layer (image, NULL, "Top");
  GimpLayer   *group;
  GList       *selected = NULL;

  gimp_context_set_image (context, image);
  selected = g_list_append (selected, bottom);
  selected = g_list_append (selected, top);
  gimp_image_set_selected_layers (image, selected);
  g_list_free (selected);
  layers_group_selected_cmd_callback (NULL, NULL, gimp);
  group = gimp_image_get_selected_layers (image)->data;
  g_assert_true (GIMP_IS_GROUP_LAYER (group));
  g_assert_cmpint (gimp_layer_get_mode (group), ==, GIMP_LAYER_MODE_PASS_THROUGH);
  g_assert_true (gimp_layer_get_parent (top) == group);
  g_assert_true (gimp_layer_get_parent (bottom) == group);
  g_assert_cmpint (gimp_item_get_index (GIMP_ITEM (top)), ==, 0);
  g_assert_cmpint (gimp_item_get_index (GIMP_ITEM (bottom)), ==, 1);
  g_assert_null (gimp_layer_get_parent (middle));
  g_assert_cmpint (gimp_item_get_index (GIMP_ITEM (group)), ==, 0);
  g_assert_true (gimp_image_undo (image));
  g_assert_null (gimp_layer_get_parent (top));
  g_assert_null (gimp_layer_get_parent (bottom));
  g_assert_cmpint (gimp_item_get_index (GIMP_ITEM (top)), ==, 0);
  g_assert_cmpint (gimp_item_get_index (GIMP_ITEM (middle)), ==, 1);
  g_assert_cmpint (gimp_item_get_index (GIMP_ITEM (bottom)), ==, 2);
  g_assert_true (gimp_image_redo (image));
  g_assert_true (gimp_layer_get_parent (top) == group);
  g_assert_true (gimp_layer_get_parent (bottom) == group);
  gimp_context_set_image (context, NULL);
}

static void
group_selected_common_parent (GimpTestFixture *fixture,
                              gconstpointer    data)
{
  Gimp        *gimp = GIMP (data);
  GimpImage   *image = fixture->image;
  GimpContext *context = gimp_get_user_context (gimp);
  GimpLayer   *outer = gimp_group_layer_new (image);
  GimpLayer   *nested = gimp_group_layer_new (image);
  GimpLayer   *a;
  GimpLayer   *b;
  GimpLayer   *group;
  GList       *selected = NULL;

  gimp_image_add_layer (image, outer, NULL, 0, FALSE);
  gimp_image_add_layer (image, nested, outer, 0, FALSE);
  a = group_test_layer (image, nested, "Nested child");
  b = group_test_layer (image, outer, "Sibling");
  gimp_context_set_image (context, image);
  selected = g_list_append (selected, a);
  selected = g_list_append (selected, b);
  gimp_image_set_selected_layers (image, selected);
  g_list_free (selected);
  layers_group_selected_cmd_callback (NULL, NULL, gimp);
  group = gimp_image_get_selected_layers (image)->data;
  g_assert_true (gimp_layer_get_parent (group) == outer);
  g_assert_true (gimp_layer_get_parent (a) == group);
  g_assert_true (gimp_layer_get_parent (b) == group);
  g_assert_true (gimp_layer_get_parent (nested) == outer);
  g_assert_true (gimp_image_undo (image));
  g_assert_true (gimp_layer_get_parent (a) == nested);
  g_assert_true (gimp_layer_get_parent (b) == outer);
  gimp_context_set_image (context, NULL);
}

static void
group_selected_ancestor_and_lock (GimpTestFixture *fixture,
                                  gconstpointer    data)
{
  Gimp        *gimp = GIMP (data);
  GimpImage   *image = fixture->image;
  GimpContext *context = gimp_get_user_context (gimp);
  GimpLayer   *parent = gimp_group_layer_new (image);
  GimpLayer   *child;
  GimpLayer   *group;
  GList       *selected = NULL;

  gimp_image_add_layer (image, parent, NULL, 0, FALSE);
  child = group_test_layer (image, parent, "Child");
  gimp_context_set_image (context, image);
  selected = g_list_append (selected, child);
  selected = g_list_append (selected, parent);
  gimp_image_set_selected_layers (image, selected);
  g_list_free (selected);
  gimp_item_set_lock_position (GIMP_ITEM (parent), TRUE, FALSE);
  layers_group_selected_cmd_callback (NULL, NULL, gimp);
  g_assert_cmpint (gimp_image_get_n_layers (image), ==, 2);
  g_assert_null (gimp_layer_get_parent (parent));
  gimp_item_set_lock_position (GIMP_ITEM (parent), FALSE, FALSE);
  layers_group_selected_cmd_callback (NULL, NULL, gimp);
  group = gimp_image_get_selected_layers (image)->data;
  g_assert_true (gimp_layer_get_parent (parent) == group);
  g_assert_true (gimp_layer_get_parent (child) == parent);
  g_assert_null (gimp_layer_get_parent (group));
  g_assert_true (gimp_image_undo (image));
  g_assert_null (gimp_layer_get_parent (parent));
  g_assert_true (gimp_layer_get_parent (child) == parent);
  gimp_context_set_image (context, NULL);
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

  /* We share the same application instance across all tests */
  gimp = gimp_init_for_testing ();

  /* Add tests */
  ADD_IMAGE_TEST (add_layer);
  ADD_IMAGE_TEST (remove_layer);
  ADD_IMAGE_TEST (rotate_non_overlapping);
  ADD_IMAGE_TEST (adjustment_group_undo_redo);
  ADD_IMAGE_TEST (adjustment_group_keeps_parent);
  ADD_IMAGE_TEST (group_selected_order_undo);
  ADD_IMAGE_TEST (group_selected_common_parent);
  ADD_IMAGE_TEST (group_selected_ancestor_and_lock);
  ADD_TEST (white_graypoint_in_red_levels);

  /* Run the tests */
  result = g_test_run ();

  /* Don't write files to the source dir */
  gimp_test_utils_set_gimp3_directory ("GIMP_TESTING_ABS_TOP_BUILDDIR",
                                       "app/tests/gimpdir-output");

  /* Exit so we don't break script-fu plug-in wire */
  gimp_exit (gimp, TRUE);

  return result;
}
