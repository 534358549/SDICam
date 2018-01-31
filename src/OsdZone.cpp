#include <stdio.h>
#include <malloc.h>
#include <string.h>

#include "OsdZone.h"
#include "SDL/SDL.h"
#include "SDL/SDL_ttf.h"
#include "mpi_region.h"
#include "sample_region.h"
#include "loadbmp.h"

#define PREFIX_CONF		""
extern char g_strBinPath[];

bool OsdZone::m_initTTF = false;
//BITMAP_S OsdZone::m_bitmapBlank = {0, 0, 0, NULL};
/*
OsdZone::OsdZone()
{
}
*/
OsdZone::OsdZone(int vpssId, int handle, const char *name, int layer, int x, int y, int w, int h)
{
	sprintf(m_configFileName, "%s/%s%s.conf", g_strBinPath, PREFIX_CONF, name);
	loadLocation(x, y, &m_posX, &m_posY);
	printf("location of %s is: %d, %d\n", m_configFileName, m_posX, m_posY);
	
	m_handle = handle;
//	m_inputId = inputId;
	m_vpssId = vpssId;
	m_layer = layer;
	m_height = h;
	m_width = w;

	m_fgAlpha = 128;
	m_bgAlpha = 0;
	
	strcpy(m_name, name);

	m_imgWidth = 0;
	m_imgHeight = 0;

	memset(&m_bitmapBase, 0, sizeof(BITMAP_S));
	m_sizeBitmapBase = 0;

	memset(&m_bitmapWork, 0, sizeof(BITMAP_S));
	m_sizeBitmapWork = 0;
	
	memset(&m_bitmapBlank, 0, sizeof(BITMAP_S));

	genBlankBmp();

}

OsdZone::~OsdZone()
{
	destroyOsdZone();
	
	if(m_bitmapBase.pData)
		free(m_bitmapBase.pData);
	if(m_bitmapWork.pData)
		free(m_bitmapWork.pData);
	if(m_bitmapBlank.pData)
		free(m_bitmapBlank.pData);
}

void OsdZone::loadLocation(int defaultX, int defaultY, int *x, int *y)
{
	*x = defaultX;
	*y = defaultY;

	FILE *filePtr = fopen(m_configFileName, "r");
	if(filePtr == NULL)
		return;

	stPosition pos;
	if(8 == fread((char *)&pos, 1, sizeof(stPosition), filePtr))
	{
		*x = pos.x;
		*y = pos.y;
	}

	fclose(filePtr);
}

void OsdZone::saveLocation()
{
	FILE *conf_file;
	stPosition pos;
	pos.x = m_posX;
	pos.y = m_posY;

	conf_file = fopen(m_configFileName, "w");
	fwrite((char *)&pos, 1, sizeof(stPosition), conf_file);
	fclose(conf_file);
}

bool OsdZone::makeBaseBitmap()
{
	return true;
}

int OsdZone::initStatic()
{
	if(m_initTTF)
		return 0;

	if(TTF_Init() == 0)
	{
		m_initTTF = true;
		return 0;
	}
	return -1;
}

void OsdZone::destroyStatic()
{
	if(!m_initTTF)
		return;

	TTF_Quit();
	m_initTTF = false;
}

void OsdZone::freeBitmap(BITMAP_S *pb)
{
	if(pb->pData)
		free(pb->pData);
	free(pb);
}

