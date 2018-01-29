#ifndef __OSDSTATE_H__
#define __OSDSTATE_H__

#include "OsdZone.h"

#define IDX_STATEBOX_GUN_STATE		0
#define IDX_STATEBOX_ZERO_STATE		1
#define IDX_STATEBOX_TRIGGER_STATE	2
#define IDX_STATEBOX_HAND_STATE		3
#define IDX_STATEBOX_REST_BULLET	4

class OsdStateBox : public OsdZone
{
public:
	OsdStateBox(int vpssId, int handle, const char *name, int layer, int x, int y, int w, int h);
	virtual ~OsdStateBox();

	void setValue(int idx, const char *string);
	void setPos(int idx, int x, int y, int width);
	void setLastCmd(int cmd);
	void showString(int x, int y, int w, const char *string);
	
private:
	struct pos
	{
		int x;
		int y;
		int w;	// 预留的宽度，需要保证新画面完全覆盖原画面
	} m_posValue[10];

	int m_maxVarNum;

	int m_lastCmd;
};

#endif

