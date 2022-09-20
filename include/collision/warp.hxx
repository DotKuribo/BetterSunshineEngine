#pragma once

#include <Dolphin/types.h>
#include <JSystem/JGeometry.hxx>
#include <JSystem/JKernel/JKRHeap.hxx>
#include <SMS/collision/BGCheck.hxx>

#include "geometry.hxx"

namespace BetterSMS {
    namespace Collision {

        enum class WarpKind : u8 { SPARKLES, WIPE, INSTANT };

        class TCollisionLink {
        public:
            friend class TWarpCollisionList;

            static constexpr u8 NullID = 0xFF;
            enum class SearchMode : u8 { HOME_TO_TARGET, DISTANCE, BOTH };
            enum class WarpType : u8 { SLOW_SPARKLE, PORTAL, PORTAL_FLUID, SLOW_SWIPE, INSTANT };

            TCollisionLink() {}
            TCollisionLink(SearchMode mode)
                : mColTriangle(nullptr), mTargetID(NullID), mHomeID(NullID), mSearchMode(mode) {}
            TCollisionLink(TBGCheckData *tri, u8 targetID, u8 homeID, SearchMode mode)
                : mColTriangle(tri), mTargetID(targetID), mHomeID(homeID), mSearchMode(mode) {}

            static SearchMode getSearchModeFrom(TBGCheckData *colTriangle);
            static WarpType getWarpTypeFrom(TBGCheckData *colTriangle);
            static u8 getTargetIDFrom(TBGCheckData *colTriangle);
            static u8 getHomeIDFrom(TBGCheckData *colTriangle);
            static f32 getMinTargetDistanceFrom(TBGCheckData *colTriangle);
            static u8 getCustomFlagFrom(TBGCheckData *colTriangle) {
                return reinterpret_cast<u8 *>(colTriangle)[7];
            }

            // Check if the given TBGCheckData has a valid warp collision type
            static bool isValidWarpCol(TBGCheckData *colTriangle);

            // Check if this link's triangle is referenced by the target ID of `other`
            bool isTargetOf(TBGCheckData *other) const;

            // Check if this link's triangle is referencing `other` by its target ID
            bool isTargeting(TBGCheckData *other) const;

            // Check if this link provides a valid home id for other links to reference
            bool isValidDest() const;

            // Check if this link provides a valid target id to reference other links with
            bool isValidSrc() const;

            f32 getMinTargetDistance() const;

            TBGCheckData *getThisColTriangle() const { return mColTriangle; }
            u8 getTargetID() const { return mTargetID; }
            u8 getHomeID() const { return mHomeID; }
            SearchMode getSearchMode() const { return mSearchMode; }

            void setThisColTriangle(TBGCheckData *colTriangle) { mColTriangle = colTriangle; }
            void setTargetID(u8 id) { mTargetID = id; }
            void setHomeID(u8 id) { mHomeID = id; }
            void setSearchMode(SearchMode mode) { mSearchMode = mode; }

        private:
            TBGCheckData *mColTriangle;  // 0x0000
            u8 mTargetID;                // 0x0004
            u8 mHomeID;                  // 0x0005
            SearchMode mSearchMode;
        };

        class TWarpCollisionList {
        public:
            TWarpCollisionList(size_t size) : mUsedSize(0), mMaxSize(size) {
                mColList = reinterpret_cast<Collision::TCollisionLink *>(
                    JKRHeap::sCurrentHeap->alloc(sizeof(Collision::TCollisionLink) * size, 4));
            };
            ~TWarpCollisionList() { delete mColList; }

            void addLink(TBGCheckData *a, TBGCheckData *b);
            void addLink(TCollisionLink &link);

            void removeLink(TBGCheckData *a, TBGCheckData *b);
            void removeLink(TCollisionLink *link);
            void removeLinkByIndex(u32 index);

            size_t size() const { return mUsedSize; }
            size_t maxsize() const { return mMaxSize; }

            TCollisionLink *getLinks() const { return mColList; }

            TBGCheckData *resolveCollisionWarp(TBGCheckData *colTriangle);
            TBGCheckData *getNearestTarget(TBGCheckData *colTriangle);

        private:
            size_t mUsedSize;
            size_t mMaxSize;
            TCollisionLink *mColList;
        };
    }  // namespace Collision
}  // namespace BetterSMS