#include <stdio.h>
#include "OsdDirCircle.h"

OsdDirCircle::OsdDirCircle(int vpssId, int handle)
	: OsdZone(vpssId, handle, "dircircle", 3, 46, 870, 180, 180)
{
	setBaseBitmap("circle/0.bmp");
	initOsdZone();
	resetWorkBitmap();
}

OsdDirCircle::~OsdDirCircle()
{
}

void OsdDirCircle::setDegree(int degree)
{
	char filename[32];
	if(degree > 170)
		degree = 170;
	if(degree < -170)
		degree = -170;
	
	sprintf(filename, "circle/%d.bmp", degree);
	setBaseBitmap(filename);
	showBaseBMP();
}