int OsdZone::initOsdZone()
{
	HI_S32 s32Ret = HI_SUCCESS;
	
	m_stRgnAttr.enType = OVERLAY_RGN;
	m_stRgnAttr.unAttr.stOverlay.enPixelFmt = PIXEL_FORMAT_RGB_1555;
	m_stRgnAttr.unAttr.stOverlay.stSize.u32Height = m_height;
	m_stRgnAttr.unAttr.stOverlay.stSize.u32Width  = m_width;
	m_stRgnAttr.unAttr.stOverlay.u32BgColor = 0x00007fff; // 全?明?0xffff全白

	printf("HI_MPI_RGN_Create %d\n", m_handle);
	s32Ret = HI_MPI_RGN_Create(m_handle, &m_stRgnAttr);
	if(s32Ret != HI_SUCCESS)
	{
		printf("HI_MPI_RGN_Create failed: %#x\n", s32Ret);
		return s32Ret;
	}

	m_stChn.enModId  = HI_ID_VPSS;
	m_stChn.s32DevId = m_vpssId;  // 注意与VI的SPSS channel号对?
	m_stChn.s32ChnId = 0;

	m_stChnAttr.bShow  = HI_TRUE;
	m_stChnAttr.enType = OVERLAY_RGN;
	m_stChnAttr.unChnAttr.stOverlayChn.stPoint.s32X = m_posX;
	m_stChnAttr.unChnAttr.stOverlayChn.stPoint.s32Y = m_posY;
	m_stChnAttr.unChnAttr.stOverlayChn.u32BgAlpha   = m_bgAlpha; //后景alpha位为0的?明度，?0?128】?越小越透明
	m_stChnAttr.unChnAttr.stOverlayChn.u32FgAlpha   = m_fgAlpha; //前景alpha位为1的?明度，?0?128】?越小越透明
	m_stChnAttr.unChnAttr.stOverlayChn.u32Layer	    = m_layer;
/*	
	m_stChnAttr.unChnAttr.stOverlayChn.stInvertColor.stInvColArea.u32Height = 16;
	m_stChnAttr.unChnAttr.stOverlayChn.stInvertColor.stInvColArea.u32Width  = 16;
	m_stChnAttr.unChnAttr.stOverlayChn.stInvertColor.u32LumThresh 		  = 60; //亮度?值?0?255?
	m_stChnAttr.unChnAttr.stOverlayChn.stInvertColor.enChgMod 			  = LESSTHAN_LUM_THRESH; //osd反色触发模式
	m_stChnAttr.unChnAttr.stOverlayChn.stInvertColor.bInvColEn			  = HI_TRUE; //OSD反色庿僿
*/
	s32Ret = HI_MPI_RGN_AttachToChn(m_handle, &m_stChn, &m_stChnAttr);
	if(s32Ret != HI_SUCCESS)
	{
		printf("HI_MPI_RGN_AttachToChn failed: %#x\n", s32Ret);
		return s32Ret;
	}

	//////////////////////////
	s32Ret = HI_MPI_RGN_GetAttr(m_handle, &m_stRgnAttr); 
	if(s32Ret != HI_SUCCESS) 
	{ 
		printf("HI_MPI_RGN_GetAttr error! %#x\n", s32Ret);
		return s32Ret;
	}

	return s32Ret;
}

void OsdZone::destroyOsdZone()
{
	HI_MPI_RGN_Destroy(m_handle);
}

void OsdZone::setTransperent(int fgLevel, int bgLevel)
{
	m_fgAlpha = fgLevel;
	m_bgAlpha = bgLevel;
}

void OsdZone::show()
{
	// 如果工作位图有数据就显示工作位图，否则显示底图
	if(m_bitmapWork.pData)
		showWorkBMP();
	else
		showBaseBMP();
}

void OsdZone::showBaseBMP()
{
	HI_MPI_RGN_SetBitMap(m_handle, &m_bitmapBase);
}

void OsdZone::showWorkBMP()
{
	HI_MPI_RGN_SetBitMap(m_handle, &m_bitmapWork);
}

void OsdZone::hide()
{
	printf("blank image: %d x %d\n", m_bitmapBlank.u32Width, m_bitmapBlank.u32Height);
	HI_S32 ret = HI_MPI_RGN_SetBitMap(m_handle, &m_bitmapBlank);
	printf("hide returns %#x\n", ret);
}

bool OsdZone::genBlankBmp()
{
/*
	if(m_bitmapBlank.pData)
		return true;
	
	HI_U32 s32Ret = HI_SUCCESS;

	s32Ret = SAMPLE_RGN_LoadBmp("blank.bmp", &m_bitmapBlank, HI_TRUE, 0xffff);
	if (HI_SUCCESS != s32Ret)
	{
//		SAMPLE_RGN_NOT_PASS(s32Ret);
		return false;
	}
	return true;
*/
	m_bitmapBlank.u32Height = 600;
	m_bitmapBlank.u32Width  = 1024;
	m_bitmapBlank.enPixelFormat = PIXEL_FORMAT_RGB_1555;
	m_bitmapBlank.pData = (HI_VOID *)malloc(m_bitmapBlank.u32Height * m_bitmapBlank.u32Width * 2);
	unsigned short *ptr = (unsigned short *)m_bitmapBlank.pData;
	int i;
	for(i = 0; i < m_bitmapBlank.u32Height * m_bitmapBlank.u32Width; i++)
	{
		*ptr = 0x7FFF;
		ptr++;
	}
	return true;
}

