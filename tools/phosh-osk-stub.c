/*
 * Copyright (C) 2018 Purism SPC
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Author: Guido Günther <agx@sigxcpu.org>
 */

#define G_LOG_DOMAIN "phosh-osk-stub"

#include "config.h"
#include "layersurface.h"

#include "input-method-unstable-v2-client-protocol.h"

#include <gio/gio.h>
#include <glib-unix.h>

#include <gtk/gtk.h>
#include <gdk/gdkwayland.h>

#define GNOME_SESSION_DBUS_NAME      "org.gnome.SessionManager"
#define GNOME_SESSION_DBUS_OBJECT    "/org/gnome/SessionManager"
#define GNOME_SESSION_DBUS_INTERFACE "org.gnome.SessionManager"
#define GNOME_SESSION_CLIENT_PRIVATE_DBUS_INTERFACE "org.gnome.SessionManager.ClientPrivate"

static GMainLoop *loop;
static GDBusProxy *_proxy;

static GtkWindow *_input_surface;

static struct wl_display *_display;
static struct wl_registry *_registry;
static struct wl_seat *_seat;
static struct zwlr_layer_shell_v1 *_layer_shell;
static struct zwp_input_method_manager_v2 *_input_method_manager;
static struct zwp_input_method_v2 *_input_method;

/* TODO:
   - handle sm.puri.OSK0
   - launch squeekboard if found and fold/unfold on desktops is fixed
   https://source.puri.sm/Librem5/squeekboard/issues/132
 */

static void
print_version (void)
{
  g_message ("OSK stub %s\n", PHOSH_VERSION);
  exit (0);
}


static gboolean
quit_cb (gpointer user_data)
{
  g_info ("Caught signal, shutting down...");

  if (loop)
    g_idle_add ((GSourceFunc) g_main_loop_quit, loop);
  else
    exit (0);

  return FALSE;
}


static void
respond_to_end_session (GDBusProxy *proxy, gboolean shutdown)
{
  /* we must answer with "EndSessionResponse" */
  g_dbus_proxy_call (proxy, "EndSessionResponse",
                     g_variant_new ("(bs)", TRUE, ""),
                     G_DBUS_CALL_FLAGS_NONE,
                     -1, NULL, NULL, NULL);
}


static void
client_proxy_signal_cb (GDBusProxy *proxy,
                        char       *sender_name,
                        char       *signal_name,
                        GVariant   *parameters,
                        gpointer    user_data)
{
  if (g_strcmp0 (signal_name, "QueryEndSession") == 0) {
    g_debug ("Got QueryEndSession signal");
    respond_to_end_session (proxy, FALSE);
  } else if (g_strcmp0 (signal_name, "EndSession") == 0) {
    g_debug ("Got EndSession signal");
    respond_to_end_session (proxy, TRUE);
  } else if (g_strcmp0 (signal_name, "Stop") == 0) {
    g_debug ("Got Stop signal");
    quit_cb (NULL);
  }
}


static void
on_client_registered (GObject      *source_object,
                      GAsyncResult *res,
                      gpointer      user_data)
{
  GVariant *variant;
  GDBusProxy *client_proxy;
  GError *error = NULL;
  char *object_path = NULL;

  variant = g_dbus_proxy_call_finish (G_DBUS_PROXY (source_object), res, &error);
  if (!variant) {
    g_warning ("Unable to register client: %s", error->message);
    g_error_free (error);
    return;
  }

  g_variant_get (variant, "(o)", &object_path);

  g_debug ("Registered client at path %s", object_path);

  client_proxy = g_dbus_proxy_new_for_bus_sync (G_BUS_TYPE_SESSION, 0, NULL,
                                                GNOME_SESSION_DBUS_NAME,
                                                object_path,
                                                GNOME_SESSION_CLIENT_PRIVATE_DBUS_INTERFACE,
                                                NULL,
                                                &error);
  if (!client_proxy) {
    g_warning ("Unable to get the session client proxy: %s", error->message);
    g_error_free (error);
    return;
  }

  g_signal_connect (client_proxy, "g-signal",
                    G_CALLBACK (client_proxy_signal_cb), NULL);

  g_free (object_path);
  g_variant_unref (variant);
}


static void
stub_session_register (const char *client_id)
{
  const char *startup_id;
  GError *err = NULL;

  if (!_proxy) {
    _proxy = g_dbus_proxy_new_for_bus_sync (G_BUS_TYPE_SESSION,
                                            G_DBUS_PROXY_FLAGS_DO_NOT_LOAD_PROPERTIES |
                                            G_DBUS_PROXY_FLAGS_DO_NOT_AUTO_START_AT_CONSTRUCTION,
                                            NULL,
                                            GNOME_SESSION_DBUS_NAME,
                                            GNOME_SESSION_DBUS_OBJECT,
                                            GNOME_SESSION_DBUS_INTERFACE,
                                            NULL,
                                            &err);
    if (!_proxy) {
      g_debug ("Failed to contact gnome-session: %s", err->message);
      g_clear_error (&err);
      return;
    }
  }

  startup_id = g_getenv ("DESKTOP_AUTOSTART_ID");
  g_dbus_proxy_call (_proxy,
                     "RegisterClient",
                     g_variant_new ("(ss)", client_id, startup_id ? startup_id : ""),
                     G_DBUS_CALL_FLAGS_NONE,
                     -1,
                     NULL,
                     (GAsyncReadyCallback) on_client_registered,
                     NULL);
}


static void
handle_activate (void                       *data,
                 struct zwp_input_method_v2 *zwp_input_method_v2)
{
  g_debug ("%s", __func__);
  gtk_window_present (_input_surface);
}


