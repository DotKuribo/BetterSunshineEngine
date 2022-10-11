
#include <Dolphin/DVD.h>
#include <Dolphin/OS.h>
#include <Dolphin/printf.h>
#include <Dolphin/string.h>
#include <JSystem/JSupport/JSUStream.hxx>
#include <SMS/Player/Mario.hxx>

#include <SMS/game/Application.hxx>
#include <SMS/camera/PolarSubCamera.hxx>
#include <SMS/macros.h>
#include <SMS/raw_fn.hxx>
#include <SMS/sound/MSBGM.hxx>

#include "libs/constmath.hxx"
#include "logging.hxx"
#include "math.hxx"
#include "module.hxx"
#include "music.hxx"
#include "stage.hxx"

using namespace BetterSMS;

// Name of the song to play, e.g. "BeachTheme"
SMS_NO_INLINE bool BetterSMS::Music::queueSong(const char *name) {
    AudioStreamer *streamer           = getAudioStreamer();
    AudioStreamer::AudioPacket packet = AudioStreamer::AudioPacket(name);

    return streamer->queueAudio(packet);
}

// Play a paused/queued song
SMS_NO_INLINE void BetterSMS::Music::playSong() {
    AudioStreamer *streamer = getAudioStreamer();
    streamer->play();
}

// Pause song, fading out by `fadeTime` seconds
SMS_NO_INLINE void BetterSMS::Music::pauseSong(f32 fadeTime) {
    AudioStreamer *streamer = getAudioStreamer();
    return streamer->pause(fadeTime);
}

SMS_NO_INLINE void BetterSMS::Music::stopSong(f32 fadeTime) {
    AudioStreamer *streamer = getAudioStreamer();
    return streamer->stop(fadeTime);
}

SMS_NO_INLINE void BetterSMS::Music::skipSong(f32 fadeTime) {
    AudioStreamer *streamer = getAudioStreamer();
    return streamer->skip(fadeTime);
}

SMS_NO_INLINE void BetterSMS::Music::setVolume(u8 left, u8 right) {
    AudioStreamer *streamer = getAudioStreamer();
    streamer->setVolumeLR(left, right);
}

SMS_NO_INLINE void BetterSMS::Music::setVolumeFade(u8 dstVolume, f32 seconds) {
    AudioStreamer *streamer = getAudioStreamer();
    streamer->setVolumeFadeTo(dstVolume, seconds);
}

SMS_NO_INLINE void BetterSMS::Music::setLoopPoint(f32 start, f32 length) {
    AudioStreamer *streamer            = getAudioStreamer();
    AudioStreamer::AudioPacket &packet = streamer->getCurrentAudio();

    packet.setLoopPoint(start, length);
    streamer->mEndPlayAddress = streamer->getStreamEnd();
}

SMS_NO_INLINE Music::AudioStreamer *BetterSMS::Music::getAudioStreamer() {
    return AudioStreamer::getInstance();
}

#pragma region Implementation

static bool _startPaused = false;
static bool _mIsPlaying  = false;
static bool _mIsPaused   = false;
static bool _mIsLooping  = false;

#if BETTER_SMS_CUSTOM_MUSIC
static void updaterLoop() {
    main__Q28JASystem10HardStreamFv();

    Music::AudioStreamer *streamer = Music::AudioStreamer::getInstance();
    streamer->update_();
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80316034, 0x8030E1C4, 0, 0), updaterLoop);

static void *mainLoop(void *param) {
    Music::AudioStreamer *streamer = reinterpret_cast<Music::AudioStreamer *>(param);
    Music::AudioStreamer::AudioCommand command;
    OSMessage msg;

    while (true) {
        OSReceiveMessage(&streamer->mMessageQueue, &msg, OS_MESSAGE_BLOCK);
        command = static_cast<Music::AudioStreamer::AudioCommand>(msg);
        switch (command) {
        case Music::AudioStreamer::AudioCommand::PLAY:
            streamer->play_();
            break;
        case Music::AudioStreamer::AudioCommand::PAUSE:
            streamer->pause_();
            break;
        case Music::AudioStreamer::AudioCommand::STOP:
            streamer->stop_();
            break;
        case Music::AudioStreamer::AudioCommand::SKIP:
            streamer->skip_();
            break;
        case Music::AudioStreamer::AudioCommand::NEXT:
            streamer->next_();
            break;
        case Music::AudioStreamer::AudioCommand::SEEK:
            streamer->seek_();
            break;
        default:
            break;
        }
    }
}
#else
static void *mainLoop(void *param) {
    Music::AudioStreamer *streamer = reinterpret_cast<Music::AudioStreamer *>(param);
    Music::AudioStreamer::AudioCommand command;
    OSMessage msg;

    while (true) {
        OSReceiveMessage(&streamer->mMessageQueue, &msg, OS_MESSAGE_BLOCK);
    }
}
#endif

