#ifndef DEFINES
#define DEFINES

/*
   Public Structure RecordBlackBox
        <VBFixedArray(11)> Dim time() As Byte
        <VBFixedArray(10)> Dim Anlg() As Single
        <VBFixedArray(15)> Dim SgnAnl() As Single
        <VBFixedArray(1)> Dim OutCww1() As Byte
        <VBFixedArray(1)> Dim OutCww3() As Byte
        <VBFixedArray(3)> Dim InCww1() As Byte
        <VBFixedArray(3)> Dim InCww2() As Byte
        <VBFixedArray(3)> Dim InCww3() As Byte
        <VBFixedArray(15)> Dim dsAn() As Byte
        Dim ValUSO As Byte
        Dim ValMK As Byte

        Public Sub Initialize()
            ReDim time(11)
            ReDim Anlg(10)
            ReDim SgnAnl(15)
            ReDim OutCww1(1)
            ReDim OutCww3(1)
            ReDim InCww1(3)
            ReDim InCww2(3)
            ReDim InCww3(3)
            ReDim dsAn(15)
        End Sub
    End Structure
 */
struct StructRecordBlackBox{
    unsigned char   time[12];
    float           Anlg[11];
    float           SgnAnl[16];
    unsigned char   OutCww1[2];
    unsigned char   OutCww3[2];
    unsigned char   InCww1[4];
    unsigned char   InCww2[4];
    unsigned char   InCww3[4];
    unsigned char   dsAn[16];
    unsigned char   ValUSO;
    unsigned char   ValMK;

};
#endif // DEFINES
