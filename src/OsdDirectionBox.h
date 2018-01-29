#ifndef __OSDDIRECTIONBOX_H__
#define __OSDDIRECTIONBOX_H__

#include "OsdStateBox.h"

#define IDX_DIR_ANG			0
#define IDX_DIR_SPEED		1
#define IDX_DIR_UP_ANG		2
#define IDX_DIR_UP_SPEED	3

class OsdDirectionBox : public OsdStateBox
{
public:
	OsdDirectionBox(int vpssId, int handle);
	virtual ~OsdDirectionBox();
	void setDirection(short hAngle, short hSpeed, short vAngle, short vSpeed);
};

#endif