#pragma region CallbackImplementation

static void volumeAlarm(OSAlarm *alarm, OSContext *context) {
    Music::AudioStreamer *streamer = Music::AudioStreamer::getInstance();
    streamer->fadeAudio_();
}

static void cbForGetStreamErrorStatusAsync_(u32 result, DVDCommandBlock *cmdBlock) {
    Music::AudioStreamer *streamer = Music::AudioStreamer::getInstance();
    streamer->mErrorStatus         = result;
    if (streamer->mErrorStatus != 1) {
        streamer->stop_();
    }
}

static void cbForGetStreamPlayAddrAsync_(u32 result, DVDCommandBlock *cmdBlock) {
    Music::AudioStreamer *streamer = Music::AudioStreamer::getInstance();
    streamer->mCurrentPlayAddress  = result;
    DVDGetStreamErrorStatusAsync(&streamer->mStatusCmd, cbForGetStreamErrorStatusAsync_);
}

static void cbForAIInterrupt(u32 trigger) {
    Music::AudioStreamer *streamer = Music::AudioStreamer::getInstance();
    AISetStreamTrigger(trigger + Music::AudioStreamRate);
    DVDGetStreamPlayAddrAsync(&streamer->mGetAddrCmd, cbForGetStreamPlayAddrAsync_);
    streamer->mErrorStatus = 2;
}

static void cbForPrepareStreamAsync_(u32 result, DVDFileInfo *finfo) {
    Music::AudioStreamer *streamer = Music::AudioStreamer::getInstance();
    u16 volLR                      = streamer->getVolumeLR();

    // Set up AI
    // AISetDSPSampleRate(0);
    // AISetStreamSampleRate(AI_SAMPLE_48K);
    AIRegisterStreamCallback(cbForAIInterrupt);
    AISetStreamVolLeft(static_cast<u8>(volLR >> 8));
    AISetStreamVolRight(static_cast<u8>(volLR));
    AISetStreamSampleRate(AI_SAMPLE_48K);
    AIResetStreamSampleCount();
    AISetStreamTrigger(Music::AudioStreamRate);
    AISetStreamPlayState(true);

    streamer->mErrorStatus = 1;
    _mIsPlaying            = true;
}

static void cbForCancelStreamAsync_(u32 result, DVDCommandBlock *callback) {
    Music::AudioStreamer *streamer = Music::AudioStreamer::getInstance();
    DVDClose(streamer->mAudioHandle);
}

static void cbForStopStreamAtEndAsync_(u32 result, DVDCommandBlock *cmdblock) {
#if 0
  Music::AudioStreamer *streamer = Music::AudioStreamer::getInstance();

  if (streamer->isLooping()) {
    DVDPrepareStreamAsync(fileinfo, 0, 0, cbForPrepareStreamAsync_);
  }
#endif
}

#pragma endregion

static SMS_ALIGN(32) DVDFileInfo sAudioFInfo;

static Music::AudioStreamer::AudioCommand sAudioCommand = Music::AudioStreamer::AudioCommand::NONE;

bool Music::AudioStreamer::isPlaying() const { return _mIsPlaying; }
bool Music::AudioStreamer::isPaused() const { return _mIsPaused; }
bool Music::AudioStreamer::isLooping() const { return _mIsLooping; }

Music::AudioStreamer Music::AudioStreamer::sInstance =
    Music::AudioStreamer(mainLoop, 18, &sAudioFInfo);

