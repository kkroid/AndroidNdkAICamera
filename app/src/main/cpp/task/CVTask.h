//
// Created by Will Zhang on 2020/10/27.
//

#ifndef CVTASK_H
#define CVTASK_H

#include "FrameTask.h"
#include "CVDetector.h"
#include "Frame.h"

class CVTask : public FrameTask {

private:

    CVDetector cvDetector;

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

    CVTask(int fps, std::string path) : FrameTask(move("CVTask"), fps) {
//            std::string path = "/sdcard/lbpcascades/lbpcascade_frontalface_improved.xml";
        cvDetector.create(path);
    }

    ~CVTask() {
        cvDetector.destroy();
    }

    void doTask(Frame *frame) {
        long start = TimeUtil::now();
        cv::Mat yuvImg, brgImg;
        yuvImg.create(frame->height * 3 / 2, frame->width, CV_8UC1);
        memcpy(yuvImg.data, frame->data, frame->len);
        cv::cvtColor(yuvImg, brgImg, cv::COLOR_YUV2GRAY_I420);

        vector<Rect> rectFaces;
        cvDetector.detect(brgImg, &rectFaces);

        string json = "[";
        for (int i = 0, n = rectFaces.size(); i < n; i++) {
            Rect rect = rectFaces[i];
            string facejson = string_format("{\"score\":%f,\"x1\":%d,\"y1\":%d,\"x2\":%d,\"y2\":%d,\"gender\":%s,"
                                            "\"age\":%d}",
                                            1.f/*weights[i]*/,
                                            rect.x,
                                            rect.y,
                                            rect.x + rect.width,
                                            rect.y + rect.height,
                                            "false",
                                            1);
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
        LOGI("CVTask loadFaceInfo done, duration:%ld, found %d faces", end - start, rectFaces.size());
        rectFaces.clear();
        rectFaces.resize(0);
    }
};

#endif //CVTASK_H
