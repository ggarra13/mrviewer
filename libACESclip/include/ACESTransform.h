/* 
Copyright (c) 2015, Gonzalo Garramuño
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer. 
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are those
of the authors and should not be interpreted as representing official policies, 
either expressed or implied, of the FreeBSD Project.
*/

#ifndef ACESTransform_h
#define ACESTransform_h

#include <string>
#include <vector>
#include <ostream>
 
#include "ACESExport.h"

namespace ACES {

enum TransformStatus
{
kPreview,
kApplied,
kLastStatus
};

/** 
 * Transform:  A class to store the name of a transform and its status
 * 
 */
class ACES_EXPORT Transform
{
  public:
    Transform() : name(""), link_transform(""), status( kLastStatus ) {}

    Transform( std::string n, TransformStatus t) :
    name( n ),
    status( t )
    {}

    Transform( std::string n, std::string link, TransformStatus t) :
    name( n ),
    link_transform( link ),
    status( t )
    {}

    ~Transform() {}

    Transform( const Transform& b ) :
    name( b.name ),
    link_transform( b.link_transform ),
    status( b.status )
    {
    }

    friend std::ostream& operator<<( std::ostream& o, const Transform& t )
    {
        o << t.name << " status: ";
        if ( t.status == kPreview ) o << "preview";
        else o << "applied";
        if ( ! t.link_transform.empty() )
            o << " link transform: " << t.link_transform;
        return o;
    }

  public:
    std::string     name;
    std::string     link_transform;
    TransformStatus status;
};



}  // namespace ACES

#endif  // ACESTransform_h
