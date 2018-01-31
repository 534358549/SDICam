extern "C"
{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <fcntl.h>
#include <termios.h> 
#include <linux/serial.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#include <net/if_arp.h>
#include <netdb.h>

}
//#include <termios.h>
#include <asm/termbits.h>

#include "serial.h"
#include "serialdata.h"
#include "ctrlcmd.h"
#include "ctrlres.h"
#include "defines.h"

#include "OsdZone.h"
#include "OsdGunStateBox.h"
#include "OsdDirectionBox.h"
#include "OsdDirCircle.h"
#include "OsdMessageBox.h"

extern OsdZone *g_osdZoneCross;
extern OsdGunStateBox *g_osdZoneRightDown;
extern OsdDirectionBox *g_osdZoneLeftDown;
extern OsdDirCircle *g_osdZoneDirCircle;
extern int g_currSdiInputId;

OsdMessageBox *g_osdMsgBox = NULL;

static int speed_arr[] = {B38400, B115200, B19200, B9600, B4800, B2400, B1200, B300, B38400, B19200, B9600, B4800, B2400, B1200, B300};
static int name_arr[] = {38400, 115200, 19200, 9600, 4800, 2400, 1200, 300, 38400, 19200, 9600, 4800, 2400, 1200, 300};

#define BUF_LEN 1024
unsigned char RecvDataBuf[BUF_LEN];

int uart1,uart2,uart3,uart4,uart5;
extern int g_keepRunning;

int g_lastKey = -1;

// 3 serial ports
SerialData g_serialData[3];

int printCmd(unsigned char *cmd, int cmdLen)
{
	printf("received command(%d): %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n", cmdLen,
		cmd[0], cmd[1], cmd[2], cmd[3], cmd[4], cmd[5], cmd[6], cmd[7], cmd[8], cmd[9], cmd[10], cmd[11], cmd[12], cmd[13], cmd[14]);
	return 0;
}

void handleKeyCode(unsigned char keyCode)
{
	if(g_osdMsgBox)
	{
		delete g_osdMsgBox;
		g_osdMsgBox = NULL;
	}

	if(keyCode == 0x0b)
	{
		switch(g_lastKey)
		{
		case 0x01:
		case 0x02:
			g_osdZoneRightDown->setGunAdjustState(g_lastKey);
			break;
		case 0x03:
		case 0x04:
			g_osdZoneRightDown->setHandState(g_lastKey);
			break;
		case 0x05: // reserved
			break;
		case 0x06:
			break;
		case 0x07:
			break;
		case 0x08: // reserved, shutdown
			break;
		default:
			// move the cross
			if(g_osdZoneRightDown->canAdjustGun())
				g_osdZoneCross->move(-2, 0);
		}
	}
	else if(keyCode == 0x0c)
	{
		switch(g_lastKey)
		{
		case 0x01:
		case 0x02:
			break;
		case 0x03:
		case 0x04:
			break;
		case 0x05: // reserved
			break;
		case 0x06:
			break;
		case 0x07:
			break;
		case 0x08: // reserved, shutdown
			break;
		default:
			// move the cross
			if(g_osdZoneRightDown->canAdjustGun())
				g_osdZoneCross->move(2, 0);
		}
	}
	else
	{
		switch(keyCode)
		{
		case 0x01:
			g_osdMsgBox = new OsdMessageBox(g_currSdiInputId, 7);
			g_osdMsgBox->showMessageBox(UTF8_CONFIRM_JIAOQIANGKAI);
			break;
		case 0x02:
			g_osdMsgBox = new OsdMessageBox(g_currSdiInputId, 7);
			g_osdMsgBox->showMessageBox(UTF8_CONFIRM_JIAOQIANGGUAN);
			break;
		case 0x03:
			g_osdMsgBox = new OsdMessageBox(g_currSdiInputId, 7);
			g_osdMsgBox->showMessageBox(UTF8_CONFIRM_SHOUBINGKAI);
			break;
		case 0x04:
			g_osdMsgBox = new OsdMessageBox(g_currSdiInputId, 7);
			g_osdMsgBox->showMessageBox(UTF8_CONFIRM_SHOUBINGGUAN);
			break;
		case 0x05:			
			break;
		case 0x06:
			g_osdMsgBox = new OsdMessageBox(g_currSdiInputId, 7);
			g_osdMsgBox->showMessageBox(UTF8_CONFIRM_DANYAOCHONG);
			break;
		case 0x07:
			g_osdMsgBox = new OsdMessageBox(g_currSdiInputId, 7);
			g_osdMsgBox->showMessageBox(UTF8_CONFIRM_QIANGTALING);
			break;
		case 0x08:
			break;
		case 0x09:
			if(g_osdZoneRightDown->canAdjustGun())
				g_osdZoneCross->move(0, -2);
			break;
		case 0x0a:
			if(g_osdZoneRightDown->canAdjustGun())
				g_osdZoneCross->move(0, 2);
			break;
		default:
			break;
		}
	}
}

