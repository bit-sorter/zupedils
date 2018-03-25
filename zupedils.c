#include <gtk/gtk.h>

#define IMAGE_HEIGHT 400
#define IMAGE_WIDTH 400
#define PIECE_HEIGHT IMAGE_HEIGHT / 4
#define PIECE_WIDTH IMAGE_WIDTH / 4

gboolean show_numbers = FALSE;

gint get_tile_home_position(GtkWidget *event_box)
{
  const gchar *name = gtk_widget_get_name(event_box);

  if (name != NULL) {
    return g_ascii_strtoll(name, NULL, 10);
  }

  return -1;
}

void set_tile_home_position(GtkWidget *event_box, gint home)
{
  gchar *name = g_strdup_printf("%d", home);

  gtk_widget_set_name(event_box, name);
  g_free(name);
}

gint get_tile_current_position(GtkWidget *event_box)
{
  gint i, j;
  GtkWidget *grid = gtk_widget_get_parent(event_box);

  for (j = 0; j < 4; j++) {
    for (i = 0; i < 4; i++) {
      GtkWidget *widget = gtk_grid_get_child_at(GTK_GRID(grid), i, j);

      if (widget == event_box) {
        return i + j * 4;
      }
    }
  }

  return -1;
}

gint get_missing_tile_position(GtkWidget *event_box)
{
  gint i, j;
  GtkWidget *grid = gtk_widget_get_parent(event_box);

  for (j = 0; j < 4; j++) {
    for (i = 0; i < 4; i++) {
      GtkWidget *widget = gtk_grid_get_child_at(GTK_GRID(grid), i, j);

      if (get_tile_home_position(widget) == 15) {
        return i + j * 4;
      }
    }
  }

  return -1;
}

gint find_empty_neighbour_position(GtkWidget *event_box)
{
  gint current_pos = get_tile_current_position(event_box);
  gint empty_pos = get_missing_tile_position(event_box);

  if (current_pos + 4 == empty_pos) {
    return current_pos + 4;
  }

  if (current_pos - 4 == empty_pos) {
    return current_pos - 4;
  }

  if (current_pos % 4 + 1 < 4 && current_pos + 1 == empty_pos) {
    return current_pos + 1;
  }

  if (current_pos % 4 - 1 >= 0 && current_pos - 1 == empty_pos) {
    return current_pos - 1;
  }

  return -1;
}

void exchange_two_tiles(GtkWidget *grid, gint tile_a_position,
                        gint tile_b_position)
{
  GtkWidget *tile_a = gtk_grid_get_child_at(GTK_GRID(grid),
                      tile_a_position % 4, tile_a_position / 4);
  GtkWidget *tile_b = gtk_grid_get_child_at(GTK_GRID(grid),
                      tile_b_position % 4, tile_b_position / 4);

  if (tile_a != NULL && tile_b != NULL) {
    g_object_ref(tile_a);
    g_object_ref(tile_b);
    gtk_container_remove(GTK_CONTAINER(grid), tile_a);
    gtk_container_remove(GTK_CONTAINER(grid), tile_b);
    gtk_grid_attach(GTK_GRID(grid), tile_a,
                    tile_b_position % 4, tile_b_position / 4, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), tile_b,
                    tile_a_position % 4, tile_a_position / 4, 1, 1);
    g_object_unref(tile_a);
    g_object_unref(tile_b);
  }
}

void move_to_empty_neighbour(GtkWidget *event_box, gint neighbour_position)
{
  gint current_position = get_tile_current_position(event_box);

  exchange_two_tiles(gtk_widget_get_parent(event_box),
                     current_position, neighbour_position);
}

gboolean all_tiles_are_ordered(GtkWidget *event_box)
{
  gint count = 0, i;
  GtkWidget *grid = gtk_widget_get_parent(event_box);

  for (i = 0; i < 16; i++) {
    GtkWidget *widget = gtk_grid_get_child_at(GTK_GRID(grid), i % 4, i / 4);

    if (get_tile_home_position(widget) == get_tile_current_position(widget)) {
      count++;
    } else {
      break;
    }
  }

  return count == 16;
}

void show_message(GtkWidget *grid, GtkMessageType type, gchar *msg)
{
  GtkWidget *message = gtk_message_dialog_new(
      GTK_WINDOW(gtk_widget_get_parent(gtk_widget_get_parent(grid))),
      GTK_DIALOG_MODAL, type, GTK_BUTTONS_OK, msg);
  gtk_dialog_run(GTK_DIALOG(message));
  gtk_widget_destroy(message);
}