static void
handle_deactivate (void                       *data,
                   struct zwp_input_method_v2 *zwp_input_method_v2)
{
  g_debug ("%s", __func__);
  gtk_widget_hide (GTK_WIDGET (_input_surface));
}


static void
handle_surrounding_text (void                       *data,
                         struct zwp_input_method_v2 *zwp_input_method_v2,
                         const char                 *text,
                         uint32_t                    cursor,
                         uint32_t                    anchor)
{
  g_debug ("%s: text: %s", __func__, text);
}


static void
handle_text_change_cause (void                       *data,
                          struct zwp_input_method_v2 *zwp_input_method_v2,
                          uint32_t                    cause)
{
  g_debug ("%s: cause: %u", __func__, cause);
}


static void
handle_content_type (void                       *data,
                     struct zwp_input_method_v2 *zwp_input_method_v2,
                     uint32_t                    hint,
                     uint32_t                    purpose)
{
  g_debug ("%s, hint: %d, purpose: %d", __func__, hint, purpose);
}


static void
handle_done (void                       *data,
             struct zwp_input_method_v2 *zwp_input_method_v2)
{
  g_debug ("%s", __func__);
}


static void
handle_unavailable (void                       *data,
                    struct zwp_input_method_v2 *zwp_input_method_v2)
{
  g_debug ("%s", __func__);
}


static const struct zwp_input_method_v2_listener input_method_listener = {
  .activate = handle_activate,
  .deactivate = handle_deactivate,
  .surrounding_text = handle_surrounding_text,
  .text_change_cause = handle_text_change_cause,
  .content_type = handle_content_type,
  .done = handle_done,
  .unavailable = handle_unavailable,
};


#define INPUT_SURFACE_HEIGHT 100

static void
create_input_surface (void)
{
  _input_surface = g_object_new (PHOSH_TYPE_LAYER_SURFACE,
                                 "layer-shell", _layer_shell,
                                 "height", INPUT_SURFACE_HEIGHT,
                                 "anchor", ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM |
                                 ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT |
                                 ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT,
                                 "layer", ZWLR_LAYER_SHELL_V1_LAYER_TOP,
                                 "kbd-interactivity", FALSE,
                                 "exclusive-zone", INPUT_SURFACE_HEIGHT,
                                 "namespace", "osk",
                                 NULL);
}


static void
registry_handle_global (void               *data,
                        struct wl_registry *registry,
                        uint32_t            name,
                        const char         *interface,
                        uint32_t            version)
{
  if (strcmp (interface, zwp_input_method_manager_v2_interface.name) == 0) {
    _input_method_manager = wl_registry_bind (registry, name,
                                              &zwp_input_method_manager_v2_interface, 1);
  } else if (strcmp (interface, wl_seat_interface.name) == 0) {
    _seat = wl_registry_bind (registry, name, &wl_seat_interface, version);
  } else if (!strcmp (interface, zwlr_layer_shell_v1_interface.name)) {
    _layer_shell = wl_registry_bind (_registry, name, &zwlr_layer_shell_v1_interface, 1);
  }

  if (_seat && _input_method_manager && _layer_shell && !_input_surface) {
    create_input_surface ();
    _input_method = zwp_input_method_manager_v2_get_input_method (_input_method_manager, _seat);
    zwp_input_method_v2_add_listener (_input_method, &input_method_listener, NULL);
  }
}


static void
registry_handle_global_remove (void               *data,
                               struct wl_registry *registry,
                               uint32_t            name)
{
  g_warning ("Global %d removed but not handled", name);
}


static const struct wl_registry_listener registry_listener = {
  registry_handle_global,
  registry_handle_global_remove
};


static gboolean
setup_input_method (void)
{
  GdkDisplay *gdk_display;

  gdk_set_allowed_backends ("wayland");
  gdk_display = gdk_display_get_default ();
  _display = gdk_wayland_display_get_wl_display (gdk_display);
  if (_display == NULL) {
    g_critical ("Failed to get display: %m\n");
    return FALSE;
  }

  _registry = wl_display_get_registry (_display);
  wl_registry_add_listener (_registry, &registry_listener, NULL);
  return TRUE;
}


int
main (int argc, char *argv[])
{
  g_autoptr (GOptionContext) opt_context = NULL;
  GError *err = NULL;
  gboolean version = FALSE;

  const GOptionEntry options [] = {
    {"version", 0, 0, G_OPTION_ARG_NONE, &version,
     "Show version information", NULL},
    { NULL, 0, 0, G_OPTION_ARG_NONE, NULL, NULL, NULL }
  };

  opt_context = g_option_context_new ("- A OSK stub for phosh");
  g_option_context_add_main_entries (opt_context, options, NULL);
  if (!g_option_context_parse (opt_context, &argc, &argv, &err)) {
    g_warning ("%s", err->message);
    g_clear_error (&err);
    return EXIT_FAILURE;
  }

  if (version) {
    print_version ();
  }

  gtk_init (&argc, &argv);

  stub_session_register ("sm.puri.OSK0");
  if (!setup_input_method ())
    return EXIT_FAILURE;

  loop = g_main_loop_new (NULL, FALSE);

  g_unix_signal_add (SIGTERM, quit_cb, NULL);
  g_unix_signal_add (SIGINT, quit_cb, NULL);

  g_main_loop_run (loop);
  g_main_loop_unref (loop);
  g_object_unref (_proxy);
  gtk_widget_destroy (GTK_WIDGET (_input_surface));

  return EXIT_SUCCESS;
}
