#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "htx_3531a.h"
#include "hiir_test.h"
#include "adv7842_def.h"

#include "sample_region.h"
#include "defines.h"

#include "OsdZone.h"
#include "OsdMessageBox.h"
#include "OsdGunStateBox.h"
#include "OsdDirectionBox.h"
#include "OsdDirCircle.h"

extern char g_strBinPath[];

VIDEO_NORM_E gs_enNorm = VIDEO_ENCODING_MODE_AUTO;

#define VPSS_BSTR_CHN     		0
#define VPSS_LSTR_CHN     		1

#define HDMI_SUPPORT

/*
static PAYLOAD_TYPE_E gs_enPayloadType = PT_LPCM;
static HI_BOOL gs_bAiVqe= HI_FALSE;
static HI_BOOL gs_bAioReSample = HI_FALSE;
static HI_BOOL gs_bUserGetMode = HI_FALSE;
static HI_BOOL gs_bAoVolumeCtrl = HI_TRUE;
static AUDIO_RESAMPLE_ATTR_S *gs_pstAiReSmpAttr = NULL;
static AUDIO_RESAMPLE_ATTR_S *gs_pstAoReSmpAttr = NULL;
static HI_U32 g_u32AiCnt = 0;
static HI_U32 g_u32AoCnt = 0;
static HI_U32 g_u32AencCnt = 0;
static HI_U32 g_u32Adec = 0;
static HI_U32 g_u32AiDev = 0;
static HI_U32 g_u32AoDev = 0;
*/

#define MAX_SDI_INPUT		6
struct stSDIInput
{
	HI_BOOL pluged;		// 是否已接�?	
	HI_U32 width;		// 画面宽度
	HI_U32 height;		// 画面高度
	HI_U32 freq;		// 刷新�?	
	HI_CHAR desc[20];	// 描述
};

stSDIInput g_sdiInput[MAX_SDI_INPUT];
int g_currSdiInputId = 0;	// the default input port is #0

int g_fdGV7704;
int g_fdADV7842;

OsdZone *g_osdZoneCross;
OsdGunStateBox *g_osdZoneRightDown;
OsdDirectionBox *g_osdZoneLeftDown;
OsdDirCircle *g_osdZoneDirCircle;


#define SAMPLE_DBG(s32Ret)\
do{\
    printf("s32Ret=%#x,fuc:%s,line:%d\n", s32Ret, __FUNCTION__, __LINE__);\
}while(0)

void initGlobalVar()
{
	int i;
	for(i = 0; i < MAX_SDI_INPUT; i++)
	{
		g_sdiInput[i].pluged = HI_FALSE;
		g_sdiInput[i].width  = 0;
		g_sdiInput[i].height = 0;
		g_sdiInput[i].freq   = 0;
		strcpy(g_sdiInput[i].desc, "no input");
	}
	g_fdGV7704 = -1;
	g_fdADV7842 = -1;
}

void releaseAllResource()
{
	if(g_fdADV7842 > 0)
	{
		close(g_fdADV7842);
		g_fdADV7842 = -1;
	}
	if(g_fdGV7704 >= 0)
	{
		close(g_fdGV7704);
		g_fdGV7704 = -1;
	}
			
}

