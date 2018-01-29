#ifndef SERIALDATA_H
#define SERIALDATA_H

class SerialData
{
public:
    SerialData();
    ~SerialData();
    void reset();
    void putData(const unsigned char *data, int size);
    bool haveAvaiablePkt();
    int popPktData(unsigned char *data);
    bool isPktOK(unsigned char *pkt);

private:
    unsigned char *m_data;
    int m_offset;
    int m_size;
};

#endif // SERIALDATA_H
