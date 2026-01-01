#include "face_detector.h"
#include <iostream>
#include <algorithm>
FaceDetector::FaceDetector(int inputW,int inputH,float confThresh)
    :inputW_(inputW),inputH_(inputH),confThresh_(confThresh),loaded_(false)
{}
bool FaceDetector::loadModel(const std::string& modelPath){
    try{
        det_ = cv::FaceDetectorYN::create(modelPath, "", cv::Size(inputW_, inputH_), confThresh_, 0.3f, 5000);
        loaded_ = (det_ != nullptr);
        std::cout << "FaceDetectorYN loaded from: " << modelPath << " ok=" << loaded_ << std::endl;
        return loaded_;
    } catch (const cv::Exception& e) {
        std::cerr << "Error loading model: " << e.what() << std::endl;
        loaded_ = false;
        return false;
    }
}
std::vector<cv::Rect> FaceDetector::detect(const cv::Mat& frame){
    std::vector<cv::Rect> res;
    if(!loaded_) return res;

    cv::Mat input;
    frame.convertTo(input,CV_8UC3);

    cv::Mat input_resized;
    cv::resize(input,input_resized,cv::Size(inputW_,inputH_));

    // ensure detector expects this input size
    det_->setInputSize(cv::Size(inputW_, inputH_));

    // debug: print sizes to ensure we pass the resized image
    // std::cout << "detect(): frame.size()=" << frame.cols << "x" << frame.rows
    //           << " input_resized.size()=" << input_resized.cols << "x" << input_resized.rows
    //           << " expected=" << inputW_ << "x" << inputH_ << std::endl;

    cv::Mat faces;
    det_->detect(input_resized, faces);
    float sx=static_cast<float>(frame.cols)/static_cast<float>(input_resized.cols);
    float sy=static_cast<float>(frame.rows)/static_cast<float>(input_resized.rows);
    for(int i=0;i<faces.rows;++i){
        float x=faces.at<float>(i,0);
        float y=faces.at<float>(i,1);
        float w=faces.at<float>(i,2);
        float h=faces.at<float>(i,3);


        int rx=std::max(0,static_cast<int>(x*sx));
        int ry=std::max(0,static_cast<int>(y*sy));
        int rw=std::min(frame.cols-rx,static_cast<int>(w*sx));
        int rh=std::min(frame.rows-ry,static_cast<int>(h*sy));
        res.emplace_back(rx,ry,rw,rh);
    }
    return res;
}