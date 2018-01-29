#ifndef CTRLCMD_H
#define CTRLCMD_H

#define CMD_HEAD     0xaa
#define DIR_BASE    32768

#pragma pack(1)

struct CtrlCmd
{
    unsigned char head;
    unsigned char hSpeedH;
    unsigned char hSpeedL;
    unsigned char hAngleH;
    unsigned char hAngleL;
    unsigned char vSpeedH;
    unsigned char vSpeedL;
    unsigned char vAngleH;
    unsigned char vAngleL;
    unsigned char kaishuan;
    unsigned char restBullet;
    unsigned char gunZero;
    unsigned char trigger;
    unsigned char keyboard;
    unsigned char sum;

    CtrlCmd();
    unsigned char calcSum();
    bool checkSum();

    void clear();
    void pack();

    short getVSpeed();
    short getVAngle();
    short getHSpeed();
    short getHAngle();

    void setVSpeed(unsigned short value);
    void setVAngle(unsigned short value);
    void setHSpeed(unsigned short value);
    void setHAngle(unsigned short value);

};

#pragma pack()

#endif // CTRLCMD_H