HI_S32 startVIO(HI_VOID)
{
    SAMPLE_VI_MODE_E enViMode = SAMPLE_VI_MODE_YD_1080P;//SAMPLE_VI_MODE_8_1080P;//SAMPLE_VI_MODE_16_1080P;

    HI_S32 s32VpssGrpCnt = 8;
    PAYLOAD_TYPE_E enPayLoad[3]= {PT_H264, PT_JPEG,PT_H264};
    PIC_SIZE_E enSize[3] = {PIC_HD1080, PIC_HD720,PIC_D1};
    HI_U32 u32Profile = 1; /*0: baseline; 1:MP; 2:HP 3:svc-t */
    VB_CONF_S stVbConf;
    VPSS_GRP VpssGrp;
    VPSS_CHN VpssChn;
    VENC_CHN VencChn;
    SAMPLE_RC_E enRcMode;
	VPSS_CHN_MODE_S stVpssChnMode;

	VO_DEV VoDev;
	VO_LAYER VoLayer;
	VO_CHN VoChn;
	VO_PUB_ATTR_S stVoPubAttr_hd0, stVoPubAttr_hd1, stVoPubAttr_sd;
	VO_VIDEO_LAYER_ATTR_S stLayerAttr;
	SAMPLE_VO_MODE_E enVoMode, enPreVoMode;
	
	HI_U32 u32WndNum;
    
    HI_S32 i;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32BlkSize;
    HI_CHAR ch = '\0';
    SIZE_S stSize;

    /******************************************
     step  1: init variable 
    ******************************************/
    memset(&stVbConf,0,sizeof(VB_CONF_S));

    u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(gs_enNorm,\
                PIC_HD1080, SAMPLE_PIXEL_FORMAT, SAMPLE_SYS_ALIGN_WIDTH,COMPRESS_MODE_SEG);
    stVbConf.u32MaxPoolCnt = 64;
    
    stVbConf.astCommPool[0].u32BlkSize = u32BlkSize;
    //stVbConf.astCommPool[0].u32BlkCnt = u32ViChnCnt * 6;
    stVbConf.astCommPool[0].u32BlkCnt = s32VpssGrpCnt * 12;
    memset(stVbConf.astCommPool[0].acMmzName,0,
        sizeof(stVbConf.astCommPool[0].acMmzName));

    
    u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(gs_enNorm,\
                PIC_CIF, SAMPLE_PIXEL_FORMAT, SAMPLE_SYS_ALIGN_WIDTH,COMPRESS_MODE_SEG);
    
    memset(stVbConf.astCommPool[1].acMmzName,0,
        sizeof(stVbConf.astCommPool[1].acMmzName));
    stVbConf.astCommPool[1].u32BlkSize = u32BlkSize;
    stVbConf.astCommPool[1].u32BlkCnt = s32VpssGrpCnt * 6;
    memset(stVbConf.astCommPool[1].acMmzName,0,
        sizeof(stVbConf.astCommPool[1].acMmzName));

    /******************************************
     step 2: mpp system init. 
    ******************************************/
    s32Ret = SAMPLE_COMM_SYS_Init(&stVbConf);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("system init failed with %d!\n", s32Ret);
    //    goto END_VENC_8_720p_0;
    }

    /******************************************
     step 3: start vi dev & chn to capture
    ******************************************/
    s32Ret = SAMPLE_COMM_VI_Start(enViMode, gs_enNorm);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("start vi failed!\n");
      //  goto END_VENC_8_720p_0;
    }
    
    /******************************************
     step 4: start vpss and vi bind vpss
    ******************************************/
    s32Ret = SAMPLE_COMM_SYS_GetPicSize(gs_enNorm, PIC_HD1080, &stSize);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_SYS_GetPicSize failed!\n");
      //  goto END_VENC_8_720p_0;
    }

	// 注意窿个参数�?3531A的VPSS暿多支抿个chn
    s32Ret = SAMPLE_COMM_VPSS_Start(s32VpssGrpCnt, &stSize, 4, NULL);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("!!!!!!!!!!Start Vpss failed!\n");
      //  goto END_VENC_8_720p_1;
    }
#if 1
	for (i=0; i<s32VpssGrpCnt; i++)
    {
		VpssGrp = i;
		s32Ret = HI_MPI_VPSS_GetChnMode(VpssGrp, VPSS_BSTR_CHN, &stVpssChnMode);
	    if (HI_SUCCESS != s32Ret)
	    {
	        SAMPLE_PRT("get Vpss chn mode failed!\n");
	    //    goto END_VENC_8_720p_2;
	    }
		memset(&stVpssChnMode,0,sizeof(VPSS_CHN_MODE_S));
		stVpssChnMode.enChnMode = VPSS_CHN_MODE_USER;
		stVpssChnMode.u32Width  = stSize.u32Width;
		stVpssChnMode.u32Height = stSize.u32Height;
		stVpssChnMode.stFrameRate.s32DstFrmRate = -1;
		stVpssChnMode.stFrameRate.s32SrcFrmRate = -1;
		stVpssChnMode.enPixelFormat  = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
		stVpssChnMode.enCompressMode = COMPRESS_MODE_NONE;

		//SAMPLE_PRT("chn %d: %d x %d\n", i, stSize.u32Width, stSize.u32Height);
		s32Ret = HI_MPI_VPSS_SetChnMode(VpssGrp, VPSS_BSTR_CHN, &stVpssChnMode);
	    if (HI_SUCCESS != s32Ret)
	    {
	        SAMPLE_PRT("Set Vpss chn mode failed!\n");
	     //   goto END_VENC_8_720p_2;
	    }
		
	}

