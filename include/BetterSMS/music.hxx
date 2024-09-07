#pragma once

#include <Dolphin/AI.h>
#include <Dolphin/DVD.h>
#include <Dolphin/OS.h>
#include <Dolphin/types.h>
#include <JSystem/JKernel/JKRHeap.hxx>
#include <JSystem/JSupport/JSUStream.hxx>
#include <SMS/MSound/MSBGM.hxx>
#include <SMS/System/Params.hxx>

namespace BetterSMS {

    namespace Music {
        /*
        / General interface
        */

        // Name of the song to play, e.g. "BeachTheme"
        bool queueSong(const char *name);
        // Play a paused/queued song
        void playSong();
        // Pause song, fading out by `fadeTime` seconds
        void pauseSong(f32 fadeTime);
        void stopSong(f32 fadeTime);
        void skipSong(f32 fadeTime);

        bool isLooping();
        bool isPlaying();
        bool isPaused();

        void setVolume(u8 left, u8 right);
        void setVolumeFade(u8 dstVolume, f32 seconds);

        u8 getMaxVolume();
        void setMaxVolume(u8 max);

        void setLooping(bool loop);
        void setLoopPoint(s32 start, s32 end);

        /*
        / Audio streamer
        */

        constexpr OSPriority AudioThreadPriority = OSPriority(18);
        constexpr size_t AudioMessageQueueSize   = 16;
        constexpr size_t AudioQueueSize          = 4;
        constexpr size_t AudioStackSize          = 0x4000;
        constexpr u8 AudioVolumeDefault          = 127;
        #if 0
        constexpr size_t AudioInterruptRate         = 14300;  // 32kHz, granular for stupid reasons
        #else
        constexpr size_t AudioInterruptRate         = 2000;
        #endif
        constexpr size_t AudioPreparePreOffset   = 0x8000;
        constexpr OSTime AudioFadeInterval       = 16;  // 16ms

        struct AudioPacket {
            union Identifier {
                u32 as_u32;
                const char *as_string;
            };

            struct PacketParams : public TParams {
                PacketParams()
                    : TParams(), SMS_TPARAM_INIT(mLoopStart, -1),
                      SMS_TPARAM_INIT(mLoopEnd, -1) {}

                TParamT<s32> mLoopStart;
                TParamT<s32> mLoopEnd;
            };

            AudioPacket() : mParams() { mIdentifier.as_u32 = 0xFFFFFFFF; }

            AudioPacket(u32 id) : mParams() {
                mIdentifier.as_u32 = id;
                mIsString          = false;
            }

            AudioPacket(const char *file) : mParams() {
                mIdentifier.as_string = file;
                mIsString             = true;
            }

            bool isString() const { return mIsString; }
            const char *getString() const { return mIdentifier.as_string; }
            u32 getID() const { return mIdentifier.as_u32; }

            s32 getLoopStart() const { return mParams.mLoopStart.get(); }
            s32 getLoopEnd() const { return mParams.mLoopEnd.get(); }

            void setLoopPoint(s32 start, s32 length);
            void setLoopPoint(s32 start, size_t length);
            void setLoopPoint(f32 start, f32 length);

        private:
            bool mIsString;
            Identifier mIdentifier;
            PacketParams mParams;
        };

        class AudioStreamer {
        public:
            enum class AudioCommand {
                NONE,
                PLAY,
                PAUSE,
                STOP,
                SKIP,
                NEXT,
                SEEK,
                CLEAR,
                FADE_OUT,
                FADE_IN
            };

        public:
            AudioStreamer(OSPriority priority, DVDFileInfo *fInfo);
            ~AudioStreamer();

            static AudioStreamer *getInstance() { return &sInstance; }
            void mainLoop_(void *param);

            bool isPlaying() const;
            bool isPaused() const;

            bool isLooping() const;
            void setLooping(bool loop);

            u32 getErrorStatus() const { return mErrorStatus; }

            u32 getStreamLength() const { return mStreamEnd - mStreamPos; }
            u32 getStreamPos() const { return mStreamPos + getStreamStart(); }
            u32 getStreamStart() const { return mAudioHandle->mStart; }
            u32 getStreamEnd() const { return mStreamEnd + getStreamStart(); }

