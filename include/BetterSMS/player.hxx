#pragma once

#include <Dolphin/mem.h>
#include <Dolphin/types.h>
#include <SMS/macros.h>

#include <JSystem/JGeometry/JGMVec.hxx>
#include <JSystem/JSupport/JSUMemoryStream.hxx>
#include <JSystem/JUtility/JUTColor.hxx>
#include <SMS/Player/Mario.hxx>
#include <SMS/Camera/PolarSubCamera.hxx>
#include <SMS/raw_fn.hxx>

#include "libs/warp.hxx"
#include "libs/container.hxx"
#include "logging.hxx"
#include "module.hxx"

using namespace BetterSMS;
using namespace BetterSMS::Collision;

namespace BetterSMS {
    namespace Player {
        void *getRegisteredData(TMario *, const char *);

        struct TPlayerData;
        TPlayerData *getData(TMario *);

        // Use this to have extended params by inheriting TPlayerData
        bool registerData(TMario *, const char *, void *);
        bool deregisterData(TMario *, const char *);

        enum InteractionFlags {
            ON_ENTER       = 1 << 0,
            ON_EXIT        = 1 << 1,
            ON_CONTACT     = 1 << 2,
            ON_4CM_CONTACT = 1 << 3,
            ON_DETACH      = 1 << 4,
            GROUNDED       = 1 << 5,
            AIRBORN        = 1 << 6
        };

        typedef void (*InitProcess)(TMario *, bool);
        typedef void (*UpdateProcess)(TMario *, bool);
        typedef bool (*MachineProcess)(TMario *);
        typedef void (*CollisionProcess)(TMario *, const TBGCheckData *, u32 /*InteractionFlags*/);

        // Player init hook
        bool registerInitProcess(const char *, InitProcess);
        // Player update hook
        bool registerUpdateProcess(const char *, UpdateProcess);
        // Player state handlers (Custom movesets)
        bool registerStateMachine(u32, MachineProcess);
        // Player collision handlers (Custom collision types)
        bool registerCollisionHandler(u16, CollisionProcess);
        bool deregisterInitProcess(const char *);
        bool deregisterUpdateProcess(const char *);
        bool deregisterStateMachine(u32);
        bool deregisterCollisionHandler(u16);

        // Warps the player to a collision face
        void warpToCollisionFace(TMario *player, const TBGCheckData *face, bool isFluid);
        void warpToPoint(TMario *player, const TVec3f &destPoint, WarpKind kind, s32 framesToWarp,
                         bool isWarpFluid);
        void rotateRelativeToCamera(TMario *, CPolarSubCamera *, Vec2, f32);
        void setFire(TMario *);
        void extinguishFire(TMario *, bool);

#pragma region ParamHandler

        struct TPlayerParams : public TParams {
            TPlayerParams()
                : TParams(), SMS_TPARAM_INIT(mMaxJumps, 1), SMS_TPARAM_INIT(mMaxHealth, 8),
                  SMS_TPARAM_INIT(mCanRideYoshi, true), SMS_TPARAM_INIT(mCanUseFludd, true),
                  SMS_TPARAM_INIT(mPlayerHasHelmet, false),
                  SMS_TPARAM_INIT(mPlayerHasGlasses, false),
                  SMS_TPARAM_INIT(mPlayerHasShirt, false),
                  SMS_TPARAM_INIT(mGravityMultiplier, 1.0f), SMS_TPARAM_INIT(mSizeMultiplier, 1.0f),
                  SMS_TPARAM_INIT(mSpeedMultiplier, 1.0f),
                  SMS_TPARAM_INIT(mBaseJumpMultiplier, 1.0f),
                  SMS_TPARAM_INIT(mMultiJumpMultiplier, 0.875f),
                  SMS_TPARAM_INIT(mMultiJumpFSpeedMulti, 0.9f),
                  SMS_TPARAM_INIT(mThrowPowerMultiplier, 1.0f),
                  SMS_TPARAM_INIT(mUnderwaterHealthMultiplier, 1.0f),
                  SMS_TPARAM_INIT(mFallDamageMinMultiplier, 1.0f),
                  SMS_TPARAM_INIT(mSlideMultiplier, 1.0f) {
                load("/Mario/BetterSMS.prm");
            }

            enum class FluddCleanType : u8 { NONE, CLEAN, GOOP };

            TParamRT<u8> mMaxJumps;
            TParamRT<u8> mMaxHealth;
            TParamRT<bool> mCanRideYoshi;
            TParamRT<bool> mCanUseFludd;
            TParamRT<bool> mPlayerHasHelmet;
            TParamRT<bool> mPlayerHasGlasses;
            TParamRT<bool> mPlayerHasShirt;
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
            const TPlayerParams *getParams() const {
                if (!mParams)
                    Console::debugLog("Trying to access player params that don't exist!\n");
                return mParams;
            }
            TMario *getPlayer() const { return mPlayer; }
            u32 getPlayerID() const { return mPlayerID; }

            bool getColliding() { return mCollisionFlags.mIsColliding; }
            bool isMario() const { return !mIsEMario; }

            void setCamera(CPolarSubCamera *camera);
            void setCanUseFludd(bool enable) { mCanUseFludd = enable; }
            void setPlayer(TMario *player);
            void setPlayerID(u32 id) { mPlayerID = id; }
            void setColliding(bool colliding) { mCollisionFlags.mIsColliding = colliding; }

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
            u32 mLastQuarterFrameState;
            u16 mPrevCollisionType;
            const TBGCheckData *mPrevCollisionFloor;
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
            u8 mWarpState;
            bool mIsWarpActive;
            bool mIsWarpFluid;
            bool mIsWarpEnding;
        };

#pragma endregion
    }  // namespace Player
}  // namespace BetterSMS