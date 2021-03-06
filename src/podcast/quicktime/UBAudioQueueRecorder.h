/*
 * UBAudioQueueRecorder.h
 *
 *  Created on: Sep 11, 2009
 *      Author: luc
 */

#ifndef UBAUDIOQUEUERECORDER_H_
#define UBAUDIOQUEUERECORDER_H_

#include <QtCore>

#include <ApplicationServices/ApplicationServices.h>
#include <AudioToolbox/AudioToolbox.h>

class UBAudioQueueRecorder : public QObject
{
    Q_OBJECT;

    public:
        UBAudioQueueRecorder(QObject* pObject = 0);
        virtual ~UBAudioQueueRecorder();

        bool init(const QString& waveInDeviceName = QString(""));
        bool close();

        static QStringList waveInDevices();

        static AudioDeviceID deviceIDFromDeviceName(const QString& name);
        static QString deviceNameFromDeviceID(AudioDeviceID id);
        static QString deviceUIDFromDeviceID(AudioDeviceID id);
        static QList<AudioDeviceID> inputDeviceIDs();
        static AudioDeviceID defaultInputDeviceID();

        QString lastErrorMessage()
        {
            return mLastErrorMessage;
        }

        static AudioStreamBasicDescription audioFormat()
        {
            return sAudioFormat;
        }

    signals:

        void newWaveBuffer(void* pBuffer, long pLength, int inNumberPacketDescriptions, const AudioStreamPacketDescription *inPacketDescs);

        void audioLevelChanged(quint8 level);

    private:
        static void audioQueueInputCallback (void *inUserData, AudioQueueRef inAQ,
            AudioQueueBufferRef inBuffer, const AudioTimeStamp *inStartTime,
            UInt32 inNumberPacketDescriptions, const AudioStreamPacketDescription *inPacketDescs);

        void emitNewWaveBuffer(AudioQueueBufferRef pBuffer, int inNumberPacketDescriptions, const AudioStreamPacketDescription *inPacketDescs);

        void emitAudioLevelChanged(quint8 level);

        void setLastErrorMessage(const QString& pLastErrorMessage)
        {
            mLastErrorMessage = pLastErrorMessage;
            qWarning() << "UBAudioQueueRecorder error:" << mLastErrorMessage;
        }

        bool isRecording() const
        {
            return mIsRecording;
        }

        AudioQueueRef mQueue;
        QList<AudioQueueBufferRef> mBuffers;
        QString mLastErrorMessage;
        volatile bool mIsRecording;
        int mBufferLengthInMs;
        volatile long mMsTimeStamp;
        UInt32 mSampleBufferSize;

        static AudioStreamBasicDescription sAudioFormat;
};

#endif /* UBAUDIOQUEUERECORDER_H_ */
