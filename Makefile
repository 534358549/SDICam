include Makefile.global

INCLUDES = -Ihi_sdk -Ihi_ir -Ihi_sdk/include -Ihi_sdk/extdrv/tlv320aic31-hisi -Isample -Icomm -Iosd/include

ALL = SDICam
all: $(ALL)

.$(C).$(OBJ):
	$(C_COMPILER) -o $@ -c $(C_FLAGS) $<       

.$(CPP).$(OBJ):
	$(CPLUSPLUS_COMPILER) -o $@ -c $(CPLUSPLUS_FLAGS) $<

SDICAM_OBJS = src/main.o src/htx_3531a.o src/OsdZone.o src/OsdMessageBox.o src/OsdGunStateBox.o\
	 src/OsdDirectionBox.o  src/OsdStateBox.o src/OsdDirCircle.o src/ctrlcmd.o src/ctrlres.o\
	 src/serialdata.o src/serial.o
	 
SAMPLE_OBJS = sample/sample_region.o sample/loadbmp.o sample/sample_comm_audio.o\
	sample/sample_comm_ivs.o sample/sample_comm_sys.o sample/sample_comm_vda.o sample/sample_comm_vdec.o\
	sample/sample_comm_venc.o sample/sample_comm_vi.o sample/sample_comm_vo.o sample/sample_comm_vpss.o
HIIR_OBJS = hi_ir/hiir_test.o
OBJS = $(SDICAM_OBJS) $(SAMPLE_OBJS) $(HIIR_OBJS) $(COMM_OBJS)

src/htx_3531a.o:	src/htx_3531a.cpp src/ctrlcmd.h
src/common/sample_region.o:	src/common/sample_region.cpp
src/main.o:	src/main.cpp
src/OsdZone.cpp: src/OsdZone.h src/defines.h
src/OsdMessageBox.cpp: src/OsdMessageBox.h src/defines.h
src/OsdStateBox.cpp: src/OsdStateBox.h src/defines.h
src/OsdGunStateBox.cpp: src/OsdGunStateBox.h src/defines.h
src/OsdDirectionBox.cpp: src/OsdDirectionBox.h src/defines.h
src/OsdDirCircle.cpp: src/OsdDirCircle.h src/defines.h
src/ctrlcmd.cpp: src/ctrlcmd.h
src/ctrlres.cpp: src/ctrlres.h
src/serialdata.cpp: src/serialdata.h
src/serial.cpp: src/serial.h

OSD_LIB = osd/lib/libfreetype.a osd/lib/libSDL.a osd/lib/libSDL_ttf.a
HISI_LIB = hi_sdk/lib/libdnvqe.a hi_sdk/lib/libhdmi.a hi_sdk/lib/libive.a hi_sdk/lib/libjpeg.a\
	hi_sdk/lib/libjpeg6b.a hi_sdk/lib/libmd.a hi_sdk/lib/libmpi.a hi_sdk/lib/libpciv.a hi_sdk/lib/libtde.a\
	hi_sdk/lib/libupvqe.a hi_sdk/lib/libVoiceEngine.a

LOCAL_LIBS =	$(OSD_LIB) 

SYS_LIBS = -lpthread -ldl -lmpi -ltde -lhdmi -lVoiceEngine -lupvqe -ldnvqe -ljpeg -lSDL -lSDL_ttf -lfreetype
LIBS =			$(LOCAL_LIBS) $(SYS_LIBS)

SDICam:	$(OBJS)
	$(LINK) bin/$@ -Lhi_sdk/lib -Losd/lib $(OBJS) $(LIBS)
	cp bin/SDICam ~/bin

clean:
	-rm -rf $(OBJS) core *.core *~
	-rm -rf bin/$(ALL)
