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
#define AOI_XDIM 640 //width
#define AOI_YDIM 512 //height
#define COLORS 1

#define BAOTOU 8


#define SYSFS_GPIO_EXPORT           "/sys/class/gpio/export"  
#define SYSFS_GPIO_RST_PIN_VAL      "388"   
#define SYSFS_GPIO_RST_DIR          "/sys/class/gpio/gpio388/direction"
#define SYSFS_GPIO_RST_DIR_VAL      "IN"  
#define SYSFS_GPIO_RST_VAL          "/sys/class/gpio/gpio388/value"
#define SYSFS_GPIO_RST_VAL_H        "1"
#define SYSFS_GPIO_RST_VAL_L        "0"
 //------------------------------全局变量--------------
int io_pos_flag = 0;
int count_nums = 0; //保存的图像序号
int count_on = 0;
int temp_count_nums = 0;
int img_x = 0;  //鼠标点击坐标
int img_y = 0; //
char detect = 0;//检测到目标与否
char mouse_on = 0;//鼠标点击坐标次数
char mouse_click = 0;//鼠标点击坐标次数
char show_on = 0;
const int AREA_THRESHOLD = 150;//面积阈值------------------------yyf


//-----------------------------鼠标点击时间------------------------
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
		mouse_on++;
                mouse_click =1;
                printf("img_x = %d\n", img_x);
                printf("img_y = %d\n", img_y);
	}
}






//-----------------保存图片---------------------------
static void* save_pic(void *data)
{
	count_nums += 1;
        uchar *img_con;
        img_con = (uchar*)data;
        Mat src(512, 640, CV_8UC1); 
	char _path[255];

	char prefix[] = "/home/nvidia/Desktop/histeq_detect_udp/pic/";
	char postfix[] = ".png";
        time_t start, end;
        start = clock();
	memset(_path, '\0', sizeof(char) * 255);
	sprintf(_path, "%sframe_%010d%s", prefix, count_nums, postfix);
        for(int i =0; i<512;i++)
           for(int j =0; j < 640;j++)
              {
                      src.at<uchar>(i, j) =img_con[i*640+j];
              }
	vector<int> compression_params;
	compression_params.push_back(CV_IMWRITE_PNG_COMPRESSION);
	compression_params.push_back(0);
        imwrite(_path, src,compression_params);//将8bits数据写入

	if (count_nums >= 12000)
	{
		count_nums = 0;
	}
        printf("保存帧号%d\n", count_nums);

		end = clock();
		double endtime = (double)(end - start) / CLOCKS_PER_SEC;
		printf("保存图像耗时线程 = %f\n", endtime);
                pthread_detach(pthread_self());
}



int main(void)
{
	//
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
	int num_upgrade = 0;

	FILE *fp;
	fp = fopen("2.txt", "w");

	//-----------------------网络通信包头信息与缓冲区的定义及初始化-----------------------//


		//初始化套接字init socket 	
	int sockClient = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockClient == -1)
	{
		printf("socket error!");
		return 0;
	}
	struct sockaddr_in addrSrv;
	addrSrv.sin_addr.s_addr = inet_addr("192.168.1.11");//ip地址重要！！！Srv IP is "192.168.1.10"
	addrSrv.sin_family = AF_INET;
	addrSrv.sin_port = htons(10011);//重要！！！端口编号10011

//-------------------------------------接收时间----------------------//
	unsigned char t_h = 10;
	unsigned char t_min = 15;
	unsigned char t_s = 56;
	unsigned short t_ms = 125;
//------------------------------------接收时间end-----------------------//


//-----------------------图像采集缓冲区以及AOI感兴趣区定义----------------------------//

		//定义存储16bits背景图像的缓冲区
	static ushort   colorimage_buf[AOI_YDIM*AOI_XDIM*COLORS];
	//定义存储16bits前景图像的缓冲区
	static ushort   colorimage_buf1[AOI_YDIM*AOI_XDIM*COLORS];
	//设置AOI感兴趣区域，一般不要修改
	int     i = 0;
	int     j = 0;
	int     cx = 0;	// left coordinate of centered AOI
	int     cy = 0;	// top	coordinate of centered AOI