void handleCmd(CtrlCmd *ctrlCmd)
{
	char str[32];
	short hSpeed = ctrlCmd->getHSpeed();
	short hAngle = ctrlCmd->getHAngle();
	short vSpeed = ctrlCmd->getVSpeed();
	short vAngle = ctrlCmd->getVAngle();

	printf("hSpeed:%d, hAngle:%d, vSpeed:%d, vAngle:%d\n", hSpeed, hAngle, vSpeed, vAngle);
	
	g_osdZoneRightDown->setState(ctrlCmd->kaishuan, ctrlCmd->restBullet, ctrlCmd->gunZero, ctrlCmd->trigger);
	g_osdZoneLeftDown->setDirection(hAngle, hSpeed, vAngle, vSpeed);
	g_osdZoneDirCircle->setDegree((hAngle + 5) / 10);

	handleKeyCode(ctrlCmd->keyboard);

	g_lastKey = ctrlCmd->keyboard;
}

int setRS232(int mFd, int band, int stopBit, int parity, int dataBit)
{
    int status = -1;

    struct termios options;
    tcgetattr( mFd, &options );
    tcflush( mFd, TCIOFLUSH ); // discard data written to RS485 that hasn't not been transmitted
    // set band rate
    switch( band )
    {
    case 1200:
        cfsetispeed( &options, B1200 );
        cfsetospeed( &options, B1200 );
        break;
    case 2400:
        cfsetispeed( &options, B2400 );
        cfsetospeed( &options, B2400 );
        break;
    case 4800:
        cfsetispeed( &options, B4800 );
        cfsetospeed( &options, B4800 );
        break;
    case 9600:
        cfsetispeed( &options, B9600 );
        cfsetospeed( &options, B9600 );
        break;
    case 19200:
        cfsetispeed( &options, B19200 );
        cfsetospeed( &options, B19200 );
        break;
    case 38400:
        cfsetispeed( &options, B38400 );
        cfsetospeed( &options, B38400 );
        break;
    case 57600:
        cfsetispeed( &options, B57600 );
        cfsetospeed( &options, B57600 );
        break;
    default:
        printf("Band rate:%d is not supported, now use 9600", band);
        cfsetispeed( &options, B9600 );
        cfsetospeed( &options, B9600 );
        break;
    }
    status = tcsetattr( mFd, TCSANOW, &options );
    if( status != 0 )
    {
        printf("tcsetattr(TCSANOW)");
        return -1;
    }
    tcflush( mFd, TCIOFLUSH );

    status = tcgetattr( mFd, &options );
    if( status < 0 )
    {
        printf("tcgetattr()");
        return -1;
    }

    // set data bits
    options.c_cflag &= (~CSIZE);
    switch( dataBit )
    {
    case 5:
        options.c_cflag |= CS5;
        break;
    case 6:
        options.c_cflag |= CS6;
        break;
    case 7:
        options.c_cflag |= CS7;
        break;
    case 8:
        options.c_cflag |= CS8;
        break;
    default:
        printf("Unsupported data bits(%d)", dataBit);
        return -1;
        break;
    }

    // set parity
    switch( parity )
    {
    case 0: // None
        options.c_cflag &= (~PARENB);
        options.c_iflag &= (~INPCK);
        break;
    case 1: // ODD
        options.c_cflag |= (PARODD | PARENB);
        options.c_iflag |= INPCK;
        break;
    case 2: // Even
        options.c_cflag |= PARENB;
        options.c_cflag &= (~PARODD);
        options.c_iflag |= INPCK;
        break;
    default:
        printf("Unsupported parity type:%d", parity);
        return -1;
        break;
    }

    // set stop bits
    switch( stopBit )
    {
    case 1:
        options.c_cflag &= (~CSTOPB);
        break;
    case 2:
        options.c_cflag |= CSTOPB;
        break;
    default:
        printf("Unsupported stop bits:%d", stopBit);
        return -1;
        break;
    }
    /* Set input parity option */
    if (parity != 0)
        options.c_iflag |= INPCK;
    /* Set Raw Mode */
    options.c_lflag  &= ~(ICANON | ECHO | ECHOE | ISIG);  /*Input*/
    options.c_oflag  &= ~OPOST;   /*Output*/
    tcflush( mFd, TCIFLUSH );
    options.c_cc[VTIME] = 150;
    options.c_cc[VMIN] = 7;

    status = tcsetattr( mFd, TCSANOW, &options );
    if( status != 0 )
    {
        printf("tcsetattr()");
        return -1;
    }

    return 0;
}


