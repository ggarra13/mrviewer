/*
    mrViewer - the professional movie and flipbook playback
    Copyright (C) 2007-2014  Gonzalo Garramuno

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
/**
 * @file   mrvCSPUtils.cpp
 * @author gga
 * @date   Fri Feb  8 10:14:11 2008
 *
 * @brief  Color space utils as in mpv
 *         This file is mainly a copy of mpv's csputils.c file.
 *
 */

extern "C" {

#include <libavutil/common.h>
#include <libavcodec/avcodec.h>

}

#include "mrvCSPUtils.h"

// A := A * B
static void mp_mul_matrix3x3(float a[3][3], float b[3][3])
{
    float a00 = a[0][0], a01 = a[0][1], a02 = a[0][2],
          a10 = a[1][0], a11 = a[1][1], a12 = a[1][2],
          a20 = a[2][0], a21 = a[2][1], a22 = a[2][2];

    for (int i = 0; i < 3; i++) {
        a[0][i] = a00 * b[0][i] + a01 * b[1][i] + a02 * b[2][i];
        a[1][i] = a10 * b[0][i] + a11 * b[1][i] + a12 * b[2][i];
        a[2][i] = a20 * b[0][i] + a21 * b[1][i] + a22 * b[2][i];
    }
}

enum mp_csp avcol_spc_to_mp_csp(int avcolorspace)
{
    switch (avcolorspace) {
    case AVCOL_SPC_BT709:
                return MP_CSP_BT_709;
    case AVCOL_SPC_BT470BG:
        return MP_CSP_BT_601;
    case AVCOL_SPC_BT2020_NCL:
        return MP_CSP_BT_2020_NC;
    case AVCOL_SPC_BT2020_CL:
        return MP_CSP_BT_2020_C;
    case AVCOL_SPC_SMPTE170M:
        return MP_CSP_BT_601;
    case AVCOL_SPC_SMPTE240M:
        return MP_CSP_SMPTE_240M;
    case AVCOL_SPC_RGB:
        return MP_CSP_RGB;
    case AVCOL_SPC_YCOCG:
        return MP_CSP_YCGCO;
    default:
        return MP_CSP_AUTO;
    }
}

enum mp_csp_levels avcol_range_to_mp_csp_levels(int avrange)
{
    switch (avrange) {
    case AVCOL_RANGE_MPEG:
                return MP_CSP_LEVELS_TV;
    case AVCOL_RANGE_JPEG:
        return MP_CSP_LEVELS_PC;
    default:
        return MP_CSP_LEVELS_AUTO;
    }
}

enum mp_csp_prim avcol_pri_to_mp_csp_prim(int avpri)
{
    switch (avpri) {
    case AVCOL_PRI_SMPTE240M:   // Same as below
            case AVCOL_PRI_SMPTE170M:
                    return MP_CSP_PRIM_BT_601_525;
    case AVCOL_PRI_BT470BG:
        return MP_CSP_PRIM_BT_601_625;
    case AVCOL_PRI_BT709:
        return MP_CSP_PRIM_BT_709;
    case AVCOL_PRI_BT2020:
        return MP_CSP_PRIM_BT_2020;
    case AVCOL_PRI_BT470M:
        return MP_CSP_PRIM_BT_470M;
    default:
        return MP_CSP_PRIM_AUTO;
    }
}

enum mp_csp_trc avcol_trc_to_mp_csp_trc(int avtrc)
{
    switch (avtrc) {
    case AVCOL_TRC_BT709:
            case AVCOL_TRC_SMPTE170M:
                case AVCOL_TRC_SMPTE240M:
                    case AVCOL_TRC_BT1361_ECG:
                        case AVCOL_TRC_BT2020_10:
                            case AVCOL_TRC_BT2020_12:
                                    return MP_CSP_TRC_BT_1886;
    case AVCOL_TRC_IEC61966_2_1:
        return MP_CSP_TRC_SRGB;
    case AVCOL_TRC_LINEAR:
        return MP_CSP_TRC_LINEAR;
    case AVCOL_TRC_GAMMA22:
        return MP_CSP_TRC_GAMMA22;
    case AVCOL_TRC_GAMMA28:
        return MP_CSP_TRC_GAMMA28;
    case AVCOL_TRC_SMPTEST2084:
        return MP_CSP_TRC_PQ;
    case AVCOL_TRC_ARIB_STD_B67:
        return MP_CSP_TRC_HLG;
    default:
        return MP_CSP_TRC_AUTO;
    }
}