//	int     count = 0;//target num_upgrade


	//捕捉16bits图像到缓冲区colorimage_buf1
	pxd_goLive(UNITSMAP, 1);
	pxd_readushort(UNITSMAP, 1, cx, cy, cx + AOI_XDIM, cy + AOI_YDIM, colorimage_buf1, sizeof(colorimage_buf1) / sizeof(ushort), "Grey");//xiugai

	//建立一个Mat类型的变量image
	Mat imageFront(256, 320, CV_8UC1);   //存储buf区的前景图像
	Mat image_med(AOI_YDIM, AOI_XDIM, CV_8UC1);//中值滤波之后图像

	//将colorimage_buf1背景图像存到Mat矩阵中
	//Mat imageBackground(AOI_YDIM, AOI_XDIM, CV_16UC1, colorimage_buf1);
	Mat imageBackground(256, 320, CV_8UC1);
	//进行16bits中值滤波
	//medianBlur(imageBackground,imageBackground,3);

	//定义目标检测变量，轮廓，腐蚀核，目标检测
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;
	Mat element = getStructuringElement(0, Size(3, 3));

        int num_100 = 0;

	int frmNum = 0;
	char _path[255];

	char prefix[] = "/home/nvidia/Desktop/histeq_detect_udp/pic/";
	char postfix[] = ".png";
	//pxd_goneLive函数源源不断的捕获图像，手册有介绍

	while (pxd_goneLive(UNITSMAP, 0))//capture picture
	{

		printf("count_nums=%d\n", count_nums);
		//pxd_doSnap(UNITSMAP, 1, 0); //保存单张图片

		//pxd_readushort函数捕捉16bits数据到缓冲区，成功i>0,i<0失败，函数中止运行
		//捕捉16bits图像到缓冲区colorimage_buf1
		if ((i = pxd_readushort(UNITSMAP, 1, cx, cy, cx + AOI_XDIM, cy + AOI_YDIM, colorimage_buf1, sizeof(colorimage_buf1) / sizeof(ushort), "Grey")) != AOI_XDIM * AOI_YDIM*COLORS) {/*xiugai*/
			if (i < 0)
				printf("pxd_readuchar: %s\n", pxd_mesgErrorCode(i));
			else
				printf("pxd_readuchar error: %d != %d\n", i, AOI_XDIM*AOI_YDIM * 3);
			user("");
			break;
		}
		//将colorimage_buf1中的16bits数据赋值给Mat矩阵
		Mat src(AOI_YDIM, AOI_XDIM, CV_16UC1, colorimage_buf1);//原始图像

	//---------------------------16bits 图像直方图均衡化----------------------------------//
		int nr = src.rows;//512
		int nc = src.cols;//640
		//像素总数
		int total = nr * nc;
		//转换后的目标矩阵，直方图均衡化结果
		Mat dst_2(nr, nc, CV_8UC1);
		//printf("total=%i\n",total);	
		//获取原图像直方图
		ushort *p_1 = NULL;
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
		uchar transf_fun[16384] = { 0 };
		transf_fun[0] = (uchar)(255 * (hist[0] * 1.0) / (total*1.0));
		//累积
		for (int i = 1; i < 16384; i++)
		{
			hist[i] = hist[i - 1] + hist[i];
			transf_fun[i] = (uchar)(255 * (hist[i] * 1.0) / (total*1.0));
		}

		uchar * p_2 = NULL;
                uchar img_con[512*640];
		for (int i = 0; i < nr; i++)
		{
			//获取第i行像素数组首指针
			p_2 = dst_2.ptr<uchar>(i);
			p_1 = src.ptr<ushort>(i);
			//根据映射关系将原图像灰度替换成直方图均衡后的灰度
			for (int j = 0; j < nc; j++)
			{
				p_2[j] = transf_fun[p_1[j]];
                                img_con[i*640+j] = transf_fun[p_1[j]];
			}
		}
		//将直方图均衡化结果dst_2复制给img，img进行网络传输。
		//注意！！！考虑等号赋值条件与深拷贝 浅拷贝之间的关系
		Mat img = dst_2.clone();
		//imshow("origin",src);
		//imshow("histeq",dst_2);
		//cvWaitKey(1);



		//-------------------------------十字线叠加------------------------
		for (int i = -25; i < 25; i++)
		{
			img.at<uchar>(AOI_YDIM / 2 + i, AOI_XDIM / 2) = 255;
			img.at<uchar>(AOI_YDIM / 2, AOI_XDIM / 2 + i) = 255;

		}
		


		//cvWaitKey(1);
		//src = cvLoadImage("lena.jpg", 1);



if(mouse_click == 0)
{
       imshow("img", img);
       cvSetMouseCallback("img", on_mouse, 0);
       waitKey(1);
}
if(mouse_click == 1)
{
      printf("mouse_click = %d\n", mouse_click);
      cv::destroyWindow("img");
      mouse_click = 2;
 //     imshow("img_click",img);
  //    waitKey(1);     
}
printf("mouse_click = %d\n", mouse_click);
//waitKey(1);

 
//waitKey(1);
//放在括号里面会导致图像不更新
//imshow("img", img);
//if(mouse_click==0)
//{		
//cvSetMouseCallback("img", on_mouse, 0);
//waitKey(1);
//}
		//		return 0;

//-----------------------------------------抠图----------------------------------------------
		char num_mouse_on = 1;
		if (img_x != 0 && img_y != 0)
		{
                        num_100 ++;
                        

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
						img_windows.at<uchar>(i, j) = dst_2.at<uchar>(i, j);
					}
				}
			//imshow("img_windows", img_windows);
                
                
