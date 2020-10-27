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

#define AOI_X 120
#define AOI_Y 128
#define AOI_WIDTH 320
#define AOI_HIGH 256
#define AREA_THRESHOLD  50
struct TargetData{
        unsigned short x;
        unsigned short y;
        int frame_num;
        uint8_t t_h;
        uint8_t t_m;
        uint8_t t_s;
        uint16_t t_ms;
};


int main(){

	int frame_num = 0;
	char prefix[] = "/media/nvidia/Elements SE/太原实验数据/8bits/第六发处理完(6.2km)/frame_";
	char postfix[] = ".png";
	char filename[255];


	Scalar color(0, 0, 255);
	struct timeval start, end;
	for(int k = 4000; k<=4999; k++)
	{
		frame_num++;
		sprintf(filename, "%s%d%s", prefix, k, postfix);	
		Mat src  = imread(filename,IMREAD_GRAYSCALE);//原始图像
		if(!src.data || k <= 3)
		{
			continue;
		}
		
		//目标检测
		
		gettimeofday(&start, NULL);
		Point point = cuda_detection(src, AREA_THRESHOLD);
		gettimeofday(&end, NULL);
		
		printf("time: %fms \n",(double)(end.tv_usec - start.tv_usec)/1000);
		

		if(point.x != 0)
		{
			drawMarker(src,point,color);
		}	

		imshow("Frame", src);
		cvWaitKey(10);		
		
	}
	
	return 0;
}
