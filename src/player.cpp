#include <Dolphin/GX.h>
#include <Dolphin/OS.h>

#include <JSystem/J2D/J2DPrint.hxx>
#include <JSystem/JKernel/JKRFileLoader.hxx>
#include <SMS/Enemy/EffectObjBase.hxx>
#include <SMS/GC2D/ConsoleStr.hxx>
#include <SMS/MSound/MSoundSESystem.hxx>
#include <SMS/Manager/MarioParticleManager.hxx>
#include <SMS/Manager/ModelWaterManager.hxx>
#include <SMS/MapObj/MapObjNormalLift.hxx>
#include <SMS/MapObj/MapObjTree.hxx>
#include <SMS/Player/Mario.hxx>
#include <SMS/Player/MarioDraw.hxx>
#include <SMS/Strategic/HitActor.hxx>
#include <SMS/Strategic/LiveActor.hxx>
#include <SMS/System/Application.hxx>
#include <SMS/System/CardManager.hxx>
#include <SMS/assert.h>
#include <SMS/macros.h>
#include <SMS/raw_fn.hxx>

#include "libs/constmath.hxx"
#include "libs/global_unordered_map.hxx"
#include "libs/profiler.hxx"
#include "libs/string.hxx"
#include "libs/triangle.hxx"

#include "libs/geometry.hxx"
#include "logging.hxx"
#include "module.hxx"
#include "p_settings.hxx"
#include "player.hxx"
#include "stage.hxx"

using namespace BetterSMS;

// Wrapper to fix void * problems with map.
struct MarioData {
    const char *mKey;
    void *mPtr;
};

struct MarioDataPair {
    TMario *mPlayer;
    TGlobalVector<MarioData> mData;
};

template <typename _I, typename _C> struct PhysicsMetaInfo {
    _I mID;
    _C mCallback;
};

static MarioDataPair sPlayerDatas[8];
static TGlobalVector<Player::InitCallback> sPlayerInitializers;
static TGlobalVector<Player::LoadAfterCallback> sPlayerLoadAfterCBs;
static TGlobalVector<Player::UpdateCallback> sPlayerUpdaters;
static TGlobalVector<PhysicsMetaInfo<u32, Player::MachineCallback>> sPlayerStateMachines;
static TGlobalVector<PhysicsMetaInfo<u16, Player::CollisionCallback>> sPlayerCollisionHandlers;

BETTER_SMS_FOR_EXPORT Player::TPlayerData *BetterSMS::Player::getData(TMario *player) {
    TPlayerData *data = nullptr;

    for (size_t i = 0; i < 8; ++i) {
        if (sPlayerDatas[i].mPlayer != player) {
            continue;
        }
        for (const MarioData &item : sPlayerDatas[i].mData) {
            if (strcmp(item.mKey, "__better_sms") == 0) {
                data = reinterpret_cast<Player::TPlayerData *>(item.mPtr);
                goto AfterSearch;
            }
        }
    }

AfterSearch:
    if (!data) {
        Console::debugLog(
            "Trying to access BetterSMS player data that is not registered! (No Data)\n");
    }

    return data;
}

BETTER_SMS_FOR_EXPORT void *BetterSMS::Player::getRegisteredData(TMario *player, const char *key) {
    TPlayerData *data = nullptr;

    for (size_t i = 0; i < 8; ++i) {
        if (sPlayerDatas[i].mPlayer != player) {
            continue;
        }
        for (const MarioData &item : sPlayerDatas[i].mData) {
            if (strcmp(item.mKey, key) == 0) {
                data = reinterpret_cast<Player::TPlayerData *>(item.mPtr);
                goto AfterSearch;
            }
        }
    }

AfterSearch:
    if (!data) {
        Console::debugLog("Trying to access player data (%s) that is not registered!\n", key);
    }

    return data;
}

// Register arbitrary module data for a player
BETTER_SMS_FOR_EXPORT bool BetterSMS::Player::registerData(TMario *player, const char *key,
                                                           void *data) {
    for (size_t i = 0; i < 8; ++i) {
        if (sPlayerDatas[i].mPlayer == player) {
            for (const MarioData &item : sPlayerDatas[i].mData) {
                if (strcmp(item.mKey, key) == 0) {
                    Console::debugLog("Player data (%s) already exists!\n", key);
                    return false;
                }
            }
            sPlayerDatas[i].mData.push_back({key, data});
        }
        if (sPlayerDatas[i].mPlayer == nullptr) {
            sPlayerDatas[i].mPlayer = player;
            sPlayerDatas[i].mData   = {};
            sPlayerDatas[i].mData.push_back({key, data});
            return true;
        }
    }
    return false;
}

BETTER_SMS_FOR_EXPORT void BetterSMS::Player::deregisterData(TMario *player, const char *key) {
    for (size_t i = 0; i < 8; ++i) {
        if (sPlayerDatas[i].mPlayer != player) {
            continue;
        }

        for (auto it = sPlayerDatas[i].mData.begin(); it != sPlayerDatas[i].mData.end(); ++it) {
            if (strcmp(it->mKey, key) == 0) {
                sPlayerDatas[i].mData.erase(it);
                return;
            }
        }
    }
}

constexpr size_t MarioAnimeDataSize = 336;
constexpr size_t MarioAnimeInfoSize = 199;

