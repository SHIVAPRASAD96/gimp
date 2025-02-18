/* GIMP - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * Screenshot plug-in
 * Copyright 1998-2007 Sven Neumann <sven@gimp.org>
 * Copyright 2003      Henrik Brix Andersen <brix@gimp.org>
 * Copyright 2012      Simone Karin Lehmann - OS X patches
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "config.h"

#ifdef PLATFORM_OSX

#include <stdlib.h> /* for system() on OSX */
#include <string.h>

#include <glib.h>
#include <glib/gstdio.h> /* g_unlink() */

#include <libgimp/gimp.h>
#include <libgimp/gimpui.h>

#include "screenshot.h"
#include "screenshot-osx.h"


/*
 * Mac OS X uses a rootless X server. This won't let us use
 * gdk_pixbuf_get_from_drawable() and similar function on the root
 * window to get the entire screen contents. With a native OS X build
 * we have to do this without X as well.
 *
 * Since Mac OS X 10.2 a system utility for screencapturing is
 * included. We can safely use this, since it's available on every OS
 * X version GIMP is running on.
 *
 * The main drawbacks are that it's not possible to shoot windows or
 * regions in scripts in noninteractive mode, and that windows always
 * include decorations, since decorations are different between X11
 * windows and native OS X app windows. But we can use this switch
 * to capture the shadow of a window, which is indeed very Mac-ish.
 *
 * This routines works well with X11 and as a navtive build
 */

gboolean
screenshot_osx_available (void)
{
  return TRUE;
}

ScreenshotCapabilities
screenshot_osx_get_capabilities (void)
{
  return (SCREENSHOT_CAN_SHOOT_DECORATIONS |
          SCREENSHOT_CAN_SHOOT_POINTER);
}

GimpPDBStatusType
screenshot_osx_shoot (ScreenshotValues *shootvals,
                      GdkScreen        *screen,
                      gint32           *image_ID)
{
  const gchar *mode    = " ";
  const gchar *cursor  = " ";
  gchar       *delay   = NULL;
  gchar       *filename;
  gchar       *command = NULL;

  switch (shootvals->shoot_type)
    {
    case SHOOT_REGION:
      mode = "-is";
      break;

    case SHOOT_WINDOW:
      mode = "-iwo";
      if (shootvals->decorate)
        mode = "-iw";
      break;

    case SHOOT_ROOT:
      mode = " ";
      break;

    default:
      break;
    }

  delay = g_strdup_printf ("-T %i", shootvals->select_delay);

  if (shootvals->show_cursor)
    cursor = "-C";

  filename = gimp_temp_name ("png");

  command = g_strjoin (" ",
                       "/usr/sbin/screencapture",
                       mode,
                       cursor,
                       delay,
                       filename,
                       NULL);

  g_free (delay);

  if (system ((const char *) command) == EXIT_SUCCESS)
    {
      *image_ID = gimp_file_load (GIMP_RUN_NONINTERACTIVE,
                                  filename, filename);
      gimp_image_set_filename (*image_ID, "screenshot.png");

      g_unlink (filename);
      g_free (filename);
      g_free (command);

      return GIMP_PDB_SUCCESS;
   }

  g_free (command);
  g_free (filename);

  return GIMP_PDB_EXECUTION_ERROR;
}

#endif /* PLATFORM_OSX */
