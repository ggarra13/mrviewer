/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%        SSSSS  TTTTT   AAA   TTTTT  IIIII  SSSSS  TTTTT  IIIII   CCCC        %
%        SS       T    A   A    T      I    SS       T      I    C            %
%         SSS     T    AAAAA    T      I     SSS     T      I    C            %
%           SS    T    A   A    T      I       SS    T      I    C            %
%        SSSSS    T    A   A    T    IIIII  SSSSS    T    IIIII   CCCC        %
%                                                                             %
%                                                                             %
%                     MagickCore Image Statistical Methods                    %
%                                                                             %
%                              Software Design                                %
%                                   Cristy                                    %
%                                 July 1992                                   %
%                                                                             %
%                                                                             %
%  Copyright 1999-2016 ImageMagick Studio LLC, a non-profit organization      %
%  dedicated to making software imaging solutions freely available.           %
%                                                                             %
%  You may not use this file except in compliance with the License.  You may  %
%  obtain a copy of the License at                                            %
%                                                                             %
%    http://www.imagemagick.org/script/license.php                            %
%                                                                             %
%  Unless required by applicable law or agreed to in writing, software        %
%  distributed under the License is distributed on an "AS IS" BASIS,          %
%  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   %
%  See the License for the specific language governing permissions and        %
%  limitations under the License.                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%
%
*/


