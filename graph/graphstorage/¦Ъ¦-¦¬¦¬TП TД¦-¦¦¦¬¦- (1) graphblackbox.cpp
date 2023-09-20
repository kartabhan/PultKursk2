#include "graph/graphstorage/graphblackbox.h"
#include <memory.h>
GraphBlackBox::GraphBlackBox()
{

   oneRec = new StructGraphBlackBox;
    memset(oneRec->time,0,12);
    memset(oneRec->Anlg,0,11*4);
    memset(oneRec->SgnAnl,0,20*4);
    memset(oneRec->OutCww1,0,2);
    memset(oneRec->OutCww3,0,2);
    memset(oneRec->InCww1,0,4);
    memset(oneRec->InCww2,0,4);
    memset(oneRec->InCww3,0,4);
    memset(oneRec->dsAn,0,20);
    oneRec->ValUSO=0;
    oneRec->ValMK=0;
    /*
    unsigned char   time[12];
    float           Anlg[11];
    float           SgnAnl[20];
    unsigned char   OutCww1[2];
    unsigned char   OutCww3[2];
    unsigned char   InCww1[4];
    unsigned char   InCww2[4];
    unsigned char   InCww3[4];
    unsigned char   dsAn[20];
    unsigned char   ValUSO;
    unsigned char   ValMK;
     */

}

GraphBlackBox::~GraphBlackBox()
{


}

