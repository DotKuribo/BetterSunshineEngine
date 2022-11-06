#pragma once

#include <Dolphin/AI.h>
#include <Dolphin/DVD.h>
#include <Dolphin/OS.h>
#include <Dolphin/types.h>
#include <JSystem/JKernel/JKRHeap.hxx>
#include <JSystem/JSupport/JSUStream.hxx>
#include <SMS/System/Params.hxx>
#include <SMS/MSound/MSBGM.hxx>

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

        void setVolume(u8 left, u8 right);
        void setVolumeFade(u8 dstVolume, f32 seconds);

        void setLoopPoint(f32 start, f32 length);

        class AudioStreamer;
        AudioStreamer *getAudioStreamer();

        /*
        / Audio streamer
        */

        constexpr size_t AudioMessageQueueSize = 16;
        constexpr size_t AudioQueueSize        = 4;
        constexpr size_t AudioStackSize        = 0x4000;
        constexpr u8 AudioVolumeDefault        = 127;
        constexpr size_t AudioStreamRate       = 48000;
        constexpr size_t AudioPreparePreOffset = 0x8000;

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
                FADE_OUT,
                FADE_IN
            };

            struct AudioPacket {
                friend class AudioStreamer;

                union Identifier {
                    u32 as_u32;
                    const char *as_string;
                };

                struct PacketParams : public TParams {
                    PacketParams()
                        : TParams(), SMS_TPARAM_INIT(mLoopStart, 0xFFFFFFFF),
                          SMS_TPARAM_INIT(mLoopEnd, 0xFFFFFFFF) {}

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

                void setLoopPoint(s32 start, size_t length);
                void setLoopPoint(f32 start, f32 length);

            private:
                bool exec(DVDFileInfo *handle);

            private:
                bool mIsString;
                Identifier mIdentifier;
                PacketParams mParams;
            };

        public:
            AudioStreamer(void *(*mainLoop)(void *), OSPriority priority, DVDFileInfo *fInfo);
            ~AudioStreamer();

            static AudioStreamer *getInstance() { return &sInstance; }

            void setLooping(bool loop);

            bool isPlaying() const;
            bool isPaused() const;
            bool isLooping() const;

            bool isLoopCustom() const {
                auto packet = _mAudioQueue[_mAudioIndex];
                return packet.mParams.mLoopStart.get() != 0xFFFFFFFF ||
                       packet.mParams.mLoopEnd.get() != 0xFFFFFFFF;
            }

            u32 getStreamPos() const { return mCurrentPlayAddress; }
            u32 getStreamStart() const { return mAudioHandle->mStart; }
            u32 getStreamEnd() const { return mEndPlayAddress; }

            u32 getLoopStart() const {
                auto packet = _mAudioQueue[_mAudioIndex];
                return packet.mParams.mLoopStart.get() != 0xFFFFFFFF
                           ? packet.mParams.mLoopStart.get()
                           : 0;
            }

            u32 getLoopEnd() const {
                auto packet = _mAudioQueue[_mAudioIndex];
                return packet.mParams.mLoopEnd.get() != 0xFFFFFFFF ? packet.mParams.mLoopEnd.get()
                                                                   : mAudioHandle->mLen;
            }

            AudioPacket &getCurrentAudio() { return _mAudioQueue[_mAudioIndex]; }
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

            bool play_();
            bool pause_();
            bool stop_();
            bool skip_();
            void next_();
            bool seek_();
            void update_();

        private:
            void initThread(OSPriority);

        public:
            OSThread mMainThread;
            OSAlarm mVolumeFadeAlarm;
            OSMessageQueue mMessageQueue;
            OSMessage mMessageList[AudioMessageQueueSize];
            DVDFileInfo *mAudioHandle;
            DVDCommandBlock mGetAddrCmd;
            DVDCommandBlock mStopCmd;
            DVDCommandBlock mStatusCmd;
            u8 *mAudioStack;
            u32 mCurrentPlayAddress;
            u32 mEndPlayAddress;
            u32 mErrorStatus;

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
}  // namespace BetterSMS::Music