void game_completed(GtkWidget *event_box)
{
  GtkWidget *grid = gtk_widget_get_parent(event_box);
  GtkWidget *parent = gtk_widget_get_parent(gtk_widget_get_parent(grid));
  GtkWidget *message = gtk_message_dialog_new(GTK_WINDOW(parent),
                                              GTK_DIALOG_MODAL,
                                              GTK_MESSAGE_INFO,
                                              GTK_BUTTONS_OK, "Well Done!");
  GtkWidget *widget = gtk_grid_get_child_at(GTK_GRID(grid), 3, 3);
  GList *child = gtk_container_get_children(GTK_CONTAINER(widget));

  gtk_widget_show(widget);
  gtk_widget_show(GTK_WIDGET(child->data));
  gtk_dialog_run(GTK_DIALOG(message));
  gtk_widget_destroy(message);
  gtk_widget_hide(widget);
  gtk_widget_hide(GTK_WIDGET(child->data));
  g_list_free(child);
}

static gboolean tile_clicked(GtkWidget *event_box,
                             GdkEventButton *event, gpointer data)
{
  gint empty_neighbour_position = find_empty_neighbour_position(event_box);

  if (empty_neighbour_position != -1) {
    move_to_empty_neighbour(event_box, empty_neighbour_position);
  }

  if (all_tiles_are_ordered(event_box)) {
    game_completed(event_box);
  }

  return TRUE;
}

void draw_number(cairo_t *cr, gint indx)
{
  gchar *digits[16] = { "1", "2", "3", "4", "5", "6",
                        "7", "8", "9", "10", "11",
                        "12", "13", "14", "15", "16" };
  cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL,
                         CAIRO_FONT_WEIGHT_BOLD);
  cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
  cairo_set_font_size(cr, 12.0);
  cairo_move_to(cr, 5.0, 12.0);
  cairo_show_text(cr, digits[indx]);
  cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
  cairo_set_font_size(cr, 10.0);
  cairo_move_to(cr, 5.0, 12.0);
  cairo_show_text(cr, digits[indx]);
}

gboolean draw(GtkWidget *widget, cairo_t *cr, gpointer data)
{
  gdk_cairo_set_source_pixbuf(cr,
                              gtk_image_get_pixbuf(GTK_IMAGE(widget)), 0, 0);
  cairo_paint(cr);
  cairo_set_source_rgb(cr, 0, 0, 0);
  cairo_set_line_width(cr, 1);
  cairo_rectangle (cr, 0, 0, PIECE_WIDTH - 1, PIECE_HEIGHT - 1);
  cairo_stroke(cr);

  if (show_numbers) {
    gint tindx = *((gint *)data);
    draw_number(cr, tindx);
  }

  return TRUE;
}

void remove_children(GtkWidget *widget)
{
  GList *child, *ch,
        *children = gtk_container_get_children(GTK_CONTAINER(widget));

  child = ch = g_list_first(children);

  while (child != NULL) {
    gtk_widget_destroy(child->data);
    child = child->next;
  }

  g_list_free(ch);
}

void attach_tiles_to_grid(GtkWidget *grid, GArray *tiles)
{
  gint i;

  for (i = 0; i < 16; i++) {
    GtkWidget *event_box = g_array_index(tiles, GtkWidget *, i);
    gint home = get_tile_home_position(event_box);
    event_box = g_array_index(tiles, GtkWidget *, home);
    gtk_grid_attach(GTK_GRID(grid), event_box, i % 4, i / 4, 1, 1);

    if (home != 15) {
      gtk_widget_show(event_box);
      gtk_widget_show(gtk_bin_get_child(GTK_BIN(event_box)));
    }
  }
}

GtkWidget *create_tile(GtkWidget *grid, GdkPixbuf *image, gint i)
{
  GdkPixbuf *small = gdk_pixbuf_new_subpixbuf(image,
                                              (i % 4) * PIECE_WIDTH,
                                              (i / 4) * PIECE_HEIGHT,
                                              PIECE_WIDTH, PIECE_HEIGHT);
  GtkWidget *tile_image = gtk_image_new_from_pixbuf(small);
  GtkWidget *event_box = gtk_event_box_new();
  gint *tindx = g_new(gint, 1);

  *tindx = i;
  g_signal_connect(tile_image, "draw", G_CALLBACK(draw), tindx);
  set_tile_home_position(event_box, i);
  gtk_container_add(GTK_CONTAINER(event_box), tile_image);
  g_signal_connect(G_OBJECT(event_box), "button_press_event",
                   G_CALLBACK(tile_clicked), grid);

  return event_box;
}

