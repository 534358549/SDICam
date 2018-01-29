#include "ctrlcmd.h"
#include "defines.h"

CtrlCmd::CtrlCmd()
{
    clear();
}

void CtrlCmd::clear()
{
    head = 0;
    hSpeedH = 0;
    hSpeedL = 0;
    hAngleH = 0;
    hAngleL = 0;
    vSpeedH = 0;
    vSpeedL = 0;
    vAngleH = 0;
    vAngleL = 0;
    kaishuan = 0;
    restBullet = 0;
    gunZero = 0;
    trigger = 0;
    keyboard = 0;
    sum = 0;
}

void CtrlCmd::pack()
{
    sum = calcSum();
    head = CMD_HEAD;
}

unsigned char CtrlCmd::calcSum()
{
    int t = hSpeedH + hSpeedL + hAngleH + hSpeedL + vSpeedH + vSpeedL + vAngleH + vAngleL
            + kaishuan + restBullet + gunZero + trigger + keyboard;
    return (t & 0xff);
}

bool CtrlCmd::checkSum()
{
    return (calcSum() == sum);
}

short CtrlCmd::getVSpeed()
{
    short value = (vSpeedH << 8 | vSpeedL);
    return value - DIR_BASE;
}

short CtrlCmd::getVAngle()
{
    short value = (vAngleH << 8 | vAngleL);
    return value - DIR_BASE;
}

short CtrlCmd::getHSpeed()
{
    short value = (hSpeedH << 8 | hSpeedL);
    return value - DIR_BASE;
}

short CtrlCmd::getHAngle()
{
    short value = (hAngleH << 8 | hAngleL);
    return value - DIR_BASE;
}

void CtrlCmd::setVSpeed(unsigned short value)
{
    vSpeedH = value >> 8 & 0xff;
    vSpeedL = value & 0xff;
}

void CtrlCmd::setVAngle(unsigned short value)
{
    vAngleH = value >> 8 & 0xff;
    vAngleL = value & 0xff;
}

void CtrlCmd::setHSpeed(unsigned short value)
{
    hSpeedH = value >> 8 & 0xff;
    hSpeedL = value & 0xff;
}

void CtrlCmd::setHAngle(unsigned short value)
{
    hAngleH = value >> 8 & 0xff;
    hAngleL = value & 0xff;
}


