#if !defined(FORMAT) && !defined(FORMATFILE)
// For PIXCI(R) SV2, SV3, SV4, SV5, SV5B, SV5L, SV6
//#define FORMAT  "RS-170"	  // RS-170 on input 2
//#define FORMAT  "NTSC"	  // NTSC on input 2
//#define FORMAT  "NTSC/YC"	  // NSTC S-Video on input 1		(N/A on SV5A,SV5B)
//#define FORMAT  "CCIR"	  // CCIR on input 2
//#define FORMAT  "PAL"	  // PAL (B,D,G,H,I) on input 2
//#define FORMAT  "PAL/YC"	  // PAL (B,D,G,H,I) S-Video on input 1 (N/A on SV5A,SV5B)
//  #define FORMAT  "default"	  // NSTC S-Video on input 1

				// For PIXCI(R) SV7
//#define FORMAT  "RS-170"	  // RS-170
//#define FORMAT  "NTSC"	  // NTSC
//#define FORMAT  "CCIR"	  // CCIR
//#define FORMAT  "PAL"	  // PAL
//#define FORMAT  "default"	  // NSTC

				// For PIXCI(R) SV8
//#define FORMAT  "RS-170"	  // RS-170 on BNC 0
//#define FORMAT  "NTSC"	  // NTSC on BNC 0
//#define FORMAT  "NTSC/YC"	  // NSTC S-Video
//#define FORMAT  "CCIR"	  // CCIR on BNC 0
//#define FORMAT  "PAL"	  // PAL on BNC 0
//#define FORMAT  "PAL/YC"	  // PAL (B,D,G,H,I) S-Video
//#define FORMAT  "default"	  // NSTC on BNC 0

				// For PIXCI(R) A, CL1, CL2, CL3SD, D, D24, D32,
				// D2X, D3X, D3XE, E1, E1DB, E4, E4DB, E8, E8CAM, E8DB, e104x4,
				// EB1, EB1-POCL, EB1mini, EC1, ECB1, ECB1-34, ECB2, EL1, EL1DB,
				// ELS2, SI, SI1, SI2, SI4
//#define FORMAT  "default"	  // as per board's intended camera

					// For any PIXCI(R) frame grabber
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

#include<sys/socket.h>
#include<netinet/in.h>

#include<iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include<opencv2/imgproc/imgproc_c.h>
#include <cv.h>



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

//---------------------------目标信息----------------------------yyf
struct DetectInfo {
    /* 质心(x,y) */
    double x,y;
    /* 方位角 */
    double r,c;
    /* 目标面积 */
    int area;

    friend bool operator>(const DetectInfo &d1, const DetectInfo &d2) {
        return d1.area > d2.area;
    }
};

/* 目标检测函数--------------------------------------------------yyf
 * background: 背景图像
 * img: 待检测图像
 * area_threshold: 面积阈值
 * 输出目标信息数组
 */
 
vector<DetectInfo> detection(Mat background, Mat img, int area_threshold = 80) {
    Mat imgFront,imglabel, stats, centroids;
    Mat element = getStructuringElement(MORPH_ELLIPSE, Size(3, 3));
    absdiff(img, background, imgFront);
    threshold(imgFront, imgFront, 70, 255, CV_THRESH_BINARY);
    morphologyEx(imgFront, imgFront, CV_MOP_OPEN, element); //消除孤立的点
    int count = connectedComponentsWithStats(imgFront, imglabel, stats, centroids, 8);
    vector<DetectInfo> detectinfos(count);
    for(int i = 1; i != count; ++i){
        auto &info = detectinfos[i];
        info.area = stats.at<int>(i, CC_STAT_AREA);
        info.x = centroids.at<double>(i, 0);
        info.y = centroids.at<double>(i, 1);
    }
    sort(detectinfos.begin(), detectinfos.end(), std::greater<DetectInfo>());
    for (int i = 0; i < count; ++i) {
        if (detectinfos[i].area < area_threshold) {
            detectinfos.resize(i);
            break;
        }
    }
    return detectinfos;
}


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

struct SendInfo {
        uint8_t flag1;
        uint8_t flag2;
        uint8_t t_h;
        uint8_t t_min;
        uint8_t t_s;
        uint8_t t_ms;
        uint8_t x1;
        uint8_t y1;          
} __attribute__((packed));

