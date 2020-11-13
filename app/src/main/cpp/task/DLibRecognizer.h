//
// Created by Will Zhang on 2020/11/12.
//

#ifndef ANDROIDNDKAICAMERA_DLIBRECOGNIZER_H
#define ANDROIDNDKAICAMERA_DLIBRECOGNIZER_H

#include <dlib/dnn.h>
#include <dlib/image_io.h>

using namespace dlib;
using namespace std;

// ----------------------------------------------------------------------------------------

// The next bit of code defines a ResNet network.  It's basically copied
// and pasted from the dnn_imagenet_ex.cpp example, except we replaced the loss
// layer with loss_metric and made the network somewhat smaller.  Go read the introductory
// dlib DNN examples to learn what all this stuff means.
//
// Also, the dnn_metric_learning_on_images_ex.cpp example shows how to train this network.
// The dlib_face_recognition_resnet_model_v1 model used by this example was trained using
// essentially the code shown in dnn_metric_learning_on_images_ex.cpp except the
// mini-batches were made larger (35x15 instead of 5x5), the iterations without progress
// was set to 10000, and the training dataset consisted of about 3 million images instead of
// 55.  Also, the input layer was locked to images of size 150.
template<template<int, template<typename> class, int, typename> class block, int N,
        template<typename> class BN, typename SUBNET>
using residual = add_prev1<block<N, BN, 1, tag1<SUBNET>>>;

template<template<int, template<typename> class, int, typename> class block, int N,
        template<typename> class BN, typename SUBNET>
using residual_down = add_prev2<avg_pool<2, 2, 2, 2, skip1<tag2<block<N, BN, 2, tag1<SUBNET>>>>>>;

template<int N, template<typename> class BN, int stride, typename SUBNET>
using block = BN<con<N, 3, 3, 1, 1, relu<BN<con<N, 3, 3, stride, stride, SUBNET>>>>>;

template<int N, typename SUBNET> using ares = relu<residual<block, N, affine, SUBNET>>;
template<int N, typename SUBNET> using ares_down = relu<residual_down<block, N, affine, SUBNET>>;

template<typename SUBNET> using alevel0 = ares_down<256, SUBNET>;
template<typename SUBNET> using alevel1 = ares<256, ares<256, ares_down<256, SUBNET>>>;
template<typename SUBNET> using alevel2 = ares<128, ares<128, ares_down<128, SUBNET>>>;
template<typename SUBNET> using alevel3 = ares<64, ares<64, ares<64, ares_down<64, SUBNET>>>>;
template<typename SUBNET> using alevel4 = ares<32, ares<32, ares<32, SUBNET>>>;
using anet_type = loss_metric<fc_no_bias<128, avg_pool_everything<alevel0<alevel1<alevel2<alevel3<alevel4<max_pool<3, 3, 2, 2, relu<affine<con<32, 7, 7, 2, 2, input_rgb_image_sized<150>>>>>>>>>>>>>;


class DLibRecognizer {
private:
    anet_type net;

    float *matrix01ToFloatArray(matrix<float, 0, 1> face_descriptor) {
        float *arr = new float[128];
        std::copy(face_descriptor.begin(), face_descriptor.end(), arr);
        return arr;
    }

public:
    DLibRecognizer() {
        deserialize("/data/user/0/com.kk.afdd/files/modules/dlib/dlib_face_recognition_resnet_model_v1.dat") >> net;
    }

    ~DLibRecognizer() {
    }

    float *getFeature(Mat croppedFace) {
        // cv mat to dlib matrix
        matrix<rgb_pixel> dlibImg;
        assign_image(dlibImg, cv_image<rgb_pixel>(croppedFace));
        // This call asks the DNN to convert each face image in faces into a 128D vector.
        // In this 128D vector space, images from the same person will be close to each other
        // but vectors from different people will be far apart.  So we can use these vectors to
        // identify if a pair of images are from the same person or from different people.
        matrix<float, 0, 1> face_descriptor = net(dlibImg);
        return matrix01ToFloatArray(face_descriptor);
    }

    float calculateSimilar(float *feature1, float *feature2) {
        // 初始化时必须确定大小
        matrix<float, 0, 1> face_descriptor1(128, 1);
        matrix<float, 0, 1> face_descriptor2(128, 1);
        for (int i = 0; i < 128; i++) {
            face_descriptor1(i, 0) = feature1[i];
            face_descriptor2(i, 0) = feature2[i];
        }
        float delta = length(face_descriptor1 - face_descriptor2);
        LOGI("calculateSimilar:delta = %f", delta);
        return delta;
    }
};

#endif //ANDROIDNDKAICAMERA_DLIBRECOGNIZER_H
