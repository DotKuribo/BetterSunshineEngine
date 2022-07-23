#include "collision/warp.hxx"
#include "libs/triangle.hxx"
#include "logging.hxx"
#include "mem.h"
#include <JSystem/JGeometry.hxx>
#include <JSystem/JKernel/JKRHeap.hxx>

using namespace BetterSMS;
using namespace BetterSMS::Geometry;
using namespace BetterSMS::Collision;

#define EXPAND_WARP_SET(base) (base) : case ((base) + 10) : case ((base) + 20) : case ((base) + 30)
#define EXPAND_WARP_CATEGORY(base)                                                                 \
    (base) : case ((base) + 1) : case ((base) + 2) : case ((base) + 3) : case ((base) + 4)

static f32 GetSqrDistBetweenColTriangles(TBGCheckData *a, TBGCheckData *b) {
    TVectorTriangle triA(a->mVertexA, a->mVertexB, a->mVertexC);
    TVectorTriangle triB(b->mVertexA, b->mVertexB, b->mVertexC);

    TVec3f thisCenter;
    TVec3f targetCenter;

    triA.center(thisCenter);
    triB.center(targetCenter);

    return PSVECSquareDistance(reinterpret_cast<Vec *>(&thisCenter),
                               reinterpret_cast<Vec *>(&targetCenter));
}

TCollisionLink::SearchMode TCollisionLink::getSearchModeFrom(TBGCheckData *colTriangle) {
    switch (colTriangle->mCollisionType) {
    case EXPAND_WARP_CATEGORY(16040):
        return SearchMode::BOTH;
    case EXPAND_WARP_CATEGORY(16050):
        return SearchMode::DISTANCE;
    case EXPAND_WARP_CATEGORY(16060):
        return SearchMode::HOME_TO_TARGET;
    case EXPAND_WARP_CATEGORY(17040):
        return SearchMode::BOTH;
    case EXPAND_WARP_CATEGORY(17050):
        return SearchMode::DISTANCE;
    case EXPAND_WARP_CATEGORY(17060):
        return SearchMode::HOME_TO_TARGET;
    default:
        return SearchMode::BOTH;
    }
}

TCollisionLink::WarpType TCollisionLink::getWarpTypeFrom(TBGCheckData *colTriangle) {
    switch (colTriangle->mCollisionType) {
    case EXPAND_WARP_SET(16040):
        return WarpType::SLOW_SPARKLE;
    case EXPAND_WARP_SET(16041):
        return WarpType::PORTAL;
    case EXPAND_WARP_SET(16042):
        return WarpType::PORTAL_FLUID;
    case EXPAND_WARP_SET(16043):
        return WarpType::SLOW_SWIPE;
    case EXPAND_WARP_SET(16044):
        return WarpType::INSTANT;
    case EXPAND_WARP_SET(17040):
        return WarpType::SLOW_SPARKLE;
    case EXPAND_WARP_SET(17041):
        return WarpType::PORTAL;
    case EXPAND_WARP_SET(17042):
        return WarpType::PORTAL_FLUID;
    case EXPAND_WARP_SET(17043):
        return WarpType::SLOW_SWIPE;
    case EXPAND_WARP_SET(17044):
        return WarpType::INSTANT;
    default:
        return WarpType::INSTANT;
    }
}

u8 TCollisionLink::getTargetIDFrom(TBGCheckData *colTriangle) {
    return static_cast<u8>(colTriangle->mValue4 >> 8);
}

u8 TCollisionLink::getHomeIDFrom(TBGCheckData *colTriangle) {
    return static_cast<u8>(colTriangle->mValue4);
}

f32 TCollisionLink::getMinTargetDistanceFrom(TBGCheckData *colTriangle) {
    return static_cast<f32>(colTriangle->mValue4);
}

bool TCollisionLink::isValidWarpCol(TBGCheckData *colTriangle) {
    switch (colTriangle->mCollisionType) {
    case EXPAND_WARP_SET(16040):
    case EXPAND_WARP_SET(16041):
    case EXPAND_WARP_SET(16042):
    case EXPAND_WARP_SET(16043):
    case EXPAND_WARP_SET(16044):
    case EXPAND_WARP_SET(16045):
    case EXPAND_WARP_SET(16046):
    case EXPAND_WARP_SET(17040):
    case EXPAND_WARP_SET(17041):
    case EXPAND_WARP_SET(17042):
    case EXPAND_WARP_SET(17043):
    case EXPAND_WARP_SET(17044):
    case EXPAND_WARP_SET(17045):
    case EXPAND_WARP_SET(17046):
        return true;
    default:
        return false;
    }
}

#undef EXPAND_WARP_SET
#undef EXPAND_WARP_CATEGORY

bool TCollisionLink::isTargetOf(TBGCheckData *other) const {
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

bool TCollisionLink::isTargeting(TBGCheckData *other) const {
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

void TWarpCollisionList::addLink(TBGCheckData *a, TBGCheckData *b) {
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

void TWarpCollisionList::removeLink(TBGCheckData *home, TBGCheckData *target) {
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

TBGCheckData *TWarpCollisionList::resolveCollisionWarp(TBGCheckData *colTriangle) {
    if (TCollisionLink::getTargetIDFrom(colTriangle) == TCollisionLink::NullID)
        return nullptr;

    return getNearestTarget(colTriangle);
}

TBGCheckData *TWarpCollisionList::getNearestTarget(TBGCheckData *colTriangle) {
    if (!TCollisionLink::isValidWarpCol(colTriangle))
        return nullptr;

    TVectorTriangle colVector(colTriangle->mVertexA, colTriangle->mVertexB, colTriangle->mVertexC);
    TVectorTriangle targetVector;

    u16 matchedIndices[mMaxSize];
    f32 nearestDist = __FLT_MAX__;

    switch (TCollisionLink::getSearchModeFrom(colTriangle)) {
    case TCollisionLink::SearchMode::HOME_TO_TARGET: {
        for (u32 i = 0; i < size(); ++i) {
            TCollisionLink colLink = mColList[i];
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
            TCollisionLink colLink = mColList[i];
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