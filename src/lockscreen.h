/*
 * Copyright (C) 2018 Purism SPC
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include "calls-manager.h"
#include "layersurface.h"

G_BEGIN_DECLS

/**
 * PhoshLockscreenPage:
 * @PHOSH_LOCKSCREEN_PAGE_INFO: The info page (clock, notifications, MPRIS, etc)
 * @PHOSH_LOCKSCREEN_PAGE_EXTRA: The extra page (an extension point used by Lockscreen subclasses)
 * @PHOSH_LOCKSCREEN_PAGE_UNLOCK: The unlock page (where PIN is entered)
 *
 * This enum indicates which page is shown on the lockscreen.
 * This helps #PhoshGnomeShellManager to decide when to emit
 * AcceleratorActivated events over DBus
 */
typedef enum {
  PHOSH_LOCKSCREEN_PAGE_INFO,
  PHOSH_LOCKSCREEN_PAGE_EXTRA,
  PHOSH_LOCKSCREEN_PAGE_UNLOCK,
} PhoshLockscreenPage;

#define PHOSH_TYPE_LOCKSCREEN (phosh_lockscreen_get_type ())

G_DECLARE_DERIVABLE_TYPE (PhoshLockscreen, phosh_lockscreen, PHOSH, LOCKSCREEN, PhoshLayerSurface)

struct _PhoshLockscreenClass
{
  PhoshLayerSurfaceClass parent_class;
  void (*unlock_submit_cb) (PhoshLockscreen *self);

  /* Padding for future expansion */
  void (*_phosh_reserved1) (void);
  void (*_phosh_reserved2) (void);
  void (*_phosh_reserved3) (void);
  void (*_phosh_reserved4) (void);
  void (*_phosh_reserved5) (void);
  void (*_phosh_reserved6) (void);
  void (*_phosh_reserved7) (void);
  void (*_phosh_reserved8) (void);
  void (*_phosh_reserved9) (void);
};

GtkWidget * phosh_lockscreen_new (GType lockscreen_type, gpointer layer_shell, gpointer wl_output,
                                  PhoshCallsManager *calls_manager);

void                phosh_lockscreen_set_page         (PhoshLockscreen *self,
                                                       PhoshLockscreenPage page);
PhoshLockscreenPage phosh_lockscreen_get_page         (PhoshLockscreen *self);
void                phosh_lockscreen_set_default_page (PhoshLockscreen *self,
                                                       PhoshLockscreenPage page);

const gchar *phosh_lockscreen_get_pin_entry   (PhoshLockscreen *self);
void         phosh_lockscreen_clear_pin_entry (PhoshLockscreen *self);
void         phosh_lockscreen_shake_pin_entry (PhoshLockscreen *self);

void phosh_lockscreen_set_unlock_status (PhoshLockscreen *self, const gchar *status);

void         phosh_lockscreen_add_extra_page (PhoshLockscreen *self, GtkWidget *widget);

G_END_DECLS
