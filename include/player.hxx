#pragma once

#include <Dolphin/mem.h>
#include <Dolphin/types.h>
#include <SMS/macros.h>

#include <JSystem/JGeometry.hxx>
#include <JSystem/JSupport/JSUMemoryStream.hxx>
#include <JSystem/JUtility/JUTColor.hxx>
#include <SMS/actor/Mario.hxx>
#include <SMS/camera/PolarSubCamera.hxx>
#include <SMS/raw_fn.hxx>

#include "collision/warp.hxx"
#include "libs/container.hxx"

using namespace BetterSMS;
using namespace BetterSMS::Collision;

namespace BetterSMS::Player {
    Optional<void *> getRegisteredData(TMario *, const char *);

    struct TPlayerData;
    TPlayerData *getData(TMario *);

    // Use this to have extended params by inheriting TPlayerData
    bool registerData(TMario *, const char *, void *);
    bool deregisterData(TMario *, const char *);

    typedef void (*InitProcess)(TMario *, bool);
    typedef void (*UpdateProcess)(TMario *, bool);

    // Player init hook
    bool registerInitProcess(const char *, InitProcess);
    // Player update hook
    bool registerUpdateProcess(const char *, UpdateProcess);
    bool deregisterInitProcess(const char *);
    bool deregisterUpdateProcess(const char *);

    // Warps the player to a collision face
    void warpToCollisionFace(TMario *, TBGCheckData *, bool);
    void warpToPoint(TMario *player, const TVec3f &destPoint, WarpKind kind, s32 framesToWarp);
    void rotateRelativeToCamera(TMario *, CPolarSubCamera *, Vec2, f32);
    void setFire(TMario *);
    void extinguishFire(TMario *, bool);

#pragma region ParamHandler

    struct TPlayerParams : public TParams {
#define CONSTRUCT_PARAM(name, val)                                                                 \
    name(this, val, JDrama::TNameRef::calcKeyCode(SMS_STRINGIZE(name)), SMS_STRINGIZE(name))

        TPlayerParams()
            : TParams(), CONSTRUCT_PARAM(mMaxJumps, 1), CONSTRUCT_PARAM(mMaxHealth, 8),
              CONSTRUCT_PARAM(mMaxWallHangTimer, 2400), CONSTRUCT_PARAM(mMaxWallClimbTimer, 1200),
              CONSTRUCT_PARAM(mCanRideYoshi, true), CONSTRUCT_PARAM(mCanUseFludd, true),
              CONSTRUCT_PARAM(mPlayerHasHelmet, false), CONSTRUCT_PARAM(mPlayerHasGlasses, false),
              CONSTRUCT_PARAM(mPlayerHasShirt, false), CONSTRUCT_PARAM(mCanCleanSeals, false),
              CONSTRUCT_PARAM(mGoopAffected, true), CONSTRUCT_PARAM(mCanHoldNPCs, false),
              CONSTRUCT_PARAM(mCanClimbWalls, false), CONSTRUCT_PARAM(mGravityMultiplier, 1.0f),
              CONSTRUCT_PARAM(mSizeMultiplier, 1.0f), CONSTRUCT_PARAM(mSpeedMultiplier, 1.0f),
              CONSTRUCT_PARAM(mBaseJumpMultiplier, 1.0f),
              CONSTRUCT_PARAM(mMultiJumpMultiplier, 0.875f),
              CONSTRUCT_PARAM(mMultiJumpFSpeedMulti, 0.9f),
              CONSTRUCT_PARAM(mThrowPowerMultiplier, 1.0f),
              CONSTRUCT_PARAM(mUnderwaterHealthMultiplier, 1.0f),
              CONSTRUCT_PARAM(mFallDamageMinMultiplier, 1.0f),
              CONSTRUCT_PARAM(mSlideMultiplier, 1.0f), CONSTRUCT_PARAM(mSprayNozzleUsable, true),
              CONSTRUCT_PARAM(mRocketNozzleUsable, true),
              CONSTRUCT_PARAM(mUnderwaterNozzleUsable, true),
              CONSTRUCT_PARAM(mYoshiNozzleUsable, true), CONSTRUCT_PARAM(mHoverNozzleUsable, true),
              CONSTRUCT_PARAM(mTurboNozzleUsable, true), CONSTRUCT_PARAM(mSniperNozzleUsable, true),
              CONSTRUCT_PARAM(mSprayNozzleBoneID, 14), CONSTRUCT_PARAM(mRocketNozzleBoneID, 14),
              CONSTRUCT_PARAM(mUnderwaterNozzleBoneID, 14), CONSTRUCT_PARAM(mYoshiNozzleBoneID, 14),
              CONSTRUCT_PARAM(mHoverNozzleBoneID, 14), CONSTRUCT_PARAM(mTurboNozzleBoneID, 14),
              CONSTRUCT_PARAM(mSniperNozzleBoneID, 14),
              CONSTRUCT_PARAM(mWaterColor, JUtility::TColor(60, 70, 120, 20)),
              CONSTRUCT_PARAM(mCleaningType, FluddCleanType::CLEAN) {
            load("/Mario/SME.prm");
        }

#undef CONSTRUCT_PARAM

