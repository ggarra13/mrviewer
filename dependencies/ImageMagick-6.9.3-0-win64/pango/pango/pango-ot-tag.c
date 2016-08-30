/* Pango
 * pango-ot-tag.h:
 *
 * Copyright (C) 2007 Red Hat Software
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "config.h"

#include "pango-ot-private.h"

/**
 * pango_ot_tag_from_script:
 * @script: A #PangoScript
 *
 * Finds the OpenType script tag corresponding to @script.
 *
 * The %PANGO_SCRIPT_COMMON, %PANGO_SCRIPT_INHERITED, and
 * %PANGO_SCRIPT_UNKNOWN scripts are mapped to the OpenType
 * 'DFLT' script tag that is also defined as
 * %PANGO_OT_TAG_DEFAULT_SCRIPT.
 *
 * Note that multiple #PangoScript values may map to the same
 * OpenType script tag.  In particular, %PANGO_SCRIPT_HIRAGANA
 * and %PANGO_SCRIPT_KATAKANA both map to the OT tag 'kana'.
 *
 * Return value: #PangoOTTag corresponding to @script or
 * %PANGO_OT_TAG_DEFAULT_SCRIPT if none found.
 *
 * Since: 1.18
 **/
PangoOTTag
pango_ot_tag_from_script (PangoScript script)
{
  hb_tag_t tag1, tag2;
  hb_ot_tags_from_script (hb_glib_script_to_script (script), &tag1, &tag2);
  return (PangoOTTag) tag1;
}

/**
 * pango_ot_tag_to_script:
 * @script_tag: A #PangoOTTag OpenType script tag
 *
 * Finds the #PangoScript corresponding to @script_tag.
 *
 * The 'DFLT' script tag is mapped to %PANGO_SCRIPT_COMMON.
 *
 * Note that an OpenType script tag may correspond to multiple
 * #PangoScript values.  In such cases, the #PangoScript value
 * with the smallest value is returned.
 * In particular, %PANGO_SCRIPT_HIRAGANA
 * and %PANGO_SCRIPT_KATAKANA both map to the OT tag 'kana'.
 * This function will return %PANGO_SCRIPT_HIRAGANA for
 * 'kana'.
 *
 * Return value: #PangoScript corresponding to @script_tag or
 * %PANGO_SCRIPT_UNKNOWN if none found.
 *
 * Since: 1.18
 **/
PangoScript
pango_ot_tag_to_script (PangoOTTag script_tag)
{
  return (PangoScript) hb_glib_script_from_script (hb_ot_tag_to_script ((hb_tag_t) script_tag));
}


/**
 * pango_ot_tag_from_language:
 * @language: A #PangoLanguage, or %NULL
 *
 * Finds the OpenType language-system tag best describing @language.
 *
 * Return value: #PangoOTTag best matching @language or
 * %PANGO_OT_TAG_DEFAULT_LANGUAGE if none found or if @language
 * is %NULL.
 *
 * Since: 1.18
 **/
PangoOTTag
pango_ot_tag_from_language (PangoLanguage *language)
{
  return (PangoOTTag) hb_ot_tag_from_language (hb_language_from_string (pango_language_to_string (language), -1));
}

/**
 * pango_ot_tag_to_language:
 * @language_tag: A #PangoOTTag OpenType language-system tag
 *
 * Finds a #PangoLanguage corresponding to @language_tag.
 *
 * Return value: #PangoLanguage best matching @language_tag or
 * #PangoLanguage corresponding to the string "xx" if none found.
 *
 * Since: 1.18
 **/
PangoLanguage *
pango_ot_tag_to_language (PangoOTTag language_tag)
{
  return pango_language_from_string (hb_language_to_string (hb_ot_tag_to_language ((hb_tag_t) language_tag)));
}
