#include <stdio.h>
#include <iostream>

#include "IccProfile.h"
#include "IccTag.h"
#include "IccUtil.h"
#include "IccDefs.h"
#include "WriteFunctions.h"

//----------------------------------------------------
// Function Declarations
//----------------------------------------------------

void ShowMenu();
std::string GetUserInput();
bool ConfirmExit();
void ShowHeader(CIccProfile *pIcc);
void ReadTag(CIccProfile *pIcc);
bool ShowTag(CIccProfile *pIcc, icTagSignature sig);
void SaveProfile(CIccProfile *pIcc);
void IsValidProfile(CIccIO *pIO, CIccProfile *pIcc);
void AddTag(CIccIO *pIO, CIccProfile *pIcc);
void DeleteTag(CIccIO *pIO, CIccProfile *pIcc);
void SaveProfCopy(CIccProfile *pIcc);


//----------------------------------------------------
// Function Definitions
//----------------------------------------------------


//===================================================

void ShowMenu()
{
  printf("\n=======================================\n");
  printf("0. Show Menu\n");
  printf("1. Show Profile Header and Tag Table\n");
  printf("2. Show tag contents\n");
  printf("3. Add Tag\n");
  printf("4. Save Profile and Exit\n");
  printf("5. Validate Profile\n");
  printf("6. Delete Tag\n");
  printf("7. Save a copy of the profile.\n");
  printf("9. Exit\n");
  printf("=======================================\n");
  printf("\nMenu Option [0-9]: ");
}

//===================================================

std::string GetUserInput()
{
  std::string Option;
	char str[256];
	std::cin.getline(str, 255);
	Option = str;
  return Option;
}

//===================================================

bool ConfirmExit()
{
  std::string Exit;
  printf("\nExit? (y/n): ");
  Exit = GetUserInput();

  return (("Y" == Exit) || ("y" == Exit));  
}

//===================================================

void ShowHeader(CIccProfile *pIcc)
{
  icHeader *pHdr = &pIcc->m_Header;
  CIccInfo Fmt;
  icChar buf[64];

  printf("\nHeader\n");
  printf(  "------\n");
  printf("Profile Size:     %d(0x%x) bytes\n", pHdr->size, pHdr->size);
  
  if(Fmt.IsProfileIDCalculated(&pHdr->profileID))
    printf("Profile ID:        %s\n", Fmt.GetProfileID(&pHdr->profileID));
  else
    printf("Profile ID:       Profile ID not calculated.\n");
  printf("Attributes:       %s\n", Fmt.GetDeviceAttrName(pHdr->attributes));
  printf("Cmm:              %s\n", Fmt.GetCmmSigName((icCmmSignature)(pHdr->cmmId)));
  printf("Creation Date:    %d/%d/%d  %02u:%02u:%02u\n",
                            pHdr->date.month, pHdr->date.day, pHdr->date.year,
                            pHdr->date.hours, pHdr->date.minutes, pHdr->date.seconds);
  printf("Creator:          %s\n", icGetSig(buf, pHdr->creator));
  printf("Data Color Space: %s\n", Fmt.GetColorSpaceSigName(pHdr->colorSpace));
  printf("Flags             %s\n", Fmt.GetProfileFlagsName(pHdr->flags));
  printf("PCS Color Space:  %s\n", Fmt.GetColorSpaceSigName(pHdr->pcs));
  printf("Platform:         %s\n", Fmt.GetPlatformSigName(pHdr->platform));
  printf("Rendering Intent: %s\n", Fmt.GetRenderingIntentName((icRenderingIntent)(pHdr->renderingIntent)));
  printf("Type:             %s\n", Fmt.GetProfileClassSigName(pHdr->deviceClass));
  printf("Version:          %s\n", Fmt.GetVersionName(pHdr->version));
  printf("Illuminant:       X=%.4lf, Y=%.4lf, Z=%.4lf\n",
                          icFtoD(pHdr->illuminant.X),
                          icFtoD(pHdr->illuminant.Y),
                          icFtoD(pHdr->illuminant.Z));

  printf("\nProfile Tags\n");
  printf(  "------------\n");

  printf("%25s    ID    %8s\t%8s\n", "Tag", "Offset", "Size");
  printf("%25s  ------  %8s\t%8s\n", "----", "------", "----");

  TagEntryList::iterator i;

  for (i=pIcc->m_Tags->begin(); i!=pIcc->m_Tags->end(); i++) {
    printf("%25s  %s  %8d\t%8d\n", Fmt.GetTagSigName(i->TagInfo.sig),
                                     icGetSig(buf, i->TagInfo.sig, false), 
                                     i->TagInfo.offset, i->TagInfo.size);
  }

}

//===================================================

void ReadTag(CIccProfile *pIcc)
{
  CIccInfo Fmt;
  icChar buf[64];

  printf("\nProfile Tags\n");
  printf(  "------------\n");

  printf("%25s    ID    %8s\t%8s\n", "Tag", "Offset", "Size");
  printf("%25s  ------  %8s\t%8s\n", "----", "------", "----");

  TagEntryList::iterator i;

  for (i=pIcc->m_Tags->begin(); i!=pIcc->m_Tags->end(); i++) {
    printf("%25s  %s  %8d\t%8d\n", Fmt.GetTagSigName(i->TagInfo.sig),
                                     icGetSig(buf, i->TagInfo.sig, false), 
                                     i->TagInfo.offset, i->TagInfo.size);
  }

  printf("\nNote: The above Offset & Size parameters DO NOT reflect any unsaved changes to profile.\n");

  printf("\nEnter the tag ID : ");
  std::string TagID;
  TagID = GetUserInput();

  if(!ShowTag(pIcc, (icTagSignature)icGetSigVal(TagID.c_str())))
    printf("Tag (%s) not found in profile\n", TagID.c_str());

}

