#ifndef SAMPLE_REGION_H
#define SAMPLE_REGION_H

HI_S32 SAMPLE_RGN_AddCoverOsdAndLineToVpss(HI_VOID);
HI_S32 SAMPLE_RGN_UpdateCanvas(const char *filename, BITMAP_S *pstBitmap, HI_BOOL bFil, HI_U32 u16FilColor, SIZE_S *pstSize, HI_U32 u32Stride, PIXEL_FORMAT_E enPixelFmt);
HI_S32 SAMPLE_RGN_LoadBmp(const char *filename, BITMAP_S *pstBitmap, HI_BOOL bFil, HI_U32 u16FilColor);
void SAMPLE_RGN_SetTransperent(BITMAP_S *pstBitmap, HI_U32 u16FilColor);


int OSD_GetBitmapFromString(int ptSize, char *str, BITMAP_S *bitmap);

#endif
