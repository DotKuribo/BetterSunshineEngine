#pragma once

#include <Dolphin/mem.h>
#include <Dolphin/types.h>
#include <SMS/macros.h>

#include <JSystem/JGeometry/JGMVec.hxx>
#include <JSystem/JSupport/JSUMemoryStream.hxx>
#include <JSystem/JUtility/JUTColor.hxx>
#include <SMS/Camera/PolarSubCamera.hxx>
#include <SMS/Player/Mario.hxx>
#include <SMS/raw_fn.hxx>

#include "libs/container.hxx"
#include "logging.hxx"
#include "module.hxx"
#include "warp.hxx"

using namespace BetterSMS;
using namespace BetterSMS::Collision;

namespace BetterSMS {
    namespace Player {
        struct TPlayerData;

        // Get the specialized BetterSMS info of a player
        void *getRegisteredData(TMario *, const char *);

        // Get arbitrary module data for a player
        TPlayerData *getData(TMario *);

        // Use this to have extended params by potentially inheriting TPlayerData
        bool registerData(TMario *, const char *, void *);
        void deregisterData(TMario *, const char *);

        enum InteractionFlags {
            ON_ENTER       = 1 << 0,
            ON_EXIT        = 1 << 1,
            ON_CONTACT     = 1 << 2,
            ON_4CM_CONTACT = 1 << 3,
            ON_DETACH      = 1 << 4,
            GROUNDED       = 1 << 5,
            AIRBORN        = 1 << 6
        };

        typedef void (*InitCallback)(TMario *, bool);
        typedef void (*LoadAfterCallback)(TMario *);
        typedef void (*UpdateCallback)(TMario *, bool);
        typedef bool (*MachineCallback)(TMario *);
        typedef void (*CollisionCallback)(TMario *, const TBGCheckData *, u32 /*InteractionFlags*/);

        // Add a function to call on player init
        bool addInitCallback(InitCallback);
        // Add a function to call after the player loads and initializes
        bool addLoadAfterCallback(LoadAfterCallback);
        // Add a function to call on player update
        bool addUpdateCallback(UpdateCallback);
        // Register a function to call for a specific player state
        bool registerStateMachine(u32, MachineCallback);
        // Register a function to call for a specific collision type
        bool registerCollisionHandler(u16, CollisionCallback);

        // Warps the player to a collision face
        void warpToCollisionFace(TMario *player, const TBGCheckData *face, bool isFluid);

        // Warps the player to a specific coordinate
        void warpToPoint(TMario *player, const TVec3f &destPoint, WarpKind kind, s32 framesToWarp,
                         bool isWarpFluid);

        // Apply a rotation to a player, which is relative to the camera
        void rotateRelativeToCamera(TMario *, CPolarSubCamera *, Vec2, f32);

#pragma region ParamHandler

        struct TPlayerParams : public TParams {
            TPlayerParams()
                : TParams(), SMS_TPARAM_INIT(mMaxHealth, 8), SMS_TPARAM_INIT(mCanRideYoshi, true),
                  SMS_TPARAM_INIT(mCanUseFludd, true), SMS_TPARAM_INIT(mPlayerHasHelmet, false),
                  SMS_TPARAM_INIT(mPlayerHasGlasses, false),
                  SMS_TPARAM_INIT(mPlayerHasShirt, false), SMS_TPARAM_INIT(mScaleMultiplier, 1.0f),
                  SMS_TPARAM_INIT(mThrowPowerMultiplier, 1.0f),
                  SMS_TPARAM_INIT(mUnderwaterHealthMultiplier, 1.0f),
                  SMS_TPARAM_INIT(mFallDamageMinMultiplier, 1.0f) {
                load("/Mario/better_sms.prm");
            }

            enum class FluddCleanType : u8 { NONE, CLEAN, GOOP };

            TParamRT<u8> mMaxHealth;
            TParamRT<bool> mCanRideYoshi;
            TParamRT<bool> mCanUseFludd;
            TParamRT<bool> mPlayerHasHelmet;
            TParamRT<bool> mPlayerHasGlasses;
            TParamRT<bool> mPlayerHasShirt;
            TParamRT<f32> mScaleMultiplier;
            TParamRT<f32> mThrowPowerMultiplier;
            TParamRT<f32> mUnderwaterHealthMultiplier;
            TParamRT<f32> mFallDamageMinMultiplier;
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
            bool getCanSprayFludd() const { return mCanSprayFludd; }
            bool getCanUseFludd() const { return mCanUseFludd; }
            const TPlayerParams *getParams() const { return mParams; }
            TMario *getPlayer() const { return mPlayer; }
            u32 getPlayerID() const { return mPlayerID; }

            bool getColliding() { return mCollisionFlags.mIsColliding; }
            bool isMario() const { return !mIsEMario; }

            void setCamera(CPolarSubCamera *camera);
            void setCanSprayFludd(bool enable) { mCanSprayFludd = enable; }
            void setCanUseFludd(bool enable) { mCanUseFludd = enable; }
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
            bool mCanSprayFludd;
            u32 mPlayerID;

        public:
            u8 mCurJump;
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