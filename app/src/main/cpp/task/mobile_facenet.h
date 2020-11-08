//
// Created by 朱林 on 2020/5/9.
//
#ifndef FACE_NCNN_MOBILE_FACENET_H
#define FACE_NCNN_MOBILE_FACENET_H
#pragma once

#include <string>
#include "net.h"
#include <vector>

class FaceRecognize {
public:
    int num_thread = 4;
    bool run_gpu = false;
    const int feature_dim = 128;
    const int resize_w_h = 112; // 96 or 112

    FaceRecognize() {
    }

    ~FaceRecognize() {
        recognizeNet.clear();
        if (run_gpu) {
            ncnn::destroy_gpu_instance();
        }
    }

    void setThreadNum(int threads) {
        num_thread = threads;
    }

    void setRunGpu(bool run) {
        run_gpu = run;
    }

    bool init(const std::string &bin_path, const std::string &param_path, bool run_gpu = false) {
        setRunGpu(run_gpu);
        if (run_gpu) {
            ncnn::create_gpu_instance();
        }
        ncnn::Option opt;
        opt.lightmode = true;
        opt.num_threads = num_thread;
        if (run_gpu && ncnn::get_gpu_count() != 0) {
            opt.use_vulkan_compute = true;
        }
        recognizeNet.opt = opt;
        int ret = recognizeNet.load_param(param_path.data());
        if (ret != 0) {
            return false;
        }
        ret = recognizeNet.load_model(bin_path.data());
        return ret == 0;
    }

    float *getFeature(ncnn::Mat &img) {
        return runNet(img);
    }

private:
    ncnn::Net recognizeNet;

    float *runNet(ncnn::Mat &img) {
        ncnn::Extractor ex = recognizeNet.create_extractor();
        if (run_gpu && ncnn::get_gpu_count() != 0) {
            ex.set_vulkan_compute(true);
        }
        ex.set_light_mode(true);
        ex.set_num_threads(num_thread);
        ncnn::Mat in;
        ncnn::resize_bilinear(img, in, resize_w_h, resize_w_h);
        ex.input("data", in);
        ncnn::Mat out;
        ex.extract("fc1", out);
        float *feature = new float[feature_dim];
        for (int j = 0; j < 128; j++) {
            feature[j] = out[j];
        }
        return feature;
    }

    double calculSimilar(const float *feature1, const float *feature2) {
        double ret = 0.0, mod1 = 0.0, mod2 = 0.0;
        for (std::vector<double>::size_type i = 0; i != feature_dim; ++i) {
            ret += feature1[i] * feature2[i];
            mod1 += feature1[i] * feature1[i];
            mod2 += feature2[i] * feature2[i];
        }
        return (ret / sqrt(mod1) / sqrt(mod2) + 1) / 2.0;
    }
};


#endif //FACE_NCNN_MOBILE_FACENET_H