/*
  Include declarations.
*/
#include "magick/studio.h"
#include "magick/accelerate.h"
#include "magick/animate.h"
#include "magick/animate.h"
#include "magick/blob.h"
#include "magick/blob-private.h"
#include "magick/cache.h"
#include "magick/cache-private.h"
#include "magick/cache-view.h"
#include "magick/client.h"
#include "magick/color.h"
#include "magick/color-private.h"
#include "magick/colorspace.h"
#include "magick/colorspace-private.h"
#include "magick/composite.h"
#include "magick/composite-private.h"
#include "magick/compress.h"
#include "magick/constitute.h"
#include "magick/deprecate.h"
#include "magick/display.h"
#include "magick/draw.h"
#include "magick/enhance.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/gem.h"
#include "magick/geometry.h"
#include "magick/list.h"
#include "magick/image-private.h"
#include "magick/magic.h"
#include "magick/magick.h"
#include "magick/memory_.h"
#include "magick/module.h"
#include "magick/monitor.h"
#include "magick/monitor-private.h"
#include "magick/option.h"
#include "magick/paint.h"
#include "magick/pixel-private.h"
#include "magick/profile.h"
#include "magick/property.h"
#include "magick/quantize.h"
#include "magick/random_.h"
#include "magick/random-private.h"
#include "magick/resource_.h"
#include "magick/segment.h"
#include "magick/semaphore.h"
#include "magick/signature-private.h"
#include "magick/statistic.h"
#include "magick/string_.h"
#include "magick/thread-private.h"
#include "magick/timer.h"
#include "magick/utility.h"
#include "magick/version.h"

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     E v a l u a t e I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  EvaluateImage() applies a value to the image with an arithmetic, relational,
%  or logical operator to an image. Use these operations to lighten or darken
%  an image, to increase or decrease contrast in an image, or to produce the
%  "negative" of an image.
%
%  The format of the EvaluateImageChannel method is:
%
%      MagickBooleanType EvaluateImage(Image *image,
%        const MagickEvaluateOperator op,const double value,
%        ExceptionInfo *exception)
%      MagickBooleanType EvaluateImages(Image *images,
%        const MagickEvaluateOperator op,const double value,
%        ExceptionInfo *exception)
%      MagickBooleanType EvaluateImageChannel(Image *image,
%        const ChannelType channel,const MagickEvaluateOperator op,
%        const double value,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o channel: the channel.
%
%    o op: A channel op.
%
%    o value: A value value.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static MagickPixelPacket **DestroyPixelThreadSet(MagickPixelPacket **pixels)
{
  register ssize_t
    i;

  assert(pixels != (MagickPixelPacket **) NULL);
  for (i=0; i < (ssize_t) GetMagickResourceLimit(ThreadResource); i++)
    if (pixels[i] != (MagickPixelPacket *) NULL)
      pixels[i]=(MagickPixelPacket *) RelinquishMagickMemory(pixels[i]);
  pixels=(MagickPixelPacket **) RelinquishMagickMemory(pixels);
  return(pixels);
}

static MagickPixelPacket **AcquirePixelThreadSet(const Image *image,
  const size_t number_images)
{
  register ssize_t
    i,
    j;

  MagickPixelPacket
    **pixels;

  size_t
    length,
    number_threads;

  number_threads=(size_t) GetMagickResourceLimit(ThreadResource);
  pixels=(MagickPixelPacket **) AcquireQuantumMemory(number_threads,
    sizeof(*pixels));
  if (pixels == (MagickPixelPacket **) NULL)
    return((MagickPixelPacket **) NULL);
  (void) ResetMagickMemory(pixels,0,number_threads*sizeof(*pixels));
  for (i=0; i < (ssize_t) number_threads; i++)
  {
    length=image->columns;
    if (length < number_images)
      length=number_images;
    pixels[i]=(MagickPixelPacket *) AcquireQuantumMemory(length,
      sizeof(**pixels));
    if (pixels[i] == (MagickPixelPacket *) NULL)
      return(DestroyPixelThreadSet(pixels));
    for (j=0; j < (ssize_t) length; j++)
      GetMagickPixelPacket(image,&pixels[i][j]);
  }
  return(pixels);
}

static inline double EvaluateMax(const double x,const double y)
{
  if (x > y)
    return(x);
  return(y);
}

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

static int IntensityCompare(const void *x,const void *y)
{
  const MagickPixelPacket
    *color_1,
    *color_2;

  int
    intensity;

  color_1=(const MagickPixelPacket *) x;
  color_2=(const MagickPixelPacket *) y;
  intensity=(int) MagickPixelIntensity(color_2)-
    (int) MagickPixelIntensity(color_1);
  return(intensity);
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

static MagickRealType ApplyEvaluateOperator(RandomInfo *random_info,
  const Quantum pixel,const MagickEvaluateOperator op,
  const MagickRealType value)
{
  MagickRealType
    result;

  result=0.0;
  switch (op)
  {
    case UndefinedEvaluateOperator:
      break;
    case AbsEvaluateOperator:
    {
      result=(MagickRealType) fabs((double) (pixel+value));
      break;
    }
    case AddEvaluateOperator:
    {
      result=(MagickRealType) (pixel+value);
      break;
    }
    case AddModulusEvaluateOperator:
    {
      /*
        This returns a 'floored modulus' of the addition which is a
        positive result.  It differs from  % or fmod() which returns a
        'truncated modulus' result, where floor() is replaced by trunc()
        and could return a negative result (which is clipped).
      */
      result=pixel+value;
      result-=(QuantumRange+1.0)*floor((double) result/(QuantumRange+1.0));
      break;
    }
    case AndEvaluateOperator:
    {
      result=(MagickRealType) ((size_t) pixel & (size_t) (value+0.5));
      break;
    }
    case CosineEvaluateOperator:
    {
      result=(MagickRealType) (QuantumRange*(0.5*cos((double) (2.0*MagickPI*
        QuantumScale*pixel*value))+0.5));
      break;
    }
    case DivideEvaluateOperator:
    {
      result=pixel/(value == 0.0 ? 1.0 : value);
      break;
    }
    case ExponentialEvaluateOperator:
    {
      result=(MagickRealType) (QuantumRange*exp((double) (value*QuantumScale*
        pixel)));
      break;
    }
    case GaussianNoiseEvaluateOperator:
    {
      result=(MagickRealType) GenerateDifferentialNoise(random_info,pixel,
        GaussianNoise,value);
      break;
    }
    case ImpulseNoiseEvaluateOperator:
    {
      result=(MagickRealType) GenerateDifferentialNoise(random_info,pixel,
        ImpulseNoise,value);
      break;
    }
    case LaplacianNoiseEvaluateOperator:
    {
      result=(MagickRealType) GenerateDifferentialNoise(random_info,pixel,
        LaplacianNoise,value);
      break;
    }
    case LeftShiftEvaluateOperator:
    {
      result=(MagickRealType) ((size_t) pixel << (size_t) (value+0.5));
      break;
    }
    case LogEvaluateOperator:
    {
      if ((QuantumScale*pixel) >= MagickEpsilon)
        result=(MagickRealType) (QuantumRange*log((double) (QuantumScale*value*
          pixel+1.0))/log((double) (value+1.0)));
      break;
    }
    case MaxEvaluateOperator:
    {
      result=(MagickRealType) EvaluateMax((double) pixel,value);
      break;
    }
    case MeanEvaluateOperator:
    {
      result=(MagickRealType) (pixel+value);
      break;
    }
    case MedianEvaluateOperator:
    {
      result=(MagickRealType) (pixel+value);
      break;
    }
    case MinEvaluateOperator:
    {
      result=(MagickRealType) MagickMin((double) pixel,value);
      break;
    }
    case MultiplicativeNoiseEvaluateOperator:
    {
      result=(MagickRealType) GenerateDifferentialNoise(random_info,pixel,
        MultiplicativeGaussianNoise,value);
      break;
    }
    case MultiplyEvaluateOperator:
    {
      result=(MagickRealType) (value*pixel);
      break;
    }
    case OrEvaluateOperator:
    {
      result=(MagickRealType) ((size_t) pixel | (size_t) (value+0.5));
      break;
    }
    case PoissonNoiseEvaluateOperator:
    {
      result=(MagickRealType) GenerateDifferentialNoise(random_info,pixel,
        PoissonNoise,value);
      break;
    }
    case PowEvaluateOperator:
    {
      result=(MagickRealType) (QuantumRange*pow((double) (QuantumScale*pixel),
        (double) value));
      break;
    }
    case RightShiftEvaluateOperator:
    {
      result=(MagickRealType) ((size_t) pixel >> (size_t) (value+0.5));
      break;
    }
    case RootMeanSquareEvaluateOperator:
    {
      result=(MagickRealType) (pixel*pixel+value);
      break;
    }
    case SetEvaluateOperator:
    {
      result=value;
      break;
    }
    case SineEvaluateOperator:
    {
      result=(MagickRealType) (QuantumRange*(0.5*sin((double) (2.0*MagickPI*
        QuantumScale*pixel*value))+0.5));
      break;
    }
    case SubtractEvaluateOperator:
    {
      result=(MagickRealType) (pixel-value);
      break;
    }
    case SumEvaluateOperator:
    {
      result=(MagickRealType) (pixel+value);
      break;
    }
    case ThresholdEvaluateOperator:
    {
      result=(MagickRealType) (((MagickRealType) pixel <= value) ? 0 :
        QuantumRange);
      break;
    }
    case ThresholdBlackEvaluateOperator:
    {
      result=(MagickRealType) (((MagickRealType) pixel <= value) ? 0 : pixel);
      break;
    }
    case ThresholdWhiteEvaluateOperator:
    {
      result=(MagickRealType) (((MagickRealType) pixel > value) ? QuantumRange :
        pixel);
      break;
    }
    case UniformNoiseEvaluateOperator:
    {
      result=(MagickRealType) GenerateDifferentialNoise(random_info,pixel,
        UniformNoise,value);
      break;
    }
    case XorEvaluateOperator:
    {
      result=(MagickRealType) ((size_t) pixel ^ (size_t) (value+0.5));
      break;
    }
  }
  return(result);
}

MagickExport MagickBooleanType EvaluateImage(Image *image,
  const MagickEvaluateOperator op,const double value,ExceptionInfo *exception)
{
  MagickBooleanType
    status;

  status=EvaluateImageChannel(image,CompositeChannels,op,value,exception);
  return(status);
}

MagickExport Image *EvaluateImages(const Image *images,
  const MagickEvaluateOperator op,ExceptionInfo *exception)
{
#define EvaluateImageTag  "Evaluate/Image"

  CacheView
    *evaluate_view;

  Image
    *image;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  MagickPixelPacket
    **magick_restrict evaluate_pixels,
    zero;

  RandomInfo
    **magick_restrict random_info;

  size_t
    number_images;

  ssize_t
    y;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
  unsigned long
    key;
#endif

  assert(images != (Image *) NULL);
  assert(images->signature == MagickSignature);
  if (images->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",images->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  image=CloneImage(images,images->columns,images->rows,MagickTrue,exception);
  if (image == (Image *) NULL)
    return((Image *) NULL);
  if (SetImageStorageClass(image,DirectClass) == MagickFalse)
    {
      InheritException(exception,&image->exception);
      image=DestroyImage(image);
      return((Image *) NULL);
    }
  number_images=GetImageListLength(images);
  evaluate_pixels=AcquirePixelThreadSet(images,number_images);
  if (evaluate_pixels == (MagickPixelPacket **) NULL)
    {
      image=DestroyImage(image);
      (void) ThrowMagickException(exception,GetMagickModule(),
        ResourceLimitError,"MemoryAllocationFailed","`%s'",images->filename);
      return((Image *) NULL);
    }
  /*
    Evaluate image pixels.
  */
  status=MagickTrue;
  progress=0;
  GetMagickPixelPacket(images,&zero);
  random_info=AcquireRandomInfoThreadSet();
  evaluate_view=AcquireAuthenticCacheView(image,exception);
  if (op == MedianEvaluateOperator)
    {
#if defined(MAGICKCORE_OPENMP_SUPPORT)
      key=GetRandomSecretKey(random_info[0]);
      #pragma omp parallel for schedule(static,4) shared(progress,status) \
        magick_threads(image,images,image->rows,key == ~0UL)
#endif
      for (y=0; y < (ssize_t) image->rows; y++)
      {
        CacheView
          *image_view;

        const Image
          *next;

        const int
          id = GetOpenMPThreadId();

        register IndexPacket
          *magick_restrict evaluate_indexes;

        register MagickPixelPacket
          *evaluate_pixel;

        register PixelPacket
          *magick_restrict q;

        register ssize_t
          x;

        if (status == MagickFalse)
          continue;
        q=QueueCacheViewAuthenticPixels(evaluate_view,0,y,
          image->columns,1,exception);
        if (q == (PixelPacket *) NULL)
          {
            status=MagickFalse;
            continue;
          }
        evaluate_indexes=GetCacheViewAuthenticIndexQueue(evaluate_view);
        evaluate_pixel=evaluate_pixels[id];
        for (x=0; x < (ssize_t) image->columns; x++)
        {
          register ssize_t
            i;

          for (i=0; i < (ssize_t) number_images; i++)
            evaluate_pixel[i]=zero;
          next=images;
          for (i=0; i < (ssize_t) number_images; i++)
          {
            register const IndexPacket
              *indexes;

            register const PixelPacket
              *p;

            image_view=AcquireVirtualCacheView(next,exception);
            p=GetCacheViewVirtualPixels(image_view,x,y,1,1,exception);
            if (p == (const PixelPacket *) NULL)
              {
                image_view=DestroyCacheView(image_view);
                break;
              }
            indexes=GetCacheViewVirtualIndexQueue(image_view);
            evaluate_pixel[i].red=ApplyEvaluateOperator(random_info[id],
              GetPixelRed(p),op,evaluate_pixel[i].red);
            evaluate_pixel[i].green=ApplyEvaluateOperator(random_info[id],
              GetPixelGreen(p),op,evaluate_pixel[i].green);
            evaluate_pixel[i].blue=ApplyEvaluateOperator(random_info[id],
              GetPixelBlue(p),op,evaluate_pixel[i].blue);
            evaluate_pixel[i].opacity=ApplyEvaluateOperator(random_info[id],
              GetPixelAlpha(p),op,evaluate_pixel[i].opacity);
            if (image->colorspace == CMYKColorspace)
              evaluate_pixel[i].index=ApplyEvaluateOperator(random_info[id],
                *indexes,op,evaluate_pixel[i].index);
            image_view=DestroyCacheView(image_view);
            next=GetNextImageInList(next);
          }
          qsort((void *) evaluate_pixel,number_images,sizeof(*evaluate_pixel),
            IntensityCompare);
          SetPixelRed(q,ClampToQuantum(evaluate_pixel[i/2].red));
          SetPixelGreen(q,ClampToQuantum(evaluate_pixel[i/2].green));
          SetPixelBlue(q,ClampToQuantum(evaluate_pixel[i/2].blue));
          SetPixelAlpha(q,ClampToQuantum(evaluate_pixel[i/2].opacity));
          if (image->colorspace == CMYKColorspace)
            SetPixelIndex(evaluate_indexes+i,ClampToQuantum(
              evaluate_pixel[i/2].index));
          q++;
        }
        if (SyncCacheViewAuthenticPixels(evaluate_view,exception) == MagickFalse)
          status=MagickFalse;
        if (images->progress_monitor != (MagickProgressMonitor) NULL)
          {
            MagickBooleanType
              proceed;

#if   defined(MAGICKCORE_OPENMP_SUPPORT)
            #pragma omp critical (MagickCore_EvaluateImages)
#endif
            proceed=SetImageProgress(images,EvaluateImageTag,progress++,
              image->rows);
            if (proceed == MagickFalse)
              status=MagickFalse;
          }
      }
    }
  else
    {
#if defined(MAGICKCORE_OPENMP_SUPPORT)
      key=GetRandomSecretKey(random_info[0]);
      #pragma omp parallel for schedule(static,4) shared(progress,status) \
        magick_threads(image,images,image->rows,key == ~0UL)
#endif
      for (y=0; y < (ssize_t) image->rows; y++)
      {
        CacheView
          *image_view;

        const Image
          *next;

        const int
          id = GetOpenMPThreadId();

        register IndexPacket
          *magick_restrict evaluate_indexes;

        register ssize_t
          i,
          x;

        register MagickPixelPacket
          *evaluate_pixel;

        register PixelPacket
          *magick_restrict q;

        if (status == MagickFalse)
          continue;
        q=QueueCacheViewAuthenticPixels(evaluate_view,0,y,
          image->columns,1,exception);
        if (q == (PixelPacket *) NULL)
          {
            status=MagickFalse;
            continue;
          }
        evaluate_indexes=GetCacheViewAuthenticIndexQueue(evaluate_view);
        evaluate_pixel=evaluate_pixels[id];
        for (x=0; x < (ssize_t) image->columns; x++)
          evaluate_pixel[x]=zero;
        next=images;
        for (i=0; i < (ssize_t) number_images; i++)
        {
          register const IndexPacket
            *indexes;

          register const PixelPacket
            *p;

          image_view=AcquireVirtualCacheView(next,exception);
          p=GetCacheViewVirtualPixels(image_view,0,y,next->columns,1,exception);
          if (p == (const PixelPacket *) NULL)
            {
              image_view=DestroyCacheView(image_view);
              break;
            }
          indexes=GetCacheViewVirtualIndexQueue(image_view);
          for (x=0; x < (ssize_t) next->columns; x++)
          {
            evaluate_pixel[x].red=ApplyEvaluateOperator(random_info[id],
              GetPixelRed(p),i == 0 ? AddEvaluateOperator : op,
              evaluate_pixel[x].red);
            evaluate_pixel[x].green=ApplyEvaluateOperator(random_info[id],
              GetPixelGreen(p),i == 0 ? AddEvaluateOperator : op,
              evaluate_pixel[x].green);
            evaluate_pixel[x].blue=ApplyEvaluateOperator(random_info[id],
              GetPixelBlue(p),i == 0 ? AddEvaluateOperator : op,
              evaluate_pixel[x].blue);
            evaluate_pixel[x].opacity=ApplyEvaluateOperator(random_info[id],
              GetPixelAlpha(p),i == 0 ? AddEvaluateOperator : op,
              evaluate_pixel[x].opacity);
            if (image->colorspace == CMYKColorspace)
              evaluate_pixel[x].index=ApplyEvaluateOperator(random_info[id],
                GetPixelIndex(indexes+x),i == 0 ? AddEvaluateOperator : op,
                evaluate_pixel[x].index);
            p++;
          }
          image_view=DestroyCacheView(image_view);
          next=GetNextImageInList(next);
        }
        if (op == MeanEvaluateOperator)
          for (x=0; x < (ssize_t) image->columns; x++)
          {
            evaluate_pixel[x].red/=number_images;
            evaluate_pixel[x].green/=number_images;
            evaluate_pixel[x].blue/=number_images;
            evaluate_pixel[x].opacity/=number_images;
            evaluate_pixel[x].index/=number_images;
          }
        if (op == RootMeanSquareEvaluateOperator)
          for (x=0; x < (ssize_t) image->columns; x++)
          {
            evaluate_pixel[x].red=sqrt((double) evaluate_pixel[x].red/
              number_images);
            evaluate_pixel[x].green=sqrt((double) evaluate_pixel[x].green/
              number_images);
            evaluate_pixel[x].blue=sqrt((double) evaluate_pixel[x].blue/
              number_images);
            evaluate_pixel[x].opacity=sqrt((double) evaluate_pixel[x].opacity/
              number_images);
            evaluate_pixel[x].index=sqrt((double) evaluate_pixel[x].index/
              number_images);
          }
        if (op == MultiplyEvaluateOperator)
          for (x=0; x < (ssize_t) image->columns; x++)
          {
            register ssize_t
              j;

            for (j=0; j < (ssize_t) (number_images-1); j++)
            {
              evaluate_pixel[x].red*=(MagickRealType) QuantumScale;
              evaluate_pixel[x].green*=(MagickRealType) QuantumScale;
              evaluate_pixel[x].blue*=(MagickRealType) QuantumScale;
              evaluate_pixel[x].opacity*=(MagickRealType) QuantumScale;
              evaluate_pixel[x].index*=(MagickRealType) QuantumScale;
            }
          }
        for (x=0; x < (ssize_t) image->columns; x++)
        {
          SetPixelRed(q,ClampToQuantum(evaluate_pixel[x].red));
          SetPixelGreen(q,ClampToQuantum(evaluate_pixel[x].green));
          SetPixelBlue(q,ClampToQuantum(evaluate_pixel[x].blue));
          SetPixelAlpha(q,ClampToQuantum(evaluate_pixel[x].opacity));
          if (image->colorspace == CMYKColorspace)
            SetPixelIndex(evaluate_indexes+x,ClampToQuantum(
              evaluate_pixel[x].index));
          q++;
        }
        if (SyncCacheViewAuthenticPixels(evaluate_view,exception) == MagickFalse)
          status=MagickFalse;
        if (images->progress_monitor != (MagickProgressMonitor) NULL)
          {
            MagickBooleanType
              proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
            #pragma omp critical (MagickCore_EvaluateImages)
#endif
            proceed=SetImageProgress(images,EvaluateImageTag,progress++,
              image->rows);
            if (proceed == MagickFalse)
              status=MagickFalse;
          }
      }
    }
  evaluate_view=DestroyCacheView(evaluate_view);
  evaluate_pixels=DestroyPixelThreadSet(evaluate_pixels);
  random_info=DestroyRandomInfoThreadSet(random_info);
  if (status == MagickFalse)
    image=DestroyImage(image);
  return(image);
}

MagickExport MagickBooleanType EvaluateImageChannel(Image *image,
  const ChannelType channel,const MagickEvaluateOperator op,const double value,
  ExceptionInfo *exception)
{
  CacheView
    *image_view;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  RandomInfo
    **magick_restrict random_info;

  ssize_t
    y;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
  unsigned long
    key;
#endif

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  if (SetImageStorageClass(image,DirectClass) == MagickFalse)
    {
      InheritException(exception,&image->exception);
      return(MagickFalse);
    }
  status=MagickTrue;
  progress=0;
  random_info=AcquireRandomInfoThreadSet();
  image_view=AcquireAuthenticCacheView(image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  key=GetRandomSecretKey(random_info[0]);
  #pragma omp parallel for schedule(static,4) shared(progress,status) \
    magick_threads(image,image,image->rows,key == ~0UL)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    const int
      id = GetOpenMPThreadId();

    register IndexPacket
      *magick_restrict indexes;

    register PixelPacket
      *magick_restrict q;

    register ssize_t
      x;

    if (status == MagickFalse)
      continue;
    q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,exception);
    if (q == (PixelPacket *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    indexes=GetCacheViewAuthenticIndexQueue(image_view);
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      MagickRealType
        result;

      if ((channel & RedChannel) != 0)
        {
          result=ApplyEvaluateOperator(random_info[id],GetPixelRed(q),op,value);
          if (op == MeanEvaluateOperator)
            result/=2.0;
          SetPixelRed(q,ClampToQuantum(result));
        }
      if ((channel & GreenChannel) != 0)
        {
          result=ApplyEvaluateOperator(random_info[id],GetPixelGreen(q),op,
            value);
          if (op == MeanEvaluateOperator)
            result/=2.0;
          SetPixelGreen(q,ClampToQuantum(result));
        }
      if ((channel & BlueChannel) != 0)
        {
          result=ApplyEvaluateOperator(random_info[id],GetPixelBlue(q),op,
            value);
          if (op == MeanEvaluateOperator)
            result/=2.0;
          SetPixelBlue(q,ClampToQuantum(result));
        }
      if ((channel & OpacityChannel) != 0)
        {
          if (image->matte == MagickFalse)
            {
              result=ApplyEvaluateOperator(random_info[id],GetPixelOpacity(q),
                op,value);
              if (op == MeanEvaluateOperator)
                result/=2.0;
              SetPixelOpacity(q,ClampToQuantum(result));
            }
          else
            {
              result=ApplyEvaluateOperator(random_info[id],GetPixelAlpha(q),
                op,value);
              if (op == MeanEvaluateOperator)
                result/=2.0;
              SetPixelAlpha(q,ClampToQuantum(result));
            }
        }
      if (((channel & IndexChannel) != 0) && (indexes != (IndexPacket *) NULL))
        {
          result=ApplyEvaluateOperator(random_info[id],GetPixelIndex(indexes+x),
            op,value);
          if (op == MeanEvaluateOperator)
            result/=2.0;
          SetPixelIndex(indexes+x,ClampToQuantum(result));
        }
      q++;
    }
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
        #pragma omp critical (MagickCore_EvaluateImageChannel)
#endif
        proceed=SetImageProgress(image,EvaluateImageTag,progress++,image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  image_view=DestroyCacheView(image_view);
  random_info=DestroyRandomInfoThreadSet(random_info);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     F u n c t i o n I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  FunctionImage() applies a value to the image with an arithmetic, relational,
%  or logical operator to an image. Use these operations to lighten or darken
%  an image, to increase or decrease contrast in an image, or to produce the
%  "negative" of an image.
%
%  The format of the FunctionImageChannel method is:
%
%      MagickBooleanType FunctionImage(Image *image,
%        const MagickFunction function,const ssize_t number_parameters,
%        const double *parameters,ExceptionInfo *exception)
%      MagickBooleanType FunctionImageChannel(Image *image,
%        const ChannelType channel,const MagickFunction function,
%        const ssize_t number_parameters,const double *argument,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o channel: the channel.
%
%    o function: A channel function.
%
%    o parameters: one or more parameters.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static Quantum ApplyFunction(Quantum pixel,const MagickFunction function,
  const size_t number_parameters,const double *parameters,
  ExceptionInfo *exception)
{
  MagickRealType
    result;

  register ssize_t
    i;

  (void) exception;
  result=0.0;
  switch (function)
  {
    case PolynomialFunction:
    {
      /*
       * Polynomial
       * Parameters:   polynomial constants,  highest to lowest order
       *   For example:      c0*x^3 + c1*x^2 + c2*x  + c3
       */
      result=0.0;
      for (i=0; i < (ssize_t) number_parameters; i++)
        result=result*QuantumScale*pixel + parameters[i];
      result*=QuantumRange;
      break;
    }
    case SinusoidFunction:
    {
      /* Sinusoid Function
       * Parameters:   Freq, Phase, Ampl, bias
       */
      double  freq,phase,ampl,bias;
      freq  = ( number_parameters >= 1 ) ? parameters[0] : 1.0;
      phase = ( number_parameters >= 2 ) ? parameters[1] : 0.0;
      ampl  = ( number_parameters >= 3 ) ? parameters[2] : 0.5;
      bias  = ( number_parameters >= 4 ) ? parameters[3] : 0.5;
      result=(MagickRealType) (QuantumRange*(ampl*sin((double) (2.0*MagickPI*
        (freq*QuantumScale*pixel + phase/360.0) )) + bias ) );
      break;
    }
    case ArcsinFunction:
    {
      /* Arcsin Function  (peged at range limits for invalid results)
       * Parameters:   Width, Center, Range, Bias
       */
      double  width,range,center,bias;
      width  = ( number_parameters >= 1 ) ? parameters[0] : 1.0;
      center = ( number_parameters >= 2 ) ? parameters[1] : 0.5;
      range  = ( number_parameters >= 3 ) ? parameters[2] : 1.0;
      bias   = ( number_parameters >= 4 ) ? parameters[3] : 0.5;
      result = 2.0/width*(QuantumScale*pixel - center);
      if ( result <= -1.0 )
        result = bias - range/2.0;
      else if ( result >= 1.0 )
        result = bias + range/2.0;
      else
        result=(MagickRealType) (range/MagickPI*asin((double) result)+bias);
      result *= QuantumRange;
      break;
    }
    case ArctanFunction:
    {
      /* Arctan Function
       * Parameters:   Slope, Center, Range, Bias
       */
      double  slope,range,center,bias;
      slope  = ( number_parameters >= 1 ) ? parameters[0] : 1.0;
      center = ( number_parameters >= 2 ) ? parameters[1] : 0.5;
      range  = ( number_parameters >= 3 ) ? parameters[2] : 1.0;
      bias   = ( number_parameters >= 4 ) ? parameters[3] : 0.5;
      result=(MagickRealType) (MagickPI*slope*(QuantumScale*pixel-center));
      result=(MagickRealType) (QuantumRange*(range/MagickPI*atan((double)
                  result) + bias ) );
      break;
    }
    case UndefinedFunction:
      break;
  }
  return(ClampToQuantum(result));
}

MagickExport MagickBooleanType FunctionImage(Image *image,
  const MagickFunction function,const size_t number_parameters,
  const double *parameters,ExceptionInfo *exception)
{
  MagickBooleanType
    status;

  status=FunctionImageChannel(image,CompositeChannels,function,
    number_parameters,parameters,exception);
  return(status);
}

MagickExport MagickBooleanType FunctionImageChannel(Image *image,
  const ChannelType channel,const MagickFunction function,
  const size_t number_parameters,const double *parameters,
  ExceptionInfo *exception)
{
#define FunctionImageTag  "Function/Image "

  CacheView
    *image_view;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  ssize_t
    y;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  if (SetImageStorageClass(image,DirectClass) == MagickFalse)
    {
      InheritException(exception,&image->exception);
      return(MagickFalse);
    }
  status=AccelerateFunctionImage(image,channel,function,number_parameters,
    parameters,exception);
  if (status != MagickFalse)
    return(status);
  status=MagickTrue;
  progress=0;
  image_view=AcquireAuthenticCacheView(image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,4) shared(progress,status) \
    magick_threads(image,image,image->rows,1)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    register IndexPacket
      *magick_restrict indexes;

    register ssize_t
      x;

    register PixelPacket
      *magick_restrict q;

    if (status == MagickFalse)
      continue;
    q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,exception);
    if (q == (PixelPacket *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    indexes=GetCacheViewAuthenticIndexQueue(image_view);
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      if ((channel & RedChannel) != 0)
        SetPixelRed(q,ApplyFunction(GetPixelRed(q),function,
          number_parameters,parameters,exception));
      if ((channel & GreenChannel) != 0)
        SetPixelGreen(q,ApplyFunction(GetPixelGreen(q),function,
          number_parameters,parameters,exception));
      if ((channel & BlueChannel) != 0)
        SetPixelBlue(q,ApplyFunction(GetPixelBlue(q),function,
          number_parameters,parameters,exception));
      if ((channel & OpacityChannel) != 0)
        {
          if (image->matte == MagickFalse)
            SetPixelOpacity(q,ApplyFunction(GetPixelOpacity(q),function,
              number_parameters,parameters,exception));
          else
            SetPixelAlpha(q,ApplyFunction((Quantum) GetPixelAlpha(q),function,
              number_parameters,parameters,exception));
        }
      if (((channel & IndexChannel) != 0) && (indexes != (IndexPacket *) NULL))
        SetPixelIndex(indexes+x,ApplyFunction(GetPixelIndex(indexes+x),function,
          number_parameters,parameters,exception));
      q++;
    }
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
        #pragma omp critical (MagickCore_FunctionImageChannel)
#endif
        proceed=SetImageProgress(image,FunctionImageTag,progress++,image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  image_view=DestroyCacheView(image_view);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t I m a g e C h a n n e l E n t r o p y                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetImageChannelEntropy() returns the entropy of one or more image channels.
%
%  The format of the GetImageChannelEntropy method is:
%
%      MagickBooleanType GetImageChannelEntropy(const Image *image,
%        const ChannelType channel,double *entropy,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o channel: the channel.
%
%    o entropy: the average entropy of the selected channels.
%
%    o exception: return any errors or warnings in this structure.
%
*/

MagickExport MagickBooleanType GetImageEntropy(const Image *image,
  double *entropy,ExceptionInfo *exception)
{
  MagickBooleanType
    status;

  status=GetImageChannelEntropy(image,CompositeChannels,entropy,exception);
  return(status);
}

MagickExport MagickBooleanType GetImageChannelEntropy(const Image *image,
  const ChannelType channel,double *entropy,ExceptionInfo *exception)
{
  ChannelStatistics
    *channel_statistics;

  size_t
    channels;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  channel_statistics=GetImageChannelStatistics(image,exception);
  if (channel_statistics == (ChannelStatistics *) NULL)
    return(MagickFalse);
  channels=0;
  channel_statistics[CompositeChannels].entropy=0.0;
  if ((channel & RedChannel) != 0)
    {
      channel_statistics[CompositeChannels].entropy+=
        channel_statistics[RedChannel].entropy;
      channels++;
    }
  if ((channel & GreenChannel) != 0)
    {
      channel_statistics[CompositeChannels].entropy+=
        channel_statistics[GreenChannel].entropy;
      channels++;
    }
  if ((channel & BlueChannel) != 0)
    {
      channel_statistics[CompositeChannels].entropy+=
        channel_statistics[BlueChannel].entropy;
      channels++;
    }
  if (((channel & OpacityChannel) != 0) &&
      (image->matte != MagickFalse))
    {
      channel_statistics[CompositeChannels].entropy+=
        channel_statistics[OpacityChannel].entropy;
      channels++;
    }
  if (((channel & IndexChannel) != 0) &&
      (image->colorspace == CMYKColorspace))
    {
      channel_statistics[CompositeChannels].entropy+=
        channel_statistics[BlackChannel].entropy;
      channels++;
    }
  channel_statistics[CompositeChannels].entropy/=channels;
  *entropy=channel_statistics[CompositeChannels].entropy;
  channel_statistics=(ChannelStatistics *) RelinquishMagickMemory(
    channel_statistics);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   G e t I m a g e C h a n n e l E x t r e m a                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetImageChannelExtrema() returns the extrema of one or more image channels.
%
%  The format of the GetImageChannelExtrema method is:
%
%      MagickBooleanType GetImageChannelExtrema(const Image *image,
%        const ChannelType channel,size_t *minima,size_t *maxima,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o channel: the channel.
%
%    o minima: the minimum value in the channel.
%
%    o maxima: the maximum value in the channel.
%
%    o exception: return any errors or warnings in this structure.
%
*/

MagickExport MagickBooleanType GetImageExtrema(const Image *image,
  size_t *minima,size_t *maxima,ExceptionInfo *exception)
{
  MagickBooleanType
    status;

  status=GetImageChannelExtrema(image,CompositeChannels,minima,maxima,
    exception);
  return(status);
}

MagickExport MagickBooleanType GetImageChannelExtrema(const Image *image,
  const ChannelType channel,size_t *minima,size_t *maxima,
  ExceptionInfo *exception)
{
  double
    max,
    min;

  MagickBooleanType
    status;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  status=GetImageChannelRange(image,channel,&min,&max,exception);
  *minima=(size_t) ceil(min-0.5);
  *maxima=(size_t) floor(max+0.5);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t I m a g e C h a n n e l K u r t o s i s                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetImageChannelKurtosis() returns the kurtosis and skewness of one or more
%  image channels.
%
%  The format of the GetImageChannelKurtosis method is:
%
%      MagickBooleanType GetImageChannelKurtosis(const Image *image,
%        const ChannelType channel,double *kurtosis,double *skewness,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o channel: the channel.
%
%    o kurtosis: the kurtosis of the channel.
%
%    o skewness: the skewness of the channel.
%
%    o exception: return any errors or warnings in this structure.
%
*/

MagickExport MagickBooleanType GetImageKurtosis(const Image *image,
  double *kurtosis,double *skewness,ExceptionInfo *exception)
{
  MagickBooleanType
    status;

  status=GetImageChannelKurtosis(image,CompositeChannels,kurtosis,skewness,
    exception);
  return(status);
}

MagickExport MagickBooleanType GetImageChannelKurtosis(const Image *image,
  const ChannelType channel,double *kurtosis,double *skewness,
  ExceptionInfo *exception)
{
  double
    area,
    mean,
    standard_deviation,
    sum_squares,
    sum_cubes,
    sum_fourth_power;

  ssize_t
    y;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  *kurtosis=0.0;
  *skewness=0.0;
  area=0.0;
  mean=0.0;
  standard_deviation=0.0;
  sum_squares=0.0;
  sum_cubes=0.0;
  sum_fourth_power=0.0;
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    register const IndexPacket
      *magick_restrict indexes;

    register const PixelPacket
      *magick_restrict p;

    register ssize_t
      x;

    p=GetVirtualPixels(image,0,y,image->columns,1,exception);
    if (p == (const PixelPacket *) NULL)
      break;
    indexes=GetVirtualIndexQueue(image);
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      if ((channel & RedChannel) != 0)
        {
          mean+=GetPixelRed(p);
          sum_squares+=(double) GetPixelRed(p)*GetPixelRed(p);
          sum_cubes+=(double) GetPixelRed(p)*GetPixelRed(p)*GetPixelRed(p);
          sum_fourth_power+=(double) GetPixelRed(p)*GetPixelRed(p)*
            GetPixelRed(p)*GetPixelRed(p);
          area++;
        }
      if ((channel & GreenChannel) != 0)
        {
          mean+=GetPixelGreen(p);
          sum_squares+=(double) GetPixelGreen(p)*GetPixelGreen(p);
          sum_cubes+=(double) GetPixelGreen(p)*GetPixelGreen(p)*
            GetPixelGreen(p);
          sum_fourth_power+=(double) GetPixelGreen(p)*GetPixelGreen(p)*
            GetPixelGreen(p)*GetPixelGreen(p);
          area++;
        }
      if ((channel & BlueChannel) != 0)
        {
          mean+=GetPixelBlue(p);
          sum_squares+=(double) GetPixelBlue(p)*GetPixelBlue(p);
          sum_cubes+=(double) GetPixelBlue(p)*GetPixelBlue(p)*GetPixelBlue(p);
          sum_fourth_power+=(double) GetPixelBlue(p)*GetPixelBlue(p)*
            GetPixelBlue(p)*GetPixelBlue(p);
          area++;
        }
      if ((channel & OpacityChannel) != 0)
        {
          mean+=GetPixelOpacity(p);
          sum_squares+=(double) GetPixelOpacity(p)*GetPixelOpacity(p);
          sum_cubes+=(double) GetPixelOpacity(p)*GetPixelOpacity(p)*
            GetPixelOpacity(p);
          sum_fourth_power+=(double) GetPixelOpacity(p)*GetPixelOpacity(p)*
            GetPixelOpacity(p)*GetPixelOpacity(p);
          area++;
        }
      if (((channel & IndexChannel) != 0) &&
          (image->colorspace == CMYKColorspace))
        {
          double
            index;

          index=GetPixelIndex(indexes+x);
          mean+=index;
          sum_squares+=index*index;
          sum_cubes+=index*index*index;
          sum_fourth_power+=index*index*index*index;
          area++;
        }
      p++;
    }
  }
  if (y < (ssize_t) image->rows)
    return(MagickFalse);
  if (area != 0.0)
    {
      mean/=area;
      sum_squares/=area;
      sum_cubes/=area;
      sum_fourth_power/=area;
    }
  standard_deviation=sqrt(sum_squares-(mean*mean));
  if (standard_deviation != 0.0)
    {
      *kurtosis=sum_fourth_power-4.0*mean*sum_cubes+6.0*mean*mean*sum_squares-
        3.0*mean*mean*mean*mean;
      *kurtosis/=standard_deviation*standard_deviation*standard_deviation*
        standard_deviation;
      *kurtosis-=3.0;
      *skewness=sum_cubes-3.0*mean*sum_squares+2.0*mean*mean*mean;
      *skewness/=standard_deviation*standard_deviation*standard_deviation;
    }
  return(y == (ssize_t) image->rows ? MagickTrue : MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t I m a g e C h a n n e l M e a n                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetImageChannelMean() returns the mean and standard deviation of one or more
%  image channels.
%
%  The format of the GetImageChannelMean method is:
%
%      MagickBooleanType GetImageChannelMean(const Image *image,
%        const ChannelType channel,double *mean,double *standard_deviation,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o channel: the channel.
%
%    o mean: the average value in the channel.
%
%    o standard_deviation: the standard deviation of the channel.
%
%    o exception: return any errors or warnings in this structure.
%
*/

MagickExport MagickBooleanType GetImageMean(const Image *image,double *mean,
  double *standard_deviation,ExceptionInfo *exception)
{
  MagickBooleanType
    status;

  status=GetImageChannelMean(image,CompositeChannels,mean,standard_deviation,
    exception);
  return(status);
}

MagickExport MagickBooleanType GetImageChannelMean(const Image *image,
  const ChannelType channel,double *mean,double *standard_deviation,
  ExceptionInfo *exception)
{
  ChannelStatistics
    *channel_statistics;

  size_t
    channels;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  channel_statistics=GetImageChannelStatistics(image,exception);
  if (channel_statistics == (ChannelStatistics *) NULL)
    return(MagickFalse);
  channels=0;
  channel_statistics[CompositeChannels].mean=0.0;
  channel_statistics[CompositeChannels].standard_deviation=0.0;
  if ((channel & RedChannel) != 0)
    {
      channel_statistics[CompositeChannels].mean+=
        channel_statistics[RedChannel].mean;
      channel_statistics[CompositeChannels].standard_deviation+=
        channel_statistics[RedChannel].variance-
        channel_statistics[RedChannel].mean*
        channel_statistics[RedChannel].mean;
      channels++;
    }
  if ((channel & GreenChannel) != 0)
    {
      channel_statistics[CompositeChannels].mean+=
        channel_statistics[GreenChannel].mean;
      channel_statistics[CompositeChannels].standard_deviation+=
        channel_statistics[GreenChannel].variance-
        channel_statistics[GreenChannel].mean*
        channel_statistics[GreenChannel].mean;
      channels++;
    }
  if ((channel & BlueChannel) != 0)
    {
      channel_statistics[CompositeChannels].mean+=
        channel_statistics[BlueChannel].mean;
      channel_statistics[CompositeChannels].standard_deviation+=
        channel_statistics[BlueChannel].variance-
        channel_statistics[BlueChannel].mean*
        channel_statistics[BlueChannel].mean;
      channels++;
    }
  if (((channel & OpacityChannel) != 0) &&
      (image->matte != MagickFalse))
    {
      channel_statistics[CompositeChannels].mean+=
        channel_statistics[OpacityChannel].mean;
      channel_statistics[CompositeChannels].standard_deviation+=
        channel_statistics[OpacityChannel].variance-
        channel_statistics[OpacityChannel].mean*
        channel_statistics[OpacityChannel].mean;
      channels++;
    }
  if (((channel & IndexChannel) != 0) &&
      (image->colorspace == CMYKColorspace))
    {
      channel_statistics[CompositeChannels].mean+=
        channel_statistics[BlackChannel].mean;
      channel_statistics[CompositeChannels].standard_deviation+=
        channel_statistics[BlackChannel].variance-
        channel_statistics[BlackChannel].mean*
        channel_statistics[BlackChannel].mean;
      channels++;
    }
  channel_statistics[CompositeChannels].mean/=channels;
  channel_statistics[CompositeChannels].standard_deviation=
    sqrt(channel_statistics[CompositeChannels].standard_deviation/channels);
  *mean=channel_statistics[CompositeChannels].mean;
  *standard_deviation=channel_statistics[CompositeChannels].standard_deviation;
  channel_statistics=(ChannelStatistics *) RelinquishMagickMemory(
    channel_statistics);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t I m a g e C h a n n e l M o m e n t s                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetImageChannelMoments() returns the normalized moments of one or more image
%  channels.
%
%  The format of the GetImageChannelMoments method is:
%
%      ChannelMoments *GetImageChannelMoments(const Image *image,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport ChannelMoments *GetImageChannelMoments(const Image *image,
  ExceptionInfo *exception)
{
#define MaxNumberImageMoments  8

  ChannelMoments
    *channel_moments;

  double
    M00[CompositeChannels+1],
    M01[CompositeChannels+1],
    M02[CompositeChannels+1],
    M03[CompositeChannels+1],
    M10[CompositeChannels+1],
    M11[CompositeChannels+1],
    M12[CompositeChannels+1],
    M20[CompositeChannels+1],
    M21[CompositeChannels+1],
    M22[CompositeChannels+1],
    M30[CompositeChannels+1];

  MagickPixelPacket
    pixel;

  PointInfo
    centroid[CompositeChannels+1];

  ssize_t
    channel,
    channels,
    y;

  size_t
    length;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  length=CompositeChannels+1UL;
  channel_moments=(ChannelMoments *) AcquireQuantumMemory(length,
    sizeof(*channel_moments));
  if (channel_moments == (ChannelMoments *) NULL)
    return(channel_moments);
  (void) ResetMagickMemory(channel_moments,0,length*sizeof(*channel_moments));
  (void) ResetMagickMemory(centroid,0,sizeof(centroid));
  (void) ResetMagickMemory(M00,0,sizeof(M00));
  (void) ResetMagickMemory(M01,0,sizeof(M01));
  (void) ResetMagickMemory(M02,0,sizeof(M02));
  (void) ResetMagickMemory(M03,0,sizeof(M03));
  (void) ResetMagickMemory(M10,0,sizeof(M10));
  (void) ResetMagickMemory(M11,0,sizeof(M11));
  (void) ResetMagickMemory(M12,0,sizeof(M12));
  (void) ResetMagickMemory(M20,0,sizeof(M20));
  (void) ResetMagickMemory(M21,0,sizeof(M21));
  (void) ResetMagickMemory(M22,0,sizeof(M22));
  (void) ResetMagickMemory(M30,0,sizeof(M30));
  GetMagickPixelPacket(image,&pixel);
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    register const IndexPacket
      *magick_restrict indexes;

    register const PixelPacket
      *magick_restrict p;

    register ssize_t
      x;

    /*
      Compute center of mass (centroid).
    */
    p=GetVirtualPixels(image,0,y,image->columns,1,exception);
    if (p == (const PixelPacket *) NULL)
      break;
    indexes=GetVirtualIndexQueue(image);
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      SetMagickPixelPacket(image,p,indexes+x,&pixel);
      M00[RedChannel]+=QuantumScale*pixel.red;
      M10[RedChannel]+=x*QuantumScale*pixel.red;
      M01[RedChannel]+=y*QuantumScale*pixel.red;
      M00[GreenChannel]+=QuantumScale*pixel.green;
      M10[GreenChannel]+=x*QuantumScale*pixel.green;
      M01[GreenChannel]+=y*QuantumScale*pixel.green;
      M00[BlueChannel]+=QuantumScale*pixel.blue;
      M10[BlueChannel]+=x*QuantumScale*pixel.blue;
      M01[BlueChannel]+=y*QuantumScale*pixel.blue;
      if (image->matte != MagickFalse)
        {
          M00[OpacityChannel]+=QuantumScale*pixel.opacity;
          M10[OpacityChannel]+=x*QuantumScale*pixel.opacity;
          M01[OpacityChannel]+=y*QuantumScale*pixel.opacity;
        }
      if (image->colorspace == CMYKColorspace)
        {
          M00[IndexChannel]+=QuantumScale*pixel.index;
          M10[IndexChannel]+=x*QuantumScale*pixel.index;
          M01[IndexChannel]+=y*QuantumScale*pixel.index;
        }
      p++;
    }
  }
  for (channel=0; channel <= CompositeChannels; channel++)
  {
    /*
      Compute center of mass (centroid).
    */
    if (M00[channel] < MagickEpsilon)
      {
        M00[channel]+=MagickEpsilon;
        centroid[channel].x=(double) image->columns/2.0;
        centroid[channel].y=(double) image->rows/2.0;
        continue;
      }
    M00[channel]+=MagickEpsilon;
    centroid[channel].x=M10[channel]/M00[channel];
    centroid[channel].y=M01[channel]/M00[channel];
  }
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    register const IndexPacket
      *magick_restrict indexes;

    register const PixelPacket
      *magick_restrict p;

    register ssize_t
      x;

    /*
      Compute the image moments.
    */
    p=GetVirtualPixels(image,0,y,image->columns,1,exception);
    if (p == (const PixelPacket *) NULL)
      break;
    indexes=GetVirtualIndexQueue(image);
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      SetMagickPixelPacket(image,p,indexes+x,&pixel);
      M11[RedChannel]+=(x-centroid[RedChannel].x)*(y-
        centroid[RedChannel].y)*QuantumScale*pixel.red;
      M20[RedChannel]+=(x-centroid[RedChannel].x)*(x-
        centroid[RedChannel].x)*QuantumScale*pixel.red;
      M02[RedChannel]+=(y-centroid[RedChannel].y)*(y-
        centroid[RedChannel].y)*QuantumScale*pixel.red;
      M21[RedChannel]+=(x-centroid[RedChannel].x)*(x-
        centroid[RedChannel].x)*(y-centroid[RedChannel].y)*QuantumScale*
        pixel.red;
      M12[RedChannel]+=(x-centroid[RedChannel].x)*(y-
        centroid[RedChannel].y)*(y-centroid[RedChannel].y)*QuantumScale*
        pixel.red;
      M22[RedChannel]+=(x-centroid[RedChannel].x)*(x-
        centroid[RedChannel].x)*(y-centroid[RedChannel].y)*(y-
        centroid[RedChannel].y)*QuantumScale*pixel.red;
      M30[RedChannel]+=(x-centroid[RedChannel].x)*(x-
        centroid[RedChannel].x)*(x-centroid[RedChannel].x)*QuantumScale*
        pixel.red;
      M03[RedChannel]+=(y-centroid[RedChannel].y)*(y-
        centroid[RedChannel].y)*(y-centroid[RedChannel].y)*QuantumScale*
        pixel.red;
      M11[GreenChannel]+=(x-centroid[GreenChannel].x)*(y-
        centroid[GreenChannel].y)*QuantumScale*pixel.green;
      M20[GreenChannel]+=(x-centroid[GreenChannel].x)*(x-
        centroid[GreenChannel].x)*QuantumScale*pixel.green;
      M02[GreenChannel]+=(y-centroid[GreenChannel].y)*(y-
        centroid[GreenChannel].y)*QuantumScale*pixel.green;
      M21[GreenChannel]+=(x-centroid[GreenChannel].x)*(x-
        centroid[GreenChannel].x)*(y-centroid[GreenChannel].y)*QuantumScale*
        pixel.green;
      M12[GreenChannel]+=(x-centroid[GreenChannel].x)*(y-
        centroid[GreenChannel].y)*(y-centroid[GreenChannel].y)*QuantumScale*
        pixel.green;
      M22[GreenChannel]+=(x-centroid[GreenChannel].x)*(x-
        centroid[GreenChannel].x)*(y-centroid[GreenChannel].y)*(y-
        centroid[GreenChannel].y)*QuantumScale*pixel.green;
      M30[GreenChannel]+=(x-centroid[GreenChannel].x)*(x-
        centroid[GreenChannel].x)*(x-centroid[GreenChannel].x)*QuantumScale*
        pixel.green;
      M03[GreenChannel]+=(y-centroid[GreenChannel].y)*(y-
        centroid[GreenChannel].y)*(y-centroid[GreenChannel].y)*QuantumScale*
        pixel.green;
      M11[BlueChannel]+=(x-centroid[BlueChannel].x)*(y-
        centroid[BlueChannel].y)*QuantumScale*pixel.blue;
      M20[BlueChannel]+=(x-centroid[BlueChannel].x)*(x-
        centroid[BlueChannel].x)*QuantumScale*pixel.blue;
      M02[BlueChannel]+=(y-centroid[BlueChannel].y)*(y-
        centroid[BlueChannel].y)*QuantumScale*pixel.blue;
      M21[BlueChannel]+=(x-centroid[BlueChannel].x)*(x-
        centroid[BlueChannel].x)*(y-centroid[BlueChannel].y)*QuantumScale*
        pixel.blue;
      M12[BlueChannel]+=(x-centroid[BlueChannel].x)*(y-
        centroid[BlueChannel].y)*(y-centroid[BlueChannel].y)*QuantumScale*
        pixel.blue;
      M22[BlueChannel]+=(x-centroid[BlueChannel].x)*(x-
        centroid[BlueChannel].x)*(y-centroid[BlueChannel].y)*(y-
        centroid[BlueChannel].y)*QuantumScale*pixel.blue;
      M30[BlueChannel]+=(x-centroid[BlueChannel].x)*(x-
        centroid[BlueChannel].x)*(x-centroid[BlueChannel].x)*QuantumScale*
        pixel.blue;
      M03[BlueChannel]+=(y-centroid[BlueChannel].y)*(y-
        centroid[BlueChannel].y)*(y-centroid[BlueChannel].y)*QuantumScale*
        pixel.blue;
      if (image->matte != MagickFalse)
        {
          M11[OpacityChannel]+=(x-centroid[OpacityChannel].x)*(y-
            centroid[OpacityChannel].y)*QuantumScale*pixel.opacity;
          M20[OpacityChannel]+=(x-centroid[OpacityChannel].x)*(x-
            centroid[OpacityChannel].x)*QuantumScale*pixel.opacity;
          M02[OpacityChannel]+=(y-centroid[OpacityChannel].y)*(y-
            centroid[OpacityChannel].y)*QuantumScale*pixel.opacity;
          M21[OpacityChannel]+=(x-centroid[OpacityChannel].x)*(x-
            centroid[OpacityChannel].x)*(y-centroid[OpacityChannel].y)*
            QuantumScale*pixel.opacity;
          M12[OpacityChannel]+=(x-centroid[OpacityChannel].x)*(y-
            centroid[OpacityChannel].y)*(y-centroid[OpacityChannel].y)*
            QuantumScale*pixel.opacity;
          M22[OpacityChannel]+=(x-centroid[OpacityChannel].x)*(x-
            centroid[OpacityChannel].x)*(y-centroid[OpacityChannel].y)*(y-
            centroid[OpacityChannel].y)*QuantumScale*pixel.opacity;
          M30[OpacityChannel]+=(x-centroid[OpacityChannel].x)*(x-
            centroid[OpacityChannel].x)*(x-centroid[OpacityChannel].x)*
            QuantumScale*pixel.opacity;
          M03[OpacityChannel]+=(y-centroid[OpacityChannel].y)*(y-
            centroid[OpacityChannel].y)*(y-centroid[OpacityChannel].y)*
            QuantumScale*pixel.opacity;
        }
      if (image->colorspace == CMYKColorspace)
        {
          M11[IndexChannel]+=(x-centroid[IndexChannel].x)*(y-
            centroid[IndexChannel].y)*QuantumScale*pixel.index;
          M20[IndexChannel]+=(x-centroid[IndexChannel].x)*(x-
            centroid[IndexChannel].x)*QuantumScale*pixel.index;
          M02[IndexChannel]+=(y-centroid[IndexChannel].y)*(y-
            centroid[IndexChannel].y)*QuantumScale*pixel.index;
          M21[IndexChannel]+=(x-centroid[IndexChannel].x)*(x-
            centroid[IndexChannel].x)*(y-centroid[IndexChannel].y)*
            QuantumScale*pixel.index;
          M12[IndexChannel]+=(x-centroid[IndexChannel].x)*(y-
            centroid[IndexChannel].y)*(y-centroid[IndexChannel].y)*
            QuantumScale*pixel.index;
          M22[IndexChannel]+=(x-centroid[IndexChannel].x)*(x-
            centroid[IndexChannel].x)*(y-centroid[IndexChannel].y)*(y-
            centroid[IndexChannel].y)*QuantumScale*pixel.index;
          M30[IndexChannel]+=(x-centroid[IndexChannel].x)*(x-
            centroid[IndexChannel].x)*(x-centroid[IndexChannel].x)*
            QuantumScale*pixel.index;
          M03[IndexChannel]+=(y-centroid[IndexChannel].y)*(y-
            centroid[IndexChannel].y)*(y-centroid[IndexChannel].y)*
            QuantumScale*pixel.index;
        }
      p++;
    }
  }
  channels=3;
  M00[CompositeChannels]+=(M00[RedChannel]+M00[GreenChannel]+M00[BlueChannel]);
  M01[CompositeChannels]+=(M01[RedChannel]+M01[GreenChannel]+M01[BlueChannel]);
  M02[CompositeChannels]+=(M02[RedChannel]+M02[GreenChannel]+M02[BlueChannel]);
  M03[CompositeChannels]+=(M03[RedChannel]+M03[GreenChannel]+M03[BlueChannel]);
  M10[CompositeChannels]+=(M10[RedChannel]+M10[GreenChannel]+M10[BlueChannel]);
  M11[CompositeChannels]+=(M11[RedChannel]+M11[GreenChannel]+M11[BlueChannel]);
  M12[CompositeChannels]+=(M12[RedChannel]+M12[GreenChannel]+M12[BlueChannel]);
  M20[CompositeChannels]+=(M20[RedChannel]+M20[GreenChannel]+M20[BlueChannel]);
  M21[CompositeChannels]+=(M21[RedChannel]+M21[GreenChannel]+M21[BlueChannel]);
  M22[CompositeChannels]+=(M22[RedChannel]+M22[GreenChannel]+M22[BlueChannel]);
  M30[CompositeChannels]+=(M30[RedChannel]+M30[GreenChannel]+M30[BlueChannel]);
  if (image->matte != MagickFalse)
    {
      channels+=1;
      M00[CompositeChannels]+=M00[OpacityChannel];
      M01[CompositeChannels]+=M01[OpacityChannel];
      M02[CompositeChannels]+=M02[OpacityChannel];
      M03[CompositeChannels]+=M03[OpacityChannel];
      M10[CompositeChannels]+=M10[OpacityChannel];
      M11[CompositeChannels]+=M11[OpacityChannel];
      M12[CompositeChannels]+=M12[OpacityChannel];
      M20[CompositeChannels]+=M20[OpacityChannel];
      M21[CompositeChannels]+=M21[OpacityChannel];
      M22[CompositeChannels]+=M22[OpacityChannel];
      M30[CompositeChannels]+=M30[OpacityChannel];
    }
  if (image->colorspace == CMYKColorspace)
    {
      channels+=1;
      M00[CompositeChannels]+=M00[IndexChannel];
      M01[CompositeChannels]+=M01[IndexChannel];
      M02[CompositeChannels]+=M02[IndexChannel];
      M03[CompositeChannels]+=M03[IndexChannel];
      M10[CompositeChannels]+=M10[IndexChannel];
      M11[CompositeChannels]+=M11[IndexChannel];
      M12[CompositeChannels]+=M12[IndexChannel];
      M20[CompositeChannels]+=M20[IndexChannel];
      M21[CompositeChannels]+=M21[IndexChannel];
      M22[CompositeChannels]+=M22[IndexChannel];
      M30[CompositeChannels]+=M30[IndexChannel];
    }
  M00[CompositeChannels]/=(double) channels;
  M01[CompositeChannels]/=(double) channels;
  M02[CompositeChannels]/=(double) channels;
  M03[CompositeChannels]/=(double) channels;
  M10[CompositeChannels]/=(double) channels;
  M11[CompositeChannels]/=(double) channels;
  M12[CompositeChannels]/=(double) channels;
  M20[CompositeChannels]/=(double) channels;
  M21[CompositeChannels]/=(double) channels;
  M22[CompositeChannels]/=(double) channels;
  M30[CompositeChannels]/=(double) channels;
  for (channel=0; channel <= CompositeChannels; channel++)
  {
    /*
      Compute elliptical angle, major and minor axes, eccentricity, & intensity.
    */
    channel_moments[channel].centroid=centroid[channel];
    channel_moments[channel].ellipse_axis.x=sqrt((2.0/M00[channel])*
      ((M20[channel]+M02[channel])+sqrt(4.0*M11[channel]*M11[channel]+
      (M20[channel]-M02[channel])*(M20[channel]-M02[channel]))));
    channel_moments[channel].ellipse_axis.y=sqrt((2.0/M00[channel])*
      ((M20[channel]+M02[channel])-sqrt(4.0*M11[channel]*M11[channel]+
      (M20[channel]-M02[channel])*(M20[channel]-M02[channel]))));
    channel_moments[channel].ellipse_angle=RadiansToDegrees(0.5*atan(2.0*
      M11[channel]/(M20[channel]-M02[channel]+MagickEpsilon)));
    channel_moments[channel].ellipse_eccentricity=sqrt(1.0-(
      channel_moments[channel].ellipse_axis.y/
      (channel_moments[channel].ellipse_axis.x+MagickEpsilon)));
    channel_moments[channel].ellipse_intensity=M00[channel]/
      (MagickPI*channel_moments[channel].ellipse_axis.x*
      channel_moments[channel].ellipse_axis.y+MagickEpsilon);
  }
  for (channel=0; channel <= CompositeChannels; channel++)
  {
    /*
      Normalize image moments.
    */
    M10[channel]=0.0;
    M01[channel]=0.0;
    M11[channel]/=pow(M00[channel],1.0+(1.0+1.0)/2.0);
    M20[channel]/=pow(M00[channel],1.0+(2.0+0.0)/2.0);
    M02[channel]/=pow(M00[channel],1.0+(0.0+2.0)/2.0);
    M21[channel]/=pow(M00[channel],1.0+(2.0+1.0)/2.0);
    M12[channel]/=pow(M00[channel],1.0+(1.0+2.0)/2.0);
    M22[channel]/=pow(M00[channel],1.0+(2.0+2.0)/2.0);
    M30[channel]/=pow(M00[channel],1.0+(3.0+0.0)/2.0);
    M03[channel]/=pow(M00[channel],1.0+(0.0+3.0)/2.0);
    M00[channel]=1.0;
  }
  for (channel=0; channel <= CompositeChannels; channel++)
  {
    /*
      Compute Hu invariant moments.
    */
    channel_moments[channel].I[0]=M20[channel]+M02[channel];
    channel_moments[channel].I[1]=(M20[channel]-M02[channel])*
      (M20[channel]-M02[channel])+4.0*M11[channel]*M11[channel];
    channel_moments[channel].I[2]=(M30[channel]-3.0*M12[channel])*
      (M30[channel]-3.0*M12[channel])+(3.0*M21[channel]-M03[channel])*
      (3.0*M21[channel]-M03[channel]);
    channel_moments[channel].I[3]=(M30[channel]+M12[channel])*
      (M30[channel]+M12[channel])+(M21[channel]+M03[channel])*
      (M21[channel]+M03[channel]);
    channel_moments[channel].I[4]=(M30[channel]-3.0*M12[channel])*
      (M30[channel]+M12[channel])*((M30[channel]+M12[channel])*
      (M30[channel]+M12[channel])-3.0*(M21[channel]+M03[channel])*
      (M21[channel]+M03[channel]))+(3.0*M21[channel]-M03[channel])*
      (M21[channel]+M03[channel])*(3.0*(M30[channel]+M12[channel])*
      (M30[channel]+M12[channel])-(M21[channel]+M03[channel])*
      (M21[channel]+M03[channel]));
    channel_moments[channel].I[5]=(M20[channel]-M02[channel])*
      ((M30[channel]+M12[channel])*(M30[channel]+M12[channel])-
      (M21[channel]+M03[channel])*(M21[channel]+M03[channel]))+
      4.0*M11[channel]*(M30[channel]+M12[channel])*(M21[channel]+M03[channel]);
    channel_moments[channel].I[6]=(3.0*M21[channel]-M03[channel])*
      (M30[channel]+M12[channel])*((M30[channel]+M12[channel])*
      (M30[channel]+M12[channel])-3.0*(M21[channel]+M03[channel])*
      (M21[channel]+M03[channel]))-(M30[channel]-3*M12[channel])*
      (M21[channel]+M03[channel])*(3.0*(M30[channel]+M12[channel])*
      (M30[channel]+M12[channel])-(M21[channel]+M03[channel])*
      (M21[channel]+M03[channel]));
    channel_moments[channel].I[7]=M11[channel]*((M30[channel]+M12[channel])*
      (M30[channel]+M12[channel])-(M03[channel]+M21[channel])*
      (M03[channel]+M21[channel]))-(M20[channel]-M02[channel])*
      (M30[channel]+M12[channel])*(M03[channel]+M21[channel]);
  }
  if (y < (ssize_t) image->rows)
    channel_moments=(ChannelMoments *) RelinquishMagickMemory(channel_moments);
  return(channel_moments);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t I m a g e C h a n n e l P e r c e p t u a l H a s h                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetImageChannelPerceptualHash() returns the perceptual hash of one or more
%  image channels.
%
%  The format of the GetImageChannelPerceptualHash method is:
%
%      ChannelPerceptualHash *GetImageChannelPerceptualHash(const Image *image,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static inline double MagickLog10(const double x)
{
#define Log10Epsilon  (1.0e-11)

 if (fabs(x) < Log10Epsilon)
   return(log10(Log10Epsilon));
 return(log10(fabs(x)));
}

MagickExport ChannelPerceptualHash *GetImageChannelPerceptualHash(
  const Image *image,ExceptionInfo *exception)
{
  ChannelMoments
    *moments;

  ChannelPerceptualHash
    *perceptual_hash;

  Image
    *hash_image;

  MagickBooleanType
    status;

  register ssize_t
    i;

  ssize_t
    channel;

  /*
    Blur then transform to sRGB colorspace.
  */
  hash_image=BlurImage(image,0.0,1.0,exception);
  if (hash_image == (Image *) NULL)
    return((ChannelPerceptualHash *) NULL);
  hash_image->depth=8;
  status=TransformImageColorspace(hash_image,sRGBColorspace);
  if (status == MagickFalse)
    return((ChannelPerceptualHash *) NULL);
  moments=GetImageChannelMoments(hash_image,exception);
  hash_image=DestroyImage(hash_image);
  if (moments == (ChannelMoments *) NULL)
    return((ChannelPerceptualHash *) NULL);
  perceptual_hash=(ChannelPerceptualHash *) AcquireQuantumMemory(
    CompositeChannels+1UL,sizeof(*perceptual_hash));
  if (perceptual_hash == (ChannelPerceptualHash *) NULL)
    return((ChannelPerceptualHash *) NULL);
  for (channel=0; channel <= CompositeChannels; channel++)
    for (i=0; i < MaximumNumberOfImageMoments; i++)
      perceptual_hash[channel].P[i]=(-MagickLog10(moments[channel].I[i]));
  moments=(ChannelMoments *) RelinquishMagickMemory(moments);
  /*
    Blur then transform to HCLp colorspace.
  */
  hash_image=BlurImage(image,0.0,1.0,exception);
  if (hash_image == (Image *) NULL)
    {
      perceptual_hash=(ChannelPerceptualHash *) RelinquishMagickMemory(
        perceptual_hash);
      return((ChannelPerceptualHash *) NULL);
    }
  hash_image->depth=8;
  status=TransformImageColorspace(hash_image,HCLpColorspace);
  if (status == MagickFalse)
    {
      perceptual_hash=(ChannelPerceptualHash *) RelinquishMagickMemory(
        perceptual_hash);
      return((ChannelPerceptualHash *) NULL);
    }
  moments=GetImageChannelMoments(hash_image,exception);
  hash_image=DestroyImage(hash_image);
  if (moments == (ChannelMoments *) NULL)
    {
      perceptual_hash=(ChannelPerceptualHash *) RelinquishMagickMemory(
        perceptual_hash);
      return((ChannelPerceptualHash *) NULL);
    }
  for (channel=0; channel <= CompositeChannels; channel++)
    for (i=0; i < MaximumNumberOfImageMoments; i++)
      perceptual_hash[channel].Q[i]=(-MagickLog10(moments[channel].I[i]));
  moments=(ChannelMoments *) RelinquishMagickMemory(moments);
  return(perceptual_hash);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t I m a g e C h a n n e l R a n g e                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetImageChannelRange() returns the range of one or more image channels.
%
%  The format of the GetImageChannelRange method is:
%
%      MagickBooleanType GetImageChannelRange(const Image *image,
%        const ChannelType channel,double *minima,double *maxima,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o channel: the channel.
%
%    o minima: the minimum value in the channel.
%
%    o maxima: the maximum value in the channel.
%
%    o exception: return any errors or warnings in this structure.
%
*/

MagickExport MagickBooleanType GetImageRange(const Image *image,
  double *minima,double *maxima,ExceptionInfo *exception)
{
  return(GetImageChannelRange(image,CompositeChannels,minima,maxima,exception));
}

MagickExport MagickBooleanType GetImageChannelRange(const Image *image,
  const ChannelType channel,double *minima,double *maxima,
  ExceptionInfo *exception)
{
  MagickPixelPacket
    pixel;

  ssize_t
    y;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  *maxima=(-MagickMaximumValue);
  *minima=MagickMaximumValue;
  GetMagickPixelPacket(image,&pixel);
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    register const IndexPacket
      *magick_restrict indexes;

    register const PixelPacket
      *magick_restrict p;

    register ssize_t
      x;

    p=GetVirtualPixels(image,0,y,image->columns,1,exception);
    if (p == (const PixelPacket *) NULL)
      break;
    indexes=GetVirtualIndexQueue(image);
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      SetMagickPixelPacket(image,p,indexes+x,&pixel);
      if ((channel & RedChannel) != 0)
        {
          if (pixel.red < *minima)
            *minima=(double) pixel.red;
          if (pixel.red > *maxima)
            *maxima=(double) pixel.red;
        }
      if ((channel & GreenChannel) != 0)
        {
          if (pixel.green < *minima)
            *minima=(double) pixel.green;
          if (pixel.green > *maxima)
            *maxima=(double) pixel.green;
        }
      if ((channel & BlueChannel) != 0)
        {
          if (pixel.blue < *minima)
            *minima=(double) pixel.blue;
          if (pixel.blue > *maxima)
            *maxima=(double) pixel.blue;
        }
      if (((channel & OpacityChannel) != 0) &&
          (image->matte != MagickFalse))
        {
          if ((QuantumRange-pixel.opacity) < *minima)
            *minima=(double) (QuantumRange-pixel.opacity);
          if ((QuantumRange-pixel.opacity) > *maxima)
            *maxima=(double) (QuantumRange-pixel.opacity);
        }
      if (((channel & IndexChannel) != 0) &&
          (image->colorspace == CMYKColorspace))
        {
          if ((double) pixel.index < *minima)
            *minima=(double) pixel.index;
          if ((double) pixel.index > *maxima)
            *maxima=(double) pixel.index;
        }
      p++;
    }
  }
  return(y == (ssize_t) image->rows ? MagickTrue : MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t I m a g e C h a n n e l S t a t i s t i c s                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetImageChannelStatistics() returns statistics for each channel in the
%  image.  The statistics include the channel depth, its minima, maxima, mean,
%  standard deviation, kurtosis and skewness.  You can access the red channel
%  mean, for example, like this:
%
%      channel_statistics=GetImageChannelStatistics(image,exception);
%      red_mean=channel_statistics[RedChannel].mean;
%
%  Use MagickRelinquishMemory() to free the statistics buffer.
%
%  The format of the GetImageChannelStatistics method is:
%
%      ChannelStatistics *GetImageChannelStatistics(const Image *image,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport ChannelStatistics *GetImageChannelStatistics(const Image *image,
  ExceptionInfo *exception)
{
  ChannelStatistics
    *channel_statistics;

  double
    area;

  MagickPixelPacket
    number_bins,
    *histogram;

  QuantumAny
    range;

  register ssize_t
    i;

  size_t
    channels,
    depth,
    length;

  ssize_t
    y;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  length=CompositeChannels+1UL;
  channel_statistics=(ChannelStatistics *) AcquireQuantumMemory(length,
    sizeof(*channel_statistics));
  histogram=(MagickPixelPacket *) AcquireQuantumMemory(MaxMap+1U,
    sizeof(*histogram));
  if ((channel_statistics == (ChannelStatistics *) NULL) ||
      (histogram == (MagickPixelPacket *) NULL))
    {
      if (histogram != (MagickPixelPacket *) NULL)
        histogram=(MagickPixelPacket *) RelinquishMagickMemory(histogram);
      if (channel_statistics != (ChannelStatistics *) NULL)
        channel_statistics=(ChannelStatistics *) RelinquishMagickMemory(
          channel_statistics);
      return(channel_statistics);
    }
  (void) ResetMagickMemory(channel_statistics,0,length*
    sizeof(*channel_statistics));
  for (i=0; i <= (ssize_t) CompositeChannels; i++)
  {
    channel_statistics[i].depth=1;
    channel_statistics[i].maxima=(-MagickMaximumValue);
    channel_statistics[i].minima=MagickMaximumValue;
  }
  (void) ResetMagickMemory(histogram,0,(MaxMap+1U)*sizeof(*histogram));
  (void) ResetMagickMemory(&number_bins,0,sizeof(number_bins));
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    register const IndexPacket
      *magick_restrict indexes;

    register const PixelPacket
      *magick_restrict p;

    register ssize_t
      x;

    p=GetVirtualPixels(image,0,y,image->columns,1,exception);
    if (p == (const PixelPacket *) NULL)
      break;
    indexes=GetVirtualIndexQueue(image);
    for (x=0; x < (ssize_t) image->columns; )
    {
      if (channel_statistics[RedChannel].depth != MAGICKCORE_QUANTUM_DEPTH)
        {
          depth=channel_statistics[RedChannel].depth;
          range=GetQuantumRange(depth);
          if (IsPixelAtDepth(GetPixelRed(p),range) == MagickFalse)
            {
              channel_statistics[RedChannel].depth++;
              continue;
            }
        }
      if (channel_statistics[GreenChannel].depth != MAGICKCORE_QUANTUM_DEPTH)
        {
          depth=channel_statistics[GreenChannel].depth;
          range=GetQuantumRange(depth);
          if (IsPixelAtDepth(GetPixelGreen(p),range) == MagickFalse)
            {
              channel_statistics[GreenChannel].depth++;
              continue;
            }
        }
      if (channel_statistics[BlueChannel].depth != MAGICKCORE_QUANTUM_DEPTH)
        {
          depth=channel_statistics[BlueChannel].depth;
          range=GetQuantumRange(depth);
          if (IsPixelAtDepth(GetPixelBlue(p),range) == MagickFalse)
            {
              channel_statistics[BlueChannel].depth++;
              continue;
            }
        }
      if (image->matte != MagickFalse)
        {
          if (channel_statistics[OpacityChannel].depth != MAGICKCORE_QUANTUM_DEPTH)
            {
              depth=channel_statistics[OpacityChannel].depth;
              range=GetQuantumRange(depth);
              if (IsPixelAtDepth(GetPixelOpacity(p),range) == MagickFalse)
                {
                  channel_statistics[OpacityChannel].depth++;
                  continue;
                }
            }
          }
      if (image->colorspace == CMYKColorspace)
        {
          if (channel_statistics[BlackChannel].depth != MAGICKCORE_QUANTUM_DEPTH)
            {
              depth=channel_statistics[BlackChannel].depth;
              range=GetQuantumRange(depth);
              if (IsPixelAtDepth(GetPixelIndex(indexes+x),range) == MagickFalse)
                {
                  channel_statistics[BlackChannel].depth++;
                  continue;
                }
            }
        }
      if ((double) GetPixelRed(p) < channel_statistics[RedChannel].minima)
        channel_statistics[RedChannel].minima=(double) GetPixelRed(p);
      if ((double) GetPixelRed(p) > channel_statistics[RedChannel].maxima)
        channel_statistics[RedChannel].maxima=(double) GetPixelRed(p);
      channel_statistics[RedChannel].sum+=GetPixelRed(p);
      channel_statistics[RedChannel].sum_squared+=(double) GetPixelRed(p)*
        GetPixelRed(p);
      channel_statistics[RedChannel].sum_cubed+=(double)
        GetPixelRed(p)*GetPixelRed(p)*GetPixelRed(p);
      channel_statistics[RedChannel].sum_fourth_power+=(double)
        GetPixelRed(p)*GetPixelRed(p)*GetPixelRed(p)*GetPixelRed(p);
      if ((double) GetPixelGreen(p) < channel_statistics[GreenChannel].minima)
        channel_statistics[GreenChannel].minima=(double) GetPixelGreen(p);
      if ((double) GetPixelGreen(p) > channel_statistics[GreenChannel].maxima)
        channel_statistics[GreenChannel].maxima=(double) GetPixelGreen(p);
      channel_statistics[GreenChannel].sum+=GetPixelGreen(p);
      channel_statistics[GreenChannel].sum_squared+=(double) GetPixelGreen(p)*
        GetPixelGreen(p);
      channel_statistics[GreenChannel].sum_cubed+=(double) GetPixelGreen(p)*
        GetPixelGreen(p)*GetPixelGreen(p);
      channel_statistics[GreenChannel].sum_fourth_power+=(double)
        GetPixelGreen(p)*GetPixelGreen(p)*GetPixelGreen(p)*GetPixelGreen(p);
      if ((double) GetPixelBlue(p) < channel_statistics[BlueChannel].minima)
        channel_statistics[BlueChannel].minima=(double) GetPixelBlue(p);
      if ((double) GetPixelBlue(p) > channel_statistics[BlueChannel].maxima)
        channel_statistics[BlueChannel].maxima=(double) GetPixelBlue(p);
      channel_statistics[BlueChannel].sum+=GetPixelBlue(p);
      channel_statistics[BlueChannel].sum_squared+=(double) GetPixelBlue(p)*
        GetPixelBlue(p);
      channel_statistics[BlueChannel].sum_cubed+=(double) GetPixelBlue(p)*
        GetPixelBlue(p)*GetPixelBlue(p);
      channel_statistics[BlueChannel].sum_fourth_power+=(double)
        GetPixelBlue(p)*GetPixelBlue(p)*GetPixelBlue(p)*GetPixelBlue(p);
      histogram[ScaleQuantumToMap(GetPixelRed(p))].red++;
      histogram[ScaleQuantumToMap(GetPixelGreen(p))].green++;
      histogram[ScaleQuantumToMap(GetPixelBlue(p))].blue++;
      if (image->matte != MagickFalse)
        {
          if ((double) GetPixelOpacity(p) < channel_statistics[OpacityChannel].minima)
            channel_statistics[OpacityChannel].minima=(double)
              GetPixelOpacity(p);
          if ((double) GetPixelOpacity(p) > channel_statistics[OpacityChannel].maxima)
            channel_statistics[OpacityChannel].maxima=(double)
              GetPixelOpacity(p);
          channel_statistics[OpacityChannel].sum+=GetPixelOpacity(p);
          channel_statistics[OpacityChannel].sum_squared+=(double)
            GetPixelOpacity(p)*GetPixelOpacity(p);
          channel_statistics[OpacityChannel].sum_cubed+=(double)
            GetPixelOpacity(p)*GetPixelOpacity(p)*GetPixelOpacity(p);
          channel_statistics[OpacityChannel].sum_fourth_power+=(double)
            GetPixelOpacity(p)*GetPixelOpacity(p)*GetPixelOpacity(p)*
            GetPixelOpacity(p);
          histogram[ScaleQuantumToMap(GetPixelOpacity(p))].opacity++;
        }
      if (image->colorspace == CMYKColorspace)
        {
          if ((double) GetPixelIndex(indexes+x) < channel_statistics[BlackChannel].minima)
            channel_statistics[BlackChannel].minima=(double)
              GetPixelIndex(indexes+x);
          if ((double) GetPixelIndex(indexes+x) > channel_statistics[BlackChannel].maxima)
            channel_statistics[BlackChannel].maxima=(double)
              GetPixelIndex(indexes+x);
          channel_statistics[BlackChannel].sum+=GetPixelIndex(indexes+x);
          channel_statistics[BlackChannel].sum_squared+=(double)
            GetPixelIndex(indexes+x)*GetPixelIndex(indexes+x);
          channel_statistics[BlackChannel].sum_cubed+=(double)
            GetPixelIndex(indexes+x)*GetPixelIndex(indexes+x)*
            GetPixelIndex(indexes+x);
          channel_statistics[BlackChannel].sum_fourth_power+=(double)
            GetPixelIndex(indexes+x)*GetPixelIndex(indexes+x)*
            GetPixelIndex(indexes+x)*GetPixelIndex(indexes+x);
          histogram[ScaleQuantumToMap(GetPixelIndex(indexes+x))].index++;
        }
      x++;
      p++;
    }
  }
  area=(double) image->columns*image->rows;
  for (i=0; i < (ssize_t) CompositeChannels; i++)
  {
    double
      mean;

    mean=channel_statistics[i].sum/area;
    channel_statistics[i].sum=mean;
    channel_statistics[i].sum_squared/=area;
    channel_statistics[i].sum_cubed/=area;
    channel_statistics[i].sum_fourth_power/=area;
    channel_statistics[i].mean=mean;
    channel_statistics[i].variance=channel_statistics[i].sum_squared;
    channel_statistics[i].standard_deviation=sqrt(
      channel_statistics[i].variance-(mean*mean));
  }
  for (i=0; i < (ssize_t) (MaxMap+1U); i++)
  {
    if (histogram[i].red > 0.0)
      number_bins.red++;
    if (histogram[i].green > 0.0)
      number_bins.green++;
    if (histogram[i].blue > 0.0)
      number_bins.blue++;
    if ((image->matte != MagickFalse) && (histogram[i].red > 0.0))
      number_bins.opacity++;
    if ((image->colorspace == CMYKColorspace) && (histogram[i].red > 0.0))
      number_bins.index++;
  }
  for (i=0; i < (ssize_t) (MaxMap+1U); i++)
  {
    histogram[i].red/=area;
    channel_statistics[RedChannel].entropy+=-histogram[i].red*
      MagickLog10(histogram[i].red)/MagickLog10((double) number_bins.red);
    histogram[i].green/=area;
    channel_statistics[GreenChannel].entropy+=-histogram[i].green*
      MagickLog10(histogram[i].green)/MagickLog10((double) number_bins.green);
    histogram[i].blue/=area;
    channel_statistics[BlueChannel].entropy+=-histogram[i].blue*
      MagickLog10(histogram[i].blue)/MagickLog10((double) number_bins.blue);
    if (image->matte != MagickFalse)
      {
        histogram[i].opacity/=area;
        channel_statistics[OpacityChannel].entropy+=-histogram[i].opacity*
          MagickLog10(histogram[i].opacity)/MagickLog10((double)
          number_bins.opacity);
      }
    if (image->colorspace == CMYKColorspace)
      {
        histogram[i].index/=area;
        channel_statistics[IndexChannel].entropy+=-histogram[i].index*
          MagickLog10(histogram[i].index)/MagickLog10((double)
          number_bins.index);
      }
  }
  for (i=0; i < (ssize_t) CompositeChannels; i++)
  {
    channel_statistics[CompositeChannels].depth=(size_t) EvaluateMax((double)
      channel_statistics[CompositeChannels].depth,(double)
      channel_statistics[i].depth);
    channel_statistics[CompositeChannels].minima=MagickMin(
      channel_statistics[CompositeChannels].minima,
      channel_statistics[i].minima);
    channel_statistics[CompositeChannels].maxima=EvaluateMax(
      channel_statistics[CompositeChannels].maxima,
      channel_statistics[i].maxima);
    channel_statistics[CompositeChannels].sum+=channel_statistics[i].sum;
    channel_statistics[CompositeChannels].sum_squared+=
      channel_statistics[i].sum_squared;
    channel_statistics[CompositeChannels].sum_cubed+=
      channel_statistics[i].sum_cubed;
    channel_statistics[CompositeChannels].sum_fourth_power+=
      channel_statistics[i].sum_fourth_power;
    channel_statistics[CompositeChannels].mean+=channel_statistics[i].mean;
    channel_statistics[CompositeChannels].variance+=
      channel_statistics[i].variance-channel_statistics[i].mean*
      channel_statistics[i].mean;
    channel_statistics[CompositeChannels].standard_deviation+=
      channel_statistics[i].variance-channel_statistics[i].mean*
      channel_statistics[i].mean;
    channel_statistics[CompositeChannels].entropy+=
      channel_statistics[i].entropy;
  }
  channels=3;
  if (image->matte != MagickFalse)
    channels++;
  if (image->colorspace == CMYKColorspace)
    channels++;
  channel_statistics[CompositeChannels].sum/=channels;
  channel_statistics[CompositeChannels].sum_squared/=channels;
  channel_statistics[CompositeChannels].sum_cubed/=channels;
  channel_statistics[CompositeChannels].sum_fourth_power/=channels;
  channel_statistics[CompositeChannels].mean/=channels;
  channel_statistics[CompositeChannels].variance/=channels;
  channel_statistics[CompositeChannels].standard_deviation=
    sqrt(channel_statistics[CompositeChannels].standard_deviation/channels);
  channel_statistics[CompositeChannels].kurtosis/=channels;
  channel_statistics[CompositeChannels].skewness/=channels;
  channel_statistics[CompositeChannels].entropy/=channels;
  for (i=0; i <= (ssize_t) CompositeChannels; i++)
  {
    if (channel_statistics[i].standard_deviation == 0.0)
      continue;
    channel_statistics[i].skewness=(channel_statistics[i].sum_cubed-
      3.0*channel_statistics[i].mean*channel_statistics[i].sum_squared+
      2.0*channel_statistics[i].mean*channel_statistics[i].mean*
      channel_statistics[i].mean)/(channel_statistics[i].standard_deviation*
      channel_statistics[i].standard_deviation*
      channel_statistics[i].standard_deviation);
    channel_statistics[i].kurtosis=(channel_statistics[i].sum_fourth_power-
      4.0*channel_statistics[i].mean*channel_statistics[i].sum_cubed+
      6.0*channel_statistics[i].mean*channel_statistics[i].mean*
      channel_statistics[i].sum_squared-3.0*channel_statistics[i].mean*
      channel_statistics[i].mean*1.0*channel_statistics[i].mean*
      channel_statistics[i].mean)/(channel_statistics[i].standard_deviation*
      channel_statistics[i].standard_deviation*
      channel_statistics[i].standard_deviation*
      channel_statistics[i].standard_deviation)-3.0;
  }
  histogram=(MagickPixelPacket *) RelinquishMagickMemory(histogram);
  if (y < (ssize_t) image->rows)
    channel_statistics=(ChannelStatistics *) RelinquishMagickMemory(
      channel_statistics);
  return(channel_statistics);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     P o l y n o m i a l I m a g e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  PolynomialImage() returns a new image where each pixel is the sum of the
%  pixels in the image sequence after applying its corresponding terms
%  (coefficient and degree pairs).
%
%  The format of the PolynomialImage method is:
%
%      Image *PolynomialImage(const Image *images,const size_t number_terms,
%        const double *terms,ExceptionInfo *exception)
%      Image *PolynomialImageChannel(const Image *images,
%        const size_t number_terms,const ChannelType channel,
%        const double *terms,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o images: the image sequence.
%
%    o channel: the channel.
%
%    o number_terms: the number of terms in the list.  The actual list length
%      is 2 x number_terms + 1 (the constant).
%
%    o terms: the list of polynomial coefficients and degree pairs and a
%      constant.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport Image *PolynomialImage(const Image *images,
  const size_t number_terms,const double *terms,ExceptionInfo *exception)
{
  Image
    *polynomial_image;

  polynomial_image=PolynomialImageChannel(images,DefaultChannels,number_terms,
    terms,exception);
  return(polynomial_image);
}

MagickExport Image *PolynomialImageChannel(const Image *images,
  const ChannelType channel,const size_t number_terms,const double *terms,
  ExceptionInfo *exception)
{
#define PolynomialImageTag  "Polynomial/Image"

  CacheView
    *polynomial_view;

  Image
    *image;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  MagickPixelPacket
    **magick_restrict polynomial_pixels,
    zero;

  size_t
    number_images;

  ssize_t
    y;

  assert(images != (Image *) NULL);
  assert(images->signature == MagickSignature);
  if (images->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",images->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  image=CloneImage(images,images->columns,images->rows,MagickTrue,exception);
  if (image == (Image *) NULL)
    return((Image *) NULL);
  if (SetImageStorageClass(image,DirectClass) == MagickFalse)
    {
      InheritException(exception,&image->exception);
      image=DestroyImage(image);
      return((Image *) NULL);
    }
  number_images=GetImageListLength(images);
  polynomial_pixels=AcquirePixelThreadSet(images,number_images);
  if (polynomial_pixels == (MagickPixelPacket **) NULL)
    {
      image=DestroyImage(image);
      (void) ThrowMagickException(exception,GetMagickModule(),
        ResourceLimitError,"MemoryAllocationFailed","`%s'",images->filename);
      return((Image *) NULL);
    }
  /*
    Polynomial image pixels.
  */
  status=MagickTrue;
  progress=0;
  GetMagickPixelPacket(images,&zero);
  polynomial_view=AcquireAuthenticCacheView(image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,4) shared(progress,status) \
    magick_threads(image,image,image->rows,1)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    CacheView
      *image_view;

    const Image
      *next;

    const int
      id = GetOpenMPThreadId();

    register IndexPacket
      *magick_restrict polynomial_indexes;

    register MagickPixelPacket
      *polynomial_pixel;

    register PixelPacket
      *magick_restrict q;

    register ssize_t
      i,
      x;

    if (status == MagickFalse)
      continue;
    q=QueueCacheViewAuthenticPixels(polynomial_view,0,y,image->columns,1,
      exception);
    if (q == (PixelPacket *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    polynomial_indexes=GetCacheViewAuthenticIndexQueue(polynomial_view);
    polynomial_pixel=polynomial_pixels[id];
    for (x=0; x < (ssize_t) image->columns; x++)
      polynomial_pixel[x]=zero;
    next=images;
    for (i=0; i < (ssize_t) number_images; i++)
    {
      register const IndexPacket
        *indexes;

      register const PixelPacket
        *p;

      if (i >= (ssize_t) number_terms)
        break;
      image_view=AcquireVirtualCacheView(next,exception);
      p=GetCacheViewVirtualPixels(image_view,0,y,image->columns,1,exception);
      if (p == (const PixelPacket *) NULL)
        {
          image_view=DestroyCacheView(image_view);
          break;
        }
      indexes=GetCacheViewVirtualIndexQueue(image_view);
      for (x=0; x < (ssize_t) image->columns; x++)
      {
        double
          coefficient,
          degree;

        coefficient=terms[i << 1];
        degree=terms[(i << 1)+1];
        if ((channel & RedChannel) != 0)
          polynomial_pixel[x].red+=coefficient*pow(QuantumScale*p->red,degree);
        if ((channel & GreenChannel) != 0)
          polynomial_pixel[x].green+=coefficient*pow(QuantumScale*p->green,
            degree);
        if ((channel & BlueChannel) != 0)
          polynomial_pixel[x].blue+=coefficient*pow(QuantumScale*p->blue,
            degree);
        if ((channel & OpacityChannel) != 0)
          polynomial_pixel[x].opacity+=coefficient*pow(QuantumScale*
            (QuantumRange-p->opacity),degree);
        if (((channel & IndexChannel) != 0) &&
            (image->colorspace == CMYKColorspace))
          polynomial_pixel[x].index+=coefficient*pow(QuantumScale*indexes[x],
            degree);
        p++;
      }
      image_view=DestroyCacheView(image_view);
      next=GetNextImageInList(next);
    }
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      SetPixelRed(q,ClampToQuantum(QuantumRange*polynomial_pixel[x].red));
      SetPixelGreen(q,ClampToQuantum(QuantumRange*polynomial_pixel[x].green));
      SetPixelBlue(q,ClampToQuantum(QuantumRange*polynomial_pixel[x].blue));
      if (image->matte == MagickFalse)
        SetPixelOpacity(q,ClampToQuantum(QuantumRange-QuantumRange*
          polynomial_pixel[x].opacity));
      else
        SetPixelAlpha(q,ClampToQuantum(QuantumRange-QuantumRange*
          polynomial_pixel[x].opacity));
      if (image->colorspace == CMYKColorspace)
        SetPixelIndex(polynomial_indexes+x,ClampToQuantum(QuantumRange*
          polynomial_pixel[x].index));
      q++;
    }
    if (SyncCacheViewAuthenticPixels(polynomial_view,exception) == MagickFalse)
      status=MagickFalse;
    if (images->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
        #pragma omp critical (MagickCore_PolynomialImages)
#endif
        proceed=SetImageProgress(images,PolynomialImageTag,progress++,
          image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  polynomial_view=DestroyCacheView(polynomial_view);
  polynomial_pixels=DestroyPixelThreadSet(polynomial_pixels);
  if (status == MagickFalse)
    image=DestroyImage(image);
  return(image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     S t a t i s t i c I m a g e                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  StatisticImage() makes each pixel the min / max / median / mode / etc. of
%  the neighborhood of the specified width and height.
%
%  The format of the StatisticImage method is:
%
%      Image *StatisticImage(const Image *image,const StatisticType type,
%        const size_t width,const size_t height,ExceptionInfo *exception)
%      Image *StatisticImageChannel(const Image *image,
%        const ChannelType channel,const StatisticType type,
%        const size_t width,const size_t height,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o channel: the image channel.
%
%    o type: the statistic type (median, mode, etc.).
%
%    o width: the width of the pixel neighborhood.
%
%    o height: the height of the pixel neighborhood.
%
%    o exception: return any errors or warnings in this structure.
%
*/

#define ListChannels  5

typedef struct _ListNode
{
  size_t
    next[9],
    count,
    signature;
} ListNode;

typedef struct _SkipList
{
  ssize_t
    level;

  ListNode
    *nodes;
} SkipList;

typedef struct _PixelList
{
  size_t
    length,
    seed,
    signature;

  SkipList
    lists[ListChannels];
} PixelList;

static PixelList *DestroyPixelList(PixelList *pixel_list)
{
  register ssize_t
    i;

  if (pixel_list == (PixelList *) NULL)
    return((PixelList *) NULL);
  for (i=0; i < ListChannels; i++)
    if (pixel_list->lists[i].nodes != (ListNode *) NULL)
      pixel_list->lists[i].nodes=(ListNode *) RelinquishMagickMemory(
        pixel_list->lists[i].nodes);
  pixel_list=(PixelList *) RelinquishMagickMemory(pixel_list);
  return(pixel_list);
}

static PixelList **DestroyPixelListThreadSet(PixelList **pixel_list)
{
  register ssize_t
    i;

  assert(pixel_list != (PixelList **) NULL);
  for (i=0; i < (ssize_t) GetMagickResourceLimit(ThreadResource); i++)
    if (pixel_list[i] != (PixelList *) NULL)
      pixel_list[i]=DestroyPixelList(pixel_list[i]);
  pixel_list=(PixelList **) RelinquishMagickMemory(pixel_list);
  return(pixel_list);
}

static PixelList *AcquirePixelList(const size_t width,const size_t height)
{
  PixelList
    *pixel_list;

  register ssize_t
    i;

  pixel_list=(PixelList *) AcquireMagickMemory(sizeof(*pixel_list));
  if (pixel_list == (PixelList *) NULL)
    return(pixel_list);
  (void) ResetMagickMemory((void *) pixel_list,0,sizeof(*pixel_list));
  pixel_list->length=width*height;
  for (i=0; i < ListChannels; i++)
  {
    pixel_list->lists[i].nodes=(ListNode *) AcquireQuantumMemory(65537UL,
      sizeof(*pixel_list->lists[i].nodes));
    if (pixel_list->lists[i].nodes == (ListNode *) NULL)
      return(DestroyPixelList(pixel_list));
    (void) ResetMagickMemory(pixel_list->lists[i].nodes,0,65537UL*
      sizeof(*pixel_list->lists[i].nodes));
  }
  pixel_list->signature=MagickSignature;
  return(pixel_list);
}

static PixelList **AcquirePixelListThreadSet(const size_t width,
  const size_t height)
{
  PixelList
    **pixel_list;

  register ssize_t
    i;

  size_t
    number_threads;

  number_threads=(size_t) GetMagickResourceLimit(ThreadResource);
  pixel_list=(PixelList **) AcquireQuantumMemory(number_threads,
    sizeof(*pixel_list));
  if (pixel_list == (PixelList **) NULL)
    return((PixelList **) NULL);
  (void) ResetMagickMemory(pixel_list,0,number_threads*sizeof(*pixel_list));
  for (i=0; i < (ssize_t) number_threads; i++)
  {
    pixel_list[i]=AcquirePixelList(width,height);
    if (pixel_list[i] == (PixelList *) NULL)
      return(DestroyPixelListThreadSet(pixel_list));
  }
  return(pixel_list);
}

static void AddNodePixelList(PixelList *pixel_list,const ssize_t channel,
  const size_t color)
{
  register SkipList
    *list;

  register ssize_t
    level;

  size_t
    search,
    update[9];

  /*
    Initialize the node.
  */
  list=pixel_list->lists+channel;
  list->nodes[color].signature=pixel_list->signature;
  list->nodes[color].count=1;
  /*
    Determine where it belongs in the list.
  */
  search=65536UL;
  for (level=list->level; level >= 0; level--)
  {
    while (list->nodes[search].next[level] < color)
      search=list->nodes[search].next[level];
    update[level]=search;
  }
  /*
    Generate a pseudo-random level for this node.
  */
  for (level=0; ; level++)
  {
    pixel_list->seed=(pixel_list->seed*42893621L)+1L;
    if ((pixel_list->seed & 0x300) != 0x300)
      break;
  }
  if (level > 8)
    level=8;
  if (level > (list->level+2))
    level=list->level+2;
  /*
    If we're raising the list's level, link back to the root node.
  */
  while (level > list->level)
  {
    list->level++;
    update[list->level]=65536UL;
  }
  /*
    Link the node into the skip-list.
  */
  do
  {
    list->nodes[color].next[level]=list->nodes[update[level]].next[level];
    list->nodes[update[level]].next[level]=color;
  } while (level-- > 0);
}

static void GetMaximumPixelList(PixelList *pixel_list,MagickPixelPacket *pixel)
{
  register SkipList
    *list;

  register ssize_t
    channel;

  size_t
    color,
    maximum;

  ssize_t
    count;

  unsigned short
    channels[ListChannels];

  /*
    Find the maximum value for each of the color.
  */
  for (channel=0; channel < 5; channel++)
  {
    list=pixel_list->lists+channel;
    color=65536L;
    count=0;
    maximum=list->nodes[color].next[0];
    do
    {
      color=list->nodes[color].next[0];
      if (color > maximum)
        maximum=color;
      count+=list->nodes[color].count;
    } while (count < (ssize_t) pixel_list->length);
    channels[channel]=(unsigned short) maximum;
  }
  pixel->red=(MagickRealType) ScaleShortToQuantum(channels[0]);
  pixel->green=(MagickRealType) ScaleShortToQuantum(channels[1]);
  pixel->blue=(MagickRealType) ScaleShortToQuantum(channels[2]);
  pixel->opacity=(MagickRealType) ScaleShortToQuantum(channels[3]);
  pixel->index=(MagickRealType) ScaleShortToQuantum(channels[4]);
}

static void GetMeanPixelList(PixelList *pixel_list,MagickPixelPacket *pixel)
{
  MagickRealType
    sum;

  register SkipList
    *list;

  register ssize_t
    channel;

  size_t
    color;

  ssize_t
    count;

  unsigned short
    channels[ListChannels];

  /*
    Find the mean value for each of the color.
  */
  for (channel=0; channel < 5; channel++)
  {
    list=pixel_list->lists+channel;
    color=65536L;
    count=0;
    sum=0.0;
    do
    {
      color=list->nodes[color].next[0];
      sum+=(MagickRealType) list->nodes[color].count*color;
      count+=list->nodes[color].count;
    } while (count < (ssize_t) pixel_list->length);
    sum/=pixel_list->length;
    channels[channel]=(unsigned short) sum;
  }
  pixel->red=(MagickRealType) ScaleShortToQuantum(channels[0]);
  pixel->green=(MagickRealType) ScaleShortToQuantum(channels[1]);
  pixel->blue=(MagickRealType) ScaleShortToQuantum(channels[2]);
  pixel->opacity=(MagickRealType) ScaleShortToQuantum(channels[3]);
  pixel->index=(MagickRealType) ScaleShortToQuantum(channels[4]);
}

static void GetMedianPixelList(PixelList *pixel_list,MagickPixelPacket *pixel)
{
  register SkipList
    *list;

  register ssize_t
    channel;

  size_t
    color;

  ssize_t
    count;

  unsigned short
    channels[ListChannels];

  /*
    Find the median value for each of the color.
  */
  for (channel=0; channel < 5; channel++)
  {
    list=pixel_list->lists+channel;
    color=65536L;
    count=0;
    do
    {
      color=list->nodes[color].next[0];
      count+=list->nodes[color].count;
    } while (count <= (ssize_t) (pixel_list->length >> 1));
    channels[channel]=(unsigned short) color;
  }
  GetMagickPixelPacket((const Image *) NULL,pixel);
  pixel->red=(MagickRealType) ScaleShortToQuantum(channels[0]);
  pixel->green=(MagickRealType) ScaleShortToQuantum(channels[1]);
  pixel->blue=(MagickRealType) ScaleShortToQuantum(channels[2]);
  pixel->opacity=(MagickRealType) ScaleShortToQuantum(channels[3]);
  pixel->index=(MagickRealType) ScaleShortToQuantum(channels[4]);
}

static void GetMinimumPixelList(PixelList *pixel_list,MagickPixelPacket *pixel)
{
  register SkipList
    *list;

  register ssize_t
    channel;

  size_t
    color,
    minimum;

  ssize_t
    count;

  unsigned short
    channels[ListChannels];

  /*
    Find the minimum value for each of the color.
  */
  for (channel=0; channel < 5; channel++)
  {
    list=pixel_list->lists+channel;
    count=0;
    color=65536UL;
    minimum=list->nodes[color].next[0];
    do
    {
      color=list->nodes[color].next[0];
      if (color < minimum)
        minimum=color;
      count+=list->nodes[color].count;
    } while (count < (ssize_t) pixel_list->length);
    channels[channel]=(unsigned short) minimum;
  }
  pixel->red=(MagickRealType) ScaleShortToQuantum(channels[0]);
  pixel->green=(MagickRealType) ScaleShortToQuantum(channels[1]);
  pixel->blue=(MagickRealType) ScaleShortToQuantum(channels[2]);
  pixel->opacity=(MagickRealType) ScaleShortToQuantum(channels[3]);
  pixel->index=(MagickRealType) ScaleShortToQuantum(channels[4]);
}

static void GetModePixelList(PixelList *pixel_list,MagickPixelPacket *pixel)
{
  register SkipList
    *list;

  register ssize_t
    channel;

  size_t
    color,
    max_count,
    mode;

  ssize_t
    count;

  unsigned short
    channels[5];

  /*
    Make each pixel the 'predominant color' of the specified neighborhood.
  */
  for (channel=0; channel < 5; channel++)
  {
    list=pixel_list->lists+channel;
    color=65536L;
    mode=color;
    max_count=list->nodes[mode].count;
    count=0;
    do
    {
      color=list->nodes[color].next[0];
      if (list->nodes[color].count > max_count)
        {
          mode=color;
          max_count=list->nodes[mode].count;
        }
      count+=list->nodes[color].count;
    } while (count < (ssize_t) pixel_list->length);
    channels[channel]=(unsigned short) mode;
  }
  pixel->red=(MagickRealType) ScaleShortToQuantum(channels[0]);
  pixel->green=(MagickRealType) ScaleShortToQuantum(channels[1]);
  pixel->blue=(MagickRealType) ScaleShortToQuantum(channels[2]);
  pixel->opacity=(MagickRealType) ScaleShortToQuantum(channels[3]);
  pixel->index=(MagickRealType) ScaleShortToQuantum(channels[4]);
}

static void GetNonpeakPixelList(PixelList *pixel_list,MagickPixelPacket *pixel)
{
  register SkipList
    *list;

  register ssize_t
    channel;

  size_t
    color,
    next,
    previous;

  ssize_t
    count;

  unsigned short
    channels[5];

  /*
    Finds the non peak value for each of the colors.
  */
  for (channel=0; channel < 5; channel++)
  {
    list=pixel_list->lists+channel;
    color=65536L;
    next=list->nodes[color].next[0];
    count=0;
    do
    {
      previous=color;
      color=next;
      next=list->nodes[color].next[0];
      count+=list->nodes[color].count;
    } while (count <= (ssize_t) (pixel_list->length >> 1));
    if ((previous == 65536UL) && (next != 65536UL))
      color=next;
    else
      if ((previous != 65536UL) && (next == 65536UL))
        color=previous;
    channels[channel]=(unsigned short) color;
  }
  pixel->red=(MagickRealType) ScaleShortToQuantum(channels[0]);
  pixel->green=(MagickRealType) ScaleShortToQuantum(channels[1]);
  pixel->blue=(MagickRealType) ScaleShortToQuantum(channels[2]);
  pixel->opacity=(MagickRealType) ScaleShortToQuantum(channels[3]);
  pixel->index=(MagickRealType) ScaleShortToQuantum(channels[4]);
}

static void GetRootMeanSquarePixelList(PixelList *pixel_list,
  MagickPixelPacket *pixel)
{
  MagickRealType
    sum;

  register SkipList
    *list;

  register ssize_t
    channel;

  size_t
    color;

  ssize_t
    count;

  unsigned short
    channels[ListChannels];

  /*
    Find the root mean square value for each of the color.
  */
  for (channel=0; channel < 5; channel++)
  {
    list=pixel_list->lists+channel;
    color=65536L;
    count=0;
    sum=0.0;
    do
    {
      color=list->nodes[color].next[0];
      sum+=(MagickRealType) (list->nodes[color].count*color*color);
      count+=list->nodes[color].count;
    } while (count < (ssize_t) pixel_list->length);
    sum/=pixel_list->length;
    channels[channel]=(unsigned short) sqrt(sum);
  }
  pixel->red=(MagickRealType) ScaleShortToQuantum(channels[0]);
  pixel->green=(MagickRealType) ScaleShortToQuantum(channels[1]);
  pixel->blue=(MagickRealType) ScaleShortToQuantum(channels[2]);
  pixel->opacity=(MagickRealType) ScaleShortToQuantum(channels[3]);
  pixel->index=(MagickRealType) ScaleShortToQuantum(channels[4]);
}

static void GetStandardDeviationPixelList(PixelList *pixel_list,
  MagickPixelPacket *pixel)
{
  MagickRealType
    sum,
    sum_squared;

  register SkipList
    *list;

  register ssize_t
    channel;

  size_t
    color;

  ssize_t
    count;

  unsigned short
    channels[ListChannels];

  /*
    Find the standard-deviation value for each of the color.
  */
  for (channel=0; channel < 5; channel++)
  {
    list=pixel_list->lists+channel;
    color=65536L;
    count=0;
    sum=0.0;
    sum_squared=0.0;
    do
    {
      register ssize_t
        i;

      color=list->nodes[color].next[0];
      sum+=(MagickRealType) list->nodes[color].count*color;
      for (i=0; i < (ssize_t) list->nodes[color].count; i++)
        sum_squared+=((MagickRealType) color)*((MagickRealType) color);
      count+=list->nodes[color].count;
    } while (count < (ssize_t) pixel_list->length);
    sum/=pixel_list->length;
    sum_squared/=pixel_list->length;
    channels[channel]=(unsigned short) sqrt(sum_squared-(sum*sum));
  }
  pixel->red=(MagickRealType) ScaleShortToQuantum(channels[0]);
  pixel->green=(MagickRealType) ScaleShortToQuantum(channels[1]);
  pixel->blue=(MagickRealType) ScaleShortToQuantum(channels[2]);
  pixel->opacity=(MagickRealType) ScaleShortToQuantum(channels[3]);
  pixel->index=(MagickRealType) ScaleShortToQuantum(channels[4]);
}

static inline void InsertPixelList(const Image *image,const PixelPacket *pixel,
  const IndexPacket *indexes,PixelList *pixel_list)
{
  size_t
    signature;

  unsigned short
    index;

  index=ScaleQuantumToShort(GetPixelRed(pixel));
  signature=pixel_list->lists[0].nodes[index].signature;
  if (signature == pixel_list->signature)
    pixel_list->lists[0].nodes[index].count++;
  else
    AddNodePixelList(pixel_list,0,index);
  index=ScaleQuantumToShort(GetPixelGreen(pixel));
  signature=pixel_list->lists[1].nodes[index].signature;
  if (signature == pixel_list->signature)
    pixel_list->lists[1].nodes[index].count++;
  else
    AddNodePixelList(pixel_list,1,index);
  index=ScaleQuantumToShort(GetPixelBlue(pixel));
  signature=pixel_list->lists[2].nodes[index].signature;
  if (signature == pixel_list->signature)
    pixel_list->lists[2].nodes[index].count++;
  else
    AddNodePixelList(pixel_list,2,index);
  index=ScaleQuantumToShort(GetPixelOpacity(pixel));
  signature=pixel_list->lists[3].nodes[index].signature;
  if (signature == pixel_list->signature)
    pixel_list->lists[3].nodes[index].count++;
  else
    AddNodePixelList(pixel_list,3,index);
  if (image->colorspace == CMYKColorspace)
    index=ScaleQuantumToShort(GetPixelIndex(indexes));
  signature=pixel_list->lists[4].nodes[index].signature;
  if (signature == pixel_list->signature)
    pixel_list->lists[4].nodes[index].count++;
  else
    AddNodePixelList(pixel_list,4,index);
}

static void ResetPixelList(PixelList *pixel_list)
{
  int
    level;

  register ListNode
    *root;

  register SkipList
    *list;

  register ssize_t
    channel;

  /*
    Reset the skip-list.
  */
  for (channel=0; channel < 5; channel++)
  {
    list=pixel_list->lists+channel;
    root=list->nodes+65536UL;
    list->level=0;
    for (level=0; level < 9; level++)
      root->next[level]=65536UL;
  }
  pixel_list->seed=pixel_list->signature++;
}

MagickExport Image *StatisticImage(const Image *image,const StatisticType type,
  const size_t width,const size_t height,ExceptionInfo *exception)
{
  Image
    *statistic_image;

  statistic_image=StatisticImageChannel(image,DefaultChannels,type,width,
    height,exception);
  return(statistic_image);
}

MagickExport Image *StatisticImageChannel(const Image *image,
  const ChannelType channel,const StatisticType type,const size_t width,
  const size_t height,ExceptionInfo *exception)
{
#define StatisticImageTag  "Statistic/Image"

  CacheView
    *image_view,
    *statistic_view;

  Image
    *statistic_image;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  PixelList
    **magick_restrict pixel_list;

  size_t
    neighbor_height,
    neighbor_width;

  ssize_t
    y;

  /*
    Initialize statistics image attributes.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  statistic_image=CloneImage(image,image->columns,image->rows,MagickTrue,
    exception);
  if (statistic_image == (Image *) NULL)
    return((Image *) NULL);
  if (SetImageStorageClass(statistic_image,DirectClass) == MagickFalse)
    {
      InheritException(exception,&statistic_image->exception);
      statistic_image=DestroyImage(statistic_image);
      return((Image *) NULL);
    }
  neighbor_width=width == 0 ? GetOptimalKernelWidth2D((double) width,0.5) :
    width;
  neighbor_height=height == 0 ? GetOptimalKernelWidth2D((double) height,0.5) :
    height;
  pixel_list=AcquirePixelListThreadSet(neighbor_width,neighbor_height);
  if (pixel_list == (PixelList **) NULL)
    {
      statistic_image=DestroyImage(statistic_image);
      ThrowImageException(ResourceLimitError,"MemoryAllocationFailed");
    }
  /*
    Make each pixel the min / max / median / mode / etc. of the neighborhood.
  */
  status=MagickTrue;
  progress=0;
  image_view=AcquireVirtualCacheView(image,exception);
  statistic_view=AcquireAuthenticCacheView(statistic_image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,4) shared(progress,status) \
    magick_threads(image,statistic_image,statistic_image->rows,1)
#endif
  for (y=0; y < (ssize_t) statistic_image->rows; y++)
  {
    const int
      id = GetOpenMPThreadId();

    register const IndexPacket
      *magick_restrict indexes;

    register const PixelPacket
      *magick_restrict p;

    register IndexPacket
      *magick_restrict statistic_indexes;

    register PixelPacket
      *magick_restrict q;

    register ssize_t
      x;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(image_view,-((ssize_t) neighbor_width/2L),y-
      (ssize_t) (neighbor_height/2L),image->columns+neighbor_width,
      neighbor_height,exception);
    q=QueueCacheViewAuthenticPixels(statistic_view,0,y,statistic_image->columns,      1,exception);
    if ((p == (const PixelPacket *) NULL) || (q == (PixelPacket *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    indexes=GetCacheViewVirtualIndexQueue(image_view);
    statistic_indexes=GetCacheViewAuthenticIndexQueue(statistic_view);
    for (x=0; x < (ssize_t) statistic_image->columns; x++)
    {
      MagickPixelPacket
        pixel;

      register const IndexPacket
        *magick_restrict s;

      register const PixelPacket
        *magick_restrict r;

      register ssize_t
        u,
        v;

      r=p;
      s=indexes+x;
      ResetPixelList(pixel_list[id]);
      for (v=0; v < (ssize_t) neighbor_height; v++)
      {
        for (u=0; u < (ssize_t) neighbor_width; u++)
          InsertPixelList(image,r+u,s+u,pixel_list[id]);
        r+=image->columns+neighbor_width;
        s+=image->columns+neighbor_width;
      }
      GetMagickPixelPacket(image,&pixel);
      SetMagickPixelPacket(image,p+neighbor_width*neighbor_height/2,indexes+x+
        neighbor_width*neighbor_height/2,&pixel);
      switch (type)
      {
        case GradientStatistic:
        {
          MagickPixelPacket
            maximum,
            minimum;

          GetMinimumPixelList(pixel_list[id],&pixel);
          minimum=pixel;
          GetMaximumPixelList(pixel_list[id],&pixel);
          maximum=pixel;
          pixel.red=MagickAbsoluteValue(maximum.red-minimum.red);
          pixel.green=MagickAbsoluteValue(maximum.green-minimum.green);
          pixel.blue=MagickAbsoluteValue(maximum.blue-minimum.blue);
          pixel.opacity=MagickAbsoluteValue(maximum.opacity-minimum.opacity);
          if (image->colorspace == CMYKColorspace)
            pixel.index=MagickAbsoluteValue(maximum.index-minimum.index);
          break;
        }
        case MaximumStatistic:
        {
          GetMaximumPixelList(pixel_list[id],&pixel);
          break;
        }
        case MeanStatistic:
        {
          GetMeanPixelList(pixel_list[id],&pixel);
          break;
        }
        case MedianStatistic:
        default:
        {
          GetMedianPixelList(pixel_list[id],&pixel);
          break;
        }
        case MinimumStatistic:
        {
          GetMinimumPixelList(pixel_list[id],&pixel);
          break;
        }
        case ModeStatistic:
        {
          GetModePixelList(pixel_list[id],&pixel);
          break;
        }
        case NonpeakStatistic:
        {
          GetNonpeakPixelList(pixel_list[id],&pixel);
          break;
        }
        case RootMeanSquareStatistic:
        {
          GetRootMeanSquarePixelList(pixel_list[id],&pixel);
          break;
        }
        case StandardDeviationStatistic:
        {
          GetStandardDeviationPixelList(pixel_list[id],&pixel);
          break;
        }
      }
      if ((channel & RedChannel) != 0)
        SetPixelRed(q,ClampToQuantum(pixel.red));
      if ((channel & GreenChannel) != 0)
        SetPixelGreen(q,ClampToQuantum(pixel.green));
      if ((channel & BlueChannel) != 0)
        SetPixelBlue(q,ClampToQuantum(pixel.blue));
      if ((channel & OpacityChannel) != 0)
        SetPixelOpacity(q,ClampToQuantum(pixel.opacity));
      if (((channel & IndexChannel) != 0) &&
          (image->colorspace == CMYKColorspace))
        SetPixelIndex(statistic_indexes+x,ClampToQuantum(pixel.index));
      p++;
      q++;
    }
    if (SyncCacheViewAuthenticPixels(statistic_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
        #pragma omp critical (MagickCore_StatisticImage)
#endif
        proceed=SetImageProgress(image,StatisticImageTag,progress++,
          image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  statistic_view=DestroyCacheView(statistic_view);
  image_view=DestroyCacheView(image_view);
  pixel_list=DestroyPixelListThreadSet(pixel_list);
  if (status == MagickFalse)
    statistic_image=DestroyImage(statistic_image);
  return(statistic_image);
}
