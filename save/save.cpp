#if !defined(FORMAT) && !defined(FORMATFILE)

#define FORMATFILE	"/usr/local/xcap/data/ddd.fmt"	  // using format file saved by XCAP
#endif

/*
 *  2.1) Set number of expected PIXCI(R) image boards.
 *  The XCLIB Simple 'C' Functions expect that the boards are
 *  identical and operated at the same resolution.
 *
 *  For PIXCI(R) frame grabbers with multiple, functional units,
 *  the XCLIB presents the two halves of the
 *  PIXCI(R) E1DB, E4DB, E8CAM, E8DB, e104x4-2f, ECB2, EL1DB, ELS2, SI2, or SV7 frame grabbers,
 *  or the three parts of the PIXCI(R) e104x4-f2b frame grabber,
 *  or the four quarters of the PIXCI(R) e104x4-4b or SI4 frame grabbers,
 *  as two, three, or four independent PIXCI(R) frame grabbers, respectively.
 *
 *  This example expects only one unit.
 */
#if !defined(UNITS)
#define UNITS	1
#endif
#define UNITSMAP    ((1<<UNITS)-1)  // shorthand - bitmap of all units


 /*
  *  2.2) Optionally, set driver configuration parameters.
  *  These are normally left to the default, "".
  *  The actual driver configuration parameters include the
  *  desired PIXCI(R) frame grabbers, but to make configuation easier,
  *  code, below, will automatically add board selection to this.
  *
  *  Note: Under Linux, the image frame buffer memory can't be set as
  *  a run time option. It MUST be set via insmod so the memory can
  *  be reserved during Linux's initialization.
  */
#if !defined(DRIVERPARMS)
  //#define DRIVERPARMS "-QU 0"       // don't use interrupts
  //#define DRIVERPARMS "-IM 8192"    // request 8192 mbyte of frame buffer memory
#define DRIVERPARMS ""	      // default
#endif


/*
 *  3)	Choose whether the optional PXIPL Image Processing Library
 *	is available.
 */
#if !defined(USE_PXIPL)
#define USE_PXIPL	0
#endif


 /*
  *  4) Select directory for saving of images.
  *  This example will not overwrite existing images files;
  *  so directory selection is not critical.
  */
#if !defined(IMAGEFILE_DIR)
#define IMAGEFILE_DIR    "."
#endif



  /*
   *  NECESSARY INCLUDES:
   */
#include <stdio.h>		// c library
#include <signal.h>		// c library
#include <stdlib.h>		// c library
#include <stdarg.h>		// c library
#include <unistd.h>		// c library
#include <limits.h>		// c library
#include "xcliball.h"		// function prototypes
#if USE_PXIPL
#include "pxipl.h"		// function prototypes
#endif

#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <malloc.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <stdarg.h>
#include <fcntl.h>
#include <stdint.h>
#include <vector>

#include<sys/socket.h>
#include<netinet/in.h>

#include<iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include<opencv2/imgproc/imgproc_c.h>
#include <cv.h>
#include <memory>
#include "main.hpp"
#include "queue.hpp"

#include<pthread.h>
#include<chrono>

using namespace cv;
using namespace std;

typedef void *(*THREAD_FUNC)(void *);

struct CamData {
	Mat image;
	int frame_num;
};
struct ImageData {
	Mat image;
	int frame_num;
};

struct TargetData{
	unsigned short x;
	unsigned short y;
	int frame_num;
	uint8_t t_h;
	uint8_t t_m;
	uint8_t t_s;
	uint16_t t_ms;
};

template<typename Input, typename Output> struct Pipe{
	Queue<Input> *input;
	Queue<Output> *output;
	Pipe(Queue<Input> *a, Queue<Output> *b):input(a), output(b){}
};

static void user(char *mesg)
{
	if (mesg && *mesg)
		printf("%s\n", mesg);
	fprintf(stderr, "\n\nContinue (Key ENTER) ?");
	while (getchar() != '\n');
	fprintf(stderr, "\n");
}

/*
 * Close the PIXCI(R) frame grabber
 */
static void do_close(void)
{
	pxd_PIXCIclose();
	user("PIXCI(R) frame grabber closed");
}

/*
 * Here and elsewhere: we declare as 'static'
 * to avoid compiler's 'missing prototype' warnings.
 */
static void hello(void)
{
	printf("\n\n");
	printf("PIXCI(R) Frame Grabbers -  XCLIB 'C' Library\n");
	printf("XCLIBEL1.C - Example program\n");
	printf("Copyright (C)  2004-2017  EPIX, Inc.  All rights reserved.\n");
	printf("\n");
	printf("This program is best used by executing this program\n");
	printf("one step at a time, while simultaneously reading\n");
	printf("the XCLIB documentation and program listing.\n");
	user("");
}

/*
 *  MAIN:
 *
 *  Each library function is demonstrated in its own subroutine,
 *  the main calls each subroutine to produce the interactive demonstration.
 *
 *  It is suggested that this program source be read at the same time
 *  as the compiled program is executed.
 *
 */
#define XDIM 640 //width
#define YDIM 512 //height
#define COLORS 1
#define AOI_X 160
#define AOI_Y 128
#define AOI_WIDTH 320
#define AOI_HIGH 256
#define AREA_THRESHOLD  50
bool end_flag = false;
void sign_handle(int sign)
{
	end_flag = true;
}