TMarioAnimeData sPlayerAnimeDatas[MAX_PLAYER_ANIMATIONS] = {};
size_t sPlayerAnimeDatasSize                             = MarioAnimeDataSize;

struct InternalAnimInfo {
    int m_unk;
    const char *m_name;
};
InternalAnimInfo sPlayerAnimeInfos[MAX_PLAYER_ANIMATIONS] = {};
size_t sPlayerAnimeInfosSize                              = MarioAnimeInfoSize;

BETTER_SMS_FOR_EXPORT bool BetterSMS::Player::isAnimationValid(u16 anm_idx) {
    if (anm_idx >= sPlayerAnimeDatasSize)
        return false;

    return sPlayerAnimeDatas[anm_idx].mAnimID < sPlayerAnimeInfosSize;
}

BETTER_SMS_FOR_EXPORT bool BetterSMS::Player::setAnimationData(u16 anm_idx, bool *fludd_use,
                                                               bool *jiggle_phys, u8 *tex_anm_id,
                                                               u8 *hand) {
    if (!isAnimationValid(anm_idx))
        return false;

    if (fludd_use) {
        if (*fludd_use) {
            sPlayerAnimeDatas[anm_idx].mAnimFluddID = 68;
            sPlayerAnimeDatas[anm_idx].mBodyFlags |= TMarioAnimeData::FLUDD_GRIP;
        } else {
            sPlayerAnimeDatas[anm_idx].mAnimFluddID = sPlayerAnimeInfosSize;
            sPlayerAnimeDatas[anm_idx].mBodyFlags &= ~TMarioAnimeData::FLUDD_GRIP;
        }
    }
    if (jiggle_phys) {
        if (*jiggle_phys) {
            sPlayerAnimeDatas[anm_idx].mBodyFlags |= TMarioAnimeData::JIGGLE_PHYS;
        } else {
            sPlayerAnimeDatas[anm_idx].mBodyFlags &= ~TMarioAnimeData::JIGGLE_PHYS;
        }
    }
    if (tex_anm_id)
        sPlayerAnimeDatas[anm_idx].mAnmTexPattern = *tex_anm_id;
    if (hand)
        sPlayerAnimeDatas[anm_idx].mMarioHand = *hand;

    return true;
}

BETTER_SMS_FOR_EXPORT u16 BetterSMS::Player::addAnimationData(const char *anm_name, bool fludd_use,
                                                              bool jiggle_phys, u8 tex_anm_id,
                                                              u8 hand) {
    // Update the sentinel ref
    for (size_t i = 0; i < sPlayerAnimeDatasSize; ++i) {
        if (sPlayerAnimeDatas[i].mAnimID >= sPlayerAnimeInfosSize) {
            sPlayerAnimeDatas[i].mAnimID += 1;
        }
        if (sPlayerAnimeDatas[i].mAnimFluddID >= sPlayerAnimeInfosSize) {
            sPlayerAnimeDatas[i].mAnimFluddID += 1;
        }
    }

    TMarioAnimeData new_data = {static_cast<u16>(sPlayerAnimeInfosSize),
                                static_cast<u16>(fludd_use ? 68 : sPlayerAnimeInfosSize + 1),
                                tex_anm_id,
                                hand,
                                static_cast<u8>(jiggle_phys ? 6 : 4),
                                0x16};

    sPlayerAnimeDatas[sPlayerAnimeDatasSize++] = new_data;

    InternalAnimInfo new_info                  = {1, anm_name};
    sPlayerAnimeInfos[sPlayerAnimeInfosSize++] = new_info;

    return sPlayerAnimeDatasSize - 1;
}

BETTER_SMS_FOR_EXPORT bool BetterSMS::Player::addAnimationDataEx(u16 anm_idx, const char *anm_name,
                                                                 bool fludd_use, bool jiggle_phys,
                                                                 u8 tex_anm_id, u8 hand) {
    if (anm_idx >= sPlayerAnimeDatasSize) {
        if (anm_idx >= MAX_PLAYER_ANIMATIONS)
            return false;
        sPlayerAnimeDatasSize = anm_idx + 1;
    }

    u16 info_idx = sPlayerAnimeDatas[anm_idx].mAnimID;

    if (info_idx >= sPlayerAnimeInfosSize) {
        if (info_idx >= MAX_PLAYER_ANIMATIONS) {
            // Update the sentinel ref
            for (size_t i = 0; i < sPlayerAnimeDatasSize; ++i) {
                if (sPlayerAnimeDatas[i].mAnimID >= sPlayerAnimeInfosSize) {
                    sPlayerAnimeDatas[i].mAnimID += 1;
                }
                if (sPlayerAnimeDatas[i].mAnimFluddID >= sPlayerAnimeInfosSize) {
                    sPlayerAnimeDatas[i].mAnimFluddID += 1;
                }
            }
            info_idx = sPlayerAnimeInfosSize++;
        } else {
            // Update the sentinel ref
            u16 end_idx = info_idx + 1;
            for (size_t i = 0; i < sPlayerAnimeDatasSize; ++i) {
                if (sPlayerAnimeDatas[i].mAnimID >= sPlayerAnimeInfosSize) {
                    sPlayerAnimeDatas[i].mAnimID = end_idx;
                }
                if (sPlayerAnimeDatas[i].mAnimFluddID >= sPlayerAnimeInfosSize) {
                    sPlayerAnimeDatas[i].mAnimFluddID = end_idx;
                }
            }
            sPlayerAnimeInfosSize = end_idx;
        }
    }

    TMarioAnimeData new_data   = {info_idx,
                                  static_cast<u16>(fludd_use ? 68 : sPlayerAnimeInfosSize),
                                  tex_anm_id,
                                  hand,
                                  static_cast<u8>(jiggle_phys ? 6 : 4),
                                  0x16};
    sPlayerAnimeDatas[anm_idx] = new_data;

    InternalAnimInfo new_info   = {1, anm_name};
    sPlayerAnimeInfos[info_idx] = new_info;

    return true;
}

