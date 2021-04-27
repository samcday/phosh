/*
 * Copyright (C) 2021 Purism SPC
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Author: Guido Günther <agx@sigxcpu.org>
 */

#define G_LOG_DOMAIN "phosh-wl-buffer"


#include "wl-buffer.h"
#include "phosh-wayland.h"

#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>

/**
 * SECTION:wl-buffer
 * @short_description: A wayland buffer
 * @Title: PhoshWlBuffer
 *
 * A buffer received from the Wayland compositor containing image
 * data.
 */

static void
randname (char *buf)
{
  struct timespec ts;
  long r;
  clock_gettime (CLOCK_REALTIME, &ts);
  r = ts.tv_nsec;
  for (int i = 0; i < 6; ++i) {
    buf[i] = 'A'+(r&15)+(r&16)*2;
    r >>= 5;
  }
}

static int
anonymous_shm_open (void)
{
  char name[] = "/phosh-XXXXXX";
  int retries = 100;
  int fd;

  do {
    randname (name + strlen (name) - 6);
    --retries;
    /* shm_open guarantees that O_CLOEXEC is set */
    fd = shm_open (name, O_RDWR | O_CREAT | O_EXCL, 0600);
    if (fd >= 0) {
      shm_unlink (name);
      return fd;
    }
  } while (retries > 0 && errno == EEXIST);

  return -1;
}

static int
create_shm_file (off_t size)
{
  int fd = anonymous_shm_open ();
  if (fd < 0) {
    return fd;
  }

  if (ftruncate (fd, size) < 0) {
    close (fd);
    return -1;
  }

  return fd;
}


PhoshWlBuffer *
phosh_wl_buffer_new (enum wl_shm_format format, uint32_t width, uint32_t height, uint32_t stride)
{
  PhoshWayland *wl = phosh_wayland_get_default ();
  size_t size = stride * height;
  struct wl_shm_pool *pool;
  void *data;
  int fd;
  PhoshWlBuffer *buf;

  g_return_val_if_fail (PHOSH_IS_WAYLAND (wl), NULL);
  g_return_val_if_fail (size, NULL);

  fd = create_shm_file (size);
  g_return_val_if_fail (fd >= 0, NULL);

  data = mmap (NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (data == MAP_FAILED) {
    g_warning ("Could not mmap buffer [fd: %d] %s", fd, g_strerror (errno));
    close (fd);
    return NULL;
  }

  buf = g_new0 (PhoshWlBuffer, 1);
  buf->width = width;
  buf->height = height;
  buf->stride = stride;
  buf->format = format;
  buf->data = data;

  pool = wl_shm_create_pool (phosh_wayland_get_wl_shm (wl), fd, size);
  buf->wl_buffer = wl_shm_pool_create_buffer (pool, 0, width, height, stride, format);
  wl_shm_pool_destroy (pool);

  close(fd);

  return buf;
}


void
phosh_wl_buffer_destroy (PhoshWlBuffer *self)
{
  if (self == NULL)
    return;

  if (munmap (self->data, self->stride * self->height) < 0)
    g_warning ("Failed to unmap buffer %p: %s", self, g_strerror (errno));

  wl_buffer_destroy (self->wl_buffer);
  g_free (self);
}

gsize
phosh_wl_buffer_get_size (PhoshWlBuffer *self)
{
  return self->stride * self->height;
}

/**
 * phosh_wl_buffer_get_bytes:
 * @self: The #PhoshWlBuffer
 *
 * Returns: (transfer full): A copy of data as #GBytes
 */
GBytes *
phosh_wl_buffer_get_bytes (PhoshWlBuffer *self)
{
  return g_bytes_new (self->data, phosh_wl_buffer_get_size (self));
}
