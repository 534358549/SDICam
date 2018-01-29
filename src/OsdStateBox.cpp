#include <stdio.h>
#include "OsdStateBox.h"
#include "defines.h"

OsdStateBox::OsdStateBox(int vpssId, int handle, const char *name, int layer, int x, int y, int w, int h)
	: OsdZone(vpssId, handle, name, layer, x, y, w, h)
{
	m_lastCmd = -1;
}

OsdStateBox::~OsdStateBox()
{
}

void OsdStateBox::setValue(int idx, const char *string)
{
	showString(m_posValue[idx].x, m_posValue[idx].y, m_posValue[idx].w, string);
	return;
	
	BITMAP_S *pb = genBmpFromString(string, 28);
	insertImageWithFill(&m_bitmapWork, pb, m_posValue[idx].x, m_posValue[idx].y, m_posValue[idx].w);
	//insertImage(&m_bitmapWork, pb, m_posValue[idx].x, m_posValue[idx].y);
	showWorkBMP();
	freeBitmap(pb);
}

void OsdStateBox::showString(int x, int y, int w, const char *string)
{
	BITMAP_S *pb = genBmpFromString(string, 28);
	insertImageWithFill(&m_bitmapWork, pb, x, y, w);
	//insertImage(&m_bitmapWork, pb, m_posValue[idx].x, m_posValue[idx].y);
	showWorkBMP();
	freeBitmap(pb);
}

void OsdStateBox::setPos(int idx, int x, int y, int width)
{
	m_posValue[idx].x = x;
	m_posValue[idx].y = y;
	m_posValue[idx].w = width;
}

void OsdStateBox::setLastCmd(int cmd)
{
	m_lastCmd = cmd;
}

