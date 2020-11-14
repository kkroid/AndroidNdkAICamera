//
// Created by Will Zhang on 2020/11/14.
//

#ifndef ANDROIDNDKAICAMERA_NCNNRECOGNIZER_H
#define ANDROIDNDKAICAMERA_NCNNRECOGNIZER_H

#include <string>
#include "net.h"
#include <algorithm>
#include <opencv2/core.hpp>

class NCNNRecognizer {
private:
    ncnn::Net recognizer;
public:
    NCNNRecognizer() {
        recognizer.load_param("/data/user/0/com.kk.afdd/files/modules/mobilefacenet.param");
        recognizer.load_model("/data/user/0/com.kk.afdd/files/modules/mobilefacenet.bin");
    }

    ~NCNNRecognizer() {
        recognizer.clear();
    }

    float *getFeature(cv::Mat croppedFace) {
        ncnn::Mat ncnnFace = ncnn::Mat::from_pixels_resize(croppedFace.data,
                                                    ncnn::Mat::PIXEL_BGR2RGB,
                                                    croppedFace.cols,
                                                    croppedFace.rows,
                                                    112,
                                                    112);
        ncnn::Extractor ex = recognizer.create_extractor();
        // x.set_num_threads(4);
        ex.input("data", ncnnFace);
        ncnn::Mat out;
        ex.extract("fc1", out);
        float *feature = new float[128];
        for (int j = 0; j < 128; j++) {
            feature[j] = out[j];
        }
        return feature;
    }


    float calculateSimilar(float *feature1, float *feature2) {
        float ret = 0.0, mod1 = 0.0, mod2 = 0.0;
        for (int i = 0; i < 128; ++i) {
            ret += feature1[i] * feature2[i];
            mod1 += feature1[i] * feature1[i];
            mod2 += feature2[i] * feature2[i];
        }
        return ret / sqrt(mod1) / sqrt(mod2);
    }

};

#endif //ANDROIDNDKAICAMERA_NCNNRECOGNIZER_H
