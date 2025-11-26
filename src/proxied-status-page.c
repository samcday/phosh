/*
 * Copyright (C) 2025 The Phosh Developers
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#define G_LOG_DOMAIN "phosh-proxied-status-page"

#include "proxied-status-page.h"

#include "proxied-list-status.h"
#include "proxied-placeholder.h"
#include "dbus/qs-status-dbus.h"
#include "util.h"

#define IMAGE_SPACING 4

/**
 * PhoshProxiedStatusPage:
 *
 * TODO:
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

struct _PhoshProxiedStatusPage {
  PhoshStatusPage parent;

  GDBusConnection       *connection;
  PhoshDBusStatus		    *proxy;
  char                  *bus_name;
  char                  *object_path;
  gboolean               connected;
};

G_DEFINE_TYPE (PhoshProxiedStatusPage, phosh_proxied_status_page, PHOSH_TYPE_STATUS_PAGE);


static void
phosh_proxied_status_page_set_property (GObject      *object,
                                       guint         property_id,
                                       const GValue *value,
                                       GParamSpec   *pspec)
{
  PhoshProxiedStatusPage *self = PHOSH_PROXIED_STATUS_PAGE (object);

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
phosh_proxied_status_page_get_property (GObject   *object,
                                       guint       property_id,
                                       GValue     *value,
                                       GParamSpec *pspec)
{
  PhoshProxiedStatusPage *self = PHOSH_PROXIED_STATUS_PAGE (object);

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
name_owner_changed_cb (PhoshProxiedStatusPage *self, gpointer data)
{
  gboolean connected = g_dbus_proxy_get_name_owner (G_DBUS_PROXY (self->proxy)) != NULL;

  if (connected == self->connected)
    return;

  self->connected = connected;

  g_debug ("%s on bus %s is now %s", self->object_path, self->bus_name,
           self->connected ? "connected" : "disconnected");
  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_CONNECTED]);
}


static gboolean
transform_content (GBinding *binding, const GValue *from_value, GValue *to_value,
                   PhoshProxiedStatusPage *self)
{
  const char *type = g_value_get_string (from_value);
  GtkWidget *content = NULL;

  if (strcmp (type, "placeholder") == 0)
    content = GTK_WIDGET (phosh_proxied_placeholder_new (self->connection, self->bus_name,
                                                         self->object_path));
  else if (strcmp (type, "list") == 0)
    content = GTK_WIDGET (phosh_proxied_list_status_new (self->connection, self->bus_name,
                                                         self->object_path));

  if (content == NULL)
    g_warning ("Unknown status page type '%s'", type);

  g_value_set_object (to_value, content);

  return TRUE;
}

static gboolean
transform_empty_str (GBinding *binding, const GValue *from_value, GValue *to_value, gpointer data)
{
  const char *str = g_value_get_string (from_value);
  if (str != NULL && strlen (str))
    g_value_set_string (to_value, str);
  return TRUE;
}


static void
header_activated_cb (GObject *source_object,
           GAsyncResult *res,
           PhoshProxiedStatusPage *self)
{
  g_autoptr (GError) err = NULL;

  phosh_dbus_status_call_activate_header_finish (self->proxy, res, &err);
  if (err != NULL)
    phosh_async_error_warn (err, "Failed to call ActivateHeader");
}


static void
on_header_clicked (PhoshProxiedStatusPage *self)
{
  phosh_dbus_status_call_activate_header (self->proxy, NULL, (GAsyncReadyCallback)header_activated_cb, self);
}


static void
footer_activated_cb (GObject *source_object,
           GAsyncResult *res,
           PhoshProxiedStatusPage *self)
{
  g_autoptr (GError) err = NULL;

  phosh_dbus_status_call_activate_footer_finish (self->proxy, res, &err);
  if (err != NULL)
    phosh_async_error_warn (err, "Failed to call ActivateFooter");
}


static void
on_footer_clicked (PhoshProxiedStatusPage *self)
{
  phosh_dbus_status_call_activate_footer (self->proxy, NULL, (GAsyncReadyCallback)footer_activated_cb, self);
}


static GtkWidget*
construct_widget (PhoshProxiedStatusPage *self, GtkWidget *current, gboolean activatable,
                  const char *type, GCallback on_clicked)
{
  GtkWidget *parent, *label, *icon;
  g_autofree char *label_prop = NULL;
  g_autofree char *icon_prop = NULL;

  icon = gtk_image_new_from_icon_name (NULL, GTK_ICON_SIZE_BUTTON);

  /* if this widget is activatable, it's a Button, else a Box. */
  if (activatable) {
    if (current != NULL && GTK_IS_BUTTON (current))
      /* we've already constructed this Button */
      return current;

    parent = gtk_button_new ();
    /* the button is both the parent and the widget that label prop is bound to */
    label = parent;
    gtk_button_set_image (GTK_BUTTON (parent), icon);
    gtk_button_set_always_show_image (GTK_BUTTON (parent), TRUE);

    g_signal_connect_object (parent, "clicked", on_clicked, self, G_CONNECT_SWAPPED);
  } else {
    if (current != NULL && GTK_IS_BOX (current))
      /* we've already constructed this Box */
      return current;
    parent = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, IMAGE_SPACING);
    label = gtk_label_new (NULL);

    /* ensures that the icon + label are correctly centered in footer */
    gtk_widget_set_halign (parent, GTK_ALIGN_CENTER);
    gtk_container_add (GTK_CONTAINER (parent), icon);
    gtk_container_add (GTK_CONTAINER (parent), label);
    gtk_widget_show (icon);
    gtk_widget_show (label);
  }

  gtk_widget_show (parent);

  label_prop = g_strdup_printf ("%s-text", type);
  g_object_bind_property_full (self->proxy, label_prop, label, "label", G_BINDING_SYNC_CREATE,
    transform_empty_str, NULL, NULL, NULL);

  icon_prop = g_strdup_printf ("%s-icon", type);
  g_object_bind_property_full (self->proxy, icon_prop, icon, "icon-name", G_BINDING_SYNC_CREATE,
    transform_empty_str, NULL, NULL, NULL);

  return parent;
}