bool OsdZone::setBaseBitmap(const char *filename)
{
	HI_U32 s32Ret = HI_SUCCESS;

	if(m_bitmapBase.pData)
	{
		free(m_bitmapBase.pData);
		m_bitmapBase.pData = NULL;
	}
	
	s32Ret = SAMPLE_RGN_LoadBmp(filename, &m_bitmapBase, HI_TRUE, 0xffff);
	if (HI_SUCCESS != s32Ret)
	{
//		SAMPLE_RGN_NOT_PASS(s32Ret);
		return false;
	}

	m_sizeBitmapBase = m_bitmapBase.u32Height * m_bitmapBase.u32Width * 2;
	
	m_imgWidth  = m_bitmapBase.u32Width;
	m_imgHeight = m_bitmapBase.u32Height;

	return true;
}

bool OsdZone::setBaseBitmap(BITMAP_S *bitmap)
{
//	SAMPLE_RGN_SetTransperent(bitmap, 0xffff);

	if(HI_MPI_RGN_SetBitMap(m_handle, bitmap) == HI_SUCCESS)
		return true;
	return false;
}

void OsdZone::resetWorkBitmap()
{
	// ARGB1555每像素占2个字节
	int bytesPerPixel = 2;
	if(m_sizeBitmapWork < m_sizeBitmapBase)
	{
		m_sizeBitmapWork = m_sizeBitmapBase;
		printf("temp bitmap size: %d\n", m_sizeBitmapWork);
		m_bitmapWork.pData = (HI_VOID *)realloc(m_bitmapWork.pData, m_sizeBitmapBase);
	}
	m_bitmapWork.enPixelFormat = m_bitmapBase.enPixelFormat;
	m_bitmapWork.u32Height     = m_bitmapBase.u32Height;
	m_bitmapWork.u32Width      = m_bitmapBase.u32Width;
	memcpy(m_bitmapWork.pData, m_bitmapBase.pData, m_sizeBitmapBase);
}

BITMAP_S *OsdZone::genBmpFromString(const char *str, int size)
{
	TTF_Font *font;
	SDL_Surface *text, *temp;

	strcpy(m_string, str);
	char fontPath[256];
	sprintf(fontPath, "%s/font.ttf", g_strBinPath);

	font = TTF_OpenFont(fontPath, size);
	if ( font == NULL )
	{
		fprintf(stderr, "Couldn't load %d pt font from %s\n", size, SDL_GetError());
		return NULL;
	}

	//TTF_SetFontStyle(font, 1);
	//TTF_SetFontOutline(font, 1);
	//	TTF_SetFontKerning(font, 1);
	//	TTF_SetFontHinting(font, 0);

	SDL_Color forecol = { 0x00, 0x00, 0x00, 0x00 };
//	SDL_Color bgcol =	{ 0xff, 0x00, 0x00, 0x00 };

	text = TTF_RenderUTF8_Solid(font, str, forecol);
	//text = TTF_RenderUTF8_Shaded(font, str, forecol, bgcol);

//	SDL_SaveBMP(text, "1.bmp");

//	printf("text's height is: %d, width is: %d\n", text->h, text->w);

	SDL_PixelFormat fmt;
	memset(&fmt, 0, sizeof(SDL_PixelFormat));
	fmt.BitsPerPixel = 16;
	fmt.BytesPerPixel = 2;

	fmt.Rmask = 0x1F << 10;
	fmt.Gmask = 0x1F << 5;
	fmt.Bmask = 0x1F << 0;
	fmt.Amask = 0x01 << 15;
//	fmt->colorkey = 0xffffffff;
//	fmt->alpha = 0xff;

	temp = SDL_ConvertSurface(text, &fmt, 0);
//	SDL_SaveBMP(temp, "1_.bmp"); 

	BITMAP_S *pb = (BITMAP_S *)malloc(sizeof(BITMAP_S));

	pb->enPixelFormat = PIXEL_FORMAT_RGB_1555;
	pb->u32Width = temp->pitch / 2;			   //do not use temp->w 
	pb->u32Height = temp->h;
	
	int bmpSize = pb->u32Width * pb->u32Height * 2;
	pb->pData = (HI_VOID *)malloc(bmpSize);
	memcpy(pb->pData, temp->pixels, bmpSize);

	SDL_FreeSurface(text);
	SDL_FreeSurface(temp);
	TTF_CloseFont(font);

	return pb;
}

