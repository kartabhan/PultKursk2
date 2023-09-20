#ifndef DEFINES_H
#define DEFINES_H


#define MAX_REC_IN_FILE	25000



#pragma pack(push,1)
struct StructGraphBlackBox{
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

};
#pragma pack(pop)
#endif // DEFINES