Music::AudioStreamer::AudioStreamer(void *(*mainLoop)(void *), OSPriority priority,
                                    DVDFileInfo *fInfo)
    : mAudioHandle(fInfo), mCurrentPlayAddress(0), mEndPlayAddress(0), _mAudioIndex(0),
      _mDelayedTime(0.0f), _mFadeTime(0.0f), _mWhere(0), _mWhence(JSUStreamSeekFrom::BEGIN),
      _mVolLeft(AudioVolumeDefault), _mVolRight(AudioVolumeDefault),
      _mFullVolLeft(AudioVolumeDefault), _mFullVolRight(AudioVolumeDefault),
      _mTargetVolume(AudioVolumeDefault), _mPreservedVolLeft(AudioVolumeDefault),
      _mPreservedVolRight(AudioVolumeDefault) {
    initThread(priority);
}

Music::AudioStreamer::~AudioStreamer() {
    JKRHeap::sRootHeap->free(mAudioStack);
    OSCancelAlarm(&mVolumeFadeAlarm);
    OSCancelThread(&mMainThread);

    AISetStreamPlayState(false);
    DVDCancelStream(&mStopCmd);
    DVDClose(mAudioHandle);
}

void Music::AudioStreamer::initThread(OSPriority threadPrio) {
    mAudioStack = static_cast<u8 *>(JKRHeap::sRootHeap->alloc(AudioStackSize, 32));
    OSInitMessageQueue(&mMessageQueue, mMessageList, AudioMessageQueueSize);
    OSCreateAlarm(&mVolumeFadeAlarm);
    OSSetPeriodicAlarm(&mVolumeFadeAlarm, OSGetTime(), OSMillisecondsToTicks(1), volumeAlarm);
    OSCreateThread(&mMainThread, mainLoop, this, mAudioStack + AudioStackSize, AudioStackSize,
                   threadPrio, OS_THREAD_ATTR_DETACH);
    OSResumeThread(&mMainThread);
}

void Music::AudioStreamer::setLooping(bool loop) { _mIsLooping = loop; }

void Music::AudioStreamer::setVolumeLR(u8 left, u8 right) {
    if (_mVolLeft != left && _mVolLeft <= _mFullVolLeft) {
        _mVolLeft = left;
        AISetStreamVolLeft(left);
    }
    if (_mVolRight != right && _mVolRight <= _mFullVolRight) {
        _mVolRight = right;
        AISetStreamVolRight(right);
    }
}

void Music::AudioStreamer::setFullVolumeLR(u8 left, u8 right) {
    _mFullVolLeft  = left;
    _mFullVolRight = right;
}

void Music::AudioStreamer::resetVolumeToFull() {
    _mTargetVolume = (_mFullVolLeft + _mFullVolRight) / 2;
    setVolumeLR(_mFullVolLeft, _mFullVolRight);
}

void Music::AudioStreamer::setVolumeFadeTo(u8 to, f32 seconds) {
    _mTargetVolume = to;
    _mFadeTime     = seconds;
    OSReport("TargetVolume = %d; fadetime = %.04f\n", to, _mFadeTime);
}

bool Music::AudioStreamer::queueAudio(const AudioPacket &packet) {
    for (u32 i = 0; i < AudioQueueSize; ++i) {
        AudioPacket &slot = _mAudioQueue[(i + _mAudioIndex) % AudioQueueSize];
        if (slot.mIdentifier.as_u32 == 0xFFFFFFFF) {
            slot = packet;
            return true;
        }
    }

    OSReport("%s: Queue is full!\n", SMS_FUNC_SIG);
    return false;
}

static OSTime sStartTime     = 0;
static bool _sHasFadeStarted = false;

