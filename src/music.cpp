
#include <Dolphin/DVD.h>
#include <Dolphin/OS.h>
#include <Dolphin/printf.h>
#include <Dolphin/string.h>
#include <JSystem/JSupport/JSUStream.hxx>
#include <SMS/Player/Mario.hxx>

#include <SMS/Camera/PolarSubCamera.hxx>
#include <SMS/MSound/MSBGM.hxx>
#include <SMS/System/Application.hxx>
#include <SMS/macros.h>
#include <SMS/raw_fn.hxx>

#include "libs/constmath.hxx"
#include "libs/lock.hxx"

#include "logging.hxx"
#include "module.hxx"
#include "music.hxx"
#include "stage.hxx"

using namespace BetterSMS;
using namespace BetterSMS::Music;

// Name of the song to play, e.g. "BeachTheme"
BETTER_SMS_FOR_EXPORT bool Music::queueSong(const char *name) {
    return AudioStreamer::getInstance()->queueAudio(AudioPacket(name));
}

// Play a paused/queued song
BETTER_SMS_FOR_EXPORT void Music::playSong() { AudioStreamer::getInstance()->play(); }

// Pause song, fading out by `fadeTime` seconds
BETTER_SMS_FOR_EXPORT void Music::pauseSong(f32 fadeTime) {
    return AudioStreamer::getInstance()->pause(fadeTime);
}

BETTER_SMS_FOR_EXPORT void Music::stopSong(f32 fadeTime) {
    return AudioStreamer::getInstance()->next(fadeTime);
}

BETTER_SMS_FOR_EXPORT void Music::skipSong(f32 fadeTime) {
    return AudioStreamer::getInstance()->skip(fadeTime);
}

BETTER_SMS_FOR_EXPORT void Music::setVolume(u8 left, u8 right) {
    AudioStreamer::getInstance()->setVolumeLR(left, right);
}

BETTER_SMS_FOR_EXPORT void Music::setVolumeFade(u8 dstVolume, f32 seconds) {
    AudioStreamer::getInstance()->setVolumeFadeTo(dstVolume, seconds);
}

BETTER_SMS_FOR_EXPORT void Music::setMaxVolume(u8 max) {
    AudioStreamer::getInstance()->setFullVolumeLR(max, max);
}

BETTER_SMS_FOR_EXPORT void Music::setLoopPoint(f32 start, f32 length) {
    AudioPacket &packet = AudioStreamer::getInstance()->getCurrentAudio();
    packet.setLoopPoint(start, length);
}

#pragma region Implementation

static SMS_ALIGN(32) DVDFileInfo sAudioFInfo;

static bool _startPaused = false;
static bool _mIsPlaying  = false;
static bool _mIsPaused   = false;
static bool _mIsLooping  = false;

SMS_NO_INLINE static void *threadMain_(void *param) {
    AudioStreamer *streamer = reinterpret_cast<Music::AudioStreamer *>(param);
    streamer->mainLoop_(param);
    return nullptr;
}

static AudioStreamer::AudioCommand sAudioCommand = AudioStreamer::AudioCommand::NONE;

bool AudioStreamer::isPlaying() const { return _mIsPlaying; }
bool AudioStreamer::isPaused() const { return _mIsPaused; }
bool AudioStreamer::isLooping() const { return _mIsLooping; }

AudioStreamer AudioStreamer::sInstance = AudioStreamer(AudioThreadPriority, &sAudioFInfo);

AudioStreamer::AudioStreamer(OSPriority priority, DVDFileInfo *fInfo)
    : mAudioHandle(fInfo), mStreamPos(0), mStreamEnd(0), _mAudioIndex(0), _mDelayedTime(0.0f),
      _mFadeTime(0.0f), _mWhere(0), _mWhence(JSUStreamSeekFrom::BEGIN),
      _mVolLeft(AudioVolumeDefault), _mVolRight(AudioVolumeDefault),
      _mFullVolLeft(AudioVolumeDefault), _mFullVolRight(AudioVolumeDefault),
      _mTargetVolume(AudioVolumeDefault), _mPreservedVolLeft(AudioVolumeDefault),
      _mPreservedVolRight(AudioVolumeDefault) {
    for (size_t i = 0; i < 4; ++i) {
        _mAudioQueue[i] = AudioPacket();
    }

    initThread(threadMain_, priority);
}