BETTER_SMS_FOR_EXPORT bool BetterSMS::Player::addInitCallback(InitCallback process) {
    sPlayerInitializers.push_back(process);
    return true;
}

BETTER_SMS_FOR_EXPORT bool BetterSMS::Player::addLoadAfterCallback(LoadAfterCallback process) {
    sPlayerLoadAfterCBs.push_back(process);
    return true;
}

BETTER_SMS_FOR_EXPORT bool BetterSMS::Player::addUpdateCallback(UpdateCallback process) {
    sPlayerUpdaters.push_back(process);
    return true;
}

BETTER_SMS_FOR_EXPORT bool BetterSMS::Player::registerStateMachine(u32 state,
                                                                   MachineCallback process) {
    if ((state & 0x1C0) != 0x1C0) {
        Console::log("[WARNING] State machine 0x%X isn't ORd with 0x1C0 (Prevents "
                     "engine collisions)!\n",
                     state);
    }
    for (auto &item : sPlayerStateMachines) {
        if (item.mID == state) {
            Console::log("[WARNING] State machine 0x%X already exists!\n", state);
            return false;
        }
    }
    sPlayerStateMachines.push_back({state, process});
    return true;
}

BETTER_SMS_FOR_EXPORT bool BetterSMS::Player::registerCollisionHandler(u16 colType,
                                                                       CollisionCallback process) {
    if ((colType & 0xC000) != 0) {
        Console::log("[WARNING] Collision type 0x%X has camera clip and shadow flags set "
                     "(0x4000 || 0x8000)! This may cause unwanted behaviour!\n",
                     colType);
    }
    for (auto &item : sPlayerCollisionHandlers) {
        if (item.mID == colType) {
            Console::log("[WARNING] Collision type 0x%X already exists!\n", colType);
            return false;
        }
    }
    sPlayerCollisionHandlers.push_back({colType, process});
    return true;
}

static bool sIsWiping = false;