//===================================================

bool ShowTag(CIccProfile *pIcc, icTagSignature sig)
{
  CIccTag *pTag = pIcc->FindTag(sig);
  icChar buf[64];
  CIccInfo Fmt;

  std::string contents;

  if (!pTag)
    return false;

  printf("\n************ Tag Contents **************\n");

  printf("\nContents of %s tag (%s)\n", Fmt.GetTagSigName(sig), icGetSig(buf, sig)); 
  printf("Type:   ");
  if (pTag->IsArrayType()) {
    printf("Array of ");
  }
  printf("%s\n", Fmt.GetTagTypeSigName(pTag->GetType()));
  pTag->Describe(contents);
  fwrite(contents.c_str(), contents.length(), 1, stdout);

  printf("\n****************************************\n");

  return true;
}


//===================================================

void SaveProfile(CIccProfile *pIcc) 
{
  printf("\nSave Profile as [Ex- Filename.icc]: ");

  std::string szFilename;
  szFilename = GetUserInput();

  if(SaveIccProfile(szFilename.c_str(),pIcc))
    printf("\nProfile successfully saved.\n");
  else
    printf("\nError saving Profile.\n");

}

//===================================================

void SaveProfCopy(CIccProfile *pIcc) 
{
  CIccProfile* pCopy = pIcc;
  SaveProfile(pCopy);
}

//===================================================

void IsValidProfile(CIccIO *pIO, CIccProfile *pIcc)
{
  std::string str;
  icValidateStatus stat = pIcc->ReadValidate(pIO, str);
  stat = icMaxStatus(stat, pIcc->Validate(str));

  switch(stat) {
    case icValidateOK:
    default:
      printf("\nVALID PROFILE! See below for details:\n");
      break;
    case icValidateWarning:
      printf("\nVALID PROFILE with warning(s)! See below for details:\n");
      break;
    case icValidateNonCompliant:
      printf("\nNON-COMPLIANT PROFILE! See below for details:\n");
      break;
    case icValidateCriticalError:
      printf("\nERROR - INVALID PROFILE! See below for details:\n");
      break;
  }

  printf("\n%s\n", str.c_str());

}

//===================================================

void AddTag(CIccIO *pIO, CIccProfile *pIcc)
{
  std::string textIn;

  printf("Enter tag signature to be added [ex- cprt] : ");
  textIn = GetUserInput();

	icTagSignature tagSig = (icTagSignature)icGetSigVal(textIn.c_str());

	switch (tagSig) {
		case icSigProfileDescriptionTag:
		case icSigCopyrightTag:
			printf("Enter the text to be saved in the tag : ");
			textIn = GetUserInput();
			if(!AddTextTag(textIn.c_str(), pIcc, tagSig))
				printf("Write operation failed.\n");
			else
				printf("Tag added successfully.\n");  
			break;
		default:
			if(!WriteTag(pIcc, tagSig))
				printf("Write operation failed.\n");
			else
				printf("Tag added successfully.\n");  
	}

}

//===================================================

void DeleteTag(CIccIO *pIO, CIccProfile *pIcc)
{
  std::string tagtype;

  printf("Enter tag signature to be deleted [ex- cprt] : ");
  tagtype = GetUserInput();

  if(!pIcc->DeleteTag((icTagSignature)icGetSigVal(tagtype.c_str())))
    printf("Delete operation failed.\n");
  else
    printf("Tag deleted successfully.\n");  
}

//===================================================

int main(int argc, icChar* argv[])
{
  if (argc<=1) {
    printf("Usage: CmdIccProfLibTest profile\n");
    return -1;
  }

  
  CIccFileIO FileIO;
  if (!FileIO.Open(argv[1], "rb")) {
    printf("Unable to open '%s'\n", argv[1]);
    return -1;
  }

  CIccProfile *pIcc = new CIccProfile;

  if(!pIcc->Read(&FileIO)) {
    printf("Unable to read '%s'\n", argv[1]);
    delete pIcc;
    return -1;
  }

  bool Exit = false;
  std::string Option;

  ShowMenu();

  while(!Exit) {
    Option = GetUserInput();
    if(Option.size() >=2) {
      printf("\nInvalid Option\n");
      return -1;
    }
    
    icChar temp = Option[0];

    switch(temp) {
    case '0':
      ShowMenu();
      break;
    case '1':
      ShowHeader(pIcc);
      ShowMenu();
      break;
    case '2':
      ReadTag(pIcc);
      ShowMenu();
      break;
    case '3':
      AddTag(&FileIO,pIcc);
      ShowMenu();
      break;
    case '4':
      SaveProfile(pIcc);
      return 0;
    case '5':
      IsValidProfile(&FileIO, pIcc);
      ShowMenu();
      break;
    case '6':
      DeleteTag(&FileIO, pIcc);
      ShowMenu();
      break;
    case '7':
      SaveProfCopy(pIcc);
      ShowMenu();
      break;
    case '9':
      Exit = ConfirmExit();
      if(!Exit) ShowMenu();
      break;
    default:
      printf("\nInvalid Option\n");
      ShowMenu();      
    }
  }


  delete pIcc;
  
  return 0;
}