AudioStreamer::~AudioStreamer() {
    JKRHeap::sSystemHeap->free(mAudioStack);
    OSCancelAlarm(&mVolumeFadeAlarm);
    OSCancelThread(&mMainThread);

    DVDCancelStream(&mStopBlock);
    DVDClose(mAudioHandle);

    deinitalizeSubsystem();
}

void AudioStreamer::initThread(void *(*threadMain)(void *), OSPriority threadPrio) {
    JKRHeap *heap = JKRHeap::sSystemHeap;
    mAudioStack   = static_cast<u8 *>(heap->alloc(AudioStackSize, 32));
    OSInitMessageQueue(&mMessageQueue, mMessageList, AudioMessageQueueSize);
    OSCreateAlarm(&mVolumeFadeAlarm);
    OSSetPeriodicAlarm(&mVolumeFadeAlarm, OSGetTime(), OSMillisecondsToTicks(AudioFadeInterval),
                       cbForVolumeAlarm);
    OSCreateThread(&mMainThread, threadMain, this, mAudioStack + AudioStackSize, AudioStackSize,
                   threadPrio, OS_THREAD_ATTR_DETACH);
    OSResumeThread(&mMainThread);
}

void AudioStreamer::initializeSubsystem() {
    AIInit(0);

    AISetStreamSampleRate(AI_SAMPLE_48K);
    AIResetStreamSampleCount();
    AISetStreamTrigger(Music::AudioStreamRate);

    AISetStreamVolLeft(0);
    AISetStreamVolRight(0);
    AIRegisterStreamCallback(AudioStreamer::cbForAIInterrupt);
    AISetStreamPlayState(false);
}

void AudioStreamer::deinitalizeSubsystem() {
    AISetStreamVolLeft(0);
    AISetStreamVolRight(0);
    AIRegisterStreamCallback(nullptr);
    AIResetStreamSampleCount();
    AISetStreamPlayState(false);
}

void AudioStreamer::mainLoop_(void *param) {
    AudioStreamer::AudioCommand command;
    OSMessage msg;

    initializeSubsystem();

    while (true) {
        OSReceiveMessage(&mMessageQueue, &msg, OS_MESSAGE_BLOCK);
        command = static_cast<AudioStreamer::AudioCommand>(msg);
        switch (command) {
        case AudioStreamer::AudioCommand::PLAY:
            play_();
            break;
        case AudioStreamer::AudioCommand::PAUSE:
            pause_();
            break;
        case AudioStreamer::AudioCommand::STOP:
            stop_();
            break;
        case AudioStreamer::AudioCommand::SKIP:
            skip_();
            break;
        case AudioStreamer::AudioCommand::NEXT:
            next_();
            break;
        case AudioStreamer::AudioCommand::SEEK:
            seek_();
            break;
        default:
            break;
        }
    }
}

bool AudioStreamer::isLoopCustom() const {
    const AudioPacket &packet = getCurrentAudio();
    return packet.getLoopStart() >= 0 || packet.getLoopEnd() >= 0;
}

u32 AudioStreamer::getLoopStart() const {
    const AudioPacket &packet = getCurrentAudio();
    return packet.getLoopStart() >= 0 ? Min(packet.getLoopStart(), mAudioHandle->mLen) : 0;
}

u32 AudioStreamer::getLoopEnd() const {
    const AudioPacket &packet = getCurrentAudio();
    return packet.getLoopEnd() >= 0 ? Min(packet.getLoopEnd(), mAudioHandle->mLen)
                                    : mAudioHandle->mLen;
}

void AudioStreamer::setLooping(bool loop) { _mIsLooping = loop; }

void AudioStreamer::setVolumeLR(u8 left, u8 right) {
    if (_mVolLeft != left && _mVolLeft <= _mFullVolLeft) {
        _mVolLeft = left;
        AISetStreamVolLeft(left);
    }
    if (_mVolRight != right && _mVolRight <= _mFullVolRight) {
        _mVolRight = right;
        AISetStreamVolRight(right);
    }
}

void AudioStreamer::setFullVolumeLR(u8 left, u8 right) {
    _mFullVolLeft  = left;
    _mFullVolRight = right;
}

