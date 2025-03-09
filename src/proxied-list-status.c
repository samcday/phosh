/*
 * Copyright (C) 2025 The Phosh Developers
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#define G_LOG_DOMAIN "phosh-proxied-list-status"

#include "proxied-list-status.h"

#include <handy.h>

#include "dbus/qs-list-status-dbus.h"
#include "util.h"

/**
 * PhoshProxiedListStatus:
 *
 * TODO:
 */

enum {
  PROP_0,
  PROP_CONNECTION,
  PROP_BUS_NAME,
  PROP_OBJECT_PATH,
  PROP_LAST_PROP,
};
static GParamSpec *props[PROP_LAST_PROP];

struct _PhoshProxiedListStatus {
  GtkListBox parent;

  GDBusConnection *connection;
  char            *bus_name;
  char            *object_path;

  PhoshDBusListStatus *proxy;
};

G_DEFINE_TYPE (PhoshProxiedListStatus, phosh_proxied_list_status,
               GTK_TYPE_LIST_BOX);


static void
phosh_proxied_list_status_set_property (GObject      *object,
                                       guint         property_id,
                                       const GValue *value,
                                       GParamSpec   *pspec)
{
  PhoshProxiedListStatus *self = PHOSH_PROXIED_LIST_STATUS (object);

  switch (property_id) {
  case PROP_CONNECTION:
    g_set_object (&self->connection, g_value_get_object (value));
    break;
  case PROP_BUS_NAME:
    g_clear_pointer (&self->bus_name, g_free);
    self->bus_name = g_value_dup_string (value);
    break;
  case PROP_OBJECT_PATH:
    g_clear_pointer (&self->object_path, g_free);
    self->object_path = g_value_dup_string (value);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}


static void
phosh_proxied_list_status_get_property (GObject   *object,
                                       guint       property_id,
                                       GValue     *value,
                                       GParamSpec *pspec)
{
  PhoshProxiedListStatus *self = PHOSH_PROXIED_LIST_STATUS (object);

  switch (property_id) {
  case PROP_CONNECTION:
    g_value_set_object (value, self->connection);
    break;
  case PROP_BUS_NAME:
    g_value_set_string (value, self->bus_name);
    break;
  case PROP_OBJECT_PATH:
    g_value_set_string (value, self->object_path);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}


/*
static void
activated_cb (GObject *source_object,
           GAsyncResult *res,
           PhoshProxiedListStatus *self)
{
  g_autoptr (GError) err = NULL;

  phosh_dbus_placeholder_status_call_activated_finish (self->proxy, res, &err);
  if (err != NULL)
    phosh_async_error_warn (err, "Failed to call Activate");
}


static void
on_clicked (PhoshProxiedListStatus *self)
{
  phosh_dbus_placeholder_status_call_activated (self->proxy, NULL,
                                                (GAsyncReadyCallback)activated_cb, self);
}
*/


static void
update_items (PhoshProxiedListStatus *self)
{
  GVariant *items;
  GVariantIter iter;
  const char *icon, *title;
  gboolean spinner, active;
  GtkWidget *row, *label;

  gtk_container_foreach (GTK_CONTAINER (self), (GtkCallback) gtk_container_remove, NULL);

  items = phosh_dbus_list_status_get_items (self->proxy);
  g_return_if_fail (items != NULL);

  g_variant_iter_init (&iter, items);

  while (g_variant_iter_next (&iter, "(&s&sbb)", &icon, &title, &spinner, &active)) {
    row = hdy_action_row_new ();
    // label = gtk_label_new (label_text);
    // gtk_container_add (GTK_CONTAINER (row), label);
    // gtk_widget_show (label);
    hdy_action_row_set_icon_name (HDY_ACTION_ROW (row), icon);
    hdy_preferences_row_set_title (HDY_PREFERENCES_ROW (row), title);
    gtk_container_add (GTK_CONTAINER (self), row);
    gtk_widget_show (row);
  }
}


static void
dbus_init_cb (GObject *source_object, GAsyncResult *res, PhoshProxiedListStatus *self)
{
  g_autoptr (GError) err = NULL;
  GtkWidget *button;

  self->proxy = phosh_dbus_list_status_proxy_new_finish (res, &err);
  if (self->proxy == NULL) {
    phosh_async_error_warn (err, "Couldn't connect to ListStatus service");
    return;
  }

  g_signal_connect_object (self->proxy, "notify::items", G_CALLBACK (update_items), self, G_CONNECT_SWAPPED);
  update_items (self);

  gtk_widget_set_visible (GTK_WIDGET (self), TRUE);
}


static void
phosh_proxied_list_status_constructed (GObject *object)
{
  PhoshProxiedListStatus *self = PHOSH_PROXIED_LIST_STATUS (object);

  if (!self->connection || !self->bus_name || !self->object_path)
    return;

  phosh_dbus_list_status_proxy_new (self->connection,
                                           G_DBUS_PROXY_FLAGS_NONE,
                                           self->bus_name,
                                           self->object_path,
                                           NULL,
                                           (GAsyncReadyCallback)dbus_init_cb,
                                           self);
}


static void
phosh_proxied_list_status_dispose (GObject *object)
{
  PhoshProxiedListStatus *self = PHOSH_PROXIED_LIST_STATUS (object);

  g_clear_object (&self->proxy);
  g_clear_object (&self->connection);
  g_clear_pointer (&self->bus_name, g_free);
  g_clear_pointer (&self->object_path, g_free);

  G_OBJECT_CLASS (phosh_proxied_list_status_parent_class)->dispose (object);
}


static void
phosh_proxied_list_status_class_init (PhoshProxiedListStatusClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->constructed = phosh_proxied_list_status_constructed;
  object_class->dispose = phosh_proxied_list_status_dispose;
  object_class->get_property = phosh_proxied_list_status_get_property;
  object_class->set_property = phosh_proxied_list_status_set_property;

  /**
   * PhoshProxiedListStatus:connection:
   *
   * The DBus connection this placeholder should connect via
   */
  props[PROP_CONNECTION] =
    g_param_spec_object ("connection", "", "",
                         G_TYPE_DBUS_CONNECTION,
                         G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

  /**
   * PhoshProxiedListStatus:bus-name:
   *
   * The bus name of the remote object to bind this placeholder to
   */
  props[PROP_BUS_NAME] =
    g_param_spec_string ("bus-name", "", "",
                         "",
                         G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

  /**
   * PhoshProxiedListStatus:object-path:
   *
   * The path of the remote bus object to bind this placeholder to
   */
  props[PROP_OBJECT_PATH] =
    g_param_spec_string ("object-path", "", "",
                         "",
                         G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, PROP_LAST_PROP, props);
}


static void
phosh_proxied_list_status_init (PhoshProxiedListStatus *self)
{
  gtk_widget_set_hexpand (GTK_WIDGET (self), TRUE);
}


PhoshProxiedListStatus*
phosh_proxied_list_status_new (GDBusConnection *connection, const char *bus_name,
                                 const char *object_path)
{
  return g_object_new (PHOSH_TYPE_PROXIED_LIST_STATUS,
                       "connection", connection,
                       "bus-name", bus_name,
                       "object-path", object_path,
                       NULL
                      );
}
