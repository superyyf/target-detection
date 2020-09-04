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
#include<pthread.h>

using namespace cv;
using namespace std;


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

int mouse_on = 0;
int mouse_click = 0;
int i_cen = YDIM / 2;
int j_cen = XDIM /2;
int img_x = 0;
int img_y = 0;

void on_mouse(int event, int x, int y, int flags, void* ustc)
{
	CvFont font;
	cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 0.5, 0.5, 0, 1, CV_AA);

	if (event == CV_EVENT_LBUTTONDOWN)
	{
		CvPoint pt = cvPoint(x, y);
		char temp[16];
		sprintf(temp, "(%d,%d)", pt.x, pt.y);
		img_x = pt.x;
		img_y = pt.y;

		printf("\n=========================\n");
                printf("x坐标 = %d\n", img_x);
                printf("y坐标 = %d\n\n", img_y);
		printf("AOI_X = %d\nAOI_Y = %d\n", img_x-160, img_y-128);
	}
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
	
	//定义存储16bits前景图像的缓冲区
	static ushort   colorimage_buf1[YDIM*XDIM*COLORS];
	//设置AOI感兴趣区域，一般不要修改
	int     i = 0;
	int     j = 0;
	int     cx = 0;	// left coordinate of centered AOI
	int     cy = 0;	// top	coordinate of centered AOI
	
	int nr = 512;//行
	int nc = 640;//列
	int total = nr*nc;//像素数
	int FrameNum = 0; //帧数
	uchar transf_fun[16384] = { 0 };//映射关系数组


	//pxd_doSnap(UNITSMAP, 1, 0); //保存单张图片
	//pxd_readushort函数捕捉16bits数据到缓冲区，成功i>0,i<0失败，函数中止运行
	//捕捉16bits图像到缓冲区colorimage_buf1
	//pxd_goneLive函数源源不断的捕获图像，手册有介绍
	pxd_goLive(UNITSMAP, 1);
	pxd_readushort(UNITSMAP, 1, cx, cy, cx + XDIM, cy + YDIM, colorimage_buf1, sizeof(colorimage_buf1) / sizeof(ushort), "Grey");

	while (pxd_goneLive(UNITSMAP, 0))//capture picture
	{
		if ((i = pxd_readushort(UNITSMAP, 1, cx, cy, cx + XDIM, cy + YDIM, colorimage_buf1, sizeof(colorimage_buf1) / sizeof(ushort), "Grey")) != XDIM * YDIM*COLORS) {/*xiugai*/
			if (i < 0)
				printf("pxd_readuchar: %s\n", pxd_mesgErrorCode(i));
			else
				printf("pxd_readuchar error: %d != %d\n", i, XDIM*YDIM * 3);
			user("");
			break;
		}
		//将colorimage_buf1中的16bits数据赋值给Mat矩阵
		Mat src(YDIM, XDIM, CV_16UC1, colorimage_buf1);//原始图像
		//Mat src_win = src(Rect( AOI_X, AOI_Y, AOI_WIDTH, AOI_HIGH));	
	//---------------------------16bits 图像直方图均衡化----------------------------------//

		ushort *p_1 = NULL;
		if (FrameNum % 80 == 0)//每50帧更新一次映射关系
		{
			//存储直方图统计结果的数组
			unsigned int hist[16384] = { 0 };
			//扫描原图像
			for (int i = 0; i < nr; i++)
			{
				//获取第i行像素数组首指针
				p_1 = src.ptr<ushort>(i);
				for (int j = 0; j < nc; j++)
				{
					hist[p_1[j]] ++;
				}
			}

			//计算灰度变换函数
			//transf_fun存储均衡前像素值与均衡后像素值的映射关系
			transf_fun[0] = (uchar)(255 * (hist[0] * 1.0) / (total*1.0));
			//累积
			for (int i = 1; i < 16384; i++)
			{
				hist[i] = hist[i - 1] + hist[i];
				transf_fun[i] = (uchar)(255 * (hist[i] * 1.0) / (total*1.0));
			}
		}
		
		Mat dst_2(512, 640, CV_8UC1);
		uchar * p_2 = NULL;
        	//uchar img_con[512*640];
		for (int i = 0; i < 512; i++)
		{
			//获取第i行像素数组首指针
			p_2 = dst_2.ptr<uchar>(i);
			p_1 = src.ptr<ushort>(i);
			//根据映射关系将原图像灰度替换成直方图均衡后的灰度
			for (int j = 0; j < 640; j++)
			{
				p_2[j] = transf_fun[p_1[j]];
                        	//img_con[i*640+j] = transf_fun[p_1[j]];
			}
		}	
		//将直方图均衡化结果dst_2复制给img，img进行网络传输。
		//注意！！！考虑等号赋值条件与深拷贝 浅拷贝之间的关系
		//-------------------------------十字线叠加------------------------
		for (int i = -256; i < 256; i++)
		{
			dst_2.at<uchar>(i_cen + i, j_cen) = 0;

		}
		for(int j = -320; j < 320; j++)
		{
			dst_2.at<uchar>(i_cen , j_cen + j) = 0;
		}
		Mat img = dst_2.clone();

	imshow("img",img);
	cvWaitKey(1);
        cvSetMouseCallback("img", on_mouse, 0);

//-----------------------------------------抠图----------------------------------------------
		if (img_x != 0 && img_y != 0)
		{

			int x_offset = img_x - 160;
			int y_offset = img_y - 128;
			Mat img_windows(256, 320, CV_8UC1);
			for (int i = 0; i < 256; i++)
				for (int j = 0; j < 320; j++)
				{
					int x_real = x_offset + j;
					int y_real = y_offset + i;
					if (x_real <= 0 || y_real <= 0 || x_real >= 640 || y_real >= 512)
					{
						img_windows.at<uchar>(i, j) = 0;
					}
					else
					{
						img_windows.at<uchar>(i, j) = img.at<uchar>(i, j);
					}
				}
	
			imshow("img_windows", img_windows);

		}
	}
	return 0;	
}