void AudioStreamer::resetVolumeToFull() {
    _mTargetVolume = (_mFullVolLeft + _mFullVolRight) / 2;
    setVolumeLR(_mFullVolLeft, _mFullVolRight);
}

void AudioStreamer::setVolumeFadeTo(u8 to, f32 seconds) {
    _mTargetVolume = to;
    _mFadeTime     = seconds;
}

bool AudioStreamer::queueAudio(const AudioPacket &packet) {
    for (u32 i = 0; i < AudioQueueSize; ++i) {
        AudioPacket &slot = _mAudioQueue[(i + _mAudioIndex) % AudioQueueSize];
        if (slot.getID() == 0xFFFFFFFF) {
            slot = packet;
            return true;
        }
    }

    OSReport("[AUDIO_STREAM] Queue is full!\n");
    return false;
}

static OSTime sStartTime     = 0;
static bool _sHasFadeStarted = false;

void AudioStreamer::fadeAudio_() {
    const u8 curVolume = ((_mVolLeft + _mVolRight) / 2);

    if (curVolume == _mTargetVolume) {
        sStartTime       = 0;
        _sHasFadeStarted = false;
        return;
    }

    if (_mFadeTime <= 0.0f) {
        setVolumeLR(_mTargetVolume, _mTargetVolume);
        sStartTime       = 0;
        _sHasFadeStarted = false;
        return;
    }

    if (!_sHasFadeStarted) {
        _mSrcVolume      = curVolume;
        sStartTime       = OSGetTime();
        _sHasFadeStarted = true;
    }

    const OSTime now  = OSGetTime();
    const f64 curTime = OSTicksToSeconds(f64(now - sStartTime));
    const f32 factor  = curTime / _mFadeTime;

    if (factor >= 1.0f) {
        setVolumeLR(_mTargetVolume, _mTargetVolume);
        sStartTime       = 0;
        _sHasFadeStarted = false;
        return;
    }

    u8 volL = lerp<u8>(_mSrcVolume, _mTargetVolume, factor);
    u8 volR = lerp<u8>(_mSrcVolume, _mTargetVolume, factor);

    setVolumeLR(volL, volR);
}

void AudioStreamer::play() {
    sAudioCommand = AudioCommand::PLAY;
    OSSendMessage(&mMessageQueue, static_cast<u32>(sAudioCommand), OS_MESSAGE_NOBLOCK);
}

void AudioStreamer::pause(f32 fadeTime) {
    sAudioCommand = AudioCommand::PAUSE;
    _mDelayedTime = fadeTime;
    OSSendMessage(&mMessageQueue, static_cast<u32>(sAudioCommand), OS_MESSAGE_NOBLOCK);
}

void AudioStreamer::stop(f32 fadeTime) {
    sAudioCommand = AudioCommand::STOP;
    _mDelayedTime = fadeTime;
    OSSendMessage(&mMessageQueue, static_cast<u32>(sAudioCommand), OS_MESSAGE_NOBLOCK);
}

void AudioStreamer::skip(f32 fadeTime) {
    sAudioCommand = AudioCommand::SKIP;
    _mDelayedTime = fadeTime;
    OSSendMessage(&mMessageQueue, static_cast<u32>(sAudioCommand), OS_MESSAGE_NOBLOCK);
}

void AudioStreamer::next(f32 fadeTime) {
    sAudioCommand = AudioCommand::NEXT;
    _mDelayedTime = fadeTime;
    OSSendMessage(&mMessageQueue, static_cast<u32>(sAudioCommand), OS_MESSAGE_NOBLOCK);
}

void AudioStreamer::seek(s32 where, JSUStreamSeekFrom whence) {
    sAudioCommand = AudioCommand::SEEK;
    _mWhere       = where;
    _mWhence      = whence;
    OSSendMessage(&mMessageQueue, static_cast<u32>(sAudioCommand), OS_MESSAGE_NOBLOCK);
}

void AudioStreamer::seek(f32 seconds, JSUStreamSeekFrom whence) {
    sAudioCommand = AudioCommand::SEEK;
    _mWhere       = AudioStreamRate * (u32)seconds;
    _mWhence      = whence;
    OSSendMessage(&mMessageQueue, static_cast<u32>(sAudioCommand), OS_MESSAGE_NOBLOCK);
}

