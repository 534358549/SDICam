#include <stdio.h>
#include <malloc.h>
#include "OsdGunStateBox.h"

#include "defines.h"

#define IDX_GUN_ADJ		0
#define IDX_GUN_ZERO	1
#define IDX_GUN_TRIGGER	2
#define IDX_GUN_HAND	3
#define IDX_GUN_BULLET	4

OsdGunStateBox::OsdGunStateBox(int vpssId, int handle)
	: OsdStateBox(vpssId, handle, "gunstate", 2, 1500, 750, 340, 408)
{
//	setTransperent(128, 64);
//	setTextColor();
	setPos(0, 180, 20, 140);
	setPos(1, 180, 80, 140);
	setPos(2, 180, 137, 140);
	setPos(3, 180, 195, 140);
	setPos(4, 180, 250, 140);

	setBaseBitmap("rightdown.bmp");
	initOsdZone();
	resetWorkBitmap();

	m_canAdjustGun = false;
}

OsdGunStateBox::~OsdGunStateBox()
{
}

void OsdGunStateBox::setState(unsigned char kaishuan, unsigned char bullet, unsigned char zero, unsigned char trigger)
{
	char str[128];
	
	sprintf(str, "%d", bullet);
	setValue(IDX_STATEBOX_REST_BULLET, str);
	
	if(zero == 1)
		setValue(IDX_STATEBOX_ZERO_STATE, UTF8_ZHENGZAIGUILING);
	else if(zero == 2)
		setValue(IDX_STATEBOX_ZERO_STATE, UTF8_YIGUILING);
	else
		setValue(IDX_STATEBOX_ZERO_STATE, UTF8_WUGUILINGMINGLING);

	if(trigger == 1)
		setValue(IDX_STATEBOX_TRIGGER_STATE, UTF8_LIANSHE);
	else
		setValue(IDX_STATEBOX_TRIGGER_STATE, UTF8_DIANSHE);
}

void OsdGunStateBox::setGunAdjustState(unsigned char state)
{
	if(state == 0x01)
	{
		setValue(IDX_STATEBOX_GUN_STATE, UTF8_KAI);
		m_canAdjustGun = true;
	}
	else if(state == 0x02)
	{
		setValue(IDX_STATEBOX_GUN_STATE, UTF8_GUAN);
		m_canAdjustGun = false;
	}
}

void OsdGunStateBox::setHandState(unsigned char state)
{
	if(state == 0x03)
		setValue(IDX_STATEBOX_HAND_STATE, UTF8_KAI);
	else if(state == 0x04)
		setValue(IDX_STATEBOX_HAND_STATE, UTF8_GUAN);
}

bool OsdGunStateBox::canAdjustGun()
{
	return m_canAdjustGun;
}