int mp_csp_to_avcol_spc(enum mp_csp colorspace)
{
    switch (colorspace) {
    case MP_CSP_BT_709:
        return AVCOL_SPC_BT709;
    case MP_CSP_BT_601:
        return AVCOL_SPC_BT470BG;
    case MP_CSP_BT_2020_NC:
        return AVCOL_SPC_BT2020_NCL;
    case MP_CSP_BT_2020_C:
        return AVCOL_SPC_BT2020_CL;
    case MP_CSP_SMPTE_240M:
        return AVCOL_SPC_SMPTE240M;
    case MP_CSP_RGB:
        return AVCOL_SPC_RGB;
    case MP_CSP_YCGCO:
        return AVCOL_SPC_YCOCG;
    default:
        return AVCOL_SPC_UNSPECIFIED;
    }
}

int mp_csp_levels_to_avcol_range(enum mp_csp_levels range)
{
    switch (range) {
    case MP_CSP_LEVELS_TV:
        return AVCOL_RANGE_MPEG;
    case MP_CSP_LEVELS_PC:
        return AVCOL_RANGE_JPEG;
    default:
        return AVCOL_RANGE_UNSPECIFIED;
    }
}

int mp_csp_prim_to_avcol_pri(enum mp_csp_prim prim)
{
    switch (prim) {
    case MP_CSP_PRIM_BT_601_525:
        return AVCOL_PRI_SMPTE170M;
    case MP_CSP_PRIM_BT_601_625:
        return AVCOL_PRI_BT470BG;
    case MP_CSP_PRIM_BT_709:
        return AVCOL_PRI_BT709;
    case MP_CSP_PRIM_BT_2020:
        return AVCOL_PRI_BT2020;
    case MP_CSP_PRIM_BT_470M:
        return AVCOL_PRI_BT470M;
    default:
        return AVCOL_PRI_UNSPECIFIED;
    }
}

int mp_csp_trc_to_avcol_trc(enum mp_csp_trc trc)
{
    switch (trc) {
    // We just call it BT.1886 since we're decoding, but it's still BT.709
    case MP_CSP_TRC_BT_1886:
        return AVCOL_TRC_BT709;
    case MP_CSP_TRC_SRGB:
        return AVCOL_TRC_IEC61966_2_1;
    case MP_CSP_TRC_LINEAR:
        return AVCOL_TRC_LINEAR;
    case MP_CSP_TRC_GAMMA22:
        return AVCOL_TRC_GAMMA22;
    case MP_CSP_TRC_GAMMA28:
        return AVCOL_TRC_GAMMA28;
    case MP_CSP_TRC_PQ:
        return AVCOL_TRC_SMPTEST2084;
    case MP_CSP_TRC_HLG:
        return AVCOL_TRC_ARIB_STD_B67;
    default:
        return AVCOL_TRC_UNSPECIFIED;
    }
}

enum mp_csp mp_csp_guess_colorspace(int width, int height)
{
    return width >= 1280 || height > 576 ? MP_CSP_BT_709 : MP_CSP_BT_601;
}

enum mp_csp_prim mp_csp_guess_primaries(int width, int height)
{
    // HD content
    if (width >= 1280 || height > 576)
        return MP_CSP_PRIM_BT_709;

    switch (height) {
    case 576: // Typical PAL content, including anamorphic/squared
        return MP_CSP_PRIM_BT_601_625;

    case 480: // Typical NTSC content, including squared
    case 486: // NTSC Pro or anamorphic NTSC
        return MP_CSP_PRIM_BT_601_525;

    default: // No good metric, just pick BT.709 to minimize damage
        return MP_CSP_PRIM_BT_709;
    }
}

