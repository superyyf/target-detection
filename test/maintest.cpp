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
#include "test.hpp"

#include<pthread.h>

using namespace cv;
using namespace std;

struct TargetData{
        unsigned short x;
        unsigned short y;
        int frame_num;
        uint8_t t_h;
        uint8_t t_m;
        uint8_t t_s;
        uint16_t t_ms;
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

	set_system_time();

	//int len1 = UART0_Send(fd, 1);
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

	unsigned short x1 = 0;
	unsigned short y1 = 0;
	bool update_flag = true;
	
	int sockClient = socket(AF_INET, SOCK_DGRAM, 0);//初始化socket
	if (sockClient == -1){
		printf("socket error!");
	}

	struct sockaddr_in addrSrv;
	addrSrv.sin_addr.s_addr = inet_addr("192.168.1.11");//ip地址重要！！！Srv IP is "192.168.1.10"
	addrSrv.sin_family = AF_INET;
	addrSrv.sin_port = htons(10011);//重要！！！端口编号10011

	Mat img_back(256, 320, CV_8UC1);

	//pxd_doSnap(UNITSMAP, 1, 0); //保存单张图片
	//pxd_readushort函数捕捉16bits数据到缓冲区，成功i>0,i<0失败，函数中止运行
	//捕捉16bits图像到缓冲区colorimage_buf1
	//pxd_goneLive函数源源不断的捕获图像，手册有介绍
	pxd_goLive(UNITSMAP, 1);
	pxd_readushort(UNITSMAP, 1, cx, cy, cx + XDIM, cy + YDIM, colorimage_buf1, sizeof(colorimage_buf1) / sizeof(ushort), "Grey");

	clock_t start_1, end_1, start_p;
	start_1 = clock();
	while (pxd_goneLive(UNITSMAP, 0))//capture picture
	{
		start_p = clock();
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
			printf("**************************映射关系更新**********************\n");
		}
		
		Mat dst_2(nr, nc, CV_8UC1);
		uchar * p_2 = NULL;
        	//uchar img_con[512*640];
		for (int i = 0; i < nr; i++)
		{
			//获取第i行像素数组首指针
			p_2 = dst_2.ptr<uchar>(i);
			p_1 = src.ptr<ushort>(i);
			//根据映射关系将原图像灰度替换成直方图均衡后的灰度
			for (int j = 0; j < nc; j++)
			{
				p_2[j] = transf_fun[p_1[j]];
                        	//img_con[i*640+j] = transf_fun[p_1[j]];
			}
		}	
		//将直方图均衡化结果dst_2复制给img，img进行网络传输。
		//注意！！！考虑等号赋值条件与深拷贝 浅拷贝之间的关系
		imshow("Frame",dst_2);
		cvWaitKey(1);
		Mat src_win = dst_2(Rect( AOI_X, AOI_Y, AOI_WIDTH, AOI_HIGH));	
		imshow("Frame_win", src_win);
		cvWaitKey(1);

		FrameNum++;

		Mat image_pro = src_win.clone();
		 
		//背景初始化
		if(update_flag)
		{
			img_back = image_pro.clone();
			printf("\n--------------------------------------背景初始化-----------------------------\n");
			update_flag = false;
		}

		TargetData targetdata;

		//目标检测
		vector<DetectInfo> detect_infos = detection(img_back, image_pro, AREA_THRESHOLD);
		if(detect_infos.size())
		{
			//target_count++;
			x1 = AOI_X + (unsigned short)detect_infos[0].x;
			y1 = AOI_Y + (unsigned short)detect_infos[0].y;
			printf("Target : [ %d , %d ]\n", x1, y1);
			//sprintf(filename, "%s%d%s", prefix, target_count,postfix);
			SendData sendata;
			get_remote_time(&sendata);
                        targetdata.t_h = sendata.t_h;
                        targetdata.t_m = sendata.t_m;
                        targetdata.t_s = sendata.t_s;
                        targetdata.t_ms = sendata.t_ms;
                        printf("target_th = %d\ntarget_tm = %d\ntarget_ts = %d\ntarget_tms = %d\n", targetdata.t_h, targetdata.t_m, targetdata.t_s, targetdata.t_ms);
//imwrite(filename, image_pro);
		}
		else
		{
			x1 = 0;
			y1 = 0;
			targetdata.t_h = 0;
			targetdata.t_m = 0;
			targetdata.t_s = 0;
			targetdata.t_ms = 0;
		}
		if (FrameNum % 50 == 0 && x1 == 0)
		{
			img_back = image_pro.clone();
			printf("****************************背景更新************************\n");
		}


		targetdata.x = x1;
		targetdata.y = y1;
		targetdata.frame_num = FrameNum;
		

		if(targetdata.x != 0){
			SendInfo sendinfos;
			sendinfos.f_num = targetdata.frame_num;
			sendinfos.t_h = targetdata.t_h;
			sendinfos.t_m = targetdata.t_m;
			sendinfos.t_s = targetdata.t_s;
			sendinfos.t_ms = targetdata.t_ms;
			sendinfos.x1 = targetdata.x;
			sendinfos.y1 = targetdata.y;
			if(int set = sendto(sockClient, &sendinfos, sizeof(sendinfos), 0, (struct sockaddr*)&addrSrv, sizeof(struct sockaddr)) < 0){
				perror("UDP Error ");
			}
		}	
		
	}
	int fd1 = serialport_inti();
	close_video_flow(fd1);
	close(sockClient);//关闭socket
	return 0;	
}




