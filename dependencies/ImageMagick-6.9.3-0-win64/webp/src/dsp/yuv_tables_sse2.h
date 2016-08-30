// Copyright 2014 Google Inc. All Rights Reserved.
//
// Use of this source code is governed by a BSD-style license
// that can be found in the COPYING file in the root of the source
// tree. An additional intellectual property rights grant can be found
// in the file PATENTS. All contributing project authors may
// be found in the AUTHORS file in the root of the source tree.
// -----------------------------------------------------------------------------
//
// SSE2 tables for YUV->RGB conversion (12kB overall)
//
// Author: Skal (pascal.massimino@gmail.com)

// This file is not compiled, but #include'd directly from yuv.c
// Only used if WEBP_YUV_USE_SSE2_TABLES is defined.

static const VP8kCstSSE2 VP8kYtoRGBA[256] = {
  {{0xfffb77b0, 0xfffb77b0, 0xfffb77b0, 0x003fc000}},
  {{0xfffbc235, 0xfffbc235, 0xfffbc235, 0x003fc000}},
  {{0xfffc0cba, 0xfffc0cba, 0xfffc0cba, 0x003fc000}},
  {{0xfffc573f, 0xfffc573f, 0xfffc573f, 0x003fc000}},
  {{0xfffca1c4, 0xfffca1c4, 0xfffca1c4, 0x003fc000}},
  {{0xfffcec49, 0xfffcec49, 0xfffcec49, 0x003fc000}},
  {{0xfffd36ce, 0xfffd36ce, 0xfffd36ce, 0x003fc000}},
  {{0xfffd8153, 0xfffd8153, 0xfffd8153, 0x003fc000}},
  {{0xfffdcbd8, 0xfffdcbd8, 0xfffdcbd8, 0x003fc000}},
  {{0xfffe165d, 0xfffe165d, 0xfffe165d, 0x003fc000}},
  {{0xfffe60e2, 0xfffe60e2, 0xfffe60e2, 0x003fc000}},
  {{0xfffeab67, 0xfffeab67, 0xfffeab67, 0x003fc000}},
  {{0xfffef5ec, 0xfffef5ec, 0xfffef5ec, 0x003fc000}},
  {{0xffff4071, 0xffff4071, 0xffff4071, 0x003fc000}},
  {{0xffff8af6, 0xffff8af6, 0xffff8af6, 0x003fc000}},
  {{0xffffd57b, 0xffffd57b, 0xffffd57b, 0x003fc000}},
  {{0x00002000, 0x00002000, 0x00002000, 0x003fc000}},
  {{0x00006a85, 0x00006a85, 0x00006a85, 0x003fc000}},
  {{0x0000b50a, 0x0000b50a, 0x0000b50a, 0x003fc000}},
  {{0x0000ff8f, 0x0000ff8f, 0x0000ff8f, 0x003fc000}},
  {{0x00014a14, 0x00014a14, 0x00014a14, 0x003fc000}},
  {{0x00019499, 0x00019499, 0x00019499, 0x003fc000}},
  {{0x0001df1e, 0x0001df1e, 0x0001df1e, 0x003fc000}},
  {{0x000229a3, 0x000229a3, 0x000229a3, 0x003fc000}},
  {{0x00027428, 0x00027428, 0x00027428, 0x003fc000}},
  {{0x0002bead, 0x0002bead, 0x0002bead, 0x003fc000}},
  {{0x00030932, 0x00030932, 0x00030932, 0x003fc000}},
  {{0x000353b7, 0x000353b7, 0x000353b7, 0x003fc000}},
  {{0x00039e3c, 0x00039e3c, 0x00039e3c, 0x003fc000}},
  {{0x0003e8c1, 0x0003e8c1, 0x0003e8c1, 0x003fc000}},
  {{0x00043346, 0x00043346, 0x00043346, 0x003fc000}},
  {{0x00047dcb, 0x00047dcb, 0x00047dcb, 0x003fc000}},
  {{0x0004c850, 0x0004c850, 0x0004c850, 0x003fc000}},
  {{0x000512d5, 0x000512d5, 0x000512d5, 0x003fc000}},
  {{0x00055d5a, 0x00055d5a, 0x00055d5a, 0x003fc000}},
  {{0x0005a7df, 0x0005a7df, 0x0005a7df, 0x003fc000}},
  {{0x0005f264, 0x0005f264, 0x0005f264, 0x003fc000}},
  {{0x00063ce9, 0x00063ce9, 0x00063ce9, 0x003fc000}},
  {{0x0006876e, 0x0006876e, 0x0006876e, 0x003fc000}},
  {{0x0006d1f3, 0x0006d1f3, 0x0006d1f3, 0x003fc000}},
  {{0x00071c78, 0x00071c78, 0x00071c78, 0x003fc000}},
  {{0x000766fd, 0x000766fd, 0x000766fd, 0x003fc000}},
  {{0x0007b182, 0x0007b182, 0x0007b182, 0x003fc000}},
  {{0x0007fc07, 0x0007fc07, 0x0007fc07, 0x003fc000}},
  {{0x0008468c, 0x0008468c, 0x0008468c, 0x003fc000}},
  {{0x00089111, 0x00089111, 0x00089111, 0x003fc000}},
  {{0x0008db96, 0x0008db96, 0x0008db96, 0x003fc000}},
  {{0x0009261b, 0x0009261b, 0x0009261b, 0x003fc000}},
  {{0x000970a0, 0x000970a0, 0x000970a0, 0x003fc000}},
  {{0x0009bb25, 0x0009bb25, 0x0009bb25, 0x003fc000}},
  {{0x000a05aa, 0x000a05aa, 0x000a05aa, 0x003fc000}},
  {{0x000a502f, 0x000a502f, 0x000a502f, 0x003fc000}},
  {{0x000a9ab4, 0x000a9ab4, 0x000a9ab4, 0x003fc000}},
  {{0x000ae539, 0x000ae539, 0x000ae539, 0x003fc000}},
  {{0x000b2fbe, 0x000b2fbe, 0x000b2fbe, 0x003fc000}},
  {{0x000b7a43, 0x000b7a43, 0x000b7a43, 0x003fc000}},
  {{0x000bc4c8, 0x000bc4c8, 0x000bc4c8, 0x003fc000}},
  {{0x000c0f4d, 0x000c0f4d, 0x000c0f4d, 0x003fc000}},
  {{0x000c59d2, 0x000c59d2, 0x000c59d2, 0x003fc000}},
  {{0x000ca457, 0x000ca457, 0x000ca457, 0x003fc000}},
  {{0x000ceedc, 0x000ceedc, 0x000ceedc, 0x003fc000}},
  {{0x000d3961, 0x000d3961, 0x000d3961, 0x003fc000}},
  {{0x000d83e6, 0x000d83e6, 0x000d83e6, 0x003fc000}},
  {{0x000dce6b, 0x000dce6b, 0x000dce6b, 0x003fc000}},
  {{0x000e18f0, 0x000e18f0, 0x000e18f0, 0x003fc000}},
  {{0x000e6375, 0x000e6375, 0x000e6375, 0x003fc000}},
  {{0x000eadfa, 0x000eadfa, 0x000eadfa, 0x003fc000}},
  {{0x000ef87f, 0x000ef87f, 0x000ef87f, 0x003fc000}},
  {{0x000f4304, 0x000f4304, 0x000f4304, 0x003fc000}},
  {{0x000f8d89, 0x000f8d89, 0x000f8d89, 0x003fc000}},
  {{0x000fd80e, 0x000fd80e, 0x000fd80e, 0x003fc000}},
  {{0x00102293, 0x00102293, 0x00102293, 0x003fc000}},
  {{0x00106d18, 0x00106d18, 0x00106d18, 0x003fc000}},
  {{0x0010b79d, 0x0010b79d, 0x0010b79d, 0x003fc000}},
  {{0x00110222, 0x00110222, 0x00110222, 0x003fc000}},
  {{0x00114ca7, 0x00114ca7, 0x00114ca7, 0x003fc000}},
  {{0x0011972c, 0x0011972c, 0x0011972c, 0x003fc000}},
  {{0x0011e1b1, 0x0011e1b1, 0x0011e1b1, 0x003fc000}},
  {{0x00122c36, 0x00122c36, 0x00122c36, 0x003fc000}},
  {{0x001276bb, 0x001276bb, 0x001276bb, 0x003fc000}},
  {{0x0012c140, 0x0012c140, 0x0012c140, 0x003fc000}},
  {{0x00130bc5, 0x00130bc5, 0x00130bc5, 0x003fc000}},
  {{0x0013564a, 0x0013564a, 0x0013564a, 0x003fc000}},
  {{0x0013a0cf, 0x0013a0cf, 0x0013a0cf, 0x003fc000}},
  {{0x0013eb54, 0x0013eb54, 0x0013eb54, 0x003fc000}},
  {{0x001435d9, 0x001435d9, 0x001435d9, 0x003fc000}},
  {{0x0014805e, 0x0014805e, 0x0014805e, 0x003fc000}},
  {{0x0014cae3, 0x0014cae3, 0x0014cae3, 0x003fc000}},
  {{0x00151568, 0x00151568, 0x00151568, 0x003fc000}},
  {{0x00155fed, 0x00155fed, 0x00155fed, 0x003fc000}},
  {{0x0015aa72, 0x0015aa72, 0x0015aa72, 0x003fc000}},
  {{0x0015f4f7, 0x0015f4f7, 0x0015f4f7, 0x003fc000}},
  {{0x00163f7c, 0x00163f7c, 0x00163f7c, 0x003fc000}},
  {{0x00168a01, 0x00168a01, 0x00168a01, 0x003fc000}},
  {{0x0016d486, 0x0016d486, 0x0016d486, 0x003fc000}},
  {{0x00171f0b, 0x00171f0b, 0x00171f0b, 0x003fc000}},
  {{0x00176990, 0x00176990, 0x00176990, 0x003fc000}},
  {{0x0017b415, 0x0017b415, 0x0017b415, 0x003fc000}},
  {{0x0017fe9a, 0x0017fe9a, 0x0017fe9a, 0x003fc000}},
  {{0x0018491f, 0x0018491f, 0x0018491f, 0x003fc000}},
  {{0x001893a4, 0x001893a4, 0x001893a4, 0x003fc000}},
  {{0x0018de29, 0x0018de29, 0x0018de29, 0x003fc000}},
  {{0x001928ae, 0x001928ae, 0x001928ae, 0x003fc000}},
  {{0x00197333, 0x00197333, 0x00197333, 0x003fc000}},
  {{0x0019bdb8, 0x0019bdb8, 0x0019bdb8, 0x003fc000}},
  {{0x001a083d, 0x001a083d, 0x001a083d, 0x003fc000}},
  {{0x001a52c2, 0x001a52c2, 0x001a52c2, 0x003fc000}},
  {{0x001a9d47, 0x001a9d47, 0x001a9d47, 0x003fc000}},
  {{0x001ae7cc, 0x001ae7cc, 0x001ae7cc, 0x003fc000}},
  {{0x001b3251, 0x001b3251, 0x001b3251, 0x003fc000}},
  {{0x001b7cd6, 0x001b7cd6, 0x001b7cd6, 0x003fc000}},
  {{0x001bc75b, 0x001bc75b, 0x001bc75b, 0x003fc000}},
  {{0x001c11e0, 0x001c11e0, 0x001c11e0, 0x003fc000}},
  {{0x001c5c65, 0x001c5c65, 0x001c5c65, 0x003fc000}},
  {{0x001ca6ea, 0x001ca6ea, 0x001ca6ea, 0x003fc000}},
  {{0x001cf16f, 0x001cf16f, 0x001cf16f, 0x003fc000}},
  {{0x001d3bf4, 0x001d3bf4, 0x001d3bf4, 0x003fc000}},
  {{0x001d8679, 0x001d8679, 0x001d8679, 0x003fc000}},
  {{0x001dd0fe, 0x001dd0fe, 0x001dd0fe, 0x003fc000}},
  {{0x001e1b83, 0x001e1b83, 0x001e1b83, 0x003fc000}},
  {{0x001e6608, 0x001e6608, 0x001e6608, 0x003fc000}},
  {{0x001eb08d, 0x001eb08d, 0x001eb08d, 0x003fc000}},
  {{0x001efb12, 0x001efb12, 0x001efb12, 0x003fc000}},
  {{0x001f4597, 0x001f4597, 0x001f4597, 0x003fc000}},
  {{0x001f901c, 0x001f901c, 0x001f901c, 0x003fc000}},
  {{0x001fdaa1, 0x001fdaa1, 0x001fdaa1, 0x003fc000}},
  {{0x00202526, 0x00202526, 0x00202526, 0x003fc000}},
  {{0x00206fab, 0x00206fab, 0x00206fab, 0x003fc000}},
  {{0x0020ba30, 0x0020ba30, 0x0020ba30, 0x003fc000}},
  {{0x002104b5, 0x002104b5, 0x002104b5, 0x003fc000}},
  {{0x00214f3a, 0x00214f3a, 0x00214f3a, 0x003fc000}},
  {{0x002199bf, 0x002199bf, 0x002199bf, 0x003fc000}},
  {{0x0021e444, 0x0021e444, 0x0021e444, 0x003fc000}},
  {{0x00222ec9, 0x00222ec9, 0x00222ec9, 0x003fc000}},
  {{0x0022794e, 0x0022794e, 0x0022794e, 0x003fc000}},
  {{0x0022c3d3, 0x0022c3d3, 0x0022c3d3, 0x003fc000}},
  {{0x00230e58, 0x00230e58, 0x00230e58, 0x003fc000}},
  {{0x002358dd, 0x002358dd, 0x002358dd, 0x003fc000}},
  {{0x0023a362, 0x0023a362, 0x0023a362, 0x003fc000}},
  {{0x0023ede7, 0x0023ede7, 0x0023ede7, 0x003fc000}},
  {{0x0024386c, 0x0024386c, 0x0024386c, 0x003fc000}},
  {{0x002482f1, 0x002482f1, 0x002482f1, 0x003fc000}},
  {{0x0024cd76, 0x0024cd76, 0x0024cd76, 0x003fc000}},
  {{0x002517fb, 0x002517fb, 0x002517fb, 0x003fc000}},
  {{0x00256280, 0x00256280, 0x00256280, 0x003fc000}},
  {{0x0025ad05, 0x0025ad05, 0x0025ad05, 0x003fc000}},
  {{0x0025f78a, 0x0025f78a, 0x0025f78a, 0x003fc000}},
  {{0x0026420f, 0x0026420f, 0x0026420f, 0x003fc000}},
  {{0x00268c94, 0x00268c94, 0x00268c94, 0x003fc000}},
  {{0x0026d719, 0x0026d719, 0x0026d719, 0x003fc000}},
  {{0x0027219e, 0x0027219e, 0x0027219e, 0x003fc000}},
  {{0x00276c23, 0x00276c23, 0x00276c23, 0x003fc000}},
  {{0x0027b6a8, 0x0027b6a8, 0x0027b6a8, 0x003fc000}},
  {{0x0028012d, 0x0028012d, 0x0028012d, 0x003fc000}},
  {{0x00284bb2, 0x00284bb2, 0x00284bb2, 0x003fc000}},
  {{0x00289637, 0x00289637, 0x00289637, 0x003fc000}},
  {{0x0028e0bc, 0x0028e0bc, 0x0028e0bc, 0x003fc000}},
  {{0x00292b41, 0x00292b41, 0x00292b41, 0x003fc000}},
  {{0x002975c6, 0x002975c6, 0x002975c6, 0x003fc000}},
  {{0x0029c04b, 0x0029c04b, 0x0029c04b, 0x003fc000}},
  {{0x002a0ad0, 0x002a0ad0, 0x002a0ad0, 0x003fc000}},
  {{0x002a5555, 0x002a5555, 0x002a5555, 0x003fc000}},
  {{0x002a9fda, 0x002a9fda, 0x002a9fda, 0x003fc000}},
  {{0x002aea5f, 0x002aea5f, 0x002aea5f, 0x003fc000}},
  {{0x002b34e4, 0x002b34e4, 0x002b34e4, 0x003fc000}},
  {{0x002b7f69, 0x002b7f69, 0x002b7f69, 0x003fc000}},
  {{0x002bc9ee, 0x002bc9ee, 0x002bc9ee, 0x003fc000}},
  {{0x002c1473, 0x002c1473, 0x002c1473, 0x003fc000}},
  {{0x002c5ef8, 0x002c5ef8, 0x002c5ef8, 0x003fc000}},
  {{0x002ca97d, 0x002ca97d, 0x002ca97d, 0x003fc000}},
  {{0x002cf402, 0x002cf402, 0x002cf402, 0x003fc000}},
  {{0x002d3e87, 0x002d3e87, 0x002d3e87, 0x003fc000}},
  {{0x002d890c, 0x002d890c, 0x002d890c, 0x003fc000}},
  {{0x002dd391, 0x002dd391, 0x002dd391, 0x003fc000}},
  {{0x002e1e16, 0x002e1e16, 0x002e1e16, 0x003fc000}},
  {{0x002e689b, 0x002e689b, 0x002e689b, 0x003fc000}},
  {{0x002eb320, 0x002eb320, 0x002eb320, 0x003fc000}},
  {{0x002efda5, 0x002efda5, 0x002efda5, 0x003fc000}},
  {{0x002f482a, 0x002f482a, 0x002f482a, 0x003fc000}},
  {{0x002f92af, 0x002f92af, 0x002f92af, 0x003fc000}},
  {{0x002fdd34, 0x002fdd34, 0x002fdd34, 0x003fc000}},
  {{0x003027b9, 0x003027b9, 0x003027b9, 0x003fc000}},
  {{0x0030723e, 0x0030723e, 0x0030723e, 0x003fc000}},
  {{0x0030bcc3, 0x0030bcc3, 0x0030bcc3, 0x003fc000}},
  {{0x00310748, 0x00310748, 0x00310748, 0x003fc000}},
  {{0x003151cd, 0x003151cd, 0x003151cd, 0x003fc000}},
  {{0x00319c52, 0x00319c52, 0x00319c52, 0x003fc000}},
  {{0x0031e6d7, 0x0031e6d7, 0x0031e6d7, 0x003fc000}},
  {{0x0032315c, 0x0032315c, 0x0032315c, 0x003fc000}},
  {{0x00327be1, 0x00327be1, 0x00327be1, 0x003fc000}},
  {{0x0032c666, 0x0032c666, 0x0032c666, 0x003fc000}},
  {{0x003310eb, 0x003310eb, 0x003310eb, 0x003fc000}},
  {{0x00335b70, 0x00335b70, 0x00335b70, 0x003fc000}},
  {{0x0033a5f5, 0x0033a5f5, 0x0033a5f5, 0x003fc000}},
  {{0x0033f07a, 0x0033f07a, 0x0033f07a, 0x003fc000}},
  {{0x00343aff, 0x00343aff, 0x00343aff, 0x003fc000}},
  {{0x00348584, 0x00348584, 0x00348584, 0x003fc000}},
  {{0x0034d009, 0x0034d009, 0x0034d009, 0x003fc000}},
  {{0x00351a8e, 0x00351a8e, 0x00351a8e, 0x003fc000}},
  {{0x00356513, 0x00356513, 0x00356513, 0x003fc000}},
  {{0x0035af98, 0x0035af98, 0x0035af98, 0x003fc000}},
  {{0x0035fa1d, 0x0035fa1d, 0x0035fa1d, 0x003fc000}},
  {{0x003644a2, 0x003644a2, 0x003644a2, 0x003fc000}},
  {{0x00368f27, 0x00368f27, 0x00368f27, 0x003fc000}},
  {{0x0036d9ac, 0x0036d9ac, 0x0036d9ac, 0x003fc000}},
  {{0x00372431, 0x00372431, 0x00372431, 0x003fc000}},
  {{0x00376eb6, 0x00376eb6, 0x00376eb6, 0x003fc000}},
  {{0x0037b93b, 0x0037b93b, 0x0037b93b, 0x003fc000}},
  {{0x003803c0, 0x003803c0, 0x003803c0, 0x003fc000}},
  {{0x00384e45, 0x00384e45, 0x00384e45, 0x003fc000}},
  {{0x003898ca, 0x003898ca, 0x003898ca, 0x003fc000}},
  {{0x0038e34f, 0x0038e34f, 0x0038e34f, 0x003fc000}},
  {{0x00392dd4, 0x00392dd4, 0x00392dd4, 0x003fc000}},
  {{0x00397859, 0x00397859, 0x00397859, 0x003fc000}},
  {{0x0039c2de, 0x0039c2de, 0x0039c2de, 0x003fc000}},
  {{0x003a0d63, 0x003a0d63, 0x003a0d63, 0x003fc000}},
  {{0x003a57e8, 0x003a57e8, 0x003a57e8, 0x003fc000}},
  {{0x003aa26d, 0x003aa26d, 0x003aa26d, 0x003fc000}},
  {{0x003aecf2, 0x003aecf2, 0x003aecf2, 0x003fc000}},
  {{0x003b3777, 0x003b3777, 0x003b3777, 0x003fc000}},
  {{0x003b81fc, 0x003b81fc, 0x003b81fc, 0x003fc000}},
  {{0x003bcc81, 0x003bcc81, 0x003bcc81, 0x003fc000}},
  {{0x003c1706, 0x003c1706, 0x003c1706, 0x003fc000}},
  {{0x003c618b, 0x003c618b, 0x003c618b, 0x003fc000}},
  {{0x003cac10, 0x003cac10, 0x003cac10, 0x003fc000}},
  {{0x003cf695, 0x003cf695, 0x003cf695, 0x003fc000}},
  {{0x003d411a, 0x003d411a, 0x003d411a, 0x003fc000}},
  {{0x003d8b9f, 0x003d8b9f, 0x003d8b9f, 0x003fc000}},
  {{0x003dd624, 0x003dd624, 0x003dd624, 0x003fc000}},
  {{0x003e20a9, 0x003e20a9, 0x003e20a9, 0x003fc000}},
  {{0x003e6b2e, 0x003e6b2e, 0x003e6b2e, 0x003fc000}},
  {{0x003eb5b3, 0x003eb5b3, 0x003eb5b3, 0x003fc000}},
  {{0x003f0038, 0x003f0038, 0x003f0038, 0x003fc000}},
  {{0x003f4abd, 0x003f4abd, 0x003f4abd, 0x003fc000}},
  {{0x003f9542, 0x003f9542, 0x003f9542, 0x003fc000}},
  {{0x003fdfc7, 0x003fdfc7, 0x003fdfc7, 0x003fc000}},
  {{0x00402a4c, 0x00402a4c, 0x00402a4c, 0x003fc000}},
  {{0x004074d1, 0x004074d1, 0x004074d1, 0x003fc000}},
  {{0x0040bf56, 0x0040bf56, 0x0040bf56, 0x003fc000}},
  {{0x004109db, 0x004109db, 0x004109db, 0x003fc000}},
  {{0x00415460, 0x00415460, 0x00415460, 0x003fc000}},
  {{0x00419ee5, 0x00419ee5, 0x00419ee5, 0x003fc000}},
  {{0x0041e96a, 0x0041e96a, 0x0041e96a, 0x003fc000}},
  {{0x004233ef, 0x004233ef, 0x004233ef, 0x003fc000}},
  {{0x00427e74, 0x00427e74, 0x00427e74, 0x003fc000}},
  {{0x0042c8f9, 0x0042c8f9, 0x0042c8f9, 0x003fc000}},
  {{0x0043137e, 0x0043137e, 0x0043137e, 0x003fc000}},
  {{0x00435e03, 0x00435e03, 0x00435e03, 0x003fc000}},
  {{0x0043a888, 0x0043a888, 0x0043a888, 0x003fc000}},
  {{0x0043f30d, 0x0043f30d, 0x0043f30d, 0x003fc000}},
  {{0x00443d92, 0x00443d92, 0x00443d92, 0x003fc000}},
  {{0x00448817, 0x00448817, 0x00448817, 0x003fc000}},
  {{0x0044d29c, 0x0044d29c, 0x0044d29c, 0x003fc000}},
  {{0x00451d21, 0x00451d21, 0x00451d21, 0x003fc000}},
  {{0x004567a6, 0x004567a6, 0x004567a6, 0x003fc000}},
  {{0x0045b22b, 0x0045b22b, 0x0045b22b, 0x003fc000}}
};

