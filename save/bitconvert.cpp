
#include<iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include<opencv2/imgproc/imgproc_c.h>
#include <cv.h>
#include <memory>

using namespace cv;
using namespace std;

int main()
{
	char prefix[] = "/home/nvidia/pic/frame_";
	char postfix[] = ".png";
	char filename[255];
	
	char prefix1[] = "/home/nvidia/pic1/frame_";
	int nr = 512;
	int nc = 640;
	int total = nr*nc; 
	unsigned short transf_fun[16384] = {0};
 	vector<int> compression_params;
        compression_params.push_back(CV_IMWRITE_PNG_COMPRESSION);
        compression_params.push_back(0);    // 无压缩png.

	for(int k = 1; k < 100; k++ )
	{
		sprintf(filename, "%s%d%s", prefix, k, postfix);
		Mat src = imread(filename, IMREAD_ANYDEPTH);

		ushort  *p_1 = NULL; 
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
 
                imshow("Frame", dst_2); 
                cvWaitKey(1); 
		sprintf(filename, "%s%d%s", prefix1, k, postfix);
		imwrite(filename, dst_2, compression_params);
	}
	return 0;
}
