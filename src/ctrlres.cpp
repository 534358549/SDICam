#include "ctrlres.h"

CtrlRes::CtrlRes()
{
    clear();
}

void CtrlRes::clear()
{
    head = 0;
    info = 0;
    sum  = 0;
}

void CtrlRes::pack()
{
    head = RES_HEAD;
    sum = calcSum();
}

unsigned char CtrlRes::calcSum()
{
    return (info);
}

bool CtrlRes::checkSum()
{
    return (calcSum() == sum);
}