void request_send(int fd)
{
	int status;
	ioctl(fd, TIOCMGET, &status);
	status &= ~TIOCM_RTS;   // RTS ??????
	ioctl(fd, TIOCMSET, &status);
}

void clear_send(int fd)
{
	int status;
	ioctl(fd, TIOCMGET, &status);
	status |= TIOCM_RTS;    // RTS ??????
	ioctl(fd, TIOCMSET, &status);
}


void close_port(int fd)
{
	if(fd== 0)return;
	close(fd);
	fd = 0;
}

void set_speed(int fd, int speed)
{
	struct termios Opt;
	tcgetattr(fd, &Opt);
	for(int i = 0; i < sizeof(speed_arr)/sizeof(int); i++)
	{
		if(speed == name_arr[i])
		{
			tcflush(fd, TCIOFLUSH);
			cfsetispeed(&Opt, speed_arr[i]);
			cfsetospeed(&Opt, speed_arr[i]);
			int status = tcsetattr(fd, TCSANOW, &Opt);
			if(status != 0) perror("tcsetattr fd1");
			return;
		}
		tcflush(fd,TCIOFLUSH);
	}
}

int set_parity(int fd, int databits, int stopbits, int parity)
{
	struct termios options;
	if  ( tcgetattr( fd,&options)  !=  0)
	{
		perror("SetupSerial 1");
		return(0);
	}

	options.c_cflag |= CLOCAL |CREAD ;  //CREAD???????  CLOCAL????????????????????
	options.c_cflag &= ~CSIZE;          //CSIZE ???�� ?????��???????????
	switch (databits)
	{
	case 7:
		options.c_cflag |= CS7;
		break;
	case 8:
		options.c_cflag |= CS8;
		break;
	default:
		fprintf(stderr,"Unsupported data size\n");
		return (0);
	}
	switch (parity)
	{
	case 'n':
	case 'N':
		options.c_cflag &= ~PARENB;   //PARENB ��??��???
		options.c_iflag &= ~INPCK;    //INPCK ��?????
		break;
	case 'o':
	case 'O':
		options.c_cflag |= (PARODD | PARENB);   //PARENB ��??��???  PARODD ?????��??
		options.c_iflag |= INPCK;
		break;
	case 'e':
	case 'E':
		options.c_cflag |= PARENB;  //PARENB ��??��???
		options.c_cflag &= ~PARODD;   //INPCK ��?????  PARODD ?????��??
		options.c_iflag |= INPCK;
		break;
	case 'S':
	case 's':
		options.c_cflag &= ~PARENB;   //PARENB ��??��???
		options.c_cflag &= ~CSTOPB;   //2????��
		break;
	case 'M':
	case 'm':
		options.c_cflag |= CMSPAR;   //PARENB ��??��???
		options.c_cflag &= ~PARODD;   //2????��
		break;
		
	default:
		fprintf(stderr,"Unsupported parity\n");
		return (0);
	}

	switch (stopbits)
	{
	case 1:
		options.c_cflag &= ~CSTOPB;//1????��
		break;
	case 2:
		options.c_cflag |= CSTOPB; //
		break;
	default:
		fprintf(stderr,"Unsupported stop bits\n");
		return (0);
	}

	/*No hardware control*/
	options.c_cflag &= ~CRTSCTS;
	/*No software control*/
	options.c_iflag &= ~(IXON | IXOFF | IXANY);
	/*delay time set */

	//options.c_cflag|= IXON|IXOFF|IXANY;   //  ????????????


	if(parity != 'n') options.c_iflag |= INPCK;
	options.c_cc[VTIME] = 150;
	options.c_cc[VMIN] = 0;

//	options.c_cc[VTIME] = 0;
//	options.c_cc[VMIN] = 14;
	/*raw model*/
#if 1
	options.c_lflag  &= ~(ICANON | ECHO | ECHOE | ISIG);
	options.c_oflag  &= ~OPOST;

    options.c_iflag &= ~(INLCR|IGNCR|ICRNL);
	options.c_iflag &= ~(ONLCR|OCRNL);

	options.c_oflag &= ~(INLCR|IGNCR|ICRNL);
	options.c_oflag &= ~(ONLCR|OCRNL);
#endif
	tcflush(fd,TCIFLUSH);
	if (tcsetattr(fd,TCSANOW,&options) != 0)
	{
		perror("SetupSerial 3");
		return (0);
	}

	return (1);
}




