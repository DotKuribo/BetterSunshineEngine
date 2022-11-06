#include <Dolphin/MTX.h>
#include <Dolphin/types.h>

#include <JSystem/JKernel/JKRHeap.hxx>
#include <JSystem/J2D/J2DOrthoGraph.hxx>
#include <JSystem/J2D/J2DPane.hxx>
#include <JSystem/J2D/J2DTextBox.hxx>
#include <SMS/Enemy/EnemyMario.hxx>
#include <SMS/Player/Mario.hxx>

#include <SMS/npc/BaseNPC.hxx>
#include <SMS/raw_fn.hxx>
#include <SMS/MSound/MSound.hxx>
#include <SMS/MSound/MSoundSESystem.hxx>
#include <SMS/System/Application.hxx>
#include <SMS/collision/MapCollisionData.hxx>

#include "debug.hxx"
#include "libs/cheathandler.hxx"
#include "libs/constmath.hxx"
#include "logging.hxx"
#include "module.hxx"
#include "music.hxx"

using namespace BetterSMS;

static char sStringBuffer[100]{};
static J2DTextBox *gpMusicStringW = nullptr;
static J2DTextBox *gpMusicStringB = nullptr;
static bool sIsInitialized        = false;

void initStreamInfo(TApplication *app) {
    gpMusicStringW                  = new J2DTextBox(gpSystemFont->mFont, "");
    gpMusicStringB                  = new J2DTextBox(gpSystemFont->mFont, "");
    gpMusicStringW->mStrPtr         = sStringBuffer;
    gpMusicStringB->mStrPtr         = sStringBuffer;
    gpMusicStringW->mNewlineSize    = 15;
    gpMusicStringW->mCharSizeX      = 12;
    gpMusicStringW->mCharSizeY      = 15;
    gpMusicStringB->mNewlineSize    = 15;
    gpMusicStringB->mCharSizeX      = 12;
    gpMusicStringB->mCharSizeY      = 15;
    gpMusicStringW->mGradientTop    = {255, 255, 255, 255};
    gpMusicStringW->mGradientBottom = {255, 255, 255, 255};
    gpMusicStringB->mGradientTop    = {0, 0, 0, 255};
    gpMusicStringB->mGradientBottom = {0, 0, 0, 255};
    sIsInitialized                  = true;
}

void printStreamInfo(TApplication *app, J2DOrthoGraph *graph) {
    TMarDirector *director = reinterpret_cast<TMarDirector *>(app->mDirector);

    if (!director || !sIsInitialized)
        return;

    if (director->mCurState != TMarDirector::STATE_INTRO_PLAYING &&
        director->mCurState != TMarDirector::STATE_NORMAL)
        return;

    Music::AudioStreamer *streamer = Music::getAudioStreamer();
    if (!streamer)
        return;

    snprintf(sStringBuffer, 100,
             "Stream:\n"
             "  Status:      %lu\n"
             "  CurAddress: 0x%lX\n"
             "  EndAddress: 0x%lX\n"
             "  FInfoAddr:   0x%lX\n"
             "  FInfoSize:    0x%lX",
             streamer->mErrorStatus, streamer->mCurrentPlayAddress, streamer->mEndPlayAddress,
             streamer->mAudioHandle->mStart, streamer->mAudioHandle->mLen);

    gpMusicStringB->draw(111, 103);
    gpMusicStringW->draw(110, 102);
}