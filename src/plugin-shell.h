/*
 * Copyright (C) 2024 The Phosh Developers
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Author: Guido Günther <agx@sigxcpu.org>
 */

#pragma once

#include "launcher-entry-manager.h"
#include "monitor-manager.h"
#include "session-manager.h"

G_BEGIN_DECLS

#define PHOSH_TYPE_SHELL phosh_shell_get_type ()

G_DECLARE_DERIVABLE_TYPE (PhoshShell, phosh_shell, PHOSH, SHELL, GObject)

struct _PhoshShellClass {
  GObjectClass parent_class;
  GType        (*get_lockscreen_type)  (PhoshShell *self);
  void         (*load_extension_point) (PhoshShell *self, const char *extension_point);

  /* Padding for future expansion */
  void (*_phosh_reserved2) (void);
  void (*_phosh_reserved3) (void);
  void (*_phosh_reserved4) (void);
  void (*_phosh_reserved5) (void);
  void (*_phosh_reserved6) (void);
  void (*_phosh_reserved7) (void);
  void (*_phosh_reserved8) (void);
  void (*_phosh_reserved9) (void);
};

void                       phosh_shell_set_default (PhoshShell *self);
PhoshShell                *phosh_shell_get_default (void);

GType                      phosh_shell_get_lockscreen_type  (PhoshShell *self);
void                       phosh_shell_load_extension_point (PhoshShell *self,
                                                             const char *extension_point);

/* Created by the shell on startup */
PhoshLauncherEntryManager *phosh_shell_get_launcher_entry_manager (PhoshShell *self);
PhoshMonitorManager       *phosh_shell_get_monitor_manager (PhoshShell *self);
PhoshSessionManager       *phosh_shell_get_session_manager (PhoshShell *self);


G_END_DECLS
