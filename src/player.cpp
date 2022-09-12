#include <Dolphin/GX.h>
#include <Dolphin/OS.h>

#include <JSystem/J2D/J2DPrint.hxx>
#include <JSystem/JKernel/JKRFileLoader.hxx>
#include <SMS/GC2D/ConsoleStr.hxx>
#include <SMS/actor/Mario.hxx>
#include <SMS/mapobj/MapObjNormalLift.hxx>
#include <SMS/mapobj/MapObjTree.hxx>

#include <SMS/SMS.hxx>
#include <SMS/actor/HitActor.hxx>
#include <SMS/actor/LiveActor.hxx>
#include <SMS/assert.h>
#include <SMS/macros.h>
#include <SMS/option/CardManager.hxx>
#include <SMS/raw_fn.hxx>
#include <SMS/sound/MSoundSESystem.hxx>

#include "collision/warp.hxx"
#include "common_sdk.h"
#include "globals.hxx"
#include "libs/constmath.hxx"
#include "libs/triangle.hxx"
#include "logging.hxx"
#include "math.hxx"
#include "module.hxx"
#include "player.hxx"
#include "stage.hxx"

using namespace BetterSMS;
using namespace BetterSMS::Geometry;

static TDictI<TDictS<void *>> sPlayerDict;
static TDictS<Player::InitProcess> sPlayerInitializers;
static TDictS<Player::UpdateProcess> sPlayerUpdaters;

SMS_NO_INLINE BetterSMS::Player::TPlayerData *BetterSMS::Player::getData(TMario *player) {
    auto dataDict = sPlayerDict.setDefault(reinterpret_cast<u32>(player), TDictS<void *>());
    if (!dataDict.hasKey("__better_sms"))
        return nullptr;
    return reinterpret_cast<BetterSMS::Player::TPlayerData *>(dataDict.get("__better_sms").value());
}

SMS_NO_INLINE Optional<void *> BetterSMS::Player::getRegisteredData(TMario *player,
                                                                    const char *key) {
    const auto &dataDict = sPlayerDict.setDefault(reinterpret_cast<u32>(player), TDictS<void *>());
    if (!dataDict.hasKey(key))
        return {};
    return dataDict.get(key);
}

SMS_NO_INLINE bool BetterSMS::Player::registerData(TMario *player, const char *key, void *data) {
    auto &dataDict = sPlayerDict.setDefault(reinterpret_cast<u32>(player), TDictS<void *>());
    if (dataDict.hasKey(key))
        return false;
    dataDict.set(key, data);
    return true;
}

SMS_NO_INLINE bool BetterSMS::Player::deregisterData(TMario *player, const char *key) {
    auto &dataDict = sPlayerDict.setDefault(reinterpret_cast<u32>(player), TDictS<void *>());
    if (!dataDict.hasKey(key))
        return false;
    dataDict.pop(key);
    return true;
}

SMS_NO_INLINE bool BetterSMS::Player::registerInitProcess(const char *key, InitProcess process) {
    if (sPlayerInitializers.hasKey(key))
        return false;
    sPlayerInitializers.set(key, process);
    return true;
}

SMS_NO_INLINE bool BetterSMS::Player::registerUpdateProcess(const char *key,
                                                            UpdateProcess process) {
    if (sPlayerUpdaters.hasKey(key))
        return false;
    sPlayerUpdaters.set(key, process);
    return true;
}

SMS_NO_INLINE bool BetterSMS::Player::deregisterInitProcess(const char *key) {
    if (!sPlayerInitializers.hasKey(key))
        return false;
    sPlayerInitializers.pop(key);
    return true;
}

