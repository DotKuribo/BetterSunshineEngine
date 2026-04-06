#include <Dolphin/MTX.h>
#include <Dolphin/types.h>

#include <JSystem/J2D/J2DOrthoGraph.hxx>
#include <JSystem/J2D/J2DPane.hxx>
#include <JSystem/J2D/J2DTextBox.hxx>
#include <JSystem/JKernel/JKRHeap.hxx>
#include <SMS/Enemy/EnemyMario.hxx>
#include <SMS/Player/Mario.hxx>

#include <SMS/MSound/MSound.hxx>
#include <SMS/MSound/MSoundSESystem.hxx>
#include <SMS/Map/MapCollisionData.hxx>
#include <SMS/System/Application.hxx>
#include <SMS/raw_fn.hxx>

#include "debug.hxx"
#include "libs/cheathandler.hxx"
#include "libs/constmath.hxx"
#include "logging.hxx"
#include "module.hxx"
#include "music.hxx"

#include "p_debug.hxx"

using namespace BetterSMS;

static char sStringBuffer[200]{};
static J2DTextBox *gpMusicStringW = nullptr;
static J2DTextBox *gpMusicStringB = nullptr;
static bool sIsInitialized        = false;

void initStreamInfo(TApplication *app) {
    gpMusicStringW                  = new J2DTextBox(gpSystemFont->mFont, "");
    gpMusicStringB                  = new J2DTextBox(gpSystemFont->mFont, "");
    gpMusicStringW->mStrPtr         = sStringBuffer;
    gpMusicStringB->mStrPtr         = sStringBuffer;
    gpMusicStringW->mNewlineSize    = 11;
    gpMusicStringW->mCharSizeX      = 11;
    gpMusicStringW->mCharSizeY      = 11;
    gpMusicStringB->mNewlineSize    = 11;
    gpMusicStringB->mCharSizeX      = 11;
    gpMusicStringB->mCharSizeY      = 11;
    gpMusicStringW->mGradientTop    = {255, 255, 255, 255};
    gpMusicStringW->mGradientBottom = {255, 255, 255, 255};
    gpMusicStringB->mGradientTop    = {0, 0, 0, 255};
    gpMusicStringB->mGradientBottom = {0, 0, 0, 255};
    sIsInitialized                  = true;
}

void printStreamInfo(TApplication *app, const J2DOrthoGraph *graph) {
    TMarDirector *director = reinterpret_cast<TMarDirector *>(app->mDirector);

    if (app->mContext != TApplication::CONTEXT_DIRECT_STAGE)
        return;

    if (!director || !sIsInitialized || gDebugUIPage == 0 || gDebugUIPage > 4 ||
        !BetterSMS::isDebugMode())
        return;

    if (director->mCurState != TMarDirector::STATE_INTRO_PLAYING &&
        director->mCurState != TMarDirector::STATE_NORMAL)
        return;

    Music::AudioStreamer *streamer = Music::AudioStreamer::getInstance();
    if (!streamer)
        return;

    u32 streamStart = streamer->getStreamStart();
    u32 streamEnd   = streamer->getStreamEnd();
    u32 streamPos   = streamer->getStreamPos();
    u32 streamSize  = streamEnd - streamStart;

    #if 0
    snprintf(sStringBuffer, 200,
             "Stream:\n"
             "  Status:      %lu\n"
             "  CurAddress: 0x%lX\n"
             "  EndAddress: 0x%lX\n"
             "  FInfoSize:    0x%lX",
             streamer->getErrorStatus(), streamPos, streamEnd, streamSize);
#else

#define combine(block)                                                                             \
    (u32)((((block).mCurCommand << 24) & 0xFF000000) | (((block).mCurState << 16) & 0x00FF0000) |  \
          ((block).mCommandResult & 0x0000FFFF))

    snprintf(sStringBuffer, 200,
             "Stream:\n"
             "  AIB: 0x%08lX\n"
             "  FileB: 0x%08lX\n"
             "  PrepareB: 0x%08lX\n"
             "  AddrB: 0x%08lX\n"
             "      AI State: 0x%02X\n"
             "      Prepare State: 0x%02X\n"
             "      PlayAddr State: 0x%02X\n"
             "      Forced Stop: 0x%02X\n",
             combine(streamer->mAIInteruptBlock), combine(streamer->mAudioHandle->mCmdBlock),
             combine(streamer->mPrepareBlock), combine(streamer->mPlayAddrBlock),
             streamer->mAICBState, streamer->mPSCBState, streamer->mPACBState,
             streamer->mStopState);
    #endif

    gpMusicStringB->draw(121, 103);
    gpMusicStringW->draw(120, 102);
}