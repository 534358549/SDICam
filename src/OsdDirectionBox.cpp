#include <stdio.h>
#include "OsdDirectionBox.h"
#include "defines.h"

extern char g_strBinPath[];

OsdDirectionBox::OsdDirectionBox(int vpssId, int handle)
	: OsdStateBox(vpssId, handle, "dirstate", 2, 10, 850, 880, 220)
{
//	setTransperent(128, 64);
//	setTextColor();

	setPos(IDX_DIR_ANG, 390, 126, 92);
	setPos(IDX_DIR_SPEED, 650, 126, 92);
	setPos(IDX_DIR_UP_ANG, 390, 170, 92);
	setPos(IDX_DIR_UP_SPEED, 650, 170, 92);

	char filename[256];
	sprintf(filename, "%s/leftdown.bmp", g_strBinPath);

	setBaseBitmap(filename);
	initOsdZone();
	resetWorkBitmap();

	showString(250, 126, 190, UTF8_DIR_ANGLE);
	showString(510, 126, 190, UTF8_DIR_SPEED);
	
	showString(250, 170, 190, UTF8_UP_ANGLE);
	showString(510, 170, 190, UTF8_UP_ANGLE);
}

OsdDirectionBox::~OsdDirectionBox()
{
}

void OsdDirectionBox::setDirection(short hAngle, short hSpeed, short vAngle, short vSpeed)
{
	char str[256];
	sprintf(str, "%.2f %s", ((float)hAngle) / 10, UTF8_DEGREE);
	setValue(IDX_DIR_ANG, str);

	sprintf(str, "%.1f %s", ((float)hSpeed) / 100, UTF8_DEGREE_PER_SEC);
	setValue(IDX_DIR_SPEED, str);

	sprintf(str, "%.2f %s", ((float)vAngle) / 10, UTF8_DEGREE);
	setValue(IDX_DIR_UP_ANG, str);

	sprintf(str, "%.1f %s", ((float)vSpeed) / 100, UTF8_DEGREE_PER_SEC);
	setValue(IDX_DIR_UP_SPEED, str);
}

