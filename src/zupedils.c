#include <gtk/gtk.h>
#include "resources.h"

#define IMAGE_HEIGHT 400
#define IMAGE_WIDTH 400
#define PIECE_HEIGHT IMAGE_HEIGHT / 4
#define PIECE_WIDTH IMAGE_WIDTH / 4

gboolean show_numbers = FALSE;

gint get_piece_home_position(GtkWidget *piece)
{
  const gchar *name;

  name = gtk_widget_get_name(piece);
  if (name != NULL) {
    return g_ascii_strtoll(name, NULL, 10);
  }

  return -1;
}

void set_piece_home_position(GtkWidget *piece, gint home)
{
  gchar *name;

  name = g_strdup_printf("%d", home);
  gtk_widget_set_name(piece, name);
  g_free(name);
}

gint get_piece_current_position(GtkWidget *piece)
{
  gint i;
  GtkWidget *check_piece, *grid;

  grid = gtk_widget_get_parent(piece);

  for (i = 0; i < 16; i++) {
    check_piece = gtk_grid_get_child_at(GTK_GRID(grid), i % 4, i / 4);
    if (check_piece == piece) {
      return i;
    }
  }

  return -1;
}

gint get_missing_piece_position(GtkWidget *piece)
{
  gint i;
  GtkWidget *grid, *no_piece;

  grid = gtk_widget_get_parent(piece);

  for (i = 0; i < 16; i++) {
    no_piece = gtk_grid_get_child_at(GTK_GRID(grid), i % 4, i / 4);
    if (get_piece_home_position(no_piece) == 15) {
      return i;
    }
  }

  return -1;
}

gint find_empty_neighbour_position(GtkWidget *piece)
{
  gint current_pos, empty_pos;

  current_pos = get_piece_current_position(piece);
  empty_pos = get_missing_piece_position(piece);
  
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

void exchange_pieces(GtkWidget *grid, gint piece_a_position,
                     gint piece_b_position)
{
  GtkWidget *piece_a, *piece_b;

  piece_a = gtk_grid_get_child_at(GTK_GRID(grid),
                                  piece_a_position % 4,
                                  piece_a_position / 4);
  piece_b = gtk_grid_get_child_at(GTK_GRID(grid),
                                  piece_b_position % 4,
                                  piece_b_position / 4);

  if (piece_a != NULL && piece_b != NULL) {
    g_object_ref(piece_a);
    g_object_ref(piece_b);
    gtk_container_remove(GTK_CONTAINER(grid), piece_a);
    gtk_container_remove(GTK_CONTAINER(grid), piece_b);
    gtk_grid_attach(GTK_GRID(grid), piece_a,
                    piece_b_position % 4, piece_b_position / 4, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), piece_b,
                    piece_a_position % 4, piece_a_position / 4, 1, 1);
    g_object_unref(piece_a);
    g_object_unref(piece_b);
  }
}

void move_to_empty_neighbour(GtkWidget *piece, gint neighbour_pos)
{
  gint current_position;
  GtkWidget *grid;

  current_position = get_piece_current_position(piece);
  grid = gtk_widget_get_parent(piece);
  exchange_pieces(grid, current_position, neighbour_pos);
}

gboolean pieces_are_ordered(GtkWidget *piece)
{
  gint count, current_position, home_position, i;
  GtkWidget *check_piece, *grid;

  count = 0;
  grid = gtk_widget_get_parent(piece);

  for (i = 0; i < 16; i++) {
    check_piece = gtk_grid_get_child_at(GTK_GRID(grid), i % 4, i / 4);
    home_position = get_piece_home_position(check_piece);
    current_position = get_piece_current_position(check_piece);
    if (home_position == current_position) {
      count++;
    } else {
      break;
    }
  }

  return count == 16;
}

void show_message(GtkWidget *grid, GtkMessageType type, gchar *msg)
{
  GtkWidget *message, *vbox, *window;

  vbox = gtk_widget_get_parent(grid);
  window = gtk_widget_get_parent(vbox);
  message = gtk_message_dialog_new(GTK_WINDOW(window), GTK_DIALOG_MODAL, type,
                                   GTK_BUTTONS_OK, msg);
  gtk_dialog_run(GTK_DIALOG(message));
  gtk_widget_destroy(message);
}

