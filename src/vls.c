/* 
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "vls_rtsp.h"

#include <glib.h>
#include <glib-unix.h>

#define DEFAUT_RTSP_PORT 8554
#define DEFAUT_LISTENING_ADDRESS "127.0.0.1"

static gboolean
vls_is_supported (gchar *path)
{
	gboolean supported_file = FALSE;
	gchar *basename, *ext;

	basename = g_path_get_basename (path);
	ext = g_strrstr (basename, ".");

	if (g_ascii_strcasecmp (ext, ".mp4") == 0)
		supported_file = TRUE;
	if (g_ascii_strcasecmp (ext, ".mov") == 0)
		supported_file = TRUE;
	if (g_ascii_strcasecmp (ext, ".m4v") == 0)
		supported_file = TRUE;

	g_free (basename);
	return supported_file;
}

static GSList *
vls_media_list_from_directory (const gchar * dir_path, GSList * media_uri_list)
{
	GFile *dir;
	GFileEnumerator *direnum;
	GError *error = NULL;

	dir = g_file_new_for_path (dir_path);
	direnum = g_file_enumerate_children (dir, NULL, 0, NULL, &error);
	g_object_unref (dir);

	if (direnum == NULL) {
		g_critical ("failed to enumerate %s: %s", dir_path, error->message);
		goto beach;
	}

	while (TRUE)
	{
		GFileInfo *info;
		GFile *file;
		gchar *uri = NULL;
		gchar *path = NULL;

		if (!g_file_enumerator_iterate (direnum, &info, &file, NULL, &error)) {
			g_clear_error (&error);
			g_critical ("failed enumerating: %s", error->message);
			continue;
		}

		// no more file
		if (!info)
			break;

		path = g_file_get_path (file);

		if (!g_file_test (path, G_FILE_TEST_IS_REGULAR))
			continue;

		if (!vls_is_supported (path)) {
			g_message ("%s not supported", path);
			continue;
		}

		uri = gst_filename_to_uri (path, NULL);
		media_uri_list = g_slist_append (media_uri_list, uri);
		g_free (path);
	}


beach:
	g_object_unref (direnum);

	return media_uri_list;
}

static void
vls_show_resource (GQuark key_id, gpointer data, gpointer user_data)
{
	gchar *type = user_data;
	gchar *uri = data;

	g_message ("%s stream ready at %s", type, uri);
}

static gboolean
intr_handler (gpointer user_data)
{
	GMainLoop *loop = (GMainLoop *) user_data;

	g_message ("handling interrupt");
    g_main_loop_quit (loop);

	return FALSE;
}

int
main (int argc, char *argv[])
{
  GMainLoop *loop;
  GOptionContext *optctx;
  GError *error = NULL;
  gboolean shared = FALSE;
  gint rtsp_listening_port = DEFAUT_RTSP_PORT;
  gchar *listening_address = g_strdup (DEFAUT_LISTENING_ADDRESS);
  GSList *media_uri_list = NULL;
  GOptionEntry options[] = {
    {"shared", 's', 0, G_OPTION_ARG_NONE, &shared,
        "Share video pipline between different client", NULL},
    {"rtsp_port", 0, 0, G_OPTION_ARG_INT, &rtsp_listening_port,
        "RTSP port (default " G_STRINGIFY (DEFAUT_RTSP_PORT) ")", NULL},
    {"listening_address", 0, 0, G_OPTION_ARG_STRING, &listening_address,
        "listening address (default " DEFAUT_LISTENING_ADDRESS ")", NULL},
    {NULL}
  };
  GstRTSPServer *rtsp_server;
  GData *datalist;
  int ret = EXIT_FAILURE;

  optctx = g_option_context_new ("Ubicast Video Server");
  g_option_context_add_main_entries (optctx, options, NULL);
  g_option_context_add_group (optctx, gst_init_get_option_group ());
  if (!g_option_context_parse (optctx, &argc, &argv, &error)) {
	  g_critical ("Error parsing options: %s", error->message);
	  g_printerr ("%s", g_option_context_get_help (optctx, TRUE, NULL));
	  g_option_context_free (optctx);
	  g_clear_error (&error);
	  return EXIT_FAILURE;
  }

  // check arguments
  if (argc < 2) {
	  g_critical ("missing arguments");
	  g_printerr ("%s", g_option_context_get_help (optctx, TRUE, NULL));
	  goto bad_arg;
  }

  g_option_context_free (optctx);

  /* check argument, can be an URI, a media file path or a directory */
  if (gst_uri_is_valid (argv[1])) {
	  media_uri_list = g_slist_append (media_uri_list, g_strdup (argv[1]));
  } else if (g_file_test (argv[1], G_FILE_TEST_IS_REGULAR) && vls_is_supported (argv[1])) {
	  media_uri_list = g_slist_append (media_uri_list, gst_filename_to_uri (argv[1], NULL));
  } else if (g_file_test (argv[1], G_FILE_TEST_IS_DIR)) {
	  media_uri_list = vls_media_list_from_directory (argv[1], media_uri_list);
  } else {
	  g_critical ("Unrecognised command line argument '%s'.\n"
			  "Please pass an URI, file or directory as argument!", argv[1]);
	  return -1;
  }

  g_message ("serving %d media", g_slist_length (media_uri_list));

  loop = g_main_loop_new (NULL, FALSE);

  // serve RTSP
  rtsp_server = vls_rtsp_serve (media_uri_list, shared, rtsp_listening_port,
		  listening_address);
  if (!rtsp_server) {
	  g_warning ("failed to start RTSP server, check --gst-debug=3 for the cause");
	  goto beach;
  }
  datalist = g_object_get_data (G_OBJECT (rtsp_server), "server_mapping");
  g_datalist_foreach (&datalist, vls_show_resource, "RTSP");

  // install signal handler
  g_unix_signal_add (SIGINT, (GSourceFunc) intr_handler, loop);
  g_unix_signal_add (SIGTERM, (GSourceFunc) intr_handler, loop);

  g_main_loop_run (loop);

  g_message ("bye");
  ret = EXIT_SUCCESS;

beach:
  g_clear_object (&rtsp_server);
  g_main_loop_unref (loop);
  g_slist_free (media_uri_list);
bad_arg:
  g_free (listening_address);
  gst_deinit ();

  return ret;
}
