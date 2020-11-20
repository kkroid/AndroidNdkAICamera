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
#include <opencv2/imgproc.hpp>
#include "NCNNRecognizer.h"

using namespace std;
using namespace cv;

class CVTask : public FrameTask {

private:

    CVDetector cvDetector;
    NCNNRecognizer *ncnnRecognizer = new NCNNRecognizer();
    // long lastRecTime = 0;

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
    }

    ~CVTask() {
        cvDetector.destroy();
        delete ncnnRecognizer;
    }

    void doTask(Frame *frame) {
         long start = TimeUtil::now();
        Mat yuvImg, brgImg;
        yuvImg.create(frame->height * 3 / 2, frame->width, CV_8UC1);
        memcpy(yuvImg.data, frame->data, frame->len);
        cvtColor(yuvImg, brgImg, COLOR_YUV2GRAY_I420);

        std::vector<Rect> rectFaces;
        cvDetector.detect(brgImg, &rectFaces);

        string json = "[";
        for (int i = 0, n = rectFaces.size(); i < n; i++) {
            Rect rect = rectFaces[i];
            string featureJson = "[";
            if (i == 0/* && (start - lastRecTime > 5000)*/) {
                // lastRecTime = start;
                Mat *croppedFace = new Mat(brgImg, rect);
                float *feature = ncnnRecognizer->getFeature(*croppedFace);
                delete croppedFace;
                for (int fi = 0, fn = 128; fi < fn; fi++) {
                    featureJson.append(string_format("%f", feature[fi]));
                    if (fi != fn - 1) {
                        featureJson.append(",");
                    }
                }
                // LOGI("face:%s", featureJson.c_str());
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
            // LOGI("face:%s", facejson.c_str());
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

    float calculateSimilar(float *feature1, float *feature2) {
        if (ncnnRecognizer) {
            return ncnnRecognizer->calculateSimilar(feature1, feature2);
        }
        return 0;
    }
};

#endif //CVTASK_H