enum mp_chroma_location avchroma_location_to_mp(int avloc)
{
    switch (avloc) {
    case AVCHROMA_LOC_LEFT:
                return MP_CHROMA_LEFT;
    case AVCHROMA_LOC_CENTER:
        return MP_CHROMA_CENTER;
    default:
        return MP_CHROMA_AUTO;
    }
}

int mp_chroma_location_to_av(enum mp_chroma_location mploc)
{
    switch (mploc) {
    case MP_CHROMA_LEFT:
        return AVCHROMA_LOC_LEFT;
    case MP_CHROMA_CENTER:
        return AVCHROMA_LOC_CENTER;
    default:
        return AVCHROMA_LOC_UNSPECIFIED;
    }
}

// Return location of chroma samples relative to luma samples. 0/0 means
// centered. Other possible values are -1 (top/left) and +1 (right/bottom).
void mp_get_chroma_location(enum mp_chroma_location loc, int *x, int *y)
{
    *x = 0;
    *y = 0;
    if (loc == MP_CHROMA_LEFT)
        *x = -1;
}

// return the primaries associated with a certain mp_csp_primaries val
struct mp_csp_primaries mp_get_csp_primaries(enum mp_csp_prim spc)
{
    /*
    Values from: ITU-R Recommendations BT.470-6, BT.601-7, BT.709-5, BT.2020-0

    https://www.itu.int/dms_pubrec/itu-r/rec/bt/R-REC-BT.470-6-199811-S!!PDF-E.pdf
    https://www.itu.int/dms_pubrec/itu-r/rec/bt/R-REC-BT.601-7-201103-I!!PDF-E.pdf
    https://www.itu.int/dms_pubrec/itu-r/rec/bt/R-REC-BT.709-5-200204-I!!PDF-E.pdf
    https://www.itu.int/dms_pubrec/itu-r/rec/bt/R-REC-BT.2020-0-201208-I!!PDF-E.pdf

    Other colorspaces from https://en.wikipedia.org/wiki/RGB_color_space#Specifications
    */

    // CIE standard illuminant series
    static const struct mp_csp_col_xy
        d50 = {0.34577, 0.35850},
    d65 = {0.31271, 0.32902},
    c   = {0.31006, 0.31616},
    dci = {0.31400, 0.35100},
    e   = {1.0/3.0, 1.0/3.0};

    switch (spc) {
    case MP_CSP_PRIM_BT_470M:
    {
        struct mp_csp_primaries t = {
            {0.670, 0.330},
            {0.210, 0.710},
            {0.140, 0.080},
            c
        };
        return t;
    }
    case MP_CSP_PRIM_BT_601_525:
    {
        struct mp_csp_primaries t = {
            {0.630, 0.340},
            {0.310, 0.595},
            {0.155, 0.070},
            d65
        };
        return t;
    }
    case MP_CSP_PRIM_BT_601_625:
    {
        struct mp_csp_primaries t = {
            {0.640, 0.330},
            {0.290, 0.600},
            {0.150, 0.060},
            d65
        };
        return t;
    }
    // This is the default assumption if no colorspace information could
    // be determined, eg. for files which have no video channel.
    case MP_CSP_PRIM_AUTO:
    case MP_CSP_PRIM_BT_709:
    {
        struct mp_csp_primaries t = {
            {0.640, 0.330},
            {0.300, 0.600},
            {0.150, 0.060},
            d65
        };
        return t;
    }
    case MP_CSP_PRIM_BT_2020:
    {
        struct mp_csp_primaries t = {
            {0.708, 0.292},
            {0.170, 0.797},
            {0.131, 0.046},
            d65
        };
        return t;
    }
    case MP_CSP_PRIM_APPLE:
    {
        struct mp_csp_primaries t = {
            {0.625, 0.340},
            {0.280, 0.595},
            {0.115, 0.070},
            d65
        };
        return t;
    }
    case MP_CSP_PRIM_ADOBE:
    {
        struct mp_csp_primaries t = {
            {0.640, 0.330},
            {0.210, 0.710},
            {0.150, 0.060},
            d65
        };
        return t;
    }
    case MP_CSP_PRIM_PRO_PHOTO:
    {
        struct mp_csp_primaries t = {
            {0.7347, 0.2653},
            {0.1596, 0.8404},
            {0.0366, 0.0001},
            d50
        };
        return t;
    }
    case MP_CSP_PRIM_CIE_1931:
    {
        struct mp_csp_primaries t = {
            {0.7347, 0.2653},
            {0.2738, 0.7174},
            {0.1666, 0.0089},
            e
        };
        return t;
    }
    // From SMPTE RP 431-2 and 432-1
    case MP_CSP_PRIM_DCI_P3:
    case MP_CSP_PRIM_DISPLAY_P3:
    {
        struct mp_csp_primaries t = {
            {0.680, 0.320},
            {0.265, 0.690},
            {0.150, 0.060},
            spc == MP_CSP_PRIM_DCI_P3 ? dci : d65
        };
        return t;
    }
    // From Panasonic VARICAM reference manual
    case MP_CSP_PRIM_V_GAMUT:
    {
        struct mp_csp_primaries t = {
            {0.730, 0.280},
            {0.165, 0.840},
            {0.100, -0.03},
            d65
        };
        return t;
    }
    // From Sony S-Log reference manual
    case MP_CSP_PRIM_S_GAMUT:
    {
        struct mp_csp_primaries t = {
            {0.730, 0.280},
            {0.140, 0.855},
            {0.100, -0.05},
            d65
        };
        return t;
    }
    default:
        struct mp_csp_primaries t = {{0}};
        return t;
    }
}

