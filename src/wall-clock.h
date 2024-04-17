/*
 * Copyright (C) 2024 The Phosh Developers
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <glib-object.h>

G_BEGIN_DECLS

#define PHOSH_TYPE_WALL_CLOCK phosh_wall_clock_get_type ()

G_DECLARE_FINAL_TYPE (PhoshWallClock, phosh_wall_clock, PHOSH, WALL_CLOCK, GObject)

PhoshWallClock  *phosh_wall_clock_get_default  (void);
const char      *phosh_wall_clock_get_clock    (PhoshWallClock *clock, gboolean time_only);

G_END_DECLS