// set baudrate to 28800, used for TAX 
int serial_set_speci_baud(int fd,int baud)
{

	struct termios2 tio;
	ioctl(fd, TCGETS2, &tio);
	tio.c_cflag &= ~CBAUD;
	tio.c_cflag |= BOTHER;
	tio.c_ispeed = baud;
	tio.c_ospeed = baud;
	/* do other miscellaneous setup options with the flags here */
	ioctl(fd, TCSETS2, &tio);

	return 0;

}

int uart_open(const char *dev,unsigned baud)
{
	int fd = 0;

	fd = open(dev, O_RDWR|O_NDELAY);

	if(-1 == fd)
	{
		fd = 0;
		printf("Can't Open Serial Port!\n");
		return 0;
	}
	else
	{
		printf("Open Serial dev=%s,baud=%d\n",dev,baud);
	}

	setRS232(fd, baud, 1, 0, 8);

//	serial_set_speci_baud(fd,baud);
//	set_parity(fd, 8, 1, 'N');

	return fd;
}

void * SendData(void *)
{

    unsigned char SendBuf[3];
    SendBuf[0]=0x5a;
    SendBuf[1]=0x54;
    SendBuf[2]=0x54;

	while(1)
	{
//		printf("SendData\n");

		write(uart1,SendBuf,sizeof(SendBuf));
		usleep(1000000);

		write(uart2,SendBuf,sizeof(SendBuf));
		usleep(1000000);

		write(uart3,SendBuf,sizeof(SendBuf));
		usleep(1000000);

//		write(uart4,SendBuf,1);
//		usleep(500);
//
//		write(uart5,SendBuf,1);
//		usleep(500);
	}
 
    return 0;
}