// Get the nominal peak for a given colorspace, relative to the reference white
// level. In other words, this returns the brightest encodable value that can
// be represented by a given transfer curve.
float mp_trc_nom_peak(enum mp_csp_trc trc)
{
    switch (trc) {
    case MP_CSP_TRC_PQ:
        return 10000.0 / MP_REF_WHITE;
    case MP_CSP_TRC_HLG:
        return 12.0;
    case MP_CSP_TRC_V_LOG:
        return 46.0855;
    case MP_CSP_TRC_S_LOG1:
        return 6.52;
    case MP_CSP_TRC_S_LOG2:
        return 9.212;
    }

    return 1.0;
}

bool mp_trc_is_hdr(enum mp_csp_trc trc)
{
    return mp_trc_nom_peak(trc) > 1.0;
}

// Compute the RGB/XYZ matrix as described here:
// http://www.brucelindbloom.com/index.html?Eqn_RGB_XYZ_Matrix.html
void mp_get_rgb2xyz_matrix(struct mp_csp_primaries space, float m[3][3])
{
    float S[3], X[4], Z[4];

    // Convert from CIE xyY to XYZ. Note that Y=1 holds true for all primaries
    X[0] = space.red.x   / space.red.y;
    X[1] = space.green.x / space.green.y;
    X[2] = space.blue.x  / space.blue.y;
    X[3] = space.white.x / space.white.y;

    Z[0] = (1 - space.red.x   - space.red.y)   / space.red.y;
    Z[1] = (1 - space.green.x - space.green.y) / space.green.y;
    Z[2] = (1 - space.blue.x  - space.blue.y)  / space.blue.y;
    Z[3] = (1 - space.white.x - space.white.y) / space.white.y;

    // S = XYZ^-1 * W
    for (int i = 0; i < 3; i++) {
        m[0][i] = X[i];
        m[1][i] = 1;
        m[2][i] = Z[i];
    }

    mp_invert_matrix3x3(m);

    for (int i = 0; i < 3; i++)
        S[i] = m[i][0] * X[3] + m[i][1] * 1 + m[i][2] * Z[3];

    // M = [Sc * XYZc]
    for (int i = 0; i < 3; i++) {
        m[0][i] = S[i] * X[i];
        m[1][i] = S[i] * 1;
        m[2][i] = S[i] * Z[i];
    }
}

// M := M * XYZd<-XYZs
static void mp_apply_chromatic_adaptation(struct mp_csp_col_xy src,
        struct mp_csp_col_xy dest, float m[3][3])
{
    // If the white points are nearly identical, this is a wasteful identity
    // operation.
    if (fabs(src.x - dest.x) < 1e-6 && fabs(src.y - dest.y) < 1e-6)
        return;