#endif
    s32Ret = SAMPLE_COMM_VI_BindVpss(enViMode);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Vi bind Vpss failed!\n");
     //   goto END_VENC_8_720p_2;
    }


	printf("start vo HD1.\n");
	VoDev = SAMPLE_VO_DEV_DHD1;
	VoLayer = SAMPLE_VO_LAYER_VHD1;
	u32WndNum = 9;
	enVoMode = VO_MODE_4MUX;

	//SAMPLE_PRT("enVoMode is: %d\n", enVoMode);

	stVoPubAttr_hd1.enIntfSync = VO_OUTPUT_1080P50;
	stVoPubAttr_hd1.enIntfType = VO_INTF_HDMI|VO_INTF_VGA;
	stVoPubAttr_hd1.u32BgColor = 0x00000000;
	s32Ret = SAMPLE_COMM_VO_StartDev(VoDev, &stVoPubAttr_hd1);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start SAMPLE_COMM_VO_StartDev failed!\n");
		//goto END_8_1080P_5;
	}

	memset(&(stLayerAttr), 0 , sizeof(VO_VIDEO_LAYER_ATTR_S));
	s32Ret = SAMPLE_COMM_VO_GetWH(stVoPubAttr_hd1.enIntfSync, \
		&stLayerAttr.stImageSize.u32Width, \
		&stLayerAttr.stImageSize.u32Height, \
		&stLayerAttr.u32DispFrmRt);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start SAMPLE_COMM_VO_GetWH failed!\n");
		//goto END_8_1080P_6;
	}

	stLayerAttr.enPixFormat = SAMPLE_PIXEL_FORMAT;
    stLayerAttr.stDispRect.s32X       = 0;
    stLayerAttr.stDispRect.s32Y       = 0;
    stLayerAttr.stDispRect.u32Width   = stLayerAttr.stImageSize.u32Width;
    stLayerAttr.stDispRect.u32Height  = stLayerAttr.stImageSize.u32Height;
	s32Ret = SAMPLE_COMM_VO_StartLayer(VoLayer, &stLayerAttr);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start SAMPLE_COMM_VO_StartLayer failed!\n");
		//goto END_8_1080P_6;
	}

	// seek the input port which the camera is connected to
	int inputId;
	for(inputId = 0; inputId < MAX_SDI_INPUT; inputId++)
	{
		if(g_sdiInput[inputId].pluged && g_sdiInput[inputId].height == 1080)
		{
			g_currSdiInputId = inputId;
			break;
		}
	}

	s32Ret = SAMPLE_COMM_VO_SetSingleChn(VoLayer, g_currSdiInputId);
//	s32Ret = SAMPLE_COMM_VO_StartChn(VoLayer, enVoMode);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start SAMPLE_COMM_VO_StartChn failed!\n");
		//goto END_8_1080P_7;
	}
#ifdef HDMI_SUPPORT
	/* if it's displayed on HDMI, we should start HDMI */
	if (stVoPubAttr_hd1.enIntfType & VO_INTF_HDMI)
	{
		if (HI_SUCCESS != SAMPLE_COMM_VO_HdmiStart(stVoPubAttr_hd1.enIntfSync))
		{
			SAMPLE_PRT("Start SAMPLE_COMM_VO_HdmiStart failed!\n");
			//goto END_8_1080P_7;
		}
	}
#endif

	for(i=0;i<u32WndNum;i++)
	{
		VoChn = i;
		VpssGrp = i;
		s32Ret = SAMPLE_COMM_VO_BindVpss(VoDev,VoChn,VpssGrp,VPSS_CHN2);
		if (HI_SUCCESS != s32Ret)
		{
			SAMPLE_PRT("Start VO failed!\n");
			//goto END_8_1080P_7;
		}
	}

