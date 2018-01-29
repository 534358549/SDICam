#include <stdio.h>
#include <malloc.h>
#include "OsdMessageBox.h"

OsdMessageBox::OsdMessageBox(int vpssId, int handle)
	: OsdZone(vpssId, handle, "messagebox", 7, 690, 300, 560, 320)
{
//	setTransperent(128, 64);
//	setTextColor();
	setBaseBitmap("messagebox.bmp");
	initOsdZone();
}

OsdMessageBox::~OsdMessageBox()
{
}

bool OsdMessageBox::makeBaseBitmap()
{
	return true;
}

void OsdMessageBox::showMessageBox(char *string)
{
	BITMAP_S *pb = genBmpFromString(string, 36);
	resetWorkBitmap();
	insertImage(&m_bitmapWork, pb, 40, 110);
	showWorkBMP();
	freeBitmap(pb);
}

// donot use this function
void OsdMessageBox::hideMessageBox()
{
	hide();
}

void OsdMessageBox::onOK()
{
}

void OsdMessageBox::onCancel()
{
}