static void warpPlayerToPoint(TMario *player, const TVec3f &point) {
    if (!player)
        return;

    auto playerData = Player::getData(player);

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

BETTER_SMS_FOR_EXPORT void BetterSMS::Player::warpToCollisionFace(TMario *player,
                                                                  const TBGCheckData *colTriangle,
                                                                  bool isFluid) {
    constexpr s32 DisableMovementTime = 80;
    constexpr s32 TeleportTime        = 140;
    constexpr s32 EnableMovementTime  = 60;
    constexpr f32 WipeKindInDelay     = 1.0f;

    if (!player || !SMS_IsMarioTouchGround4cm__Fv())
        return;

    auto playerData = Player::getData(player);

    TVec3f triCenter;
    TVectorTriangle vectorTri(colTriangle->mVertices[0], colTriangle->mVertices[1],
                              colTriangle->mVertices[2]);
    vectorTri.center(triCenter);

    TVec3f triFluidCenter = triCenter;
    triFluidCenter.y += 300.0f;

#define EXPAND_WARP_SET(base)                                                                      \
    (base) : case ((base) + 10):                                                                   \
    case ((base) + 20):                                                                            \
    case ((base) + 30)
#define EXPAND_WARP_CATEGORY(base)                                                                 \
    (base) : case ((base) + 1):                                                                    \
    case ((base) + 2):                                                                             \
    case ((base) + 3):                                                                             \
    case ((base) + 4)

    const u16 type = colTriangle->mType & 0x7FFF;
    switch (type) {
    case EXPAND_WARP_SET(16040):
    case EXPAND_WARP_SET(17040): {
        warpToPoint(player, triCenter, WarpKind::SPARKLES, TeleportTime,
                    false);  // Sparkles and then warp
        break;
    }
    case EXPAND_WARP_SET(16041):
    case EXPAND_WARP_SET(17041): {
        warpToPoint(player, triCenter, WarpKind::INSTANT, 0,
                    true);  // Portal momentum warp (locking)
        break;
    }
    case EXPAND_WARP_SET(16042):
    case EXPAND_WARP_SET(17042): {
        warpToPoint(player, triCenter, WarpKind::INSTANT, 0, true);  // Portal momentum warp (free)
        break;
    }
    case EXPAND_WARP_SET(16043):
    case EXPAND_WARP_SET(17043):
        warpToPoint(player, triCenter, WarpKind::WIPE, TeleportTime, false);  // Pipe warp
        break;
    case EXPAND_WARP_SET(16044):
    case EXPAND_WARP_SET(17044):
        warpToPoint(player, triCenter, WarpKind::INSTANT, 0, false);  // Portal warp
        break;
    case EXPAND_WARP_SET(16045):
    case EXPAND_WARP_SET(17045):
        warpToPoint(player, triCenter, WarpKind::SPARKLES, 0, true);  // Sparkled momentum warp
        break;
    case EXPAND_WARP_SET(16046):
    case EXPAND_WARP_SET(17046):
        warpToPoint(player, triCenter, WarpKind::WIPE, 0, true);  // Pipe momentum warp
        break;
    }
#undef EXPAND_WARP_SET
#undef EXPAND_WARP_CATEGORY
}

BETTER_SMS_FOR_EXPORT void BetterSMS::Player::warpToPoint(TMario *player, const TVec3f &destPoint,
                                                          WarpKind kind, s32 framesToWarp,
                                                          bool isWarpFluid) {
    if (!player)
        return;

    auto playerData              = Player::getData(player);
    playerData->mWarpDestination = destPoint;
    playerData->mWarpKind        = kind;
    playerData->mIsWarpFluid     = isWarpFluid;
    playerData->mWarpTimerMax    = framesToWarp;

    if (playerData->mWarpTimer == -1) {
        playerData->mIsWarpActive = true;
    }
}

static f32 normalizeAngle(f32 angle) {
    angle = fmodf__3stdFff(angleToRadians(angle) + M_PI, 2 * M_PI);
    if (angle < 0)
        angle += 2 * M_PI;
    return radiansToAngle(angle - M_PI);
}

static f32 getAngleDelta(f32 a, f32 b) {
    f32 delta = normalizeAngle(a) - normalizeAngle(b);
    if (delta > 180.0f) {
        delta -= 360.0f;
    } else if (delta < -180.0f) {
        delta += 360.0f;
    }
    return delta;
}

BETTER_SMS_FOR_EXPORT void BetterSMS::Player::rotateRelativeToCamera(TMario *player,
                                                                     CPolarSubCamera *camera,
                                                                     Vec2 dir, f32 lerp_) {
    f32 accelAngle = radiansToAngle(atan2f(-dir.x, dir.y));
    f32 camAngle   = convertAngleS16ToFloat(camera->mHorizontalAngle);
    f32 delta      = getAngleDelta(convertAngleS16ToFloat(player->mAngle.y), camAngle + accelAngle);

    player->mAngle.y += convertAngleFloatToS16(delta) * lerp_;
}

#pragma region Patches

static u32 patchYStorage() {
    TMario *player;
    SMS_FROM_GPR(31, player);

    if (BetterSMS::areExploitsPatched()) {
        if (player->mState != static_cast<u32>(TMario::STATE_IDLE))
            player->mSpeed.y = 0.0f;
    }

    return 0;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x802571F0, 0x8024EF7C, 0, 0), patchYStorage);

static void patchRideMovementUpWarp(Mtx out, Vec *ride, Vec *pos) {
    TMario *player;
    SMS_FROM_GPR(30, player);

    if (!(player->mState & static_cast<u32>(TMario::STATE_AIRBORN)) ||
        !BetterSMS::areExploitsPatched()) {
        PSMTXMultVec(out, ride, pos);
    }
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80250514, 0x802482A0, 0, 0), patchRideMovementUpWarp);

#pragma endregion

#pragma region TPlayerDataImplementation

Player::TPlayerData::TPlayerData(TMario *player, CPolarSubCamera *camera, bool isMario)
    : mPlayer(player), mCamera(camera), mIsEMario(!isMario), mPlayerID(0), mCanSprayFludd(true),
      mCurJump(0), mIsClimbTired(false), mLastQuarterFrameState(player->mState),
      mPrevCollisionFloorType(0), mCollisionTimer(0), mClimbTiredTimer(0),
      mSlideSpeedMultiplier(1.0f), mMaxAddVelocity(1000.0f), mYoshiWaterSpeed(0.0f, 0.0f, 0.0f),
      mDefaultAttrs(player), mWarpTimer(-1), mWarpState(0xFF) {

    mParams = new TPlayerParams();

    mFluddHistory.mHadFludd     = false;
    mFluddHistory.mMainNozzle   = TWaterGun::Spray;
    mFluddHistory.mSecondNozzle = TWaterGun::Hover;
    mFluddHistory.mWaterLevel   = 0;

    mDefaultDirtyParams = player->mDirtyParams;

    mCollisionFlags.mIsAirborn       = false;
    mCollisionFlags.mIsCollisionUsed = false;
    mCollisionFlags.mIsDisableInput  = false;
    mCollisionFlags.mIsFaceUsed      = false;
    mCollisionFlags.mIsSpinBounce    = false;

    if (isMario) {
        loadPrm("/better_sms.prm");
    }

    mCanUseFludd = mParams->mCanUseFludd.get();

    if (mParams->mPlayerHasGlasses.get() && player->mCap)
        reinterpret_cast<u16 *>(player->mCap)[2] |= 0b100;
}

bool Player::TPlayerData::loadPrm(const char *prm = "/better_sms.prm") {
    JKRArchive *archive = JKRFileLoader::getVolume("mario");
    SMS_DEBUG_ASSERT(archive, "Mario archive could not be located!");

    void *resource = archive->getResource(prm);
    if (resource) {
        JSUMemoryInputStream stream(resource, archive->getResSize(resource));
        mParams->load(stream);
        return true;
    }
    return false;
}