SMS_NO_INLINE bool AudioStreamer::play_() {
    if (isPlaying()) {
        if (isPaused()) {
            AISetStreamPlayState(true);
            setVolumeFadeTo((_mFullVolLeft + _mFullVolRight) / 2, _mFadeTime);

            _mIsPaused = false;
            return true;
        } else {
            return false;
        }
    }

    bool isStreamAllowed = BetterSMS::isMusicStreamingAllowed();
    if (!isStreamAllowed) {
        OSPanic(__FILE__, __LINE__,
                "A music stream attempted to play, but music streaming is disabled! Set byte 8 of "
                "boot.bin to 0x01 to enable music streaming.");
        return false;
    }

    _mIsPlaying = startLowStream();
    if (_mIsPlaying) {
        OSReport("[AUDIO_STREAM] Playing new audio!\n");
        mStreamEnd = getLoopEnd() - 0x8000;
        resetVolumeToFull();
        return true;
    }

    OSReport("[AUDIO_STREAM] No audio queued to play!\n");
    return false;
}

SMS_NO_INLINE bool AudioStreamer::pause_() {
    if (_mIsPaused)
        return false;

    _mFadeTime          = _mDelayedTime;
    _mPreservedVolLeft  = _mVolLeft;
    _mPreservedVolRight = _mVolRight;
    setVolumeFadeTo(0, _mFadeTime);

    _mIsPaused = true;
    return true;
}

SMS_NO_INLINE bool AudioStreamer::stop_() {
    if (!_mIsPlaying && !_mIsPaused)
        return false;

    _mFadeTime          = _mDelayedTime;
    _mPreservedVolLeft  = _mVolLeft;
    _mPreservedVolRight = _mVolRight;
    setVolumeFadeTo(0, _mFadeTime);

    OSReport("[AUDIO_STREAM] Stopping audio!\n");

    _mIsPaused  = false;
    _mIsPlaying = false;
    return true;
}

bool AudioStreamer::skip_() {
    next_();
    return play_();
}

SMS_NO_INLINE void AudioStreamer::next_() {
    stop_();
    nextTrack_();
}

SMS_NO_INLINE bool AudioStreamer::seek_() {
    s32 streamPos = 0;
    switch (_mWhence) {
    case JSUStreamSeekFrom::BEGIN:
        streamPos = _mWhere;
        break;
    case JSUStreamSeekFrom::CURRENT:
        streamPos = mStreamPos + _mWhere;
        break;
    case JSUStreamSeekFrom::END:
        streamPos = mAudioHandle->mLen - _mWhere;
        break;
    }

    if (streamPos > mAudioHandle->mLen) {
        return skip_();
    } else {
        return seekLowStream(streamPos);
    }
}

SMS_NO_INLINE void AudioStreamer::update_() {
    // Check if pause menu is active to mute music
    if (gpMarDirector) {
        bool isGamePaused = gpMarDirector->mCurState == TMarDirector::STATE_PAUSE_MENU;
        if (!_startPaused && isGamePaused) {
            if (isPlaying()) {
                _mDelayedTime = 0.7f;
                pause_();
            }
            _startPaused = true;
        } else if (_startPaused && !isGamePaused) {
            if (isPaused()) {
                play_();
            }
            _startPaused = false;
        }
    }

    fadeAudio_();

    // Check if volume has finished fading for pause/stop
    const u8 curVolume = ((_mVolLeft + _mVolRight) / 2);
    if (curVolume == _mTargetVolume) {
        const u8 avgVolume = (_mVolLeft + _mVolRight) / 2;
        if (avgVolume == 0) {
            if (isPaused()) {
                pauseLowStream();
            } else if (!isPlaying()) {
                stopLowStream();
            }
            return;
        }
    }
}

void AudioStreamer::nextTrack_() {
    {
        TAtomicGuard guard;

        _mAudioQueue[_mAudioIndex] = AudioPacket();
        _mAudioIndex               = (_mAudioIndex + 1) % AudioQueueSize;

        mStreamPos = 0;
    }
}

