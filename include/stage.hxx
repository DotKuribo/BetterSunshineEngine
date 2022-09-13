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

        struct TStageParams;
        TStageParams *getStageConfiguration();

        const char *getStageName(TApplication *application);
        bool isStageInitRegistered(const char *name);
        bool isStageUpdateRegistered(const char *name);
        bool isDraw2DRegistered(const char *name);
        bool registerInitCallback(const char *name, InitCallback cb);
        bool registerUpdateCallback(const char *name, UpdateCallback cb);
        bool registerDraw2DCallback(const char *name, Draw2DCallback cb);
        bool deregisterInitCallback(const char *name);
        bool deregisterUpdateCallback(const char *name);
        bool deregisterDraw2DCallback(const char *name);

#pragma region ConfigImplementation
        struct TStageParams : public TParams {
#define CONSTRUCT_PARAM(name, val)                                                                 \
    name(this, val, JDrama::TNameRef::calcKeyCode(SMS_STRINGIZE(name)), SMS_STRINGIZE(name))

            TStageParams(const char *prm)
                : TParams(), CONSTRUCT_PARAM(mIsExStage, false),
                  CONSTRUCT_PARAM(mIsDivingStage, false), CONSTRUCT_PARAM(mIsOptionStage, false),
                  CONSTRUCT_PARAM(mIsMultiplayerStage, false),
                  CONSTRUCT_PARAM(mIsYoshiHungry, false), CONSTRUCT_PARAM(mIsEggFree, true),
                  CONSTRUCT_PARAM(mPlayerHealth, 8), CONSTRUCT_PARAM(mPlayerMaxHealth, 8),
                  CONSTRUCT_PARAM(mPlayerHasFludd, true), CONSTRUCT_PARAM(mPlayerHasHelmet, false),
                  CONSTRUCT_PARAM(mPlayerHasGlasses, false),
                  CONSTRUCT_PARAM(mPlayerHasShirt, false),
                  CONSTRUCT_PARAM(mPlayerCanRideYoshi, true),
                  CONSTRUCT_PARAM(mPlayerSizeMultiplier, 1.0f), CONSTRUCT_PARAM(mFluddPrimary, 0),
                  CONSTRUCT_PARAM(mFluddSecondary, 4),
                  CONSTRUCT_PARAM(mFluddWaterColor, JUtility::TColor(60, 70, 120, 20)),
                  CONSTRUCT_PARAM(mFluddShouldColorWater, false),
                  CONSTRUCT_PARAM(mMusicVolume, 0.75f), CONSTRUCT_PARAM(mMusicSpeed, 1.0f),
                  CONSTRUCT_PARAM(mMusicPitch, 1.0f), CONSTRUCT_PARAM(mMusicID, 1),
                  CONSTRUCT_PARAM(mMusicAreaID, 1), CONSTRUCT_PARAM(mMusicEpisodeID, 0),
                  CONSTRUCT_PARAM(mMusicEnabled, true), CONSTRUCT_PARAM(mMusicSetCustom, false),
                  CONSTRUCT_PARAM(mGravityMultiplier, 1.0f) {
                delete sStageConfig;
                sStageConfig = this;

                if (prm)
                    load(prm);
            }

#undef CONSTRUCT_PARAM
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
            TParamT<bool> mIsYoshiHungry;
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