#if 0
    /******************************************
     step 5: select rc mode
    ******************************************/
    enRcMode = SAMPLE_RC_CBR;
    /******************************************
     step 5: start stream venc (big + little)
    ******************************************/
    for (i=0; i<s32VpssGrpCnt; i++)
    {
        /*** main stream,H264**/
        VencChn = i*2;
        VpssGrp = i;
        s32Ret = SAMPLE_COMM_VENC_Start(VencChn, enPayLoad[0],\
                                       gs_enNorm, enSize[0], enRcMode,u32Profile);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("Start Venc failed!\n");
         //   goto END_VENC_8_720p_2;
        }

        s32Ret = SAMPLE_COMM_VENC_BindVpss(VencChn,VpssGrp, VPSS_BSTR_CHN);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("Start Venc failed!\n");
         //   goto END_VENC_8_720p_3;
        }

       
	   /*** Sub stream **/
        VencChn ++;
        s32Ret = SAMPLE_COMM_VENC_Start(VencChn, enPayLoad[2], \
                                        gs_enNorm, enSize[2], enRcMode,u32Profile);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("Start Venc failed!\n");
        //    goto END_VENC_8_720p_3;
        }

        s32Ret = SAMPLE_COMM_VENC_BindVpss(VencChn, VpssGrp, VPSS_LSTR_CHN);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("Start Venc failed!\n");
        //    goto END_VENC_8_720p_3;
        }
#if 0
		/*** Main stream jpeg**/
        VencChn = 2*(s32VpssGrpCnt + i);
		s32Ret = SAMPLE_COMM_SYS_GetPicSize(gs_enNorm, enSize[1], &stSize);
	    if (HI_SUCCESS != s32Ret)
	    {
	        SAMPLE_PRT("Get picture size failed!\n");
	   //     goto END_VENC_8_720p_3;
	    }
		
        s32Ret = SAMPLE_COMM_VENC_SnapStart(VencChn,&stSize);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("Start Venc failed!\n");
          //  goto END_VENC_8_720p_3;
        }
#endif
    }


    /******************************************
     step 6: stream venc process -- get stream, then save it to file. 
    ******************************************/
    s32Ret = SAMPLE_COMM_VENC_StartGetStream(s32VpssGrpCnt*2);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Start Venc failed!\n");
       // goto END_VENC_8_720p_3;
    }
#endif


/*
	int kkk;
	for(kkk = 0; kkk < 10; kkk++)
	{
		oz1->move(10, 4);
		sleep(1);
	}
*/
#if 0
	printf("print OSD...\n");
//	SAMPLE_RGN_AddCoverOsdAndLineToVpss();
	RGN_HANDLE handle;
	RGN_ATTR_S stRgnAttr;
	handle = 0;
//	stRgnAttr.enType = COVER_RGN;

	stRgnAttr.enType = OVERLAY_RGN;
	stRgnAttr.unAttr.stOverlay.enPixelFmt = PIXEL_FORMAT_RGB_1555;
	stRgnAttr.unAttr.stOverlay.stSize.u32Height = 1000;
	stRgnAttr.unAttr.stOverlay.stSize.u32Width  = 2400;
	stRgnAttr.unAttr.stOverlay.u32BgColor = 0x00007fff;	// 全翏明ﺿxffff全白

	printf("HI_MPI_RGN_Create\n");
	s32Ret = HI_MPI_RGN_Create(handle, &stRgnAttr);
	if(s32Ret != HI_SUCCESS)
	{
		printf("HI_MPI_RGN_Create failed: %#x\n", s32Ret);
		return s32Ret;
	}

	MPP_CHN_S stChn;
	stChn.enModId  = HI_ID_VPSS;
    stChn.s32DevId = inputId;  // 注意与VI的SPSS channel号对�?    stChn.s32ChnId = 0;

	RGN_CHN_ATTR_S stChnAttr;
#if 1
	stChnAttr.bShow  = HI_TRUE;
	stChnAttr.enType = OVERLAY_RGN;
	stChnAttr.unChnAttr.stOverlayChn.stPoint.s32X = 180;
	stChnAttr.unChnAttr.stOverlayChn.stPoint.s32Y = 80;
	stChnAttr.unChnAttr.stOverlayChn.u32BgAlpha   = 0; //后景alpha位为0的翏明度，⾿�?28】㿼越小越�?�明
	stChnAttr.unChnAttr.stOverlayChn.u32FgAlpha   = 128; //前景alpha位为1的翏明度，⾿�?28】㿼越小越�?�明
	stChnAttr.unChnAttr.stOverlayChn.u32Layer     = 1;
	stChnAttr.unChnAttr.stOverlayChn.stInvertColor.stInvColArea.u32Height = 16;
	stChnAttr.unChnAttr.stOverlayChn.stInvertColor.stInvColArea.u32Width  = 16;
	stChnAttr.unChnAttr.stOverlayChn.stInvertColor.u32LumThresh           = 60; //亮度閿�?�⾿�?55�?	stChnAttr.unChnAttr.stOverlayChn.stInvertColor.enChgMod               = LESSTHAN_LUM_THRESH; //osd反色触发模式
	stChnAttr.unChnAttr.stOverlayChn.stInvertColor.bInvColEn              = HI_FALSE; //OSD反色庿偿#else
	stChnAttr.bShow  = HI_TRUE;
    stChnAttr.enType = COVER_RGN;
    stChnAttr.unChnAttr.stCoverChn.enCoverType = AREA_RECT;
    stChnAttr.unChnAttr.stCoverChn.stRect.s32X      = 10;
    stChnAttr.unChnAttr.stCoverChn.stRect.s32Y      = 10;
    stChnAttr.unChnAttr.stCoverChn.stRect.u32Height = 100;
    stChnAttr.unChnAttr.stCoverChn.stRect.u32Width  = 100;
    stChnAttr.unChnAttr.stCoverChn.u32Color         = 0x000000ff;
	stChnAttr.unChnAttr.stCoverChn.u32Layer         = 0;