SMS_NO_INLINE bool AudioStreamer::startLowStream() {
    const AudioPacket &packet = getCurrentAudio();

    char adpPath[0x40];
    char cfgPath[0x40];

    if (packet.getID() == 0xFFFFFFFF)
        return false;

    if (packet.isString()) {
        snprintf(adpPath, 64, "/AudioRes/Streams/Music/%s.adp", packet.getString());
        snprintf(cfgPath, 64, "/AudioRes/Streams/Music/%s.txt", packet.getString());
    } else {
        snprintf(adpPath, 64, "/AudioRes/Streams/Music/%lu.adp", packet.getID());
        snprintf(cfgPath, 64, "/AudioRes/Streams/Music/%lu.txt", packet.getID());
    }

    if (!DVDOpen(adpPath, mAudioHandle))
        return false;

    AISetStreamVolLeft(_mVolLeft);
    AISetStreamVolRight(_mVolRight);
    AIResetStreamSampleCount();
    AISetStreamTrigger(Music::AudioStreamRate);
    AISetStreamPlayState(true);

    DVDPrepareStreamAsync(mAudioHandle, getLoopEnd(), 0, AudioStreamer::cbForPrepareStreamAsync_);

    return true;
}

SMS_NO_INLINE void AudioStreamer::pauseLowStream() {
    AISetStreamPlayState(false);
    // DVDCancelStreamAsync(&mStopBlock, cbForCancelStreamOnPauseAsync_);
}

SMS_NO_INLINE bool AudioStreamer::seekLowStream(s32 streamPos) {
    streamPos &= ~0x7FFF;
    if (streamPos < 0)
        return false;

    mStreamPos = streamPos;

    OSReport("[AUDIO_STREAM] Seeking to %d\n", streamPos);

    return DVDCancelStreamAsync(&mSeekBlock, AudioStreamer::cbForCancelStreamOnSeekAsync_);
}

SMS_NO_INLINE bool AudioStreamer::stopLowStream() {
    AISetStreamVolLeft(0);
    AISetStreamVolRight(0);
    AISetStreamPlayState(false);
    return DVDCancelStreamAsync(&mStopBlock, AudioStreamer::cbForCancelStreamOnStopAsync_);
}

void AudioPacket::setLoopPoint(s32 start, s32 end) {
    mParams.mLoopStart.set(start);
    mParams.mLoopEnd.set(end);
}

void AudioPacket::setLoopPoint(s32 start, size_t length) {
    mParams.mLoopStart.set(start);
    mParams.mLoopEnd.set(start + length);
}

void AudioPacket::setLoopPoint(f32 start, f32 length) {
    mParams.mLoopStart.set(start);
    mParams.mLoopEnd.set(start + length);
}

SMS_NO_INLINE void AudioStreamer::cbForVolumeAlarm(OSAlarm *alarm, OSContext *context) {
    AudioStreamer *streamer = AudioStreamer::getInstance();
    streamer->update_();
}

SMS_NO_INLINE void AudioStreamer::cbForAIInterrupt(u32 trigger) {
    AudioStreamer *streamer = AudioStreamer::getInstance();
    AISetStreamTrigger(trigger + Music::AudioStreamRate);
    DVDGetStreamPlayAddrAsync(&streamer->mAIInteruptBlock,
                              AudioStreamer::cbForGetStreamPlayAddrAsync_);
    // streamer->mErrorStatus = 2;
}

SMS_NO_INLINE void AudioStreamer::cbForGetStreamErrorStatusAsync_(u32 result,
                                                                  DVDCommandBlock *cmdBlock) {
    AudioStreamer *streamer = AudioStreamer::getInstance();
    streamer->mErrorStatus  = result;
    if (streamer->mErrorStatus != 1) {
        streamer->stop_();
    }
}

SMS_NO_INLINE void AudioStreamer::cbForGetStreamPlayAddrAsync_(u32 result,
                                                               DVDCommandBlock *cmdBlock) {
    AudioStreamer *streamer = AudioStreamer::getInstance();
    streamer->mStreamPos    = result - streamer->getStreamStart();

    DVDGetStreamErrorStatusAsync(&streamer->mPlayAddrBlock,
                                 AudioStreamer::cbForGetStreamErrorStatusAsync_);

    OSReport("[AUDIO_STREAM] Stream position: %d\n", streamer->getStreamPos());
    OSReport("[AUDIO_STREAM] Stream end: %d\n", streamer->getStreamEnd());

    // Check if we've reached the end of the stream
    if (streamer->getStreamPos() < streamer->getStreamEnd()) {
        return;
    }

    // Here we either loop or play the next track
    if (streamer->isLooping()) {
        // Seek to loop start
        OSReport("[AUDIO_STREAM] Loop start: %d, Loop end: %d\n", streamer->getLoopStart(),
                 streamer->getLoopEnd());
        streamer->_mWhere  = streamer->getLoopStart();
        streamer->_mWhence = BEGIN;
        streamer->seek_();
    } else {
        streamer->_mDelayedTime = 0.0f;
        streamer->stopLowStream();
        streamer->nextTrack_();
        if (!streamer->startLowStream()) {
            OSReport("[AUDIO_STREAM] Failed to start next track!\n");
            _mIsPlaying = false;
        }
    }
}