static void
update_header (PhoshProxiedStatusPage *self, gpointer data)
{
  const char *text = phosh_dbus_status_get_header_text (self->proxy);
  const char *icon = phosh_dbus_status_get_header_icon (self->proxy);
  gboolean activatable = phosh_dbus_status_get_header_activatable (self->proxy);
  GtkWidget *widget;

  if (!strlen (text) && !strlen (icon)) {
    /* neither a label nor icon is set, thus there's no header. */
    phosh_status_page_set_header (PHOSH_STATUS_PAGE (self), NULL);
    return;
  }

  widget = phosh_status_page_get_header (PHOSH_STATUS_PAGE (self));
  widget = construct_widget (self, widget, activatable, "header", (GCallback)on_header_clicked);
  phosh_status_page_set_header (PHOSH_STATUS_PAGE (self), widget);
}


static void
update_footer (PhoshProxiedStatusPage *self, gpointer data)
{
  const char *text = phosh_dbus_status_get_footer_text (self->proxy);
  const char *icon = phosh_dbus_status_get_footer_icon (self->proxy);
  gboolean activatable = phosh_dbus_status_get_footer_activatable (self->proxy);
  GtkWidget *widget;

  if (!strlen (text) && !strlen (icon)) {
    /* neither a label nor icon is set, thus there's no footer. */
    phosh_status_page_set_footer (PHOSH_STATUS_PAGE (self), NULL);
    return;
  }

  widget = phosh_status_page_get_footer (PHOSH_STATUS_PAGE (self));
  widget = construct_widget (self, widget, activatable, "footer", (GCallback)on_footer_clicked);
  gtk_widget_set_hexpand (widget, TRUE);

  phosh_status_page_set_footer (PHOSH_STATUS_PAGE (self), widget);
}