SMS_NO_INLINE bool BetterSMS::Player::deregisterUpdateProcess(const char *key) {
    if (!sPlayerUpdaters.hasKey(key))
        return false;
    sPlayerUpdaters.pop(key);
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

// Extern to player process
void processWarp(TMario *player, bool isMario) {
    constexpr s32 DisableMovementTime = 80;
    constexpr s32 TeleportTime        = 140;
    constexpr s32 EnableMovementTime  = 60;
    constexpr f32 WipeKindInDelay     = 1.0f;

    auto playerData = Player::getData(player);

    TBGCheckData *linkedCol =
        BetterSMS::sWarpColArray->resolveCollisionWarp(player->mFloorTriangle);
    if (!linkedCol)
        return;

    TVectorTriangle colTriangle =
        TVectorTriangle(linkedCol->mVertexA, linkedCol->mVertexB, linkedCol->mVertexC);

    TVec3f colCenter;
    colTriangle.center(colCenter);

    const f32 speed = PSVECMag(reinterpret_cast<Vec *>(&player->mSpeed));

    if (playerData->mIsWarpActive) {
        switch (playerData->mWarpKind) {
        case WarpKind::SPARKLES: {
            if (playerData->mWarpTimer > EnableMovementTime) {
                playerData->mCollisionFlags.mIsDisableInput = false;
                playerData->mIsWarpActive                   = false;
                player->mController->State.mReadInput       = true;
                playerData->mWarpTimer                      = 0;
            } else {
                playerData->mWarpTimer += 1;
            }
            break;
        }
        case WarpKind::WIPE: {
            if (gpApplication.mFader->mFadeStatus == TSMSFader::FADE_OFF) {
                playerData->mCollisionFlags.mIsDisableInput = false;
                playerData->mIsWarpActive                   = false;
                player->mController->State.mReadInput       = true;
                playerData->mWarpTimer                      = 0;
            } else {
                playerData->mWarpTimer += 1;
            }
            break;
        }
        case WarpKind::INSTANT:
        default:
            playerData->mCollisionFlags.mIsDisableInput = false;
            playerData->mIsWarpActive                   = false;
        }
    } else {
        if (!linkedCol) {
            playerData->mWarpTimer = 0;
            return;
        }

        switch (playerData->mWarpKind) {
        case WarpKind::SPARKLES: {
            size_t timeCut = 0;
            if (playerData->mWarpTimerMax == -1) {
                timeCut = DisableMovementTime;
            } else if (speed > 1.0f) {
                playerData->mWarpTimer = 0;
                return;
            }

            if (playerData->mWarpTimer >= TeleportTime - timeCut) {
                warpPlayerToPoint(player, colCenter);

                playerData->mIsWarpActive = true;
                playerData->mWarpTimer    = 0;
                startSoundActor__6TMarioFUl(player, static_cast<u32>(TMario::VOICE_JUMP));
            } else if (playerData->mWarpTimer >= DisableMovementTime - timeCut) {
                if (!playerData->mCollisionFlags.mIsDisableInput) {
                    emitGetEffect__6TMarioFv(player);
                }
                playerData->mCollisionFlags.mIsDisableInput = true;
                player->mController->State.mReadInput       = false;
            }
            playerData->mWarpTimer += 1;
            return;
        }
        case WarpKind::WIPE: {
            size_t timeCut = 0;
            if (playerData->mWarpTimerMax == -1) {
                timeCut = DisableMovementTime;
            } else if (speed > 1.0f) {
                playerData->mWarpTimer = 0;
                return;
            }

            if (gpApplication.mFader->mFadeStatus == TSMSFader::FADE_ON) {
                warpPlayerToPoint(player, colCenter);

                playerData->mIsWarpActive = true;
                playerData->mWarpTimer    = 0;
                sIsWiping                 = false;

                gpApplication.mFader->startWipe(TSMSFader::WipeRequest::FADE_CIRCLE_IN, 1.0f,
                                                WipeKindInDelay);
            } else if (playerData->mWarpTimer >= DisableMovementTime - timeCut) {
                playerData->mCollisionFlags.mIsDisableInput = true;
                player->mController->State.mReadInput       = false;
                if (gpApplication.mFader->mFadeStatus == TSMSFader::FADE_OFF && !sIsWiping) {
                    gpApplication.mFader->startWipe(TSMSFader::WipeRequest::FADE_SPIRAL_OUT, 1.0f,
                                                    0.0f);
                    MSoundSE::startSoundSystemSE(0x4859, 0, nullptr, 0);
                    sIsWiping = true;
                }
            }
            playerData->mWarpTimer += 1;
            return;
        }
        case WarpKind::INSTANT:
        default: {
            warpPlayerToPoint(player, colCenter);
            playerData->mCollisionTimer = 0;
            return;
        }
        }
    }
}

SMS_NO_INLINE void BetterSMS::Player::warpToCollisionFace(TMario *player, TBGCheckData *colTriangle,
                                                          bool isFluid) {
    constexpr s32 DisableMovementTime = 80;
    constexpr s32 TeleportTime        = 140;
    constexpr s32 EnableMovementTime  = 60;
    constexpr f32 WipeKindInDelay     = 1.0f;

    if (!player)
        return;

    auto playerData = Player::getData(player);

    TVec3f triCenter;
    TVectorTriangle vectorTri(colTriangle->mVertexA, colTriangle->mVertexB, colTriangle->mVertexC);
    vectorTri.center(triCenter);

    TVec3f triFluidCenter = triCenter;
    triFluidCenter.y += 300.0f;

#define EXPAND_WARP_SET(base) (base) : case ((base) + 10) : case ((base) + 20) : case ((base) + 30)
#define EXPAND_WARP_CATEGORY(base)                                                                 \
    (base) : case ((base) + 1) : case ((base) + 2) : case ((base) + 3) : case ((base) + 4)

    const u16 type = colTriangle->mCollisionType & 0x7FFF;
    switch (type) {
    case EXPAND_WARP_SET(16040):
    case EXPAND_WARP_SET(17040):
        warpToPoint(player, triCenter, WarpKind::SPARKLES, TeleportTime);  // Sparkles and then warp
        break;
    case EXPAND_WARP_SET(16041):
    case EXPAND_WARP_SET(17041):
        warpToPoint(player, triFluidCenter, WarpKind::INSTANT,
                    0);  // Portal momentum warp (locking)
        break;
    case EXPAND_WARP_SET(16042):
    case EXPAND_WARP_SET(17042):
        warpToPoint(player, triFluidCenter, WarpKind::INSTANT, 0);  // Portal momentum warp (free)
        break;
    case EXPAND_WARP_SET(16043):
    case EXPAND_WARP_SET(17043):
        warpToPoint(player, triCenter, WarpKind::WIPE, TeleportTime);  // Pipe warp
        break;
    case EXPAND_WARP_SET(16044):
    case EXPAND_WARP_SET(17044):
        warpToPoint(player, triCenter, WarpKind::INSTANT, 0);  // Portal warp
        break;
    case EXPAND_WARP_SET(16045):
    case EXPAND_WARP_SET(17045):
        warpToPoint(player, triCenter, WarpKind::SPARKLES,
                    0);  // Sparkled momentum warp
        break;
    case EXPAND_WARP_SET(16046):
    case EXPAND_WARP_SET(17046):
        warpToPoint(player, triCenter, WarpKind::WIPE, 0);  // Pipe momentum warp
        break;
    }
#undef EXPAND_WARP_SET
#undef EXPAND_WARP_CATEGORY
}

SMS_NO_INLINE void BetterSMS::Player::warpToPoint(TMario *player, const TVec3f &destPoint,
                                                  WarpKind kind, s32 framesToWarp) {
    if (!player)
        return;

    auto playerData              = Player::getData(player);
    playerData->mIsWarpActive    = true;
    playerData->mWarpDestination = destPoint;
    playerData->mWarpKind        = kind;
    playerData->mWarpTimerMax    = framesToWarp;
}

SMS_NO_INLINE void BetterSMS::Player::rotateRelativeToCamera(TMario *player,
                                                             CPolarSubCamera *camera, Vec2 dir,
                                                             f32 lerp_) {
    player->mAngle.y = lerp<f32>(
        player->mAngle.y,
        camera->mHorizontalAngle + s16(radiansToAngle(atan2f(dir.x, -dir.y)) * 182), lerp_);
}

#pragma region FireAPI

static constexpr s32 MaxFireDamageTime = 300;
static constexpr s32 MaxFireTime       = MaxFireDamageTime * 3;

SMS_NO_INLINE void BetterSMS::Player::setFire(TMario *player) {
    auto playerData = Player::getData(player);

    if (playerData->mIsOnFire) {
        playerData->mFireTimer %= MaxFireDamageTime;
        return;
    }

    playerData->mIsOnFire     = true;
    playerData->mFireTimer    = 0;
    playerData->mFireTimerMax = MaxFireTime;
    changePlayerStatus__6TMarioFUlUlb(player, 0x80000588, 0, 1);
}

SMS_NO_INLINE void BetterSMS::Player::extinguishFire(TMario *player, bool expired) {
    auto playerData = Player::getData(player);

    if (playerData->mIsOnFire && !expired)
        MSoundSE::startSoundActor(0x28C5, reinterpret_cast<Vec *>(&player->mPosition), 0, nullptr,
                                  0, 0);

    playerData->mIsOnFire  = false;
    playerData->mFireTimer = 0;
}

// Externed to player update process
void blazePlayer(TMario *player) {
    auto playerData = Player::getData(player);

    const f32 fireScale = 3.0f - (static_cast<f32>(playerData->mFireTimer) / MaxFireTime) * 2.0f;

    JPABaseEmitter *emitterFire =
        gpMarioParticleManager->emitAndBindToPosPtr(0x135, &player->mPosition, 1, player);
    emitterFire->mSize2.set(player->mSize.x * fireScale, player->mSize.y * fireScale,
                            player->mSize.z * fireScale);

    JPABaseEmitter *emitterSmoke =
        gpMarioParticleManager->emitAndBindToPosPtr(0x136, &player->mPosition, 1, player);
    emitterSmoke->mSize2.set(player->mSize.x * fireScale, player->mSize.y * fireScale,
                             player->mSize.z * fireScale);

    JPABaseEmitter *emitterEmber =
        gpMarioParticleManager->emitAndBindToPosPtr(0x137, &player->mPosition, 1, player);
    emitterEmber->mSize2.set(player->mSize.x * fireScale, player->mSize.y * fireScale,
                             player->mSize.z * fireScale);

    const s32 fireFrame = playerData->mFireTimer % MaxFireDamageTime;
    playerData->mFireTimer += 1;

    if (player->mFludd->mCurrentNozzle == TWaterGun::Hover && player->mFludd->mIsEmitWater)
        playerData->mFireTimerMax -= 1;

    if (fireFrame == 0) {
        decHP__6TMarioFi(player, 1);
        changePlayerStatus__6TMarioFUlUlb(player, 0x208B6, 0, 0);
        startVoice__6TMarioFUl(player, 0x7849);
    }

    if (!(player->mState & static_cast<u32>(TMario::STATE_AIRBORN))) {
        switch (static_cast<TMario::State>(player->mState)) {
        case TMario::STATE_TURNING_MID:
            break;
        case TMario::STATE_IDLE:
            changePlayerStatus__6TMarioFUlUlb(player, TMario::STATE_RUNNING, 0, false);
        default:
            setPlayerVelocity__6TMarioFf(player, 50.0f * player->mSize.z);
            player->mActionState = 1;
        }
    }

    if (playerData->mFireTimer > playerData->mFireTimerMax)
        Player::extinguishFire(player, true);
}

static void flameMario(TEffectObjBase *fire, u32 message) {
    s32 marioIdx;
    SMS_FROM_GPR(30, marioIdx);

    TMario *player = reinterpret_cast<TMario *>(fire->mCollidingObjs[marioIdx]);
    Player::setFire(player);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80038148, 0x80038200, 0, 0), flameMario);

