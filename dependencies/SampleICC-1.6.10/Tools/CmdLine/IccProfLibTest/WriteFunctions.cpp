#include "WriteFunctions.h"
#include <time.h>

//----------------------------------------------------
// Function Definitions
//----------------------------------------------------


bool AddTextTag(const icChar* ptext, CIccProfile *pIcc, icTagSignature sig)
{
	icChar buf[64];
	if (!pIcc) {
		return false;
	}

	if(pIcc->FindTag(sig)) {
		printf("Tag of signature %s already present.\n", icGetSig(buf, sig, false));
		return false;
	}

	switch(sig) {

		case icSigProfileDescriptionTag:
			{
				CIccTag* pTag=NULL;
				if(pIcc->m_Header.version<icVersionNumberV4) {
					pTag = CIccTag::Create(icSigTextDescriptionType);
					if(!pTag)
						return false;

					CIccTagTextDescription *pProfDescTag = (CIccTagTextDescription*)pTag;
					pProfDescTag->SetText(ptext);
				}
				else {
					pTag = CIccTag::Create(icSigMultiLocalizedUnicodeType);
					if(!pTag)
						return false;

					CIccTagMultiLocalizedUnicode *pProfDescTag = (CIccTagMultiLocalizedUnicode*)pTag;
					CIccLocalizedUnicode localized;
					localized.SetText(ptext);
					pProfDescTag->m_Strings->push_back(localized);
				}

				if(!pIcc->AttachTag(sig,pTag)) {
					delete pTag;
					return false;
				}

				break;
			}

		case icSigCopyrightTag:
			{
				CIccTag* pTag=NULL;
				if(pIcc->m_Header.version<icVersionNumberV4) {
					pTag = CIccTag::Create(icSigTextType);
					if(!pTag)
						return false;

					CIccTagText *pCopyTag = (CIccTagText*)pTag;
					pCopyTag->SetText(ptext);
				}
				else {
					pTag = CIccTag::Create(icSigMultiLocalizedUnicodeType);
					if(!pTag)
						return false;

					CIccTagMultiLocalizedUnicode *pCopyTag = (CIccTagMultiLocalizedUnicode*)pTag;
					CIccLocalizedUnicode localized;
					localized.SetText(ptext);
					pCopyTag->m_Strings->push_back(localized);
				}

				if(!pIcc->AttachTag(sig,pTag))
				{
					delete pTag;
					return false;
				}

				break;
			}

		default:
			return false;
	}

	return true;
}


//===================================================

