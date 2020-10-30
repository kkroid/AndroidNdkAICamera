//
// Created by Will Zhang on 2020/10/27.
//

#ifndef CVDETECTOR_H
#define CVDETECTOR_H

#include <opencv2/core.hpp>
#include <opencv2/objdetect.hpp>
#include "native_debug.h"
#include <string>
#include <vector>


using namespace std;
using namespace cv;

class CascadeDetectorAdapter : public DetectionBasedTracker::IDetector {
public:
    CascadeDetectorAdapter(cv::Ptr<cv::CascadeClassifier> detector) :
            IDetector(),
            detector(detector) {
        LOGD("CascadeDetectorAdapter::Detect::Detect");
        CV_Assert(detector);
    }

    void detect(const cv::Mat &Image,
                std::vector<cv::Rect> &objects) {
        LOGD("CascadeDetectorAdapter::Detect: begin:%d x %d", Image.rows, Image.cols);
        LOGD("CascadeDetectorAdapter::Detect: scaleFactor=%.2f, minNeighbours=%d, minObjSize=(%dx%d), maxObjSize=(%dx%d)",
             scaleFactor, minNeighbours, minObjSize.width, minObjSize.height, maxObjSize.width, maxObjSize.height);
        detector->detectMultiScale(Image,
                                   objects,
                                   scaleFactor,
                                   minNeighbours,
                                   0,
                                   minObjSize,
                                   maxObjSize);
        LOGD("CascadeDetectorAdapter::Detect: end");
    }

    virtual ~CascadeDetectorAdapter() {
        LOGD("CascadeDetectorAdapter::Detect::~Detect");
    }

private:
    CascadeDetectorAdapter() {};

    cv::Ptr<cv::CascadeClassifier> detector;
};

struct DetectorAgregator {
    cv::Ptr<CascadeDetectorAdapter> mainDetector;
    cv::Ptr<CascadeDetectorAdapter> trackingDetector;

    cv::Ptr<DetectionBasedTracker> tracker;

    DetectorAgregator(cv::Ptr<CascadeDetectorAdapter> &_mainDetector,
                      cv::Ptr<CascadeDetectorAdapter> &_trackingDetector) :
            mainDetector(_mainDetector),
            trackingDetector(_trackingDetector) {
        CV_Assert(_mainDetector);
        CV_Assert(_trackingDetector);

        DetectionBasedTracker::Parameters DetectorParams;
        tracker = makePtr<DetectionBasedTracker>(mainDetector, trackingDetector, DetectorParams);
    }
};

class CVDetector {

private:
    cv::Ptr<cv::CascadeClassifier> detector;
    DetectorAgregator *detectorAgregator;
public:

    CVDetector() {
    }

    ~CVDetector() {
    }

    void create(string path) {
        try {
            cv::Ptr<CascadeDetectorAdapter> mainDetector = makePtr<CascadeDetectorAdapter>(
                    makePtr<CascadeClassifier>(path));
            cv::Ptr<CascadeDetectorAdapter> trackingDetector = makePtr<CascadeDetectorAdapter>(
                    makePtr<CascadeClassifier>(path));
            detectorAgregator = new DetectorAgregator(mainDetector, trackingDetector);
//            if (faceSize > 0)
//            {
            mainDetector->setMinObjectSize(Size(20, 20));
            trackingDetector->setMinObjectSize(Size(20, 20));
//            }
            detectorAgregator->tracker->run();
        } catch (const cv::Exception &e) {
            LOGD("nativeCreateObject caught cv::Exception: %s", e.what());
        } catch (...) {
            LOGD("nativeCreateObject caught unknown exception");
        }
    }

    void destroy() {
        try {
            detectorAgregator->tracker->stop();
            delete detectorAgregator;
        } catch (const cv::Exception &e) {
            LOGD("nativeestroyObject caught cv::Exception: %s", e.what());
        } catch (...) {
            LOGD("nativeDestroyObject caught unknown exception");
        }
    }

    void detect(Mat gray, vector<Rect> *faces) {
        detectorAgregator->tracker->process(gray);
        detectorAgregator->tracker->getObjects(*faces);
    }
};

#endif //CVDETECTOR_H