GArray *create_tiles(GtkWidget *grid, gchar *filename)
{
  GArray *tiles = g_array_new(FALSE, FALSE, sizeof(GtkWidget *));
  GtkWidget *image = gtk_image_new_from_file(filename);
  gint i;
  GdkPixbuf *pixbuf, *scaled;

  gtk_widget_set_name(grid, filename);
  pixbuf = gtk_image_get_pixbuf(GTK_IMAGE(image));

  if (pixbuf == NULL) {
    show_message(grid, GTK_MESSAGE_WARNING, "Error Loading Image!");
    return NULL;
  }

  scaled = gdk_pixbuf_scale_simple(pixbuf, IMAGE_WIDTH,
                                   IMAGE_HEIGHT, GDK_INTERP_HYPER);

  if (scaled == NULL) {
    show_message(grid, GTK_MESSAGE_ERROR, "Out Of Memory!");
    gtk_main_quit();
    return NULL;
  }

  for (i = 0; i < 16; i++) {
    GtkWidget *tile = create_tile(grid, scaled, i);
    g_array_append_val(tiles, tile);
  }

  return tiles;
}

gboolean is_progress_file(GIOChannel *channel)
{
  gboolean retval = FALSE;
  gchar *input;
  GIOStatus status;

  status = g_io_channel_read_line(channel, &input, NULL, NULL, NULL);

  if (status == G_IO_STATUS_NORMAL) {
    if (g_strcmp0(input, "Slider\n") == 0) {
      retval = TRUE;
    }
  }

  g_free(input);

  return retval;
}

GArray *get_tiles(GtkWidget *grid, GIOChannel *channel)
{
  gsize terminator_pos;
  gchar *input;
  GIOStatus status = g_io_channel_read_line(channel, &input, NULL,
                                            &terminator_pos, NULL);

  if (status == G_IO_STATUS_NORMAL) {
    GArray *tiles;

    input[terminator_pos] = '\0';
    tiles = create_tiles(grid, input);
    remove_children(grid);
    g_free(input);

    return tiles;
  }

  return NULL;
}

gint open_progress_file_error(GtkWidget *grid, GIOChannel *channel, gint status)
{
  gchar *message;

  g_io_channel_shutdown(channel, TRUE, NULL);
  g_io_channel_unref(channel);

  switch (status) {
    case 1:
      message = "Failed to open progress file.";
      break;
    case 2:
      message = "Failed to create tiles from progress file (Corrupt?).";
      break;
    case 3:
      message = "Failed to read line from progress file.";
      break;
    case 4:
      message = "Corrupt progress file.";
      break;
    case 5:
      message = "Not a progress file.";
      break;
    default:
      message = "Unknown problem with progress file.";
      break;
  }
  
  show_message(grid, GTK_MESSAGE_WARNING, message);

  return status;
}

/* Have we 16 strings between "0" and "15", unique values? */
gboolean is_valid_array(gchar **numstr)
{
  gboolean *used;
  gchar *endptr;
  gint i, value;

  used = g_new0(gboolean, 16);

  for (i = 0; i < 16; i++) {
    if (numstr[i] == NULL) {
      return FALSE;
    }

    value = g_ascii_strtoll(numstr[i], &endptr, 10);

    if (value < 0 || value > 15 || endptr == numstr[i]) {
      return FALSE;
    }

    if (used[value]) {
      return FALSE;
    }

    used[value] = TRUE;
  }

  g_free(used);

  return TRUE;
}

gint open_progress_file(GtkWidget *grid, gchar *filename)
{
  gchar *input, **numstr;
  gint indx;
  GIOChannel *channel;
  GIOStatus status;

  channel = g_io_channel_new_file(filename, "r", NULL);

  if (channel == NULL) {
    return open_progress_file_error(grid, channel, 1);
  }

  if (is_progress_file(channel)) {
    GtkWidget *tile;
    gint64 home;
    GArray *tiles = get_tiles(grid, channel);

    if (tiles == NULL) {
      return open_progress_file_error(grid, channel, 2);
    }

    status = g_io_channel_read_line(channel, &input, NULL, NULL, NULL);

    if (status != G_IO_STATUS_NORMAL) {
      return open_progress_file_error(grid, channel, 3);
    }

    numstr = g_strsplit(input, " ", 16);

    if (!is_valid_array(numstr)) {
      return open_progress_file_error(grid, channel, 4);
    }

    for (indx = 0; indx < 16; indx++) {
      home = g_ascii_strtoll(numstr[indx], NULL, 10);
      tile = g_array_index(tiles, GtkWidget *, home);
      set_tile_home_position(tile, home);
      gtk_grid_attach(GTK_GRID(grid), tile, indx % 4, indx / 4, 1, 1);

      if (home != 15) {
        gtk_widget_show(tile);
        gtk_widget_show(gtk_bin_get_child(GTK_BIN(tile)));
      }
    }

    g_strfreev(numstr);
    g_free(input);
    g_array_free(tiles, FALSE);
  } else {
    return open_progress_file_error(grid, channel, 5);
  }

  g_io_channel_shutdown(channel, TRUE, NULL);
  g_io_channel_unref(channel);

  return 0;
}