#endif

	printf("HI_MPI_RGN_AttachToChn\n");
	s32Ret = HI_MPI_RGN_AttachToChn(handle, &stChn, &stChnAttr);
	if(s32Ret != HI_SUCCESS)
	{
		printf("HI_MPI_RGN_AttachToChn failed: %#x\n", s32Ret);
		return s32Ret;
	}

	//////////////////////////
    s32Ret = HI_MPI_RGN_GetAttr(handle, &stRgnAttr); 
    if(s32Ret != HI_SUCCESS) 
    { 
		printf("HI_MPI_RGN_GetAttr error! %#x\n", s32Ret);
		return s32Ret;
    }

	BITMAP_S bitmap;
	OSD_GetBitmapFromString(48, NULL, &bitmap);
	HI_MPI_RGN_SetBitMap(handle, &bitmap);

#if 0	
//    stRgnAttr.unAttr.stOverlay.u32BgColor = 0x0000cccc;
    //获取画布信息
    RGN_CANVAS_INFO_S osdCanvasInfo;
    s32Ret = HI_MPI_RGN_GetCanvasInfo(handle, &osdCanvasInfo);
    if(HI_SUCCESS != s32Ret)
    {
		printf("HI_MPI_RGN_GetCanvasInfo error! %#x\n", s32Ret);
		return s32Ret;
    }
	

    BITMAP_S stBitmap;//位图图像信息结构�?    SIZE_S size;

    stBitmap.pData = (HI_VOID*)osdCanvasInfo.u32VirtAddr;
    size.u32Width  = osdCanvasInfo.stSize.u32Width;
    size.u32Height = osdCanvasInfo.stSize.u32Height;
	printf("osdCanvasInfo: %d x %d\n", size.u32Width, size.u32Height);
    /*更新画布数据*/
    s32Ret = SAMPLE_RGN_UpdateCanvas("mm1.bmp", &stBitmap, HI_TRUE, 0xffff, &size, osdCanvasInfo.u32Stride, stRgnAttr.unAttr.stOverlay.enPixelFmt);
    if(HI_SUCCESS != s32Ret)
    {
	    printf("SAMPLE_RGN_UpdataCanvas error! %#x\n", s32Ret);
	    return s32Ret;
    }

	printf("HI_MPI_RGN_UpdateCanvas\n");
    s32Ret = HI_MPI_RGN_UpdateCanvas(handle);
    if(HI_SUCCESS != s32Ret)
    {
		printf("HI_MPI_RGN_UpdateCanvas error! %#x\n", s32Ret);
		return s32Ret;
    }
#endif
#if 0	// move!
	for(i = 0; i < 100; i++)
	{
		stChnAttr.unChnAttr.stOverlayChn.stPoint.s32X = 80 + i;
		stChnAttr.unChnAttr.stOverlayChn.stPoint.s32Y = 80;
		s32Ret = HI_MPI_RGN_SetDisplayAttr(handle, &stChn, &stChnAttr);
		if (HI_SUCCESS != s32Ret)
        {
            printf("HI_MPI_RGN_SetDisplayAttr error! %#x\n", s32Ret);
        }
		usleep(10000);
	}
#endif

	printf("print OSD done...\n");
#endif



