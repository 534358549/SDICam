#ifndef __OSDMESSAGEBOX_H__
#define __OSDMESSAGEBOX_H__

#include "OsdZone.h"

class OsdMessageBox : public OsdZone
{
public:
	OsdMessageBox(int vpssId, int handle);
	virtual ~OsdMessageBox();

	void showMessageBox(char *string);
	void hideMessageBox();

	bool makeBaseBitmap();

	void onOK();
	void onCancel();

private:
};

#endif
