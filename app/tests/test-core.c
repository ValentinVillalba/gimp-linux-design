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
#include "core/gimpchannel.h"
#include "core/gimpchannel-combine.h"
#include "core/gimpdrawable.h"
#include "core/gimpdrawable-filters.h"
#include "core/gimpdrawablefilter.h"
#include "core/gimpcontainer.h"
#include "core/gimplayermask.h"
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

static void
copy_selection_pixels_offsets_undo (GimpTestFixture *fixture,
                                    gconstpointer    data)
{
  Gimp        *gimp = GIMP (data);
  GimpImage   *image = fixture->image;
  GimpContext *context = gimp_get_user_context (gimp);
  GimpLayer   *parent = gimp_group_layer_new (image);
  GimpLayer   *source;
  GimpLayer   *copy;
  GimpChannel *selection = gimp_image_get_mask (image);
  GimpObject  *clipboard = gimp_get_clipboard_object (gimp);
  GeglColor   *white = gegl_color_new ("white");
  gfloat       mask[3] = { 1.0, 0.5, 0.0 };
  gfloat       pixels[12];
  gfloat       mask_pixels[3] = { 0.25, 0.75, 1.0 };
  GimpLayerMask *layer_mask;
  GeglNode     *operation;
  GimpDrawableFilter *filter;
  gint         x, y;

  gimp_image_add_layer (image, parent, NULL, 0, FALSE);
  source = group_test_layer (image, parent, "Source");
  gimp_item_set_offset (GIMP_ITEM (source), -5, 7);
  gimp_layer_set_opacity (source, 0.7, FALSE);
  gegl_buffer_set_color (gimp_drawable_get_buffer (GIMP_DRAWABLE (source)),
                         NULL, white);
  g_object_unref (white);
  layer_mask = gimp_layer_create_mask (source, GIMP_ADD_MASK_WHITE, NULL);
  g_assert_nonnull (gimp_layer_add_mask (source, layer_mask, FALSE, FALSE, NULL));
  gegl_buffer_set (gimp_drawable_get_buffer (GIMP_DRAWABLE (layer_mask)),
                   GEGL_RECTANGLE (7, 2, 3, 1), 0, babl_format ("Y float"),
                   mask_pixels, GEGL_AUTO_ROWSTRIDE);
  operation = gegl_node_new_child (NULL, "operation", "gegl:invert-linear", NULL);
  filter = gimp_drawable_filter_new (GIMP_DRAWABLE (source), "Invert", operation, NULL);
  gimp_drawable_filter_apply (filter, NULL);
  g_assert_true (gimp_drawable_filter_commit (filter, TRUE, NULL, FALSE));
  gimp_drawable_filter_layer_mask_freeze (filter);
  g_object_unref (filter);
  g_object_unref (operation);
  gimp_channel_combine_rect (selection, GIMP_CHANNEL_OP_REPLACE, 2, 9, 3, 2);
  gegl_buffer_set (gimp_drawable_get_buffer (GIMP_DRAWABLE (selection)),
                   GEGL_RECTANGLE (2, 9, 3, 1), 0, babl_format ("Y float"),
                   mask, GEGL_AUTO_ROWSTRIDE);
  gimp_context_set_image (context, image);
  layers_copy_selection_cmd_callback (NULL, NULL, gimp);
  copy = gimp_image_get_selected_layers (image)->data;
  g_assert_true (copy != source);
  g_assert_true (gimp_layer_get_parent (copy) == parent);
  g_assert_cmpint (gimp_item_get_index (GIMP_ITEM (copy)), ==, 0);
  g_assert_cmpint (gimp_item_get_width (GIMP_ITEM (copy)), ==, 3);
  g_assert_cmpint (gimp_item_get_height (GIMP_ITEM (copy)), ==, 2);
  gimp_item_get_offset (GIMP_ITEM (copy), &x, &y);
  g_assert_cmpint (x, ==, 2);
  g_assert_cmpint (y, ==, 9);
  g_assert_cmpfloat (gimp_layer_get_opacity (copy), ==, 0.7);
  gegl_buffer_get (gimp_drawable_get_buffer (GIMP_DRAWABLE (copy)),
                   GEGL_RECTANGLE (0, 0, 3, 1), 1.0, babl_format ("RGBA float"),
                   pixels, GEGL_AUTO_ROWSTRIDE, GEGL_ABYSS_NONE);
  g_assert_cmpfloat_with_epsilon (pixels[0], 1.0, 0.01);
  g_assert_cmpfloat_with_epsilon (pixels[3], 1.0, 0.01);
  g_assert_cmpfloat_with_epsilon (pixels[7], 0.5, 0.01);
  g_assert_cmpfloat_with_epsilon (pixels[11], 0.0, 0.01);
  gegl_buffer_get (gimp_drawable_get_buffer (GIMP_DRAWABLE (source)),
                   GEGL_RECTANGLE (7, 2, 3, 1), 1.0, babl_format ("RGBA float"),
                   pixels, GEGL_AUTO_ROWSTRIDE, GEGL_ABYSS_NONE);
  g_assert_cmpfloat_with_epsilon (pixels[7], 1.0, 0.01);
  g_assert_nonnull (gimp_layer_get_mask (copy));
  g_assert_true (gimp_layer_get_mask (copy) != layer_mask);
  g_assert_cmpint (gimp_item_get_width (GIMP_ITEM (gimp_layer_get_mask (copy))), ==, 3);
  g_assert_cmpint (gimp_item_get_width (GIMP_ITEM (layer_mask)), ==, 10);
  gegl_buffer_get (gimp_drawable_get_buffer (GIMP_DRAWABLE (gimp_layer_get_mask (copy))),
                   GEGL_RECTANGLE (0, 0, 3, 1), 1.0, babl_format ("Y float"),
                   mask_pixels, GEGL_AUTO_ROWSTRIDE, GEGL_ABYSS_NONE);
  g_assert_cmpfloat_with_epsilon (mask_pixels[0], 0.25, 0.01);
  g_assert_cmpfloat_with_epsilon (mask_pixels[1], 0.75, 0.01);
  g_assert_cmpint (gimp_container_get_n_children (gimp_drawable_get_filters (GIMP_DRAWABLE (copy))), ==, 1);
  g_assert_cmpint (gimp_container_get_n_children (gimp_drawable_get_filters (GIMP_DRAWABLE (source))), ==, 1);
  {
    GeglBuffer *rendered = gimp_drawable_get_buffer_with_effects (GIMP_DRAWABLE (copy));

    gegl_buffer_get (rendered, GEGL_RECTANGLE (0, 0, 1, 1), 1.0,
                     babl_format ("RGBA float"), pixels,
                     GEGL_AUTO_ROWSTRIDE, GEGL_ABYSS_NONE);
    g_assert_cmpfloat_with_epsilon (pixels[0], 0.0, 0.01);
    g_object_unref (rendered);
  }
  g_assert_true (gimp_get_clipboard_object (gimp) == clipboard);
  g_assert_false (gimp_channel_is_empty (selection));
  g_assert_true (gimp_image_undo (image));
  g_assert_cmpint (gimp_image_get_n_layers (image), ==, 2);
  g_assert_true (gimp_image_redo (image));
  g_assert_cmpint (gimp_image_get_n_layers (image), ==, 3);
  g_assert_true (gimp_layer_get_parent (copy) == parent);
  gimp_context_set_image (context, NULL);
}

