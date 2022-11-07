#pragma once

#include <Dolphin/types.h>
#include <JSystem/JGeometry.hxx>
#include <JSystem/JKernel/JKRHeap.hxx>
#include <SMS/Map/BGCheck.hxx>

#include "libs/geometry.hxx"

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
            TCollisionLink(const TBGCheckData *tri, u8 targetID, u8 homeID, SearchMode mode)
                : mColTriangle(tri), mTargetID(targetID), mHomeID(homeID), mSearchMode(mode) {}

            static SearchMode getSearchModeFrom(const TBGCheckData *colTriangle);
            static WarpType getWarpTypeFrom(const TBGCheckData *colTriangle);
            static u8 getTargetIDFrom(const TBGCheckData *colTriangle);
            static u8 getHomeIDFrom(const TBGCheckData *colTriangle);
            static f32 getMinTargetDistanceFrom(const TBGCheckData *colTriangle);
            static u8 getCustomFlagFrom(const TBGCheckData *colTriangle) {
                return reinterpret_cast<const u8 *>(colTriangle)[7];
            }

            // Check if the given TBGCheckData has a valid warp collision type
            static bool isValidWarpCol(const TBGCheckData *colTriangle);

            // Check if this link's triangle is referenced by the target ID of `other`
            bool isTargetOf(const TBGCheckData *other) const;

            // Check if this link's triangle is referencing `other` by its target ID
            bool isTargeting(const TBGCheckData *other) const;

            // Check if this link provides a valid home id for other links to reference
            bool isValidDest() const;

            // Check if this link provides a valid target id to reference other links with
            bool isValidSrc() const;

            f32 getMinTargetDistance() const;

            const TBGCheckData *getThisColTriangle() const { return mColTriangle; }
            u8 getTargetID() const { return mTargetID; }
            u8 getHomeID() const { return mHomeID; }
            SearchMode getSearchMode() const { return mSearchMode; }

            void setThisColTriangle(TBGCheckData *colTriangle) { mColTriangle = colTriangle; }
            void setTargetID(u8 id) { mTargetID = id; }
            void setHomeID(u8 id) { mHomeID = id; }
            void setSearchMode(SearchMode mode) { mSearchMode = mode; }

        private:
            const TBGCheckData *mColTriangle;  // 0x0000
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

            void addLink(const TBGCheckData *a, const TBGCheckData *b);
            void addLink(TCollisionLink &link);

            void removeLink(const TBGCheckData *a, const TBGCheckData *b);
            void removeLink(TCollisionLink *link);
            void removeLinkByIndex(u32 index);

            size_t size() const { return mUsedSize; }
            size_t maxsize() const { return mMaxSize; }

            TCollisionLink *getLinks() const { return mColList; }

            const TBGCheckData *resolveCollisionWarp(const TBGCheckData *colTriangle);
            const TBGCheckData *getNearestTarget(const TBGCheckData *colTriangle);

        private:
            size_t mUsedSize;
            size_t mMaxSize;
            TCollisionLink *mColList;
        };
    }  // namespace Collision
}  // namespace BetterSMS