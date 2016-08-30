/*
  Copyright 1999-2016 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.
  
  You may not use this file except in compliance with the License.
  obtain a copy of the License at
  
    http://www.imagemagick.org/script/license.php
  
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  MagickCore cipher methods.
*/
#ifndef _MAGICKCORE_CIPHER_H
#define _MAGICKCORE_CIPHER_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

extern MagickExport MagickBooleanType
  DecipherImage(Image *,const char *,ExceptionInfo *),
  EncipherImage(Image *,const char *,ExceptionInfo *),
  PasskeyDecipherImage(Image *,const StringInfo *,ExceptionInfo *),
  PasskeyEncipherImage(Image *,const StringInfo *,ExceptionInfo *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