void Music::AudioStreamer::fadeAudio_() {
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

    OSReport("ticks = %llu; curTime = %.04f; lerp = %.04f\n", now - sStartTime, curTime, factor);

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

void Music::AudioStreamer::play() {
    sAudioCommand = AudioCommand::PLAY;
    OSSendMessage(&mMessageQueue, static_cast<u32>(sAudioCommand), OS_MESSAGE_NOBLOCK);
}

void Music::AudioStreamer::pause(f32 fadeTime) {
    sAudioCommand = AudioCommand::PAUSE;
    _mDelayedTime = fadeTime;
    OSSendMessage(&mMessageQueue, static_cast<u32>(sAudioCommand), OS_MESSAGE_NOBLOCK);
}

void Music::AudioStreamer::stop(f32 fadeTime) {
    sAudioCommand = AudioCommand::STOP;
    _mDelayedTime = fadeTime;
    OSSendMessage(&mMessageQueue, static_cast<u32>(sAudioCommand), OS_MESSAGE_NOBLOCK);
}

void Music::AudioStreamer::skip(f32 fadeTime) {
    sAudioCommand = AudioCommand::SKIP;
    _mDelayedTime = fadeTime;
    OSSendMessage(&mMessageQueue, static_cast<u32>(sAudioCommand), OS_MESSAGE_NOBLOCK);
}

void Music::AudioStreamer::next(f32 fadeTime) {
    sAudioCommand = AudioCommand::NEXT;
    _mDelayedTime = fadeTime;
    OSSendMessage(&mMessageQueue, static_cast<u32>(sAudioCommand), OS_MESSAGE_NOBLOCK);
}

void Music::AudioStreamer::seek(s32 where, JSUStreamSeekFrom whence) {
    sAudioCommand = AudioCommand::SEEK;
    _mWhere       = where;
    _mWhence      = whence;
    OSSendMessage(&mMessageQueue, static_cast<u32>(sAudioCommand), OS_MESSAGE_NOBLOCK);
}

void Music::AudioStreamer::seek(f32 seconds, JSUStreamSeekFrom whence) {
    sAudioCommand = AudioCommand::SEEK;
    _mWhere       = AudioStreamRate * (u32)seconds;
    _mWhence      = whence;
    OSSendMessage(&mMessageQueue, static_cast<u32>(sAudioCommand), OS_MESSAGE_NOBLOCK);
}

bool Music::AudioStreamer::play_() {
    if (isPlaying()) {
        if (isPaused()) {
            AISetStreamPlayState(true);
            setVolumeFadeTo((_mFullVolLeft + _mFullVolRight) / 2, _mFadeTime);

            OSReport("%s: Resuming paused audio!\n", SMS_FUNC_SIG);
            _mIsPaused = false;
            return true;
        } else {
            OSReport("%s: Already playing audio!\n", SMS_FUNC_SIG);
            return false;
        }
    }

    _mIsPlaying = getCurrentAudio().exec(mAudioHandle);
    if (_mIsPlaying) {
        OSReport("%s: Playing new audio!\n", SMS_FUNC_SIG);
        mEndPlayAddress = (mAudioHandle->mStart + getLoopEnd()) - 0x8000;
        resetVolumeToFull();
        return true;
    }

    OSReport("%s: No audio queued to play!\n", SMS_FUNC_SIG);
    return false;
}

bool Music::AudioStreamer::pause_() {
    if (_mIsPaused)
        return false;

    _mFadeTime          = _mDelayedTime;
    _mPreservedVolLeft  = _mVolLeft;
    _mPreservedVolRight = _mVolRight;
    setVolumeFadeTo(0, _mFadeTime);

    OSReport("%s: Pausing audio!\n", SMS_FUNC_SIG);

    _mIsPaused = true;
    return true;
}

bool Music::AudioStreamer::stop_() {
    if (!_mIsPlaying && !_mIsPaused)
        return false;

    _mFadeTime          = _mDelayedTime;
    _mPreservedVolLeft  = _mVolLeft;
    _mPreservedVolRight = _mVolRight;
    setVolumeFadeTo(0, _mFadeTime);

    OSReport("%s: Stopping audio!\n", SMS_FUNC_SIG);

    _mIsPaused  = false;
    _mIsPlaying = false;
    return true;
}

bool Music::AudioStreamer::skip_() {
    next_();
    return play_();
}

void Music::AudioStreamer::next_() {
    stop_();

    // clang-format off
    SMS_ATOMIC_CODE (
        _mAudioQueue[_mAudioIndex] = AudioPacket();
        _mAudioIndex = (_mAudioIndex + 1) % AudioQueueSize;
    )
    // clang-format on
}

bool Music::AudioStreamer::seek_() {
    AudioPacket &packet = getCurrentAudio();

    const u32 streamSize = mAudioHandle->mLen;

    s32 streamPos = 0;
    switch (_mWhence) {
    case JSUStreamSeekFrom::BEGIN:
        streamPos = _mWhere;
        break;
    case JSUStreamSeekFrom::CURRENT:
        streamPos = (mCurrentPlayAddress - mAudioHandle->mStart) + _mWhere;
        break;
    case JSUStreamSeekFrom::END:
        streamPos = streamSize - _mWhere;
        break;
    }

    if (streamPos > streamSize) {
        skip_();
    } else if (streamPos < 0) {
        streamPos = 0;
    }

    DVDCancelStreamAsync(&mStopCmd, nullptr);
    _mIsPlaying = false;
    return DVDPrepareStreamAsync(mAudioHandle, getLoopEnd() - streamPos, streamPos,
                                 cbForPrepareStreamAsync_);
}

void Music::AudioStreamer::update_() {
    const u8 vol = (_mVolLeft + _mVolRight) / 2;
    if (vol == 0) {
        if (_mIsPaused)
            AISetStreamPlayState(false);
        else if (!_mIsPlaying) {
            AISetStreamPlayState(false);
            if (mStopCmd.mCurCommand != 7)
                DVDCancelStreamAsync(&mStopCmd, cbForCancelStreamAsync_);
        }
        return;
    }

    if (!isPlaying())
        return;

    if (!gpMarDirector) {
        stop(0.0f);
        return;
    }

    AudioPacket &packet = getCurrentAudio();

    if (!_startPaused && gpMarDirector->mCurState == TMarDirector::STATE_PAUSE_MENU) {
        pause(0.7f);
        _startPaused = true;
    } else if (_startPaused && gpMarDirector->mCurState != TMarDirector::STATE_PAUSE_MENU) {
        play();
        _startPaused = false;
    }

    if (isLooping() && mCurrentPlayAddress == mEndPlayAddress) {
        OSReport("%s: Preparing loop stream!\n", SMS_FUNC_SIG);
        _mIsPlaying = false;
        DVDPrepareStreamAsync(mAudioHandle, getLoopEnd() - getLoopStart(), getLoopStart(),
                              cbForPrepareStreamAsync_);
    } else if (mCurrentPlayAddress == mEndPlayAddress || mEndPlayAddress == 0xFFFF8000) {
        next(0.0f);
        play();
    }
}

bool Music::AudioStreamer::AudioPacket::exec(DVDFileInfo *handle) {
    char adpPath[0x40];
    char cfgPath[0x40];

    if (mIdentifier.as_u32 == 0xFFFFFFFF)
        return false;

    if (mIsString) {
        snprintf(adpPath, 64, "/AudioRes/Streams/Music/%s.adp", mIdentifier.as_string);
        snprintf(cfgPath, 64, "/AudioRes/Streams/Music/%s.txt", mIdentifier.as_string);
    } else {
        snprintf(adpPath, 64, "/AudioRes/Streams/Music/%lu.adp", mIdentifier.as_u32);
        snprintf(cfgPath, 64, "/AudioRes/Streams/Music/%lu.txt", mIdentifier.as_u32);
    }

    if (!DVDOpen(adpPath, handle))
        return false;

    OSReport("%s: Executing audio packet!\n", SMS_FUNC_SIG);
    DVDPrepareStreamAsync(
        handle, mParams.mLoopEnd.get() != 0xFFFFFFFF ? mParams.mLoopEnd.get() : handle->mLen, 0,
        cbForPrepareStreamAsync_);

    return true;
}

void Music::AudioStreamer::AudioPacket::setLoopPoint(s32 start, size_t length) {
    mParams.mLoopStart.set(start);
    mParams.mLoopEnd.set(start + length);
}

void Music::AudioStreamer::AudioPacket::setLoopPoint(f32 start, f32 length) {
    mParams.mLoopStart.set(start);
    mParams.mLoopEnd.set(start + length);
}

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
    /*TStageParams *config = TStageParams::sStageConfig;

    gOldAreaID = areaID;
    gOldEpisodeID = episodeID;
    if (config->mMusicSetCustom.get()) {
      areaID = config->mMusicAreaID.get();
      episodeID = config->mMusicEpisodeID.get();
    }*/
    Console::debugLog("Initializing the sound bank...\n");
    setMSoundEnterStage__10MSMainProcFUcUc(areaID, episodeID);
    Console::debugLog("Initializing the sound bank... DONE!\n");
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

    Music::AudioStreamer *streamer = Music::getAudioStreamer();
    Music::AudioStreamer::AudioPacket packet(gStageBGM & 0x3FF);

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

    if ((gStageBGM & 0xFC00) == 0) {
        startStageBGM__10MSMainProcFUcUc(gpMarDirector->mAreaID, gpMarDirector->mEpisodeID);
        return;
    }

    if (config->mIsExStage.get())
        return;

#if 1
    Music::AudioStreamer *streamer = Music::getAudioStreamer();
    Music::AudioStreamer::AudioPacket packet(gStageBGM & 0x3FF);

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


// 0x802A670C
static void stopMusicOnStageExit(TMarioGamePad *gamepad) {
    Music::AudioStreamer *streamer = Music::getAudioStreamer();
    streamer->next(PauseFadeSpeed);

    reset__9RumbleMgrFv(gamepad);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x802A670C, 0x8029E664, 0, 0), stopMusicOnStageExit);

