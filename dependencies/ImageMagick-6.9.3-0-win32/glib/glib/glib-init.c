/*
 * Copyright © 2011 Canonical Limited
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * licence, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <http://www.gnu.org/licenses/>.
 *
 * Author: Ryan Lortie <desrt@desrt.ca>
 */

#include "config.h"

#include "glib-init.h"

#include "glib-private.h"
#include "gtypes.h"
#include "gutils.h"     /* for GDebugKey */
#include "gconstructor.h"
#include "gmem.h"       /* for g_mem_gc_friendly */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

/* This seems as good a place as any to make static assertions about platform
 * assumptions we make throughout GLib. */

/* We do not support 36-bit bytes or other historical curiosities. */
G_STATIC_ASSERT (CHAR_BIT == 8);

/* We assume that data pointers are the same size as function pointers... */
G_STATIC_ASSERT (sizeof (gpointer) == sizeof (GFunc));
G_STATIC_ASSERT (_g_alignof (gpointer) == _g_alignof (GFunc));
/* ... and that all function pointers are the same size. */
G_STATIC_ASSERT (sizeof (GFunc) == sizeof (GCompareDataFunc));
G_STATIC_ASSERT (_g_alignof (GFunc) == _g_alignof (GCompareDataFunc));

/**
 * g_mem_gc_friendly:
 *
 * This variable is %TRUE if the `G_DEBUG` environment variable
 * includes the key `gc-friendly`.
 */
#ifdef ENABLE_GC_FRIENDLY_DEFAULT
gboolean g_mem_gc_friendly = TRUE;
#else
gboolean g_mem_gc_friendly = FALSE;
#endif
GLogLevelFlags g_log_msg_prefix = G_LOG_LEVEL_ERROR | G_LOG_LEVEL_WARNING |
                                  G_LOG_LEVEL_CRITICAL | G_LOG_LEVEL_DEBUG;
GLogLevelFlags g_log_always_fatal = G_LOG_FATAL_MASK;

static gboolean
debug_key_matches (const gchar *key,
                   const gchar *token,
                   guint        length)
{
  /* may not call GLib functions: see note in g_parse_debug_string() */
  for (; length; length--, key++, token++)
    {
      char k = (*key   == '_') ? '-' : tolower (*key  );
      char t = (*token == '_') ? '-' : tolower (*token);

      if (k != t)
        return FALSE;
    }

  return *key == '\0';
}

/**
 * g_parse_debug_string:
 * @string: (allow-none): a list of debug options separated by colons, spaces, or
 * commas, or %NULL.
 * @keys: (array length=nkeys): pointer to an array of #GDebugKey which associate
 *     strings with bit flags.
 * @nkeys: the number of #GDebugKeys in the array.
 *
 * Parses a string containing debugging options
 * into a %guint containing bit flags. This is used
 * within GDK and GTK+ to parse the debug options passed on the
 * command line or through environment variables.
 *
 * If @string is equal to "all", all flags are set. Any flags
 * specified along with "all" in @string are inverted; thus,
 * "all,foo,bar" or "foo,bar,all" sets all flags except those
 * corresponding to "foo" and "bar".
 *
 * If @string is equal to "help", all the available keys in @keys
 * are printed out to standard error.
 *
 * Returns: the combined set of bit flags.
 */
guint
g_parse_debug_string  (const gchar     *string,
                       const GDebugKey *keys,
                       guint            nkeys)
{
  guint i;
  guint result = 0;

  if (string == NULL)
    return 0;

  /* this function is used during the initialisation of gmessages, gmem
   * and gslice, so it may not do anything that causes memory to be
   * allocated or risks messages being emitted.
   *
   * this means, more or less, that this code may not call anything
   * inside GLib.
   */

  if (!strcasecmp (string, "help"))
    {
      /* using stdio directly for the reason stated above */
      fprintf (stderr, "Supported debug values:");
      for (i = 0; i < nkeys; i++)
       fprintf (stderr, " %s", keys[i].key);
      fprintf (stderr, " all help\n");
    }
  else
    {
      const gchar *p = string;
      const gchar *q;
      gboolean invert = FALSE;

      while (*p)
       {
         q = strpbrk (p, ":;, \t");
         if (!q)
           q = p + strlen (p);

         if (debug_key_matches ("all", p, q - p))
           {
             invert = TRUE;
           }
         else
           {
             for (i = 0; i < nkeys; i++)
               if (debug_key_matches (keys[i].key, p, q - p))
                 result |= keys[i].value;
           }

         p = q;
         if (*p)
           p++;
       }

      if (invert)
        {
          guint all_flags = 0;

          for (i = 0; i < nkeys; i++)
            all_flags |= keys[i].value;

          result = all_flags & (~result);
        }
    }

  return result;
}

static guint
g_parse_debug_envvar (const gchar     *envvar,
                      const GDebugKey *keys,
                      gint             n_keys,
                      guint            default_value)
{
  const gchar *value;

#ifdef OS_WIN32
  /* "fatal-warnings,fatal-criticals,all,help" is pretty short */
  gchar buffer[100];

  if (GetEnvironmentVariable (envvar, buffer, 100) < 100)
    value = buffer;
  else
    return 0;
#else
  value = getenv (envvar);
#endif

  if (value == NULL)
    return default_value;

  return g_parse_debug_string (value, keys, n_keys);
}

static void
g_messages_prefixed_init (void)
{
  const GDebugKey keys[] = {
    { "error", G_LOG_LEVEL_ERROR },
    { "critical", G_LOG_LEVEL_CRITICAL },
    { "warning", G_LOG_LEVEL_WARNING },
    { "message", G_LOG_LEVEL_MESSAGE },
    { "info", G_LOG_LEVEL_INFO },
    { "debug", G_LOG_LEVEL_DEBUG }
  };

  g_log_msg_prefix = g_parse_debug_envvar ("G_MESSAGES_PREFIXED", keys, G_N_ELEMENTS (keys), g_log_msg_prefix);
}

static void
g_debug_init (void)
{
  const GDebugKey keys[] = {
    { "gc-friendly", 1 },
    {"fatal-warnings",  G_LOG_LEVEL_WARNING | G_LOG_LEVEL_CRITICAL },
    {"fatal-criticals", G_LOG_LEVEL_CRITICAL }
  };
  GLogLevelFlags flags;

  flags = g_parse_debug_envvar ("G_DEBUG", keys, G_N_ELEMENTS (keys), 0);

  g_log_always_fatal |= flags & G_LOG_LEVEL_MASK;

  g_mem_gc_friendly = flags & 1;
}

static void
glib_init (void)
{
  g_messages_prefixed_init ();
  g_debug_init ();
  g_quark_init ();
}

HMODULE glib_dll = NULL;

#if defined (G_HAS_CONSTRUCTORS)

#ifdef G_DEFINE_CONSTRUCTOR_NEEDS_PRAGMA
#pragma G_DEFINE_CONSTRUCTOR_PRAGMA_ARGS(glib_init_ctor)
#endif
G_DEFINE_CONSTRUCTOR(glib_init_ctor)
G_DEFINE_DESTRUCTOR(glib_init_dtor)

static void
glib_init_ctor (void)
{
  g_clock_win32_init();
  g_thread_win32_init();
  glib_init();
  gobject_init_ctor();
}

static void
glib_init_dtor(void)
{
  g_thread_win32_thread_detach();
}

#else
# error Your platform/compiler is missing constructor support
#endif
