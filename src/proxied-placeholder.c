/*
 * Copyright (C) 2025 The Phosh Developers
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#define G_LOG_DOMAIN "phosh-proxied-placeholder"

#include "proxied-placeholder.h"

#include "dbus/qs-placeholder-status-dbus.h"
#include "util.h"

/**
 * PhoshProxiedPlaceholder:
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

struct _PhoshProxiedPlaceholder {
  PhoshStatusPagePlaceholder parent;

  GDBusConnection *connection;
  char            *bus_name;
  char            *object_path;

  PhoshDBusPlaceholderStatus *proxy;
};

G_DEFINE_TYPE (PhoshProxiedPlaceholder, phosh_proxied_placeholder,
               PHOSH_TYPE_STATUS_PAGE_PLACEHOLDER);


static void
phosh_proxied_placeholder_set_property (GObject      *object,
                                       guint         property_id,
                                       const GValue *value,
                                       GParamSpec   *pspec)
{
  PhoshProxiedPlaceholder *self = PHOSH_PROXIED_PLACEHOLDER (object);

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
phosh_proxied_placeholder_get_property (GObject   *object,
                                       guint       property_id,
                                       GValue     *value,
                                       GParamSpec *pspec)
{
  PhoshProxiedPlaceholder *self = PHOSH_PROXIED_PLACEHOLDER (object);

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


static gboolean
transform_visibility (GBinding *binding, const GValue *from_value, GValue *to_value, gpointer data)
{
  const char *value = g_value_get_string (from_value);
  g_value_set_boolean (to_value, value != NULL && strlen(value) > 0);
  return TRUE;
}


static void
activated_cb (GObject *source_object,
           GAsyncResult *res,
           PhoshProxiedPlaceholder *self)
{
  g_autoptr (GError) err = NULL;

  phosh_dbus_placeholder_status_call_activated_finish (self->proxy, res, &err);
  if (err != NULL)
    phosh_async_error_warn (err, "Failed to call Activate");
}


static void
on_clicked (PhoshProxiedPlaceholder *self)
{
  phosh_dbus_placeholder_status_call_activated (self->proxy, NULL,
                                                (GAsyncReadyCallback)activated_cb, self);
}


static void
dbus_init_cb (GObject *source_object, GAsyncResult *res, PhoshProxiedPlaceholder *self)
{
  g_autoptr (GError) err = NULL;
  GtkWidget *button;

  self->proxy = phosh_dbus_placeholder_status_proxy_new_finish (res, &err);
  if (self->proxy == NULL) {
    phosh_async_error_warn (err, "Couldn't connect to PlaceholderStatus service");
    return;
  }

  g_object_bind_property (self->proxy, "title", self, "title", G_BINDING_SYNC_CREATE);
  g_object_bind_property (self->proxy, "icon-name", self, "icon-name", G_BINDING_SYNC_CREATE);
  g_object_bind_property_full (self->proxy, "title", self, "visible",
                               G_BINDING_SYNC_CREATE, transform_visibility, NULL, self, NULL);

  button = gtk_button_new ();
  gtk_widget_set_halign (button, GTK_ALIGN_CENTER);
  g_object_bind_property (self->proxy, "button-label", button, "label", G_BINDING_SYNC_CREATE);
  g_object_bind_property_full (self->proxy, "button-label", button, "visible",
                               G_BINDING_SYNC_CREATE, transform_visibility, NULL, self, NULL);
  g_signal_connect_object (button, "clicked", G_CALLBACK (on_clicked), self, G_CONNECT_SWAPPED);
  phosh_status_page_placeholder_set_extra_widget (PHOSH_STATUS_PAGE_PLACEHOLDER (self), button);

  gtk_widget_set_visible (GTK_WIDGET (self), TRUE);
}


static void
phosh_proxied_placeholder_constructed (GObject *object)
{
  PhoshProxiedPlaceholder *self = PHOSH_PROXIED_PLACEHOLDER (object);

  if (!self->connection || !self->bus_name || !self->object_path)
    return;

  phosh_dbus_placeholder_status_proxy_new (self->connection,
                                           G_DBUS_PROXY_FLAGS_NONE,
                                           self->bus_name,
                                           self->object_path,
                                           NULL,
                                           (GAsyncReadyCallback)dbus_init_cb,
                                           self);
}


static void
phosh_proxied_placeholder_dispose (GObject *object)
{
  PhoshProxiedPlaceholder *self = PHOSH_PROXIED_PLACEHOLDER (object);

  g_clear_object (&self->proxy);
  g_clear_object (&self->connection);
  g_clear_pointer (&self->bus_name, g_free);
  g_clear_pointer (&self->object_path, g_free);

  G_OBJECT_CLASS (phosh_proxied_placeholder_parent_class)->dispose (object);
}


static void
phosh_proxied_placeholder_class_init (PhoshProxiedPlaceholderClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->constructed = phosh_proxied_placeholder_constructed;
  object_class->dispose = phosh_proxied_placeholder_dispose;
  object_class->get_property = phosh_proxied_placeholder_get_property;
  object_class->set_property = phosh_proxied_placeholder_set_property;

  /**
   * PhoshProxiedPlaceholder:connection:
   *
   * The DBus connection this placeholder should connect via
   */
  props[PROP_CONNECTION] =
    g_param_spec_object ("connection", "", "",
                         G_TYPE_DBUS_CONNECTION,
                         G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

  /**
   * PhoshProxiedPlaceholder:bus-name:
   *
   * The bus name of the remote object to bind this placeholder to
   */
  props[PROP_BUS_NAME] =
    g_param_spec_string ("bus-name", "", "",
                         "",
                         G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

  /**
   * PhoshProxiedPlaceholder:object-path:
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
phosh_proxied_placeholder_init (PhoshProxiedPlaceholder *self)
{
}


PhoshProxiedPlaceholder*
phosh_proxied_placeholder_new (GDBusConnection *connection, const char *bus_name,
                                 const char *object_path)
{
  return g_object_new (PHOSH_TYPE_PROXIED_PLACEHOLDER,
                       "connection", connection,
                       "bus-name", bus_name,
                       "object-path", object_path,
                       NULL
                      );
}
