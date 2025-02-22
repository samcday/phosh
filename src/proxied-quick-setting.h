/*
 * Copyright (C) 2025 The Phosh Developers
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <glib-object.h>

#include "quick-setting.h"

G_BEGIN_DECLS

#define PHOSH_TYPE_PROXIED_QUICK_SETTING (phosh_proxied_quick_setting_get_type ())
G_DECLARE_FINAL_TYPE (PhoshProxiedQuickSetting,
                      phosh_proxied_quick_setting,
                      PHOSH, PROXIED_QUICK_SETTING, PhoshQuickSetting)

PhoshProxiedQuickSetting* phosh_proxied_quick_setting_new (GDBusConnection *connection,
                                                           const char *bus_name,
                                                           const char *object_path);

G_END_DECLS