SMS_NO_INLINE void AudioStreamer::cbForPrepareStreamAsync_(u32 result, DVDFileInfo *finfo) {
    AudioStreamer *streamer = AudioStreamer::getInstance();
    DVDStopStreamAtEndAsync(&streamer->mPrepareBlock, nullptr);
}

SMS_NO_INLINE void AudioStreamer::cbForCancelStreamOnStopAsync_(u32 result,
                                                                DVDCommandBlock *callback) {
    AudioStreamer *streamer = AudioStreamer::getInstance();
    DVDClose(streamer->mAudioHandle);
}

SMS_NO_INLINE void AudioStreamer::cbForCancelStreamOnSeekAsync_(u32 result,
                                                                DVDCommandBlock *callback) {
    AudioStreamer *streamer = AudioStreamer::getInstance();
    DVDPrepareStreamAsync(streamer->mAudioHandle, streamer->getLoopEnd() - streamer->mStreamPos,
                          streamer->mStreamPos, AudioStreamer::cbForPrepareStreamAsync_);
}

SMS_NO_INLINE void AudioStreamer::cbForStopStreamAtEndAsync_(u32 result,
                                                             DVDCommandBlock *cmdblock) {}

#pragma endregion

#pragma region BGMValidation

bool Music::isValidBGM(MSStageInfo id) {
    switch (id) {
    case BGM_AIRPORT:
    case BGM_BIANCO:
    case BGM_CASINO:
    case BGM_CORONA:
    case BGM_DELFINO:
    case BGM_DOLPIC:
    case BGM_EVENT:
    case BGM_EXTRA:
    case BGM_MAMMA:
    case BGM_MARE_SEA:
    case BGM_MAREVILLAGE:
    case BGM_MERRY_GO_ROUND:
    case BGM_MONTE_NIGHT:
    case BGM_MONTE_ONSEN:
    case BGM_MONTE_RESCUE:
    case BGM_MONTEVILLAGE:
    case BGM_PINNAPACO:
    case BGM_PINNAPACO_SEA:
    case BGM_RICCO:
    case BGM_SHILENA:
    case BGM_SKY_AND_SEA:
        return true;
    default:
        return false;
    }
}

bool Music::isValidBGM(u32 id) {
    switch (id & 0x3FF) {
    case BGM_AIRPORT & 0xFF:
    case BGM_BIANCO & 0xFF:
    case BGM_CASINO & 0xFF:
    case BGM_CORONA & 0xFF:
    case BGM_DELFINO & 0xFF:
    case BGM_DOLPIC & 0xFF:
    case BGM_EVENT & 0xFF:
    case BGM_EXTRA & 0xFF:
    case BGM_MAMMA & 0xFF:
    case BGM_MARE_SEA & 0xFF:
    case BGM_MAREVILLAGE & 0xFF:
    case BGM_MERRY_GO_ROUND & 0xFF:
    case BGM_MONTE_NIGHT & 0xFF:
    case BGM_MONTE_ONSEN & 0xFF:
    case BGM_MONTE_RESCUE & 0xFF:
    case BGM_MONTEVILLAGE & 0xFF:
    case BGM_PINNAPACO & 0xFF:
    case BGM_PINNAPACO_SEA & 0xFF:
    case BGM_RICCO & 0xFF:
    case BGM_SHILENA & 0xFF:
    case BGM_SKY_AND_SEA & 0xFF:
#if SMS_DEMO
    case BGM_CHUBOSS & 0xFF:
    case BGM_CHUBOSS2 & 0xFF:
#endif
        return true;
    default:
        return false;
    }
}

bool Music::isWeakBGM(MSStageInfo id) {
    switch (id) {
    case BGM_UNDERGROUND:
    case BGM_SHINE_APPEAR:
        return true;
    default:
        return false;
    }
}

