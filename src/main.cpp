#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "htx_3531a.h"
#include "serial.h"
#include "sample_region.h"

#include "OsdZone.h"
// hello

int g_keepRunning = 1;
char g_strBinPath[256];

int main(int argc, char **argv)
{
	printf("argv0 is %s\n", argv[0]);
	char *ptr = strrchr(argv[0], '/');
	if(ptr == NULL)
		strcpy(g_strBinPath, ".");
	else
	{
		int len = ptr - argv[0];
		memcpy(g_strBinPath,  argv[0], len);
	}
	printf("bin path is %s\n", g_strBinPath);
	
	printf("Hi3531A_6SDI_HDMI_VGA DEMO Build Time:%s %s \n", __DATE__, __TIME__);
	float a1111 = 100.21;
	float b = 9100.1;
	printf("this is a test of a1111: %10.2f, and b: %10.2f\n", a1111, b);

	initGlobalVar();
	checkSDIInput();
	
	startVIO();
	startOSD();
	
//	HTX_Audio_AiAenc_Start();

//	HTX_CreatThread();
//	Serial_test();

	serialHandle();

#if 0
	while(true)
	{
		usleep(500);
	}
#endif

	releaseAllResource();

	return 0;
}