        enum class FluddCleanType : u8 { NONE, CLEAN, GOOP };

        TParamRT<u8> mMaxJumps;
        TParamRT<u8> mMaxHealth;
        TParamRT<u16> mMaxWallHangTimer;
        TParamRT<s16> mMaxWallClimbTimer;
        TParamRT<bool> mCanRideYoshi;
        TParamRT<bool> mCanUseFludd;
        TParamRT<bool> mPlayerHasHelmet;
        TParamRT<bool> mPlayerHasGlasses;
        TParamRT<bool> mPlayerHasShirt;
        TParamRT<bool> mCanCleanSeals;
        TParamRT<bool> mGoopAffected;
        TParamRT<bool> mCanHoldNPCs;
        TParamRT<bool> mCanClimbWalls;
        TParamRT<f32> mGravityMultiplier;
        TParamRT<f32> mSizeMultiplier;
        TParamRT<f32> mSpeedMultiplier;
        TParamRT<f32> mBaseJumpMultiplier;
        TParamRT<f32> mMultiJumpMultiplier;
        TParamRT<f32> mMultiJumpFSpeedMulti;
        TParamRT<f32> mThrowPowerMultiplier;
        TParamRT<f32> mUnderwaterHealthMultiplier;
        TParamRT<f32> mFallDamageMinMultiplier;
        TParamRT<f32> mSlideMultiplier;
        TParamRT<bool> mSprayNozzleUsable;
        TParamRT<bool> mRocketNozzleUsable;
        TParamRT<bool> mUnderwaterNozzleUsable;
        TParamRT<bool> mYoshiNozzleUsable;
        TParamRT<bool> mHoverNozzleUsable;
        TParamRT<bool> mTurboNozzleUsable;
        TParamRT<bool> mSniperNozzleUsable;
        TParamRT<u8> mSprayNozzleBoneID;
        TParamRT<u8> mRocketNozzleBoneID;
        TParamRT<u8> mUnderwaterNozzleBoneID;
        TParamRT<u8> mYoshiNozzleBoneID;
        TParamRT<u8> mHoverNozzleBoneID;
        TParamRT<u8> mTurboNozzleBoneID;
        TParamRT<u8> mSniperNozzleBoneID;
        TParamRT<JUtility::TColor> mWaterColor;
        TParamRT<FluddCleanType> mCleaningType;
    };

    class TPlayerData {

    public:
        struct ParamHistory {
            ParamHistory();
            ParamHistory(TMario *player);

            bool hasHistory() const { return mHasHistory; }

            void applyHistoryTo(TMario *player);
            void recordFrom(TMario *player);
            void reset();