    // XYZd<-XYZs = Ma^-1 * (I*[Cd/Cs]) * Ma
    // http://www.brucelindbloom.com/index.html?Eqn_ChromAdapt.html
    float C[3][2], tmp[3][3] = {{0}};

    // Ma = Bradford matrix, arguably most popular method in use today.
    // This is derived experimentally and thus hard-coded.
    float bradford[3][3] = {
        {  0.8951,  0.2664, -0.1614 },
        { -0.7502,  1.7135,  0.0367 },
        {  0.0389, -0.0685,  1.0296 },
    };

    for (int i = 0; i < 3; i++) {
        // source cone
        C[i][0] = bradford[i][0] * mp_xy_X(src)
                  + bradford[i][1] * 1
                  + bradford[i][2] * mp_xy_Z(src);

        // dest cone
        C[i][1] = bradford[i][0] * mp_xy_X(dest)
                  + bradford[i][1] * 1
                  + bradford[i][2] * mp_xy_Z(dest);
    }

    // tmp := I * [Cd/Cs] * Ma
    for (int i = 0; i < 3; i++)
        tmp[i][i] = C[i][1] / C[i][0];

    mp_mul_matrix3x3(tmp, bradford);

    // M := M * Ma^-1 * tmp
    mp_invert_matrix3x3(bradford);
    mp_mul_matrix3x3(m, bradford);
    mp_mul_matrix3x3(m, tmp);
}

// get the coefficients of the source -> dest cms matrix
void mp_get_cms_matrix(struct mp_csp_primaries src,
                       struct mp_csp_primaries dest,
                       enum mp_render_intent intent, float m[3][3])
{
    float tmp[3][3];

    // In saturation mapping, we don't care about accuracy and just want
    // primaries to map to primaries, making this an identity transformation.
    if (intent == MP_INTENT_SATURATION) {
        for (int i = 0; i < 3; i++)
            m[i][i] = 1;
        return;
    }

    // RGBd<-RGBs = RGBd<-XYZd * XYZd<-XYZs * XYZs<-RGBs
    // Equations from: http://www.brucelindbloom.com/index.html?Math.html
    // Note: Perceptual is treated like relative colorimetric. There's no
    // definition for perceptual other than "make it look good".

    // RGBd<-XYZd, inverted from XYZd<-RGBd
    mp_get_rgb2xyz_matrix(dest, m);
    mp_invert_matrix3x3(m);

    // Chromatic adaptation, except in absolute colorimetric intent
    if (intent != MP_INTENT_ABSOLUTE_COLORIMETRIC)
        mp_apply_chromatic_adaptation(src.white, dest.white, m);

    // XYZs<-RGBs
    mp_get_rgb2xyz_matrix(src, tmp);
    mp_mul_matrix3x3(m, tmp);
}



void mp_invert_matrix3x3(float m[3][3])
{
    float m00 = m[0][0], m01 = m[0][1], m02 = m[0][2],
          m10 = m[1][0], m11 = m[1][1], m12 = m[1][2],
          m20 = m[2][0], m21 = m[2][1], m22 = m[2][2];

    // calculate the adjoint
    m[0][0] =  (m11 * m22 - m21 * m12);
    m[0][1] = -(m01 * m22 - m21 * m02);
    m[0][2] =  (m01 * m12 - m11 * m02);
    m[1][0] = -(m10 * m22 - m20 * m12);
    m[1][1] =  (m00 * m22 - m20 * m02);
    m[1][2] = -(m00 * m12 - m10 * m02);
    m[2][0] =  (m10 * m21 - m20 * m11);
    m[2][1] = -(m00 * m21 - m20 * m01);
    m[2][2] =  (m00 * m11 - m10 * m01);

    // calculate the determinant (as inverse == 1/det * adjoint,
    // adjoint * m == identity * det, so this calculates the det)
    float det = m00 * m[0][0] + m10 * m[0][1] + m20 * m[0][2];
    det = 1.0f / det;

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++)
            m[i][j] *= det;
    }
}