static void
dbus_init_cb (GObject *source_object, GAsyncResult *res, PhoshProxiedStatusPage *self)
{
  g_autoptr (GError) err = NULL;

  self->proxy = phosh_dbus_status_proxy_new_finish (res, &err);
  if (self->proxy == NULL) {
    phosh_async_error_warn (err, "Couldn't connect to Status service");
    return;
  }

  g_object_bind_property (self->proxy, "title", self, "title", G_BINDING_SYNC_CREATE);

  g_signal_connect_object (self->proxy, "notify::footer-text",
                           (GCallback) update_footer, self, G_CONNECT_SWAPPED);
  g_signal_connect_object (self->proxy, "notify::footer-icon",
                           (GCallback) update_footer, self, G_CONNECT_SWAPPED);
  g_signal_connect_object (self->proxy, "notify::footer-activatable",
                           (GCallback) update_footer, self, G_CONNECT_SWAPPED);

  g_signal_connect_object (self->proxy, "notify::header-text",
                           (GCallback) update_header, self, G_CONNECT_SWAPPED);
  g_signal_connect_object (self->proxy, "notify::header-icon",
                           (GCallback) update_header, self, G_CONNECT_SWAPPED);
  g_signal_connect_object (self->proxy, "notify::header-activatable",
                           (GCallback) update_header, self, G_CONNECT_SWAPPED);

  update_footer (self, NULL);
  update_header (self, NULL);

  g_object_bind_property_full (self->proxy, "type", self, "content", G_BINDING_SYNC_CREATE,
    (GBindingTransformFunc)transform_content, NULL, self, NULL);

  g_signal_connect_object (self->proxy, "notify::g-name-owner",
                           (GCallback) name_owner_changed_cb, self, G_CONNECT_SWAPPED);
  name_owner_changed_cb (self, NULL);
}


static void
phosh_proxied_status_page_constructed (GObject *object)
{
  PhoshProxiedStatusPage *self = PHOSH_PROXIED_STATUS_PAGE (object);

  if (!self->connection || !self->bus_name || !self->object_path)
    return;

  phosh_dbus_status_proxy_new (self->connection,
                               G_DBUS_PROXY_FLAGS_NONE,
                               self->bus_name,
                               self->object_path,
                               NULL,
                               (GAsyncReadyCallback)dbus_init_cb,
                               self);

  g_object_bind_property (self, "connected", self, "visible", G_BINDING_SYNC_CREATE);
}


static void
phosh_proxied_status_page_dispose (GObject *object)
{
  PhoshProxiedStatusPage *self = PHOSH_PROXIED_STATUS_PAGE (object);

  g_clear_object (&self->proxy);
  g_clear_object (&self->connection);
  g_clear_pointer (&self->bus_name, g_free);
  g_clear_pointer (&self->object_path, g_free);

  G_OBJECT_CLASS (phosh_proxied_status_page_parent_class)->dispose (object);
}


static void
phosh_proxied_status_page_class_init (PhoshProxiedStatusPageClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->constructed = phosh_proxied_status_page_constructed;
  object_class->dispose = phosh_proxied_status_page_dispose;
  object_class->get_property = phosh_proxied_status_page_get_property;
  object_class->set_property = phosh_proxied_status_page_set_property;

  /**
   * PhoshProxiedStatusPage:connected:
   *
   * Whether this quick setting is actively connected to a remote bus object
   */
  props[PROP_CONNECTED] =
    g_param_spec_boolean ("connected", "", "",
                          FALSE,
                          G_PARAM_READABLE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS);

  /**
   * PhoshProxiedStatusPage:connection:
   *
   * The DBus connection this quick setting should connect via
   */
  props[PROP_CONNECTION] =
    g_param_spec_object ("connection", "", "",
                         G_TYPE_DBUS_CONNECTION,
                         G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

  /**
   * PhoshProxiedStatusPage:bus-name:
   *
   * The bus name of the remote object to bind this quick setting to
   */
  props[PROP_BUS_NAME] =
    g_param_spec_string ("bus-name", "", "",
                         "",
                         G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

  /**
   * PhoshProxiedStatusPage:object-path:
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
phosh_proxied_status_page_init (PhoshProxiedStatusPage *self)
{
}


PhoshProxiedStatusPage*
phosh_proxied_status_page_new (GDBusConnection *connection, const char *bus_name,
                                 const char *object_path)
{
  return g_object_new (PHOSH_TYPE_PROXIED_STATUS_PAGE,
                       "connection", connection,
                       "bus-name", bus_name,
                       "object-path", object_path,
                       NULL
                      );
}