int OsdZone::showString(char *str, int size)
{
	TTF_Font *font;
	SDL_Surface *text, *temp;

	strcpy(m_string, str);

	char fontPath[256];
	sprintf(fontPath, "%s/font.ttf", g_strBinPath);

	font = TTF_OpenFont(fontPath, size);
	if ( font == NULL )
	{
		fprintf(stderr, "Couldn't load %d pt font from %s\n", size, SDL_GetError());
		return -2;
	}

	//TTF_SetFontStyle(font, 1);
	//TTF_SetFontOutline(font, 1);
	//	TTF_SetFontKerning(font, 1);
	//	TTF_SetFontHinting(font, 0);

	SDL_Color forecol =	{ 0x00, 0x00, 0x00, 0x00 };
//	SDL_Color bgcol =	{ 0xff, 0x00, 0x00, 0x00 };

	text = TTF_RenderUTF8_Solid(font, str, forecol);
	//text = TTF_RenderUTF8_Shaded(font, str, forecol, bgcol);

//	SDL_SaveBMP(text, "1.bmp");

//	printf("text's height is: %d, width is: %d\n", text->h, text->w);

	SDL_PixelFormat fmt;
	memset(&fmt, 0, sizeof(SDL_PixelFormat));
	fmt.BitsPerPixel = 16;
	fmt.BytesPerPixel = 2;

	fmt.Rmask = 0x1F << 10;
	fmt.Gmask = 0x1F << 5;
	fmt.Bmask = 0x1F << 0;
	fmt.Amask = 0x01 << 15;
//	fmt->colorkey = 0xffffffff;
//	fmt->alpha = 0xff;

	temp = SDL_ConvertSurface(text, &fmt, 0);
//	SDL_SaveBMP(temp, "1_.bmp"); 

	BITMAP_S bitmap;
	bitmap.u32Width = temp->pitch / 2; 			   //do not use temp->w 
	bitmap.u32Height = temp->h;
	bitmap.pData = temp->pixels; 
	bitmap.enPixelFormat = PIXEL_FORMAT_RGB_1555;

	m_imgHeight = bitmap.u32Height;
	m_imgWidth  = bitmap.u32Width;
	setBaseBitmap(&bitmap);

	SDL_FreeSurface(text);
	SDL_FreeSurface(temp);
	TTF_CloseFont(font);

	return 0;
}

void OsdZone::moveTo(int x, int y)
{
	m_posX = x;
	m_posY = y;
	if(m_posX + m_imgWidth > 1920)
		m_posX = 1920 - m_imgWidth;
	if(m_posY + m_imgHeight > 1080)
		m_posY = 1080 - m_imgHeight;
	
//	saveLocation();
	HI_MPI_RGN_GetDisplayAttr(m_handle, &m_stChn, &m_stChnAttr);

	printf("before moving: %d, %d\n", m_stChnAttr.unChnAttr.stOverlayChn.stPoint.s32X, m_stChnAttr.unChnAttr.stOverlayChn.stPoint.s32Y);
	m_stChnAttr.unChnAttr.stOverlayChn.stPoint.s32X = m_posX;
	m_stChnAttr.unChnAttr.stOverlayChn.stPoint.s32Y = m_posY;
	// the position x,y must be even
	HI_S32 s32Ret = HI_MPI_RGN_SetDisplayAttr(m_handle, &m_stChn, &m_stChnAttr);
	if (HI_SUCCESS != s32Ret)
	{
		printf("HI_MPI_RGN_SetDisplayAttr error! %#x\n", s32Ret);
	}
	printf("#########OSD is moved to %d, %d\n", m_posX, m_posY);
	saveLocation();
}

void OsdZone::move(int x, int y)
{
	moveTo(m_posX + x, m_posY + y);
}

void OsdZone::saveString()
{
}

void OsdZone::loadString()
{
}

