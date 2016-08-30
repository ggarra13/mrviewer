;  Copyright 1999-2013 ImageMagick Studio LLC, a non-profit organization
;  dedicated to making software imaging solutions freely available.
;
;  You may not use this file except in compliance with the License.  You may
;  obtain a copy of the License at
;
;    http://www.imagemagick.org/script/license.php
;
;  Unless required by applicable law or agreed to in writing, software
;  distributed under the License is distributed on an "AS IS" BASIS,
;  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
;  See the License for the specific language governing permissions and
;  limitations under the License.
;
;  Copyright (C) 2003 - 2008 GraphicsMagick Group

; #pragma option -v+
; #pragma verboselevel 4
; #define DEBUG 1

#define  public MagickDynamicPackage 1
#define  public MagickHDRI 1
#define  public QuantumDepth "16"
#include "inc\body.isx"

#ifdef Debug
  #expr SaveToFile(AddBackslash(SourcePath) + "im-x86-hdri-dll-Q16.isp")
#endif
