#ifndef __OSDDIRCIRCLE_H__
#define __OSDDIRCIRCLE_H__

#include "OsdZone.h"

class OsdDirCircle : public OsdZone
{
public:
	OsdDirCircle(int vpssId, int handle);
	~OsdDirCircle();
	void setDegree(int degree);
};

#endif