#if 0	
    printf("peress ENTER to capture one picture to file\n");
	while (ch != 'q')
	{
		for (i=0; i<s32VpssGrpCnt; i++)
		{
			/*** main frame **/
			VpssGrp = i;
			VencChn = 2*(s32VpssGrpCnt + i);
			s32Ret = SAMPLE_COMM_VENC_SnapProcess(VencChn, VpssGrp, VPSS_BSTR_CHN);
			if (HI_SUCCESS != s32Ret)
			{
				SAMPLE_PRT("snap process failed!\n");
			}
			printf("snap chn %d ok!\n", i);
		
			//sleep(1);
			
			printf("press 'q' to exit snap process!peress ENTER to capture one picture to file\n");
			if((ch = getchar()) == 'q')
			{
				break;
			}
		}

		
	}
    printf("please press twice ENTER to exit this sample\n");
    getchar();
    getchar();

    /******************************************
     step 7: exit process
    ******************************************/
    SAMPLE_COMM_VENC_StopGetStream();
    //for (i=0; i<s32VpssGrpCnt; i++)
    //   {
    //	VencChn = 2*(s32VpssGrpCnt + i);
    //	s32Ret = SAMPLE_COMM_VENC_SnapStop(VencChn);
    //    if (HI_SUCCESS != s32Ret)
    //    {
    //        SAMPLE_PRT("Stop snap failed!\n");
    //        goto END_VENC_8_720p_3;
    //    }
    //}
    
END_VENC_8_720p_3:
	for (i=0; i<s32VpssGrpCnt; i++)
	{
		VencChn = (i+s32VpssGrpCnt)*2;
		VpssGrp = i;
		VpssChn = VPSS_BSTR_CHN;
		SAMPLE_COMM_VENC_UnBindVpss(VencChn, VpssGrp, VpssChn);
		SAMPLE_COMM_VENC_SnapStop(VencChn);
	}

    for (i=0; i<s32VpssGrpCnt*2; i++)
    {
        VencChn = i;
        VpssGrp = i/2;
        VpssChn = (VpssGrp%2)?VPSS_LSTR_CHN:(VPSS_BSTR_CHN);
        SAMPLE_COMM_VENC_UnBindVpss(VencChn, VpssGrp, VpssChn);
        SAMPLE_COMM_VENC_Stop(VencChn);
    }
	
	SAMPLE_COMM_VI_UnBindVpss(enViMode);
END_VENC_8_720p_2:	
    SAMPLE_COMM_VPSS_Stop(s32VpssGrpCnt, VPSS_MAX_CHN_NUM);
END_VENC_8_720p_1:	
    SAMPLE_COMM_VI_Stop(enViMode);
END_VENC_8_720p_0:	
    SAMPLE_COMM_SYS_Exit();
    #endif
    return s32Ret;
}

void stopVIO()
{
}


void testOSD()
{
	g_osdZoneRightDown->setValue(0, UTF8_KAI);
	g_osdZoneRightDown->setValue(1, UTF8_WUGUILINGMINGLING);
	g_osdZoneRightDown->setValue(1, UTF8_KAI);
	g_osdZoneRightDown->setValue(2, UTF8_DIANSHE);
	g_osdZoneRightDown->setValue(3, UTF8_GUAN);
	g_osdZoneRightDown->setValue(4, "60");
	
	g_osdZoneLeftDown->setValue(0, "100.00");
	g_osdZoneLeftDown->setValue(1, "200.00");
	g_osdZoneLeftDown->setValue(2, "300.00");
	g_osdZoneLeftDown->setValue(3, "400.00");

	int i;
	for(i = 0; i < 30; i++)
	{
		g_osdZoneCross->moveTo(430 + i * 20, 160);
		sleep(1);
	}
	g_osdZoneCross->saveLocation();

	printf("show messagebox\n");
	while(0)
	{
		OsdMessageBox *osdMessageBox;
		osdMessageBox = new OsdMessageBox(g_currSdiInputId, 7);
		osdMessageBox->showMessageBox(UTF8_CONFIRM_JIAOQIANGKAI);
		sleep(5);
		delete osdMessageBox;
		sleep(1);
		osdMessageBox = new OsdMessageBox(g_currSdiInputId, 7);
		osdMessageBox->showMessageBox(UTF8_CONFIRM_JIAOQIANGGUAN);
		sleep(5);
		delete osdMessageBox;
		sleep(1);
		osdMessageBox = new OsdMessageBox(g_currSdiInputId, 7);
		osdMessageBox->showMessageBox(UTF8_CONFIRM_SHOUBINGKAI);
		sleep(5);
		delete osdMessageBox;
		sleep(1);
		osdMessageBox = new OsdMessageBox(g_currSdiInputId, 7);
		osdMessageBox->showMessageBox(UTF8_CONFIRM_SHOUBINGGUAN);
		sleep(5);
		delete osdMessageBox;
		sleep(1);
		osdMessageBox = new OsdMessageBox(g_currSdiInputId, 7);
		osdMessageBox->showMessageBox(UTF8_CONFIRM_DANYAOCHONG);
		sleep(5);
		delete osdMessageBox;
		sleep(1);
		osdMessageBox = new OsdMessageBox(g_currSdiInputId, 7);
		osdMessageBox->showMessageBox(UTF8_CONFIRM_QIANGTALING);
		sleep(5);
		delete osdMessageBox;
		sleep(1);
	} 
}

