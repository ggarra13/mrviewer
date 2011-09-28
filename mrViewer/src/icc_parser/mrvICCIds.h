/**
 * @file   mrvICCIds.h
 * @author gga
 * @date   Sat Feb 16 08:28:27 2008
 * 
 * @brief  
 * 
 * 
 */

#ifndef mrvICCIds_h
#define mrvICCIds_h

#define ICC_TAG(a,b,c,d) (unsigned int) ( (unsigned int)(d << 24)	\
					   + (unsigned int)(c << 16)	\
					   + (unsigned int)(b << 8)	\
					   + (unsigned int)a)

#define MAKE_TAG( name, a, b, c, d ) const unsigned name = ICC_TAG(a,b,c,d)

namespace mrv {

  namespace icc {

    const Imath::V3f kD50WhitePoint( 0.9642f, 1.0f, 0.8249f );
    const Imath::V3f kD50BlackPoint( 0.f, 0.f, 0.f );

    // Profile Tags
    MAKE_TAG( inputDeviceTag,    's', 'c', 'n', 'r' );
    MAKE_TAG( displayDeviceTag,  'm', 'n', 't', 'r' );
    MAKE_TAG( outputDeviceTag,   'p', 'r', 't', 'r' );
    MAKE_TAG( deviceLinkTag,     'l', 'i', 'n', 'k' );
    MAKE_TAG( colorSpaceConvTag, 's', 'p', 'a', 'c' );
    MAKE_TAG( abstractTag,       'a', 'b', 's', 't' );
    MAKE_TAG( namedColorProfile, 'n', 'm', 'c', 'l' );

    // Color spaces
    MAKE_TAG( XYZData,           'X', 'Y', 'Z', ' ' );
    MAKE_TAG( labData,           'L', 'a', 'b', ' ' );
    MAKE_TAG( luvData,           'L', 'u', 'v', ' ' );
    MAKE_TAG( YCbCrData,         'Y', 'C', 'b', 'r' );
    MAKE_TAG( YxyData,           'Y', 'x', 'y', ' ' );
    MAKE_TAG( rgbData,           'R', 'G', 'B', ' ' );
    MAKE_TAG( grayData,          'G', 'R', 'A', 'Y' );
    MAKE_TAG( hsvData,           'H', 'S', 'V', ' ' );
    MAKE_TAG( hlsData,           'H', 'L', 'S', ' ' );
    MAKE_TAG( cmykData,          'C', 'M', 'Y', 'K' );
    MAKE_TAG( cmyData,           'C', 'M', 'Y', ' ' );
    MAKE_TAG( color2Data,        '2', 'C', 'L', 'R' );
    MAKE_TAG( color3Data,        '3', 'C', 'L', 'R' );
    MAKE_TAG( color4Data,        '4', 'C', 'L', 'R' );
    MAKE_TAG( color5Data,        '5', 'C', 'L', 'R' );
    MAKE_TAG( color6Data,        '6', 'C', 'L', 'R' );
    MAKE_TAG( color7Data,        '7', 'C', 'L', 'R' );
    MAKE_TAG( color8Data,        '8', 'C', 'L', 'R' );
    MAKE_TAG( color9Data,        '9', 'C', 'L', 'R' );
    MAKE_TAG( color10Data,       'A', 'C', 'L', 'R' );
    MAKE_TAG( color11Data,       'B', 'C', 'L', 'R' );
    MAKE_TAG( color12Data,       'C', 'C', 'L', 'R' );
    MAKE_TAG( color13Data,       'D', 'C', 'L', 'R' );
    MAKE_TAG( color14Data,       'E', 'C', 'L', 'R' );
    MAKE_TAG( color15Data,       'F', 'C', 'L', 'R' );

    // Platform Tags
    MAKE_TAG( appleTag,          'A', 'P', 'P', 'L' );
    MAKE_TAG( microsoftTag,      'M', 'S', 'F', 'T' );
    MAKE_TAG( sgiTag,            'S', 'G', 'I', ' ' );
    MAKE_TAG( sunTag,            'S', 'U', 'N', 'W' );

