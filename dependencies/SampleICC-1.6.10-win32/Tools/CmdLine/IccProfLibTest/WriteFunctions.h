#include <stdio.h>
#include <iostream>
#include <cstring>

#include "IccProfile.h"
#include "IccTag.h"
#include "IccUtil.h"
#include "IccDefs.h"

//----------------------------------------------------
// Function Declarations
//----------------------------------------------------

bool AddTextTag(const icChar* ptext, CIccProfile *pIcc, icTagSignature sig);
bool WriteTag(CIccProfile *pIcc, icSignature sig);

