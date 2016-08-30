/* Pango
 * pango-coverage.c: Coverage maps for fonts
 *
 * Copyright (C) 2000 Red Hat Software
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
#include <string.h>

#include "pango-coverage.h"

typedef struct _PangoBlockInfo PangoBlockInfo;

#define N_BLOCKS_INCREMENT 256

/* The structure of a PangoCoverage object is a two-level table, with blocks of size 256.
 * each block is stored as a packed array of 2 bit values for each index, in LSB order.
 */

struct _PangoBlockInfo
{
  guchar *data;
  PangoCoverageLevel level;	/* Used if data == NULL */
};

struct _PangoCoverage
{
  guint ref_count;
  int n_blocks;

  PangoBlockInfo *blocks;
};

/**
 * pango_coverage_new:
 *
 * Create a new #PangoCoverage
 *
 * Return value: the newly allocated #PangoCoverage,
 *               initialized to %PANGO_COVERAGE_NONE
 *               with a reference count of one, which
 *               should be freed with pango_coverage_unref().
 **/
PangoCoverage *
pango_coverage_new (void)
{
  PangoCoverage *coverage = g_slice_new (PangoCoverage);

  coverage->n_blocks = N_BLOCKS_INCREMENT;
  coverage->blocks = g_new0 (PangoBlockInfo, coverage->n_blocks);
  coverage->ref_count = 1;

  return coverage;
}

/**
 * pango_coverage_copy:
 * @coverage: a #PangoCoverage
 *
 * Copy an existing #PangoCoverage. (This function may now be unnecessary
 * since we refcount the structure. File a bug if you use it.)
 *
 * Return value: (transfer full): the newly allocated #PangoCoverage,
 *               with a reference count of one, which should be freed
 *               with pango_coverage_unref().
 **/
PangoCoverage *
pango_coverage_copy (PangoCoverage *coverage)
{
  int i;
  PangoCoverage *result;

  g_return_val_if_fail (coverage != NULL, NULL);

  result = g_slice_new (PangoCoverage);
  result->n_blocks = coverage->n_blocks;
  result->blocks = g_new (PangoBlockInfo, coverage->n_blocks);
  result->ref_count = 1;

  for (i=0; i<coverage->n_blocks; i++)
    {
      if (coverage->blocks[i].data)
	{
	  result->blocks[i].data = g_new (guchar, 64);
	  memcpy (result->blocks[i].data, coverage->blocks[i].data, 64);
	}
      else
	result->blocks[i].data = NULL;

      result->blocks[i].level = coverage->blocks[i].level;
    }

  return result;
}

/**
 * pango_coverage_ref:
 * @coverage: a #PangoCoverage
 *
 * Increase the reference count on the #PangoCoverage by one
 *
 * Return value: @coverage
 **/
PangoCoverage *
pango_coverage_ref (PangoCoverage *coverage)
{
  g_return_val_if_fail (coverage != NULL, NULL);

  g_atomic_int_inc ((int *) &coverage->ref_count);

  return coverage;
}

/**
 * pango_coverage_unref:
 * @coverage: a #PangoCoverage
 *
 * Decrease the reference count on the #PangoCoverage by one.
 * If the result is zero, free the coverage and all associated memory.
 **/
void
pango_coverage_unref (PangoCoverage *coverage)
{
  int i;

  g_return_if_fail (coverage != NULL);
  g_return_if_fail (coverage->ref_count > 0);

  if (g_atomic_int_dec_and_test ((int *) &coverage->ref_count))
    {
      for (i=0; i<coverage->n_blocks; i++)
	g_slice_free1 (64, coverage->blocks[i].data);

      g_free (coverage->blocks);
      g_slice_free (PangoCoverage, coverage);
    }
}

/**
 * pango_coverage_get:
 * @coverage: a #PangoCoverage
 * @index_: the index to check
 *
 * Determine whether a particular index is covered by @coverage
 *
 * Return value: the coverage level of @coverage for character @index_.
 **/
PangoCoverageLevel
pango_coverage_get (PangoCoverage *coverage,
		    int            index)
{
  int block_index;

  g_return_val_if_fail (coverage != NULL, PANGO_COVERAGE_NONE);

  /* index should really have been defined unsigned.  Work around
   * it by just returning NONE.
   */
  if (G_UNLIKELY (index < 0))
    return PANGO_COVERAGE_NONE;

  block_index = index / 256;

  if (block_index >= coverage->n_blocks)
    return PANGO_COVERAGE_NONE;
  else
    {
      guchar *data = coverage->blocks[block_index].data;
      if (data)
	{
	  int i = index % 256;
	  int shift = (i % 4) * 2;

	  return (data[i/4] >> shift) & 0x3;
	}
      else
	return coverage->blocks[block_index].level;
    }
}

