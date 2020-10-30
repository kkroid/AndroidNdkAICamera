#ifndef ULTRADEMO_H
#define ULTRADEMO_H

#include <opencv2/opencv.hpp>
#include "FrameTask.h"
#include "Frame.h"
#include "native_debug.h"
#include "UltraFace.hpp"
#include "BaseNcnnTask.h"
#include "UIPreview.h"

UltraFace *faceDetector;

class UltraFaceTask : public BaseNcnnTask {
public:
    explicit UltraFaceTask(int fps, int width, int height, int threadCount, float scoreThreshold)
            : BaseNcnnTask("UltraFaceTask", fps) {
//        std::string bin_path = "/sdcard/RFB-320.bin";
//        std::string param_path = "/sdcard/RFB-320.param";
        std::string bin_path = "/sdcard/slim_320.bin";
        std::string param_path = "/sdcard/slim_320.param";
        faceDetector = new UltraFace(bin_path, param_path, width, height, scoreThreshold, scoreThreshold);

        std::string param = "/sdcard/gender-age.param";
        std::string bin = "/sdcard/gender-age.bin";
        genderAgeNet.load_param(param.data());
        genderAgeNet.load_model(bin.data());

    }

    ~UltraFaceTask() {
        genderAgeNet.clear();
    }

    void loadFaceInfo(ncnn::Mat inmat, std::vector<FaceInfo> *faceInfoList) {
//        std::lock_guard<std::mutex> lock(mutex);
        faceDetector->detect(inmat, *faceInfoList);
    }

    void loadGenderAgeInfo(ncnn::Mat inmat, FullFaceInfo *faceInfo) {
//        std::lock_guard<std::mutex> lock(mutex);
        ncnn::Extractor ex = genderAgeNet.create_extractor();
        ex.input("data", inmat);
        ncnn::Mat img_out;
        ex.extract("fc1", img_out);
        std::vector<float> out;
        for (int i = 0; i < img_out.w; ++i) {
            out.push_back(img_out[i]);
        }
        faceInfo->gender = out[0] > out[1];
        int counts = 0;
        for (int i = 2; i < 101; ++i) {
            if (out[2 * i] < out[2 * i + 1]) {
                ++counts;
            }
        }
        faceInfo->age = counts;
    }

private:
    std::mutex mutex;
    ncnn::Net genderAgeNet;
};

#endif //ULTRADEMO_H
