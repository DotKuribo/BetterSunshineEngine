#include <Dolphin/mem.h>
#include <JSystem/JGeometry.hxx>
#include <JSystem/JKernel/JKRHeap.hxx>

#include <SMS/System/Application.hxx>
#include <SMS/GC2D/SMSFader.hxx>
#include <SMS/MSound/MSoundSESystem.hxx>

#include "libs/warp.hxx"
#include "libs/triangle.hxx"
#include "logging.hxx"
#include "player.hxx"

#include "p_globals.hxx"

using namespace BetterSMS;
using namespace BetterSMS::Geometry;
using namespace BetterSMS::Collision;

#define EXPAND_WARP_SET(base) (base) : case ((base) + 10) : case ((base) + 20) : case ((base) + 30)
#define EXPAND_WARP_CATEGORY(base)                                                                 \
    (base) : case ((base) + 1) : case ((base) + 2) : case ((base) + 3) : case ((base) + 4)

static f32 GetSqrDistBetweenColTriangles(const TBGCheckData *a, const TBGCheckData *b) {
    TVectorTriangle triA(a->mVertexA, a->mVertexB, a->mVertexC);
    TVectorTriangle triB(b->mVertexA, b->mVertexB, b->mVertexC);

    TVec3f thisCenter;
    TVec3f targetCenter;

    triA.center(thisCenter);
    triB.center(targetCenter);

    return PSVECSquareDistance(reinterpret_cast<Vec *>(&thisCenter),
                               reinterpret_cast<Vec *>(&targetCenter));
}

