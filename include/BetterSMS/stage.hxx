#pragma once

#include <JSystem/J2D/J2DOrthoGraph.hxx>
#include <JSystem/JDrama/JDRNameRef.hxx>
#include <SMS/MapObj/MapObjInit.hxx>
#include <SMS/System/Application.hxx>
#include <SMS/System/MarDirector.hxx>
#include <SMS/System/Params.hxx>

namespace BetterSMS {
    namespace Stage {
        typedef void (*InitCallback)(TMarDirector *);
        typedef void (*UpdateCallback)(TMarDirector *);
        typedef void (*Draw2DCallback)(TMarDirector *, const J2DOrthoGraph *);
        typedef void (*ExitCallback)(TApplication *);

        struct TStageParams;
        TStageParams *getStageConfiguration();

        bool addInitCallback(InitCallback cb);
        bool addUpdateCallback(UpdateCallback cb);
        bool addDraw2DCallback(Draw2DCallback cb);
        bool addExitCallback(ExitCallback cb);

        const char *getStageName(u8 area, u8 episode);
        bool isDivingStage(u8 area, u8 episode);
        bool isExStage(u8 area, u8 episode);

#pragma region ConfigImplementation
        struct TStageParams : public TParams {

            TStageParams()
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
                  SMS_TPARAM_INIT(mStreamLoopStart, -1),
                  SMS_TPARAM_INIT(mStreamLoopEnd, -1),
                  SMS_TPARAM_INIT(mGravityMultiplier, 1.0f) {}

            TStageParams(const char *prm) : TStageParams() {
                if (prm)
                    load(prm);
            }

            ~TStageParams() {
                if (this == sStageConfig)
                    sStageConfig = nullptr;
            }

            static TStageParams *sStageConfig;

            static char *stageNameToParamPath(char *dst, const char *stage, bool global = false);

            bool isCustomConfig() const { return mIsCustomConfigLoaded; }
            void load(const char *stageName);
            void reset();

            // Stage Info
            TParamRT<bool> mIsExStage;
            TParamRT<bool> mIsDivingStage;
            TParamRT<bool> mIsOptionStage;
            TParamRT<bool> mIsMultiplayerStage;
            TParamRT<bool> mIsEggFree;

            // Player Info
            TParamRT<u16> mPlayerHealth;
            TParamRT<u16> mPlayerMaxHealth;
            TParamRT<bool> mPlayerHasFludd;
            TParamRT<bool> mPlayerHasHelmet;
            TParamRT<bool> mPlayerHasGlasses;
            TParamRT<bool> mPlayerHasShirt;
            TParamRT<bool> mPlayerCanRideYoshi;
            TParamRT<f32> mPlayerSizeMultiplier;

            // Fludd Info
            TParamRT<u8> mFluddPrimary;
            TParamRT<u8> mFluddSecondary;
            TParamRT<JUtility::TColor> mFluddWaterColor;
            TParamRT<bool> mFluddShouldColorWater;

            // Music Info
            TParamRT<f32> mMusicVolume;
            TParamRT<f32> mMusicSpeed;
            TParamRT<f32> mMusicPitch;
            TParamRT<u16> mMusicID;
            TParamRT<u8> mMusicAreaID;
            TParamRT<u8> mMusicEpisodeID;
            TParamRT<bool> mMusicEnabled;
            TParamRT<bool> mMusicSetCustom;
            TParamRT<s32> mStreamLoopStart;
            TParamRT<s32> mStreamLoopEnd;

            // Global Info
            TParamRT<f32> mGravityMultiplier;

        private:
            bool mIsCustomConfigLoaded;
        };
#pragma endregion
    }  // namespace Stage
}  // namespace BetterSMS