void stopOSD()
{
	delete g_osdZoneCross;
	delete g_osdZoneLeftDown;
	delete g_osdZoneRightDown;
	delete g_osdZoneDirCircle;

	OsdZone::destroyStatic();
}

void startOSD()
{
	printf("start OSD...\n");
	OsdZone::initStatic();

	printf("create the OsdZone...\n");
	g_osdZoneCross     = new OsdZone(g_currSdiInputId, 0, "cross", 0, 240, 150, 1600, 1200);
	printf("create the OsdDirectionBox...\n");
	g_osdZoneLeftDown  = new OsdDirectionBox(g_currSdiInputId, 1);
	printf("create the OsdGunStateBox...\n");
	g_osdZoneRightDown = new OsdGunStateBox(g_currSdiInputId, 2);
	printf("create the OsdDirCircle...\n");
	g_osdZoneDirCircle = new OsdDirCircle(g_currSdiInputId, 3);

	printf("show cross zone\n");
	g_osdZoneCross->initOsdZone();
	char bmpPath[256];
	sprintf(bmpPath, "%s/cross.bmp", g_strBinPath);
	g_osdZoneCross->setBaseBitmap(bmpPath);
	g_osdZoneCross->show();

	printf("show rightdown zone\n");
	g_osdZoneRightDown->show();
	printf("show leftdown zone\n");
	g_osdZoneLeftDown->show();
	printf("show circle zone\n");
	g_osdZoneDirCircle->show();

	printf("OSD is done\n");
//	testOSD();
}

void * CheckADV7842(void *)
{
	Adv7842_Input_Fmt inputFmt;
	
	g_fdADV7842 = open("/dev/adv7842dev", 0);
	if (g_fdADV7842 < 0)
	{
		printf("Open adv7842 dev error!\n");
		exit(1);
	}
    
	while(1)
	{      
		ioctl(g_fdADV7842, ADV7842_GET_STATE, &inputFmt);        

		printf("Input Format: %d   %d\n", inputFmt.input_src,inputFmt.video_fmt);
		sleep(1);
	}
    
}


#define GPIO_SPI_READ   0x01
#define SDI_RESET		0x04

int input_fbl(int type, int inputId)
{
	switch(type)
	{
	case 0:
		g_sdiInput[inputId].pluged = HI_FALSE;
		strcpy(g_sdiInput[inputId].desc, "no input");
		break;
	case 0x21:
		g_sdiInput[inputId].pluged = HI_TRUE;
		g_sdiInput[inputId].width  = 1920;
		g_sdiInput[inputId].height = 1080;
		g_sdiInput[inputId].freq   = 25;
		strcpy(g_sdiInput[inputId].desc, "1080p25");
		break;
	case 0x1f:
		g_sdiInput[inputId].pluged = HI_TRUE;
		g_sdiInput[inputId].width  = 1920;
		g_sdiInput[inputId].height = 1080;
		g_sdiInput[inputId].freq   = 30;
		strcpy(g_sdiInput[inputId].desc, "1080p30");
		break;
	case 0x17:
		g_sdiInput[inputId].pluged = HI_TRUE;
		g_sdiInput[inputId].width  = 1920;
		g_sdiInput[inputId].height = 1080;
		g_sdiInput[inputId].freq   = 50;
		strcpy(g_sdiInput[inputId].desc, "1080p50");
		break;
	case 0x16:
		g_sdiInput[inputId].pluged = HI_TRUE;
		g_sdiInput[inputId].width  = 1920;
		g_sdiInput[inputId].height = 1080;
		g_sdiInput[inputId].freq   = 60;
		strcpy(g_sdiInput[inputId].desc, "1080p60");
		break;
	case 0xd:
		g_sdiInput[inputId].pluged = HI_TRUE;
		g_sdiInput[inputId].width  = 1280;
		g_sdiInput[inputId].height = 720;
		g_sdiInput[inputId].freq   = 50;
		strcpy(g_sdiInput[inputId].desc, "720p50");
		break;
	case 0xc:
		g_sdiInput[inputId].pluged = HI_TRUE;
		g_sdiInput[inputId].width  = 1280;
		g_sdiInput[inputId].height = 720;
		g_sdiInput[inputId].freq   = 60;
		strcpy(g_sdiInput[inputId].desc, "720p60");
		break;
	default:
		g_sdiInput[inputId].pluged = HI_TRUE;
		g_sdiInput[inputId].width  = 0;
		g_sdiInput[inputId].height = 0;
		g_sdiInput[inputId].freq   = 0;
		strcpy(g_sdiInput[inputId].desc, "unknown");
		break;
	}

	return 0;
}

