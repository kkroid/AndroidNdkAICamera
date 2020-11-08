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
    CascadeDetectorAdapter(Ptr<CascadeClassifier> detector) :
            IDetector(),
            detector(detector) {
        LOGD("CascadeDetectorAdapter::Detect::Detect");
        CV_Assert(detector);
//        scaleFactor = 1.3f;
    }

    void detect(const Mat &Image,
                std::vector<Rect> &objects) {
//        LOGD("CascadeDetectorAdapter::Detect: begin:%d x %d", Image.rows, Image.cols);
//        LOGD("CascadeDetectorAdapter::Detect: scaleFactor=%.2f, minNeighbours=%d, minObjSize=(%dx%d), maxObjSize=(%dx%d)",
//             scaleFactor, minNeighbours, minObjSize.width, minObjSize.height, maxObjSize.width, maxObjSize.height);
        detector->detectMultiScale(Image,
                                   objects,
                                   scaleFactor,
                                   minNeighbours,
                                   0,
                                   minObjSize,
                                   maxObjSize);
//        LOGD("CascadeDetectorAdapter::Detect: end");
    }

    virtual ~CascadeDetectorAdapter() {
        LOGD("CascadeDetectorAdapter::Detect::~Detect");
    }

private:
    CascadeDetectorAdapter() {};

    Ptr<CascadeClassifier> detector;
};

struct DetectorAgregator {
    Ptr<CascadeDetectorAdapter> mainDetector;
    Ptr<CascadeDetectorAdapter> trackingDetector;

    Ptr<DetectionBasedTracker> tracker;

    DetectorAgregator(Ptr<CascadeDetectorAdapter> &_mainDetector,
                      Ptr<CascadeDetectorAdapter> &_trackingDetector) :
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
    Ptr<CascadeClassifier> detector;
    DetectorAgregator *detectorAgregator;
public:

    CVDetector() {
    }

    ~CVDetector() {
        if (detectorAgregator) {
            delete detectorAgregator;
        }
    }

    void create(string path) {
        try {
            Ptr<CascadeDetectorAdapter> mainDetector = makePtr<CascadeDetectorAdapter>(
                    makePtr<CascadeClassifier>(path));
            Ptr<CascadeDetectorAdapter> trackingDetector = makePtr<CascadeDetectorAdapter>(
                    makePtr<CascadeClassifier>(path));
            detectorAgregator = new DetectorAgregator(mainDetector, trackingDetector);
//            if (faceSize > 0)
//            {
            mainDetector->setMinObjectSize(Size(20, 20));
            trackingDetector->setMinObjectSize(Size(20, 20));
//            }
            detectorAgregator->tracker->run();
        } catch (const Exception &e) {
            LOGD("nativeCreateObject caught Exception: %s", e.what());
        } catch (...) {
            LOGD("nativeCreateObject caught unknown exception");
        }
    }

    void destroy() {
        try {
            detectorAgregator->tracker->stop();
            delete detectorAgregator;
        } catch (const Exception &e) {
            LOGD("nativeestroyObject caught Exception: %s", e.what());
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
