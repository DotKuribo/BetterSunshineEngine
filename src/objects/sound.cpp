#include <Dolphin/stdlib.h>
#include <Dolphin/string.h>
#include <JSystem/JParticle/JPAResourceManager.hxx>
#include <SMS/Camera/PolarSubCamera.hxx>
#include <SMS/MSound/MSound.hxx>
#include <SMS/MSound/MSoundSESystem.hxx>
#include <SMS/MarioUtil/MathUtil.hxx>
#include <SMS/rand.h>
#include <SMS/raw_fn.hxx>

#include "libs/boundbox.hxx"
#include "libs/constmath.hxx"
#include "objects/sound.hxx"

constexpr float BinEditorScale = 130.0f;

void TSoundBox::load(JSUMemoryInputStream &in) {
    JDrama::TActor::load(in);

    in.readData(&mID, 4);
    in.readData(&mVolume, 4);
    in.readData(&mPitch, 4);
    in.readData(&mSpawnRate, 4);
    in.readData(&mShape, 1);
}

void TSoundBox::perform(u32 flags, JDrama::TGraphics *) {
    if ((flags & 2)) {
        if (mID != -1) {
            auto bb = BoundingBox(mTranslation, mScale, mRotation);
            if (mSpawnTimer++ >= mSpawnRate) {
                mSoundPos = getRandomParticlePosition(bb);
                if (gpMSound->gateCheck(mID)) {
                    auto *sound = MSoundSE::startSoundActor(mID, mSoundPos, 0, nullptr, 0, 4);
                    if (sound) {
                        sound->setVolume(mVolume, 0, 0);
                        sound->setPitch(mPitch, 0, 0);
                    }
                }
                mSpawnTimer = 0;
            }
        }
    }
}

TVec3f TSoundBox::getRandomParticlePosition(const BoundingBox &bb) const {
    // Get random sample
    f32 sampleX = rand() / 32768.0f;
    f32 sampleY = rand() / 32768.0f;
    f32 sampleZ = rand() / 32768.0f;

    return bb.sample(sampleX, sampleY, sampleZ, BinEditorScale, mShape);
}
