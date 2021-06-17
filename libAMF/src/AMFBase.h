#include <string>
#include <time.h>

typedef std::string tnInputTransform;
typedef std::string tnInverseOutputTransform;
typedef std::string tnInverseOutputDeviceTransform;
typedef std::string tnInverseReferenceRenderingTransform;
typedef std::string tnLookTransform;
typedef std::string tnColorSpaceConversionTransform;
typedef std::string tnOutputTransform;
typedef std::string tnOutputDeviceTransform;
typedef std::string tnReferenceRenderingTransform;
typedef std::string UUIDType;
typedef std::string anyURI;
typedef std::string hashAlgoType;


namespace {
    const std::string kTRANSFORM_ID = "urn:ampas:aces:transformId:v1.5:";
    const std::string kUUID = "urn:uuid:";
}

struct authorType
{
    std::string name;
    std::string emailAddress;
};

struct dateTimeType
{
    std::string creationDateTime;
    std::string modificationDateTime;

    dateTimeType()
        {
            time_t t = time(0);
            char buf[64];
            struct tm* now = localtime( &t );
            sprintf( buf, "%d-%02d-%02dT%02d:%02d:%02d",
                     now->tm_year+1900, now->tm_mon+1, now->tm_mday,
                     now->tm_hour, now->tm_min, now->tm_sec );
            creationDateTime = modificationDateTime = buf;
        };
};

struct versionType
{
    versionType() :
        majorVersion(1),
        minorVersion(1),
        patchVersion(0)
        {
        };

    short majorVersion;
    short minorVersion;
    short patchVersion;
};

struct pipelineInfoType
{
    std::string description;
    authorType author;
    dateTimeType dateTime;
    UUIDType uuid;
    versionType systemVersion;
};

struct hashType
{
    hashType() : algorithm( "sha256" ) {};
    std::string hash;
    hashAlgoType algorithm;
};

struct inverseOutputTransformType
{
    std::string description;
    hashType hash;
    UUIDType uuid;
    anyURI file;
    tnInverseOutputTransform transformId;
};

struct inverseOutputDeviceTransformType
{
    std::string description;
    hashType hash;
    UUIDType uuid;
    anyURI file;
    tnInverseOutputDeviceTransform transformId;
};

struct inverseReferenceRenderingTransformType
{
    std::string description;
    hashType hash;
    UUIDType uuid;
    anyURI file;
    tnInverseReferenceRenderingTransform transformId;
};

struct inputTransformType
{
    bool applied;
    bool enabled;
    std::string description;
    hashType hash;
    UUIDType uuid;
    anyURI file;
    tnInputTransform transformId;
    inverseOutputTransformType inverseOutputTransform;
    inverseOutputDeviceTransformType inverseOutputDeviceTransform;
    inverseReferenceRenderingTransformType inverseReferenceRenderingTransform;
};

struct toCdlWorkingSpaceType
{
    std::string description;
    hashType hash;
    UUIDType uuid;
    anyURI file;
    tnColorSpaceConversionTransform transformId;
};

struct fromCdlWorkingSpaceType
{
    std::string description;
    hashType hash;
    UUIDType uuid;
    anyURI file;
    tnColorSpaceConversionTransform transformId;
};

struct cdlWorkingSpaceType
{
    toCdlWorkingSpaceType   toCdlWorkingSpace;
    fromCdlWorkingSpaceType fromCdlWorkingSpace;
};

struct SOPNodeType
{
    SOPNodeType()
        {
            slope[0] = slope[1] = slope[2] = 1.0f;
            offset[0] = offset[1] = offset[2] = 0.0f;
            power[0] = power[1] = power[2] = 1.0f;
        }

    float slope[3];
    float offset[3];
    float power[3];
};

typedef float SatNodeType;

struct ColorCorrectionRefType
{
    anyURI ref;
};

struct lookTransformType
{
    bool applied;
    bool enabled;
    std::string description;
    hashType hash;
    UUIDType uuid;
    anyURI file;
    tnLookTransform transformId;
    cdlWorkingSpaceType cdlWorkingSpace;
    SOPNodeType SOPNode;
    SatNodeType SatNode;
    ColorCorrectionRefType ColorCorrectionRef;
};

struct outputDeviceTransformType
{
    std::string description;
    hashType hash;
    UUIDType uuid;
    anyURI file;
    tnOutputDeviceTransform transformId;
};

struct referenceRenderingTransformType
{
    std::string description;
    hashType hash;
    UUIDType uuid;
    anyURI file;
    tnReferenceRenderingTransform transformId;
};

struct outputTransformType
{
    std::string description;
    hashType hash;
    UUIDType uuid;
    anyURI file;
    tnOutputTransform transformId;
    outputDeviceTransformType outputDeviceTransform;
    referenceRenderingTransformType referenceRenderingTransform;
};

struct pipelineType
{
    pipelineInfoType pipelineInfo;
    inputTransformType inputTransform;
    lookTransformType lookTransform;
    outputTransformType outputTransform;
};

struct sequenceType
{
    std::string file;
    std::string idx;
    unsigned min;
    unsigned max;
};


struct clipIdType
{
    std::string clipName;
    sequenceType sequence;
    anyURI file;
    UUIDType uuid;
};

struct infoType
{
    std::string description;
    authorType author;
    dateTimeType dateTime;
    UUIDType uuid;
};

struct acesMetadataFile
{
    infoType amfInfo;
    clipIdType clipId;
    pipelineType pipeline;
    pipelineType archivedPipeline;
    int version;
    std::string xmins;

};
