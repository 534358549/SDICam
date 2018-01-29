#ifndef CTRLRES_H
#define CTRLRES_H

#define RES_HEAD    0x5a
#pragma pack(1)

#define INFO_RECEIVE_OK     0
#define INFO_RECEIVE_FAIL   1
#define INFO_SERV_READY     2

struct CtrlRes
{
    unsigned char head;
    unsigned char info;
    unsigned char sum;

    CtrlRes();
    void clear();
    void pack();

    unsigned char calcSum();
    bool checkSum();
};

#endif // CTRLRES_H
