#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>
#include "face_detector.h"
#include <iostream>
#include <string>
#include <filesystem>

namespace fs = std::filesystem;
void applyBlur(cv::Mat& frame, const cv::Rect& face,int blurSize){
    int k=std::max(1,blurSize|1); // ensure odd

    cv::Mat roi=frame(face);
    cv::GaussianBlur(roi,roi,cv::Size(k,k),0);
}
void applyPixelation(cv::Mat& frame, const cv::Rect& face, int pixelSize){
    int p=std::max(1,pixelSize);
    cv::Mat roi=frame(face);
    cv::Mat small;
    cv::resize(roi,small,cv::Size(roi.cols/p,roi.rows/p),0,0,cv::INTER_LINEAR);
    cv::resize(small,roi,roi.size(),0,0,cv::INTER_NEAREST);
}
void applyMask(cv::Mat& frame,const cv::Rect& face,const cv::Mat& maskImage){
    if(maskImage.empty()) return;
    cv::Mat resizedMask;
    cv::resize(maskImage,resizedMask,face.size());
    cv::Mat roi =frame(face);
    if(resizedMask.channels()==4){
        std::vector<cv::Mat> channels;
        cv::split(resizedMask,channels);
        cv::Mat alpha=channels[3];

        std::vector<cv::Mat> bgrChannels={channels[0],channels[1],channels[2]};
        cv::Mat bgrMask;
        cv::merge(bgrChannels,bgrMask);
        bgrMask.copyTo(roi,alpha);

    }
    else{
        resizedMask.copyTo(roi);
    }
}
int main(int argc, char** argv) {
    bool enablePrivacy = false;
    std::string mode = "blur";
    int blur_size = 25;
    int pixel_size = 16;
    std::string mask_image;
    int device = 0;

    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        if (a == "-mode" && i + 1 < argc) mode = argv[++i];
        else if (a == "-blur_size" && i + 1 < argc) blur_size = std::stoi(argv[++i]);
        else if (a == "-pixel_size" && i + 1 < argc) pixel_size = std::stoi(argv[++i]);
        else if (a == "-mask_image" && i + 1 < argc) mask_image = argv[++i];
        else if (a == "-device" && i + 1 < argc) device = std::stoi(argv[++i]);
    }

    cv::VideoCapture cap;
    // Open requested device using DirectShow on Windows; fall back to default if needed
    cap.open(device, cv::CAP_DSHOW);
    if (!cap.isOpened()) {
        cap.open(device);
    }
    if (!cap.isOpened()) {
        std::cerr << "Cannot open camera device: " << device << ". Check permissions and index." << std::endl;
        return -1;
    }

    // Try to load YuNet model from models/ (search current working dir and executable parents)
    std::string relModel = "models/face_detection_yunet_2023mar.onnx";
    FaceDetector detector(320, 320, 0.5f);
    bool hasModel = false;

    if(fs::exists(relModel)){
        hasModel = detector.loadModel(relModel);
    }else{
        fs::path exePath =fs::absolute(argv[0]);
        fs::path cur = exePath.parent_path();
        for(int i=0;i<6 && !cur.empty();++i){
            fs::path candidate =cur /"models" / "face_detection_yunet_2023mar.onnx";
            if(fs::exists(candidate)){
                hasModel =detector.loadModel(candidate.string());
                if(hasModel){
                    std::cout<<"Loaded model from: "<<candidate.string()<<std::endl;
                }
                break;
            }
            cur=cur.parent_path();
        }
        if(!hasModel){
            std::cout<<"Model not found at '"<<relModel<<"' or in parent dirs. Running without detection."<<std::endl;
        }
    }
    
    std::string mask_image_default="image/qianzaoaiyin.png";
    cv::Mat maskImg=cv::imread(mask_image.empty()?mask_image_default:mask_image,cv::IMREAD_UNCHANGED);
    cv::Mat frame;
    for(;;){
        cap>>frame;
        if(frame.empty()) break;
        if(hasModel){
            auto boxes=detector.detect(frame);
            for(const auto &r:boxes){
                cv::Rect faceRect = r & cv::Rect(0, 0, frame.cols, frame.rows);
                if(faceRect.area() <=0) continue;
                if(enablePrivacy){
                    if(mode=="blur"){
                        applyBlur(frame, faceRect, blur_size);
                    }else if(mode=="pixelate"){
                        applyPixelation(frame, faceRect, pixel_size);
                    }else if(mode=="mask"){
                        applyMask(frame, faceRect, maskImg);
                        
                    }
                }
                else{cv::rectangle(frame,r,cv::Scalar(0,255,0),2);}
            }
            std::string statusText=enablePrivacy?("Privacy ON(" +mode+ ")"):"Privacy OFF";
            cv::Scalar statusColor =enablePrivacy?cv::Scalar(0,0,255):cv::Scalar(0,255,0);
            cv::putText(frame,statusText,cv::Point(10,30),cv::FONT_HERSHEY_SIMPLEX,0.7,statusColor,2);

            std::string paramText="";
            if(enablePrivacy){
                if(mode=="blur"){
                    paramText="Blur Size: " + std::to_string(blur_size);
                }else if(mode=="pixelate"){
                    paramText="Pixel Size: " + std::to_string(pixel_size);
                }else if(mode=="mask"){
                    paramText="Mask Image: " + mask_image;
                }
                cv::putText(frame,paramText,cv::Point(10,60),cv::FONT_HERSHEY_SIMPLEX,0.6,cv::Scalar(255,255,0),1);
            }
        }else{
            cv::putText(frame,"Model missing",cv::Point(10,30),cv::FONT_HERSHEY_SIMPLEX,0.7,cv::Scalar(0,0,255),2);
        }

        std::string info ="[Q]Toggle Privacy|[1]Blur [2]Pixel [3]Mask | [ [ / ] ] Adjust Blur/Pixel: " ;
        cv::putText(frame,info,cv::Point(10,frame.rows-10),cv::FONT_HERSHEY_SIMPLEX,0.6,cv::Scalar(255,255,255),1);

        cv::imshow("Privacy Protector",frame);
        int key=cv::waitKey(1);
        if(key==27) break; // ESC to quit

        if(key=='q' || key=='Q'){
            enablePrivacy = !enablePrivacy;
        }
        if(key=='1') mode="blur";
        if(key=='2') mode="pixelate";
        if(key=='3') mode="mask";
        if(key=='[' && blur_size>1){
            if(mode=="blur")blur_size=std::max(1,blur_size-4);
            if(mode=="pixelate")pixel_size=std::max(1,pixel_size-2);
        }
        if(key==']'){
            if(mode=="blur")blur_size+=4;
            if(mode=="pixelate")pixel_size+=2;
        }
        
        if(key=='u'||key=='U'){
            std::string path;
            std::cout<<"Enter mask image path: ";
            std::cin>>path;
            cv::Mat temp=cv::imread(path,cv::IMREAD_UNCHANGED);
            if(!temp.empty()){
                maskImg=temp;
                mask_image=path;
                std::cout<<"Image loaded successfully"<<std::endl;
            }else{
                std::cout<<"Failed to load image: "<<path<<std::endl;
            }
        }
        
        if(!temp.empty()){
            maskImg=temp;
            std::cout<<"Image loaded successfully"<<std::endl;
        }else{
            std::cout<<"Failed to load image: "<<path<<std::endl;
        }
    }


    return 0;
}
