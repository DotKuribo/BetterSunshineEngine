#include <Dolphin/mem.h>
#include <JSystem/JGeometry/JGMVec.hxx>
#include <JSystem/JKernel/JKRHeap.hxx>

#include <SMS/GC2D/SMSFader.hxx>
#include <SMS/MSound/MSoundSESystem.hxx>
#include <SMS/System/Application.hxx>

#include "libs/global_unordered_map.hxx"
#include "libs/triangle.hxx"
#include "logging.hxx"
#include "player.hxx"

#include "libs/geometry.hxx"
#include "p_globals.hxx"
#include "p_warp.hxx"

using namespace BetterSMS;
using namespace BetterSMS::Collision;
using namespace BetterSMS::Geometry;

#define EXPAND_WARP_SET(base)                                                                      \
    (base) : case ((base) + 10) :                                                                  \
    case ((base) + 20):                                                                            \
    case ((base) + 30)
#define EXPAND_WARP_CATEGORY(base)                                                                 \
    (base) : case ((base) + 1) :                                                                   \
    case ((base) + 2):                                                                             \
    case ((base) + 3):                                                                             \
    case ((base) + 4)

static f32 GetSqrDistBetweenColTriangles(const TBGCheckData *a, const TBGCheckData *b) {
    TVectorTriangle triA(a->mVertices[0], a->mVertices[1], a->mVertices[2]);
    TVectorTriangle triB(b->mVertices[0], b->mVertices[1], b->mVertices[2]);

    TVec3f thisCenter;
    TVec3f targetCenter;

    triA.center(thisCenter);
    triB.center(targetCenter);

    return PSVECSquareDistance(reinterpret_cast<Vec *>(&thisCenter),
                               reinterpret_cast<Vec *>(&targetCenter));
}

TCollisionLink::SearchMode TCollisionLink::getSearchModeFrom(const TBGCheckData *colTriangle) {
    switch (colTriangle->mType & 0xFFF) {
    case EXPAND_WARP_CATEGORY(3060): {
        return SearchMode::BOTH;
    }
    case EXPAND_WARP_CATEGORY(3070): {
        return SearchMode::DISTANCE;
    }
    case EXPAND_WARP_CATEGORY(3080): {
        return SearchMode::HOME_TO_TARGET;
    }
    default:
        return SearchMode::BOTH;
    }
}

TCollisionLink::WarpType TCollisionLink::getWarpTypeFrom(const TBGCheckData *colTriangle) {
    switch (colTriangle->mType & 0xFFF) {
    case EXPAND_WARP_SET(3060): {
        return WarpType::INSTANT;
    }
    case EXPAND_WARP_SET(3061): {
        return WarpType::SLOW_SWIPE;
    }
    case EXPAND_WARP_SET(3062): {
        return WarpType::SLOW_SPARKLE;
    }
    case EXPAND_WARP_SET(3063): {
        return WarpType::PORTAL;
    }
    case EXPAND_WARP_SET(3064): {
        return WarpType::PORTAL_FLUID;
    }
    default:
        return WarpType::INSTANT;
    }
}

u8 TCollisionLink::getTargetIDFrom(const TBGCheckData *colTriangle) {
    return static_cast<u8>(colTriangle->mValue >> 8);
}

u8 TCollisionLink::getHomeIDFrom(const TBGCheckData *colTriangle) {
    return static_cast<u8>(colTriangle->mValue);
}

f32 TCollisionLink::getMinTargetDistanceFrom(const TBGCheckData *colTriangle) {
    return static_cast<f32>(colTriangle->mValue);
}

bool TCollisionLink::isValidWarpCol(const TBGCheckData *colTriangle) {
    switch (colTriangle->mType & 0xFFF) {
    case EXPAND_WARP_SET(3060):
    case EXPAND_WARP_SET(3061):
    case EXPAND_WARP_SET(3062):
    case EXPAND_WARP_SET(3063):
    case EXPAND_WARP_SET(3064):
        return true;
    default:
        return false;
    }
}

#undef EXPAND_WARP_SET
#undef EXPAND_WARP_CATEGORY

