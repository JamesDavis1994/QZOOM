#include "inputstreamhandler.h"

InputStreamHandler::InputStreamHandler(ImageHandler* imageHandler, int bufferSize, QHostAddress address, QObject *parent) : QObject(parent)
{
    mImageHandler = imageHandler;
    mBufferSize = bufferSize;
    mAddress = address;
}

void InputStreamHandler::init()
{
    //new TcpServerHandler();
    //TcpServerHandler.init();
}

void InputStreamHandler::close()
{
    for(auto i : mVideoHeaderVector)
        delete i;
    mVideoHeaderVector.clear();

    for(auto i : mAudioBufferVector)
        delete i;
    mAudioBufferVector.clear();

    for(auto i : mVideoBufferVector)
        delete i;
    mVideoBufferVector.clear();

    for(auto i : mAudioMutexVector)
        delete i;
    mAudioMutexVector.clear();

    for(auto i : mVideoMutexVector)
        delete i;
    mVideoMutexVector.clear();

    for(auto i : mAudioPlaybackHandlerVector)
        delete i;
    mAudioPlaybackHandlerVector.clear();

    for(auto i : mVideoPlaybackHandlerVector)
        delete i;
    mVideoPlaybackHandlerVector.clear();
}

void InputStreamHandler::removeStream(QString streamId)
{
    qDebug() << "Trying to remove user with streamId: " << streamId;
    int index = -1;

    for(size_t i=0;i<mStreamIdVector.size();i++)
    {
        if(QString::compare(streamId, mStreamIdVector[i], Qt::CaseSensitive)==0)
        {
            index = i;
        }
    }
    if(index != -1)
    {
        delete mVideoHeaderVector.at(index);
        delete mAudioBufferVector.at(index);
        delete mVideoBufferVector.at(index);
        delete mAudioMutexVector.at(index);
        delete mVideoMutexVector.at(index);
        delete mAudioPlaybackHandlerVector.at(index);
        delete mVideoPlaybackHandlerVector.at(index);
        mVideoHeaderVector.erase(mVideoHeaderVector.begin() + index);
        mAudioBufferVector.erase(mAudioBufferVector.begin() + index);
        mVideoBufferVector.erase(mVideoBufferVector.begin() + index);
        mAudioMutexVector.erase(mAudioMutexVector.begin() + index);
        mVideoMutexVector.erase(mVideoMutexVector.begin() + index);
        mAudioPlaybackHandlerVector.erase(mAudioPlaybackHandlerVector.begin() + index);
        mVideoPlaybackHandlerVector.erase(mVideoPlaybackHandlerVector.begin() + index);
        mAudioPlaybackStartedVector.erase(mAudioPlaybackStartedVector.begin() + index);
        mVideoPlaybackStartedVector.erase(mVideoPlaybackStartedVector.begin() + index);
        mStreamIdVector.erase(mStreamIdVector.begin() + index);
        qDebug() << "Successfully removed stream with streamId: " << streamId;
    }
    else
    {
        qDebug() << "Could not find stream with streamId " << streamId << " when trying to remove it";
    }
}

/**
 * Reads the streamId from the data and removes it, then finds the matching index
 * or creates a new one and appends the header data to the appropriate buffer.
 * @param data QByteArray header data recieved from the TCP request
 */

void InputStreamHandler::handleHeader(QByteArray data)
{
    int streamIdLength = data[0];
    data.remove(0,1);

    //Finds the streamId header, stores it and removes it;
    QByteArray streamIdArray = QByteArray(data, streamIdLength);
    QString streamId(streamIdArray);
    data.remove(0, streamIdLength);

    int index = findStreamIdIndex(streamId);

    mVideoMutexVector[index]->lock();
    if(mVideoHeaderVector[index]->isEmpty())
    {
        mVideoHeaderVector[index]->append(data);
        mVideoBufferVector[index]->append(data);
    }
    mVideoMutexVector[index]->unlock();
}

/**
 * When recieving a TCP request with a header from a unknown streamId,
 * we need to create new buffers, mutex and playbackhandlers for both video and audio.
 * @param streamId QString to add to mStreamIdVector
 * @param index int to add a peer to ImageHandler
 */
void InputStreamHandler::addStreamToVector(QString streamId,int index)
{
    qDebug() << "Adding streamId: " << streamId;
    QByteArray* tempVideoHeaderBuffer = new QByteArray();
    mVideoHeaderVector.push_back(tempVideoHeaderBuffer);
    QByteArray* tempAudioBuffer = new QByteArray();
    QByteArray* tempVideoBuffer = new QByteArray();
    mAudioBufferVector.push_back(tempAudioBuffer);
    mVideoBufferVector.push_back(tempVideoBuffer);
    std::mutex* tempAudioLock = new std::mutex;
    std::mutex* tempVideoLock = new std::mutex;
    mAudioMutexVector.push_back(tempAudioLock);
    mVideoMutexVector.push_back(tempVideoLock);
    mAudioPlaybackHandlerVector.push_back(new AudioPlaybackHandler(tempAudioLock,tempAudioBuffer,mBufferSize));
    mVideoPlaybackHandlerVector.push_back(new VideoPlaybackHandler(tempVideoLock,mImageHandler, tempVideoHeaderBuffer, tempVideoBuffer,mBufferSize,index+1));
    mStreamIdVector.push_back(streamId);
    mAudioPlaybackStartedVector.push_back(false);
    mVideoPlaybackStartedVector.push_back(false);

    //Your own image is at 0, so we add 1 here and in videoPlayback constructor
    mImageHandler->addPeer(index+1);
}


/**
 * When recieving a TCP request we need to know if the streamId already exists in mStreamIdVector.
 * @param streamId QString to find in mStreamIdVector
 * @return int index where streamId was found, or 0 if not found
 */
int InputStreamHandler::findStreamIdIndex(QString streamId)
{
    qDebug() << "InputStreamHandler findStreamId index: " << streamId;
    if(mStreamIdVector.size()>=1)
    {
        for(size_t i=0;i<mStreamIdVector.size();i++)
        {
            if(QString::compare(streamId, mStreamIdVector[i], Qt::CaseSensitive)==0)
            {
                return i;
            }
        }
        //If the streamId does not exist, push it and buffers/locks
        addStreamToVector(streamId, mStreamIdVector.size());
        return mStreamIdVector.size() - 1;
    }
    else
    {
        //If this stream is the first to join, push it and buffer/locks
        addStreamToVector(streamId,0);
        return 0;
    }

}