    // Technology Tags
    MAKE_TAG( filmScannerTag,                'f', 's', 'c', 'n' );
    MAKE_TAG( digitalCameraTag,              'd', 'c', 'a', 'm' );
    MAKE_TAG( reflectiveScannerTag,          'r', 's', 'c', 'n' );
    MAKE_TAG( inkJetPrinterTag,              'i', 'j', 'e', 't' );
    MAKE_TAG( thermalWaxPrinterTag,          't', 'w', 'a', 'x' );
    MAKE_TAG( electrophotographicPrinterTag, 'e', 'p', 'h', 'o' );
    MAKE_TAG( dyeSublimationPrinterTag,      'd', 's', 'u', 'b' );
    MAKE_TAG( photographicPaperPrinterTag,   'r', 'p', 'h', 'o' );
    MAKE_TAG( filmWriterTag,                 'f', 'p', 'r', 'n' );
    MAKE_TAG( videoMonitorTag,               'v', 'i', 'd', 'm' );
    MAKE_TAG( videoCameraTag,                'v', 'i', 'd', 'c' );
    MAKE_TAG( projectionTelevisionTag,       'p', 'j', 't', 'v' );
    MAKE_TAG( cathodeRayTubeDisplayTag,      'C', 'R', 'T', ' ' );
    MAKE_TAG( passiveMatrixDisplayTag,       'P', 'M', 'D', ' ' );
    MAKE_TAG( activeeMatrixDisplayTag,       'A', 'M', 'D', ' ' );
    MAKE_TAG( photoCDTag,                    'K', 'P', 'C', 'D' );
    MAKE_TAG( photoImageSetterTag,           'i', 'm', 'g', 's' );
    MAKE_TAG( gravureTag,                    'g', 'r', 'a', 'v' );
    MAKE_TAG( offsetLithographyTag,          'o', 'f', 'f', 's' );
    MAKE_TAG( silkscreenTag,                 's', 'i', 'l', 'k' );
    MAKE_TAG( flexographyTag,                'f', 'l', 'e', 'x' );

    // TagEntry Tags
    MAKE_TAG( AtoB0Tag,                 'A', '2', 'B', '0' );
    MAKE_TAG( AtoB1Tag,                 'A', '2', 'B', '1' );
    MAKE_TAG( AtoB2Tag,                 'A', '2', 'B', '2' );
    MAKE_TAG( blueMatrixColumnTag,      'b', 'X', 'Y', 'Z' );
    MAKE_TAG( blueTRCTag,               'b', 'T', 'R', 'C' );
    MAKE_TAG( chromaticityTag,          'c', 'h', 'r', 'm' );
    MAKE_TAG( BtoA0Tag,                 'B', '2', 'A', '0' );
    MAKE_TAG( BtoA1Tag,                 'B', '2', 'A', '1' );
    MAKE_TAG( BtoA2Tag,                 'B', '2', 'A', '2' );
    MAKE_TAG( calibrationDateTimeTag,   'c', 'a', 'l', 't' );
    MAKE_TAG( charTargetTag,            't', 'a', 'r', 'g' );
    MAKE_TAG( chromaticAdaptationTag,   'c', 'h', 'a', 'd' );
    MAKE_TAG( colorantOrderTag,         'c', 'l', 'r', 'o' );
    MAKE_TAG( colorantTableTag,         'c', 'l', 'r', 't' );
    MAKE_TAG( colorantTableOutTag,      'c', 'l', 'o', 't' );
    MAKE_TAG( copyrightTag,             'c', 'p', 'r', 't' );
    MAKE_TAG( deviceMfgDescTag,         'd', 'm', 'n', 'd' );
    MAKE_TAG( deviceModelDescTag,       'd', 'm', 'd', 'd' );
    MAKE_TAG( gamutTag,                 'g', 'a', 'm', 't' );
    MAKE_TAG( grayTRCTag,               'k', 'T', 'R', 'C' );
    MAKE_TAG( greenMatrixColumnTag,     'g', 'X', 'Y', 'Z' );
    MAKE_TAG( greenTRCTag,              'g', 'T', 'R', 'C' );
    MAKE_TAG( luminanceTag,             'l', 'u', 'm', 'i' );
    MAKE_TAG( measurementTag,           'm', 'e', 'a', 's' );
    MAKE_TAG( mediaBlackPointTag,       'b', 'k', 'p', 't' );
    MAKE_TAG( mediaWhitePointTag,       'w', 't', 'p', 't' );
    MAKE_TAG( namedColor2Tag,           'n', 'c', 'l', '2' );
    MAKE_TAG( outputResponseTag,        'r', 'e', 's', 'p' );
    MAKE_TAG( preview0Tag,              'p', 'r', 'e', '0' );
    MAKE_TAG( preview1Tag,              'p', 'r', 'e', '1' );
    MAKE_TAG( preview2Tag,              'p', 'r', 'e', '2' );
    MAKE_TAG( profileDescriptionTag,    'd', 'e', 's', 'c' );
    MAKE_TAG( profileSequenceDescTag,   'p', 's', 'e', 'q' );
    MAKE_TAG( redMatrixColumnTag,       'r', 'X', 'Y', 'Z' );
    MAKE_TAG( redTRCTag,                'r', 'T', 'R', 'C' );
    MAKE_TAG( technologyTag,            't', 'e', 'c', 'h' );
    MAKE_TAG( viewingCondDescTag,       'v', 'u', 'e', 'd' );
    MAKE_TAG( viewingConditionsTag,     'v', 'i', 'e', 'w' );
    MAKE_TAG( MicrosoftWCSTag,          'M', 'S', '0', '0' );