bool TCollisionLink::isTargetOf(const TBGCheckData *other) const {
    if (!isValidWarpCol(other)) {
        return false;
    }

    if (getSearchModeFrom(other) == SearchMode::BOTH || getSearchMode() == SearchMode::BOTH) {
        return true;
    } else if (getSearchModeFrom(other) == SearchMode::HOME_TO_TARGET &&
               getSearchMode() == SearchMode::HOME_TO_TARGET) {
        const u8 targetID = getTargetIDFrom(other);
        return (targetID != NullID) && (targetID == mTargetID);
    } else if (getSearchModeFrom(other) == SearchMode::DISTANCE &&
               getSearchMode() == SearchMode::DISTANCE) {
        const f32 minDist = static_cast<f32>(other->mValue);
        return (minDist * minDist) < GetSqrDistBetweenColTriangles(other, getThisColTriangle());
    }

    return false;
}

bool TCollisionLink::isTargeting(const TBGCheckData *other) const {
    if (!isValidWarpCol(other)) {
        return false;
    }

    if (getSearchMode() == SearchMode::BOTH || getSearchModeFrom(other) == SearchMode::BOTH) {
        return true;
    } else if (getSearchMode() == SearchMode::HOME_TO_TARGET &&
               getSearchModeFrom(other) == SearchMode::HOME_TO_TARGET) {
        return (getHomeIDFrom(other) == mTargetID) && (mTargetID != NullID);
    } else if (getSearchMode() == SearchMode::DISTANCE &&
               getSearchModeFrom(other) == SearchMode::DISTANCE) {
        const f32 minDist = static_cast<f32>(getThisColTriangle()->mValue);
        return (minDist * minDist) < GetSqrDistBetweenColTriangles(getThisColTriangle(), other);
    }

    return false;
}

// Check if this link provides a valid home id for other links to reference
bool TCollisionLink::isValidDest() const {
    return (mHomeID != 0xFF) || (getSearchMode() == SearchMode::DISTANCE);
}

// Check if this link provides a valid target id to reference other links with
bool TCollisionLink::isValidSrc() const {
    return (mTargetID != 0xFF) || (getSearchMode() == SearchMode::DISTANCE);
}

f32 TCollisionLink::getMinTargetDistance() const {
    return static_cast<f32>(getThisColTriangle()->mValue);
}

void TWarpCollisionList::addLink(const TBGCheckData *a, const TBGCheckData *b) {
    TCollisionLink link(a, static_cast<u8>(b->mValue), static_cast<u8>(a->mValue),
                        TCollisionLink::getSearchModeFrom(a));
    addLink(link);
}

void TWarpCollisionList::addLink(TCollisionLink &link) {
    if (mUsedSize >= mMaxSize) {
        Console::debugLog("TWarpCollision::addLink(): Collision list is full!\n");
        return;
    }
    Console::debugLog("TWarpCollision::addLink(): (%d) Added link of type %d at 0x%X\n",
                      link.getThisColTriangle()->mType, link.getSearchMode(), &mColList[mUsedSize]);
    mColList[mUsedSize++] = link;
}

void TWarpCollisionList::removeLink(const TBGCheckData *home, const TBGCheckData *target) {
    TCollisionLink *link;
    for (u32 i = 0; i < mUsedSize; ++i) {
        link = &mColList[i];
        if (link->mColTriangle == home &&
            link->mTargetID == TCollisionLink::getHomeIDFrom(target)) {
            removeLinkByIndex(i);
        }
    }
}

void TWarpCollisionList::removeLink(TCollisionLink *colLink) {
    TCollisionLink *link;
    for (u32 i = 0; i < mUsedSize; ++i) {
        link = &mColList[i];
        if (link == colLink) {
            removeLinkByIndex(i);
        }
    }
}

void TWarpCollisionList::removeLinkByIndex(u32 index) {
    memmove(&mColList[index], &mColList[index + 1],
            sizeof(TCollisionLink *) * (mUsedSize - index + 1));
    mUsedSize -= 1;
}

const TBGCheckData *TWarpCollisionList::resolveCollisionWarp(const TBGCheckData *colTriangle) {
    if (TCollisionLink::getTargetIDFrom(colTriangle) == TCollisionLink::NullID)
        return nullptr;

    return getNearestTarget(colTriangle);
}