#pragma endregion

#pragma region Patches

#if BETTER_SMS_BUGFIXES

static u32 patchYStorage() {
    TMario *player;
    SMS_FROM_GPR(31, player);

    if (player->mState != static_cast<u32>(TMario::STATE_IDLE))
        player->mSpeed.y = 0.0f;

    return 0;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x802571F0, 0x8024EF7C, 0, 0), patchYStorage);

static void patchRideMovementUpWarp(Mtx out, Vec *ride, Vec *pos) {
    TMario *player;
    SMS_FROM_GPR(30, player);

    if (!(player->mState & static_cast<u32>(TMario::STATE_AIRBORN))) {
        PSMTXMultVec(out, ride, pos);
    }
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80250514, 0x802482A0, 0, 0), patchRideMovementUpWarp);

static void patchRoofCollisionSpeed(TMario *player, f32 _speed) {
    TBGCheckData *roof = player->mRoofTriangle;
    if (!roof) {
        setPlayerVelocity__6TMarioFf(player, _speed);
        return;
    }

    TVec3f down(0.0f, -1.0f, 0.0f);

    TVec3f nroofvec;
    Vector3::normalized(*roof->getNormal(), nroofvec);

    const f32 ratio = Vector3::angleBetween(nroofvec, down);
    setPlayerVelocity__6TMarioFf(player, lerp(_speed, player->mForwardSpeed, ratio));
}
SMS_PATCH_BL(SMS_PORT_REGION(0x802569bc, 0x8024E748, 0, 0), patchRoofCollisionSpeed);