static const VP8kCstSSE2 VP8kUtoRGBA[256] = {
  {{0, 0x000c8980, 0xffbf7300, 0}}, {{0, 0x000c706d, 0xffbff41a, 0}},
  {{0, 0x000c575a, 0xffc07534, 0}}, {{0, 0x000c3e47, 0xffc0f64e, 0}},
  {{0, 0x000c2534, 0xffc17768, 0}}, {{0, 0x000c0c21, 0xffc1f882, 0}},
  {{0, 0x000bf30e, 0xffc2799c, 0}}, {{0, 0x000bd9fb, 0xffc2fab6, 0}},
  {{0, 0x000bc0e8, 0xffc37bd0, 0}}, {{0, 0x000ba7d5, 0xffc3fcea, 0}},
  {{0, 0x000b8ec2, 0xffc47e04, 0}}, {{0, 0x000b75af, 0xffc4ff1e, 0}},
  {{0, 0x000b5c9c, 0xffc58038, 0}}, {{0, 0x000b4389, 0xffc60152, 0}},
  {{0, 0x000b2a76, 0xffc6826c, 0}}, {{0, 0x000b1163, 0xffc70386, 0}},
  {{0, 0x000af850, 0xffc784a0, 0}}, {{0, 0x000adf3d, 0xffc805ba, 0}},
  {{0, 0x000ac62a, 0xffc886d4, 0}}, {{0, 0x000aad17, 0xffc907ee, 0}},
  {{0, 0x000a9404, 0xffc98908, 0}}, {{0, 0x000a7af1, 0xffca0a22, 0}},
  {{0, 0x000a61de, 0xffca8b3c, 0}}, {{0, 0x000a48cb, 0xffcb0c56, 0}},
  {{0, 0x000a2fb8, 0xffcb8d70, 0}}, {{0, 0x000a16a5, 0xffcc0e8a, 0}},
  {{0, 0x0009fd92, 0xffcc8fa4, 0}}, {{0, 0x0009e47f, 0xffcd10be, 0}},
  {{0, 0x0009cb6c, 0xffcd91d8, 0}}, {{0, 0x0009b259, 0xffce12f2, 0}},
  {{0, 0x00099946, 0xffce940c, 0}}, {{0, 0x00098033, 0xffcf1526, 0}},
  {{0, 0x00096720, 0xffcf9640, 0}}, {{0, 0x00094e0d, 0xffd0175a, 0}},
  {{0, 0x000934fa, 0xffd09874, 0}}, {{0, 0x00091be7, 0xffd1198e, 0}},
  {{0, 0x000902d4, 0xffd19aa8, 0}}, {{0, 0x0008e9c1, 0xffd21bc2, 0}},
  {{0, 0x0008d0ae, 0xffd29cdc, 0}}, {{0, 0x0008b79b, 0xffd31df6, 0}},
  {{0, 0x00089e88, 0xffd39f10, 0}}, {{0, 0x00088575, 0xffd4202a, 0}},
  {{0, 0x00086c62, 0xffd4a144, 0}}, {{0, 0x0008534f, 0xffd5225e, 0}},
  {{0, 0x00083a3c, 0xffd5a378, 0}}, {{0, 0x00082129, 0xffd62492, 0}},
  {{0, 0x00080816, 0xffd6a5ac, 0}}, {{0, 0x0007ef03, 0xffd726c6, 0}},
  {{0, 0x0007d5f0, 0xffd7a7e0, 0}}, {{0, 0x0007bcdd, 0xffd828fa, 0}},
  {{0, 0x0007a3ca, 0xffd8aa14, 0}}, {{0, 0x00078ab7, 0xffd92b2e, 0}},
  {{0, 0x000771a4, 0xffd9ac48, 0}}, {{0, 0x00075891, 0xffda2d62, 0}},
  {{0, 0x00073f7e, 0xffdaae7c, 0}}, {{0, 0x0007266b, 0xffdb2f96, 0}},
  {{0, 0x00070d58, 0xffdbb0b0, 0}}, {{0, 0x0006f445, 0xffdc31ca, 0}},
  {{0, 0x0006db32, 0xffdcb2e4, 0}}, {{0, 0x0006c21f, 0xffdd33fe, 0}},
  {{0, 0x0006a90c, 0xffddb518, 0}}, {{0, 0x00068ff9, 0xffde3632, 0}},
  {{0, 0x000676e6, 0xffdeb74c, 0}}, {{0, 0x00065dd3, 0xffdf3866, 0}},
  {{0, 0x000644c0, 0xffdfb980, 0}}, {{0, 0x00062bad, 0xffe03a9a, 0}},
  {{0, 0x0006129a, 0xffe0bbb4, 0}}, {{0, 0x0005f987, 0xffe13cce, 0}},
  {{0, 0x0005e074, 0xffe1bde8, 0}}, {{0, 0x0005c761, 0xffe23f02, 0}},
  {{0, 0x0005ae4e, 0xffe2c01c, 0}}, {{0, 0x0005953b, 0xffe34136, 0}},
  {{0, 0x00057c28, 0xffe3c250, 0}}, {{0, 0x00056315, 0xffe4436a, 0}},
  {{0, 0x00054a02, 0xffe4c484, 0}}, {{0, 0x000530ef, 0xffe5459e, 0}},
  {{0, 0x000517dc, 0xffe5c6b8, 0}}, {{0, 0x0004fec9, 0xffe647d2, 0}},
  {{0, 0x0004e5b6, 0xffe6c8ec, 0}}, {{0, 0x0004cca3, 0xffe74a06, 0}},
  {{0, 0x0004b390, 0xffe7cb20, 0}}, {{0, 0x00049a7d, 0xffe84c3a, 0}},
  {{0, 0x0004816a, 0xffe8cd54, 0}}, {{0, 0x00046857, 0xffe94e6e, 0}},
  {{0, 0x00044f44, 0xffe9cf88, 0}}, {{0, 0x00043631, 0xffea50a2, 0}},
  {{0, 0x00041d1e, 0xffead1bc, 0}}, {{0, 0x0004040b, 0xffeb52d6, 0}},
  {{0, 0x0003eaf8, 0xffebd3f0, 0}}, {{0, 0x0003d1e5, 0xffec550a, 0}},
  {{0, 0x0003b8d2, 0xffecd624, 0}}, {{0, 0x00039fbf, 0xffed573e, 0}},
  {{0, 0x000386ac, 0xffedd858, 0}}, {{0, 0x00036d99, 0xffee5972, 0}},
  {{0, 0x00035486, 0xffeeda8c, 0}}, {{0, 0x00033b73, 0xffef5ba6, 0}},
  {{0, 0x00032260, 0xffefdcc0, 0}}, {{0, 0x0003094d, 0xfff05dda, 0}},
  {{0, 0x0002f03a, 0xfff0def4, 0}}, {{0, 0x0002d727, 0xfff1600e, 0}},
  {{0, 0x0002be14, 0xfff1e128, 0}}, {{0, 0x0002a501, 0xfff26242, 0}},
  {{0, 0x00028bee, 0xfff2e35c, 0}}, {{0, 0x000272db, 0xfff36476, 0}},
  {{0, 0x000259c8, 0xfff3e590, 0}}, {{0, 0x000240b5, 0xfff466aa, 0}},
  {{0, 0x000227a2, 0xfff4e7c4, 0}}, {{0, 0x00020e8f, 0xfff568de, 0}},
  {{0, 0x0001f57c, 0xfff5e9f8, 0}}, {{0, 0x0001dc69, 0xfff66b12, 0}},
  {{0, 0x0001c356, 0xfff6ec2c, 0}}, {{0, 0x0001aa43, 0xfff76d46, 0}},
  {{0, 0x00019130, 0xfff7ee60, 0}}, {{0, 0x0001781d, 0xfff86f7a, 0}},
  {{0, 0x00015f0a, 0xfff8f094, 0}}, {{0, 0x000145f7, 0xfff971ae, 0}},
  {{0, 0x00012ce4, 0xfff9f2c8, 0}}, {{0, 0x000113d1, 0xfffa73e2, 0}},
  {{0, 0x0000fabe, 0xfffaf4fc, 0}}, {{0, 0x0000e1ab, 0xfffb7616, 0}},
  {{0, 0x0000c898, 0xfffbf730, 0}}, {{0, 0x0000af85, 0xfffc784a, 0}},
  {{0, 0x00009672, 0xfffcf964, 0}}, {{0, 0x00007d5f, 0xfffd7a7e, 0}},
  {{0, 0x0000644c, 0xfffdfb98, 0}}, {{0, 0x00004b39, 0xfffe7cb2, 0}},
  {{0, 0x00003226, 0xfffefdcc, 0}}, {{0, 0x00001913, 0xffff7ee6, 0}},
  {{0, 0x00000000, 0x00000000, 0}}, {{0, 0xffffe6ed, 0x0000811a, 0}},
  {{0, 0xffffcdda, 0x00010234, 0}}, {{0, 0xffffb4c7, 0x0001834e, 0}},
  {{0, 0xffff9bb4, 0x00020468, 0}}, {{0, 0xffff82a1, 0x00028582, 0}},
  {{0, 0xffff698e, 0x0003069c, 0}}, {{0, 0xffff507b, 0x000387b6, 0}},
  {{0, 0xffff3768, 0x000408d0, 0}}, {{0, 0xffff1e55, 0x000489ea, 0}},
  {{0, 0xffff0542, 0x00050b04, 0}}, {{0, 0xfffeec2f, 0x00058c1e, 0}},
  {{0, 0xfffed31c, 0x00060d38, 0}}, {{0, 0xfffeba09, 0x00068e52, 0}},
  {{0, 0xfffea0f6, 0x00070f6c, 0}}, {{0, 0xfffe87e3, 0x00079086, 0}},
  {{0, 0xfffe6ed0, 0x000811a0, 0}}, {{0, 0xfffe55bd, 0x000892ba, 0}},
  {{0, 0xfffe3caa, 0x000913d4, 0}}, {{0, 0xfffe2397, 0x000994ee, 0}},
  {{0, 0xfffe0a84, 0x000a1608, 0}}, {{0, 0xfffdf171, 0x000a9722, 0}},
  {{0, 0xfffdd85e, 0x000b183c, 0}}, {{0, 0xfffdbf4b, 0x000b9956, 0}},
  {{0, 0xfffda638, 0x000c1a70, 0}}, {{0, 0xfffd8d25, 0x000c9b8a, 0}},
  {{0, 0xfffd7412, 0x000d1ca4, 0}}, {{0, 0xfffd5aff, 0x000d9dbe, 0}},
  {{0, 0xfffd41ec, 0x000e1ed8, 0}}, {{0, 0xfffd28d9, 0x000e9ff2, 0}},
  {{0, 0xfffd0fc6, 0x000f210c, 0}}, {{0, 0xfffcf6b3, 0x000fa226, 0}},
  {{0, 0xfffcdda0, 0x00102340, 0}}, {{0, 0xfffcc48d, 0x0010a45a, 0}},
  {{0, 0xfffcab7a, 0x00112574, 0}}, {{0, 0xfffc9267, 0x0011a68e, 0}},
  {{0, 0xfffc7954, 0x001227a8, 0}}, {{0, 0xfffc6041, 0x0012a8c2, 0}},
  {{0, 0xfffc472e, 0x001329dc, 0}}, {{0, 0xfffc2e1b, 0x0013aaf6, 0}},
  {{0, 0xfffc1508, 0x00142c10, 0}}, {{0, 0xfffbfbf5, 0x0014ad2a, 0}},
  {{0, 0xfffbe2e2, 0x00152e44, 0}}, {{0, 0xfffbc9cf, 0x0015af5e, 0}},
  {{0, 0xfffbb0bc, 0x00163078, 0}}, {{0, 0xfffb97a9, 0x0016b192, 0}},
  {{0, 0xfffb7e96, 0x001732ac, 0}}, {{0, 0xfffb6583, 0x0017b3c6, 0}},
  {{0, 0xfffb4c70, 0x001834e0, 0}}, {{0, 0xfffb335d, 0x0018b5fa, 0}},
  {{0, 0xfffb1a4a, 0x00193714, 0}}, {{0, 0xfffb0137, 0x0019b82e, 0}},
  {{0, 0xfffae824, 0x001a3948, 0}}, {{0, 0xfffacf11, 0x001aba62, 0}},
  {{0, 0xfffab5fe, 0x001b3b7c, 0}}, {{0, 0xfffa9ceb, 0x001bbc96, 0}},
  {{0, 0xfffa83d8, 0x001c3db0, 0}}, {{0, 0xfffa6ac5, 0x001cbeca, 0}},
  {{0, 0xfffa51b2, 0x001d3fe4, 0}}, {{0, 0xfffa389f, 0x001dc0fe, 0}},
  {{0, 0xfffa1f8c, 0x001e4218, 0}}, {{0, 0xfffa0679, 0x001ec332, 0}},
  {{0, 0xfff9ed66, 0x001f444c, 0}}, {{0, 0xfff9d453, 0x001fc566, 0}},
  {{0, 0xfff9bb40, 0x00204680, 0}}, {{0, 0xfff9a22d, 0x0020c79a, 0}},
  {{0, 0xfff9891a, 0x002148b4, 0}}, {{0, 0xfff97007, 0x0021c9ce, 0}},
  {{0, 0xfff956f4, 0x00224ae8, 0}}, {{0, 0xfff93de1, 0x0022cc02, 0}},
  {{0, 0xfff924ce, 0x00234d1c, 0}}, {{0, 0xfff90bbb, 0x0023ce36, 0}},
  {{0, 0xfff8f2a8, 0x00244f50, 0}}, {{0, 0xfff8d995, 0x0024d06a, 0}},
  {{0, 0xfff8c082, 0x00255184, 0}}, {{0, 0xfff8a76f, 0x0025d29e, 0}},
  {{0, 0xfff88e5c, 0x002653b8, 0}}, {{0, 0xfff87549, 0x0026d4d2, 0}},
  {{0, 0xfff85c36, 0x002755ec, 0}}, {{0, 0xfff84323, 0x0027d706, 0}},
  {{0, 0xfff82a10, 0x00285820, 0}}, {{0, 0xfff810fd, 0x0028d93a, 0}},
  {{0, 0xfff7f7ea, 0x00295a54, 0}}, {{0, 0xfff7ded7, 0x0029db6e, 0}},
  {{0, 0xfff7c5c4, 0x002a5c88, 0}}, {{0, 0xfff7acb1, 0x002adda2, 0}},
  {{0, 0xfff7939e, 0x002b5ebc, 0}}, {{0, 0xfff77a8b, 0x002bdfd6, 0}},
  {{0, 0xfff76178, 0x002c60f0, 0}}, {{0, 0xfff74865, 0x002ce20a, 0}},
  {{0, 0xfff72f52, 0x002d6324, 0}}, {{0, 0xfff7163f, 0x002de43e, 0}},
  {{0, 0xfff6fd2c, 0x002e6558, 0}}, {{0, 0xfff6e419, 0x002ee672, 0}},
  {{0, 0xfff6cb06, 0x002f678c, 0}}, {{0, 0xfff6b1f3, 0x002fe8a6, 0}},
  {{0, 0xfff698e0, 0x003069c0, 0}}, {{0, 0xfff67fcd, 0x0030eada, 0}},
  {{0, 0xfff666ba, 0x00316bf4, 0}}, {{0, 0xfff64da7, 0x0031ed0e, 0}},
  {{0, 0xfff63494, 0x00326e28, 0}}, {{0, 0xfff61b81, 0x0032ef42, 0}},
  {{0, 0xfff6026e, 0x0033705c, 0}}, {{0, 0xfff5e95b, 0x0033f176, 0}},
  {{0, 0xfff5d048, 0x00347290, 0}}, {{0, 0xfff5b735, 0x0034f3aa, 0}},
  {{0, 0xfff59e22, 0x003574c4, 0}}, {{0, 0xfff5850f, 0x0035f5de, 0}},
  {{0, 0xfff56bfc, 0x003676f8, 0}}, {{0, 0xfff552e9, 0x0036f812, 0}},
  {{0, 0xfff539d6, 0x0037792c, 0}}, {{0, 0xfff520c3, 0x0037fa46, 0}},
  {{0, 0xfff507b0, 0x00387b60, 0}}, {{0, 0xfff4ee9d, 0x0038fc7a, 0}},
  {{0, 0xfff4d58a, 0x00397d94, 0}}, {{0, 0xfff4bc77, 0x0039feae, 0}},
  {{0, 0xfff4a364, 0x003a7fc8, 0}}, {{0, 0xfff48a51, 0x003b00e2, 0}},
  {{0, 0xfff4713e, 0x003b81fc, 0}}, {{0, 0xfff4582b, 0x003c0316, 0}},
  {{0, 0xfff43f18, 0x003c8430, 0}}, {{0, 0xfff42605, 0x003d054a, 0}},
  {{0, 0xfff40cf2, 0x003d8664, 0}}, {{0, 0xfff3f3df, 0x003e077e, 0}},
  {{0, 0xfff3dacc, 0x003e8898, 0}}, {{0, 0xfff3c1b9, 0x003f09b2, 0}},
  {{0, 0xfff3a8a6, 0x003f8acc, 0}}, {{0, 0xfff38f93, 0x00400be6, 0}}
};

