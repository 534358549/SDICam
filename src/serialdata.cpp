#include "serialdata.h"
#include "ctrlcmd.h"

#include <malloc.h>
#include <memory.h>
#include <string.h>

#define MAX_DATA_SIZE   300

SerialData::SerialData()
{
    m_data = (unsigned char *)malloc(MAX_DATA_SIZE);
    m_size = 0;
    m_offset = 0;
}

SerialData::~SerialData()
{
    free(m_data);
}

void SerialData::reset()
{
    m_offset = 0;
    m_size = 0;
}

void SerialData::putData(const unsigned char *data, int size)
{
    if(m_offset + m_size + size >= MAX_DATA_SIZE)
    {
        reset();
    }
    memcpy(&m_data[m_offset + m_size], data, size);
    m_size += size;
}

int SerialData::popPktData(unsigned char *data)
{
    int size = sizeof(CtrlCmd);
    memcpy(data, m_data, size);
    memmove(m_data, &m_data[size], size);
    m_size -= size;
    return size;
}

bool SerialData::haveAvaiablePkt()
{
    int head = -1;
    bool bFindTail = false;
    int i;

    // seeking head
    for(i = m_offset; i < m_size + m_offset; i++)
    {
        if(m_data[i] == CMD_HEAD)
        {
            head = i;
            if(m_offset + m_size - head >= sizeof(CtrlCmd))
            {
                bFindTail = true;
            }
            break;
        }
    }
    // drop all bytes before the head
    if(head >= 0)
    {
        memmove(m_data, &m_data[head], m_size - head);
        m_offset = 0;
        m_size -= head;
        if(bFindTail && isPktOK(m_data))
            return true;
    }
    else if(head < 0)
    {
        reset();
        return false;
    }
    return false;
}

bool SerialData::isPktOK(unsigned char *pkt)
{
    CtrlCmd *res = (CtrlCmd *)pkt;
    return res->checkSum();
}

