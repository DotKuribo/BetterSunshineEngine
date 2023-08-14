#pragma once

#include <Dolphin/GX.h>
#include <Dolphin/types.h>

#include <JSystem/JAudio/JAISound.hxx>
#include <JSystem/JDrama/JDRActor.hxx>
#include <JSystem/JUtility/JUTColor.hxx>

#include "libs/boundbox.hxx"
#include "module.hxx"

class TSoundBox : public JDrama::TActor {
public:
    BETTER_SMS_FOR_CALLBACK static JDrama::TNameRef *instantiate() {
        return new TSoundBox("TSoundBox");
    }

    TSoundBox(const char *name)
        : TActor(name), mID(-1), mVolume(1.0f), mPitch(1.0f), mSpawnRate(1),
          mShape(BoundingType::Box), mSpawnTimer(), mSoundPos() {}
    virtual ~TSoundBox() override {}

    void load(JSUMemoryInputStream &in) override;
    void perform(u32 flags, JDrama::TGraphics *) override;

private:
    TVec3f getRandomParticlePosition(const BoundingBox &bb) const;

    // Bin parameters
    s32 mID;
    f32 mVolume;
    f32 mPitch;
    s32 mSpawnRate;
    BoundingType mShape;

    // Game state
    s32 mSpawnTimer;
    TVec3f mSoundPos;
};