const TBGCheckData *TWarpCollisionList::getNearestTarget(const TBGCheckData *colTriangle) const {
    if (!TCollisionLink::isValidWarpCol(colTriangle))
        return nullptr;

    TVectorTriangle colVector(colTriangle->mVertices[0], colTriangle->mVertices[1],
                              colTriangle->mVertices[2]);
    TVectorTriangle targetVector;

    u16 matchedIndices[mMaxSize];
    f32 nearestDist = __FLT_MAX__;

    switch (TCollisionLink::getSearchModeFrom(colTriangle)) {
    case TCollisionLink::SearchMode::HOME_TO_TARGET: {
        for (u32 i = 0; i < size(); ++i) {
            const TCollisionLink &colLink = mColList[i];
            if (colLink.getSearchMode() == TCollisionLink::SearchMode::DISTANCE ||
                !colLink.isValidDest())
                continue;
            if (colLink.getHomeID() == TCollisionLink::getTargetIDFrom(colTriangle) &&
                colLink.getThisColTriangle() != colTriangle) {
                return colLink.getThisColTriangle();
            }
            return nullptr;
        }
    }
    case TCollisionLink::SearchMode::DISTANCE: {
        s32 numLinked = 0;
        for (u32 i = 0; i < size(); ++i) {
            const TCollisionLink &colLink = mColList[i];
            f32 minDist                   = TCollisionLink::getMinTargetDistanceFrom(colTriangle);
            if (!colLink.isValidDest())
                continue;
            if (GetSqrDistBetweenColTriangles(colLink.getThisColTriangle(), colTriangle) >
                    (minDist * minDist) &&
                colLink.getThisColTriangle() != colTriangle) {
                matchedIndices[numLinked++] = i;
            }
        }

        TVec3f a;
        TVec3f b;
        s32 index = -1;

        for (u32 i = 0; i < numLinked; ++i) {
            targetVector.a = mColList[matchedIndices[i]].mColTriangle->mVertices[0];
            targetVector.b = mColList[matchedIndices[i]].mColTriangle->mVertices[1];
            targetVector.c = mColList[matchedIndices[i]].mColTriangle->mVertices[2];

            TVec3f thisCenter;
            TVec3f targetCenter;

            colVector.center(thisCenter);
            targetVector.center(targetCenter);

            f32 sqrDist = PSVECSquareDistance(reinterpret_cast<Vec *>(&thisCenter),
                                              reinterpret_cast<Vec *>(&targetCenter));
            if (sqrDist < nearestDist) {
                nearestDist = sqrDist;
                index       = i;
            }
        }
        if (index == -1)
            return nullptr;

        return mColList[matchedIndices[index]].mColTriangle;
    }
    case TCollisionLink::SearchMode::BOTH: {
        s32 numLinked = 0;
        for (u32 i = 0; i < size(); ++i) {
            if (!mColList[i].isValidDest())
                continue;
            if (mColList[i].getHomeID() == TCollisionLink::getTargetIDFrom(colTriangle) &&
                mColList[i].getThisColTriangle() != colTriangle) {
                matchedIndices[numLinked++] = i;
            }
        }

        TVec3f a;
        TVec3f b;
        s32 index = -1;

        for (u32 i = 0; i < numLinked; ++i) {
            targetVector.a = mColList[matchedIndices[i]].mColTriangle->mVertices[0];
            targetVector.b = mColList[matchedIndices[i]].mColTriangle->mVertices[1];
            targetVector.c = mColList[matchedIndices[i]].mColTriangle->mVertices[2];

            TVec3f thisCenter;
            TVec3f targetCenter;

            colVector.center(thisCenter);
            targetVector.center(targetCenter);

            f32 sqrDist = PSVECSquareDistance(reinterpret_cast<Vec *>(&thisCenter),
                                              reinterpret_cast<Vec *>(&targetCenter));
            if (sqrDist < nearestDist) {
                nearestDist = sqrDist;
                index       = i;
            }
        }
        if (index == -1)
            return nullptr;

        return mColList[matchedIndices[index]].mColTriangle;
    }
    }
}