bool Player::TPlayerData::loadPrm(JSUMemoryInputStream &stream) {
    mParams->load(stream);
    return true;
}

void Player::TPlayerData::ParamHistory::applyHistoryTo(TMario *player) {
    SMS_ASSERT(player != nullptr, "Can't apply param history to a nullptr");
    player->mDeParams                       = mDeParams;
    player->mBodyAngleFreeParams            = mBodyAngleFreeParams;
    player->mBodyAngleWaterGunParams        = mBodyAngleWaterGunParams;
    player->mPunchFenceParams               = mPunchFenceParams;
    player->mKickRoofParams                 = mKickRoofParams;
    player->mJumpParams                     = mJumpParams;
    player->mRunParams                      = mRunParams;
    player->mSwimParams                     = mSwimParams;
    player->mHangFenceParams                = mHangFenceParams;
    player->mHangRoofParams                 = mHangRoofParams;
    player->mWireParams                     = mWireParams;
    player->mPullBGBeakParams               = mPullBGBeakParams;
    player->mPullBGTentacleParams           = mPullBGTentacleParams;
    player->mPullBGFireWanWanBossTailParams = mPullBGFireWanWanBossTailParams;
    player->mPullFireWanWanTailParams       = mPullFireWanWanTailParams;
    player->mHoverParams                    = mHoverParams;
    player->mDivingParams                   = mDivingParams;
    player->mYoshiParams                    = mYoshiParams;
    player->mWaterEffectParams              = mWaterEffectParams;
    player->mControllerParams               = mControllerParams;
    player->mGraffitoParams                 = mGraffitoParams;
    player->mDirtyParams                    = mDirtyParams;
    player->mMotorParams                    = mMotorParams;
    player->mParticleParams                 = mParticleParams;
    player->mEffectParams                   = mEffectParams;
    player->mSlipNormalParams               = mSlipNormalParams;
    player->mSlipOilParams                  = mSlipOilParams;
    player->mSlipAllParams                  = mSlipAllParams;
    player->mSlipAllSliderParams            = mSlipAllSliderParams;
    player->mSlip45Params                   = mSlip45Params;
    player->mSlipWaterSlopeParams           = mSlipWaterSlopeParams;
    player->mSlipWaterGroundParams          = mSlipWaterGroundParams;
    player->mSlipYoshiParams                = mSlipYoshiParams;
    player->mUpperBodyParams                = mUpperBodyParams;
}

Player::TPlayerData::ParamHistory::ParamHistory()
    : mDeParams(), mBodyAngleFreeParams("/Mario/BodyAngleFree.prm"),
      mBodyAngleWaterGunParams("/Mario/BodyAngleWaterGun.prm"),
      mPunchFenceParams("/Mario/AttackFencePunch.prm"),
      mKickRoofParams("/Mario/AttackKickRoof.prm"), mJumpParams(), mRunParams(), mSwimParams(),
      mHangFenceParams(), mHangRoofParams(), mWireParams(),
      mPullBGBeakParams("/Mario/PullParamBGBeak.prm"),
      mPullBGTentacleParams("/Mario/PullParamBGTentacle.prm"),
      mPullBGFireWanWanBossTailParams("/Mario/PullParamBGFireWanWanBossTail.prm"),
      mPullFireWanWanTailParams("/Mario/PullParamFireWanWanTail.prm"), mHoverParams(),
      mDivingParams(), mYoshiParams(), mWaterEffectParams(), mControllerParams(), mGraffitoParams(),
      mDirtyParams(), mMotorParams(), mParticleParams(), mEffectParams(),
      mSlipNormalParams("/Mario/SlipParamNormal.prm"), mSlipOilParams("/Mario/SlipParamOil.prm"),
      mSlipAllParams("/Mario/SlipParamAll.prm"),
      mSlipAllSliderParams("/Mario/SlipParamAll_Slider.prm"),
      mSlip45Params("/Mario/SlipParam45.prm"),
      mSlipWaterSlopeParams("/Mario/SlipParamWaterSlope.prm"),
      mSlipWaterGroundParams("/Mario/SlipParamWaterGround.prm"),
      mSlipYoshiParams("/Mario/SlipParamYoshi.prm"), mUpperBodyParams() {}

Player::TPlayerData::ParamHistory::ParamHistory(TMario *player) : ParamHistory() {
    recordFrom(player);
}