/**
 * pango_coverage_set:
 * @coverage: a #PangoCoverage
 * @index_: the index to modify
 * @level: the new level for @index_
 *
 * Modify a particular index within @coverage
 **/
void
pango_coverage_set (PangoCoverage     *coverage,
		    int                index,
		    PangoCoverageLevel level)
{
  int block_index, i;
  guchar *data;

  g_return_if_fail (coverage != NULL);
  g_return_if_fail (index >= 0);
  g_return_if_fail ((guint) level <= 3);

  block_index = index / 256;

  if (block_index >= coverage->n_blocks)
    {
      int old_n_blocks = coverage->n_blocks;

      coverage->n_blocks =
	N_BLOCKS_INCREMENT * ((block_index + N_BLOCKS_INCREMENT) / N_BLOCKS_INCREMENT);

      coverage->blocks = g_renew (PangoBlockInfo, coverage->blocks, coverage->n_blocks);
      memset (coverage->blocks + old_n_blocks, 0,
	      sizeof (PangoBlockInfo) * (coverage->n_blocks - old_n_blocks));
    }

  data = coverage->blocks[block_index].data;
  if (!data)
    {
      guchar byte;

      if (level == coverage->blocks[block_index].level)
	return;

      data = g_slice_alloc (64);
      coverage->blocks[block_index].data = data;

      byte = coverage->blocks[block_index].level |
	(coverage->blocks[block_index].level << 2) |
	(coverage->blocks[block_index].level << 4) |
	(coverage->blocks[block_index].level << 6);

      memset (data, byte, 64);
    }

  i = index % 256;
  data[i/4] |= level << ((i % 4) * 2);
}

/**
 * pango_coverage_max:
 * @coverage: a #PangoCoverage
 * @other: another #PangoCoverage
 *
 * Set the coverage for each index in @coverage to be the max (better)
 * value of the current coverage for the index and the coverage for
 * the corresponding index in @other.
 **/
void
pango_coverage_max (PangoCoverage *coverage,
		    PangoCoverage *other)
{
  int block_index, i;
  int old_blocks;

  g_return_if_fail (coverage != NULL);

  old_blocks = MIN (coverage->n_blocks, other->n_blocks);

  if (other->n_blocks > coverage->n_blocks)
    {
      coverage->n_blocks = other->n_blocks;
      coverage->blocks = g_renew (PangoBlockInfo, coverage->blocks, coverage->n_blocks);

      for (block_index = old_blocks; block_index < coverage->n_blocks; block_index++)
	{
	  if (other->blocks[block_index].data)
	    {
	      coverage->blocks[block_index].data = g_new (guchar, 64);
	      memcpy (coverage->blocks[block_index].data, other->blocks[block_index].data, 64);
	    }
	  else
	    coverage->blocks[block_index].data = NULL;

	  coverage->blocks[block_index].level = other->blocks[block_index].level;
	}
    }

  for (block_index = 0; block_index < old_blocks; block_index++)
    {
      if (!coverage->blocks[block_index].data && !other->blocks[block_index].data)
	{
	  coverage->blocks[block_index].level = MAX (coverage->blocks[block_index].level, other->blocks[block_index].level);
	}
      else if (coverage->blocks[block_index].data && other->blocks[block_index].data)
	{
	  guchar *data = coverage->blocks[block_index].data;

	  for (i=0; i<64; i++)
	    {
	      int byte1 = data[i];
	      int byte2 = other->blocks[block_index].data[i];

	      /* There are almost certainly some clever logical ops to do this */
	      data[i] =
		MAX (byte1 & 0x3, byte2 & 0x3) |
		MAX (byte1 & 0xc, byte2 & 0xc) |
		MAX (byte1 & 0x30, byte2 & 0x30) |
		MAX (byte1 & 0xc0, byte2 & 0xc0);
	    }
	}
      else
	{
	  guchar *src, *dest;
	  int level, byte2;

	  if (coverage->blocks[block_index].data)
	    {
	      src = dest = coverage->blocks[block_index].data;
	      level = other->blocks[block_index].level;
	    }
	  else
	    {
	      src = other->blocks[block_index].data;
	      dest = g_new (guchar, 64);
	      coverage->blocks[block_index].data = dest;
	      level = coverage->blocks[block_index].level;
	    }

	  byte2 = level | (level << 2) | (level << 4) | (level << 6);

	  for (i=0; i<64; i++)
	    {
	      int byte1 = src[i];

	      /* There are almost certainly some clever logical ops to do this */
	      dest[i] =
		MAX (byte1 & 0x3, byte2 & 0x3) |
		MAX (byte1 & 0xc, byte2 & 0xc) |
		MAX (byte1 & 0x30, byte2 & 0x30) |
		MAX (byte1 & 0xc0, byte2 & 0xc0);
	    }
	}
    }
}

