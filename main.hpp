#ifndef MAIN_HPP
#define MAIN_HPP

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

#endif /* ifndef MAIN_HPP */