            bool isLoopCustom() const;
            u32 getLoopStart() const;
            u32 getLoopEnd() const;

            AudioPacket &getCurrentAudio() { return _mAudioQueue[_mAudioIndex]; }
            const AudioPacket &getCurrentAudio() const { return _mAudioQueue[_mAudioIndex]; }

            u16 getVolumeLR() const { return (_mVolLeft << 8) | _mVolRight; }
            u8 getFullVolumeLR() const { return (_mFullVolLeft << 8) | _mFullVolRight; }

            void setVolumeLR(u8 left, u8 right);
            void setFullVolumeLR(u8 left, u8 right);
            void resetVolumeToFull();

            void setVolumeFadeTo(u8 to, f32 seconds);

            bool queueAudio(const AudioPacket &packet);
            void fadeAudio_();

            void play();
            void pause(f32 fadeTime);
            void stop(f32 fadeTime);
            void skip(f32 fadeTime);
            void next(f32 fadeTime);
            void seek(s32 where, JSUStreamSeekFrom whence);
            void seek(f32 seconds, JSUStreamSeekFrom whence);
            void clear();

        private:
            void initThread(void *(*threadMain)(void *), OSPriority);
            void initializeSubsystem();
            void deinitalizeSubsystem();

            bool play_();
            bool pause_();
            bool stop_();
            bool skip_();
            void next_();
            bool seek_();
            void clear_();
            void update_();

            void nextTrack_();

            bool startLowStream();
            void pauseLowStream();
            bool seekLowStream(s32 streamPos);
            bool stopLowStream();

            bool canPlayNextTrack() const;

            static void cbForVolumeAlarm(OSAlarm *alarm, OSContext *context);
            static void cbForAIInterrupt(u32 trigger);
            static void cbForGetStreamErrorStatusAsync_(u32 result, DVDCommandBlock *cmdBlock);
            static void cbForGetStreamPlayAddrAsync_(u32 result, DVDCommandBlock *cmdBlock);
            static void cbForPrepareStreamAsync_(u32 result, DVDFileInfo *finfo);
            static void cbForCancelStreamOnStopAsync_(u32 result, DVDCommandBlock *callback);
            static void cbForCancelStreamOnSeekAsync_(u32 result, DVDCommandBlock *callback);
            static void cbForStopStreamAtEndAsync_(u32 result, DVDCommandBlock *cmdblock);

        protected:
            OSThread mMainThread;
            OSAlarm mVolumeFadeAlarm;
            OSMessageQueue mMessageQueue;
            OSMessage mMessageList[AudioMessageQueueSize];
            DVDFileInfo *mAudioHandle;
            mutable DVDCommandBlock mAIInteruptBlock;
            mutable DVDCommandBlock mRunBlock;
            mutable DVDCommandBlock mPrepareBlock;
            mutable DVDCommandBlock mSeekBlock;
            mutable DVDCommandBlock mPauseBlock;
            mutable DVDCommandBlock mStopBlock;
            mutable DVDCommandBlock mPlayAddrBlock;
            mutable DVDCommandBlock mCanPlayBlock;
            u8 *mAudioStack;
            u32 mStreamPos;
            u32 mStreamEnd;
            u32 mErrorStatus;
            u32 mLastErrorStatus;
            bool mPlayNextTrack;
            bool mRequestedNext;
            bool mRequestedStop;
            bool mRequestedPlay;
            bool mRequestedPause;
            bool mRequestedClear;

        private:
            AudioPacket _mAudioQueue[AudioQueueSize];
            s32 _mAudioIndex;
            f32 _mDelayedTime;
            f32 _mFadeTime;
            u8 _mSrcVolume;
            u8 _mTargetVolume;
            u8 _mVolLeft;
            u8 _mVolRight;
            u8 _mFullVolLeft;
            u8 _mFullVolRight;
            u8 _mPreservedVolLeft;
            u8 _mPreservedVolRight;

            s32 _mWhere;
            JSUStreamSeekFrom _mWhence;

            static AudioStreamer sInstance;
        };

        bool isValidBGM(u32 id);
        bool isValidBGM(MSStageInfo id);
        bool isWeakBGM(u32 id);
        bool isWeakBGM(MSStageInfo id);

    }  // namespace Music

}  // namespace BetterSMS