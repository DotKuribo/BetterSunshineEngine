#pragma once

#include <Dolphin/AI.h>
#include <Dolphin/DVD.h>
#include <Dolphin/OS.h>
#include <Dolphin/types.h>
#include <JSystem/JKernel/JKRHeap.hxx>
#include <JSystem/JSupport/JSUStream.hxx>
#include <SMS/sound/MSBGM.hxx>

namespace BetterSMS::Music {
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

    void setLoopPoint(f64 start, f64 length);

    class AudioStreamer;
    AudioStreamer *getAudioStreamer();

    /*
    / Audio streamer
    */

    constexpr size_t AudioMessageQueueSize = 16;
    constexpr size_t AudioQueueSize        = 4;
    constexpr size_t AudioStackSize        = 0x4000;
    constexpr u8 AudioVolumeDefault        = 127;

    class AudioStreamer {
    public:
        enum class AudioCommand { NONE, PLAY, PAUSE, STOP, SKIP, NEXT, SEEK, FADE_OUT, FADE_IN };

        struct AudioPacket {
            friend class AudioStreamer;

            union Identifier {
                u32 as_u32;
                const char *as_string;
            };

            AudioPacket() : mLoopStart(-1), mLoopEnd(-1) { mIdentifier.as_u32 = 0xFFFFFFFF; }

            AudioPacket(u32 id) : mLoopStart(-1), mLoopEnd(-1) {
                mIdentifier.as_u32 = id;
                mIstring           = false;
            }

            AudioPacket(const char *file) : mLoopStart(-1), mLoopEnd(-1) {
                mIdentifier.as_string = file;
                mIstring              = true;
            }

            bool exec(DVDFileInfo *handle);
            void setLoopPoint(s32 start, size_t length);
            void setLoopPoint(f64 start, f64 length);

            bool mIstring;
            Identifier mIdentifier;

        private:
            s32 mLoopStart;
            s32 mLoopEnd;
        };

    public:
        AudioStreamer(void *(*mainLoop)(void *), OSPriority priority, DVDFileInfo *fInfo,
                      DVDCommandBlock *cb);
        ~AudioStreamer();

        static AudioStreamer *getInstance() { return &sInstance; }
        AudioPacket &getCurrentAudio() { return mAudioQueue[mAudioIndex]; }
        u16 getVolumeLR() const { return (mVolLeft << 8) | mVolRight; }
        u8 getFullVolumeLR() const { return (mFullVolLeft << 8) | mFullVolRight; }

        void setLooping(bool loop) { mIsLooping = loop; }

        bool isPlaying() const { return mIsPlaying; }
        bool isPaused() const { return mIsPaused; }
        bool isLooping() const { return mIsLooping; }

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
        void seek(f64 seconds, JSUStreamSeekFrom whence);

        bool play_();
        bool pause_();
        bool stop_();
        bool skip_();
        void next_();
        bool seek_();
        void update_();

        OSThread mMainThread;
        OSAlarm mVolumeFadeAlarm;
        OSMessageQueue mMessageQueue;
        OSMessage mMessageList[AudioMessageQueueSize];
        DVDFileInfo *mAudioHandle;
        DVDCommandBlock *mAudioCommandBlock;
        u8 *mAudioStack;

    private:
        AudioPacket mAudioQueue[AudioQueueSize];
        s32 mAudioIndex;
        f32 mDelayedTime;
        f32 mFadeTime;
        u8 mSrcVolume;
        u8 mTargetVolume;
        u8 mVolLeft;
        u8 mVolRight;
        u8 mFullVolLeft;
        u8 mFullVolRight;
        u8 mPreservedVolLeft;
        u8 mPreservedVolRight;

        s32 _mWhere;
        JSUStreamSeekFrom _mWhence;

        bool mIsPlaying;
        bool mIsPaused;
        bool mIsLooping;

        static AudioStreamer sInstance;
    };

    bool isValidBGM(u32 id);
    bool isValidBGM(MSStageInfo id);
    bool isWeakBGM(u32 id);
    bool isWeakBGM(MSStageInfo id);
}  // namespace BetterSMS::Music