void game_completed(GtkWidget *piece)
{
  GtkWidget *blanked_piece, *grid, *blanked_image;

  grid = gtk_widget_get_parent(piece);
  blanked_piece = gtk_grid_get_child_at(GTK_GRID(grid), 3, 3);
  blanked_image = gtk_bin_get_child(GTK_BIN(blanked_piece));
  gtk_widget_show(blanked_piece);
  gtk_widget_show(blanked_image);
  show_message(grid, GTK_MESSAGE_INFO, "Well Done!");
  gtk_widget_hide(blanked_image);
  gtk_widget_hide(blanked_piece);
}

gboolean piece_clicked(GtkWidget *piece,
                       GdkEventButton *event, gpointer data)
{
  gint empty_neighbour_position;

  empty_neighbour_position = find_empty_neighbour_position(piece);
  if (empty_neighbour_position != -1) {
    move_to_empty_neighbour(piece, empty_neighbour_position);
  }

  if (pieces_are_ordered(piece)) {
    game_completed(piece);
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
  gint tindx;

  gdk_cairo_set_source_pixbuf(cr,
                              gtk_image_get_pixbuf(GTK_IMAGE(widget)),
                              0, 0);
  cairo_paint(cr);
  cairo_set_source_rgb(cr, 0, 0, 0);
  cairo_set_line_width(cr, 1);
  cairo_rectangle (cr, 0, 0, PIECE_WIDTH - 1, PIECE_HEIGHT - 1);
  cairo_stroke(cr);

  if (show_numbers) {
    tindx = *((gint *)data);
    draw_number(cr, tindx);
  }

  return TRUE;
}

void remove_pieces(GtkWidget *grid)
{
  GList *child, *children;

  children = child = gtk_container_get_children(GTK_CONTAINER(grid));

  while (child != NULL) {
    gtk_widget_destroy(child->data);
    child = child->next;
  }

  g_list_free(children);
}

void attach_pieces_to_grid(GtkWidget *grid, GArray *pieces)
{
  gint home, i;
  GtkWidget *event_box;

  for (i = 0; i < 16; i++) {
    event_box = g_array_index(pieces, GtkWidget *, i);
    home = get_piece_home_position(event_box);
    event_box = g_array_index(pieces, GtkWidget *, home);
    gtk_grid_attach(GTK_GRID(grid), event_box, i % 4, i / 4, 1, 1);

    if (home != 15) {
      gtk_widget_show(event_box);
      gtk_widget_show(gtk_bin_get_child(GTK_BIN(event_box)));
    }
  }
}

GtkWidget *create_piece(GtkWidget *grid, GdkPixbuf *image, gint i)
{
  GdkPixbuf *small;
  gint *tindx;
  GtkWidget *event_box, *piece_image;

  small = gdk_pixbuf_new_subpixbuf(image, (i % 4) * PIECE_WIDTH,
                                   (i / 4) * PIECE_HEIGHT,
                                   PIECE_WIDTH, PIECE_HEIGHT);
  piece_image = gtk_image_new_from_pixbuf(small);
  event_box = gtk_event_box_new();
  tindx = g_new(gint, 1);
  *tindx = i;
  g_signal_connect(piece_image, "draw", G_CALLBACK(draw), tindx);
  set_piece_home_position(event_box, i);
  gtk_container_add(GTK_CONTAINER(event_box), piece_image);
  g_signal_connect(G_OBJECT(event_box), "button_press_event",
                   G_CALLBACK(piece_clicked), grid);

  return event_box;
}

GArray *create_pieces(GtkWidget *grid, gchar *filename)
{
  GArray *pieces;
  GtkWidget *image, *piece;
  gint i;
  GdkPixbuf *pixbuf, *scaled;

  pieces = g_array_new(FALSE, FALSE, sizeof(GtkWidget *));
  image = gtk_image_new_from_file(filename);
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
    piece = create_piece(grid, scaled, i);
    g_array_append_val(pieces, piece);
  }

  return pieces;
}