static void warpPlayerToPoint(TMario *player, const TVec3f &point) {
    if (!player)
        return;

    player->mTranslation = point;

    gpCamera->mTranslation.x = lerp<f32>(gpCamera->mTranslation.x, point.x, 0.9375f);
    gpCamera->mTranslation.y = point.y + 300.0f;
    gpCamera->mTranslation.z = lerp<f32>(gpCamera->mTranslation.z, point.z, 0.9375f);
    gpCamera->mTargetPos     = point;
}

static void redirectPlayerWithNormal(TMario *player, const TVec3f &normal, f32 minVelocity) {
    const f32 magnitude = Max(PSVECMag(reinterpret_cast<Vec *>(&player->mSpeed)), minVelocity);

    player->mAngle.y = convertAngleFloatToS16(Vector3::getNormalAngle(normal));
    player->setPlayerVelocity(magnitude * normal.x + magnitude * normal.z);
    player->mSpeed.y = magnitude * normal.y;

    player->mTranslation.add(TVec3f{50.0f * normal.x, 50.0f * normal.y, 50.0f * normal.z});

    if (!(player->mState & TMario::STATE_AIRBORN))
        player->changePlayerStatus(TMario::STATE_FALL, 0, false);
}

struct WarpCallbackInfo {
    const TBGCheckData *source;
    const TBGCheckData *dest;
    void (*callback)(TMario *, const TBGCheckData *, const TBGCheckData *);
};

static TGlobalUnorderedMap<TMario *, WarpCallbackInfo> sCallbackMap(4);

static void defaultWarpCallback(TMario *player, const TBGCheckData *src, const TBGCheckData *dst) {}

BETTER_SMS_FOR_CALLBACK void initializeWarpCallback(TMario *player, bool isMario) {
    sCallbackMap[player] = {player->mFloorTriangle, nullptr, defaultWarpCallback};
}

BETTER_SMS_FOR_CALLBACK void processWarpCallback(TMario *player, bool isMario) {
    auto callbackInfo = sCallbackMap[player];
    callbackInfo.callback(player, callbackInfo.source, callbackInfo.dest);
}

BETTER_SMS_FOR_CALLBACK void instantWarpHandler(TMario *player, const TBGCheckData *data,
                                                u32 flags) {
    auto *playerData = Player::getData(player);

    if (((flags & Player::InteractionFlags::ON_ENTER) ||
         (flags & Player::InteractionFlags::ON_CONTACT)) &&
        (flags & Player::InteractionFlags::GROUNDED)) {
        if (playerData->mWarpState == 0xFF) {
            const TBGCheckData *linkedCol = BetterSMS::sWarpColArray->resolveCollisionWarp(data);
            if (!linkedCol)
                return;

            TVectorTriangle triangle(linkedCol->mVertices[0], linkedCol->mVertices[1],
                                     linkedCol->mVertices[2]);

            TVec3f center;
            triangle.center(center);

            warpPlayerToPoint(player, center);
            playerData->mWarpState          = 0xFF;
            playerData->mPrevCollisionFloor = player->mFloorTriangle;
            playerData->mPrevCollisionType  = player->mFloorTriangle->mType;
        }
    }
}

