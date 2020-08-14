#include <stdio.h>		// c library
#include <signal.h>		// c library
#include <stdlib.h>		// c library
#include <stdarg.h>		// c library
#include <unistd.h>		// c library
#include <limits.h>		// c library

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
#include <queue>
#include<pthread.h>
#include "main.hpp"
using namespace cv;
using namespace std;

int AREA_THRESHOLD = 150;


int main(){

	int detect = 0;
	unsigned short x1 = 0;
	unsigned short y1 = 0;
	int update_flag = 0;
	clock_t start_1, end_1;
	clock_t start_2, end_2;
	char prefix[] = "/home/nvidia/Desktop/multi-threading/test/pic13/frame_000000";
	char postfix[] = ".png";
	char filename[255];
	for(int k = 1; k<=5000; k++)
	{
		start_1 = clock();
		sprintf(filename, "%s%04d%s", prefix, k, postfix);	
		Mat src  = imread(filename,IMREAD_ANYDEPTH);//原始图像
	//---------------------------16bits 图像直方图均衡化----------------------------------//
		int nr = src.rows;//256
		int nc = src.cols;//320
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
		Mat dst_3(dst_2, Rect(120, 128, 320, 256));
		imshow("Window", dst_3);
		cvWaitKey(1);
		Mat image_pro = dst_2.clone();
		
		end_1 = clock();
		printf("Image Enhance = %fs-------------------------------------------------------\n", double(end_1 - start_1)/CLOCKS_PER_SEC);

		printf("**********************************************");	

		

		start_2 = clock();
		Mat img_back(512, 640, CV_8UC1);

		//背景初始化
		if(update_flag == 0)
		{
			img_back = image_pro.clone();
			printf("\n--------------------------------------背景初始化-----------------------------\n");
			update_flag = 1;
		}

		
		//目标检测
		vector<DetectInfo> detect_infos = detection(img_back, image_pro, AREA_THRESHOLD);
		if(detect_infos.size())
		{

			x1 = 128 + (unsigned short)detect_infos[0].x;
			y1 = 160 + (unsigned short)detect_infos[0].y;
			printf("Target : [ %d , %d ]\n", x1, y1);
			detect = 1;
		}
		else
		{
			detect = 0;
		}

		//背景更新
		if ( k % 50 == 0 && detect == 0)
		{
			img_back = image_pro.clone();
			printf("---------------------------------背景更新----------------------------------\n");
		}
		
		end_2 = clock();
		printf("--------------------Image Process = %fs-----------------------------------\n", double(end_2 - start_2)/CLOCKS_PER_SEC);
		
	}
return 0;
}