static void
copy_without_selection_duplicates (GimpTestFixture *fixture,
                                    gconstpointer    data)
{
  Gimp        *gimp = GIMP (data);
  GimpImage   *image = fixture->image;
  GimpContext *context = gimp_get_user_context (gimp);
  GimpLayer   *source = group_test_layer (image, NULL, "Whole layer");
  GimpLayer   *copy;
  gint         x, y;

  gimp_item_set_offset (GIMP_ITEM (source), -3, 11);
  gimp_context_set_image (context, image);
  layers_copy_selection_cmd_callback (NULL, NULL, gimp);
  copy = gimp_image_get_selected_layers (image)->data;
  g_assert_true (copy != source);
  g_assert_cmpint (gimp_item_get_width (GIMP_ITEM (copy)), ==, 10);
  gimp_item_get_offset (GIMP_ITEM (copy), &x, &y);
  g_assert_cmpint (x, ==, -3);
  g_assert_cmpint (y, ==, 11);
  g_assert_true (gimp_image_undo (image));
  g_assert_cmpint (gimp_image_get_n_layers (image), ==, 1);
  gimp_context_set_image (context, NULL);
}

static void
ungroup_order_properties_undo (GimpTestFixture *fixture,
                               gconstpointer    data)
{
  Gimp        *gimp = GIMP (data);
  GimpImage   *image = fixture->image;
  GimpContext *context = gimp_get_user_context (gimp);
  GimpLayer   *outer = gimp_group_layer_new (image);
  GimpLayer   *group = gimp_group_layer_new (image);
  GimpLayer   *bottom;
  GimpLayer   *top;
  GimpLayer   *a;
  GimpLayer   *b;
  GimpLayerMask *mask;
  GeglBuffer  *pixels;
  GeglNode    *operation;
  GimpDrawableFilter *filter;
  GList       *selected = NULL;
  gint         x, y;

  gimp_image_add_layer (image, outer, NULL, 0, FALSE);
  bottom = group_test_layer (image, outer, "Bottom");
  gimp_image_add_layer (image, group, outer, 0, FALSE);
  a = group_test_layer (image, group, "A");
  b = group_test_layer (image, group, "B");
  top = group_test_layer (image, outer, "Top");
  gimp_image_reorder_item (image, GIMP_ITEM (top), GIMP_ITEM (outer), 0, FALSE, NULL);
  gimp_item_set_offset (GIMP_ITEM (a), -7, 12);
  pixels = gimp_drawable_get_buffer (GIMP_DRAWABLE (a));
  gimp_layer_set_opacity (group, 0.4, FALSE);
  mask = gimp_layer_create_mask (group, GIMP_ADD_MASK_WHITE, NULL);
  g_assert_nonnull (gimp_layer_add_mask (group, mask, FALSE, FALSE, NULL));
  operation = gegl_node_new_child (NULL, "operation", "gegl:invert-linear", NULL);
  filter = gimp_drawable_filter_new (GIMP_DRAWABLE (group), "Invert", operation, NULL);
  gimp_drawable_filter_apply (filter, NULL);
  g_assert_true (gimp_drawable_filter_commit (filter, TRUE, NULL, FALSE));
  gimp_drawable_filter_layer_mask_freeze (filter);
  g_object_unref (filter);
  g_object_unref (operation);
  selected = g_list_prepend (selected, group);
  gimp_image_set_selected_layers (image, selected);
  g_list_free (selected);
  gimp_context_set_image (context, image);

  layers_ungroup_selected_cmd_callback (NULL, NULL, gimp);
  g_assert_cmpint (gimp_image_get_n_layers (image), ==, 5);
  g_assert_true (gimp_layer_get_parent (a) == outer);
  g_assert_true (gimp_layer_get_parent (b) == outer);
  g_assert_cmpint (gimp_item_get_index (GIMP_ITEM (top)), ==, 0);
  g_assert_cmpint (gimp_item_get_index (GIMP_ITEM (b)), ==, 1);
  g_assert_cmpint (gimp_item_get_index (GIMP_ITEM (a)), ==, 2);
  g_assert_cmpint (gimp_item_get_index (GIMP_ITEM (bottom)), ==, 3);
  g_assert_true (gimp_drawable_get_buffer (GIMP_DRAWABLE (a)) == pixels);
  gimp_item_get_offset (GIMP_ITEM (a), &x, &y);
  g_assert_cmpint (x, ==, -7);
  g_assert_cmpint (y, ==, 12);
  g_assert_cmpint (g_list_length (gimp_image_get_selected_layers (image)), ==, 2);
  g_assert_true (gimp_image_undo (image));
  g_assert_true (gimp_layer_get_parent (a) == group);
  g_assert_true (gimp_layer_get_parent (b) == group);
  g_assert_true (gimp_layer_get_parent (group) == outer);
  g_assert_true (gimp_layer_get_mask (group) == mask);
  g_assert_cmpint (gimp_container_get_n_children (gimp_drawable_get_filters (GIMP_DRAWABLE (group))), ==, 1);
  g_assert_cmpfloat (gimp_layer_get_opacity (group), ==, 0.4);
  g_assert_cmpint (gimp_item_get_index (GIMP_ITEM (group)), ==, 1);
  g_assert_true (gimp_image_redo (image));
  g_assert_true (gimp_layer_get_parent (a) == outer);
  g_assert_cmpint (gimp_item_get_index (GIMP_ITEM (b)), ==, 1);
  gimp_context_set_image (context, NULL);
}