bool OsdZone::insertImage(BITMAP_S *baseBmp, BITMAP_S *upBmp, int x, int y)
{
	// currently only supports ARGB1555
	if((baseBmp->enPixelFormat != PIXEL_FORMAT_RGB_1555) || (upBmp->enPixelFormat != PIXEL_FORMAT_RGB_1555))
		return false;
	if(baseBmp->u32Width < upBmp->u32Width + x)
		return false;
	if(baseBmp->u32Height < upBmp->u32Height + y)
		return false;

	char *ptrBase = (char *)(baseBmp->pData) + (baseBmp->u32Width * y + x) * 2;
	char *ptrUp = (char *)(upBmp->pData);

	int lineSizeBase = baseBmp->u32Width * 2;
	int lineSizeUp = upBmp->u32Width * 2;
	for(HI_U32 i = 0; i < upBmp->u32Height; i++)
	{
		memcpy(ptrBase, ptrUp, lineSizeUp);
		ptrUp += lineSizeUp;
		ptrBase += lineSizeBase;
	}
	return true;
}

bool OsdZone::insertImageWithFill(BITMAP_S *baseBmp, BITMAP_S *upBmp, int x, int y, int fitW)
{
	// currently only supports ARGB1555
	if((baseBmp->enPixelFormat != PIXEL_FORMAT_RGB_1555) || (upBmp->enPixelFormat != PIXEL_FORMAT_RGB_1555))
		return false;
	if(baseBmp->u32Width < upBmp->u32Width + x)
		return false;
	if(baseBmp->u32Height < upBmp->u32Height + y)
		return false;

	char *ptrBase = (char *)(baseBmp->pData) + (baseBmp->u32Width * y + x) * 2;
	char *ptrUp = (char *)(upBmp->pData);

	int lineSizeBase = baseBmp->u32Width * 2;
	int lineSizeUp = upBmp->u32Width * 2;
	int fillPixel = fitW - upBmp->u32Width;
	for(HI_U32 i = 0; i < upBmp->u32Height; i++)
	{
		memcpy(ptrBase, ptrUp, lineSizeUp);
		if(fillPixel > 0)
		{
			for(int j = 0; j < fillPixel; j++)
			{
				*(((unsigned short *)(&ptrBase[lineSizeUp])) + j)  = 0x7fff;
			}
		}
		ptrUp += lineSizeUp;
		ptrBase += lineSizeBase;
	}
	return true;
}

/*
void OsdZone::showBox()
{
	RGN_ATTR_S stRgnAttr;
	stRgnAttr.enType = COVER_RGN;
	RGN_CHN_ATTR_S stChnAttr;

	s32Ret = HI_MPI_RGN_Create(m_handle + 20, &stRgnAttr);
	if(s32Ret != HI_SUCCESS)
	{
		printf("HI_MPI_RGN_Create fail! s32Ret: 0x%x.\n", s32Ret);
		return s32Ret;
	}

	stChnAttr.bShow  = HI_TRUE;
	stChnAttr.enType = COVER_RGN;
	stChnAttr.unChnAttr.stCoverChn.enCoverType		= AREA_RECT;
	stChnAttr.unChnAttr.stCoverChn.stRect.s32X		= m_posX;
	stChnAttr.unChnAttr.stCoverChn.stRect.s32Y		= m_posY;
	stChnAttr.unChnAttr.stCoverChn.stRect.u32Height = m_height;
	stChnAttr.unChnAttr.stCoverChn.stRect.u32Width	= m_width;
	stChnAttr.unChnAttr.stCoverChn.u32Color 		= 0x000000ff;
	stChnAttr.unChnAttr.stCoverChn.u32Layer 		= m_layer + 1;

	MPP_CHN_S stChn;
	stChn.enModId  = HI_ID_VPSS;
	stChn.s32DevId = m_vpssId;	// 注意与VI的SPSS channel号对?
	stChn.s32ChnId = 0;

	s32Ret = HI_MPI_RGN_AttachToChn(m_handle, &stChn, &stChnAttr);
	if(s32Ret != HI_SUCCESS)
	{
		printf("COVER: HI_MPI_RGN_AttachToChn fail! s32Ret: 0x%x.\n", s32Ret);
		return s32Ret;
	}
}
*/

