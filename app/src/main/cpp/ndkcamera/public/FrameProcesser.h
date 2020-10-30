#ifndef FRAMEPROCESSER_H
#define FRAMEPROCESSER_H

#include <stdint.h>
#include "Frame.h"
#include "FrameTask.h"
#include "native_debug.h"
#include <map>
#include "blockingconcurrentqueue.h"
#include <ctpl_stl.h>
#include <libyuv/convert.h>
#include "PreviewCallback.h"
#include "time_util.h"

class FrameProcessor {
private:
    ctpl::thread_pool *tp;
    moodycamel::BlockingConcurrentQueue<Frame *> frameQueue;

    void process() {
        while (true) {
            // long startTime = TimeUtil::now();
            Frame *frame;
            frameQueue.wait_dequeue(frame);
            // long endTime = TimeUtil::now();
            // LOGI("wait time duration:%ld", (endTime - startTime));
            if (nullptr == frame || nullptr == frame->data) {
                delete frame;
                LOGI("Ignore empty frame");
                return;
            }
            // LOGI("process, size = %lu", frameTaskMap->size());
            // 单任务
            if (frameTaskMap->size() == 1) {
                auto *frameTask = frameTaskMap->begin()->second;
                if (!frameTask->ignoreFrame()) {
                    tp->push([frameTask, frame](int id) {
                        frameTask->doTask(frame);
                    });
                }
            } else {
                // 多任务
                std::map<std::string, FrameTask *>::iterator taskIterator;
                for (taskIterator = frameTaskMap->begin(); taskIterator != frameTaskMap->end(); taskIterator++) {
                    auto *frameTask = taskIterator->second;
                    if (!frameTask->ignoreFrame()) {
                        Frame *frameCopy = frame->copy();
                        tp->push([frameTask, frameCopy](int id) {
                            frameTask->doTask(frameCopy);
                        });
                    }
                }
                delete frame;
            }
            // long endTime = TimeUtil::now();
            // LOGI("process time duration:%ld", (endTime - startTime));
            if (frameTaskMap->empty()) {
                break;
            }
        }
    }

public:
    std::map<std::string, FrameTask *> *frameTaskMap;

    explicit FrameProcessor(std::map<std::string, FrameTask *> *_frameTaskMap) : frameTaskMap(_frameTaskMap) {
        tp = new ctpl::thread_pool(3);
        tp->push([this](int id) {
            this->process();
        });
    }

    virtual ~FrameProcessor() {
        LOGI("~FrameProcessor");
        std::map<std::string, FrameTask *>::iterator taskIterator;
        for (taskIterator = frameTaskMap->begin(); taskIterator != frameTaskMap->end(); taskIterator++) {
            if (taskIterator->second) {
                delete taskIterator->second;
            }
        }
        frameTaskMap->clear();
        Frame *breakWaitingQueueFrame = new Frame(nullptr, 0, 0, 0);
        frameQueue.enqueue(breakWaitingQueueFrame);
        tp->stop();
        tp->clear_queue();
        delete tp;
        tp = nullptr;
    }

    void push(Frame *frame) {
        auto *frameCopy = frame->copy();
        frameQueue.enqueue(frameCopy);
        delete frame;
    }
};

#endif //FRAMEPROCESSER_H
