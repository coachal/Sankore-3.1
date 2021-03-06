/*
 * UBWindowsMediaVideoEncoder.cpp
 *
 *  Created on: 7 sept. 2009
 *      Author: Luc
 */

#include "UBQuickTimeVideoEncoder.h"

#include <QuickTime/QuickTime.h>

#include <QtGui>

#include "frameworks/UBDesktopServices.h"
#include "frameworks/UBFileSystemUtils.h"

#include "core/UBApplication.h"
#include "core/UBSettings.h"
#include "core/UBSetting.h"

#include "UBQuickTimeFile.h"


UBQuickTimeVideoEncoder::UBQuickTimeVideoEncoder(QObject* pParent)
    : UBAbstractVideoEncoder(pParent)
    , mQuickTimeCompressionSession(0)
    , mShouldRecordAudio(true)

{
    // NOOP
}


UBQuickTimeVideoEncoder::~UBQuickTimeVideoEncoder()
{
    // NOOP
}


bool UBQuickTimeVideoEncoder::start()
{
    QString quality = UBSettings::settings()->podcastQuickTimeQuality->get().toString();

    if(!mQuickTimeCompressionSession.init(videoFileName(), quality, framesPerSecond(), videoSize()
                , mShouldRecordAudio, audioRecordingDevice()))
    {
        setLastErrorMessage("Cannot init QT compression session" + mQuickTimeCompressionSession.lastErrorMessage());
        return false;
    }

    connect(&mQuickTimeCompressionSession, SIGNAL(finished()), this, SLOT(compressionFinished()));
    connect(&mQuickTimeCompressionSession, SIGNAL(audioLevelChanged(quint8)), this, SIGNAL(audioLevelChanged(quint8)));

    mQuickTimeCompressionSession.start();

    return true;
}


bool UBQuickTimeVideoEncoder::stop()
{
    if (mQuickTimeCompressionSession.isRunning())
    {
        mQuickTimeCompressionSession.stop();
    }

    UBQuickTimeFile::frameBufferNotEmpty.wakeAll();

    return true;
}


void UBQuickTimeVideoEncoder::compressionFinished()
{
        mLastErrorMessage = mQuickTimeCompressionSession.lastErrorMessage();

        emit encodingFinished(mLastErrorMessage.length() > 0);
}


void UBQuickTimeVideoEncoder::newPixmap(const QImage& pImage, long timestamp)
{
        //qDebug() << "New Frame at ms" << timestamp;

    if(mQuickTimeCompressionSession.isCompressionSessionRunning())
    {
        if(mPendingImageFrames.length() > 0)
        {
            foreach(ImageFrame frame, mPendingImageFrames)
            {
                    encodeFrame(frame.image, frame.timestamp);
            }

            mPendingImageFrames.clear();
        }

        encodeFrame(pImage, timestamp);

        UBQuickTimeFile::frameBufferNotEmpty.wakeAll();
    }
    else
    {
        qDebug() << "queuing frame, compressor not ready";

        ImageFrame frame;
        frame.image = pImage;
        frame.timestamp = timestamp;

        mPendingImageFrames << frame;
    }
}

void UBQuickTimeVideoEncoder::encodeFrame(const QImage& pImage, long timestamp)
{
    Q_ASSERT(pImage.format() == QImage::QImage::Format_RGB32);  // <=> CVPixelBuffers / k32BGRAPixelFormat

    CVPixelBufferRef pixelBuffer = mQuickTimeCompressionSession.newPixelBuffer();
    if (!pixelBuffer)
    {
        setLastErrorMessage("Could not retreive a new pixel buffer");
        return;
    }

    if (CVPixelBufferLockBaseAddress(pixelBuffer, 0) != kCVReturnSuccess)
    {
        setLastErrorMessage("Could not lock pixel buffer base address");
        return;
    }

    void *pixelBufferAddress = CVPixelBufferGetBaseAddress(pixelBuffer);

    if (!pixelBufferAddress)
    {
        setLastErrorMessage("Could not get pixel buffer base address");
        return;
    }

    const uchar* imageBuffer = pImage.bits();

    memcpy((void*) pixelBufferAddress, imageBuffer, pImage.numBytes());

    CVPixelBufferUnlockBaseAddress(pixelBuffer, 0);

    //qDebug() << "newVideoFrame - PixelBuffer @" << pixelBufferAddress << QTime::currentTime().toString("ss:zzz") << QThread::currentThread();

    UBQuickTimeFile::VideoFrame videoFrame;
    videoFrame.buffer = pixelBuffer;
    videoFrame.timestamp = timestamp;

    UBQuickTimeFile::frameQueueMutex.lock();
    UBQuickTimeFile::frameQueue.enqueue(videoFrame);
    UBQuickTimeFile::frameQueueMutex.unlock();
}


void UBQuickTimeVideoEncoder::setRecordAudio(bool pRecordAudio)
{
    mShouldRecordAudio = pRecordAudio;
}



