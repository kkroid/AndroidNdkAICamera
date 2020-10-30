#ifndef BASENCNNTASK_H
#define BASENCNNTASK_H

#include "FrameTask.h"
#include "Frame.h"
#include "UIPreview.h"
#include <opencv2/opencv.hpp>


typedef struct FullFaceInfo {
    FaceInfo faceInfo;
    bool gender;
    int age;
public:
    std::string toJson() {
        return string_format("{\"score\":%f,\"x1\":%f,\"y1\":%f,\"x2\":%f,\"y2\":%f,\"gender\":%s,\"age\":%d}",
                             faceInfo.score,
                             faceInfo.x1,
                             faceInfo.y1,
                             faceInfo.x2,
                             faceInfo.y2,
                             gender ? "true" : "false",
                             age);
    }

private:
    static std::string string_format(const std::string fmt, ...) {
        int size = ((int) fmt.size()) * 2 + 50;   // Use a rubric appropriate for your code
        std::string str;
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
} FullFaceInfo;

class BaseNcnnTask : public FrameTask {
public:
    BaseNcnnTask(std::string name, int fps) : FrameTask(std::move(name), fps) {
    }

    void doTask(Frame *frame) {
        long start = TimeUtil::now();
        long start0 = TimeUtil::now();
        cv::Mat yuvImg;
        cv::Mat brgImg(frame->height, frame->width, CV_8UC3);
        yuvImg.create(frame->height * 3 / 2, frame->width, CV_8UC1);
        memcpy(yuvImg.data, frame->data, frame->len);
        cv::cvtColor(yuvImg, brgImg, cv::COLOR_YUV2BGR_I420);
        ncnn::Mat inmat = ncnn::Mat::from_pixels(brgImg.data, ncnn::Mat::PIXEL_BGR2RGB, brgImg.cols, brgImg.rows);
        std::vector<FaceInfo> faceInfoList;
        loadFaceInfo(inmat, &faceInfoList);
        long end0 = TimeUtil::now();
        LOGI("loadFaceInfo done, duration:%ld", end0 - start0);
        int n = faceInfoList.size();
        LOGI("%d faces detected", n);
        std::string json = "[";
        for (int i = 0; i < n; i++) {
            auto face = faceInfoList[i];
            FullFaceInfo *fullFaceInfo = new FullFaceInfo();
            fullFaceInfo->faceInfo = face;
//            if (i == 0) {
//                long start1 = TimeUtil::now();
//                cv::Rect faceRect(face.x1, face.y1, face.x2 - face.x1, face.y2 - face.y1);
//                cv::Mat faceYuvImg;
//                cv::Mat faceBrgImg(frame->height, frame->width, CV_8UC3);
//                faceYuvImg.create(frame->height * 3 / 2, frame->width, CV_8UC1);
//                memcpy(faceYuvImg.data, frame->data, frame->len);
//                cv::cvtColor(faceYuvImg, faceBrgImg, cv::COLOR_YUV2BGR_I420);
//                cv::Mat faceMat = faceBrgImg(faceRect);
//                // cv::imwrite("/sdcard/face.jpg", faceMat);
//                ncnn::Mat resizedMat = ncnn::Mat::from_pixels(faceMat.data,
//                                                              ncnn::Mat::PIXEL_BGR2RGB,
//                                                              faceMat.cols,
//                                                              faceMat.rows);
//                loadGenderAgeInfo(resizedMat, fullFaceInfo);
//                long end1 = TimeUtil::now();
//                LOGI("loadGenderAgeInfo done, duration:%ld", end1 - start1);
//            }
            std::string facejson = fullFaceInfo->toJson();
            LOGI("face:%s", facejson.c_str());
            json.append(facejson);
            if (i != n - 1) {
                json.append(",");
            }
        }
        json.append("]");
        delete (frame);
        UIPreview::getInstance()->onFaceDetected(json);
        long end = TimeUtil::now();
        LOGI("detection done, duration:%ld", end - start);
    }

    virtual void loadFaceInfo(ncnn::Mat inmat, std::vector<FaceInfo> *faceInfoList) = 0;

    virtual void loadGenderAgeInfo(ncnn::Mat inmat, FullFaceInfo *faceInfo) = 0;
};

#endif // BASENCNNTASK_H