bool WriteTag(CIccProfile *pIcc, icSignature sig)
{
  icChar buf[64];

  if(pIcc->FindTag(sig)) {
    printf("Tag of signature %s already present.\n", icGetSig(buf, sig, false));
    return false;
  }

  switch(sig) {

    case icSigChromaticityType:
      {

        CIccTagChromaticity *pTag = (CIccTagChromaticity*)CIccTag::Create(icSigChromaticityType);

        pTag->SetSize(3);
        icChromaticityNumber *pChrome = pTag->Getxy(0);
        pTag->m_nColorantType = icColorantITU;

        pChrome[0].x = icDtoUF(0.640f); pChrome[0].y = icDtoUF(0.330f);
        pChrome[1].x = icDtoUF(0.300f); pChrome[1].y = icDtoUF(0.600f);
        pChrome[2].x = icDtoUF(0.150f); pChrome[2].y = icDtoUF(0.060f);


        if(!pIcc->AttachTag(sig, pTag))
          return false;

        break;
      }

    case icSigColorantOrderType:
      {
        CIccTagColorantOrder *pTag = (CIccTagColorantOrder*)CIccTag::Create(icSigColorantOrderType);
        icUInt16Number nsize = pIcc->GetSpaceSamples(); 
        pTag->SetSize(nsize);

        
        icUInt8Number* pData = pTag->GetData(0);

        for (icUInt8Number i=0; i<nsize; i++)
          pData[i] = i+1;

        if(!pIcc->AttachTag(sig,pTag))
          return false;

        break;        
      }

    case icSigColorantTableType:
      {
        CIccTagColorantTable *pTag = (CIccTagColorantTable*)CIccTag::Create(icSigColorantTableType);
        icUInt16Number nsize = pIcc->GetSpaceSamples(); 
        pTag->SetSize(nsize);

        pTag->SetPCS(pIcc->m_Header.pcs);

        
        icColorantTableEntry* pEntry = pTag->GetEntry(0);

        if(pTag->GetPCS() == icSigXYZData) {
          for (icUInt8Number i=0; i<nsize; i++) {
            memset(&pEntry[i].name[0], 0, sizeof(pEntry[i].name));
            sprintf(pEntry[i].name, "Color %d", i+1);
            pEntry[i].data[0] = icDtoUSF(0.9642f);
            pEntry[i].data[1] = icDtoUSF(1.0f);
            pEntry[i].data[2] = icDtoUSF(0.8249f);
          }
        }
        else {
          icFloatNumber Lab[3];
          Lab[0] = 90.0; Lab[1] = -10.0; Lab[2] = -130.0;
          icLabToPcs(Lab);
          for (icUInt8Number i=0; i<nsize; i++) {
            memset(&pEntry[i].name[0], 0, sizeof(pEntry[i].name));
            sprintf(pEntry[i].name, "Color %d", i+1);
            pEntry[i].data[0] = icFtoU16(Lab[0]);
            pEntry[i].data[1] = icFtoU16(Lab[1]);
            pEntry[i].data[2] = icFtoU16(Lab[2]);
          }
        }

        if(!pIcc->AttachTag(sig,pTag))
          return false;

        break;        
      }

    case icSigDataType:
      {
        CIccTagData *pTag = (CIccTagData*)CIccTag::Create(icSigDataType);
        pTag->SetTypeAscii(false);
        pTag->SetSize(4);
        icUInt8Number* pData = pTag->GetData(0);

        if(pTag->IsTypeAscii()) {
          memset(&pData[0],0,sizeof(pData));
          sprintf((icChar*)pData, "HI");
        }
        else {
          pData[0] = 1; pData[1] = 2;
          pData[2] = 3; pData[3] = 4;
        }

        if(!pIcc->AttachTag(sig,pTag))
          return false;

        break;
      }

    case icSigDateTimeType:
      {
        CIccTagDateTime *pTag = (CIccTagDateTime*)CIccTag::Create(icSigDateTimeType);
        
        icDateTimeNumber nDateTime;

        struct tm *newtime;
        time_t long_time;

        time( &long_time );
        newtime = localtime( &long_time ); 

        nDateTime.year = newtime->tm_year+1900;
        nDateTime.month = newtime->tm_mon;
        nDateTime.day = newtime->tm_mday;
        nDateTime.hours = newtime->tm_hour;
        nDateTime.minutes = newtime->tm_min;
        nDateTime.seconds = newtime->tm_sec;

        pTag->SetDateTime(nDateTime);

        if(!pIcc->AttachTag(sig,pTag))
          return false;

        break;
      }

    case icSigDeviceMfgDescTag:
    case icSigDeviceModelDescTag:
    case icSigMultiLocalizedUnicodeType:
      {
        CIccTagMultiLocalizedUnicode *pTag = (CIccTagMultiLocalizedUnicode*)CIccTag::Create(icSigMultiLocalizedUnicodeType);
        CIccLocalizedUnicode localized;
        localized.SetText("MultiLocailized Tag Type.");

        pTag->m_Strings->push_back(localized);

        if(!pIcc->AttachTag(sig,pTag))
          return false;

        break;
        
      }

    case icSigProfileSequenceDescType:
      {
        CIccTagProfileSeqDesc *pTag = (CIccTagProfileSeqDesc*)CIccTag::Create(icSigProfileSequenceDescType);
        int version = 4;

        CIccProfileDescStruct desc, desc2;
        desc.m_deviceMfg = icSigMicrosoft;
        desc.m_deviceModel = pIcc->m_Header.model;
        desc.m_attributes = pIcc->m_Header.attributes;
        desc.m_technology = icSigCRTDisplay;
        desc2.m_deviceMfg = pIcc->m_Header.manufacturer;
        desc2.m_deviceModel = pIcc->m_Header.model;
        desc2.m_attributes = pIcc->m_Header.attributes;
        desc2.m_technology = icSigDigitalCamera;

        if(version == 4) {

          CIccTagMultiLocalizedUnicode *pLocalized, *pLocalized2;
          CIccLocalizedUnicode localized, localized2;

          desc.m_deviceMfgDesc.SetType(icSigMultiLocalizedUnicodeType);
          desc.m_deviceModelDesc.SetType(icSigMultiLocalizedUnicodeType);
          pLocalized = (CIccTagMultiLocalizedUnicode*)desc.m_deviceMfgDesc.GetTag();

          if (pLocalized) {
            localized.SetText("Device Mfg Desc1");
            pLocalized->m_Strings->push_back(localized);
          }

          pLocalized = (CIccTagMultiLocalizedUnicode*)desc.m_deviceModelDesc.GetTag();
          if (pLocalized) {
            localized.SetText("Device Model Desc1");
            pLocalized->m_Strings->push_back(localized);
          }

          desc2.m_deviceMfgDesc.SetType(icSigMultiLocalizedUnicodeType);
          desc2.m_deviceModelDesc.SetType(icSigMultiLocalizedUnicodeType);
          pLocalized2 = (CIccTagMultiLocalizedUnicode*)desc2.m_deviceMfgDesc.GetTag();

          if (pLocalized2) {
            localized2.SetText("Device Mfg Desc2");
            pLocalized2->m_Strings->push_back(localized2);
          }

          pLocalized2 = (CIccTagMultiLocalizedUnicode*)desc2.m_deviceModelDesc.GetTag();
          if (pLocalized2) {
            localized2.SetText("Device Model Desc2");
            pLocalized2->m_Strings->push_back(localized2);
          }
        
        }
        else {
          CIccTagTextDescription *pTextDesc, *pTextDesc2;

          desc.m_deviceMfgDesc.SetType(icSigTextDescriptionType);
          desc.m_deviceModelDesc.SetType(icSigTextDescriptionType);
          pTextDesc = (CIccTagTextDescription*)desc.m_deviceMfgDesc.GetTag();

          if (pTextDesc) {
            pTextDesc->SetText("Device Mfg Desc1");
          }

          pTextDesc = (CIccTagTextDescription*)desc.m_deviceModelDesc.GetTag();
          if (pTextDesc) {
            pTextDesc->SetText("Device Model Desc1");
          }

          desc2.m_deviceMfgDesc.SetType(icSigTextDescriptionType);
          desc2.m_deviceModelDesc.SetType(icSigTextDescriptionType);
          pTextDesc2 = (CIccTagTextDescription*)desc2.m_deviceMfgDesc.GetTag();

          if (pTextDesc2) {
            pTextDesc2->SetText("Device Mfg Desc2");
          }

          pTextDesc2 = (CIccTagTextDescription*)desc2.m_deviceModelDesc.GetTag();
          if (pTextDesc2) {
            pTextDesc2->SetText("Device Model Desc2");
          }
        }

        pTag->m_Descriptions->push_back(desc);
        pTag->m_Descriptions->push_back(desc2);

        if(!pIcc->AttachTag(sig, pTag))
          return false;

        break;
      }

    case icSigResponseCurveSet16Type:
      {
        CIccTagResponseCurveSet16 *pTag = (CIccTagResponseCurveSet16*)CIccTag::Create(icSigResponseCurveSet16Type);

        icUInt32Number nsize = pIcc->GetSpaceSamples();

        pTag->SetNumChannels((icUInt16Number)nsize);

        CIccResponseCurveStruct* responseCurve = pTag->NewResponseCurves(icSigStatusT);

        if(!responseCurve)
          return false;

        icXYZNumber* XYZ = responseCurve->GetXYZ(0);
        icResponse16Number number;
        number.deviceCode = 255;
        number.reserved = 0;
        number.measurementValue = icDtoF(2.0);

        for(icUInt16Number i=0; i<nsize; i++) {
          XYZ[i].X = icDtoUSF(0.9642f); XYZ[i].Y = icDtoUSF(1.0f); XYZ[i].Z = icDtoUSF(0.8249f);
          responseCurve->GetResponseList(i)->push_back(number);
        }

        if(!pIcc->AttachTag(sig,pTag))
          return false;

        break;
      }

    case icSigViewingConditionsType:
      {
        CIccTagViewingConditions *pTag = (CIccTagViewingConditions*)CIccTag::Create(icSigViewingConditionsType);

        pTag->m_XYZIllum.X = icDtoUSF(0.9642f);
        pTag->m_XYZIllum.Y = icDtoUSF(1.0f);
        pTag->m_XYZIllum.Z = icDtoUSF(0.8249f);

        pTag->m_XYZSurround.X = icDtoUSF(0.5f);
        pTag->m_XYZSurround.Y = icDtoUSF(0.5f);
        pTag->m_XYZSurround.Z = icDtoUSF(0.5f);

        pTag->m_illumType = icIlluminantD50;


        if(!pIcc->AttachTag(sig,pTag))
          return false;

        break;
      }

    default:
      printf("Tag of signature %s not supported currently.\n", icGetSig(buf, sig));
      return false;

  }
  
  
  return true;
}
