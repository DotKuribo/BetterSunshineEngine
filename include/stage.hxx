#pragma once

#include <JSystem/J2D/J2DOrthoGraph.hxx>
#include <JSystem/JDrama/JDRNameRef.hxx>
#include <SMS/game/Application.hxx>
#include <SMS/game/MarDirector.hxx>
#include <SMS/mapobj/MapObjInit.hxx>
#include <SMS/params/Params.hxx>

namespace BetterSMS {
    namespace Stage {
        typedef void (*InitCallback)(TMarDirector *);
        typedef void (*UpdateCallback)(TMarDirector *);
        typedef void (*Draw2DCallback)(TMarDirector *, J2DOrthoGraph *);
        typedef void (*ExitCallback)(TApplication *);

        struct TStageParams;
        TStageParams *getStageConfiguration();

        const char *getStageName(TApplication *application);
        bool isStageInitRegistered(const char *name);
        bool isStageUpdateRegistered(const char *name);
        bool isDraw2DRegistered(const char *name);
        bool registerInitCallback(const char *name, InitCallback cb);
        bool registerUpdateCallback(const char *name, UpdateCallback cb);
        bool registerDraw2DCallback(const char *name, Draw2DCallback cb);
        bool registerExitCallback(const char *name, ExitCallback cb);
        bool deregisterInitCallback(const char *name);
        bool deregisterUpdateCallback(const char *name);
        bool deregisterDraw2DCallback(const char *name);
        bool deregisterExitCallback(const char *name);

#pragma region ConfigImplementation
        struct TStageParams : public TParams {

            TStageParams(const char *prm)
                : TParams(), SMS_TPARAM_INIT(mIsExStage, false),
                  SMS_TPARAM_INIT(mIsDivingStage, false), SMS_TPARAM_INIT(mIsOptionStage, false),
                  SMS_TPARAM_INIT(mIsMultiplayerStage, false), SMS_TPARAM_INIT(mIsEggFree, true),
                  SMS_TPARAM_INIT(mPlayerHealth, 8), SMS_TPARAM_INIT(mPlayerMaxHealth, 8),
                  SMS_TPARAM_INIT(mPlayerHasFludd, true), SMS_TPARAM_INIT(mPlayerHasHelmet, false),
                  SMS_TPARAM_INIT(mPlayerHasGlasses, false),
                  SMS_TPARAM_INIT(mPlayerHasShirt, false),
                  SMS_TPARAM_INIT(mPlayerCanRideYoshi, true),
                  SMS_TPARAM_INIT(mPlayerSizeMultiplier, 1.0f), SMS_TPARAM_INIT(mFluddPrimary, 0),
                  SMS_TPARAM_INIT(mFluddSecondary, 4),
                  SMS_TPARAM_INIT(mFluddWaterColor, JUtility::TColor(60, 70, 120, 20)),
                  SMS_TPARAM_INIT(mFluddShouldColorWater, false),
                  SMS_TPARAM_INIT(mMusicVolume, 0.75f), SMS_TPARAM_INIT(mMusicSpeed, 1.0f),
                  SMS_TPARAM_INIT(mMusicPitch, 1.0f), SMS_TPARAM_INIT(mMusicID, 1),
                  SMS_TPARAM_INIT(mMusicAreaID, 1), SMS_TPARAM_INIT(mMusicEpisodeID, 0),
                  SMS_TPARAM_INIT(mMusicEnabled, true), SMS_TPARAM_INIT(mMusicSetCustom, false),
                  SMS_TPARAM_INIT(mGravityMultiplier, 1.0f) {
                delete sStageConfig;
                sStageConfig = this;

                if (prm)
                    load(prm);
            }

            ~TStageParams() {
                if (this == sStageConfig)
                    sStageConfig = nullptr;
            }

            static TStageParams *sStageConfig;

            static char *stageNameToParamPath(char *dst, const char *stage,
                                              bool generalize = false);

            bool isCustomConfig() const { return mIsCustomConfigLoaded; }
            void load(const char *stageName);
            void reset();

            // Stage Info
            TParamT<bool> mIsExStage;
            TParamT<bool> mIsDivingStage;
            TParamT<bool> mIsOptionStage;
            TParamT<bool> mIsMultiplayerStage;
            TParamT<bool> mIsEggFree;

            // Player Info
            TParamT<u16> mPlayerHealth;
            TParamT<u16> mPlayerMaxHealth;
            TParamT<bool> mPlayerHasFludd;
            TParamT<bool> mPlayerHasHelmet;
            TParamT<bool> mPlayerHasGlasses;
            TParamT<bool> mPlayerHasShirt;
            TParamT<bool> mPlayerCanRideYoshi;
            TParamT<f32> mPlayerSizeMultiplier;

            // Fludd Info
            TParamT<u8> mFluddPrimary;
            TParamT<u8> mFluddSecondary;
            TParamT<JUtility::TColor> mFluddWaterColor;
            TParamT<bool> mFluddShouldColorWater;

            // Music Info
            TParamT<f32> mMusicVolume;
            TParamT<f32> mMusicSpeed;
            TParamT<f32> mMusicPitch;
            TParamT<u16> mMusicID;
            TParamT<u8> mMusicAreaID;
            TParamT<u8> mMusicEpisodeID;
            TParamT<bool> mMusicEnabled;
            TParamT<bool> mMusicSetCustom;

            // Global Info
            TParamT<f32> mGravityMultiplier;

        private:
            bool mIsCustomConfigLoaded;
        };
#pragma endregion
    }  // namespace Stage
}  // namespace BetterSMS