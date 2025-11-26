/*
 * Copyright (C) 2025 The Phosh Developers
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define PHOSH_TYPE_PROXIED_LIST_STATUS (phosh_proxied_list_status_get_type ())
G_DECLARE_FINAL_TYPE (PhoshProxiedListStatus,
                      phosh_proxied_list_status,
                      PHOSH, PROXIED_LIST_STATUS, GtkListBox)

PhoshProxiedListStatus* phosh_proxied_list_status_new (GDBusConnection *connection,
                                                        const char *bus_name,
                                                        const char *object_path);

G_END_DECLS
