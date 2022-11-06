#include <Dolphin/types.h>
#include <JSystem/JAudio/JAISound.hxx>
#include <SMS/MapObj/MapObjRail.hxx>

class TGenericRailObj : public TRailMapObj {
public:
    static JDrama::TNameRef *instantiate() { return new TGenericRailObj("GenericRailObj"); }

    TGenericRailObj(const char *name)
        : TRailMapObj(name), mSoundID(-1), mSoundStrength(1.0f), mContactAnim(false),
          mModelName(nullptr), mCurrentSound(nullptr) {}
    virtual ~TGenericRailObj() override {}

    virtual void load(JSUMemoryInputStream &in) override;
    virtual void initMapCollisionData() override;
    virtual void initMapObj() override;
    virtual void makeMActors() override;
    virtual void control() override;
    virtual void setGroundCollision() override;
    virtual void readRailFlag() override;
    virtual void resetPosition() override;

    bool checkMarioRiding();
    void playAnimations(s8 state);
    void stopAnimations();

    const char *mModelName;
    bool mContactAnim;
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