#endif

#pragma endregion

#pragma region TPlayerDataImplementation

Player::TPlayerData::TPlayerData(TMario *player, CPolarSubCamera *camera, bool isMario)
    : mPlayer(player), mCamera(camera), mIsEMario(!isMario), mPlayerID(0), mCurJump(0),
      mIsLongJumping(false), mIsClimbTired(false), mPrevCollisionType(0), mCollisionTimer(0),
      mClimbTiredTimer(0), mSlideSpeedMultiplier(1.0f), mMaxAddVelocity(150.0f),
      mYoshiWaterSpeed(0.0f, 0.0f, 0.0f), mDefaultAttrs(player), mIsOnFire(false), mFireTimer(0),
      mFireTimerMax(0) {

    mParams = new TPlayerParams();
    OSReport("CREATED NEW PLAYER PARAMS AT 0x%X\n", mParams);

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

    if (isMario && loadPrm("/sme.prm")) {
        Console::debugLog("Custom character params loaded!\n");
    } else {
        Console::debugLog("Default character params loaded!\n");
    }

    mCanUseFludd = mParams->mCanUseFludd.get();

    if (mParams->mPlayerHasGlasses.get() && player->mCap)
        reinterpret_cast<u16 *>(player->mCap)[2] |= 0b100;

    scalePlayerAttrs(mParams->mSizeMultiplier.get());
}