bool Music::isWeakBGM(u32 id) {
    switch (id & 0x3FF) {
    case BGM_UNDERGROUND & 0xFF:
    case BGM_SHINE_APPEAR & 0xFF:
        return true;
    default:
        return false;
    }
}

#pragma endregion

#pragma region BGMStreamingHooks

// for future intro sound patch
static u8 gOldAreaID    = 0;
static u8 gOldEpisodeID = 0;

// 0x802B7A4C
static void initSoundBank(u8 areaID, u8 episodeID) {
    Stage::TStageParams *config = Stage::TStageParams::sStageConfig;

    gOldAreaID    = areaID;
    gOldEpisodeID = episodeID;
    if (config->mMusicSetCustom.get()) {
        areaID    = config->mMusicAreaID.get();
        episodeID = config->mMusicEpisodeID.get();
    }
    setMSoundEnterStage__10MSMainProcFUcUc(areaID, episodeID);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x802B7A4C, 0x802AFA1C, 0, 0), initSoundBank);

constexpr f32 PauseFadeSpeed = 0.2f;

// 0x802BB89C
static void initExMusic(MSStageInfo bgm) {
    Stage::TStageParams *config = Stage::getStageConfiguration();

    if (config->mMusicSetCustom.get()) {
        gStageBGM = 0x80010000 | config->mMusicID.get();
    }

    gAudioSpeed  = config->mMusicSpeed.get();
    gAudioPitch  = config->mMusicPitch.get();
    gAudioVolume = Clamp(config->mMusicVolume.get(), 0, 1);

    if (!config->mMusicEnabled.get())
        return;

    if (!config->mIsExStage.get()) {
        MSBgm::startBGM(bgm);
        return;
    }

    AudioStreamer *streamer = AudioStreamer::getInstance();
    AudioPacket packet(gStageBGM & 0x3FF);
    packet.setLoopPoint(config->mStreamLoopStart.get(), config->mStreamLoopEnd.get());

    streamer->queueAudio(packet);
    streamer->setLooping(true);

    if (streamer->isPlaying()) {
        streamer->skip(0.0f);
    }

    streamer->setVolumeLR(Music::AudioVolumeDefault, Music::AudioVolumeDefault);
    streamer->play();
    return;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x802BB89C, 0x802B386C, 0, 0), initExMusic);

#include <JSystem/JAudio/JAIHardStream.hxx>
using namespace JASystem;

static char streamFiles[][JASystem::HardStream::streamFilesSize] = {"test.adp", "test2.adp"};

// 0x802983F0
// 0x80298420
// 0x802984D0
static void initStageMusic() {
    Stage::TStageParams *config = Stage::getStageConfiguration();

    if (config->mMusicSetCustom.get()) {
        gStageBGM = 0x80010000 | config->mMusicID.get();
    }

    gAudioSpeed  = config->mMusicSpeed.get();
    gAudioPitch  = config->mMusicPitch.get();
    gAudioVolume = Max(Min(config->mMusicVolume.get(), 1), 0);

    if (!config->mMusicEnabled.get())
        return;

    if ((gStageBGM & ~0x800103FF) == 0) {
        startStageBGM__10MSMainProcFUcUc(gpMarDirector->mAreaID, gpMarDirector->mEpisodeID);
        return;
    }

    if (config->mIsExStage.get())
        return;

#if 1
    AudioStreamer *streamer = AudioStreamer::getInstance();
    AudioPacket packet(gStageBGM & 0x3FF);
    packet.setLoopPoint(config->mStreamLoopStart.get(), config->mStreamLoopEnd.get());

    streamer->queueAudio(packet);
    streamer->setLooping(true);

    if (streamer->isPlaying()) {
        streamer->skip(0.0f);
    }

    streamer->setVolumeLR(Music::AudioVolumeDefault, Music::AudioVolumeDefault);
    streamer->play();

#else
    HardStream::useHardStreaming = true;
    HardStream::streamFiles      = reinterpret_cast<char *>(streamFiles);
    HardStream::strCtrl.mState   = 1;

    HardStream::TFrame *frame = new HardStream::TFrame();
    frame->mInfo              = new HardStream::TPlayInfo();

    frame->mInfo->mIntroCount = 0;
    frame->mInfo->mLoopCount  = 1;

    HardStream::strCtrl.mFrame = frame;

    strncpy(HardStream::rootDir, "/AudioRes/Streams/Music/", 32);
#endif

    return;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x802983F0, 0x80290288, 0, 0), initStageMusic);
