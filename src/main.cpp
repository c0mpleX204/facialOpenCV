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
    
    cv::Mat frame;
    for(;;){
        cap>>frame;
        if(frame.empty()) break;
        if(hasModel){
            auto boxes=detector.detect(frame);
            for(const auto &r:boxes){
                cv::Rect faceRect = r & cv::Rect(0, 0, frame.cols, frame.rows);
                if(validRect.area() <=0) continue;
                if(enablePrivacy){
                    if(mode=="blur"){
                        applyBlur()(frame, faceRect, blur_size);
                    }
                }
                else{cv::rectangle(frame,r,cv::Scalar(0,255,0),2);}
            }
            std::string statusText=enablePrivacy?("Privacy ON(" +mode+ ")"):"Privacy OFF";
            cv::Scalar statusColor =enablePrivacy?cv::Scalar(0,0,255):cv::Scalar(0,255,0);
            cv::putText(frame,statusText,cv::Point(10,30),cv::FONT_HERSHEY_SIMPLEX,0.7,statusColor,2);
        }else{
            cv::putText(frame,"Model missing",cv::Point(10,30),cv::FONT_HERSHEY_SIMPLEX,0.7,cv::Scalar(0,0,255),2);
        }


        std::string info ="[Q]Toggle Privacy | [ [ / ] ] Adjust Blur: " + std::to_string(blur_size);
        cv::putText(frame,info,cv::Point(10,frame.rows-10),cv::FONT_HERSHEY_SIMPLEX,0.6,cv::Scalar(255,255,255),1);

        cv::imshow("Privacy Protector",frame);
        int key=cv::waitKey(1);
        if(key==27) break; // ESC to quit

        if(key=='q' || key=='Q'){
            enablePrivacy = !enablePrivacy;
    }
        if(key=='[' && blur_size>1){
            blur_size=std::max(1,blur_size-4);
        }
        if(key==']'){
            blur_size+=4;
        }
    }


    return 0;
}
