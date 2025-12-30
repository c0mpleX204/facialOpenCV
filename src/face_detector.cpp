#include "face_detector.h"
#include <iostream>
FaceDectector::FaceDetector(int inputW,int inputH,float confThresh)
    :inputW_(inputW),inputH_(inputH),confThresh_(confThresh),loaded_(false)
{}
bool FaceDetector::loadModel(const std::string& path){
    try{
        net_=cv::dnn::readNet(path);
        net_.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
        net_.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);
        loaded_=true;
        std::cout<<"Model loaded from: "<<path<<std::endl;
        return true;
    }catch(const cv::Exception& e){
        std::cerr<<"Error loading model: "<<e.what()<<std::endl;
        return false;
    }
}
std::vector<cv::Rect> FaceDetector::detect(const cv::Mat& frame){
    std::vector<cv::Rect> res;
    if(!loaded_) return res;
    cv::Mat blob;
    cv::Mat input;
    cv::resize(frame.input,cv::Size(inputW_,inputH_));
    cv::dnn::blobFromImage(input,blob,1.0/255.0,cv::Size(inputW_,inputH_),cv::Scalar(),true,false);
    net_.setInput(blob);
    cv::Mat output=net_.forward();
    std::cout<<"net Output dims:"<<output.dims<<", size:";
    for(int i=0;i<output.dims;++i){
        std::cout<<" "<<output.size[i];
    }
    std::cout<<"total ="<<output.total()<<std::endl;

    int toShow=static_cast<int>(std::min<int64_t>(out.total(),20));
    const float* p=reinterpret_cast<const float*>(output.data);
    std::cout<<"first values:";
    for(int i=0;i<toShow;++i){
        std::cout<<" "<<p[i];
    }
    std::cout<<std::endl;

    return res;
}