/*
 * Copyright (C) 2025 The Phosh Developers
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#define G_LOG_DOMAIN "phosh-proxied-quick-setting"

#include "proxied-quick-setting.h"

#include "dbus/quick-setting-dbus.h"
#include "proxied-status-page.h"
#include "quick-setting.h"
#include "status-icon.h"
#include "util.h"

/**
 * PhoshProxiedQuickSetting:
 *
 * Proxied quick settings are [type@QuickSetting]s that are bound to a remote DBus object. This bus
 * object is expected to implement the mobi.phosh.shell.QuickSetting interface.This class will
 * bind its QS label, icon and active state to the appropriate properties in that interface, as well
 * as invoking Press and LongPress when those events occur.
 *
 * The visibility of this widget is bound to its connected state.
 */

enum {
  PROP_0,
  PROP_CONNECTION,
  PROP_BUS_NAME,
  PROP_OBJECT_PATH,
  PROP_CONNECTED,
  PROP_LAST_PROP,
};
static GParamSpec *props[PROP_LAST_PROP];

struct _PhoshProxiedQuickSetting {
  PhoshQuickSetting parent;

  PhoshStatusIcon       *status_icon;
  GDBusConnection       *connection;
  PhoshDBusQuickSetting *proxy;
  char                  *bus_name;
  char                  *object_path;
  gboolean               connected;
};

G_DEFINE_TYPE (PhoshProxiedQuickSetting, phosh_proxied_quick_setting, PHOSH_TYPE_QUICK_SETTING);


static void
phosh_proxied_quick_setting_set_property (GObject      *object,
                                       guint         property_id,
                                       const GValue *value,
                                       GParamSpec   *pspec)
{
  PhoshProxiedQuickSetting *self = PHOSH_PROXIED_QUICK_SETTING (object);

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
phosh_proxied_quick_setting_get_property (GObject    *object,
                                       guint       property_id,
                                       GValue     *value,
                                       GParamSpec *pspec)
{
  PhoshProxiedQuickSetting *self = PHOSH_PROXIED_QUICK_SETTING (object);

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
  case PROP_CONNECTED:
    g_value_set_boolean (value, self->connected);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}


static void
name_owner_changed_cb (PhoshProxiedQuickSetting *self, gpointer data)
{
  gboolean connected = g_dbus_proxy_get_name_owner (G_DBUS_PROXY (self->proxy)) != NULL;

  if (connected == self->connected)
    return;

  self->connected = connected;
  g_debug ("%s on bus %s is now %s", self->object_path, self->bus_name,
           self->connected ? "connected" : "disconnected");
  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_CONNECTED]);
}


static void
press_cb (GObject *source_object,
           GAsyncResult *res,
           PhoshProxiedQuickSetting *self)
{
  g_autoptr (GError) err = NULL;

  phosh_dbus_quick_setting_call_press_finish (self->proxy, res, &err);
  if (err != NULL)
    phosh_async_error_warn (err, "Failed to call Press");
}


static void
long_press_cb (GObject *source_object,
           GAsyncResult *res,
           PhoshProxiedQuickSetting *self)
{
  g_autoptr (GError) err = NULL;

  phosh_dbus_quick_setting_call_long_press_finish (self->proxy, res, &err);
  if (err != NULL)
    phosh_async_error_warn (err, "Failed to call LongPress");
}


static void
on_clicked (PhoshProxiedQuickSetting *self)
{
  phosh_dbus_quick_setting_call_press (self->proxy, NULL,
                                       (GAsyncReadyCallback) press_cb, self);
}


static void
on_long_pressed (PhoshProxiedQuickSetting *self)
{
  phosh_dbus_quick_setting_call_long_press (self->proxy, NULL,
                                            (GAsyncReadyCallback) long_press_cb, self);
}


static gboolean
transform_can_show_status (GBinding *binding, const GValue *from_value, GValue *to_value,
                           PhoshProxiedQuickSetting *self)
{
  g_value_set_boolean (to_value, !!g_value_get_object (from_value));
  return TRUE;
}


static gboolean
transform_status_page (GBinding *binding, const GValue *from_value, GValue *to_value,
                       PhoshProxiedQuickSetting *self)
{
  g_autoptr (PhoshProxiedStatusPage) status_page = NULL;
  const char *object_path = g_value_get_string (from_value);

  if (object_path != NULL)
    status_page = phosh_proxied_status_page_new (self->connection,
                                                 self->bus_name,
                                                 object_path);

  g_value_set_object (to_value, g_steal_pointer (&status_page));

  return TRUE;
}