static void internalWipeWarpHandler(TMario *player, const TBGCheckData *src,
                                    const TBGCheckData *dst) {
    auto *playerData = Player::getData(player);

    constexpr s32 DisableMovementTime = 80;
    constexpr s32 EnableMovementTime  = 60;
    constexpr f32 WipeKindInDelay     = 1.0f;

    TVec3f center;
    {
        TVectorTriangle triangle(dst->mVertices[0], dst->mVertices[1], dst->mVertices[2]);
        triangle.center(center);
    }

    TSMSFader *fader = gpApplication.mFader;

    ++playerData->mWarpTimer;
    switch (playerData->mWarpState) {
    case 0: {
        if (player->mFloorTriangle->mType != src->mType) {
            playerData->mWarpTimer    = -1;
            playerData->mWarpState    = 0xFF;
            playerData->mIsWarpActive = false;
            sCallbackMap[player]      = {player->mFloorTriangle, nullptr, defaultWarpCallback};
        }

        if (playerData->mWarpTimer > DisableMovementTime) {
            playerData->mCollisionFlags.mIsDisableInput = true;
            player->mController->mState.mReadInput      = false;
            player->mController->mState.mDisable        = true;

            playerData->mWarpTimer = 0;
            playerData->mWarpState = 1;

            gpApplication.mFader->startWipe(TSMSFader::WipeRequest::FADE_SPIRAL_OUT, 1.0f, 0.0f);
            MSoundSE::startSoundSystemSE(0x4859, 0, nullptr, 0);
        }
        break;
    }
    case 1: {
        if (fader->mFadeStatus == TSMSFader::FADE_ON) {
            warpPlayerToPoint(player, center);

            playerData->mWarpTimer = 0;
            playerData->mWarpState = 2;

            fader->startWipe(TSMSFader::WipeRequest::FADE_CIRCLE_IN, 1.0f, WipeKindInDelay);
        }
        break;
    }
    case 2: {
        if (playerData->mWarpTimer > EnableMovementTime) {
            playerData->mCollisionFlags.mIsDisableInput = false;
            player->mController->mState.mReadInput      = true;
            player->mController->mState.mDisable        = false;

            playerData->mWarpTimer    = -1;
            playerData->mWarpState    = 0xFF;
            playerData->mIsWarpActive = false;
            sCallbackMap[player]      = {player->mFloorTriangle, nullptr, defaultWarpCallback};
        }
        break;
    }
    default:
        break;
    }
}

BETTER_SMS_FOR_CALLBACK void screenWipeWarpHandler(TMario *player, const TBGCheckData *data,
                                                   u32 flags) {
    auto *playerData = Player::getData(player);

    if (((flags & Player::InteractionFlags::ON_ENTER) ||
         (flags & Player::InteractionFlags::ON_CONTACT)) &&
        (flags & Player::InteractionFlags::GROUNDED)) {
        if (playerData->mWarpState == 0xFF) {
            if (PSVECMag(player->mSpeed) > 1.0f || !(flags & Player::InteractionFlags::GROUNDED))
                return;

            const TBGCheckData *linkedCol = BetterSMS::sWarpColArray->resolveCollisionWarp(data);
            if (!linkedCol)
                return;

            sCallbackMap[player] = {player->mFloorTriangle, linkedCol, internalWipeWarpHandler};
            playerData->mIsWarpActive = true;
            playerData->mWarpState    = 0;
        }
    }
}

BETTER_SMS_FOR_CALLBACK void instantScreenWipeWarpHandler(TMario *player, const TBGCheckData *data,
                                                          u32 flags) {
    auto *playerData = Player::getData(player);

    if (!(flags & Player::InteractionFlags::GROUNDED)) {
        return;
    }

    if ((flags & Player::InteractionFlags::ON_ENTER) ||
        (flags & Player::InteractionFlags::ON_CONTACT)) {
        if (playerData->mWarpState == 0xFF) {
            const TBGCheckData *linkedCol = BetterSMS::sWarpColArray->resolveCollisionWarp(data);
            if (!linkedCol)
                return;

            sCallbackMap[player] = {player->mFloorTriangle, linkedCol, internalWipeWarpHandler};
            playerData->mIsWarpActive = true;
            playerData->mWarpState    = 0;
            playerData->mWarpTimer    = 81;
        }
    }
}

