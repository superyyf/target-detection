#include "cuda_runtime.h"
#include "device_launch_parameters.h"

#include <stdio.h>

#include <stdio.h>
#include <iostream>  
#include <cmath>
#include <math.h>
#include <iomanip>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include "device_functions.h"
#include <cuda.h> 
#include"device_atomic_functions.h"

using namespace std;
using namespace cv;


//block:16*16个thread
//grid维度根据图像大小计算可得：(width+TILE_WIDTH-1)/ TILE_WIDTH, (height+TILE_WIDTH-1)/ TILE_WIDTH）

#define TILE_WIDTH 16//thread的宽度

//_Calhistogramkernel核函数，计算图像的直方图
__global__ void _Calhistogramkernel(unsigned char* input_data, unsigned int*histOrg,
    unsigned int step, unsigned int width, unsigned int height)
{
    /* __shared__ unsigned int temp[256]; */
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;
    unsigned int value = 0;
    if (x<width && y<height)
    {
        value = input_data[y*step + x];
        atomicAdd(&(histOrg[value]), 1);
    }
    __syncthreads();
    /*  if(value<256)
    atomicAdd(&histOrg[value], temp[value]); */

}
//调用_Calhistogramkernel核函数的主机端函数     
void Calhistogram(unsigned int *p_hist, unsigned char *srcImagedata,
    unsigned int step, unsigned int width, unsigned int height){


    //只将图像的data数据作为参数传递给device
    unsigned char *devicechar;
    cudaMalloc((void **)(&devicechar), width*height*sizeof (unsigned char));

    cudaMemcpy(devicechar, srcImagedata, width*height*sizeof(unsigned char), cudaMemcpyHostToDevice);

    // 计算调用核函数的线程块的尺寸和线程块的数量。
    dim3 dimBlock(TILE_WIDTH, TILE_WIDTH);                  //线程块的维度   
    dim3 dimGrid((width + TILE_WIDTH - 1) / TILE_WIDTH, (height + TILE_WIDTH - 1) / TILE_WIDTH); //线程网格的维度

    // 在 Device 端为直方图申请一段空间
    unsigned int *devhisto;
    cudaMalloc((void**)&devhisto, 256 * sizeof (unsigned int));

    //初始化设备端的直方图数组
    cudaMemset(devhisto, 0, 256 * sizeof (unsigned int));

    // 调用核函数，计算输入图像的直方图。
    _Calhistogramkernel << < dimGrid, dimBlock >> >(devicechar, devhisto, step, width, height);  //调用内核函数

    // 将直方图的结果拷回 Host 端内存中。
    cudaMemcpy(p_hist, devhisto, 256 * sizeof (unsigned int), cudaMemcpyDeviceToHost);

    cudaFree(devicechar);
    cudaFree(devhisto);

}
//_calequhistker计算变换后灰度值
__global__ void _Calequhistker(unsigned int *devequhist, float *devicecdfhist, unsigned int size){

    __shared__ unsigned int sharedequhist[256];

    int Id = threadIdx.x;
    sharedequhist[Id] = (unsigned int)(255.0*devicecdfhist[Id] + 0.5);

    __syncthreads();

    devequhist[Id] = sharedequhist[Id];

}
//计算变换后灰度值的主机端函数，调用_calequhistker
void Calequhist(unsigned int *equhist, float *cdfhist, unsigned int size){

    float *devicecdfhist;
    cudaMalloc((void **)(&devicecdfhist), 256 * sizeof (float));
    cudaMemcpy(devicecdfhist, cdfhist, 256 * sizeof (float), cudaMemcpyHostToDevice);

    unsigned int *devequhist;
    cudaMalloc((void**)&devequhist, 256 * sizeof (unsigned int));
    //初始化设备端的直方图数组
    cudaMemset(devequhist, 0, 256 * sizeof (unsigned int));

    _Calequhistker << <1, 256 >> >(devequhist, devicecdfhist, size);

    cudaMemcpy(equhist, devequhist, 256 * sizeof (unsigned int), cudaMemcpyDeviceToHost);

    cudaFree(devicecdfhist);
    cudaFree(devequhist);

}
//_MapImagekernel：由灰度映射关系计算输出图像的核函数
__global__ void _MapImagekernel(unsigned char* devicesrcdata, unsigned char* devicedstdata,
    unsigned int* devhistequ, unsigned int step, unsigned int width, unsigned int height)
{
    /* __shared__ unsigned int temp[256]; */
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;
    unsigned int value = 0;
    if (x<width && y<height)
    {
        value = devicedstdata[y*step + x];
        devicesrcdata[y*step + x] = devhistequ[value];
    }
    __syncthreads();
    /*  if(value<256)
    atomicAdd(&histOrg[value], temp[value]); */

}
//计算输出图像的主机端函数：调用_MapImagekernel 
void MapImage(unsigned char *dstdata, unsigned char *srcdata, unsigned int *p_histequ,
    unsigned int step, unsigned int width, unsigned int height){

    //将图像的data数据作为参数传递给device
    unsigned char *devicesrcdata, *devicedstdata;
    cudaMalloc((void **)(&devicesrcdata), width*height*sizeof (unsigned char));
    cudaMalloc((void **)(&devicedstdata), width*height*sizeof (unsigned char));
    cudaMemcpy(devicesrcdata, srcdata, width*height*sizeof(unsigned char), cudaMemcpyHostToDevice);
    cudaMemcpy(devicedstdata, srcdata, width*height*sizeof(unsigned char), cudaMemcpyHostToDevice);
    // 在 Device 端为均衡直方图申请一段空间
    unsigned int *devhistequ;
    cudaMalloc((void**)&devhistequ, 256 * sizeof (unsigned int));
    cudaMemcpy(devhistequ, p_histequ, 256 * sizeof(unsigned int), cudaMemcpyHostToDevice);

    // 计算调用核函数的线程块的尺寸和线程块的数量。
    dim3 dimBlock(TILE_WIDTH, TILE_WIDTH);                  //线程块的维度   
    dim3 dimGrid((width + TILE_WIDTH - 1) / TILE_WIDTH, (height + TILE_WIDTH - 1) / TILE_WIDTH); //线程网格的维度
    // 调用核函数，计算输入图像的直方图。
    _MapImagekernel << < dimGrid, dimBlock >> >(devicedstdata, devicesrcdata, devhistequ, step, width, height);  //调用内核函数

    // 将直方图的结果拷回 Host 端内存中。
    cudaMemcpy(dstdata, devicedstdata, width*height*sizeof(unsigned char), cudaMemcpyDeviceToHost);
    //释放内存
    cudaFree(devicesrcdata);
    cudaFree(devicedstdata);
    cudaFree(devhistequ);

}
//主函数
int main(int argc, char **argv)
{
    // Read images using OpenCV
    unsigned int i = 0, step = 0;
    cv::Mat srcImage = cv::imread("picture/512x510.jpg", 1);//图片路径
    unsigned int height = srcImage.rows;
    unsigned int width = srcImage.cols;
    unsigned int size = width*height;
    //std::cout << "Image size = (" << width << "," << height << ")" << std::endl;
    // 灰度化处理
    cv::cvtColor(srcImage, srcImage, CV_BGR2GRAY);
    cv::imshow( "原始图", srcImage );
    cv::Mat dstImage1 = srcImage;
    unsigned char *hostdata = srcImage.data;
    unsigned char *dstdata = dstImage1.data;

    if (width % 4 == 0)
        step = width;
    else
        step = (width / 4) * 4 + 4;
    printf("step=%d\n", step);


    cudaEvent_t start, stop;
    float time;
    cudaEventCreate(&start);
    cudaEventCreate(&stop);
    cudaEventRecord(start, 0);

    //定义host端的直方图数组
    unsigned int hosthist[256] = { 0 };
    unsigned int *p_hist = hosthist;
    Calhistogram(p_hist, hostdata, step, width, height);

    /* printf("直方图统计信息：\n");
    for(i=0;i<256;i++)
    printf("%d\t",hosthist[i]);
    printf("\n"); */

    //归一化直方图（串行）  
    float histPDF[256] = { 0 };
    for (i = 0; i<255; i++)
    {
        histPDF[i] = (float)hosthist[i] / size;
    }

    //累积直方图 （串行） 
    float histCDF[256] = { 0 };
    for (i = 0; i<256; i++)
    {
        if (0 == i) histCDF[i] = histPDF[i];
        else histCDF[i] = histCDF[i - 1] + histPDF[i];
    }


    //直方图均衡化(并行)
    unsigned int histEQU[256] = { 0 };

    unsigned int *p_histequ = histEQU;
    Calequhist(p_histequ, histCDF, size);

    //映射(并行)

    MapImage(dstdata, hostdata, p_histequ, step, width, height);

    cudaEventRecord(stop, 0);
    cudaEventSynchronize(stop);
    cudaEventElapsedTime(&time, start, stop);
    cudaEventDestroy(start);
    cudaEventDestroy(stop);

    printf("The time of (%d,%d) GPU Image histogram equalization is :%fms\n", width, height, time);

    // 【4】显示结果
    cv::imwrite("picture/cuda512x510.jpg", dstImage1);
    cv::imshow("picture/cuda512x510.jpg", dstImage1);
    waitKey(0);
    return 0;

}