TCollisionLink::SearchMode TCollisionLink::getSearchModeFrom(const TBGCheckData *colTriangle) {
    switch (colTriangle->mCollisionType & 0xFFF) {
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
    switch (colTriangle->mCollisionType & 0xFFF) {
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
    return static_cast<u8>(colTriangle->mValue4 >> 8);
}

u8 TCollisionLink::getHomeIDFrom(const TBGCheckData *colTriangle) {
    return static_cast<u8>(colTriangle->mValue4);
}

f32 TCollisionLink::getMinTargetDistanceFrom(const TBGCheckData *colTriangle) {
    return static_cast<f32>(colTriangle->mValue4);
}

bool TCollisionLink::isValidWarpCol(const TBGCheckData *colTriangle) {
    switch (colTriangle->mCollisionType & 0xFFF) {
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
        const f32 minDist = static_cast<f32>(other->mValue4);
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
        const f32 minDist = static_cast<f32>(getThisColTriangle()->mValue4);
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
    return static_cast<f32>(getThisColTriangle()->mValue4);
}

void TWarpCollisionList::addLink(const TBGCheckData *a, const TBGCheckData *b) {
    TCollisionLink link(a, static_cast<u8>(b->mValue4), static_cast<u8>(a->mValue4),
                        TCollisionLink::getSearchModeFrom(a));
    addLink(link);
}

void TWarpCollisionList::addLink(TCollisionLink &link) {
    if (mUsedSize >= mMaxSize) {
        Console::debugLog("TWarpCollision::addLink(): Collision list is full!\n");
        return;
    }
    Console::debugLog("TWarpCollision::addLink(): (%d) Added link of type %d at 0x%X\n",
                      link.getThisColTriangle()->mCollisionType, link.getSearchMode(),
                      &mColList[mUsedSize]);
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

const TBGCheckData *TWarpCollisionList::getNearestTarget(const TBGCheckData *colTriangle) {
    if (!TCollisionLink::isValidWarpCol(colTriangle))
        return nullptr;

    TVectorTriangle colVector(colTriangle->mVertexA, colTriangle->mVertexB, colTriangle->mVertexC);
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
            f32 minDist            = TCollisionLink::getMinTargetDistanceFrom(colTriangle);
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
            targetVector.a = mColList[matchedIndices[i]].mColTriangle->mVertexA;
            targetVector.b = mColList[matchedIndices[i]].mColTriangle->mVertexB;
            targetVector.c = mColList[matchedIndices[i]].mColTriangle->mVertexC;

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
            targetVector.a = mColList[matchedIndices[i]].mColTriangle->mVertexA;
            targetVector.b = mColList[matchedIndices[i]].mColTriangle->mVertexB;
            targetVector.c = mColList[matchedIndices[i]].mColTriangle->mVertexC;

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

    TVec3f cameraPos;
    gpCamera->JSGGetViewPosition(reinterpret_cast<Vec *>(&cameraPos));

    {
        f32 x = lerp<f32>(cameraPos.x, point.x, 0.9375f);
        f32 y = point.y + 300.0f;
        f32 z = lerp<f32>(cameraPos.z, point.z, 0.9375f);
        cameraPos.set(x, y, z);
    }

    player->JSGSetTranslation(reinterpret_cast<const Vec &>(point));
    gpCamera->JSGSetViewPosition(reinterpret_cast<Vec &>(cameraPos));
    gpCamera->JSGSetViewTargetPosition(reinterpret_cast<const Vec &>(point));
}

static void redirectPlayerWithNormal(TMario *player, const TVec3f &normal, f32 minVelocity) {
    const f32 magnitude = Max(PSVECMag(reinterpret_cast<Vec *>(&player->mSpeed)), minVelocity);

    player->mAngle.y = static_cast<u16>(Vector3::getNormalAngle(normal)) * 182;
    player->setPlayerVelocity(magnitude * normal.x + magnitude * normal.z);
    player->mSpeed.y = magnitude * normal.y;

    player->mPosition.add(TVec3f{50.0f * normal.x, 50.0f * normal.y, 50.0f * normal.z});

    if (!(player->mState & TMario::STATE_AIRBORN))
        player->changePlayerStatus(TMario::STATE_FALL, 0, false);
}

void instantWarpHandler(TMario *player, const TBGCheckData *data, u32 flags) {
    auto *playerData = Player::getData(player);

    if ((flags & Player::InteractionFlags::ON_ENTER) ||
        (flags & Player::InteractionFlags::ON_CONTACT)) {
        if (playerData->mWarpState == 0xFF)
            playerData->mIsWarpActive = true;
        playerData->mWarpState    = 0;
    }

    if ((flags & Player::InteractionFlags::ON_EXIT) ||
        (flags & Player::InteractionFlags::ON_DETACH)) {
        playerData->mIsWarpActive = false;
    }

    if (!playerData->mIsWarpActive)
        return;

    const TBGCheckData *linkedCol = BetterSMS::sWarpColArray->resolveCollisionWarp(data);
    if (!linkedCol)
        return;

    TVectorTriangle triangle(linkedCol->mVertexA, linkedCol->mVertexB, linkedCol->mVertexC);

    TVec3f center;
    triangle.center(center);

    warpPlayerToPoint(player, center);
    playerData->mIsWarpActive = false;
}

void screenWipeWarpHandler(TMario *player, const TBGCheckData *data,
                           u32 flags) {
    auto *playerData = Player::getData(player);

    if ((flags & Player::InteractionFlags::ON_ENTER) ||
         (flags & Player::InteractionFlags::ON_CONTACT)) {
        if (playerData->mWarpState == 0xFF)
            playerData->mIsWarpActive = true;
        playerData->mWarpState    = 0;
    }

    if ((flags & Player::InteractionFlags::ON_EXIT) ||
        (flags & Player::InteractionFlags::ON_DETACH)) {
        playerData->mIsWarpActive = false;
    }

    if (!playerData->mIsWarpActive)
        return;

    if (PSVECMag(player->mSpeed) > 1.0f || !(flags & Player::InteractionFlags::GROUNDED))
        return;

    const TBGCheckData *linkedCol = BetterSMS::sWarpColArray->resolveCollisionWarp(data);
    if (!linkedCol)
        return;

    constexpr s32 DisableMovementTime = 80;
    constexpr s32 EnableMovementTime  = 60;
    constexpr f32 WipeKindInDelay     = 1.0f;

    TVec3f center;
    {
        TVectorTriangle triangle(linkedCol->mVertexA, linkedCol->mVertexB, linkedCol->mVertexC);
        triangle.center(center);
    }

    TSMSFader *fader = gpApplication.mFader;

    ++playerData->mWarpTimer;
    switch (playerData->mWarpState) {
    case 0: {
        if (playerData->mWarpTimer > DisableMovementTime) {
            playerData->mCollisionFlags.mIsDisableInput = true;
            player->mController->mState.mReadInput      = false;

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

            playerData->mWarpTimer    = -1;
            playerData->mWarpState = 0xFF;
            playerData->mIsWarpActive = false;
        }
        break;
    }
    default:
        break;
    }
}

void effectWarpHandler(TMario *player, const TBGCheckData *data, u32 flags) {
    auto *playerData = Player::getData(player);

    if ((flags & Player::InteractionFlags::ON_ENTER) ||
        (flags & Player::InteractionFlags::ON_CONTACT)) {
        if (playerData->mWarpState == 0xFF)
            playerData->mIsWarpActive = true;
        playerData->mWarpState    = 0;
    }

    if ((flags & Player::InteractionFlags::ON_EXIT) ||
        (flags & Player::InteractionFlags::ON_DETACH)) {
        playerData->mIsWarpActive = false;
    }

    if (!playerData->mIsWarpActive)
        return;

    if (PSVECMag(player->mSpeed) > 1.0f || !(flags & Player::InteractionFlags::GROUNDED))
        return;

    const TBGCheckData *linkedCol = BetterSMS::sWarpColArray->resolveCollisionWarp(data);
    if (!linkedCol)
        return;

    constexpr s32 DisableMovementTime = 80;
    constexpr s32 TeleportTime        = 60;
    constexpr s32 EnableMovementTime  = 60;

    TVec3f center;
    {
        TVectorTriangle triangle(linkedCol->mVertexA, linkedCol->mVertexB, linkedCol->mVertexC);
        triangle.center(center);
    }

    ++playerData->mWarpTimer;
    switch (playerData->mWarpState) {
    case 0: {
        if (playerData->mWarpTimer > DisableMovementTime) {
            playerData->mCollisionFlags.mIsDisableInput = true;
            player->mController->mState.mReadInput      = false;

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

            playerData->mWarpTimer    = -1;
            playerData->mWarpState = 0xFF;
            playerData->mIsWarpActive = false;
        }
        break;
    }
    default:
        break;
    }
}

void portalWarpHandler(TMario *player, const TBGCheckData *data, u32 flags) {
    auto *playerData = Player::getData(player);

    if ((flags & Player::InteractionFlags::ON_ENTER)) {
        if (playerData->mWarpState == 0xFF)
            playerData->mIsWarpActive = true;
        playerData->mWarpState    = 0;
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
        TVectorTriangle triangle(linkedCol->mVertexA, linkedCol->mVertexB, linkedCol->mVertexC);
        triangle.center(center);
    }

    warpPlayerToPoint(player, center);
    redirectPlayerWithNormal(player, *linkedCol->getNormal(), 50.0f);
}

void portalFreeWarpHandler(TMario *player, const TBGCheckData *data,
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
        TVectorTriangle triangle(linkedCol->mVertexA, linkedCol->mVertexB, linkedCol->mVertexC);
        triangle.center(center);
    }

    warpPlayerToPoint(player, center);
    redirectPlayerWithNormal(player, *linkedCol->getNormal(), 50.0f);
}