void Player::TPlayerData::scalePlayerAttrs(f32 scale) {
    scale = Max(scale, 0.0f);

    TVec3f size(1.0f, 1.0f, 1.0f);
    size.scale(scale);

    mPlayer->JSGSetScaling(reinterpret_cast<Vec &>(size));
    mPlayer->mModelData->mModel->mBaseScale = size;

    mDefaultAttrs.applyHistoryTo(const_cast<TMario *>(getPlayer()));

#define SCALE_PARAM(param, scale) param.set(param.get() * scale)

    const TPlayerParams *params = getParams();

    const f32 yoshiAgility = Math::sigmoidCurve(size.y, 0.0f, 1.2f, 1.321887582486f, -5.0f);

    f32 factor            = scaleLinearAtAnchor<f32>(scale, 0.5f, 1.0f);
    f32 speedMultiplier   = params->mSpeedMultiplier.get();
    f32 jumpMultiplier    = params->mBaseJumpMultiplier.get();
    f32 gravityMultiplier = params->mGravityMultiplier.get();
    if (onYoshi__6TMarioCFv(mPlayer)) {
        factor          = 1.0f;
        scale           = 1.0f;
        speedMultiplier = 1.0f;
        jumpMultiplier  = 1.0f;
    }

    SCALE_PARAM(mPlayer->mDeParams.mRunningMax, factor * speedMultiplier * 1.5f);
    SCALE_PARAM(mPlayer->mDeParams.mDashMax, factor * speedMultiplier * 1.5f);
    SCALE_PARAM(mPlayer->mDeParams.mShadowSize, scale);
    SCALE_PARAM(mPlayer->mDeParams.mHoldRadius, scale);
    SCALE_PARAM(mPlayer->mDeParams.mDamageRadius, scale);
    SCALE_PARAM(mPlayer->mDeParams.mDamageHeight, scale);
    SCALE_PARAM(mPlayer->mDeParams.mAttackHeight, scale);
    SCALE_PARAM(mPlayer->mDeParams.mTrampleRadius, scale);
    SCALE_PARAM(mPlayer->mDeParams.mPushupRadius, scale);
    SCALE_PARAM(mPlayer->mDeParams.mPushupHeight, scale);
    SCALE_PARAM(mPlayer->mDeParams.mHipdropRadius, scale);
    SCALE_PARAM(mPlayer->mDeParams.mQuakeRadius, scale);
    SCALE_PARAM(mPlayer->mDeParams.mQuakeHeight, scale);
    SCALE_PARAM(mPlayer->mDeParams.mJumpWallHeight, factor);
    SCALE_PARAM(mPlayer->mDeParams.mThrowPower, factor * params->mThrowPowerMultiplier.get());
    SCALE_PARAM(mPlayer->mDeParams.mFeelDeep, factor);
    SCALE_PARAM(mPlayer->mDeParams.mDamageFallHeight, factor);
    SCALE_PARAM(mPlayer->mDeParams.mClashSpeed, factor * speedMultiplier);
    SCALE_PARAM(mPlayer->mDeParams.mSleepingCheckDist, factor);
    SCALE_PARAM(mPlayer->mDeParams.mSleepingCheckHeight, factor);
    SCALE_PARAM(mPlayer->mPunchFenceParams.mRadius, factor);
    SCALE_PARAM(mPlayer->mPunchFenceParams.mHeight, factor);
    SCALE_PARAM(mPlayer->mKickRoofParams.mRadius, factor);
    SCALE_PARAM(mPlayer->mKickRoofParams.mHeight, factor);
    SCALE_PARAM(mPlayer->mJumpParams.mGravity, factor * gravityMultiplier);
    SCALE_PARAM(mPlayer->mJumpParams.mSpinJumpGravity, factor * gravityMultiplier);
    SCALE_PARAM(mPlayer->mJumpParams.mJumpSpeedAccelControl, factor * speedMultiplier);
    SCALE_PARAM(mPlayer->mJumpParams.mPopUpSpeedY, factor * jumpMultiplier);
    SCALE_PARAM(mPlayer->mJumpParams.mJumpingMax, factor * jumpMultiplier);
    SCALE_PARAM(mPlayer->mJumpParams.mFenceSpeed, factor * speedMultiplier);
    SCALE_PARAM(mPlayer->mJumpParams.mFireBackVelocity, factor * jumpMultiplier);
    SCALE_PARAM(mPlayer->mJumpParams.mBroadJumpForce, factor * 1.5f);
    SCALE_PARAM(mPlayer->mJumpParams.mBroadJumpForceY, factor * jumpMultiplier);
    SCALE_PARAM(mPlayer->mJumpParams.mRotateJumpForceY, factor * jumpMultiplier);
    SCALE_PARAM(mPlayer->mJumpParams.mBackJumpForce, factor);
    SCALE_PARAM(mPlayer->mJumpParams.mBackJumpForceY, factor * jumpMultiplier);
    SCALE_PARAM(mPlayer->mJumpParams.mHipAttackSpeedY, factor * gravityMultiplier);
    SCALE_PARAM(mPlayer->mJumpParams.mSuperHipAttackSpeedY, factor * gravityMultiplier);
    SCALE_PARAM(mPlayer->mJumpParams.mRotBroadJumpForce, factor);
    SCALE_PARAM(mPlayer->mJumpParams.mRotBroadJumpForceY, factor * jumpMultiplier);
    SCALE_PARAM(mPlayer->mJumpParams.mSecJumpForce, factor * jumpMultiplier);
    SCALE_PARAM(mPlayer->mJumpParams.mUltraJumpForce, factor * jumpMultiplier);
    SCALE_PARAM(mPlayer->mJumpParams.mTurnJumpForce, factor * jumpMultiplier);
    SCALE_PARAM(mPlayer->mJumpParams.mTriJumpEnableSp, scale);
    SCALE_PARAM(mPlayer->mJumpParams.mValleyDepth, factor);
    SCALE_PARAM(mPlayer->mJumpParams.mTremblePower, 1.0f / factor);
    SCALE_PARAM(mPlayer->mJumpParams.mTrembleTime, static_cast<s16>(1.0f / factor));
    SCALE_PARAM(mPlayer->mJumpParams.mGetOffYoshiY, factor);
    SCALE_PARAM(mPlayer->mJumpParams.mSuperHipAttackCt, static_cast<s16>(1.0f / factor));
    SCALE_PARAM(mPlayer->mRunParams.mMaxSpeed, factor * speedMultiplier);
    SCALE_PARAM(mPlayer->mRunParams.mAddBase, factor * speedMultiplier);
    SCALE_PARAM(mPlayer->mRunParams.mDecBrake, factor * speedMultiplier);
    SCALE_PARAM(mPlayer->mRunParams.mSoft2Walk, factor);
    SCALE_PARAM(mPlayer->mRunParams.mWalk2Soft, factor);
    SCALE_PARAM(mPlayer->mRunParams.mSoftStepAnmMult, 1 / factor);
    SCALE_PARAM(mPlayer->mRunParams.mRunAnmSpeedMult, 1 / factor);
    SCALE_PARAM(mPlayer->mRunParams.mMotBlendWalkSp, 1 / factor);
    SCALE_PARAM(mPlayer->mRunParams.mMotBlendRunSp, 1 / factor);
    SCALE_PARAM(mPlayer->mRunParams.mSwimDepth, factor);
    SCALE_PARAM(mPlayer->mRunParams.mTurnNeedSp, factor);
    SCALE_PARAM(mPlayer->mSwimParams.mStartSp, factor);
    SCALE_PARAM(mPlayer->mSwimParams.mMoveSp, factor * speedMultiplier);
    SCALE_PARAM(mPlayer->mSwimParams.mGravity, factor * gravityMultiplier);
    SCALE_PARAM(mPlayer->mSwimParams.mWaitBouyancy, factor);
    SCALE_PARAM(mPlayer->mSwimParams.mMoveBouyancy, factor);
    SCALE_PARAM(mPlayer->mSwimParams.mCanJumpDepth, scale);
    SCALE_PARAM(mPlayer->mSwimParams.mEndDepth, scale);
    SCALE_PARAM(mPlayer->mSwimParams.mFloatHeight, scale);
    SCALE_PARAM(mPlayer->mSwimParams.mRush, factor * speedMultiplier);
    SCALE_PARAM(mPlayer->mSwimParams.mPaddleSpeedUp, factor * speedMultiplier);
    SCALE_PARAM(mPlayer->mSwimParams.mPaddleJumpUp, factor * speedMultiplier);
    SCALE_PARAM(mPlayer->mSwimParams.mFloatUp, factor);
    SCALE_PARAM(mPlayer->mSwimParams.mWaterLevelCheckHeight, 1.0f / scale);
    SCALE_PARAM(mPlayer->mSwimParams.mPaddleDown, factor * speedMultiplier);
    SCALE_PARAM(mPlayer->mSwimParams.mCanBreathDepth, scale);
    SCALE_PARAM(mPlayer->mSwimParams.mWaitSinkSpeed, factor);
    SCALE_PARAM(mPlayer->mSwimParams.mAirDec, 1.0f / params->mUnderwaterHealthMultiplier.get());
    SCALE_PARAM(mPlayer->mSwimParams.mAirDecDive, 1.0f / params->mUnderwaterHealthMultiplier.get());
    SCALE_PARAM(mPlayer->mHangFenceParams.mMoveSp, factor * speedMultiplier);
    SCALE_PARAM(mPlayer->mHangFenceParams.mDescentSp, factor * speedMultiplier);
    SCALE_PARAM(mPlayer->mPullBGBeakParams.mPullRateV, factor);
    SCALE_PARAM(mPlayer->mPullBGBeakParams.mPullRateH, factor);
    SCALE_PARAM(mPlayer->mPullBGBeakParams.mOilPullRateV, factor);
    SCALE_PARAM(mPlayer->mPullBGBeakParams.mOilPullRateH, factor);
    SCALE_PARAM(mPlayer->mPullBGTentacleParams.mPullRateV, factor);
    SCALE_PARAM(mPlayer->mPullBGTentacleParams.mPullRateH, factor);
    SCALE_PARAM(mPlayer->mPullBGTentacleParams.mOilPullRateV, factor);
    SCALE_PARAM(mPlayer->mPullBGTentacleParams.mOilPullRateH, factor);
    SCALE_PARAM(mPlayer->mPullBGFireWanWanBossTailParams.mPullRateV, factor);
    SCALE_PARAM(mPlayer->mPullBGFireWanWanBossTailParams.mPullRateH, factor);
    SCALE_PARAM(mPlayer->mPullBGFireWanWanBossTailParams.mOilPullRateV, factor);
    SCALE_PARAM(mPlayer->mPullBGFireWanWanBossTailParams.mOilPullRateH, factor);
    SCALE_PARAM(mPlayer->mPullFireWanWanTailParams.mPullRateV, factor);
    SCALE_PARAM(mPlayer->mPullFireWanWanTailParams.mPullRateH, factor);
    SCALE_PARAM(mPlayer->mPullFireWanWanTailParams.mOilPullRateV, factor);
    SCALE_PARAM(mPlayer->mPullFireWanWanTailParams.mOilPullRateH, factor);
    SCALE_PARAM(mPlayer->mDivingParams.mGravity, factor);
    SCALE_PARAM(mPlayer->mYoshiParams.mRunYoshiMult, yoshiAgility);
    SCALE_PARAM(mPlayer->mYoshiParams.mJumpYoshiMult, yoshiAgility);
    SCALE_PARAM(mPlayer->mYoshiParams.mRotYoshiMult, yoshiAgility);
    SCALE_PARAM(mPlayer->mWaterEffectParams.mJumpIntoMinY, factor);
    SCALE_PARAM(mPlayer->mWaterEffectParams.mJumpIntoMaxY, factor);
    SCALE_PARAM(mPlayer->mWaterEffectParams.mJumpIntoScaleMin, factor);
    SCALE_PARAM(mPlayer->mWaterEffectParams.mJumpIntoScaleWidth, factor);
    SCALE_PARAM(mPlayer->mWaterEffectParams.mRunningRippleDepth, factor);
    SCALE_PARAM(mPlayer->mGraffitoParams.mSinkHeight, factor);
    SCALE_PARAM(mPlayer->mGraffitoParams.mSinkMoveMin, factor);
    SCALE_PARAM(mPlayer->mGraffitoParams.mSinkMoveMax, factor);
    SCALE_PARAM(mPlayer->mGraffitoParams.mSinkRecover, factor);
    SCALE_PARAM(mPlayer->mGraffitoParams.mFootEraseSize, scale);
    SCALE_PARAM(mPlayer->mDirtyParams.mPolSizeSlip, scale);
    SCALE_PARAM(mPlayer->mDirtyParams.mPolSizeRun, scale);
    SCALE_PARAM(mPlayer->mDirtyParams.mPolSizeFootPrint, scale);
    SCALE_PARAM(mPlayer->mDirtyParams.mPolSizeJump, scale);
    SCALE_PARAM(mPlayer->mDirtyParams.mSlipAnmSpeed, 1 / factor);
    SCALE_PARAM(mPlayer->mDirtyParams.mSlipRunSp, factor * speedMultiplier);
    SCALE_PARAM(mPlayer->mDirtyParams.mSlipCatchSp, factor * speedMultiplier);

#undef SCALE_PARAM
}

