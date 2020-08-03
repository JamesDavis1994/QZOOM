#include "imagehandler.h"

ImageHandler::ImageHandler(Settings* settings) : QQuickImageProvider(QQuickImageProvider::Image)
{
    mSettings = settings;
    mDefaultImage = generateGenericImage(mSettings->getDisplayName());//QImage("0.png");

    this->blockSignals(false);
    //addPeer(0, mSettings->getDisplayName());
}

QImage ImageHandler::requestImage(const QString &id, QSize *size, const QSize &requestedSize)
{
    int index = 0;
    QStringList onlyId = id.split("=");
    if(onlyId.size() >= 2)
    {
        QStringList idIndex = onlyId[1].split("&");
        if(idIndex.size() >= 2)
        {

            index = idIndex[1].toInt();
            if(index > 0)
            {
                //qDebug() << index;
            }
        }
    }
    QImage result = mImageMap[index].first;

    if(result.isNull())
    {
        //result = mDefaultImage;
        result = generateGenericImage(mImageMap[index].second);
        //qDebug() << "Default image is null";
    }

    if(size)
    {
        *size = result.size();
    }

    if(requestedSize.width() > 0 && requestedSize.height() > 0)
    {
        result = result.scaled(requestedSize.width(), requestedSize.height(), Qt::KeepAspectRatio);
    }

    return result;
}

/**
 * Adds one default image to the map at the index and sends the signal refreshScreens
 * which is connected to TaskBar.qml Connections function onRefreshScreens()
 * @param index uint8_t
 */
void ImageHandler::addPeer(uint8_t index, QString displayName)
{
    qDebug() << "Added peer to ImageHandler map: " << index;
    mImageMap[index].first = generateGenericImage(displayName);
    mImageMap[index].second = displayName;
    emit refreshScreens();
}

void ImageHandler::removeAllPeers()
{
    qDebug() << "Removing all peers";
    mImageMap.clear();
}

/**
 * Removes index from mImageMap and sends the signal refreshScreens
 * which is connected to TaskBar.qml Connections function onRefreshScreens()
 * @param index uint8_t
 */
void ImageHandler::removePeer(uint8_t index)
{
    qDebug() << "Removing peer from ImageHandler map: " << index;

    mImageMap.erase(index + 1);

    emit refreshScreens();
}

void ImageHandler::updatePeerDisplayName(uint8_t index, QString displayName)
{
    qDebug() << index << "Updated their display name to:" << displayName;
    mImageMap[index].second = displayName;
    mImageMap[index].first = generateGenericImage(mImageMap[index].second);
}

void ImageHandler::setPeerVideoAsDisabled(uint8_t index)
{
    mImageMap[index].first = generateGenericImage(mImageMap[index].second);
}

/**
 * If input image is not the same QImage already in the mImageMap,
 * update the mImageMap at index with the new QImage
 * @param index uint8_t
 * @param image QImage
 */
void ImageHandler::updateImage(const QImage &image, uint8_t index)
{
    imgLock.lock();
    if(mImageMap[index].first != image)
    {
        mImageMap[index].first = image;
    }
    else
    {
        qDebug() << "Same Image";
    }
    imgLock.unlock();
}

/**
 * One for the history books.
 */
void ImageHandler::veryFunStianLoop()
{
    /*QtConcurrent::run([this]()
    {
        int ms = 41; //1000/24
        int i = 1;
        QString overhead = "";
        while (true)
        {
            if (i < 100)
            {
                overhead = "0";
                if (i < 10)
                {
                    overhead = "00";
                }
            }
            else
            {
                overhead = "";
            }
            QString str = "/home/stian/Downloads/testtttt/" + overhead + QString::number(i) + ".jpg";
            QImage test = QImage(str);
            updateImage(test);
            if (i == 382)
            {
                i = 1;
            }
            i++;

            struct timespec ts = { ms / 1000, (ms % 1000) * 1000 * 1000 };
            nanosleep(&ts, NULL);
        }
    });*/
}
/**
 * We need to convert the video frame to RGB24 before turning it into a QImage object.
 * Then we update the map at said index with the new QImage.
 * @param codecContext AVCodecContext* the video decoder codec context
 * @param[in,out] frame AVFrame* which contains data for 1 video frame
 * @param index uint8_t the index in the map to update the new QImage
 */
void ImageHandler::readImage(AVCodecContext* codecContext, AVFrame* frame, uint8_t index)
{
    SwsContext *imgConvertCtx = nullptr;
    AVFrame	*frameRGB = av_frame_alloc();


    if(codecContext == nullptr)
    {
        emit updateImage(generateGenericImage(mSettings->getDisplayName()), 0);
        return;
    }

    QImage img(frame->width, frame->height, QImage::Format_RGB888);


    frameRGB->format = AV_PIX_FMT_RGB24;
    frameRGB->width = frame->width;
    frameRGB->height = frame->height;

    //avpicture_alloc((AVPicture*)frameRGB, A frame->width, frame->height);
    av_image_alloc(frameRGB->data,frameRGB->linesize,frame->width,frame->height,AV_PIX_FMT_RGB24,1);

    imgConvertCtx = sws_getContext(codecContext->width, codecContext->height,
                                   codecContext->pix_fmt, frame->width, frame->height, AV_PIX_FMT_RGB24,
                                   SWS_BICUBIC, NULL, NULL, NULL);
    if (imgConvertCtx)
    {
        //conversion frame to frameRGB
        sws_scale(imgConvertCtx, frame->data, frame->linesize, 0, codecContext->height, frameRGB->data, frameRGB->linesize);
        //setting QImage from frameRGB

        for (int y = 0; y < frame->height; ++y)
        {
            memcpy( img.scanLine(y), frameRGB->data[0]+y * frameRGB->linesize[0], frameRGB->linesize[0]);
        }
    }
    //av_freep(&frame->data[0]);
    //av_frame_unref(frame);

    av_freep(&frameRGB->data[0]);
    av_frame_unref(frameRGB);

    sws_freeContext(imgConvertCtx);

    //av_frame_free(&frameRGB);
    emit updateImage(img, index);
    //av_frame_unref(frameRGB);
    //av_frame_free(&frameRGB);
    //delete frameRGB;
    //sws_freeContext(imgConvertCtx);
    av_freep(&frameRGB->data[0]);

    av_frame_free(&frameRGB);

    //av_packet_unref(&packet);
}


