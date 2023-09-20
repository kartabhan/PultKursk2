#ifndef RECORDBLACKBOX_H
#define RECORDBLACKBOX_H
#include <memory.h>
#include "debug.h"

#pragma pack(push,1)
struct RecordBlackBox
{

    unsigned char   time[12];
    float           Anlg[12];
    float           SgnAnl[20];
    unsigned char   OutCww1[2];
    unsigned char   OutCww3[2];
    unsigned char   InCww1[4];
    unsigned char   InCww2[4];
    unsigned char   InCww3[4];
    unsigned char   dsAn[20];
    unsigned char   ValUSO;
    unsigned char   ValMK;
//    RecordBlackBox();
//    ~RecordBlackBox();
};
#pragma pack(pop)

#endif // RECORDBLACKBOX_H
