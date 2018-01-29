#ifndef __OSDGUNSTATEBOX_H__
#define __OSDGUNSTATEBOX_H__

#include "OsdStateBox.h"

class OsdGunStateBox : public OsdStateBox
{
public:
	OsdGunStateBox(int vpssId, int handle);
	virtual ~OsdGunStateBox();
	void setState(unsigned char kaishuan, unsigned char bullet, unsigned char zero, unsigned char trigger);
	void setGunAdjustState(unsigned char state);
	void setHandState(unsigned char state);
	bool canAdjustGun();
	
private:
	bool m_canAdjustGun;
};


#endif

