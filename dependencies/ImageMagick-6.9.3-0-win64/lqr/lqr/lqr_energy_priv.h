/* LiquidRescaling Library
 * Copyright (C) 2007-2009 Carlo Baldassi (the "Author") <carlobaldassi@gmail.com>.
 * All Rights Reserved.
 *
 * This library implements the algorithm described in the paper
 * "Seam Carving for Content-Aware Image Resizing"
 * by Shai Avidan and Ariel Shamir
 * which can be found at http://www.faculty.idc.ac.il/arik/imret.pdf
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; version 3 dated June, 2007.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>
 */

#ifndef __LQR_ENERGY_PRIV_H__
#define __LQR_ENERGY_PRIV_H__

#ifndef __LQR_BASE_H__
#error "lqr_base.h must be included prior to lqr_energy_priv.h"
#endif /* __LQR_BASE_H__ */

#ifndef __LQR_GRADIENT_H__
#error "lqr_gradient.h must be included prior to lqr_energy_priv.h"
#endif /* __LQR_GRADIENT_H__ */

#ifndef __LQR_READER_WINDOW_PUB_H__
#error "lqr_rwindow_pub.h must be included prior to lqr_energy_priv.h"
#endif /* __LQR_READER_WINDOW_PUB_H__ */

#define LQR_SATURATE_(x) (1 / (1 + (1 / (x))))
#define LQR_SATURATE(x) ((x) >= 0 ? LQR_SATURATE_(x) : -LQR_SATURATE_(-x))

gdouble lqr_pixel_get_norm(void *src, gint src_ind, LqrColDepth col_depth);
void lqr_pixel_set_norm(gdouble val, void *rgb, gint rgb_ind, LqrColDepth col_depth);
gdouble lqr_pixel_get_rgbcol(void *rgb, gint rgb_ind, LqrColDepth col_depth, LqrImageType image_type,
                             gint channel);
gdouble lqr_carver_read_brightness_grey(LqrCarver *r, gint x, gint y);
gdouble lqr_carver_read_brightness_std(LqrCarver *r, gint x, gint y);
gdouble lqr_carver_read_brightness_custom(LqrCarver *r, gint x, gint y);
gdouble lqr_carver_read_brightness(LqrCarver *r, gint x, gint y);
gdouble lqr_carver_read_luma_std(LqrCarver *r, gint x, gint y);
gdouble lqr_carver_read_luma(LqrCarver *r, gint x, gint y);
gdouble lqr_carver_read_rgba(LqrCarver *r, gint x, gint y, gint channel);
gdouble lqr_carver_read_custom(LqrCarver *r, gint x, gint y, gint channel);

gdouble lqr_carver_read_cached_std(LqrCarver *r, gint x, gint y);
gdouble lqr_carver_read_cached_rgba(LqrCarver *r, gint x, gint y, gint channel);
gdouble lqr_carver_read_cached_custom(LqrCarver *r, gint x, gint y, gint channel);

/* cache brightness (or luma or else) to speedup energy computation */
gdouble *lqr_carver_generate_rcache_bright();
gdouble *lqr_carver_generate_rcache_luma();
gdouble *lqr_carver_generate_rcache_rgba();
gdouble *lqr_carver_generate_rcache_custom();
gdouble *lqr_carver_generate_rcache();

gfloat lqr_energy_builtin_grad_all(gint x, gint y, gint img_width, gint img_height, LqrReadingWindow *rwindow,
                                   LqrGradFunc gf);
gfloat lqr_energy_builtin_grad_norm(gint x, gint y, gint img_width, gint img_height, LqrReadingWindow *rwindow,
                                    gpointer extra_data);
gfloat lqr_energy_builtin_grad_sumabs(gint x, gint y, gint img_width, gint img_height, LqrReadingWindow *rwindow,
                                      gpointer extra_data);
gfloat lqr_energy_builtin_grad_xabs(gint x, gint y, gint img_width, gint img_height, LqrReadingWindow *rwindow,
                                    gpointer extra_data);
gfloat lqr_energy_builtin_null(gint x, gint y, gint img_width, gint img_height, LqrReadingWindow *rwindow,
                               gpointer extra_data);

#endif /* __LQR_ENERGY_PRIV_H__ */
