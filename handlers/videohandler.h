#ifndef VIDEOHANDLER_H
#define VIDEOHANDLER_H

#include <QUdpSocket>
#include <QObject>
#include <QtConcurrent/QtConcurrent>
#include <QGuiApplication>
#include <QScreen>
#include <string.h>
//#include <X11/Xlib.h>
extern "C" {
#include "libavcodec/avcodec.h"
#include "libavutil/pixfmt.h"
#include "libavformat/avformat.h"
#include "libavutil/time.h"
#include <libavutil/opt.h>
#include <libswscale/swscale.h>
#include "libavutil/frame.h"
#include "libavutil/imgutils.h"
}
#include "handlers/udpsockethandler.h"
#include "handlers/imagehandler.h"
class UdpSocketHandler;
class TcpSocketHandler;

class VideoHandler : public QObject
{
    Q_OBJECT
public:
    VideoHandler(QString cDeviceName, std::mutex* _writeLock,
                 int64_t time, ImageHandler* imageHandler,
                 UdpSocketHandler* _socketHandler,
                 int bufferSize, TcpSocketHandler* tcpSocketHandler, bool screenShare,
                 QObject* parent = 0);
    ~VideoHandler();
    int init();
    void grabFrames();
    void close();
    bool writeToFile = true;
    bool isActive();
    int numberOfFrames;
    const char* filename = "nyTest.ismv";
    QString aDeviceName;
    QString cDeviceName;
    std::mutex* writeLock;
    bool firstPacket = true;
    int start_pts;
    int start_dts;
    static int custom_io_write(void* opaque, uint8_t *buffer, int buffer_size);
    void toggleGrabFrames(bool a);

private:
    //Trenger kanskje ikke denne likevel?
    struct mSocketStruct {
        UdpSocketHandler* udpSocket;
        TcpSocketHandler* tcpSocket;
        bool headerSent;
    };
    mSocketStruct* mStruct;
    bool mActive = false;
    bool mScreenCapture = false;
    const char* mSource;
    int64_t time;
    int mBufferSize;
    int skipped_frames = 0;
    //std::ofstream outfile;
    AVCodecContext* inputVideoCodecContext;
    AVCodecContext* outputVideoCodecContext;
    AVFrame* videoFrame;
    AVFrame* scaledFrame;
    AVFormatContext *ifmt_ctx, *ofmt_ctx;
    AVCodec* inputVideoCodec;
    AVCodec* outputVideoCodec;
    int videoStream;
    UdpSocketHandler *socketHandler;
    struct SwsContext* img_convert_ctx;
    ImageHandler* imageHandler;
    bool mAbortGrabFrames = false;
    int mScreenWidth;
    int mScreenHeight;
    QString buildScreenDeviceName();
};
#endif // VideoHandler_H