static VP8kCstSSE2 VP8kVtoRGBA[256] = {
  {{0xffcced80, 0x001a0400, 0, 0}}, {{0xffcd53a5, 0x0019cff8, 0, 0}},
  {{0xffcdb9ca, 0x00199bf0, 0, 0}}, {{0xffce1fef, 0x001967e8, 0, 0}},
  {{0xffce8614, 0x001933e0, 0, 0}}, {{0xffceec39, 0x0018ffd8, 0, 0}},
  {{0xffcf525e, 0x0018cbd0, 0, 0}}, {{0xffcfb883, 0x001897c8, 0, 0}},
  {{0xffd01ea8, 0x001863c0, 0, 0}}, {{0xffd084cd, 0x00182fb8, 0, 0}},
  {{0xffd0eaf2, 0x0017fbb0, 0, 0}}, {{0xffd15117, 0x0017c7a8, 0, 0}},
  {{0xffd1b73c, 0x001793a0, 0, 0}}, {{0xffd21d61, 0x00175f98, 0, 0}},
  {{0xffd28386, 0x00172b90, 0, 0}}, {{0xffd2e9ab, 0x0016f788, 0, 0}},
  {{0xffd34fd0, 0x0016c380, 0, 0}}, {{0xffd3b5f5, 0x00168f78, 0, 0}},
  {{0xffd41c1a, 0x00165b70, 0, 0}}, {{0xffd4823f, 0x00162768, 0, 0}},
  {{0xffd4e864, 0x0015f360, 0, 0}}, {{0xffd54e89, 0x0015bf58, 0, 0}},
  {{0xffd5b4ae, 0x00158b50, 0, 0}}, {{0xffd61ad3, 0x00155748, 0, 0}},
  {{0xffd680f8, 0x00152340, 0, 0}}, {{0xffd6e71d, 0x0014ef38, 0, 0}},
  {{0xffd74d42, 0x0014bb30, 0, 0}}, {{0xffd7b367, 0x00148728, 0, 0}},
  {{0xffd8198c, 0x00145320, 0, 0}}, {{0xffd87fb1, 0x00141f18, 0, 0}},
  {{0xffd8e5d6, 0x0013eb10, 0, 0}}, {{0xffd94bfb, 0x0013b708, 0, 0}},
  {{0xffd9b220, 0x00138300, 0, 0}}, {{0xffda1845, 0x00134ef8, 0, 0}},
  {{0xffda7e6a, 0x00131af0, 0, 0}}, {{0xffdae48f, 0x0012e6e8, 0, 0}},
  {{0xffdb4ab4, 0x0012b2e0, 0, 0}}, {{0xffdbb0d9, 0x00127ed8, 0, 0}},
  {{0xffdc16fe, 0x00124ad0, 0, 0}}, {{0xffdc7d23, 0x001216c8, 0, 0}},
  {{0xffdce348, 0x0011e2c0, 0, 0}}, {{0xffdd496d, 0x0011aeb8, 0, 0}},
  {{0xffddaf92, 0x00117ab0, 0, 0}}, {{0xffde15b7, 0x001146a8, 0, 0}},
  {{0xffde7bdc, 0x001112a0, 0, 0}}, {{0xffdee201, 0x0010de98, 0, 0}},
  {{0xffdf4826, 0x0010aa90, 0, 0}}, {{0xffdfae4b, 0x00107688, 0, 0}},
  {{0xffe01470, 0x00104280, 0, 0}}, {{0xffe07a95, 0x00100e78, 0, 0}},
  {{0xffe0e0ba, 0x000fda70, 0, 0}}, {{0xffe146df, 0x000fa668, 0, 0}},
  {{0xffe1ad04, 0x000f7260, 0, 0}}, {{0xffe21329, 0x000f3e58, 0, 0}},
  {{0xffe2794e, 0x000f0a50, 0, 0}}, {{0xffe2df73, 0x000ed648, 0, 0}},
  {{0xffe34598, 0x000ea240, 0, 0}}, {{0xffe3abbd, 0x000e6e38, 0, 0}},
  {{0xffe411e2, 0x000e3a30, 0, 0}}, {{0xffe47807, 0x000e0628, 0, 0}},
  {{0xffe4de2c, 0x000dd220, 0, 0}}, {{0xffe54451, 0x000d9e18, 0, 0}},
  {{0xffe5aa76, 0x000d6a10, 0, 0}}, {{0xffe6109b, 0x000d3608, 0, 0}},
  {{0xffe676c0, 0x000d0200, 0, 0}}, {{0xffe6dce5, 0x000ccdf8, 0, 0}},
  {{0xffe7430a, 0x000c99f0, 0, 0}}, {{0xffe7a92f, 0x000c65e8, 0, 0}},
  {{0xffe80f54, 0x000c31e0, 0, 0}}, {{0xffe87579, 0x000bfdd8, 0, 0}},
  {{0xffe8db9e, 0x000bc9d0, 0, 0}}, {{0xffe941c3, 0x000b95c8, 0, 0}},
  {{0xffe9a7e8, 0x000b61c0, 0, 0}}, {{0xffea0e0d, 0x000b2db8, 0, 0}},
  {{0xffea7432, 0x000af9b0, 0, 0}}, {{0xffeada57, 0x000ac5a8, 0, 0}},
  {{0xffeb407c, 0x000a91a0, 0, 0}}, {{0xffeba6a1, 0x000a5d98, 0, 0}},
  {{0xffec0cc6, 0x000a2990, 0, 0}}, {{0xffec72eb, 0x0009f588, 0, 0}},
  {{0xffecd910, 0x0009c180, 0, 0}}, {{0xffed3f35, 0x00098d78, 0, 0}},
  {{0xffeda55a, 0x00095970, 0, 0}}, {{0xffee0b7f, 0x00092568, 0, 0}},
  {{0xffee71a4, 0x0008f160, 0, 0}}, {{0xffeed7c9, 0x0008bd58, 0, 0}},
  {{0xffef3dee, 0x00088950, 0, 0}}, {{0xffefa413, 0x00085548, 0, 0}},
  {{0xfff00a38, 0x00082140, 0, 0}}, {{0xfff0705d, 0x0007ed38, 0, 0}},
  {{0xfff0d682, 0x0007b930, 0, 0}}, {{0xfff13ca7, 0x00078528, 0, 0}},
  {{0xfff1a2cc, 0x00075120, 0, 0}}, {{0xfff208f1, 0x00071d18, 0, 0}},
  {{0xfff26f16, 0x0006e910, 0, 0}}, {{0xfff2d53b, 0x0006b508, 0, 0}},
  {{0xfff33b60, 0x00068100, 0, 0}}, {{0xfff3a185, 0x00064cf8, 0, 0}},
  {{0xfff407aa, 0x000618f0, 0, 0}}, {{0xfff46dcf, 0x0005e4e8, 0, 0}},
  {{0xfff4d3f4, 0x0005b0e0, 0, 0}}, {{0xfff53a19, 0x00057cd8, 0, 0}},
  {{0xfff5a03e, 0x000548d0, 0, 0}}, {{0xfff60663, 0x000514c8, 0, 0}},
  {{0xfff66c88, 0x0004e0c0, 0, 0}}, {{0xfff6d2ad, 0x0004acb8, 0, 0}},
  {{0xfff738d2, 0x000478b0, 0, 0}}, {{0xfff79ef7, 0x000444a8, 0, 0}},
  {{0xfff8051c, 0x000410a0, 0, 0}}, {{0xfff86b41, 0x0003dc98, 0, 0}},
  {{0xfff8d166, 0x0003a890, 0, 0}}, {{0xfff9378b, 0x00037488, 0, 0}},
  {{0xfff99db0, 0x00034080, 0, 0}}, {{0xfffa03d5, 0x00030c78, 0, 0}},
  {{0xfffa69fa, 0x0002d870, 0, 0}}, {{0xfffad01f, 0x0002a468, 0, 0}},
  {{0xfffb3644, 0x00027060, 0, 0}}, {{0xfffb9c69, 0x00023c58, 0, 0}},
  {{0xfffc028e, 0x00020850, 0, 0}}, {{0xfffc68b3, 0x0001d448, 0, 0}},
  {{0xfffcced8, 0x0001a040, 0, 0}}, {{0xfffd34fd, 0x00016c38, 0, 0}},
  {{0xfffd9b22, 0x00013830, 0, 0}}, {{0xfffe0147, 0x00010428, 0, 0}},
  {{0xfffe676c, 0x0000d020, 0, 0}}, {{0xfffecd91, 0x00009c18, 0, 0}},
  {{0xffff33b6, 0x00006810, 0, 0}}, {{0xffff99db, 0x00003408, 0, 0}},
  {{0x00000000, 0x00000000, 0, 0}}, {{0x00006625, 0xffffcbf8, 0, 0}},
  {{0x0000cc4a, 0xffff97f0, 0, 0}}, {{0x0001326f, 0xffff63e8, 0, 0}},
  {{0x00019894, 0xffff2fe0, 0, 0}}, {{0x0001feb9, 0xfffefbd8, 0, 0}},
  {{0x000264de, 0xfffec7d0, 0, 0}}, {{0x0002cb03, 0xfffe93c8, 0, 0}},
  {{0x00033128, 0xfffe5fc0, 0, 0}}, {{0x0003974d, 0xfffe2bb8, 0, 0}},
  {{0x0003fd72, 0xfffdf7b0, 0, 0}}, {{0x00046397, 0xfffdc3a8, 0, 0}},
  {{0x0004c9bc, 0xfffd8fa0, 0, 0}}, {{0x00052fe1, 0xfffd5b98, 0, 0}},
  {{0x00059606, 0xfffd2790, 0, 0}}, {{0x0005fc2b, 0xfffcf388, 0, 0}},
  {{0x00066250, 0xfffcbf80, 0, 0}}, {{0x0006c875, 0xfffc8b78, 0, 0}},
  {{0x00072e9a, 0xfffc5770, 0, 0}}, {{0x000794bf, 0xfffc2368, 0, 0}},
  {{0x0007fae4, 0xfffbef60, 0, 0}}, {{0x00086109, 0xfffbbb58, 0, 0}},
  {{0x0008c72e, 0xfffb8750, 0, 0}}, {{0x00092d53, 0xfffb5348, 0, 0}},
  {{0x00099378, 0xfffb1f40, 0, 0}}, {{0x0009f99d, 0xfffaeb38, 0, 0}},
  {{0x000a5fc2, 0xfffab730, 0, 0}}, {{0x000ac5e7, 0xfffa8328, 0, 0}},
  {{0x000b2c0c, 0xfffa4f20, 0, 0}}, {{0x000b9231, 0xfffa1b18, 0, 0}},
  {{0x000bf856, 0xfff9e710, 0, 0}}, {{0x000c5e7b, 0xfff9b308, 0, 0}},
  {{0x000cc4a0, 0xfff97f00, 0, 0}}, {{0x000d2ac5, 0xfff94af8, 0, 0}},
  {{0x000d90ea, 0xfff916f0, 0, 0}}, {{0x000df70f, 0xfff8e2e8, 0, 0}},
  {{0x000e5d34, 0xfff8aee0, 0, 0}}, {{0x000ec359, 0xfff87ad8, 0, 0}},
  {{0x000f297e, 0xfff846d0, 0, 0}}, {{0x000f8fa3, 0xfff812c8, 0, 0}},
  {{0x000ff5c8, 0xfff7dec0, 0, 0}}, {{0x00105bed, 0xfff7aab8, 0, 0}},
  {{0x0010c212, 0xfff776b0, 0, 0}}, {{0x00112837, 0xfff742a8, 0, 0}},
  {{0x00118e5c, 0xfff70ea0, 0, 0}}, {{0x0011f481, 0xfff6da98, 0, 0}},
  {{0x00125aa6, 0xfff6a690, 0, 0}}, {{0x0012c0cb, 0xfff67288, 0, 0}},
  {{0x001326f0, 0xfff63e80, 0, 0}}, {{0x00138d15, 0xfff60a78, 0, 0}},
  {{0x0013f33a, 0xfff5d670, 0, 0}}, {{0x0014595f, 0xfff5a268, 0, 0}},
  {{0x0014bf84, 0xfff56e60, 0, 0}}, {{0x001525a9, 0xfff53a58, 0, 0}},
  {{0x00158bce, 0xfff50650, 0, 0}}, {{0x0015f1f3, 0xfff4d248, 0, 0}},
  {{0x00165818, 0xfff49e40, 0, 0}}, {{0x0016be3d, 0xfff46a38, 0, 0}},
  {{0x00172462, 0xfff43630, 0, 0}}, {{0x00178a87, 0xfff40228, 0, 0}},
  {{0x0017f0ac, 0xfff3ce20, 0, 0}}, {{0x001856d1, 0xfff39a18, 0, 0}},
  {{0x0018bcf6, 0xfff36610, 0, 0}}, {{0x0019231b, 0xfff33208, 0, 0}},
  {{0x00198940, 0xfff2fe00, 0, 0}}, {{0x0019ef65, 0xfff2c9f8, 0, 0}},
  {{0x001a558a, 0xfff295f0, 0, 0}}, {{0x001abbaf, 0xfff261e8, 0, 0}},
  {{0x001b21d4, 0xfff22de0, 0, 0}}, {{0x001b87f9, 0xfff1f9d8, 0, 0}},
  {{0x001bee1e, 0xfff1c5d0, 0, 0}}, {{0x001c5443, 0xfff191c8, 0, 0}},
  {{0x001cba68, 0xfff15dc0, 0, 0}}, {{0x001d208d, 0xfff129b8, 0, 0}},
  {{0x001d86b2, 0xfff0f5b0, 0, 0}}, {{0x001decd7, 0xfff0c1a8, 0, 0}},
  {{0x001e52fc, 0xfff08da0, 0, 0}}, {{0x001eb921, 0xfff05998, 0, 0}},
  {{0x001f1f46, 0xfff02590, 0, 0}}, {{0x001f856b, 0xffeff188, 0, 0}},
  {{0x001feb90, 0xffefbd80, 0, 0}}, {{0x002051b5, 0xffef8978, 0, 0}},
  {{0x0020b7da, 0xffef5570, 0, 0}}, {{0x00211dff, 0xffef2168, 0, 0}},
  {{0x00218424, 0xffeeed60, 0, 0}}, {{0x0021ea49, 0xffeeb958, 0, 0}},
  {{0x0022506e, 0xffee8550, 0, 0}}, {{0x0022b693, 0xffee5148, 0, 0}},
  {{0x00231cb8, 0xffee1d40, 0, 0}}, {{0x002382dd, 0xffede938, 0, 0}},
  {{0x0023e902, 0xffedb530, 0, 0}}, {{0x00244f27, 0xffed8128, 0, 0}},
  {{0x0024b54c, 0xffed4d20, 0, 0}}, {{0x00251b71, 0xffed1918, 0, 0}},
  {{0x00258196, 0xffece510, 0, 0}}, {{0x0025e7bb, 0xffecb108, 0, 0}},
  {{0x00264de0, 0xffec7d00, 0, 0}}, {{0x0026b405, 0xffec48f8, 0, 0}},
  {{0x00271a2a, 0xffec14f0, 0, 0}}, {{0x0027804f, 0xffebe0e8, 0, 0}},
  {{0x0027e674, 0xffebace0, 0, 0}}, {{0x00284c99, 0xffeb78d8, 0, 0}},
  {{0x0028b2be, 0xffeb44d0, 0, 0}}, {{0x002918e3, 0xffeb10c8, 0, 0}},
  {{0x00297f08, 0xffeadcc0, 0, 0}}, {{0x0029e52d, 0xffeaa8b8, 0, 0}},
  {{0x002a4b52, 0xffea74b0, 0, 0}}, {{0x002ab177, 0xffea40a8, 0, 0}},
  {{0x002b179c, 0xffea0ca0, 0, 0}}, {{0x002b7dc1, 0xffe9d898, 0, 0}},
  {{0x002be3e6, 0xffe9a490, 0, 0}}, {{0x002c4a0b, 0xffe97088, 0, 0}},
  {{0x002cb030, 0xffe93c80, 0, 0}}, {{0x002d1655, 0xffe90878, 0, 0}},
  {{0x002d7c7a, 0xffe8d470, 0, 0}}, {{0x002de29f, 0xffe8a068, 0, 0}},
  {{0x002e48c4, 0xffe86c60, 0, 0}}, {{0x002eaee9, 0xffe83858, 0, 0}},
  {{0x002f150e, 0xffe80450, 0, 0}}, {{0x002f7b33, 0xffe7d048, 0, 0}},
  {{0x002fe158, 0xffe79c40, 0, 0}}, {{0x0030477d, 0xffe76838, 0, 0}},
  {{0x0030ada2, 0xffe73430, 0, 0}}, {{0x003113c7, 0xffe70028, 0, 0}},
  {{0x003179ec, 0xffe6cc20, 0, 0}}, {{0x0031e011, 0xffe69818, 0, 0}},
  {{0x00324636, 0xffe66410, 0, 0}}, {{0x0032ac5b, 0xffe63008, 0, 0}}
};
