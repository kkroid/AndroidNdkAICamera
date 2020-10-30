#ifndef PREVIEWCALLBACK_H
#define PREVIEWCALLBACK_H

#include <stdint.h>

class PreviewCallback {
public:
    virtual void onFrameAvailable(Frame *frame) = 0;

    virtual ~PreviewCallback() = default;
};

#endif // PREVIEWCALLBACK_H