void checkSDIInput()
{
	unsigned int device_addr, reg_addr, reg_value, value;

	if(g_fdGV7704 < 0)
		g_fdGV7704 = open("/dev/gv7704dev", 0);

	if(g_fdGV7704 < 0)
    {
    	printf("Open gv7704 dev error!\n");
    	return;
    }

	device_addr = 0x0;
	reg_addr = 0x4411;
	value = ((device_addr&0xf)<<28) | ((reg_addr&0xfffff)<<8);
	ioctl(g_fdGV7704, GPIO_SPI_READ, &value);
	reg_value = value&0xff;
	input_fbl(reg_value, 0);
	
	device_addr = 0x0;
	reg_addr = 0x5011;
	value = ((device_addr&0xf)<<28) | ((reg_addr&0xfffff)<<8);
	ioctl(g_fdGV7704, GPIO_SPI_READ, &value);
	reg_value = value&0xff;
	input_fbl(reg_value, 1);
	
	device_addr = 0x0;
	reg_addr = 0x5c11;
	value = ((device_addr&0xf)<<28) | ((reg_addr&0xfffff)<<8);
	ioctl(g_fdGV7704, GPIO_SPI_READ, &value);
	reg_value = value&0xff;
	input_fbl(reg_value, 2);
	
	device_addr = 0x0;
	reg_addr = 0x6811;
	value = ((device_addr&0xf)<<28) | ((reg_addr&0xfffff)<<8);
	ioctl(g_fdGV7704, GPIO_SPI_READ, &value);
	reg_value = value&0xff;
	input_fbl(reg_value, 3);
	
	device_addr = 0x1;
	reg_addr = 0x4411;
	value = ((device_addr&0xf)<<28) | ((reg_addr&0xfffff)<<8);
	ioctl(g_fdGV7704, GPIO_SPI_READ, &value);
	reg_value = value&0xff;
	input_fbl(reg_value, 4);
	
	device_addr = 0x1;
	reg_addr = 0x5011;
	value = ((device_addr&0xf)<<28) | ((reg_addr&0xfffff)<<8);
	ioctl(g_fdGV7704, GPIO_SPI_READ, &value);
	reg_value = value&0xff;
	input_fbl(reg_value, 5);

	printf("gv7704 value, ch1: %s, ch2: %s, ch3: %s, ch4: %s, ch5: %s, ch6: %s\n", 
		g_sdiInput[0].desc, g_sdiInput[1].desc, g_sdiInput[2].desc, g_sdiInput[3].desc, g_sdiInput[4].desc, g_sdiInput[5].desc);

}

void * CheckGV7704(void *)
{
	while(1)
	{
		checkSDIInput();
		sleep(2);
	}
}


HI_S32 HTX_CreatThread(HI_VOID)
{
	pthread_t  hiir_testid, checkadv7842id, checkgv7704id;
	int ret;

/*
	ret = pthread_create (&hiir_testid, NULL,hisi_ir_test, NULL);
	if (ret != 0)
	{
		 perror ("Create hisi_ir_test timer faile!\n");
		 exit (1);
	}
	ret = pthread_create (&checkadv7842id, NULL,CheckADV7842, NULL);
	if (ret != 0)
	{
		 perror ("Create hisi_ir_test timer faile!\n");
		 exit (1);
	}
*/

	ret = pthread_create(&checkgv7704id, NULL, CheckGV7704, NULL);
	if (ret != 0)
	{
		 perror ("Create gv7601 timer faile!\n");
		 exit (1);
	}


	return 0;
}