static void
dbus_init_cb (GObject               *source_object,
              GAsyncResult          *res,
              PhoshProxiedQuickSetting *self)
{
  g_autoptr (GError) err = NULL;

  self->proxy = phosh_dbus_quick_setting_proxy_new_finish (res, &err);
  if (self->proxy == NULL) {
    phosh_async_error_warn (err, "Couldn't connect to QS service");
    return;
  }

  g_object_bind_property (self->proxy, "label", self->status_icon, "info", G_BINDING_SYNC_CREATE);
  g_object_bind_property (self->proxy, "active", self, "active", G_BINDING_SYNC_CREATE);
  g_object_bind_property (self->proxy, "icon", self->status_icon, "icon-name", G_BINDING_SYNC_CREATE);
  g_signal_connect_object (self->proxy, "notify::g-name-owner",
                           (GCallback) name_owner_changed_cb, self, G_CONNECT_SWAPPED);

  g_object_bind_property_full (self->proxy, "status", self, "status-page", G_BINDING_SYNC_CREATE,
                               (GBindingTransformFunc)transform_status_page, NULL, self, NULL);
  g_object_bind_property_full (self, "status-page", self, "can-show-status", G_BINDING_SYNC_CREATE,
                               (GBindingTransformFunc)transform_can_show_status, NULL, self, NULL);
  name_owner_changed_cb (self, NULL);
}


static void
phosh_proxied_quick_setting_constructed (GObject *object)
{
  PhoshProxiedQuickSetting *self = PHOSH_PROXIED_QUICK_SETTING (object);

  if (!self->connection || !self->bus_name || !self->object_path)
    return;

  phosh_dbus_quick_setting_proxy_new (self->connection,
                                      G_DBUS_PROXY_FLAGS_NONE,
                                      self->bus_name,
                                      self->object_path,
                                      NULL,
                                      (GAsyncReadyCallback)dbus_init_cb,
                                      self);

  g_object_bind_property (self, "connected", self, "visible", G_BINDING_SYNC_CREATE);
}


static void
phosh_proxied_quick_setting_dispose (GObject *object)
{
  PhoshProxiedQuickSetting *self = PHOSH_PROXIED_QUICK_SETTING (object);

  g_clear_object (&self->proxy);
  g_clear_object (&self->connection);
  g_clear_pointer (&self->bus_name, g_free);
  g_clear_pointer (&self->object_path, g_free);

  G_OBJECT_CLASS (phosh_proxied_quick_setting_parent_class)->dispose (object);
}


static void
phosh_proxied_quick_setting_class_init (PhoshProxiedQuickSettingClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->constructed = phosh_proxied_quick_setting_constructed;
  object_class->dispose = phosh_proxied_quick_setting_dispose;
  object_class->get_property = phosh_proxied_quick_setting_get_property;
  object_class->set_property = phosh_proxied_quick_setting_set_property;

  /**
   * PhoshProxiedQuickSetting:connected:
   *
   * Whether this quick setting is actively connected to a remote bus object
   */
  props[PROP_CONNECTED] =
    g_param_spec_boolean ("connected", "", "",
                          FALSE,
                          G_PARAM_READABLE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS);

  /**
   * PhoshProxiedQuickSetting:connection:
   *
   * The DBus connection this quick setting should connect via
   */
  props[PROP_CONNECTION] =
    g_param_spec_object ("connection", "", "",
                         G_TYPE_DBUS_CONNECTION,
                         G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

  /**
   * PhoshProxiedQuickSetting:bus-name:
   *
   * The bus name of the remote object to bind this quick setting to
   */
  props[PROP_BUS_NAME] =
    g_param_spec_string ("bus-name", "", "",
                         "",
                         G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

  /**
   * PhoshProxiedQuickSetting:object-path:
   *
   * The path of the remote bus object to bind this quick setting to
   */
  props[PROP_OBJECT_PATH] =
    g_param_spec_string ("object-path", "", "",
                         "",
                         G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, PROP_LAST_PROP, props);
}


static void
phosh_proxied_quick_setting_init (PhoshProxiedQuickSetting *self)
{
  self->status_icon = PHOSH_STATUS_ICON (phosh_status_icon_new ());

  phosh_quick_setting_set_status_icon (PHOSH_QUICK_SETTING (self), self->status_icon);
  gtk_widget_show (GTK_WIDGET (self->status_icon));

  g_signal_connect (self, "clicked", (GCallback)on_clicked, NULL);
  g_signal_connect (self, "long-pressed", (GCallback)on_long_pressed, NULL);
}


PhoshProxiedQuickSetting*
phosh_proxied_quick_setting_new (GDBusConnection *connection, const char *bus_name,
                                 const char *object_path)
{
  return g_object_new (PHOSH_TYPE_PROXIED_QUICK_SETTING,
                       "connection", connection,
                       "bus-name", bus_name,
                       "object-path", object_path,
                       NULL
                      );
}
