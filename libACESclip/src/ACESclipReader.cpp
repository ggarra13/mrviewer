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

#include <iostream>

#ifdef _WIN32
#define strtod_l _strtod_l
#endif

#include "ACESclipReader.h"


namespace ACES {

using namespace tinyxml2;



/** 
 * Set status of a transform
 * 
 * @param s kPreview or kApplied
 */
ACESclipReader::BitDepth ACESclipReader::get_bit_depth( const std::string& d )
{
    if ( d == "10i" ) return k10i;
    if ( d == "12i" ) return k12i;
    if ( d == "16i" ) return k16i;
    if ( d == "16f" ) return k16f;
    if ( d == "32f" ) return k32f;
    return kLastBitDepth;
}

/** 
 * Set status of a transform
 * 
 * @param s kPreview or kApplied
 */
std::string ACESclipReader::date_time( const char* i )
{
    if (!i) return "";

    std::string s = i;
    size_t e = s.find('T');
    std::string r = s.substr(0, e );
    r += " Time: ";
    r += s.substr( e+1, s.size() );
    return r;
}

/** 
 * Set status of a transform
 * 
 * @param s kPreview or kApplied
 */
TransformStatus ACESclipReader::get_status( const std::string& s )
{
    if ( s == "applied" ) return kApplied;
    else return kPreview;
}


/** 
 * Parse a vector of 3 float numbers
 * 
 * @param s    3 float numbers as a string separated by spaces
 * @param out  the 3 float numbers
 */
void ACESclipReader::parse_V3( const char* v3, float out[3]  )
{
    char* s = strdup( v3 );
    char* t = s;
    int idx = 0;
    char* e = s;
    for ( ; *e != 0 || idx == 3; ++e )
    {
        if ( *e == ' ') {
            *e = 0;
            out[idx++] = (float) strtod_l( s, &e, loc );
            ++e;
            s = e;
        }
    }
    free( t );
}

/** 
 * Constructor
 * 
 */
ACESclipReader::ACESclipReader()
{
#ifdef _WIN32
    // The following line should in theory work, but it doesn't
    // _loc = _create_locale( LC_ALL, "en-US" );
    // We instead use a full name
    loc = _create_locale( LC_ALL, "English" );
#else
    locale_t empty;
    memset( &empty, 0, sizeof(locale_t) );
    loc = newlocale( LC_ALL, "en_US.UTF-8", empty );
#endif

}

ACESclipReader::~ACESclipReader()
{
#ifdef _WIN32
    _free_locale( loc );
#else
    freelocale( loc );
#endif
}

/** 
 * Standard header.
 * 
 * @return XML_NO_ERROR on success, XML_ERROR_FILE_READ_ERROR on failure
 */
XMLError ACESclipReader::header()
{
    root = doc.FirstChildElement("aces:ACESmetadata");
    if ( !root ) return XML_ERROR_FILE_READ_ERROR;

    element = root->FirstChildElement( "ContainerFormatVersion" );
    if ( !element ) return XML_ERROR_PARSING_ELEMENT;
    float version = atof( element->GetText() );
    if ( version > 1.0f )
        return XML_ERROR_FILE_READ_ERROR;

    return XML_NO_ERROR;
}

XMLError ACESclipReader::info()
{
    root2 = root->FirstChildElement( "aces:Info" );
    if (!root2) 
        return XML_ERROR_PARSING_ELEMENT;

    element = root2->FirstChildElement( "Application" );
    if ( element )
    {
        const char* tmp = element->GetText();
        if ( tmp ) application = tmp;

        tmp = element->Attribute("version");
        if ( tmp ) version = tmp;
    }

    element = root2->FirstChildElement( "Comment" );
    if ( element )
    {
        const char* tmp = element->GetText();
        if ( tmp ) comment = tmp;
    }

    return XML_NO_ERROR;
}

XMLError ACESclipReader::clip_id()
{
    root2 = root->FirstChildElement( "aces:ClipID" );
    if ( !root2 ) return XML_ERROR_PARSING_ELEMENT;

    element = root2->FirstChildElement( "ClipName" );
    if ( element )
    {
        const char* tmp = element->GetText();
        if ( tmp ) clip_name = tmp;
    }

    element = root2->FirstChildElement( "Source_MediaID" );
    if ( element )
    {
        const char* tmp = element->GetText();
        if ( tmp ) media_id = tmp;
    }

    element = root2->FirstChildElement( "ClipDate" );
    if ( element )
    {
        const char* tmp = element->GetText();
        if ( tmp ) clip_date = date_time( tmp );
    }

    return XML_NO_ERROR;
}

XMLError ACESclipReader::config()
{
    root2 = root->FirstChildElement( "aces:Config" );
    if ( !root2 ) return XML_ERROR_PARSING_ELEMENT;

    element = root2->FirstChildElement( "ACESrelease_Version" );
    if ( element )
    {
        const char* tmp = element->GetText();
        if ( tmp )
        {
            double version = atof( tmp );
            if ( version > 1.0 )
            {
                return XML_ERROR_FILE_READ_ERROR;
            }
        }
    }

    element = root2->FirstChildElement( "ClipDate" );
    if ( element )
    {
        const char* tmp = element->GetText();
        if ( tmp ) timestamp = tmp;
    }

    return XML_NO_ERROR;
}

XMLError ACESclipReader::GradeRef()
{
    element = root3->FirstChildElement( "aces:GradeRef" );
    if ( !element ) return XML_NO_ERROR;

    root4 = element;
    element = root4->FirstChildElement( "Convert_to_WorkSpace" );
    if ( !element ) return XML_ERROR_FILE_READ_ERROR;

    const char* tmp = element->Attribute( "TransformID" );
    if ( ! tmp ) return XML_ERROR_FILE_READ_ERROR;

    convert_to = tmp;

    XMLNode* root5 = root4->NextSiblingElement( "ColorDecisionList" );
    if ( !root5 ) return XML_ERROR_FILE_READ_ERROR;

    element = root5->FirstChildElement( "ASC CDL" );
    if ( !element ) return XML_ERROR_FILE_READ_ERROR;

    tmp = element->Attribute( "inBitDepth" );
    if ( tmp )
    {
        std::string depth = tmp;
        in_bit_depth = get_bit_depth( depth );
    }
    tmp = element->Attribute( "outBitDepth" );
    if ( tmp )
    {
        std::string depth = tmp;
        out_bit_depth = get_bit_depth( depth );
    }

    XMLNode* root6 = root5->FirstChildElement( "SOPNode" );
    if ( root6 )
    {
        float out[3];
        element = root6->FirstChildElement( "Slope" );
        if ( element )
        {
            const char* tmp = element->GetText();
            if ( tmp )
            {
                parse_V3( tmp, out );
                sops.slope( out );
            }
        }
        element = root6->NextSiblingElement( "Offset" );
        if ( element )
        {
            const char* tmp = element->GetText();
            if ( tmp )
            {
                parse_V3( tmp, out );
                sops.offset( out );
            }
        }
        element = root6->NextSiblingElement( "Power" );
        if ( element )
        {
            const char* tmp = element->GetText();
            if ( tmp )
            {
                parse_V3( tmp, out );
                sops.power( out );
            }
        }
    }

    root6 = root5->FirstChildElement( "SatNode" );
    if ( root5 )
    {
        element = root6->FirstChildElement( "Saturation" );
        if ( element )
        {
            const char* s = element->GetText();
            if ( s )
            {
                char* e = (char*) s + strlen(s) - 1;
                sops.saturation( (float) strtod_l( s, &e, loc ) );
            }
        }
    }

    element = root4->FirstChildElement( "Convert_from_WorkSpace" );
    if ( !element ) return XML_ERROR_FILE_READ_ERROR;

    tmp = element->Attribute( "TransformID" );
    if ( ! tmp ) return XML_ERROR_FILE_READ_ERROR;

    convert_from = tmp;

    return XML_NO_ERROR;
}

XMLError ACESclipReader::ITL()
{
    root3 = root2->FirstChildElement( "aces:InputTransformList" );
    if ( !root3 ) {
        root3 = root2->FirstChildElement( "InputTransformList" );
        if ( !root3 ) return XML_ERROR_PARSING_ELEMENT;
    }


    std::string name, link_transform;
    TransformStatus status = kPreview;
    element = root3->FirstChildElement( "aces:IDTref" );
    if ( element )
    {
        root4 = element;
        const char* tmp = element->Attribute( "TransformID" );
        if ( tmp ) name = tmp;
        else
        {
            // For backwards compatibility
            tmp = element->Attribute( "name" );
            if ( tmp ) name = tmp;
        }

        tmp = element->Attribute( "status" );
        if ( tmp ) status = get_status( tmp );

        element = root4->FirstChildElement( "LinkTransform" );
        if ( element )
        {
            const char* tmp = element->GetText();
            if ( tmp ) link_transform = tmp;
        }
    }

    IDT.name = name;
    IDT.link_transform = link_transform;
    IDT.status = status;

    XMLError err = GradeRef();
    if ( err != XML_NO_ERROR )
        return err;


    element = root3->FirstChildElement( "LinkInputTransformList" );
    if ( element )
    {
        const char* tmp = element->GetText();
        if ( tmp ) link_ITL = tmp;
    }

    return XML_NO_ERROR;
}

XMLError ACESclipReader::PTL()
{
    root3 = root2->FirstChildElement( "aces:PreviewTransformList" );
    if ( !root3 ) {
        root3 = root2->FirstChildElement( "PreviewTransformList" );
        if ( !root3 ) return XML_ERROR_PARSING_ELEMENT;
    }

    element = root3->FirstChildElement( "aces:LMTref" );
    while( element )
    {
        std::string name, link_transform;
        TransformStatus status = kPreview;
        if ( element )
        {

	  const char* tmp = element->Attribute( "TransformID" );
	  if ( tmp ) name = tmp;
          else
          {
              // For backwards compatibility
              tmp = element->Attribute( "name" );
              if ( tmp ) name = tmp;
          }

	  tmp = element->Attribute( "status" );
	  if ( tmp ) status = get_status( tmp );

	  root4 = element;
	  XMLElement* elem = root4->FirstChildElement( "LinkTransform" );
	  if ( elem )
	    {
	      const char* tmp = elem->GetText();
	      if ( tmp ) link_transform = tmp;
	    }

	  LMT.push_back( Transform( name, link_transform, status ) );

	  
	  element = element->NextSiblingElement( "aces:LMTref" );
	}
    }

    std::string name;
    TransformStatus status = kPreview;
    element = root3->FirstChildElement( "aces:RRTODTref" );
    if ( element )
    {
        const char* tmp = element->Attribute( "TransformID" );
        if ( tmp ) name = tmp;

        tmp = element->Attribute( "status" );
        if ( tmp ) status = get_status( tmp );

        RRTODT.name = name;
        RRTODT.status = status;
    }
    else
    {
        std::string name;
        TransformStatus status = kPreview;
        element = root3->FirstChildElement( "aces:RRTref" );
        if ( element )
        {
            root4 = element;

            const char* tmp = element->Attribute( "TransformID" );
            if ( tmp ) name = tmp;
            else
            {
                // For backwards compatibility
                tmp = element->Attribute( "name" );
                if ( tmp ) name = tmp;
            }

            tmp = element->Attribute( "status" );
            if ( tmp ) status = get_status( tmp );
        }

        RRT.name = name;
        RRT.status = status;
    }


    {
        std::string name, link_transform;
        TransformStatus status = kPreview;
        element = root3->FirstChildElement( "aces:ODTref" );
        if ( element )
        {
            root4 = element;

            const char* tmp = element->Attribute( "TransformID" );
            if ( tmp ) name = tmp;
            else
            {
                // For backwards compatibility
                tmp = element->Attribute( "name" );
                if ( tmp ) name = tmp;
            }

            tmp = element->Attribute( "status" );
            if ( tmp ) status = get_status( tmp );

            element = root4->FirstChildElement( "LinkTransform" );
            if ( element )
            {
                const char* tmp = element->GetText();
                if ( tmp ) link_transform = tmp;
            }
        }

        ODT.name = name;
        ODT.link_transform = link_transform;
        ODT.status = status;
    }


    element = root3->FirstChildElement( "LinkPreviewTransformList" );
    if ( element )
    {
        const char* tmp = element->GetText();
        if ( tmp ) link_PTL = tmp;
    }

    return XML_NO_ERROR;
}

/** 
 * First Step.  Load the XML file.
 * 
 * @param filename file to load the XML file from. 
 * 
 * @return true on success, false on failure
 */
bool ACESclipReader::load( const char* filename )
{
    XMLError err = doc.LoadFile( filename );
    if ( err != XML_NO_ERROR ) return false;

    err = header();
    if ( err != XML_NO_ERROR ) return false;

    err = info();
    if ( err != XML_NO_ERROR ) return false;

    err = clip_id();
    if ( err != XML_NO_ERROR ) return false;

    err = config();
    if ( err != XML_NO_ERROR ) return false;

    err = ITL();
    if ( err != XML_NO_ERROR ) return false;

    err = PTL();
    if ( err != XML_NO_ERROR ) return false;

    return true;
}


}  // namespace ACES