void Player::TPlayerData::setPlayer(TMario *player) {
    mPlayer = player;
    mDefaultAttrs.recordFrom(player);
}

bool Player::TPlayerData::loadPrm(const char *prm = "/sme.prm") {
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

bool Player::TPlayerData::canUseNozzle(TWaterGun::NozzleType nozzle) const {
    switch (nozzle) {
    case TWaterGun::NozzleType::Spray:
        return mParams->mSprayNozzleUsable.get();
    case TWaterGun::NozzleType::Rocket:
        return mParams->mRocketNozzleUsable.get();
    case TWaterGun::NozzleType::Underwater:
        return mParams->mUnderwaterNozzleUsable.get();
    case TWaterGun::NozzleType::Yoshi:
        return mParams->mYoshiNozzleUsable.get();
    case TWaterGun::NozzleType::Hover:
        return mParams->mHoverNozzleUsable.get();
    case TWaterGun::NozzleType::Turbo:
        return mParams->mTurboNozzleUsable.get();
    case TWaterGun::NozzleType::Sniper:
        return mParams->mSniperNozzleUsable.get();
    default:
        return false;
    }
}

u8 Player::TPlayerData::getNozzleBoneID(TWaterGun::NozzleType nozzle) const {
    switch (nozzle) {
    case TWaterGun::NozzleType::Spray:
        return mParams->mSprayNozzleBoneID.get();
    case TWaterGun::NozzleType::Rocket:
        return mParams->mRocketNozzleBoneID.get();
    case TWaterGun::NozzleType::Underwater:
        return mParams->mUnderwaterNozzleBoneID.get();
    case TWaterGun::NozzleType::Yoshi:
        return mParams->mYoshiNozzleBoneID.get();
    case TWaterGun::NozzleType::Hover:
        return mParams->mHoverNozzleBoneID.get();
    case TWaterGun::NozzleType::Turbo:
        return mParams->mTurboNozzleBoneID.get();
    case TWaterGun::NozzleType::Sniper:
        return mParams->mSniperNozzleBoneID.get();
    default:
        return false;
    }
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

static void initFluddInLoadAfter(TWaterGun *fludd) { fludd->mNozzleList[4]->mDamageLoss = 250; }
SMS_PATCH_B(SMS_PORT_REGION(0x8026A3B8, 0x80262144, 0, 0), initFluddInLoadAfter);

static void initFludd(TMario *player, Player::TPlayerData *playerData) {
    SMS_ASSERT(playerData, "Can't init fludd with non existant playerData!");
    Stage::TStageParams *config = Stage::getStageConfiguration();

    if (!playerData->isMario())
        return;

    if (!playerData->canUseNozzle(
            static_cast<TWaterGun::NozzleType>(player->mFludd->mCurrentNozzle))) {
        for (u8 i = 0; i < 7; ++i) {
            if (playerData->canUseNozzle(static_cast<TWaterGun::NozzleType>(i))) {
                player->mAttributes.mHasFludd  = playerData->getCanUseFludd();
                player->mFludd->mCurrentNozzle = i;
                player->mFludd->mCurrentWater =
                    player->mFludd->mNozzleList[(u8)player->mFludd->mCurrentNozzle]->mMaxWater;
                break;
            } else if (i == 6) {
                player->mAttributes.mHasFludd = false;
                playerData->setCanUseFludd(false);
            }
        }
    }

    if (!playerData->canUseNozzle(
            static_cast<TWaterGun::NozzleType>(player->mFludd->mSecondNozzle))) {
        for (u8 i = 0; i < 7; ++i) {
            if (playerData->canUseNozzle(static_cast<TWaterGun::NozzleType>(i))) {
                player->mAttributes.mHasFludd = playerData->getCanUseFludd();
                player->mFludd->mSecondNozzle = i;
                break;
            }
            player->mFludd->mSecondNozzle = player->mFludd->mCurrentNozzle;
        }
    }

    if (config->mFluddShouldColorWater.get())
        gModelWaterManagerWaterColor = config->mFluddWaterColor.get();

    player->mFludd->mCurrentNozzle = config->mFluddPrimary.get();
    player->mFludd->mSecondNozzle  = config->mFluddSecondary.get();

    player->mFludd->mCurrentWater =
        player->mFludd->mNozzleList[(u8)player->mFludd->mCurrentNozzle]->mMaxWater;
}

// Extern to Player Init CB
void initMario(TMario *player, bool isMario) {
    Stage::TStageParams *config = Stage::getStageConfiguration();

    Player::TPlayerData *params = new Player::TPlayerData(player, nullptr, isMario);
    Player::registerData(player, "__better_sms", params);

    params->scalePlayerAttrs(config->mPlayerSizeMultiplier.get());

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

static TMario *playerInitHandler(TMario *player) {
    player->initValues();

    for (auto &item : sPlayerInitializers.items()) {
        item.mItem.mValue(player, true);
    }

    return player;
}
SMS_WRITE_32(SMS_PORT_REGION(0x80276C90, 0x8026EA1C, 0, 0), 0x60000000);
SMS_PATCH_BL(SMS_PORT_REGION(0x80276C94, 0x8026EA20, 0, 0), playerInitHandler);

// 0x800397DC
static bool shadowMarioInitHandler() {
    TMario *player;
    SMS_ASM_BLOCK("lwz %0, 0x150 (31)" : "=r"(player));

    for (const auto &item : sPlayerInitializers.items()) {
        item.mItem.mValue(player, false);
    }

    return SMS_isMultiPlayerMap__Fv();
}
SMS_PATCH_BL(SMS_PORT_REGION(0x800397DC, 0x80039894, 0, 0), shadowMarioInitHandler);

static void playerUpdateHandler(TMario *player) {
    for (auto &item : sPlayerUpdaters.items()) {
        item.mItem.mValue(player, true);
    }

    setPositions__6TMarioFv(player);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x8024D3A8, 0x80245134, 0, 0), playerUpdateHandler);  // Mario

static void shadowMarioUpdateHandler(TMario *player) {
    for (auto &item : sPlayerUpdaters.items()) {
        item.mItem.mValue(player, false);
    }

    setPositions__6TMarioFv(player);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x8003F8F0, 0x8003F740, 0, 0), shadowMarioUpdateHandler);  // EMario