void * RecvData(void *)
{
	int uart1num,uart2num,uart3num,uart4num,uart5num;
    while(1)
    {
 
		uart1num = read(uart1,RecvDataBuf,BUF_LEN);
		if(uart1num>0)
		{
			printf("============= %d bytes ===========\n", uart1num);
			for(int i=0;i<uart1num;i++)
				printf("###### uart1 RecvData[%d]=%02x\n",i,RecvDataBuf[i]);

			memset(RecvDataBuf,0,sizeof(RecvDataBuf));
			uart1num=0;
		}
        usleep(500);

		uart2num = read(uart2,RecvDataBuf,BUF_LEN);
		if(uart2num>0)
		{
			printf("============= %d bytes ===========\n", uart2num);
			for(int i=0;i<uart2num;i++)
				printf("###### uart2 RecvData[%d]=%02x\n",i,RecvDataBuf[i]);

			memset(RecvDataBuf,0,sizeof(RecvDataBuf));
			uart2num=0;
		}
        usleep(500);

		uart3num = read(uart3,RecvDataBuf,BUF_LEN);
		if(uart3num>0)
		{
			printf("============= %d bytes ===========\n", uart3num);
			for(int i=0;i<uart3num;i++)
				printf("###### uart3 RecvData[%d]=%02x\n",i,RecvDataBuf[i]);

			memset(RecvDataBuf,0,sizeof(RecvDataBuf));
			uart3num=0;
		}
        usleep(500);
//
//		uart4num = read(uart4,RecvDataBuf,BUF_LEN);
//		if(uart4num>0)
//		{
//			for(int i=0;i<uart4num;i++)
//			printf("###### uart4 RecvData=%c\n",RecvDataBuf[i]);
//
//			memset(RecvDataBuf,0,sizeof(RecvDataBuf));
//			uart4num=0;
//		}
//        usleep(500);
//
//		uart5num = read(uart5,RecvDataBuf,BUF_LEN);
//		if(uart5num>0)
//		{
//			for(int i=0;i<uart5num;i++)
//			printf("###### uart5 RecvData=%c\n",RecvDataBuf[i]);
//
//			memset(RecvDataBuf,0,sizeof(RecvDataBuf));
//			uart5num=0;
//		}

        usleep(500);
    }
    return 0;
}

int Serial_test(void)
{

   int ret;
   pthread_t SendId,RecvId;
   
//   uart1 = uart_open("/dev/tty1", 9600);
	uart1 = uart_open("/dev/ttyAMA1", 9600);
	uart2 = uart_open("/dev/ttyAMA2", 9600);
	uart3 = uart_open("/dev/ttyAMA3", 9600);
//	uart4=uart_open("/dev/ttyUSB0",9600);
//	uart5=uart_open("/dev/ttyUSB1",9600);

   ret=pthread_create(&SendId,NULL,SendData,NULL);
   if( ret != 0 )
   {
       printf("create SendData thread error");
       exit(1);
   }

   ret=pthread_create(&RecvId,NULL,RecvData,NULL);
   if( ret != 0 )
   {
       printf("create RecvData thread error");
       exit(1);
   }

   return 0;
}

int serialHandle()
{
	int fd[3];
	char buff[64];
	CtrlCmd ctrlCmd;
	int len = 0;
	int i;
	int maxfd = 0;

	fd[0] = uart_open("/dev/ttyAMA1",9600);
	fd[1] = uart_open("/dev/ttyAMA2",9600);
	fd[2] = uart_open("/dev/ttyAMA3",9600);

	CtrlRes res;

	for(i = 0; i < 3; i++)
	{
		res.info = 2;
		res.pack();
		write(fd[i], &res, sizeof(res));
		
		if(maxfd < fd[i] + 1)
			maxfd = fd[i] + 1;
	}

	res.info = 0;	// receive OK
	res.pack();

	struct timeval timeout;
	fd_set read_fds;

	while(g_keepRunning)
	{
		memset(buff, 0, sizeof(buff));
	    timeout.tv_sec = 0;
		timeout.tv_usec = 900000;
        FD_ZERO(&read_fds);
        FD_SET(fd[0], &read_fds);
        FD_SET(fd[1], &read_fds);
        FD_SET(fd[2], &read_fds);
		int ret = select(maxfd, &read_fds, NULL, NULL, &timeout);
		//printf("select returns %d\n", ret);
		if(ret > 0)
		{
			for(i = 0; i < 3; i++)
			{
				if(FD_ISSET(fd[i], &read_fds))
				{
					len = read(fd[i], buff, sizeof(buff));
					g_serialData[i].putData((unsigned char *)buff, len);
					if(g_serialData[i].haveAvaiablePkt())
					{
						g_serialData[i].popPktData((unsigned char *)&ctrlCmd);
						
						// send response first
						write(fd[i], &res, sizeof(res));
						
						printCmd((unsigned char *)&ctrlCmd, 15);
						handleCmd(&ctrlCmd);
					}
				}
			}
		}
	}

	close(fd[0]);
	close(fd[1]);
	close(fd[2]);
}


