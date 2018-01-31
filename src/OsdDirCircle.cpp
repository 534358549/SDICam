#include <stdio.h>
#include "OsdDirCircle.h"

extern char g_strBinPath[];

OsdDirCircle::OsdDirCircle(int vpssId, int handle)
	: OsdZone(vpssId, handle, "dircircle", 3, 46, 870, 180, 180)
{
	char filename[256];
	sprintf(filename, "%s/circle/0.bmp", g_strBinPath);
	setBaseBitmap(filename);
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
	
	sprintf(filename, "%s/circle/%d.bmp", g_strBinPath, degree);
	setBaseBitmap(filename);
	showBaseBMP();
}

