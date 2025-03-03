/*
 * Copyright (C) 2025 The Phosh Developers
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <glib-object.h>

#include "status-page.h"

G_BEGIN_DECLS

#define PHOSH_TYPE_PROXIED_STATUS_PAGE (phosh_proxied_status_page_get_type ())
G_DECLARE_FINAL_TYPE (PhoshProxiedStatusPage,
                      phosh_proxied_status_page,
                      PHOSH, PROXIED_STATUS_PAGE, PhoshStatusPage)

PhoshProxiedStatusPage* phosh_proxied_status_page_new (GDBusConnection *connection,
                                                       const char *bus_name,
                                                       const char *object_path);

G_END_DECLS
