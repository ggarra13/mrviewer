
#include <vector>

#include <fltk/run.h>

#include <tinyxml2.h>

#include <boost/uuid/uuid.hpp>            // uuid class
#include <boost/uuid/uuid_generators.hpp> // generators
#include <boost/uuid/uuid_io.hpp> // generators
#include <boost/lexical_cast.hpp> // for casting into std::string

#include "core/CMedia.h"
#include "gui/mrvVersion.h"
#include "gui/mrvPreferences.h"

namespace mrv {

bool save_aces_xml( const CMedia* img, const char* filename )
{
    using namespace tinyxml2;

    XMLDocument doc;
    XMLNode* pRoot, *pRoot2, *pRoot3, *pRoot4;
    XMLElement* pElement;
    XMLText* pText;
    char buf[256];
    char buf2[256];
    unsigned id = 1;

    XMLDeclaration* decl = doc.NewDeclaration( NULL );
    doc.InsertFirstChild( decl );

    pElement = doc.NewElement( "aces:ACESmetadata" );
    pElement->SetAttribute( "xmlns:aces", 
                            "http://www.oscars.org/aces/ref/acesmetadata");
    doc.InsertAfterChild( decl, pElement );
    pRoot = pElement;

    pElement = doc.NewElement("ContainerFormatVersion");
    pElement->SetText(1.0);
    pRoot->InsertEndChild( pElement );

    boost::uuids::uuid uuid = boost::uuids::random_generator()();
    std::string UUID = boost::lexical_cast<std::string>(uuid);
    

    pElement = doc.NewElement("UUID");
    pElement->SetText( UUID.c_str() );
    pRoot->InsertEndChild( pElement );

    pElement = doc.NewElement("ModificationTime");

    time_t t = time(0);   // get time now
    struct tm * now = localtime( & t );
    sprintf( buf, "%d-%02d-%02dT%02d:%02d:%02d",
             now->tm_year+1900, now->tm_mon+1, now->tm_mday,
             now->tm_hour, now->tm_min, now->tm_sec );
    pElement->SetText( buf );
    pRoot->InsertEndChild( pElement );

    pRoot2 = doc.NewElement( "aces:Info" );
    pRoot->InsertEndChild( pRoot2 );

    pElement = doc.NewElement("Application");
    pElement->SetAttribute( "version", mrv::version() );
    pElement->SetText( "mrViewer" );
    pRoot2->InsertEndChild( pElement );

    pElement = doc.NewElement("Comment");
    pElement->SetText( "mrViewer CTL settings" );
    pRoot2->InsertEndChild( pElement );

    pRoot2 = doc.NewElement( "aces:ClipID" );
    pRoot->InsertEndChild( pRoot2 );

    pElement = doc.NewElement("ClipName");
    pElement->SetText( img->fileroot() );
    pRoot2->InsertEndChild( pElement );

    const char* show = getenv("SHOW");
    if ( !show ) show = "Unknown Show";

    const char* shot = getenv("SHOT");
    if ( !shot ) shot = "Unknown Shot";

    std::string idname = show;
    idname += "-";
    idname += shot;

    pElement = doc.NewElement("Source_MediaID");
    pElement->SetText( idname.c_str() );
    pRoot2->InsertEndChild( pElement );

    time_t mt = img->mtime();
    now = localtime( &mt );
    sprintf( buf2, "%d-%02d-%02dT%02d:%02d:%02d",
             now->tm_year+1900, now->tm_mon+1, now->tm_mday,
             now->tm_hour, now->tm_min, now->tm_sec );

    pElement = doc.NewElement("ClipDate");
    pElement->SetText( buf2 );
    pRoot2->InsertEndChild( pElement );

    pRoot2 = doc.NewElement( "aces:Config" );
    pRoot->InsertEndChild( pRoot2 );

    pElement = doc.NewElement("ACESrelease_Version");
    pElement->SetText(1.0);
    pRoot2->InsertEndChild( pElement );

    pElement = doc.NewElement("Timestamp");
    
    pElement->SetText( buf );
    pRoot2->InsertEndChild( pElement );

    if ( img->idt_transform() )
    {
        pElement = doc.NewElement("InputTransformList");
        pRoot2->InsertEndChild( pElement );
        pRoot3 = pElement;

        std::string name = img->idt_transform();
        pElement = doc.NewElement( "aces:IDTref" );
        pElement->SetAttribute( "name", name.c_str() );
        pElement->SetAttribute( "status", "preview" );
        sprintf( buf, "id%d", id ); ++id;
        pElement->SetAttribute( "transformID", buf );
        pRoot3->InsertEndChild( pElement );
        pRoot4 = pElement;

        pElement = doc.NewElement( "LinkTransform" );
        pElement->SetText( name.c_str() );
        pRoot4->InsertEndChild( pElement );

#if 0
        pElement = doc.NewElement("aces:GradeRef");
        pElement->SetAttribute( "name", "CDL.prod.a01234.ctl" );
        pRoot2->InsertEndChild( pElement );
        pRoot3 = pElement;

        pElement = doc.NewElement( "Convert_to_WorkSpace" );
        pElement->SetAttribute( "name", "ACEScsc.ACES_to_ACEScc.a1.ctl" );
        pRoot3->InsertEndChild( pElement );

        pElement = doc.NewElement( "ColorDecisionList" );
        pRoot3->InsertEndChild( pElement );

        XMLNode* pRoot5 = pElement;
        pElement = doc.NewElement( "InputDescriptor" );
        pElement->SetText( "ACEScc" );
        pRoot5->InsertEndChild( pElement );

        pElement = doc.NewElement( "ASC_CDL" );
        pElement->SetAttribute( "id", "cc01234" );
        pElement->SetAttribute( "inBitDepth", "16f" );
        pElement->SetAttribute( "outBitDepth", "16f" );
        pRoot5->InsertEndChild( pElement );

        pElement = doc.NewElement( "SOPNode" );
        pRoot5->InsertEndChild( pElement );
        XMLNode* pRoot6 = pElement;

        pElement = doc.NewElement( "Description" );
        pElement->SetText( "Not a real example" );
        pRoot6->InsertEndChild( pElement );

        pElement = doc.NewElement( "SatNode" );
        pRoot5->InsertEndChild( pElement );
        pRoot6 = pElement;

        pElement = doc.NewElement( "Saturation" );
        pElement->SetText( 0.9 );
        pRoot6->InsertEndChild( pElement );

        pElement = doc.NewElement( "Convert_from_WorkSpace" );
        pElement->SetAttribute( "name", "ACEScc_to_ACES.a1.ctl" );
        pRoot4->InsertEndChild( pElement );
#endif

        pElement = doc.NewElement( "LinkInputTransformList" );
        pElement->SetText( "inputTransformID" );
        pRoot3->InsertEndChild( pElement );

    }

    if ( img->number_of_lmts() || img->rendering_transform() ||
         !mrv::Preferences::ODT_CTL_transform.empty() )
    {
        pElement = doc.NewElement("PreviewTransformList");
        pRoot2->InsertEndChild( pElement );
        pRoot3 = pElement;
    }

    if ( img->number_of_lmts() )
    {
        size_t num = img->number_of_lmts();
        size_t i = 0;
        for ( ; i < num; ++i )
        {
            std::string name = img->look_mod_transform(i);
            pElement = doc.NewElement( "aces:LMTref" );
            pElement->SetAttribute( "name", name.c_str() );
            pElement->SetAttribute( "status", "preview" );
            sprintf( buf, "id%d", id ); ++id;
            pElement->SetAttribute( "transformID", buf );
            pRoot3->InsertEndChild( pElement );
            pRoot4 = pElement;
         
            pElement = doc.NewElement( "LinkTransform" );
            pElement->SetText( name.c_str() );
            pRoot4->InsertEndChild( pElement );
        }
    }

    if ( img->rendering_transform() )
    {
        std::string name = img->rendering_transform();
        pElement = doc.NewElement( "aces:RRTref" );
        pElement->SetAttribute( "name", name.c_str() );
        pElement->SetAttribute( "status", "preview" );
        sprintf( buf, "id%d", id ); ++id;
        pElement->SetAttribute( "transformID", buf );
        pRoot3->InsertEndChild( pElement );
        pRoot4 = pElement;

        pElement = doc.NewElement( "LinkTransform" );
        pElement->SetText( name.c_str() );
        pRoot4->InsertEndChild( pElement );
    }

    if ( !mrv::Preferences::ODT_CTL_transform.empty() )
    {
        std::string name = mrv::Preferences::ODT_CTL_transform;

        pElement = doc.NewElement( "aces:ODTref" );
        pElement->SetAttribute( "name", name.c_str() );
        pElement->SetAttribute( "status", "preview" );
        sprintf( buf, "id%d", id ); ++id;
        pElement->SetAttribute( "transformID", buf );
        pRoot3->InsertEndChild( pElement );
        pRoot4 = pElement;

        pElement = doc.NewElement( "LinkTransform" );
        pElement->SetText( name.c_str() );
        pRoot4->InsertEndChild( pElement );
    }

    pElement = doc.NewElement( "LinkPreviewTransformList" );
    pElement->SetText( "CombinedLMT_RRT_ODT" );
    pRoot3->InsertEndChild( pElement );

#if 0
    pElement = doc.NewElement( "aces:TransformLibrary" );
    pRoot->InsertEndChild( pElement );
    pRoot2 = pElement;

    pElement = doc.NewElement( "ProcessList" );
    pElement->SetAttribute( "id", "UUID NAME HERE" );
    pRoot2->InsertEndChild( pElement );
    pRoot3 = pElement;

    pElement = doc.NewElement( "Description" );
    pElement->SetText( "Sample 3DLUT for an LMT" );
    pRoot3->InsertEndChild( pElement );

    pElement = doc.NewElement( "InputDescriptor" );
    pElement->SetText( "ACES" );
    pRoot3->InsertEndChild( pElement );

    pElement = doc.NewElement( "OutputDescriptor" );
    pElement->SetText( "ACES" );
    pRoot3->InsertEndChild( pElement );

    pElement = doc.NewElement( "LUT3D" );
    pElement->SetAttribute( "id", "lmt_prodv2" );
    pElement->SetAttribute( "name", "LMT Sequence 1 day exterior" );
    pElement->SetAttribute( "interpolation", "tetrahedral" );
    pElement->SetAttribute( "inBitDepth", "16f" );
    pElement->SetAttribute( "outBitDepth", "12i" );
    pRoot3->InsertEndChild( pElement );
    pRoot4 = pElement;

    pElement = doc.NewElement( "Description" );
    pElement->SetText( "LMT Test File" );
    pRoot4->InsertEndChild( pElement );

    pElement = doc.NewElement( "Array" );
    pElement->SetAttribute( "dim", "33 33 33 3" );
    pElement->SetText( "0 0 0" );
    pRoot4->InsertEndChild( pElement );
#endif

    XMLError err = doc.SaveFile( filename );

    return true;
}

}
