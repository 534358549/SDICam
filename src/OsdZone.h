#ifndef OSD_ZONE_H
#define OSD_ZONE_H

#include "hi_comm_video.h"
#include "hi_comm_region.h"

struct stPosition
{
	int x;
	int y;
};

class OsdZone
{
public:
//	OsdZone();
	OsdZone(int vpssId, int handle, const char *name, int layer, int x, int y, int w, int h);
	virtual ~OsdZone();

	void loadLocation(int defaultX, int defaultY, int *x, int *y);
	void saveLocation();
	
	int initOsdZone();
	void destroyOsdZone();

	// level: 0-128, 越小越透明
	void setTransperent(int fgLevel, int bgLevel);

	virtual bool makeBaseBitmap();

	void show();
	void showWorkBMP();
	void showBaseBMP();
	void hide();
	
	bool setBaseBitmap(const char *filename);
	bool setBaseBitmap(BITMAP_S *bitmap);
	void resetWorkBitmap();
	bool genBlankBmp();
	
	BITMAP_S *genBmpFromString(const char *str, int size);
	int showString(char *str, int size);

	void moveTo(int x, int y);
	void move(int x, int y);

	void saveString();
	void loadString();

	static int initStatic();
	static void destroyStatic();

	bool insertImage(BITMAP_S *baseBmp, BITMAP_S *upBmp, int x, int y);
	bool insertImageWithFill(BITMAP_S *baseBmp, BITMAP_S *upBmp, int x, int y, int fitW);

	static void freeBitmap(BITMAP_S *pb);

private:
	char m_name[64];
	char m_string[128];
	
	int m_posX;
	int m_posY;
	int m_width;
	int m_height;

	RGN_HANDLE m_handle;
	int m_inputId;
	int m_vpssId;
	int m_layer;

	RGN_CHN_ATTR_S m_stChnAttr;
	MPP_CHN_S m_stChn;
	RGN_ATTR_S m_stRgnAttr;

	int m_imgWidth;
	int m_imgHeight;

	int m_fgAlpha;
	int m_bgAlpha;

	char m_configFileName[128];

	static bool m_initTTF;

protected:
	// 用于存储OSD的底图，OSD区域上的数据发生变化时，覆盖到底图再显示
	BITMAP_S m_bitmapBase;
	int m_sizeBitmapBase;

	// 工作位图
	BITMAP_S m_bitmapWork;
	int m_sizeBitmapWork;

	// 空白位图，用于隐藏OSD
	BITMAP_S m_bitmapBlank;
	
};

#endif