void Player::TPlayerData::ParamHistory::recordFrom(TMario *player) {
    SMS_ASSERT(player != nullptr, "Can't record param history from a nullptr");
    mDeParams                       = player->mDeParams;
    mBodyAngleFreeParams            = player->mBodyAngleFreeParams;
    mBodyAngleWaterGunParams        = player->mBodyAngleWaterGunParams;
    mPunchFenceParams               = player->mPunchFenceParams;
    mKickRoofParams                 = player->mKickRoofParams;
    mJumpParams                     = player->mJumpParams;
    mRunParams                      = player->mRunParams;
    mSwimParams                     = player->mSwimParams;
    mHangFenceParams                = player->mHangFenceParams;
    mHangRoofParams                 = player->mHangRoofParams;
    mWireParams                     = player->mWireParams;
    mPullBGBeakParams               = player->mPullBGBeakParams;
    mPullBGTentacleParams           = player->mPullBGTentacleParams;
    mPullBGFireWanWanBossTailParams = player->mPullBGFireWanWanBossTailParams;
    mPullFireWanWanTailParams       = player->mPullFireWanWanTailParams;
    mHoverParams                    = player->mHoverParams;
    mDivingParams                   = player->mDivingParams;
    mYoshiParams                    = player->mYoshiParams;
    mWaterEffectParams              = player->mWaterEffectParams;
    mControllerParams               = player->mControllerParams;
    mGraffitoParams                 = player->mGraffitoParams;
    mDirtyParams                    = player->mDirtyParams;
    mMotorParams                    = player->mMotorParams;
    mParticleParams                 = player->mParticleParams;
    mEffectParams                   = player->mEffectParams;
    mSlipNormalParams               = player->mSlipNormalParams;
    mSlipOilParams                  = player->mSlipOilParams;
    mSlipAllParams                  = player->mSlipAllParams;
    mSlipAllSliderParams            = player->mSlipAllSliderParams;
    mSlip45Params                   = player->mSlip45Params;
    mSlipWaterSlopeParams           = player->mSlipWaterSlopeParams;
    mSlipWaterGroundParams          = player->mSlipWaterGroundParams;
    mSlipYoshiParams                = player->mSlipYoshiParams;
    mUpperBodyParams                = player->mUpperBodyParams;
}

void Player::TPlayerData::ParamHistory::reset() { *this = ParamHistory(); }

#pragma endregion

static void initFluddInLoadAfter(TWaterGun *fludd) {
    fludd->mNozzleList[4]->mEmitParams.mDamageLoss.set(250);
}
SMS_PATCH_B(SMS_PORT_REGION(0x8026A3B8, 0x80262144, 0, 0), initFluddInLoadAfter);

static void initFludd(TMario *player, Player::TPlayerData *playerData) {
    SMS_ASSERT(playerData, "Can't init fludd with non existant playerData!");
    Stage::TStageParams *config = Stage::getStageConfiguration();

    player->mFludd->mMario = player;

    if (!playerData->isMario())
        return;

    if (config->mFluddShouldColorWater.get())
        waterColor[0] = config->mFluddWaterColor.get();

    // FIXME: Default params are all wrong
    // player->mFludd->mCurrentNozzle = config->mFluddPrimary.get();
    // player->mFludd->mSecondNozzle  = config->mFluddSecondary.get();

    player->mFludd->mCurrentWater = player->mFludd->mNozzleList[(u8)player->mFludd->mCurrentNozzle]
                                        ->mEmitParams.mAmountMax.get();
}

// Extern to Player Init CB
BETTER_SMS_FOR_CALLBACK void initMario(TMario *player, bool isMario) {
    Stage::TStageParams *config = Stage::getStageConfiguration();

    Player::TPlayerData *params = new Player::TPlayerData(player, nullptr, isMario);
    Player::registerData(player, "__better_sms", params);

    bool isGlasses = false;

    if (config->isCustomConfig()) {
        params->setPlayerID(0);
        player->mHealth = config->mPlayerHealth.get();
        player->mDeParams.mHPMax.set(config->mPlayerMaxHealth.get());
        player->mJumpParams.mGravity.set(config->mGravityMultiplier.get());

        if (isMario) {
            player->mAttributes.mGainHelmet   = config->mPlayerHasHelmet.get();
            player->mAttributes.mHasFludd     = config->mPlayerHasFludd.get();
            player->mAttributes.mIsShineShirt = config->mPlayerHasShirt.get();
        }

        isGlasses = config->mPlayerHasGlasses.get();
    }

    if (isMario) {
        player->mAttributes.mGainHelmet = params->getParams()->mPlayerHasHelmet.get();
        player->mAttributes.mHasFludd &= params->getParams()->mCanUseFludd.get();
        player->mAttributes.mIsShineShirt = params->getParams()->mPlayerHasShirt.get();
        isGlasses                         = params->getParams()->mPlayerHasGlasses.get();

        initFludd(player, params);
    }

    if (isGlasses && player->mCap)
        reinterpret_cast<u16 *>(player->mCap)[2] |= 0b100;
}

BETTER_SMS_FOR_CALLBACK void resetPlayerDatas(TMarDirector *application) {
    for (size_t i = 0; i < 8; ++i) {
        sPlayerDatas[i].mPlayer = nullptr;
        sPlayerDatas[i].mData.clear();
    }
}

static TMario *playerInitHandler(TMario *player) {
    player->initValues();

    initMario(player, true);

    for (auto &item : sPlayerInitializers) {
        item(player, true);
    }

    return player;
}
SMS_WRITE_32(SMS_PORT_REGION(0x80276C90, 0x8026EA1C, 0, 0), 0x60000000);
SMS_PATCH_BL(SMS_PORT_REGION(0x80276C94, 0x8026EA20, 0, 0), playerInitHandler);

// 0x800397DC
static bool shadowMarioInitHandler() {
    TMario *player;
    SMS_ASM_BLOCK("lwz %0, 0x150 (31)" : "=r"(player));

    initMario(player, false);

    for (auto &item : sPlayerInitializers) {
        item(player, false);
    }

    return SMS_isMultiPlayerMap__Fv();
}
SMS_PATCH_BL(SMS_PORT_REGION(0x800397DC, 0x80039894, 0, 0), shadowMarioInitHandler);