SMS_PATCH_BL(SMS_PORT_REGION(0x80298420, 0x802902B8, 0, 0), initStageMusic);
SMS_PATCH_BL(SMS_PORT_REGION(0x802984D0, 0x80290368, 0, 0), initStageMusic);

// 0x80297B7C
static void stopMusicOnShineGet(u32 musicID) {
    AudioStreamer *streamer = AudioStreamer::getInstance();

    if ((gpMarDirector->mCollectedShine->mType & 0x10) == 0 && streamer->isPlaying()) {
        streamer->stop(PauseFadeSpeed);
    }

    MSBgm::startBGM(musicID);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80297B7C, 0x8028FA14, 0, 0), stopMusicOnShineGet);

// 0x8024FAB8
static void stopMusicOnManholeEnter(u32 musicID) {
    AudioStreamer *streamer = AudioStreamer::getInstance();

    if (streamer->isPlaying())
        streamer->pause(PauseFadeSpeed);

    MSBgm::startBGM(musicID);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x8024FAB8, 0x80247844, 0, 0), stopMusicOnManholeEnter);

// 0x8024FB0C
static void startMusicOnManholeExit(u32 musicID, u32 unk_0) {
    AudioStreamer *streamer = AudioStreamer::getInstance();

    if (streamer->isPaused())
        streamer->play();

    MSBgm::stopBGM(musicID, unk_0);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x8024FB0C, 0x80247898, 0, 0), startMusicOnManholeExit);

// 0x802981A8
static void stopMusicBeforeShineCamera(CPolarSubCamera *cam, const char *demo, const TVec3f *pos,
                                       s32 unk_0, f32 unk_1, bool unk_2) {
    cam->startDemoCamera(demo, pos, unk_0, unk_1, unk_2);

    AudioStreamer *streamer = AudioStreamer::getInstance();
    if (streamer->isPlaying())
        streamer->pause(PauseFadeSpeed);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x802981A8, 0x80290040, 0, 0), stopMusicBeforeShineCamera);

// 0x80297FD4
static void startMusicAfterShineCamera(CPolarSubCamera *cam) {
    cam->endDemoCamera();

    AudioStreamer *streamer = AudioStreamer::getInstance();
    if (streamer->isPaused())
        streamer->play();
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80297FD4, 0x8028FE6C, 0, 0), startMusicAfterShineCamera);

static void stopMusicOnDeathExec(u32 musicID) {
    AudioStreamer *streamer = AudioStreamer::getInstance();

    if (streamer->isPlaying())
        streamer->stop(PauseFadeSpeed);

    MSBgm::startBGM(musicID);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80298868, 0x80290700, 0, 0), stopMusicOnDeathExec);

static void stopMusicOnGameOver(u32 musicID) {
    AudioStreamer *streamer = AudioStreamer::getInstance();

    if (streamer->isPlaying())
        streamer->stop(PauseFadeSpeed);

    MSBgm::startBGM(musicID);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x802988B0, 0x80290748, 0, 0), stopMusicOnGameOver);

static void pauseMusicOnScriptMusicStart(u32 musicID) {
    AudioStreamer *streamer = AudioStreamer::getInstance();

    if (streamer->isPlaying())
        streamer->pause(PauseFadeSpeed);

    MSBgm::startBGM(musicID);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x8028b7f0, 0, 0, 0), pauseMusicOnScriptMusicStart);

static void startMusicOnScriptMusicStop(u32 musicID, u32 _unk) {
    AudioStreamer *streamer = AudioStreamer::getInstance();

    if (streamer->isPaused())
        streamer->play();

    MSBgm::stopBGM(musicID, _unk);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x8028B318, 0, 0, 0), startMusicOnScriptMusicStop);

BETTER_SMS_FOR_CALLBACK void stopMusicOnExitStage(TApplication *app) {
    if (app->mContext == TApplication::CONTEXT_DIRECT_STAGE) {
        AudioStreamer *streamer = AudioStreamer::getInstance();
        streamer->next(0.2f);
    }
}

#pragma endregion
