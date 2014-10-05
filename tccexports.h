#ifndef _TCCEXPORTS_H
#define _TCCEXPORTS_H

//File for creating the C linkages to the C++ noneuclid.

#ifdef __cplusplus
extern "C" {
#endif

#include "tccengine.h"


//Call this on your new TCC Engine.
void PopulateTCCE(TCCEngine * tce );


//List linked functions here.
#include "noneuclidtcc.h"



#ifdef __cplusplus
};
#endif


#endif