static void
ungroup_nested_and_mixed_selection (GimpTestFixture *fixture,
                                     gconstpointer    data)
{
  Gimp        *gimp = GIMP (data);
  GimpImage   *image = fixture->image;
  GimpContext *context = gimp_get_user_context (gimp);
  GimpLayer   *outer = gimp_group_layer_new (image);
  GimpLayer   *nested = gimp_group_layer_new (image);
  GimpLayer   *sibling = gimp_group_layer_new (image);
  GimpLayer   *a;
  GimpLayer   *b;
  GimpLayer   *plain;
  GList       *selected = NULL;

  gimp_image_add_layer (image, outer, NULL, 0, FALSE);
  gimp_image_add_layer (image, nested, outer, 0, FALSE);
  a = group_test_layer (image, nested, "A");
  gimp_image_add_layer (image, sibling, NULL, 1, FALSE);
  b = group_test_layer (image, sibling, "B");
  plain = group_test_layer (image, NULL, "Plain");
  gimp_image_reorder_item (image, GIMP_ITEM (plain), NULL, 2, FALSE, NULL);
  selected = g_list_append (selected, outer);
  selected = g_list_append (selected, plain);
  selected = g_list_append (selected, sibling);
  selected = g_list_append (selected, nested);
  gimp_image_set_selected_layers (image, selected);
  g_list_free (selected);
  gimp_context_set_image (context, image);
  layers_ungroup_selected_cmd_callback (NULL, NULL, gimp);
  g_assert_cmpint (gimp_image_get_n_layers (image), ==, 3);
  g_assert_null (gimp_layer_get_parent (a));
  g_assert_null (gimp_layer_get_parent (b));
  g_assert_cmpint (gimp_item_get_index (GIMP_ITEM (a)), ==, 0);
  g_assert_cmpint (gimp_item_get_index (GIMP_ITEM (b)), ==, 1);
  g_assert_cmpint (gimp_item_get_index (GIMP_ITEM (plain)), ==, 2);
  g_assert_cmpint (g_list_length (gimp_image_get_selected_layers (image)), ==, 3);
  g_assert_true (gimp_image_undo (image));
  g_assert_true (gimp_layer_get_parent (a) == nested);
  g_assert_true (gimp_layer_get_parent (nested) == outer);
  g_assert_true (gimp_layer_get_parent (b) == sibling);
  g_assert_true (gimp_image_redo (image));
  g_assert_cmpint (gimp_image_get_n_layers (image), ==, 3);
  gimp_context_set_image (context, NULL);
}

