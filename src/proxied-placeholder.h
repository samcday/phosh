/*
 * Copyright (C) 2025 The Phosh Developers
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <glib-object.h>

#include "status-page-placeholder.h"

G_BEGIN_DECLS

#define PHOSH_TYPE_PROXIED_PLACEHOLDER (phosh_proxied_placeholder_get_type ())
G_DECLARE_FINAL_TYPE (PhoshProxiedPlaceholder,
                      phosh_proxied_placeholder,
                      PHOSH, PROXIED_PLACEHOLDER, PhoshStatusPagePlaceholder)

PhoshProxiedPlaceholder* phosh_proxied_placeholder_new (GDBusConnection *connection,
                                                        const char *bus_name,
                                                        const char *object_path);

G_END_DECLS
