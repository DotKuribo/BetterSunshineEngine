#include "music.hxx"

#include "p_sunscript.hxx"

using namespace BetterSMS::Music;

void Spc::getStageBGM(TSpcInterp *interp, u32 argc) {
    interp->verifyArgNum(0, &argc);
    Spc::Stack::pushItem(interp, gStageBGM, Spc::ValueType::INT);
}

void Spc::queueStream(TSpcInterp *interp, u32 argc) {
    interp->verifyArgNum(1, &argc);
    AudioStreamer *streamer = AudioStreamer::getInstance();

    TSpcSlice item = Spc::Stack::popItem(interp);
    if (item.mType == Spc::ValueType::STRING) {
        streamer->queueAudio(AudioPacket(reinterpret_cast<const char *>(item.mValue)));
    } else {
        streamer->queueAudio(AudioPacket(item.mValue));
    }

    OSReport("Item type: %d, Item value: %X\n", item.mType, item.mValue);
}

void Spc::playStream(TSpcInterp *interp, u32 argc) {
    interp->verifyArgNum(0, &argc);
    AudioStreamer *streamer = AudioStreamer::getInstance();

    streamer->play();
}

void Spc::pauseStream(TSpcInterp *interp, u32 argc) {
    interp->verifyArgNum(1, &argc);
    AudioStreamer *streamer = AudioStreamer::getInstance();

    f32 fadeOut = static_cast<f32>(Spc::Stack::popItem(interp).mValue);
    streamer->pause(fadeOut);
}

void Spc::stopStream(TSpcInterp *interp, u32 argc) {
    interp->verifyArgNum(1, &argc);
    AudioStreamer *streamer = AudioStreamer::getInstance();

    f32 fadeOut = static_cast<f32>(Spc::Stack::popItem(interp).mValue);
    streamer->stop(fadeOut);
}

void Spc::seekStream(TSpcInterp *interp, u32 argc) {
    interp->verifyArgNum(1, &argc);
    AudioStreamer *streamer = AudioStreamer::getInstance();

    JSUStreamSeekFrom whence = static_cast<JSUStreamSeekFrom>(Spc::Stack::popItem(interp).mValue);
    s32 where                = Spc::Stack::popItem(interp).mValue;

    streamer->seek(where, whence);
}

void Spc::nextStream(TSpcInterp *interp, u32 argc) {
    interp->verifyArgNum(1, &argc);
    AudioStreamer *streamer = AudioStreamer::getInstance();

    f32 fadeOut = static_cast<f32>(Spc::Stack::popItem(interp).mValue);
    streamer->next(fadeOut);
}

void Spc::skipStream(TSpcInterp *interp, u32 argc) {
    interp->verifyArgNum(1, &argc);
    AudioStreamer *streamer = AudioStreamer::getInstance();

    f32 fadeOut = static_cast<f32>(Spc::Stack::popItem(interp).mValue);
    streamer->skip(fadeOut);
}

void Spc::getStreamVolume(TSpcInterp *interp, u32 argc) {
    interp->verifyArgNum(0, &argc);
    AudioStreamer *streamer = AudioStreamer::getInstance();

    Spc::Stack::pushItem(interp, streamer->getVolumeLR(), Spc::ValueType::INT);
}

void Spc::setStreamVolume(TSpcInterp *interp, u32 argc) {
    interp->verifyArgNum(1, &argc);
    AudioStreamer *streamer = AudioStreamer::getInstance();

    u32 volLR = Spc::Stack::popItem(interp).mValue;
    streamer->setVolumeLR(static_cast<u8>(volLR >> 8), static_cast<u8>(volLR));
}

void Spc::getStreamLooping(TSpcInterp *interp, u32 argc) {
    interp->verifyArgNum(0, &argc);
    AudioStreamer *streamer = AudioStreamer::getInstance();

    Spc::Stack::pushItem(interp, streamer->isLooping(), Spc::ValueType::INT);
}
void Spc::setStreamLooping(TSpcInterp *interp, u32 argc) {
    interp->verifyArgNum(1, &argc);
    AudioStreamer *streamer = AudioStreamer::getInstance();

    bool looping = static_cast<bool>(Spc::Stack::popItem(interp).mValue);
    streamer->setLooping(looping);
}