static void
ungroup_empty_and_position_locked (GimpTestFixture *fixture,
                                    gconstpointer    data)
{
  Gimp        *gimp = GIMP (data);
  GimpImage   *image = fixture->image;
  GimpContext *context = gimp_get_user_context (gimp);
  GimpLayer   *empty = gimp_group_layer_new (image);
  GimpLayer   *group = gimp_group_layer_new (image);
  GimpLayer   *child;
  GList       *selected = NULL;

  gimp_image_add_layer (image, empty, NULL, 0, FALSE);
  gimp_image_add_layer (image, group, NULL, 1, FALSE);
  child = group_test_layer (image, group, "Locked");
  selected = g_list_append (selected, empty);
  selected = g_list_append (selected, group);
  gimp_image_set_selected_layers (image, selected);
  g_list_free (selected);
  gimp_context_set_image (context, image);
  gimp_item_set_lock_position (GIMP_ITEM (child), TRUE, FALSE);
  layers_ungroup_selected_cmd_callback (NULL, NULL, gimp);
  g_assert_cmpint (gimp_image_get_n_layers (image), ==, 3);
  g_assert_true (gimp_layer_get_parent (child) == group);
  g_assert_cmpint (gimp_item_get_index (GIMP_ITEM (empty)), ==, 0);
  gimp_item_set_lock_position (GIMP_ITEM (child), FALSE, FALSE);
  layers_ungroup_selected_cmd_callback (NULL, NULL, gimp);
  g_assert_cmpint (gimp_image_get_n_layers (image), ==, 1);
  g_assert_null (gimp_layer_get_parent (child));
  g_assert_true (gimp_image_undo (image));
  g_assert_cmpint (gimp_image_get_n_layers (image), ==, 3);
  g_assert_true (gimp_layer_get_parent (child) == group);
  g_assert_true (gimp_image_redo (image));
  g_assert_cmpint (gimp_image_get_n_layers (image), ==, 1);
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
  ADD_IMAGE_TEST (copy_selection_pixels_offsets_undo);
  ADD_IMAGE_TEST (copy_without_selection_duplicates);
  ADD_IMAGE_TEST (ungroup_order_properties_undo);
  ADD_IMAGE_TEST (ungroup_nested_and_mixed_selection);
  ADD_IMAGE_TEST (ungroup_empty_and_position_locked);
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