//----------------------------------------------目标检测部分--------开始-----------------------------------------

			unsigned short x1 = 0;//目标坐标
			unsigned short y1 = 0;
			int area_max = 0;//目标面积
			


			if (num_mouse_on != mouse_on || num_upgrade == 0)
			{
				Mat imageBackground = img_windows.clone();
				num_mouse_on = mouse_on;
				num_upgrade = 0;
				
                                printf("--------------------------背景更新--------------------------------------\n");
			}
			else
			{
			        vector<DetectInfo> detect_infos = detection(imageBackground, img_windows, AREA_THRESHOLD);//目标检测---------------------yyf
			        if(detect_infos.size()){
			                x1 = (unsigned short)detect_infos[0].x;
			                y1 = (unsigned short)detect_infos[0].y;
			                area_max = detect_infos[0].area;
			                detect = 1;
			        }
			        
			}

                        x1 = x_offset + x1;
                        y1 = y_offset + y1;
                        printf("x1 = %d\n", x1);
                        printf("y1 = %d\n", y1);
                        printf("area = %d\n", area_max);
			
			SendInfo sendinfos(t_h, t_min, t_s, t_ms, x1, y1);
                       
			Net_Send_new(sockClient, addrSrv, &sendinfos);

			num_upgrade += 1;


			if (num_upgrade >= 50 && detect == 0)//有目标不更新------------------------yyf
			{
				Mat imageBackground = dst_2.clone();
				num_upgrade = 0;
			}//背景更新 放在后面，预防出现刚好背景是有目标的那一帧
				//printf("beijinggenxin = %i\n",count);


//--------------------------------目标检测部分--------结束-------------------------------
      imshow("img_click",img);
      waitKey(1);
      //cv::destroyWindow("img_click");
      
		}


	
	}
		//&& detect ==0
		//关闭采集视频流
		fclose(fp);
		do_close();
		//关闭网络套接字
		close(sockClient);

		return(0);	
}