void *get_image_thread(Queue<CamData> *c)
{
	static ushort   colorimage_buf1[YDIM*XDIM*COLORS];
	//设置AOI感兴趣区域，一般不要修改
	int     i = 0;
	int     j = 0;
	int     cx = 0;	// left coordinate of centered AOI
	int     cy = 0;	// top	coordinate of centered AOI

	int FrameNum = 0; //帧数

	//pxd_goneLive函数源源不断的捕获图像，手册有介绍
	pxd_goLive(UNITSMAP, 1);
	pxd_readushort(UNITSMAP, 1, cx, cy, cx + XDIM, cy + YDIM, colorimage_buf1, sizeof(colorimage_buf1) / sizeof(ushort), "Grey");

	struct timeval start_1, end_1;
	while (pxd_goneLive(UNITSMAP, 0))//capture picture
	{
		gettimeofday(&start_1, NULL);
		if ((i = pxd_readushort(UNITSMAP, 1, cx, cy, cx + XDIM, cy + YDIM, colorimage_buf1, sizeof(colorimage_buf1) / sizeof(ushort), "Grey")) != XDIM * YDIM*COLORS) {/*xiugai*/
			if (i < 0)
				printf("pxd_readuchar: %s\n", pxd_mesgErrorCode(i));
			else
				printf("pxd_readuchar error: %d != %d\n", i, XDIM*YDIM * 3);
			user("");
			break;
		}
		FrameNum++;
		usleep(8000);
		//将colorimage_buf1中的16bits数据赋值给Mat矩阵
		Mat src(YDIM, XDIM, CV_16UC1, colorimage_buf1);//原始图像
		CamData camdata;
		camdata.image = src.clone();
		camdata.frame_num = FrameNum;
		c->push(move(camdata));
		if(FrameNum >= 15000)
		{
			FrameNum = 0;
		}
		if(end_flag)
		{
			break;
		}
		gettimeofday(&end_1, NULL);
                printf("Read Image = %fms-------------------------------------------------------\n", (double)(end_1.tv_usec - start_1.tv_usec)/1000);
		printf("-----------------------------------------------------------------------Frame = %d\n", FrameNum);
	}
	c->end();
	do_close();
	return NULL;
}

void *save_image_thread(Queue<CamData> *c)
{
	char prefix[] = "/home/nvidia/pic/frame_";
	char postfix[] = ".png";
	char filename[255];
	vector<int> compression_params;
	compression_params.push_back(CV_IMWRITE_PNG_COMPRESSION);
	compression_params.push_back(0);    // 无压缩png.

	struct timeval start_2, end_2;
	while(true)
	{
		unique_ptr<CamData> camdata;
		camdata = c->pop();
		if(camdata == NULL)
			break;
		gettimeofday(&start_2,NULL);
		Mat image = camdata->image;
		int frame_num = camdata->frame_num;
		sprintf(filename, "%s%d%s", prefix, frame_num, postfix);
		imwrite(filename, image, compression_params);
		printf("-------------------------------------------Frame_save = %d----------\n", frame_num);
		gettimeofday(&end_2, NULL);
                printf("----------------Save Image = %fms-----------------------------------\n", (double)(end_2.tv_usec - start_2.tv_usec)/1000);
	}
	return NULL;
}
int main(void)
{
	// Say Hello
	//判断是否xclib与相机连接成功
	hello();
	//pxd_PIXCIopen函数很重要，注意函数内部文件路径的格式，.fmt文件如何保存
	pxd_PIXCIopen("", "", "/home/nvidia/Desktop/try1/ddd14.fmt");//important
	//pxd_PIXCIopen("", "", "/home/nvidia/Desktop/histeq_detect_udp/ddd_1024.fmt");//important
	printf("Image frame buffer memory size: %.3f Kbytes\n", (double)pxd_infoMemsize(UNITSMAP) / 1024);
	printf("Image frame buffers           : %d\n", pxd_imageZdim());
	printf("Number of boards              : %d\n", pxd_infoUnits());

	printf("Frame Grabber %d\n", pxd_infoModel(UNITSMAP));

	printf("xdim           = %d\n", pxd_imageXdim());
	printf("ydim           = %d\n", pxd_imageYdim());
	printf("colors         = %d\n", pxd_imageCdim());
	printf("bits per pixel = %d\n", pxd_imageCdim()*pxd_imageBdim());
	
	struct sigaction signinfo;
	signinfo.sa_handler = sign_handle;
	signinfo.sa_flags = SA_RESETHAND;
	sigemptyset(&signinfo.sa_mask);
	sigaction(SIGINT, &signinfo, NULL);
	Queue<CamData> c;
	pthread_t t1, t2, t3;
	pthread_create(&t1, NULL, (THREAD_FUNC)get_image_thread, &c);	
	pthread_create(&t2, NULL, (THREAD_FUNC)save_image_thread, &c);	
	pthread_create(&t3, NULL, (THREAD_FUNC)save_image_thread, &c);	

	
	pthread_join(t1, NULL);
	pthread_join(t2, NULL);
	pthread_join(t3, NULL);

	return 0;	
}


