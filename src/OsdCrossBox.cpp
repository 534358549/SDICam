#include "OsdCrossBox.h"

OsdCrossBox::OsdCrossBox(int vpssId, int handle)
	: OsdZone(vpssId, handle, "gunstate", 2, 1500, 750, 340, 408)
{
}

OsdGunStateBox::~OsdGunStateBox()
{
}