// 0x80297B7C
static void stopMusicOnShineGet(u32 musicID) {
    Music::AudioStreamer *streamer = Music::getAudioStreamer();

    if ((gpMarDirector->mCollectedShine->mType & 0x10) == 0 && streamer->isPlaying()) {
        streamer->stop(PauseFadeSpeed);
    }

    MSBgm::startBGM(musicID);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80297B7C, 0x8028FA14, 0, 0), stopMusicOnShineGet);

// 0x8024FAB8
static void stopMusicOnManholeEnter(u32 musicID) {
    Music::AudioStreamer *streamer = Music::getAudioStreamer();

    if (streamer->isPlaying())
        streamer->pause(PauseFadeSpeed);

    MSBgm::startBGM(musicID);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x8024FAB8, 0x80247844, 0, 0), stopMusicOnManholeEnter);

// 0x8024FB0C
static void stopMusicOnManholeExit(u32 musicID, u32 unk_0) {
    Music::AudioStreamer *streamer = Music::getAudioStreamer();

    if (streamer->isPaused())
        streamer->play();

    MSBgm::stopBGM(musicID, unk_0);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x8024FB0C, 0x80247898, 0, 0), stopMusicOnManholeExit);

// 0x802981A8
static void stopMusicBeforeShineCamera(CPolarSubCamera *cam, const char *demo, const TVec3f *pos,
                                       s32 unk_0, f32 unk_1, bool unk_2) {
    cam->startDemoCamera(demo, pos, unk_0, unk_1, unk_2);

    Music::AudioStreamer *streamer = Music::getAudioStreamer();
    if (streamer->isPlaying())
        streamer->pause(PauseFadeSpeed);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x802981A8, 0x80290040, 0, 0), stopMusicBeforeShineCamera);

// 0x80297FD4
static void startMusicAfterShineCamera(CPolarSubCamera *cam) {
    cam->endDemoCamera();

    Music::AudioStreamer *streamer = Music::getAudioStreamer();
    if (streamer->isPaused())
        streamer->play();
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80297FD4, 0x8028FE6C, 0, 0), startMusicAfterShineCamera);

static void stopMusicOnDeathExec(u32 musicID) {
    Music::AudioStreamer *streamer = Music::getAudioStreamer();

    if (streamer->isPlaying())
        streamer->stop(PauseFadeSpeed);

    MSBgm::startBGM(musicID);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80298868, 0x80290700, 0, 0), stopMusicOnDeathExec);

static void stopMusicOnGameOver(u32 musicID) {
    Music::AudioStreamer *streamer = Music::getAudioStreamer();

    if (streamer->isPlaying())
        streamer->stop(PauseFadeSpeed);

    MSBgm::startBGM(musicID);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x802988B0, 0x80290748, 0, 0), stopMusicOnGameOver);

#pragma endregion
