/*
 * Demo参考：https://blog.csdn.net/sinat_31425585/article/details/88878839
 * 原始模型：MAXNET
 * 模型来源：https://github.com/deepinsight/insightface/tree/master/gender-age
 * 转换工具：https://github.com/Tencent/ncnn/tree/master/tools/mxnet
 *
 */

#include "BaseNcnnTask.h"
#include "mat.h"
#include "native_debug.h"

typedef struct GenderAgeInfo {
    bool gender;
    int age;
} GenderAgeInfo;

class GenderAgeDemo : public BaseNcnnTask {
public:
    ncnn::Net net;

    GenderAgeDemo(int fps) : BaseNcnnTask("GenderAgeDemo", fps) {
        std::string param = "/sdcard/gender-age.param";
        std::string bin = "/sdcard/gender-age.bin";
        net.load_param(param.data());
        net.load_model(bin.data());
    }

    ~GenderAgeDemo() {
        net.clear();
    }

    void onMatAvailable(ncnn::Mat inmat) {
        ncnn::Extractor ex = net.create_extractor();
        ex.input("data", inmat);
        ncnn::Mat img_out;
        ex.extract("fc1", img_out);
        std::vector<float> out;
        for (int i = 0; i < img_out.w; ++i) {
            out.push_back(img_out[i]);
        }
        GenderAgeInfo genderAgeInfo;
        genderAgeInfo.gender = out[0] > out[1];

        int counts = 0;
        for (int i = 2; i < 101; ++i) {
            if (out[2 * i] < out[2 * i + 1]) {
                ++counts;
            }
        }
        genderAgeInfo.age = counts;
        LOGI("gender:%d, age:%d", genderAgeInfo.gender, genderAgeInfo.age);
    }
};
