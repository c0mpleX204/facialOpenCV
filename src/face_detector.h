#pragma once
#include<opencv2/opencv.hpp>
#include<opencv2/dnn.hpp>
#include<vector>
#include<string>
class FaceDetector{
    public:
        FaceDetector(int inputW=320,int inputH=320,float confThresh=0.5f);
        bool loadModel(const std::string& modelPath);
        std::vector<cv::Rect> detect(const cv::Mat& frame);
    private:
        cv::dnn::Net net;
        int inputW_;
        int inputH_;
        float confThresh_;
        bool loaded_;

};