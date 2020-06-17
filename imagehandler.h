#ifndef IMAGEHANDLER_H
#define IMAGEHANDLER_H

#include <QObject>
#include <QImage>
#include <QQuickImageProvider>
#include <QtConcurrent/QtConcurrent>
extern "C" {
#include <libavutil/avassert.h>
#include <libavutil/channel_layout.h>
#include <libavutil/opt.h>
#include <libavutil/mathematics.h>
#include <libavutil/timestamp.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include <libavdevice/avdevice.h>
#include "libavformat/avformat.h"
#include "libavformat/avio.h"
#include "libavutil/frame.h"
#include "libavcodec/avcodec.h"
#include "libavfilter/avfilter.h"
#include "libavutil/avutil.h"
#include "libavutil/dict.h"
#include "libavutil/eval.h"
#include "libavutil/fifo.h"
#include "libavutil/pixfmt.h"
#include "libavutil/rational.h"
#include "libswresample/swresample.h"
}

class ImageHandler : public QObject, public QQuickImageProvider
{
    Q_OBJECT
public:
    explicit ImageHandler();
    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize) override;
    void veryFunStianLoop();
    void readPacket(uint8_t *buffer, int buffer_size);
    void readImage(AVCodecContext* codecContext, AVFrame* scaledFrame, uint8_t index);
    void addPeer(uint8_t index);

public slots:
    void updateImage(const QImage &image, uint8_t index);

signals:
    void imageChanged();

private:
    QMap<uint8_t, QImage> mImageMap;
    QImage mDefaultImage;
};

#endif // IMAGEHANDLER_H