            TMario::TDeParams mDeParams;
            TMario::TBodyAngleParams mBodyAngleFreeParams;
            TMario::TBodyAngleParams mBodyAngleWaterGunParams;
            TMario::TAttackParams mPunchFenceParams;
            TMario::TAttackParams mKickRoofParams;
            TMario::TJumpParams mJumpParams;
            TMario::TRunParams mRunParams;
            TMario::TSwimParams mSwimParams;
            TMario::THangFenceParams mHangFenceParams;
            TMario::THangRoofParams mHangRoofParams;
            TMario::TWireParams mWireParams;
            TMario::TPullParams mPullBGBeakParams;
            TMario::TPullParams mPullBGTentacleParams;
            TMario::TPullParams mPullBGFireWanWanBossTailParams;
            TMario::TPullParams mPullFireWanWanTailParams;
            TMario::THoverParams mHoverParams;
            TMario::TDivingParams mDivingParams;
            TMario::TYoshiParams mYoshiParams;
            TMario::TWaterEffectParams mWaterEffectParams;
            TMario::TControllerParams mControllerParams;
            TMario::TGraffitoParams mGraffitoParams;
            TMario::TDirtyParams mDirtyParams;
            TMario::TMotorParams mMotorParams;
            TMario::TParticleParams mParticleParams;
            TMario::TEffectParams mEffectParams;
            TMario::TSlipParams mSlipNormalParams;
            TMario::TSlipParams mSlipOilParams;
            TMario::TSlipParams mSlipAllParams;
            TMario::TSlipParams mSlipAllSliderParams;
            TMario::TSlipParams mSlip45Params;
            TMario::TSlipParams mSlipWaterSlopeParams;
            TMario::TSlipParams mSlipWaterGroundParams;
            TMario::TSlipParams mSlipYoshiParams;
            TMario::TUpperBodyParams mUpperBodyParams;

        private:
            bool mHasHistory;
        };

        struct FluddHistory {
            s32 mWaterLevel;
            u8 mMainNozzle;
            u8 mSecondNozzle;
            bool mHadFludd;
        };

        TPlayerData() = delete;
        TPlayerData(TMario *player, CPolarSubCamera *camera, bool isMario);

        CPolarSubCamera *getCamera() const { return mCamera; }
        bool getCanUseFludd() const { return mCanUseFludd; }
        u8 getMaxJumps() const { return mParams->mMaxJumps.get(); }
        u8 getNozzleBoneID(TWaterGun::NozzleType nozzle) const;
        const TPlayerParams *getParams() const { return mParams; }
        const TMario *getPlayer() const { return mPlayer; }
        u32 getPlayerID() const { return mPlayerID; }
        const u16 getPlayerKey() const { return JDrama::TNameRef::calcKeyCode(getPlayerName()); }
        bool getColliding() { return mCollisionFlags.mIsColliding; }
        bool isMario() const { return !mIsEMario; }

        void setCamera(CPolarSubCamera *camera);
        void setCanUseFludd(bool enable) { mCanUseFludd = enable; }
        void setPlayer(TMario *player);
        void setPlayerID(u32 id) { mPlayerID = id; }
        void setColliding(bool colliding) { mCollisionFlags.mIsColliding = colliding; }

        bool canUseNozzle(TWaterGun::NozzleType nozzle) const;
        const char *getPlayerName() const;
        bool loadPrm(const char *prm);
        bool loadPrm(JSUMemoryInputStream &stream);
        void resetPlayer() { mDefaultAttrs.applyHistoryTo(mPlayer); };
        void scalePlayerAttrs(f32 scale);

    private:
        TMario *mPlayer;
        CPolarSubCamera *mCamera;
        TPlayerParams *mParams;
        bool mIsEMario;
        bool mCanUseFludd;
        u32 mPlayerID;

    public:
        u8 mCurJump;
        bool mIsLongJumping;
        bool mIsClimbTired;
        u16 mPrevCollisionType;
        TBGCheckData *mPrevCollisionFloor;
        s32 mCollisionTimer;
        s32 mClimbTiredTimer;
        f32 mSlideSpeedMultiplier;
        f32 mMaxAddVelocity;

        struct {
            bool mIsColliding     : 1;
            bool mIsAirborn       : 1;
            bool mIsFaceUsed      : 1;
            bool mIsSpinBounce    : 1;
            bool mIsDisableInput  : 1;
            bool mIsCollisionUsed : 1;
            bool mIsWarpUsed      : 1;
            s16 mCrushedTimer;
        } mCollisionFlags;

        TVec3f mYoshiWaterSpeed;
        FluddHistory mFluddHistory;
        ParamHistory mDefaultAttrs;
        TMario::TDirtyParams mDefaultDirtyParams;
        bool mIsOnFire;
        s16 mFireTimer;
        s16 mFireTimerMax;

        TVec3f mWarpDestination;
        s32 mWarpTimer;
        s32 mWarpTimerMax;
        WarpKind mWarpKind;
        bool mIsWarpActive;
    };

#pragma endregion

}  // namespace BetterSMS::Player