int ImageHandler::getNumberOfScreens()
{
    return mImageMap.size();
    //return 5;
}

QImage ImageHandler::generateGenericImage(QString displayName)
{
    QImage image(QSize(1280, 720), QImage::Format_RGB32);
    QPainter painter(&image);
    painter.setBrush(QBrush(Qt::green));
    painter.fillRect(QRectF(0, 0, 1280, 720), QColor(27, 29, 54, 255));
    //painter.fillRect(QRectF(200,150,800,600), Qt::blue);

    painter.setPen(QPen(Qt::white));
    painter.setFont(QFont("Helvetica [Cronyx]", 26, QFont::Bold));
    //QString text = displayName + " hat seinen Kamera ausgeschaltet";
    QString text = displayName + "\n (Screen Disabled)";
    painter.drawText(QRectF(0, 0, 1280, 720), Qt::AlignCenter | Qt::AlignTop, text);
    return image;
}

/*
void ImageHandler::readPacket(uint8_t *buffer, int buffer_size)
{
    QtConcurrent::run([this]()
    {
        AVFormatContext *fmt_ctx = nullptr;
        AVIOContext *avio_ctx = nullptr;
        uint8_t *avio_ctx_buffer = nullptr;
        size_t avio_ctx_buffer_size = 4096;

        int ret = 0;
        fmt_ctx = avformat_alloc_context();
        Q_ASSERT(fmt_ctx);

        avio_ctx_buffer = reinterpret_cast<uint8_t*>(av_malloc(avio_ctx_buffer_size));
        Q_ASSERT(avio_ctx_buffer);
        //avio_ctx = avio_alloc_context(avio_ctx_buffer, static_cast<int>(avio_ctx_buffer_size), 0, &bdudpSocket, read_packet(), nullptr, nullptr);
        //avio_ctx = avio_alloc_context()
        Q_ASSERT(avio_ctx);


        fmt_ctx->pb = avio_ctx;
        ret = avformat_open_input(&fmt_ctx, nullptr, nullptr, nullptr);
        Q_ASSERT(ret >= 0);
        ret = avformat_find_stream_info(fmt_ctx, nullptr);
        Q_ASSERT(ret >= 0);

        AVStream	*video_stream = nullptr;
        for (uint i=0; i<fmt_ctx->nb_streams; ++i) {
            auto	st = fmt_ctx->streams[i];
            qDebug() << st->id << st->index << st->start_time << st->duration << st->codecpar->codec_type;
            if (st->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) video_stream = st;
        }
        Q_ASSERT(video_stream);


        AVCodecParameters	*codecpar = video_stream->codecpar;
        AVCodec* codec = avcodec_find_decoder(codecpar->codec_id);
        AVCodecContext *codec_context = avcodec_alloc_context3(codec);
        int err = avcodec_open2(codec_context, codec, nullptr);
        Q_ASSERT(err>=0);
        qDebug() << codec->name << codec->id;
        qDebug() << codecpar->width << codecpar->height << codecpar->format << codec_context->pix_fmt;
        AVFrame	*frameRGB = av_frame_alloc();
        frameRGB->format = AV_PIX_FMT_RGB24;
        frameRGB->width = codecpar->width;
        frameRGB->height = codecpar->height;
        err = av_frame_get_buffer(frameRGB, 0);
        Q_ASSERT(err == 0);
        ///
        SwsContext *imgConvertCtx = nullptr;

        AVFrame* frame = av_frame_alloc();
        AVPacket packet;

        while (av_read_frame(fmt_ctx, &packet) >= 0) {
            avcodec_send_packet(codec_context, &packet);
            err = avcodec_receive_frame(codec_context, frame);
            if (err == 0) {
                //qDebug() << frame->height << frame->width << codec_context->pix_fmt;

                imgConvertCtx = sws_getCachedContext(imgConvertCtx,
                                                     codecpar->width, codecpar->height, static_cast<AVPixelFormat>(codecpar->format),
                                                     frameRGB->width, frameRGB->height,
                                                     AV_PIX_FMT_RGB24, SWS_BICUBIC, nullptr, nullptr, nullptr);
                Q_ASSERT(imgConvertCtx);

                //conversion frame to frameRGB
                sws_scale(imgConvertCtx, frame->data, frame->linesize, 0, codec_context->height, frameRGB->data, frameRGB->linesize);

                //setting QImage from frameRGB
                QImage image(frameRGB->data[0],
                        frameRGB->width,
                        frameRGB->height,
                        frameRGB->linesize[0],
                        QImage::Format_RGB888);

                emit updateImage(image);
            }

            av_frame_unref(frame);
            av_packet_unref(&packet);
        }
        if (imgConvertCtx) sws_freeContext(imgConvertCtx);

        //end:
        avformat_close_input(&fmt_ctx);
    });
}
*/
