#include "tcpsockethandler.h"


TcpSocketHandler::TcpSocketHandler(InputStreamHandler* inputStreamHandler, SessionHandler* sessionHandler, QHostAddress address, int port, QObject* parent): QObject(parent)
{
    mAddress = address;
    mPort = port;
    qDebug() << mAddress;
    qDebug() << "Tcp port" << mPort;
    mInputStreamHandler = inputStreamHandler;
    mSessionHandler = sessionHandler;
}

void TcpSocketHandler::init()
{
    mSocket = new QTcpSocket(this);
    /*connect(mSocket, SIGNAL(connected()), this, SLOT(connected()));
    connect(mSocket, SIGNAL(disconnected()), this, SLOT(disconnected()));
    connect(mSocket, SIGNAL(readyRead()), this, SLOT(readyRead()));
*/
    connect(mSocket, SIGNAL(bytesWritten(qint64)), this, SLOT(bytesWritten(qint64)));







}

void TcpSocketHandler::writeLeaveSignal()
{
    mSocket->connectToHost(mAddress, mPort);
    if(!mSocket->waitForConnected(3000))
    {
        qDebug() << "TcpSocketError: " << mSocket->errorString();
    }
    mSocket->write(QByteArray("-1"));
}

//Send header to server, and receive headers from other participants back
void TcpSocketHandler::writeHeader()
{
    static bool firstRound = true;
    //mSocket->connectToHost(mAddress, mPort);
    mSocket->connectToHost(mAddress, mPort);
    //qDebug() << "After ConnectToHost, addr: " << mAddress << " port: " << mPort;
    if(!mSocket->waitForConnected(3000))
    {
        qDebug() << "TcpSocketError: " << mSocket->errorString();
    }

    if(firstRound)
    {
        QString streamId = mSessionHandler->getUser()->getStreamId();
        QString roomId = mSessionHandler->getRoomId();

        myHeader.prepend(streamId.toLocal8Bit().data());
        myHeader.prepend(streamId.size());

        //Puts the roomId and its size at the front of the array
        myHeader.prepend(roomId.toLocal8Bit().data());
        myHeader.prepend(roomId.size());

        firstRound = false;
    }



    //qDebug() << "My Header: " << myHeader.length() << "\n" << myHeader;


    mSocket->write(myHeader);
    myHeader.clear();
    //mSocket->write("HEAD / HTTP/1.0\r\n\r\n\r\n\r\n");
    while (mSocket->waitForReadyRead(3000));

    QByteArray reply = mSocket->readAll();

    //qDebug() << "Reply from Server: \n" << reply;
    if(reply.size() <= 0)
    {
        qDebug() << "Reply from tcp request was empty, should not happen @ " << Q_FUNC_INFO;
        return;
    }
    else if(reply.size()==1)
    {
        int returnCode = reply[0];
        switch(returnCode)
        {
        case mTcpReturnValues::SESSION_STARTED:
            //TODO how to handle session started and nothing returned from server
            qDebug() << "Server responded with session started";
            return;
        case mTcpReturnValues::ROOM_ID_NOT_FOUND:
            //TODO handle wrong roomId
            qDebug() << "Server did not find roomId";
            return;
        case mTcpReturnValues::STREAM_ID_NOT_FOUND:
            qDebug() << "Server did not find streamId";
            //TODO handle wrong streamId
            return;
        default:
            qDebug() << "Unkown return code from tcp server @ " << Q_FUNC_INFO;
            exit(-1);
        }
    }

    int numOfHeaders = reply[0];
    qDebug() << "number of headers recieved from server: " << numOfHeaders;
    reply.remove(0,1);
    //QString data(reply);


    //qDebug() << "DataString: " << data;
    //qDebug() << reply.indexOf(27);


    for(int i = 0; i < numOfHeaders; i++)
    {
        QByteArray temp = QByteArray(reply, reply.indexOf(27));
        //qDebug() << "Temp: " << temp;
        mInputStreamHandler->handleHeader(temp);
        reply.remove(0, reply.indexOf(27));
    }

    //mSocket->write("0");

    mSocket->close();
}


void TcpSocketHandler::connected()
{
    qDebug() << "Tcp socket connected to " << mAddress << "on port " << mPort;
    qDebug() << "TCP Request: " << mRequest;
    //mSocket->write(mRequest);
}

void TcpSocketHandler::disconnected()
{
    qDebug() << "TCPSocket disconnected";
}

void TcpSocketHandler::bytesWritten(qint64 bytes)
{
    qDebug() << "Tcpsocket wrote " << bytes << " bytes";
    mBytesWritten = bytes;
}

void TcpSocketHandler::wait()
{
    while (mSocket->waitForReadyRead(1000));
}

int TcpSocketHandler::getBytesWritten()
{
    return mBytesWritten;
}

void TcpSocketHandler::readyRead()
{
    qDebug() << "TcpSocket reading...";

    //qDebug() << mSocket->readAll();
    mReply = mSocket->readAll();
    mReady = true;
}

QByteArray TcpSocketHandler::getReply()
{
    return mReply;
}

bool TcpSocketHandler::isReady()
{
    return mReady;
}
