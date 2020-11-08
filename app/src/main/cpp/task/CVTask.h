//
// Created by Will Zhang on 2020/10/27.
//

#ifndef CVTASK_H
#define CVTASK_H

#include "FrameTask.h"
#include "CVDetector.h"
#include "Frame.h"
#include <opencv2/core.hpp>
#include <opencv2/objdetect.hpp>
#include "mobile_facenet.h"

using namespace std;
using namespace cv;

class CVTask : public FrameTask {

private:

    CVDetector cvDetector;
//    Ptr<CascadeDetectorAdapter> detector;
    FaceRecognize *recognizer = new FaceRecognize();
    long lastRecTime = 0;

    static string string_format(const string fmt, ...) {
        int size = ((int) fmt.size()) * 2 + 50;   // Use a rubric appropriate for your code
        string str;
        va_list ap;
        while (1) {     // Maximum two passes on a POSIX system...
            str.resize(size);
            va_start(ap, fmt);
            int n = vsnprintf((char *) str.data(), size, fmt.c_str(), ap);
            va_end(ap);
            if (n > -1 && n < size) {  // Everything worked
                str.resize(n);
                break;
            }
            if (n > -1)  // Needed size returned
                size = n + 1;   // For null char
            else
                size *= 2;      // Guess at a larger size (OS specific)
        }
        return str;
    }

public:

    CVTask(int fps, string path) : FrameTask(move("CVTask"), fps) {
        cvDetector.create(path);
//        detector = makePtr<CascadeDetectorAdapter>(makePtr<CascadeClassifier>(path));
//        detector->setMinObjectSize(Size(20, 20));
        recognizer->setThreadNum(4);
        recognizer->init("/data/user/0/com.kk.afdd/files/modules/mobilefacenet.bin",
                         "/data/user/0/com.kk.afdd/files/modules/mobilefacenet.param",
                         false);
    }

    ~CVTask() {
        cvDetector.destroy();
//        delete detector;
        delete recognizer;
    }

    void doTask(Frame *frame) {
        long start = TimeUtil::now();
        Mat yuvImg, brgImg;
        yuvImg.create(frame->height * 3 / 2, frame->width, CV_8UC1);
        memcpy(yuvImg.data, frame->data, frame->len);
        cvtColor(yuvImg, brgImg, COLOR_YUV2GRAY_I420);

        vector<Rect> rectFaces;
        cvDetector.detect(brgImg, &rectFaces);
//        detector->detect(brgImg, rectFaces);

        string json = "[";
        for (int i = 0, n = rectFaces.size(); i < n; i++) {
            Rect rect = rectFaces[i];
            string featureJson = "[";
            if (i == 0 && (start - lastRecTime > 1000)) {
                lastRecTime = start;
                Mat *croppedFace = new Mat(brgImg, rect);
                ncnn::Mat ncnnFace = ncnn::Mat::from_pixels(croppedFace->data,
                                                            ncnn::Mat::PIXEL_BGR2RGB,
                                                            croppedFace->cols,
                                                            croppedFace->rows);
                float *feature = recognizer->getFeature(ncnnFace);
                delete croppedFace;

                for (int fi = 0, fn = recognizer->feature_dim; fi < fn; fi++) {
                    featureJson.append(string_format("%f", feature[fi]));
                    if (i != fn - 1) {
                        featureJson.append(",");
                    }
                }
            }
            featureJson.append("]");
            string facejson = string_format("{\"score\":%f,\"x1\":%d,\"y1\":%d,\"x2\":%d,\"y2\":%d,\"feature\":%s}",
                                            1.f,
                                            rect.x,
                                            rect.y,
                                            rect.x + rect.width,
                                            rect.y + rect.height,
                                            featureJson.c_str());
            json.append(facejson);
            LOGI("face:%s", facejson.c_str());
            if (i != n - 1) {
                json.append(",");
            }
        }
        json.append("]");
        long end = TimeUtil::now();
        delete frame;
        UIPreview::getInstance()->onFaceDetected(json);
        int faceCount = rectFaces.size();
        if (faceCount > 0) {
            LOGI("CVTask loadFaceInfo done, duration:%ld, found %d faces", end - start, faceCount);
        }
        rectFaces.clear();
        rectFaces.resize(0);
    }
};

#endif //CVTASK_H
