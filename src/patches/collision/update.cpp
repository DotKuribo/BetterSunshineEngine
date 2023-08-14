#include <SMS/Map/Map.hxx>
#include <SMS/Map/MapCollisionData.hxx>
#include <SMS/Map/MapMakeList.hxx>
#include <SMS/Player/Mario.hxx>
#include <SMS/Strategic/Strategy.hxx>
#include <SMS/macros.h>

#include "memory.hxx"

#include "libs/profiler.hxx"
#include "module.hxx"
#include "p_settings.hxx"

constexpr float cellSize = 1024.0f;

static TBGCheckList *addGroundNode_(TBGCheckList *list, TBGCheckData *data) {
    TBGCheckData *col;

    do {
        if (!list->mNextTriangle)
            return list;

        list = list->mNextTriangle;
        col  = list->mColTriangle;

        if (col->mMinHeight < data->mMinHeight)
            return list;

        if (col->mMinHeight == data->mMinHeight)
            return list;

        if (col->mMaxHeight < data->mMaxHeight)
            return list;
    } while (true);
}

static TBGCheckList *addWallNode_(TBGCheckList *list, TBGCheckData *data) {
    TBGCheckData *col;

    do {
        if (!list->mNextTriangle)
            return list;

        list = list->mNextTriangle;
        col  = list->mColTriangle;

        if (col->mMaxHeight < data->mMaxHeight)
            return list;

        if (col->mMaxHeight == data->mMaxHeight)
            return list;

        if (col->mMinHeight < data->mMinHeight)
            return list;
    } while (true);
}

static TBGCheckList *addRoofNode_(TBGCheckList *list, TBGCheckData *data) {
    TBGCheckData *col;

    do {
        if (!list->mNextTriangle)
            return list;

        list = list->mNextTriangle;
        col  = list->mColTriangle;

        if (data->mMaxHeight < col->mMaxHeight)
            return list;

        if (col->mMaxHeight == data->mMaxHeight)
            return list;

        if (data->mMinHeight < col->mMinHeight)
            return list;
    } while (true);
}

static void addAfterPreNode_(int cellx, int cellz, TBGCheckList *addlist, TBGCheckList *newlist,
                             int type) {
    newlist->mNextTriangle = addlist->mNextTriangle;
    if (type == COLLISION_WARP) {
        TBGCheckListWarp *warp = static_cast<TBGCheckListWarp *>(newlist);
        warp->mCellX           = cellx;
        warp->mCellZ           = cellz;
    }
    if (addlist->mNextTriangle) {
        addlist->mNextTriangle->setPreNode(newlist);
    }
    addlist->mNextTriangle = newlist;
}

// Skip wall padding (new collision)
// SMS_WRITE_32(0x8019295C, 0x60000000);
// SMS_WRITE_32(0x80192960, 0x60000000);
// SMS_WRITE_32(0x80192964, 0x60000000);
// SMS_WRITE_32(0x80192968, 0x60000000);
// SMS_WRITE_32(0x8019296C, 0x60000000);
// SMS_WRITE_32(0x80192970, 0x60000000);
// SMS_WRITE_32(0x80192974, 0x60000000);

// Skip safety checks
// SMS_WRITE_32(0x80192994, 0x60000000);
// SMS_WRITE_32(0x80192998, 0x60000000);
// SMS_WRITE_32(0x8019299C, 0x60000000);
// SMS_WRITE_32(0x801929A0, 0x60000000);
// SMS_WRITE_32(0x801929A4, 0x60000000);
//
// SMS_WRITE_32(0x801929C8, 0x60000000);
// SMS_WRITE_32(0x801929CC, 0x60000000);
// SMS_WRITE_32(0x801929D0, 0x60000000);
// SMS_WRITE_32(0x801929D4, 0x60000000);
// SMS_WRITE_32(0x801929D8, 0x60000000);
// SMS_WRITE_32(0x801929DC, 0x60000000);
//
// SMS_WRITE_32(0x80192A00, 0x60000000);
// SMS_WRITE_32(0x80192A04, 0x60000000);
// SMS_WRITE_32(0x80192A08, 0x60000000);
// SMS_WRITE_32(0x80192A0C, 0x60000000);
// SMS_WRITE_32(0x80192A10, 0x60000000);
//
// SMS_WRITE_32(0x80192A34, 0x60000000);
// SMS_WRITE_32(0x80192A38, 0x60000000);
// SMS_WRITE_32(0x80192A3C, 0x60000000);
// SMS_WRITE_32(0x80192A40, 0x60000000);
// SMS_WRITE_32(0x80192A44, 0x60000000);
// SMS_WRITE_32(0x80192A48, 0x60000000);