gboolean is_progress_file(GIOChannel *channel)
{
  gboolean retval = FALSE;
  gchar *input;
  GIOStatus status;

  status = g_io_channel_read_line(channel, &input, NULL, NULL, NULL);

  if (status == G_IO_STATUS_NORMAL) {
    if (g_strcmp0(input, "zupedils\n") == 0) {
      retval = TRUE;
    }
  }

  g_free(input);

  return retval;
}

GArray *get_pieces(GtkWidget *grid, GIOChannel *channel)
{
  GArray *pieces;
  gsize terminator_pos;
  gchar *input;
  GIOStatus status;

  status = g_io_channel_read_line(channel, &input, NULL,
                                  &terminator_pos, NULL);

  if (status == G_IO_STATUS_NORMAL) {
    input[terminator_pos] = '\0';
    pieces = create_pieces(grid, input);
    remove_pieces(grid);
    g_free(input);

    return pieces;
  }

  return NULL;
}

gint open_progress_file_error(GtkWidget *grid,
                              GIOChannel *channel, gint status)
{
  gchar *message;

  g_io_channel_shutdown(channel, TRUE, NULL);
  g_io_channel_unref(channel);

  switch (status) {
    case 1:
      message = "Failed to open progress file.";
      break;
    case 2:
      message = "Failed to create pieces from progress file (Corrupt?).";
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
  GArray *pieces;
  gchar *input, **numstr;
  gint indx;
  gint64 home;
  GIOChannel *channel;
  GIOStatus status;
  GtkWidget *piece;

  channel = g_io_channel_new_file(filename, "r", NULL);
  if (channel == NULL) {
    return open_progress_file_error(grid, channel, 1);
  }

  if (is_progress_file(channel)) {
    pieces = get_pieces(grid, channel);
    if (pieces == NULL) {
      return open_progress_file_error(grid, channel, 2);
    }

    status = g_io_channel_read_line(channel, &input, NULL, NULL, NULL);
    if (status != G_IO_STATUS_NORMAL) {
      return open_progress_file_error(grid, channel, 3);
    }

    numstr = g_strsplit(input, " ", 16);
    if (!is_valid_array(numstr)) {
      g_free(input);
      g_strfreev(numstr);
      g_array_free(pieces, FALSE);
      return open_progress_file_error(grid, channel, 4);
    }

    for (indx = 0; indx < 16; indx++) {
      home = g_ascii_strtoll(numstr[indx], NULL, 10);
      piece = g_array_index(pieces, GtkWidget *, home);
      set_piece_home_position(piece, home);
      gtk_grid_attach(GTK_GRID(grid), piece, indx % 4, indx / 4, 1, 1);

      if (home != 15) {
        gtk_widget_show(piece);
        gtk_widget_show(gtk_bin_get_child(GTK_BIN(piece)));
      }
    }

    g_strfreev(numstr);
    g_free(input);
    g_array_free(pieces, FALSE);
  } else {
    return open_progress_file_error(grid, channel, 5);
  }

  g_io_channel_shutdown(channel, TRUE, NULL);
  g_io_channel_unref(channel);

  return 0;
}

void open_file(GtkWidget *grid, gchar *filename)
{
  GArray *pieces;
  GFile *file;
  GFileInfo *info; 
  const gchar *content_type;

  file = g_file_new_for_path(filename);
  info = g_file_query_info(file, "standard::*",
                           G_FILE_QUERY_INFO_NONE, NULL, NULL);
  content_type = g_file_info_get_attribute_string(info,
                 G_FILE_ATTRIBUTE_STANDARD_CONTENT_TYPE);
  if (g_ascii_strncasecmp(content_type, "image/", 6) == 0) {
    pieces = create_pieces(grid, filename);
    remove_pieces(grid);
    attach_pieces_to_grid(grid, pieces);
    g_array_free(pieces, FALSE);
  } else if (g_strcmp0(content_type, "text/plain") == 0) {
    open_progress_file(grid, filename);
  } else {
    show_message(grid, GTK_MESSAGE_WARNING, "Unknown file type.");
  }

  g_object_unref(info);
  g_object_unref(file);
}

gchar *get_filename(GtkWidget *grid, gchar *title,
                    gint action, gchar *type)
{
  gchar *filename = NULL;
  GtkWidget *dialogue, *vbox;
  GtkWindow *window;
  gint status;

  vbox = gtk_widget_get_parent(grid);
  window = GTK_WINDOW(gtk_widget_get_parent(vbox));
  dialogue = gtk_file_chooser_dialog_new(title, window, action, "Cancel",
                                         GTK_RESPONSE_CANCEL, type,
                                         GTK_RESPONSE_ACCEPT, NULL);
  gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialogue),
                                                 TRUE);
  status = gtk_dialog_run(GTK_DIALOG(dialogue));
  if (status == GTK_RESPONSE_ACCEPT) {
    filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialogue));
  }

  gtk_widget_destroy(dialogue);
  return filename;
}

