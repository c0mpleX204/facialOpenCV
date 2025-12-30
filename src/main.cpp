#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>
#include "face_detector.h"
#include <iostream>
#include <string>
#include <filesystem>

namespace fs = std::filesystem;

int main(int argc, char** argv) {
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
    cv::dnn::Net net;
    bool hasModel = false;
    try {
        // 1) check current working directory
        if (fs::exists(relModel)) {
            net = cv::dnn::readNet(relModel);
            hasModel = true;
            std::cout << "Loaded model from cwd: " << relModel << std::endl;
        } else {
            // 2) search upward from executable path for a models/ folder
            fs::path exePath = fs::absolute(argv[0]);
            fs::path cur = exePath.parent_path();
            bool found = false;
            for (int i = 0; i < 6 && !cur.empty(); ++i) {
                fs::path candidate = cur / "models" / "face_detection_yunet_2023mar.onnx";
                if (fs::exists(candidate)) {
                    net = cv::dnn::readNet(candidate.string());
                    hasModel = true;
                    found = true;
                    std::cout << "Loaded model from: " << candidate.string() << std::endl;
                    break;
                }
                cur = cur.parent_path();
            }
            if (!found) {
                std::cout << "Model not found at '" << relModel << "' or in parent dirs. Running without detection." << std::endl;
            }
        }
        if (hasModel) {
            net.setPreferableBackend(cv::dnn::DNN_BACKEND_DEFAULT);
            net.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);
        }
    } catch (const std::exception& e) {
        std::cerr << "Failed to load model: " << e.what() << std::endl;
    }

    cv::Mat frame;
    for (;;) {
        cap >> frame;
        if (frame.empty()) break;

        if (hasModel) {
            // Placeholder: later will implement proper YuNet preprocessing + inference
            cv::putText(frame, "YuNet model loaded (placeholder detection)", cv::Point(10,30), cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0,255,0), 2);
        } else {
            cv::putText(frame, "Model missing: place ONNX in models/", cv::Point(10,30), cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0,0,255), 2);
        }

        // Display current mode and params
        std::string info = "Mode:" + mode + "  blur:" + std::to_string(blur_size) + "  pixel:" + std::to_string(pixel_size);
        cv::putText(frame, info, cv::Point(10, frame.rows - 10), cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(255,255,255), 2);

        cv::imshow("privacy_protector", frame);
        int key = cv::waitKey(1);
        if (key == 27) break; // ESC
    }
    FaceDetector detector(320,320,0.5f);
    bool hasModel=false;
    if(fs::exists(relModel)){
        hasModel=detector.loadModel(relModel);
    }else{

    }
    for(;;){
        cap>>frame;
        if(frame.empty()) break;
        if(hasModel){
            auto boxes=detector.detect(frame);
            for(const auto &r:boxes){
                cv::rectangle(frame,r,cv::Scalar(0,255,0),2);

            }
        }else{
            cv::putText(frame,"Model missing: place ONNX in models/",cv::Point(10,30),cv::FONT_HERSHEY_SIMPLEX,0.7,cv::Scalar(0,0,255),2);
        }
    }


    return 0;
}
