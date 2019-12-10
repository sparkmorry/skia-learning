//
//  main.cpp
//  skia-learning
//
//  Created by 九瑶 on 2019/12/10.
//  Copyright © 2019 九瑶. All rights reserved.
//

#include "experimental/ffmpeg/SkVideoEncoder.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkGraphics.h"
#include "include/core/SkStream.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTime.h"
#include "include/utils/SkRandom.h"

struct AsyncRec {
    SkImageInfo info;
    SkVideoEncoder* encoder;
};

void produce_frame(SkSurface* surf, int i) {
    SkCanvas* canvas = surf->getCanvas();
    SkPaint paint;
    
    canvas->clear(SK_ColorWHITE);
    paint.setColor(SK_ColorBLACK);
    SkRect rect = { 0, 0, 25, 25 };
    canvas->drawRect(rect, paint);
}

int main(int argc, char** argv) {
    SkGraphics::Init();
    
    int loop = true;
    // 初始化编码器
    SkVideoEncoder encoder;
    // 初始化大小及参数
    SkISize dim;
    dim.fWidth = 200;
    dim.fHeight = 200;
    double duration = 2.0;
    int fps = 25;
    
    float scale = 1;
    const int frames = SkScalarRoundToInt(duration * fps);
//    const double frame_duration = 1.0 / fps;
    
    sk_sp<SkSurface> surf;
    sk_sp<SkData> data;

    
    //    init image size and color space
    const auto info = SkImageInfo::MakeN32Premul(dim);
    do {
        double loop_start = SkTime::GetSecs();
        
        if(!encoder.beginRecording(dim, fps)) {
            SkDEBUGF("Invalid video stream configuration.\n");
            return -1;
        }
        
        if (!surf) {
            surf = SkSurface::MakeRaster(info);
        }
        
        surf->getCanvas()->scale(scale, scale);
        SkCanvas* canvas = surf->getCanvas();
        canvas->clear(SK_ColorWHITE);
        SkPaint paint;
        paint.setColor(SK_ColorBLACK);
        SkRandom rand;
        
        
        for (int i = 0; i <= frames; ++i) {
            const double frame = i;
            SkDebugf("rendering frame %g\n", frame);
            
            // produce frame
            paint.setColor(rand.nextU() | 0x44808080);
            canvas->save();
            canvas->translate(i, i);
            int centerX = rand.nextRangeU(30, 170);
            int centerY = rand.nextRangeU(30, 170);
            canvas->drawCircle(centerX, centerY, 30, paint);
            canvas->restore();
            
            AsyncRec asyncRec = { info, &encoder };
            
            SkPixmap pm;
            // 从surf里面拿pm矩阵
            SkAssertResult(surf->peekPixels(&pm));
            encoder.addFrame(pm);
        }
        
        data = encoder.endRecording();

        if (loop) {
            double loop_dur = SkTime::GetSecs() - loop_start;
            SkDebugf("recording secs %g, frames %d, recording fps %d\n",
                     loop_dur, frames, (int)(frames / loop_dur));
            loop = false;
        }
        
    } while (loop);
    
    SkFILEWStream ostream("react.mp4");
    if (!ostream.isValid()) {
        SkDebugf("Can't create output file %s\n", "rect.mp4");
        return -1;
    }
    ostream.write(data->data(), data->size());
    
    
    return 0;
}
    