void open_file(GtkWidget *grid, gchar *filename)
{
  GFile *file = g_file_new_for_path(filename);
  GFileInfo *info = g_file_query_info(file, "standard::*",
                                      G_FILE_QUERY_INFO_NONE, NULL, NULL);
  const gchar *content_type = g_file_info_get_attribute_string(info,
                              G_FILE_ATTRIBUTE_STANDARD_CONTENT_TYPE);

  if (g_ascii_strncasecmp(content_type, "image/", 6) == 0) {
    GArray *tiles = create_tiles(grid, filename);
    remove_children(grid);
    attach_tiles_to_grid(grid, tiles);
    g_array_free(tiles, FALSE);
  } else if (g_strcmp0(content_type, "text/plain") == 0) {
    open_progress_file(grid, filename);
  } else {
    show_message(grid, GTK_MESSAGE_WARNING,
                 "I don't know what type of file that is.");
  }

  g_object_unref(info);
  g_object_unref(file);
}

static void new_action(GtkWidget *grid)
{
  gchar *filename = NULL;
  GtkWidget *dialogue;
  gint status;

  dialogue = gtk_file_chooser_dialog_new("Choose Image",
      GTK_WINDOW(gtk_widget_get_parent(gtk_widget_get_parent(grid))),
      GTK_FILE_CHOOSER_ACTION_OPEN, ("_Cancel"), GTK_RESPONSE_CANCEL,
      ("_Open"), GTK_RESPONSE_ACCEPT, NULL);
  status = gtk_dialog_run(GTK_DIALOG(dialogue));

  if (status == GTK_RESPONSE_ACCEPT) {
    GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialogue);
    filename = gtk_file_chooser_get_filename(chooser);
  }

  gtk_widget_destroy(dialogue);

  if (filename != NULL) {
    open_file(grid, filename);
    g_free(filename);
  }
}

GIOChannel *get_save_channel(GtkWidget *grid)
{
  GIOChannel *channel;
  gchar *filename = NULL;
  GtkWidget *dialogue;
  gint status;

  dialogue = gtk_file_chooser_dialog_new("Save Progress",
      GTK_WINDOW(gtk_widget_get_parent(gtk_widget_get_parent(grid))),
      GTK_FILE_CHOOSER_ACTION_SAVE, ("_Cancel"), GTK_RESPONSE_CANCEL,
      ("_Save"), GTK_RESPONSE_ACCEPT, NULL);
  status = gtk_dialog_run(GTK_DIALOG(dialogue));

  if (status == GTK_RESPONSE_ACCEPT) {
    GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialogue);
    filename = gtk_file_chooser_get_filename(chooser);
  }

  gtk_widget_destroy(dialogue);

  if (filename != NULL) {
    channel = g_io_channel_new_file(filename, "w", NULL);
    g_free(filename);
    return channel;
  }

  return NULL;
}

static void save_action(GtkWidget *grid)
{
  gchar *data;
  gint i;
  const gchar *image_path = gtk_widget_get_name(grid);
  GIOChannel *channel;

  if (image_path == NULL) {
    return;
  }

  channel = get_save_channel(grid);

  if (channel == NULL) {
    return;
  }

  data = g_strdup_printf("Slider\n%s\n", image_path);
  g_io_channel_write_chars(channel, data, -1, NULL, NULL);
  g_free(data);

  for (i = 0; i < 16; i++) {
    GtkWidget *widget = gtk_grid_get_child_at(GTK_GRID(grid), i % 4, i / 4);
    gint home = get_tile_home_position(widget);
    data = g_strdup_printf("%d ", home);
    g_io_channel_write_chars(channel, data, -1, NULL, NULL);
    g_free(data);
  }

  g_io_channel_shutdown(channel, TRUE, NULL);
  g_io_channel_unref(channel);
}