void addCheckDataToGridAll(TMapCollisionData *collision, TBGCheckData *data, s32 kind) {
    s32 type = data->getPlaneType();
    int xmin, zmin, xmax, zmax;

    if (!gpMapCollisionData->getGridArea(data, type, &xmin, &zmin, &xmax, &zmax))
        return;

    f32 areaX = collision->mAreaSizeX;
    f32 areaZ = collision->mAreaSizeZ;
    for (int cellz = zmin; cellz <= zmax; cellz += 1) {
        float mapz = cellz * cellSize;
        for (int cellx = xmin; cellx <= xmax; cellx += 1) {
            float mapx = cellx * cellSize;

            TBGCheckList *list;
            TBGCheckList *addNode;
            if (kind == COLLISION_MOVE) {
                list = &gpMapCollisionData
                            ->mMoveCollisionRoot[cellx + (cellz * collision->mBlockXCount)]
                            .mCheckList[type];
                switch (type) {
                case TBGCheckListRoot::GROUND:
                    addNode = addGroundNode_(list, data);
                    break;
                case TBGCheckListRoot::ROOF:
                    addNode = addRoofNode_(list, data);
                    break;
                case TBGCheckListRoot::WALL:
                    addNode = addWallNode_(list, data);
                    break;
                }
                TBGCheckList *newlist = gpMapCollisionData->allocCheckList(COLLISION_MOVE, 1);
                newlist->mColTriangle = data;
                addAfterPreNode_(cellx, cellz, addNode, newlist, COLLISION_MOVE);
            } else {
                f32 baseX = mapx - areaX;
                f32 baseZ = mapz - areaZ;
                if (!gpMapCollisionData->polygonIsInGrid(baseX, baseZ, baseX + cellSize + 80.0f,
                                                         baseZ + cellSize + 80.0f, data)) {
                    continue;
                }

                list = &gpMapCollisionData
                            ->mStaticCollisionRoot[cellx + (cellz * collision->mBlockXCount)]
                            .mCheckList[type];
                switch (type) {
                case TBGCheckListRoot::GROUND:
                    addNode = addGroundNode_(list, data);
                    break;
                case TBGCheckListRoot::ROOF:
                    addNode = addRoofNode_(list, data);
                    break;
                case TBGCheckListRoot::WALL:
                    addNode = addWallNode_(list, data);
                    break;
                }
                TBGCheckList *newlist = gpMapCollisionData->allocCheckList(kind, 1);
                newlist->mColTriangle = data;
                addAfterPreNode_(cellx, cellz, addNode, newlist, type);
            }
        }
    }
}
// SMS_PATCH_BL(0x8018E210, addCheckDataToGridAll);
// SMS_PATCH_BL(0x80191568, addCheckDataToGridAll);
// SMS_PATCH_BL(0x801917BC, addCheckDataToGridAll);
// SMS_PATCH_BL(0x80191A58, addCheckDataToGridAll);
// SMS_PATCH_BL(0x80191AAC, addCheckDataToGridAll);

bool isActiveFromGroup(TIdxGroupObj *group, THitActor *target, f32 range) {
    for (auto &obj : group->mViewObjList) {
        auto *actor = reinterpret_cast<THitActor *>(obj);
        auto a_xz   = actor->mTranslation;
        a_xz.y      = 0;
        auto t_xz   = target->mTranslation;
        t_xz.y      = 0;
        if (PSVECSquareDistance(a_xz, t_xz) < range) {
            return true;
        }
    }
    return false;
}

bool isActiveFromObjectGroup(THitActor *target, f32 range) {
    for (auto &obj : gpStrategy->mObjectGroup->mViewObjList) {
        auto *actor = reinterpret_cast<TLiveActor *>(obj);
        if (actor->mCollisionManager)
            continue;
        auto a_xz = actor->mTranslation;
        a_xz.y    = 0;
        auto t_xz = target->mTranslation;
        t_xz.y    = 0;
        if (PSVECSquareDistance(a_xz, t_xz) < range) {
            return true;
        }
    }
    return false;
}

bool isActive(TLiveActor *target) {
    f32 scale = target->mScale.x;
    scale     = Max(scale, target->mScale.y);
    scale     = Max(scale, target->mScale.z);

    f32 range = 10000 * 10000 * scale;

    return isActiveFromGroup(gpStrategy->mPlayerGroup, target, range);

    /*if (isActiveFromGroup(gpStrategy->mPlayerGroup, target, range))
        return true;

    if (isActiveFromGroup(gpStrategy->mEnemyGroup, target, range))
        return true;

    if (isActiveFromGroup(gpStrategy->mNPCGroup, target, range))
        return true;

    if (isActiveFromGroup(gpStrategy->mBossGroup, target, range))
        return true;

    if (isActiveFromObjectGroup(target, range))
        return true;

    return false;*/
}

static void filterMoveUpdates(TLiveActor *target) {
    if (target->hasMapCollision() && isActive(target)) {
        target->setGroundCollision();
    }
}
// SMS_PATCH_BL(SMS_PORT_REGION(0x80218260, 0, 0, 0), filterMoveUpdates);
// SMS_WRITE_32(SMS_PORT_REGION(0x80218264, 0, 0, 0), 0x60000000);
// SMS_WRITE_32(SMS_PORT_REGION(0x80218268, 0, 0, 0), 0x60000000);
// SMS_WRITE_32(SMS_PORT_REGION(0x8021826C, 0, 0, 0), 0x60000000);
// SMS_PATCH_BL(SMS_PORT_REGION(0x801AFBF8, 0, 0, 0), filterMoveUpdates);
// SMS_WRITE_32(SMS_PORT_REGION(0x801AFBFC, 0, 0, 0), 0x60000000);
// SMS_WRITE_32(SMS_PORT_REGION(0x801AFC00, 0, 0, 0), 0x60000000);
// SMS_WRITE_32(SMS_PORT_REGION(0x801AFC04, 0, 0, 0), 0x60000000);

static TProfiler profiler("MoveReset");
static void profileMoveReset(TMapCollisionData *data) {
    profiler.start();
    data->initMoveCollision();
    profiler.stop();
    profiler.report();
}
// SMS_PATCH_BL(0x80189758, profileMoveReset);