void open_(GtkWidget *grid)
{
  gchar *filename;

  filename = get_filename(grid, "Choose Image / Progress file",
                          GTK_FILE_CHOOSER_ACTION_OPEN, "Open");
  if (filename != NULL) {
    open_file(grid, filename);
    g_free(filename);
  }
}

GIOChannel *get_save_channel(GtkWidget *grid)
{
  GIOChannel *channel;
  gchar *filename;

  filename = get_filename(grid, "Save Progress",
                          GTK_FILE_CHOOSER_ACTION_SAVE, "Save");
  if (filename != NULL) {
    channel = g_io_channel_new_file(filename, "w", NULL);
    if (!channel) {
      show_message(grid, GTK_MESSAGE_WARNING,
                   "Couldn't save! Is the location writable?");
    }
    g_free(filename);
    return channel;
  }

  return NULL;
}

void save(GtkWidget *grid)
{
  gchar *data;
  gint home, i;
  const gchar *image_path;
  GIOChannel *channel;
  GtkWidget *widget;

  if (gtk_grid_get_child_at(GTK_GRID(grid), 0, 0) == NULL) {
    return;
  }

  channel = get_save_channel(grid);
  if (channel == NULL) {
    return;
  }

  image_path = gtk_widget_get_name(grid);
  data = g_strdup_printf("zupedils\n%s\n", image_path);
  g_io_channel_write_chars(channel, data, -1, NULL, NULL);
  g_free(data);

  for (i = 0; i < 16; i++) {
    widget = gtk_grid_get_child_at(GTK_GRID(grid), i % 4, i / 4);
    home = get_piece_home_position(widget);
    data = g_strdup_printf("%d ", home);
    g_io_channel_write_chars(channel, data, -1, NULL, NULL);
    g_free(data);
  }

  g_io_channel_shutdown(channel, TRUE, NULL);
  g_io_channel_unref(channel);
}

void quit(GtkWidget *grid)
{
  gtk_main_quit();
}

void shuffle_pieces(GtkWidget *grid)
{
  gint current, i, new;
  GRand *rand_;
  GtkWidget *piece;

  if (gtk_grid_get_child_at(GTK_GRID(grid), 0, 0) == NULL) {
    return;
  }

  rand_ = g_rand_new();

  for (i = 0; i < 16; i++) {
    piece = gtk_grid_get_child_at(GTK_GRID(grid), i % 4, i / 4);
    current = get_piece_current_position(piece);
    while ((new = g_rand_int_range(rand_, 0, 15)) == current);
    exchange_pieces(grid, current, new);
  }

  g_rand_free(rand_);
}

void reset_pieces(GtkWidget *grid)
{
  gint current_pos, home_pos, i;
  GtkWidget *piece, *piece_needs_moving[16];

  if (gtk_grid_get_child_at(GTK_GRID(grid), 0, 0) == NULL) {
    return;
  }

  for (i = 0; i < 16; i++) {
    piece = gtk_grid_get_child_at(GTK_GRID(grid), i % 4, i / 4);
    home_pos = get_piece_home_position(piece);
    current_pos = get_piece_current_position(piece);
    if (home_pos != current_pos) {
      piece_needs_moving[i] = piece;
    } else {
      piece_needs_moving[i] = NULL;
    }
  }

  for (i = 0; i < 16; i++) {
    if (piece_needs_moving[i]) {
      home_pos = get_piece_home_position(piece_needs_moving[i]);
      current_pos = get_piece_current_position(piece_needs_moving[i]);
      if (home_pos != current_pos) {
        exchange_pieces(grid, current_pos, home_pos);
      }
    }
  }
}

