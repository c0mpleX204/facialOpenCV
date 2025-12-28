#include <opencv2/opencv.hpp>
#include <iostream>

int main() {
    std::cout << "Scanning camera indices 0..10...\n";
    for (int i = 0; i <= 10; ++i) {
        cv::VideoCapture cap(i);
        if (cap.isOpened()) {
            std::cout << "Device " << i << ": OPEN (" << cap.get(cv::CAP_PROP_FRAME_WIDTH) << "x" << cap.get(cv::CAP_PROP_FRAME_HEIGHT) << ")\n";
            cap.release();
        } else {
            std::cout << "Device " << i << ": closed\n";
        }
    }
    std::cout << "Scan complete. If no device is open, check Windows Camera permissions or drivers.\n";
    return 0;
}