    // Type tags
    MAKE_TAG( chromaticityTypeTag,          'c', 'h', 'r', 'm' );
    MAKE_TAG( colorantOrderTypeTag,         'c', 'l', 'r', 'o' );
    MAKE_TAG( colorantTableTypeTag,         'c', 'l', 'r', 't' );
    MAKE_TAG( curveTypeTag,                 'c', 'u', 'r', 'v' );
    MAKE_TAG( dataTypeTag,                  'd', 'a', 't', 'a' );
    MAKE_TAG( dateTimeTypeTag,              'd', 't', 'i', 'm' );
    MAKE_TAG( lut8TypeTag,                  'm', 'f', 't', '1' );
    MAKE_TAG( lut16TypeTag,                 'm', 'f', 't', '2' );
    MAKE_TAG( lutAtoBTypeTag,               'm', 'A', 'B', ' ' );
    MAKE_TAG( lutBtoATypeTag,               'm', 'B', 'A', ' ' );
    MAKE_TAG( measurementTypeTag,           'm', 'e', 'a', 's' );
    MAKE_TAG( multiLocalizedUnicodeTypeTag, 'm', 'l', 'u', 'c' );
    MAKE_TAG( namedColor2TypeTag,           'n', 'c', 'l', '2' );
    MAKE_TAG( parametricCurveTypeTag,       'p', 'a', 'r', 'a' );
    MAKE_TAG( profileDescriptionTypeTag,    'd', 'e', 's', 'c' );
    MAKE_TAG( profileSequenceDescTypeTag,   'p', 's', 'e', 'q' );
    MAKE_TAG( responseCurveSet16TypeTag,    'r', 'c', 's', '2' );
    MAKE_TAG( s15Fixed16ArrayTypeTag,       's', 'f', '3', '2' );
    MAKE_TAG( signatureTypeTag,             's', 'i', 'g', ' ' );
    MAKE_TAG( textTypeTag,                  't', 'e', 'x', 't' );
    MAKE_TAG( u16Fixed16ArrayTypeTag,       'u', 'f', '3', '2' );
    MAKE_TAG( uInt16ArrayTypeTag,           'u', 'i', '1', '6' );
    MAKE_TAG( uInt32ArrayTypeTag,           'u', 'i', '3', '2' );
    MAKE_TAG( uInt64ArrayTypeTag,           'u', 'i', '6', '4' );
    MAKE_TAG( uInt8ArrayTypeTag,            'u', 'i', '0', '8' );
    MAKE_TAG( viewingConditionsTypeTag,     'v', 'i', 'e', 'w' );
    MAKE_TAG( XYZTypeTag,                   'X', 'Y', 'Z', ' ' );
    MAKE_TAG( MS10TypeTag,                  'M', 'S', '1', '0' );

  }  // namespace icc

} // namespace mrv

#undef MAKE_TAG

#endif // mrvICCIds_h