static void playerLoadAfterHandler(TMario *player) {
    player->initMirrorModel();

    for (auto &item : sPlayerLoadAfterCBs) {
        item(player);
    }
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80276BB8, 0, 0, 0), playerLoadAfterHandler);

static void playerUpdateHandler(TMario *player, JDrama::TGraphics *graphics) {
    for (auto &item : sPlayerUpdaters) {
        item(player, true);
    }

    player->playerControl(graphics);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x8024D3A0, 0x80245134, 0, 0), playerUpdateHandler);  // Mario

static void shadowMarioUpdateHandler(TMario *player, JDrama::TGraphics *graphics) {
    for (auto &item : sPlayerUpdaters) {
        item(player, false);
    }

    player->playerControl(graphics);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x8003F8E8, 0x8003F740, 0, 0), shadowMarioUpdateHandler);  // EMario

static bool stateMachineHandler(TMario *player) {
    auto currentState = player->mState;

    bool shouldProgressState = true;
    for (auto &item : sPlayerStateMachines) {
        if (item.mID == currentState) {
            shouldProgressState = item.mCallback(player);
            break;
        }
    }

    return shouldProgressState;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x802500B8, 0, 0, 0), stateMachineHandler);

static u32 collisionHandler(TMario *player) {
    auto *playerData = Player::getData(player);

    if ((player->mState != static_cast<u32>(TMario::STATE_JUMPSPINR) &&
         player->mState != static_cast<u32>(TMario::STATE_JUMPSPINL)))
        playerData->mCollisionFlags.mIsSpinBounce = false;

    const u16 colType     = player->mFloorTriangle->mType & 0xFFF;
    const u16 prevColType = playerData->mPrevCollisionFloorType & 0xFFF;

    u32 marioFlags = 0;
    if ((player->mState & TMario::STATE_AIRBORN)) {
        marioFlags |= Player::InteractionFlags::AIRBORN;
        if (!(playerData->mLastQuarterFrameState & TMario::STATE_AIRBORN))
            marioFlags |= Player::InteractionFlags::ON_DETACH;
        if (SMS_IsMarioTouchGround4cm__Fv())
            marioFlags |= Player::InteractionFlags::ON_4CM_CONTACT;
    } else {
        marioFlags |= Player::InteractionFlags::GROUNDED;
        if ((playerData->mLastQuarterFrameState & TMario::STATE_AIRBORN))
            marioFlags |= Player::InteractionFlags::ON_CONTACT;
    }

    if (colType != prevColType) {
        // Here we keep the loops apart as to force exit callbacks before enter callbacks
        for (auto &item : sPlayerCollisionHandlers) {
            if (item.mID == prevColType)
                item.mCallback(player, const_cast<TBGCheckData *>(playerData->mPrevCollisionFloor),
                               marioFlags | Player::InteractionFlags::ON_EXIT);
        }
        for (auto &item : sPlayerCollisionHandlers) {
            if (item.mID == colType)
                item.mCallback(player, const_cast<TBGCheckData *>(player->mFloorTriangle),
                               marioFlags | Player::InteractionFlags::ON_ENTER);
        }
        playerData->mPrevCollisionFloorType     = colType;
        playerData->mPrevCollisionFloor         = player->mFloorTriangle;
        playerData->mCollisionFlags.mIsFaceUsed = false;
        playerData->mCollisionTimer             = 0;
    } else if (colType >= 3000) {  // Custom collision is routinely updated
        for (auto &item : sPlayerCollisionHandlers) {
            if (item.mID == colType)
                item.mCallback(player, const_cast<TBGCheckData *>(player->mFloorTriangle),
                               marioFlags);
        }
    }

    /*TSMSFader *fader = gpApplication.mFader;
    if (fader->mFadeStatus != TSMSFader::FADE_OFF) {
        playerData->mCollisionFlags.mIsDisableInput = true;
        player->mController->mState.mReadInput      = false;
    } else {
        playerData->mCollisionFlags.mIsDisableInput = false;
        player->mController->mState.mReadInput      = true;
    }*/

    playerData->mLastQuarterFrameState = player->mState;
    return player->mState;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x8025059C, 0x80248328, 0, 0), collisionHandler);
SMS_WRITE_32(SMS_PORT_REGION(0x802505A0, 0x8024832C, 0, 0), 0x546004E7);

extern InternalAnimInfo marioAnimeFiles[199];

BETTER_SMS_FOR_CALLBACK void initExtendedPlayerAnims() {
    for (size_t i = 0; i < MarioAnimeDataSize; ++i) {
        sPlayerAnimeDatas[i] = gMarioAnimeData[i];
        if (sPlayerAnimeDatas[i].mAnimID == 200) {
            sPlayerAnimeDatas[i].mAnimID = 199;
        }
        if (sPlayerAnimeDatas[i].mAnimFluddID == 200) {
            sPlayerAnimeDatas[i].mAnimFluddID = 199;
        }
    }

    for (size_t i = 0; i < MarioAnimeInfoSize; ++i) {
        sPlayerAnimeInfos[i] = marioAnimeFiles[i];
    }
}