//-----------------------------UDP传输目标信息---------------------
void Net_Send_new(int sockClient, struct sockaddr_in addrSrv, const SendInfo *data_pack)
{
/*
	char *sendBuf;
	sendBuf = (char*)malloc(11);//后加10字节用于传输时间、目标坐标及方位角
	memset(sendBuf, 0, 11);
*/

	char flag1 = 0xAB;//start 1
	char flag2 = 0x00;//标志位 有无检测到目标
	if (x1 != 0)
	{
		flag2 = 0x01;
	}
	unsigned size = sizeof(data_pack);
/*
	memcpy(sendBuf, &flag1, sizeof(char));
	memcpy(sendBuf + 1, &flag2, sizeof(unsigned char));
	memcpy(sendBuf + 2, &t_h, sizeof(unsigned char));
	memcpy(sendBuf + 3, &t_min, sizeof(unsigned char));
	memcpy(sendBuf + 4, &t_s, sizeof(unsigned char));
	memcpy(sendBuf + 5, &t_ms, sizeof(unsigned short));
	memcpy(sendBuf + 7, &x1, sizeof(unsigned short));
	memcpy(sendBuf + 9, &y1, sizeof(unsigned short));
*/

	int set = sendto(sockClient, &data_pack, 12, 0, (struct sockaddr*)&addrSrv, sizeof(struct sockaddr));
	/*    if( i%50 == 0)
		{
			usleep(0.001);
		}*/
}

//----------------------------------保存图片----------------------
//static void *save_pic(void *data)
//{
//
//	int frmNum = 0;
//	char _path[255];
//	char prefix[] = "/home/nvidia/Desktop/histeq_detect_udp/pic/";
//	char postfix[] = ".png";
//	int row = 512;
//	int col = 640;
//	Mat img2 = Mat(row, col, CV_8UC1);//图像img2：row*col大小  这里只是定义了img2图像的大小还没有传递图像的信息
//	frmNum = ptr[512 * col + 0] * 1000 + ptr[512 * col + 1] * 10 + ptr[512 * col + 3];
//	uchar *ptmp = NULL;
//	for (i = 0; i < row; i++)
//	{
//		ptmp = img2.ptr<uchar>(i);//指针指向img2的第i行
//		for (j = 0; j < col; j++)
//		{
//			ptmp[j] = ptr[i*col + j];
//		}
//	}
//	memset(_path, '\0', sizeof(char) * 255);
//	sprintf(_path, "%sframe_%010d%s", prefix, frmNum, postfix);
//	vector<int> compression_params;
//	compression_params.push_back(CV_IMWRITE_PNG_COMPRESSION);
//	compression_params.push_back(0);
//	imwrite(_path, img2, compression_params);
//}



static void* save_pic(void *data)
{
	//info param = data;
        // Param* param = data;
	//Mat src = data->message;
	//int frmNum = 0;
	count_nums += 1;
        uchar *img_con;
        img_con = (uchar*)data;
        Mat src(512, 640, CV_8UC1); 
	char _path[255];

	char prefix[] = "/home/nvidia/Desktop/histeq_detect_udp/pic/";
	char postfix[] = ".png";
        time_t start, end;
        start = clock();
	//frmNum++;
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
		//frmNum = 0;
	}
        printf("保存帧号%d\n", count_nums);

		end = clock();
		double endtime = (double)(end - start) / CLOCKS_PER_SEC;
		printf("保存图像耗时线程 = %f\n", endtime);
                pthread_detach(pthread_self());
        //pthread_exit((void*)pt);
      // break;
}
//void createThread(Mat img)
// {
//printf("11111111");
	//info *param = (info*)malloc(sizeof(info));
//        info *param
	//param->message = img;
	//pthread_t pt;
//printf("22222222");
	//pthread_create(&pt, NULL, &save_pic, (void*)param);