#define PANGO_COVERAGE_MAGIC 0xc89dbd5e

/**
 * pango_coverage_to_bytes:
 * @coverage: a #PangoCoverage
 * @bytes: (out) (array length=n_bytes) (element-type guint8):
 *   location to store result (must be freed with g_free())
 * @n_bytes: (out): location to store size of result
 *
 * Convert a #PangoCoverage structure into a flat binary format
 **/
void
pango_coverage_to_bytes   (PangoCoverage  *coverage,
			   guchar        **bytes,
			   int            *n_bytes)
{
  int i, j;
  int size = 8 + 4 * coverage->n_blocks;
  guchar *data;
  int offset;

  for (i=0; i<coverage->n_blocks; i++)
    {
      if (coverage->blocks[i].data)
	size += 64;
    }

  data = g_malloc (size);

  *(guint32 *)&data[0] = g_htonl (PANGO_COVERAGE_MAGIC); /* Magic */
  *(guint32 *)&data[4] = g_htonl (coverage->n_blocks);
  offset = 8;

  for (i=0; i<coverage->n_blocks; i++)
    {
      guint32 header_val;

      /* Check for solid blocks. This is a sort of random place
       * to do the optimization, but we care most about getting
       * it right when storing it somewhere persistant.
       */
      if (coverage->blocks[i].data != NULL)
	{
	  guchar *data = coverage->blocks[i].data;
	  guchar first_val = data[0];

	  if (first_val == 0 || first_val == 0xff)
	    {
	      for (j = 1 ; j < 64; j++)
		if (data[j] != first_val)
		  break;

	      if (j == 64)
		{
		  g_slice_free1 (64, data);
		  coverage->blocks[i].data = NULL;
		  coverage->blocks[i].level = first_val & 0x3;
		}
	    }
	}

      if (coverage->blocks[i].data != NULL)
	header_val = (guint32)-1;
      else
	header_val = coverage->blocks[i].level;

      *(guint32 *)&data[offset] = g_htonl (header_val);
      offset += 4;

      if (coverage->blocks[i].data)
	{
	  memcpy (data + offset, coverage->blocks[i].data, 64);
	  offset += 64;
	}
    }

  *bytes = data;
  *n_bytes = size;
}

static guint32
pango_coverage_get_uint32 (guchar **ptr)
{
  guint32 val;

  memcpy (&val, *ptr, 4);
  *ptr += 4;

  return g_ntohl (val);
}

/**
 * pango_coverage_from_bytes:
 * @bytes: (array length=n_bytes) (element-type guint8): binary data
 *   representing a #PangoCoverage
 * @n_bytes: the size of @bytes in bytes
 *
 * Convert data generated from pango_converage_to_bytes() back
 * to a #PangoCoverage
 *
 * Return value: (transfer full): a newly allocated #PangoCoverage, or
 *               %NULL if the data was invalid.
 **/
PangoCoverage *
pango_coverage_from_bytes (guchar *bytes,
			   int     n_bytes)
{
  PangoCoverage *coverage = g_slice_new0 (PangoCoverage);
  guchar *ptr = bytes;
  int i;

  coverage->ref_count = 1;

  if (n_bytes < 8)
    goto error;

  if (pango_coverage_get_uint32 (&ptr) != PANGO_COVERAGE_MAGIC)
    goto error;

  coverage->n_blocks = pango_coverage_get_uint32 (&ptr);
  coverage->blocks = g_new0 (PangoBlockInfo, coverage->n_blocks);

  for (i = 0; i < coverage->n_blocks; i++)
    {
      guint val;

      if (ptr + 4 > bytes + n_bytes)
	goto error;

      val = pango_coverage_get_uint32 (&ptr);
      if (val == (guint32)-1)
	{
	  if (ptr + 64 > bytes + n_bytes)
	    goto error;

	  coverage->blocks[i].data = g_new (guchar, 64);
	  memcpy (coverage->blocks[i].data, ptr, 64);
	  ptr += 64;
	}
      else
	coverage->blocks[i].level = val;
    }

  return coverage;

 error:

  pango_coverage_unref (coverage);
  return NULL;
}