static void internalEffectWarpHandler(TMario *player, const TBGCheckData *src,
                                      const TBGCheckData *dst) {
    constexpr s32 DisableMovementTime = 80;
    constexpr s32 TeleportTime        = 60;
    constexpr s32 EnableMovementTime  = 60;

    auto *playerData = Player::getData(player);

    TVec3f center;
    {
        TVectorTriangle triangle(dst->mVertices[0], dst->mVertices[1], dst->mVertices[2]);
        triangle.center(center);
    }

    ++playerData->mWarpTimer;
    switch (playerData->mWarpState) {
    case 0: {
        if (PSVECMag(player->mSpeed) > 1.0f) {
            if (player->mFloorTriangle->mType == src->mType) {
                playerData->mWarpTimer = 0;
            } else {
                playerData->mWarpTimer    = -1;
                playerData->mWarpState    = 0xFF;
                playerData->mIsWarpActive = false;
                sCallbackMap[player]      = {player->mFloorTriangle, nullptr, defaultWarpCallback};
            }
            break;
        }

        if (playerData->mWarpTimer > DisableMovementTime) {
            playerData->mCollisionFlags.mIsDisableInput = true;
            player->mController->mState.mReadInput      = false;
            player->mController->mState.mDisable        = true;

            playerData->mWarpTimer = 0;
            playerData->mWarpState = 1;

            player->emitGetEffect();
        }
        break;
    }
    case 1: {
        if (playerData->mWarpTimer > TeleportTime) {
            warpPlayerToPoint(player, center);

            playerData->mWarpTimer = 0;
            playerData->mWarpState = 2;

            player->startSoundActor(TMario::VOICE_JUMP);
        }
        break;
    }
    case 2: {
        if (playerData->mWarpTimer >= EnableMovementTime) {
            playerData->mCollisionFlags.mIsDisableInput = false;
            player->mController->mState.mReadInput      = true;
            player->mController->mState.mDisable        = false;

            playerData->mWarpTimer    = -1;
            playerData->mWarpState    = 0xFF;
            playerData->mIsWarpActive = false;
            sCallbackMap[player]      = {player->mFloorTriangle, nullptr, defaultWarpCallback};
        }
        break;
    }
    default:
        break;
    }
}

BETTER_SMS_FOR_CALLBACK void effectWarpHandler(TMario *player, const TBGCheckData *data,
                                               u32 flags) {
    auto *playerData = Player::getData(player);

    if (((flags & Player::InteractionFlags::ON_ENTER) ||
         (flags & Player::InteractionFlags::ON_CONTACT)) &&
        (flags & Player::InteractionFlags::GROUNDED)) {
        if (playerData->mWarpState == 0xFF) {
            const TBGCheckData *linkedCol = BetterSMS::sWarpColArray->resolveCollisionWarp(data);
            if (!linkedCol)
                return;

            sCallbackMap[player] = {player->mFloorTriangle, linkedCol, internalEffectWarpHandler};
            playerData->mIsWarpActive = true;
            playerData->mWarpState    = 0;
        }
    }
}

BETTER_SMS_FOR_CALLBACK void portalWarpHandler(TMario *player, const TBGCheckData *data,
                                               u32 flags) {
    auto *playerData = Player::getData(player);

    if ((flags & Player::InteractionFlags::ON_ENTER)) {
        if (playerData->mWarpState == 0xFF)
            playerData->mIsWarpActive = true;
        playerData->mWarpState = 0;
    }

    if ((flags & Player::InteractionFlags::ON_EXIT)) {
        playerData->mIsWarpActive = false;
    }

    if (!playerData->mIsWarpActive || !(flags & Player::InteractionFlags::ON_4CM_CONTACT))
        return;

    const TBGCheckData *linkedCol = BetterSMS::sWarpColArray->resolveCollisionWarp(data);
    if (!linkedCol)
        return;

    TVec3f center;
    {
        TVectorTriangle triangle(linkedCol->mVertices[0], linkedCol->mVertices[1],
                                 linkedCol->mVertices[2]);
        triangle.center(center);
    }

    warpPlayerToPoint(player, center);
    redirectPlayerWithNormal(player, *linkedCol->getNormal(), 50.0f);
}

BETTER_SMS_FOR_CALLBACK void portalFreeWarpHandler(TMario *player, const TBGCheckData *data,
                                                   u32 flags) {
    auto *playerData = Player::getData(player);

    if (!(flags & Player::InteractionFlags::ON_4CM_CONTACT) &&
        !(flags & Player::InteractionFlags::ON_CONTACT) &&
        !(flags & Player::InteractionFlags::GROUNDED))
        return;

    const TBGCheckData *linkedCol = BetterSMS::sWarpColArray->resolveCollisionWarp(data);
    if (!linkedCol)
        return;

    TVec3f center;
    {
        TVectorTriangle triangle(linkedCol->mVertices[0], linkedCol->mVertices[1],
                                 linkedCol->mVertices[2]);
        triangle.center(center);
    }

    warpPlayerToPoint(player, center);
    redirectPlayerWithNormal(player, *linkedCol->getNormal(), 50.0f);
}