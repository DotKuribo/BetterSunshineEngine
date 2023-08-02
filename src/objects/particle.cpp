#include <Dolphin/stdlib.h>
#include <Dolphin/string.h>
#include <JSystem/JParticle/JPAResourceManager.hxx>
#include <SMS/Camera/PolarSubCamera.hxx>
#include <SMS/Manager/MarioParticleManager.hxx>
#include <SMS/MarioUtil/MathUtil.hxx>
#include <SMS/rand.h>
#include <SMS/raw_fn.hxx>

#include "libs/boundbox.hxx"
#include "libs/constmath.hxx"
#include "objects/particle.hxx"

constexpr float BinEditorScale = 130.0f;

void TParticleBox::load(JSUMemoryInputStream &in) {
    JDrama::TActor::load(in);

    mParticleName = in.readString();
    in.readData(&mID, 4);
    in.readData(&mSpawnRate, 4);
    in.readData(&mSpawnScale, 4);
    in.readData(&mShape, 1);
    in.readData(&mIsStrict, 1);

    if (strcmp(mParticleName, "((null))") == 0) {
        mParticleName = nullptr;
        return;
    }

    char mParticlePath[64];
    snprintf(mParticlePath, 64, "/scene/envjpa/%s.jpa", mParticleName);

    gpResourceManager->load(mParticlePath, mID);
}

void TParticleBox::perform(u32 flags, JDrama::TGraphics *) {
    if ((flags & 2)) {
        if (mID != -1) {
            auto bb = BoundingBox(mTranslation, mScale, mRotation);
            if (mIsStrict && !bb.contains(gpCamera->mTranslation, BinEditorScale, mShape)) {
                return;
            }
            if (mSpawnTimer++ >= mSpawnRate) {
                TVec3f position = getRandomParticlePosition(bb);
                auto *particle  = gpMarioParticleManager->emit(mID, &position, 1, this);
                if (particle) {
                    particle->mSize1 = {mSpawnScale, mSpawnScale, mSpawnScale};
                    particle->mSize3 = {mSpawnScale, mSpawnScale, mSpawnScale};
                }
                mSpawnTimer = 0;
            }
        }
    }
}

TVec3f TParticleBox::getRandomParticlePosition(const BoundingBox &bb) const {
    // Get random sample
    f32 sampleX = rand() / 32768.0f;
    f32 sampleY = rand() / 32768.0f;
    f32 sampleZ = rand() / 32768.0f;

    return bb.sample(sampleX, sampleY, sampleZ, BinEditorScale, mShape);
}