//}
//------------------------保存图片结束-----------------------------



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
	//start = clock();
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

	time_t start, end;
	time_t start_total, end_total;
        time_t start_p, end_p;
        time_t start_100, end_100;
       int num_100 = 0;
	//  start = clock();

	int frmNum = 0;
	char _path[255];

	char prefix[] = "/home/nvidia/Desktop/histeq_detect_udp/pic/";
	char postfix[] = ".png";
	//pxd_goneLive函数源源不断的捕获图像，手册有介绍
        start = clock();

        double all_time = 0;
	while (pxd_goneLive(UNITSMAP, 0))//capture picture
	{
                
                
                start_100 = clock();
		end = clock();
		double endtime = (double)(end - start) / CLOCKS_PER_SEC;
		printf("捕获图像时间 = %f\n", endtime);

		start = clock();
		start_total = clock();

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
        start_p = clock();

		end = clock();
		endtime = (double)(end - start) / CLOCKS_PER_SEC;
		printf("直方图均衡化耗时 = %f\n", endtime);
                start = clock();

	end_p = clock();
	double endtime_p = (double)(end_p - start_p) / CLOCKS_PER_SEC;
	printf("time printf = %f\n", endtime_p);
		//----------------------------------保存图像-------------------------

  //      pthread_t pt;


//	        pthread_t pt0,pt1,pt2,pt3,pt4;
//     int pt_num = count_nums % 5 ;
//        switch(pt_num)
//        {
  //           case 0: pthread_create(&pt0, NULL, &save_pic, (void*)img_con);
    //         case 1: pthread_create(&pt0, NULL, &save_pic, (void*)img_con);
      //       case 2: pthread_create(&pt0, NULL, &save_pic, (void*)img_con);
        //     case 3: pthread_create(&pt0, NULL, &save_pic, (void*)img_con);
        //     case 4: pthread_create(&pt0, NULL, &save_pic, (void*)img_con);
      // }    
             
//   pthread_create(&pt, NULL, &save_pic, (void*)img_con);


		printf("receiveandwritecount_nums=%d\n", count_nums);

		end = clock();
		endtime = (double)(end - start) / CLOCKS_PER_SEC;
		printf("保存图像耗时 = %f\n", endtime);

 start = clock();

		//--------------------------------保存图像结束------------------------

		//-------------------------------十字线叠加------------------------
		for (int i = -25; i < 25; i++)
		{
			img.at<uchar>(AOI_YDIM / 2 + i, AOI_XDIM / 2) = 255;
			img.at<uchar>(AOI_YDIM / 2, AOI_XDIM / 2 + i) = 255;

		}
		

                end = clock();
		endtime = (double)(end - start) / CLOCKS_PER_SEC;
		printf("十字线叠加 = %f\n", endtime);

		//cvWaitKey(1);
		//src = cvLoadImage("lena.jpg", 1);
start =clock();



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
		end = clock();
		endtime = (double)(end - start) / CLOCKS_PER_SEC;
		printf("显示图像= %f\n", endtime);

                start = clock();

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
                end = clock();
		endtime = (double)(end - start) / CLOCKS_PER_SEC;
		printf("抠图= %f\n", endtime);
                start = clock();
                
                
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
			        auto detect_infos = detection(imageBackground, img_windows, AREA_THRESHOLD);//目标检测---------------------yyf
			        if(detect_infos.size()){
			                x1 = (unsigned short)detect_infos[0].x;
			                y1 = (unsigned short)detect_infos[0].y;
			                area_max = detect_infos[0].area;
			                detect = 1;
			        }
			        
/*
				detect = 0;
				absdiff(img_windows, imageBackground, imageFront);//与背景做差  
		end = clock();
		endtime = (double)(end - start) / CLOCKS_PER_SEC;
		printf("与背景做差= %f\n", endtime);

                start = clock();

//				threshold(imageFront, imageFront, 100, 255, CV_THRESH_BINARY);  //阈值分割 
				threshold(imageFront, imageFront, 0, 255, CV_THRESH_OTSU);  //阈值分割  
		end = clock();
		endtime = (double)(end - start) / CLOCKS_PER_SEC;
		printf("阈值分割= %f\n", endtime);

                start = clock();

				morphologyEx(imageFront, imageFront, CV_MOP_OPEN, element); //消除孤立的点

			//	imshow("image_med",image_med);
			//	imshow("imageFront",imageFront);
			//	cvWaitKey(1);
		end = clock();
		endtime = (double)(end - start) / CLOCKS_PER_SEC;
		printf("消除孤立的点= %f\n", endtime);

                start = clock();

				findContours(imageFront, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE, Point());
				Mat imageContours = Mat::zeros(img_windows.size(), CV_8UC1);
				Mat Contours = Mat::zeros(img_windows.size(), CV_8UC1);  //绘制
				for (int i = 0; i < contours.size(); i++)
				{
					//绘制轮廓
					drawContours(imageContours, contours, i, Scalar(255), 1, 8, hierarchy);
				}
		end = clock();
		endtime = (double)(end - start) / CLOCKS_PER_SEC;
		printf("绘制轮廓= %f\n", endtime);

                start = clock();
				//计算轮廓矩
				vector<Moments> mu(contours.size());
				for (int i = 0; i < contours.size(); i++)
				{
					mu[i] = moments(contours[i], false);
				}
		end = clock();
		endtime = (double)(end - start) / CLOCKS_PER_SEC;
		printf("计算轮廓矩= %f\n", endtime);

                start = clock();
				//计算轮廓的质心
				vector<Point2f> mc(contours.size());

				end = clock();
				endtime = (double)(end - start) / CLOCKS_PER_SEC;
				printf("计算轮廓质心时间 = %f\n", endtime);

				start = clock();
				for (int i = 0; i < contours.size(); i++)
				{
					double g_dConArea = fabs(contourArea(contours[i], true));//计算轮廓的面积
					//cout << "【用轮廓面积计算函数计算出来的第" << i << "个轮廓的面积为：】" << g_dConArea << endl;
					mc[i] = Point2f(mu[i].m10 / mu[i].m00, mu[i].m01 / mu[i].m00);

					if (g_dConArea > 8 && g_dConArea > area_max)
					{
						//printf("%.0f",mc[i].x);
						//printf(",%.0f\n",mc[i].y);
						area_max = g_dConArea;
						x1 = mc[i].x;
						y1 = mc[i].y;
						detect = 1;

						//float shuiping = (x1 - 320)*1.5 / 320;
						//float fuyang = (y1 - 256)*1.2 / 256;

					}
				}
				end = clock();
				endtime = (double)(end - start) / CLOCKS_PER_SEC;
				printf("计算轮廓面积 = %f\n", endtime);



				//x1 = 128;
				//y1 = 256;

				printf("x1=%d\n", x1);
				printf("目标面积=%d\n", area_max);
				printf("yisi目标shumu=%d\n", contours.size());
				//start = clock();
*/
			}
                        start = clock();


                        x1 = x_offset + x1;
                        y1 = y_offset + y1;
                        printf("x1 = %d\n", x1);
                        printf("y1 = %d\n", y1);
                        printf("area = %d\n", area_max);

                        
			Net_Send_new(sockClient, addrSrv, t_h, t_min, t_s, t_ms, x1, y1);
			end = clock();
			double endtime = (double)(end - start) / CLOCKS_PER_SEC;
			printf("UDP传输时间= %f\n", endtime);

			num_upgrade += 1;
		        printf("更新 = %i\n", num_upgrade);
			printf("detect = %i\n", detect);


			if (num_upgrade >= 50 && detect == 0)//有目标不更新------------------------yyf
			{
				Mat imageBackground = dst_2.clone();
				num_upgrade = 0;
				printf("背景更新 = %i\n", num_upgrade);
			}//背景更新 放在后面，预防出现刚好背景是有目标的那一帧
				//printf("beijinggenxin = %i\n",count);



			//printf("----------------------------------------------------------------");
			printf("\n", endtime);
			printf("\n", endtime);
			printf("\n", endtime);

         end_100 = clock();
         endtime = (double)(end_100 -start_100) / CLOCKS_PER_SEC;
         printf("当前帧检测时间%f\n\n",endtime);
         all_time  += endtime;



         double average_time = all_time/num_100;
 	 printf("平均每帧时间 = %f\n", average_time);

             printf("帧数%d\n", num_100);
             printf("总时间%f\n\n",all_time);	
             printf("-----------------------------------\n\n");

//--------------------------------目标检测部分--------结束-------------------------------
      imshow("img_click",img);
      waitKey(1);
      //cv::destroyWindow("img_click");
      
		}


			end_total = clock();
			endtime = (double)(end_total - start_total) / CLOCKS_PER_SEC;
			printf("当前帧帧总时间 = %f\n", endtime);
                        printf("-----------------------------结束------------------\n\n");
	
	}
		//&& detect ==0
		//关闭采集视频流
		fclose(fp);
		do_close();
		//关闭网络套接字
		close(sockClient);

		return(0);	
}