SMS_WRITE_32(SMS_PORT_REGION(0x80246AE4, 0, 0, 0), 0x38600800);
SMS_WRITE_32(SMS_PORT_REGION(0x80246AF0, 0, 0, 0), 0x38600800);

static SMS_ASM_FUNC void getExtendedPlayerAnimData() {
    SMS_ASM_BLOCK("lis       28, sPlayerAnimeDatas@h     \n\t"
                  "ori       28, 28, sPlayerAnimeDatas@l \n\t"
                  "add       28, 28, 30                  \n\t"
                  "lhz       6, 0 (28)                   \n\t"
                  "blr                                   \n\t");
}
SMS_PATCH_BL(SMS_PORT_REGION(0x8024786C, 0, 0, 0), getExtendedPlayerAnimData);

static SMS_ASM_FUNC void getExtendedPlayerAnimInfo() {
    SMS_ASM_BLOCK("lis       25, sPlayerAnimeInfos@h     \n\t"
                  "ori       25, 25, sPlayerAnimeInfos@l \n\t"
                  "blr                                   \n\t");
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80246B00, 0, 0, 0), getExtendedPlayerAnimInfo);

static SMS_ASM_FUNC void getExtendedPlayerAnimInfoSize() {
    SMS_ASM_BLOCK("lis       3, sPlayerAnimeInfosSize@ha    \n\t"
                  "lwz       3, sPlayerAnimeInfosSize@l (3) \n\t"
                  "cmpw      20, 3                          \n\t"
                  "blr                                      \n\t");
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80246B90, 0, 0, 0), getExtendedPlayerAnimInfoSize);

static SMS_ASM_FUNC void checkExtendedPlayerAnimDataBodyFlags() {
    SMS_ASM_BLOCK("lis       3, sPlayerAnimeDatas@h      \n\t"
                  "ori       3, 3, sPlayerAnimeDatas@l   \n\t"
                  "add       3, 3, 0                     \n\t"
                  "lbz       0, 6 (3)                    \n\t"
                  "blr                                   \n\t");
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80245210, 0, 0, 0), checkExtendedPlayerAnimDataBodyFlags);
SMS_PATCH_BL(SMS_PORT_REGION(0x802452CC, 0, 0, 0), checkExtendedPlayerAnimDataBodyFlags);

static SMS_ASM_FUNC void checkExtendedPlayerAnimDataFluddUpper() {
    SMS_ASM_BLOCK("lis       6, sPlayerAnimeDatas@h         \n\t"
                  "ori       6, 6, sPlayerAnimeDatas@l      \n\t"
                  "addi      6, 6, 2                        \n\t"
                  "lhzx      0, 6, 0                        \n\t"
                  "lis       3, sPlayerAnimeInfosSize@ha    \n\t"  // This part is for sentinel
                                                                   // check after func
                  "lwz       3, sPlayerAnimeInfosSize@l (3) \n\t"
                  "blr                                      \n\t");
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80244f00, 0, 0, 0), checkExtendedPlayerAnimDataFluddUpper);
SMS_WRITE_32(SMS_PORT_REGION(0x80244f04, 0, 0, 0), 0x7C001800);

static SMS_ASM_FUNC void getExtendedPlayerAnimDataTexPattern() {
    SMS_ASM_BLOCK("lis       3, sPlayerAnimeDatas@h      \n\t"
                  "ori       3, 3, sPlayerAnimeDatas@l   \n\t"
                  "add       3, 3, 30                    \n\t"
                  "lbz       5, 4 (3)                    \n\t"
                  "blr                                   \n\t");
}
SMS_PATCH_BL(SMS_PORT_REGION(0x802478BC, 0, 0, 0), getExtendedPlayerAnimDataTexPattern);

static SMS_ASM_FUNC void getExtendedPlayerAnimDataHand() {
    SMS_ASM_BLOCK("lis       3, sPlayerAnimeDatas@h      \n\t"
                  "ori       3, 3, sPlayerAnimeDatas@l   \n\t"
                  "add       3, 3, 30                    \n\t"
                  "lbz       4, 5 (3)                    \n\t"
                  "blr                                   \n\t");
}
SMS_PATCH_BL(SMS_PORT_REGION(0x8024794C, 0, 0, 0), getExtendedPlayerAnimDataHand);

static SMS_ASM_FUNC void isPumpOkCallingExt() {
    SMS_ASM_BLOCK("lis       3, sPlayerAnimeDatas@h    \n\t"
                  "ori       0, 3, sPlayerAnimeDatas@l \n\t"
                  "blr                                 \n\t");
}
SMS_PATCH_BL(SMS_PORT_REGION(0x802624E8, 0, 0, 0), isPumpOkCallingExt);

static SMS_ASM_FUNC void isPumpOk() {
    SMS_ASM_BLOCK("lhz       3, 2 (3)                       \n\t"
                  "lis       4, sPlayerAnimeInfosSize@ha    \n\t"
                  "lwz       4, sPlayerAnimeInfosSize@l (4) \n\t"
                  "cmplw     3, 4                           \n\t"
                  "li 3, 0                                  \n\t"
                  "bgelr                                    \n\t"
                  "li 3, 1                                  \n\t"
                  "blr                                      \n\t");
}
SMS_PATCH_B(SMS_PORT_REGION(0x80248F14, 0, 0, 0), isPumpOk);
