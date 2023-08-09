#include <Dolphin/types.h>
#include <JSystem/JAudio/JAISound.hxx>
#include <SMS/MapObj/MapObjRail.hxx>

#include "module.hxx"

class TGenericRailObj : public TRailMapObj {
public:
    BETTER_SMS_FOR_CALLBACK static JDrama::TNameRef *instantiate() {
        return new TGenericRailObj("GenericRailObj");
    }

    TGenericRailObj(const char *name)
        : TRailMapObj(name), mSoundID(-1), mSoundStrength(1.0f), mContactAnim(false),
          mModelName(nullptr), mCurrentSound(nullptr) {}
    ~TGenericRailObj() override {}

    void load(JSUMemoryInputStream &in) override;
    void initMapCollisionData() override;
    void initMapObj() override;
    void makeMActors() override;
    void control() override;
    void setGroundCollision() override;
    void readRailFlag() override;
    void resetPosition() override;
    void requestShadow() override;

    bool checkMarioRiding();
    void playAnimations(s8 state);
    void stopAnimations();

    const char *mModelName;
    u8 mContactAnim;
    bool mCullModel;
    f32 mFrameRate;
    u32 mSoundID;
    u32 mSoundSpeed;
    f32 mSoundStrength;
    TVec3f mBaseRotation;
    TVec3f mCurrentRotation;
    TVec3f mCurrentNodeRotation;
    TVec3f mTargetNodeRotation;
    s32 mPathDistance;
    u32 mSoundCounter;
    JAISound *mCurrentSound;
    bool mIsContactAnimPlayed;
};

extern ObjData generic_railobj_data;