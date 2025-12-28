#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>
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
    // Try requested device first, then try 0..3
    bool opened = false;
    for (int d = device; d <= device + 3; ++d) {
        cap.open(d, cv::CAP_DSHOW);
        if (cap.isOpened()) {
            std::cout << "Opened camera device: " << d << std::endl;
            opened = true;
            break;
        }
        // close and try next
        cap.release();
    }
    if (!opened) {
        std::cerr << "Cannot open camera. Check Windows Camera permissions or try different -device index (0,1,2...)." << std::endl;
        std::cerr << "Also ensure no other app is using the camera and desktop apps are allowed in Settings > Privacy & security > Camera." << std::endl;
        return -1;
    }

    // Try to load YuNet model from models/ (user should download into models/)
    std::string modelPath = "models/face_detection_yunet_2023mar.onnx";
    cv::dnn::Net net;
    bool hasModel = false;
    try {
        // 1) check current working directory first
        if (fs::exists(modelPath)) {
            net = cv::dnn::readNet(modelPath);
            hasModel = true;
            std::cout << "Loaded model from cwd: " << modelPath << std::endl;
        } else {
            // 2) search upward from executable directory for a models/ folder
            fs::path exePath = fs::absolute(argv[0]);
            fs::path cur = exePath.parent_path();
            bool found = false;
            for (int i = 0; i < 6 && !cur.empty(); ++i) {
                fs::path candidate = cur / "models" / "face_detection_yunet_2023mar.onnx";
                if (fs::exists(candidate)) {
                    net = cv::dnn::readNet(candidate.string());
                    hasModel = true;
                    found = true;
                    modelPath = candidate.string();
                    std::cout << "Loaded model from: " << candidate.string() << std::endl;
                    break;
                }
                cur = cur.parent_path();
            }
            if (!found) {
                std::cout << "Model not found at '" << modelPath << "' or in parent dirs. Running without detection." << std::endl;
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
    int frameCount = 0;
    for (;;) {
        bool ok = cap.read(frame);
        if (!ok || frame.empty()) {
            // don't exit; show placeholder and continue
            frame = cv::Mat(480, 640, CV_8UC3, cv::Scalar(80, 80, 80));
            cv::putText(frame, "No frame from camera (check permissions or index)", cv::Point(10,30), cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0,0,255), 2);
            std::cout << "Warning: failed to read frame from camera." << std::endl;
        } else {
            if ((frameCount++ % 150) == 0) {
                std::cout << "Frame size: " << frame.cols << "x" << frame.rows << std::endl;
            }
        }

        if (hasModel) {
            // Placeholder: user will replace with actual YuNet preprocessing + inference
            // For now, just display frame and a notice
            cv::putText(frame, "YuNet model loaded (placeholder detection)", cv::Point(10,30), cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0,255,0), 2);
        } else {
            cv::putText(frame, "Model missing: place ONNX in models/", cv::Point(10,30), cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0,0,255), 2);
        }

        // Display current mode and params
        std::string info = "Mode:" + mode + "  blur:" + std::to_string(blur_size) + "  pixel:" + std::to_string(pixel_size);
        cv::putText(frame, info, cv::Point(10, frame.rows - 10), cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(255,255,255), 2);

        cv::imshow("privacy_protector", frame);
        int key = cv::waitKey(30);
        if (key == 27) break; // ESC
    }

    return 0;
}