static void quit_action(GtkWidget *grid)
{
  gtk_main_quit();
}

static void shuffle_tiles(GtkWidget *grid)
{
  gint i, new;
  GRand *rand_;

  if (gtk_grid_get_child_at(GTK_GRID(grid), 0, 0) == NULL) {
    return;
  }

  rand_ = g_rand_new();

  for (i = 0; i < 16; i++) {
    GtkWidget *widget = gtk_grid_get_child_at(GTK_GRID(grid), i % 4, i / 4);
    gint current = get_tile_current_position(widget);
    while ((new = g_rand_int_range(rand_, 0, 15)) == current);
    exchange_two_tiles(grid, current, new);
  }

  g_rand_free(rand_);
}

void show_help(GtkWidget *grid)
{
  gchar *msg = "File - Open opens an image file or a saved progress file.\n"
               "File - Save Progress creates a saved progress file.\n\n"
               "Click on a tile beside an empty space to move the tile.";

  show_message(grid, GTK_MESSAGE_INFO, msg);
}

void create_menu_item(GtkWidget *grid, gchar *action,
                      GtkWidget *menu, GCallback cb)
{
  GtkWidget *menu_item = gtk_menu_item_new_with_mnemonic(action);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);
  g_signal_connect_swapped(G_OBJECT(menu_item), "activate",
                           G_CALLBACK(cb), grid);
}

void create_file_menu_items(GtkWidget *grid, GtkWidget *menu)
{
  create_menu_item(grid, "_Open", menu, G_CALLBACK(new_action));
  create_menu_item(grid, "_Save Progress",
                   menu, G_CALLBACK(save_action));
  create_menu_item(grid, "_Quit", menu, G_CALLBACK(quit_action));
}

void set_show_numbers(GtkWidget *grid)
{
  GtkWidget *widget = gtk_grid_get_child_at(GTK_GRID(grid), 0, 0);

  if (widget != NULL) {
    show_numbers = !show_numbers;
    gtk_widget_queue_draw(grid);
  }
}

void create_options_menu_items(GtkWidget *grid, GtkWidget *menu)
{
  create_menu_item(grid, "_Shuffle", menu, G_CALLBACK(shuffle_tiles));
  create_menu_item(grid, "Show _Numbers",
                   menu, G_CALLBACK(set_show_numbers));
}

GtkWidget *create_menu_bar(GtkWidget *grid)
{
  GtkWidget *item, *menu, *menu_bar;

  menu_bar = gtk_menu_bar_new();
  gtk_widget_set_hexpand(menu_bar, TRUE);

  menu = gtk_menu_new();
  create_file_menu_items(grid, menu);
  item = gtk_menu_item_new_with_mnemonic("_File");
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(item), menu);
  gtk_container_add(GTK_CONTAINER(menu_bar), item);

  menu = gtk_menu_new();
  create_options_menu_items(grid, menu);
  item = gtk_menu_item_new_with_mnemonic("_Options");
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(item), menu);
  gtk_container_add(GTK_CONTAINER(menu_bar), item);

  menu = gtk_menu_new();
  item = gtk_menu_item_new_with_mnemonic("_Help");
  g_signal_connect_swapped(G_OBJECT(item), "activate",
                           G_CALLBACK(show_help), grid);
  gtk_container_add(GTK_CONTAINER(menu_bar), item);

  return menu_bar;
}

GtkWidget *create_main_window(gint width, gint height)
{
  GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_default_size(GTK_WINDOW(window), width, height);
  gtk_window_set_resizable(GTK_WINDOW(window), FALSE);
  g_signal_connect(G_OBJECT(window), "delete_event",
                   G_CALLBACK(gtk_main_quit), NULL);

  return window;
}

gint main(gint argc, gchar *argv[])
{
  GtkWidget *grid, *menu_bar, *vbox, *window;

  gtk_init(&argc, &argv);
  window = create_main_window(IMAGE_WIDTH, IMAGE_HEIGHT);
  vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  grid = gtk_grid_new();
  menu_bar = create_menu_bar(grid);
  gtk_box_pack_start(GTK_BOX(vbox), menu_bar, FALSE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(vbox), grid, TRUE, TRUE, 0);
  gtk_container_add(GTK_CONTAINER(window), vbox);
  gtk_widget_show_all(window);
  gtk_main();

  return 0;
}