void create_menu_item(GtkWidget *grid, gchar *action,
                      GtkWidget *menu, GCallback cb)
{
  GtkWidget *menu_item;

  menu_item = gtk_menu_item_new_with_label(action);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);
  g_signal_connect_swapped(G_OBJECT(menu_item), "activate",
                           G_CALLBACK(cb), grid);
}

void help(GtkWidget *grid)
{
  gchar *msg;

  msg = "File - Open opens an image file or a saved progress file.\n"
        "File - Save Progress creates a saved progress file.\n\n"
        "Click on a piece beside an empty space to move the piece.";

  show_message(grid, GTK_MESSAGE_INFO, msg);
}

void about(GtkWidget *grid)
{
  gchar *msg;

  msg = "Zupedils\nA sliding puzzle game.\n"
        "Copyright © 2018 Craig McPartland";

  show_message(grid, GTK_MESSAGE_INFO, msg);
}

void toggle_numbers(GtkWidget *grid)
{
  GtkWidget *widget;

  widget = gtk_grid_get_child_at(GTK_GRID(grid), 0, 0);
  if (widget != NULL) {
    show_numbers = !show_numbers;
    gtk_widget_queue_draw(grid);
  }
}

void create_help_menu_items(GtkWidget *grid, GtkWidget *menu)
{
  create_menu_item(grid, "Help", menu, G_CALLBACK(help));
  create_menu_item(grid, "About", menu, G_CALLBACK(about));
}

void create_file_menu_items(GtkWidget *grid, GtkWidget *menu)
{
  create_menu_item(grid, "Open", menu, G_CALLBACK(open_));
  create_menu_item(grid, "Save Progress",
                   menu, G_CALLBACK(save));
  create_menu_item(grid, "Quit", menu, G_CALLBACK(quit));
}

void create_options_menu_items(GtkWidget *grid, GtkWidget *menu)
{
  create_menu_item(grid, "Shuffle", menu, G_CALLBACK(shuffle_pieces));
  create_menu_item(grid, "Show Numbers",
                   menu, G_CALLBACK(toggle_numbers));
  create_menu_item(grid, "Reset", menu, G_CALLBACK(reset_pieces));
}

GtkWidget *create_menu_bar(GtkWidget *grid)
{
  GtkWidget *item, *menu, *menu_bar;

  menu_bar = gtk_menu_bar_new();
  gtk_widget_set_hexpand(menu_bar, TRUE);

  menu = gtk_menu_new();
  create_file_menu_items(grid, menu);
  item = gtk_menu_item_new_with_label("File");
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(item), menu);
  gtk_container_add(GTK_CONTAINER(menu_bar), item);

  menu = gtk_menu_new();
  create_options_menu_items(grid, menu);
  item = gtk_menu_item_new_with_label("Options");
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(item), menu);
  gtk_container_add(GTK_CONTAINER(menu_bar), item);

  menu = gtk_menu_new();
  create_help_menu_items(grid, menu);
  item = gtk_menu_item_new_with_label("Help");
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(item), menu);
  gtk_container_add(GTK_CONTAINER(menu_bar), item);

  return menu_bar;
}

GtkWidget *create_main_window(gint width, gint height)
{
  GdkPixbuf *icon;
  GtkWidget *window;

  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_default_size(GTK_WINDOW(window), width, height);
  gtk_window_set_resizable(GTK_WINDOW(window), FALSE);
  g_signal_connect(G_OBJECT(window), "delete_event",
                   G_CALLBACK(gtk_main_quit), NULL);
  icon = gdk_pixbuf_new_from_resource("/zupedils/icon.png", NULL);
  if (icon) {
    gtk_window_set_icon(GTK_WINDOW(window), icon);
    g_object_unref(icon);
  }

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
  gtk_box_pack_start(GTK_BOX(vbox), menu_bar, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(vbox), grid, TRUE, TRUE, 0);
  gtk_container_add(GTK_CONTAINER(window), vbox);
  gtk_widget_show_all(window);
  gtk_main();

  return 0;
}

