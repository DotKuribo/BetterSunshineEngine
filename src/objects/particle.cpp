#include <Dolphin/stdlib.h>
#include <Dolphin/string.h>
#include <JSystem/JParticle/JPAResourceManager.hxx>
#include <SMS/Camera/PolarSubCamera.hxx>
#include <SMS/Enemy/Conductor.hxx>
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

    if (strcmp(mParticleName, "(null)") == 0) {
        mParticleName = nullptr;
        return;
    }

    char mParticlePath[64];
    snprintf(mParticlePath, 64, "/scene/envjpa/%s.jpa", mParticleName);

    gpResourceManager->load(mParticlePath, mID);

    mGraphName = "(null)";

    s32 pos = in.getPosition();
    if (pos < in.getLength()) {
        mGraphName = in.readString();
    }

    mGraphTracer         = new TGraphTracer();
    mGraphTracer->mGraph = gpConductor->getGraphByName(mGraphName);

    if (mGraphTracer->mGraph) {
        int index = mGraphTracer->mGraph->findNearestNodeIndex(mTranslation, -1);
        mGraphTracer->setTo(index);

        TVec3f nodePos  = mGraphTracer->mGraph->indexToPoint(index);
        mDistanceToNext = PSVECDistance(nodePos, mTranslation) / mTravelSpeed;
        mPathDistance   = mDistanceToNext;

        TGraphNode &graphNextNode = mGraphTracer->mGraph->mNodes[index];
        TRailNode *railNextNode   = graphNextNode.mRailNode;

        if (railNextNode->mValues[0] != 0xFFFF) {
            mTravelSpeed = static_cast<f32>(railNextNode->mValues[0]) / 100.0f;
        } else {
            mTravelSpeed = 10.0f;
        }
    }
}

void TParticleBox::loadAfter() {}

void TParticleBox::perform(u32 flags, JDrama::TGraphics *) {
    if ((flags & 1)) {
        control();
    }

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
                    f32 scale        = mSpawnScale * mCurScale;
                    particle->mSize1 = {scale, scale, scale};
                    particle->mSize3 = {scale, scale, scale};
                }
                mSpawnTimer = 0;
            }
        }
    }
}

void TParticleBox::control() {
    TGraphWeb *graph = mGraphTracer->mGraph;
    if (!graph || graph->isDummy() || !mIsMoving) {
        return;
    }

    TVec3f nodePos;
    if (moveToNextNode(mTravelSpeed)) {
        {
            int nextIndex = graph->getShortestNextIndex(mGraphTracer->mCurrentNode,
                                                        mGraphTracer->mPreviousNode, -1);
            mGraphTracer->moveTo(nextIndex);
        }

        readRailFlag();

        nodePos         = graph->indexToPoint(mGraphTracer->mCurrentNode);
        mDistanceToNext = PSVECDistance(nodePos, mTranslation) / mTravelSpeed;
        mPathDistance   = mDistanceToNext;
    } else {
        nodePos = graph->indexToPoint(mGraphTracer->mCurrentNode);
    }

    f32 distanceLerp = 1.0f - (static_cast<f32>(mDistanceToNext) / static_cast<f32>(mPathDistance));

    mCurScale = lerp<f32>(mPrevScale, mNextScale, distanceLerp);
}

bool TParticleBox::moveToNextNode(f32 speed) {
    if (!mGraphTracer->mGraph || mGraphTracer->mGraph->isDummy()) {
        return false;
    }

    if (mGraphTracer->mGraph->mSplineRail) {
        f32 spline_speed = mGraphTracer->calcSplineSpeed(speed);
        bool traced      = mGraphTracer->traceSpline(spline_speed);

        mGraphTracer->mGraph->mSplineRail->getPosAndRot(mGraphTracer->_02, &mTranslation,
                                                        &mRotation);

        if (traced) {
            // readRailFlag();
        }

        mDistanceToNext = Max(0, mDistanceToNext - 1);

        return traced;
    } else {
        TVec3f point = mGraphTracer->mGraph->indexToPoint(mGraphTracer->mCurrentNode);
        TVec3f diff  = point - mTranslation;

        if (PSVECDotProduct(diff, diff) < 2.0f * (speed * speed)) {
            // readRailFlag();
            mGraphTracer->mGraph->mNodes[mGraphTracer->mCurrentNode].getPoint(mTranslation);
            return true;
        } else {
            mDistanceToNext = Max(0, mDistanceToNext - 1);
        }

        TVec3f direction;
        PSVECNormalize(diff, direction);
        direction.scale(speed);

        mTranslation += direction;
        return false;
    }
}

void TParticleBox::readRailFlag() {
    auto currentIndex = mGraphTracer->mCurrentNode;
    auto prevIndex    = mGraphTracer->mPreviousNode;

    TGraphWeb *graph = mGraphTracer->mGraph;
    if (graph->isDummy())
        return;

    if (prevIndex == -1)
        mPrevScale = 1.0f;
    else {
        TGraphNode &graphNode = graph->mNodes[prevIndex];
        TRailNode *railNode   = graphNode.mRailNode;

        if (railNode->mValues[1] != 0xFFFF) {
            mPrevScale = static_cast<f32>(railNode->mValues[1]) / 100.0f;
        } else {
            mPrevScale = 1.0f;
        }

        if ((railNode->mFlags & 8) != 0) {
            mIsMoving = false;
        }
    }

    if (currentIndex == -1) {
        mNextScale = mPrevScale;
    } else {
        TGraphNode &graphNextNode = graph->mNodes[currentIndex];
        TRailNode *railNextNode   = graphNextNode.mRailNode;

        if (railNextNode->mValues[1] != 0xFFFF) {
            mNextScale = static_cast<f32>(railNextNode->mValues[1]) / 100.0f;
        } else {
            mNextScale = 1.0f;
        }

        if (railNextNode->mValues[0] != 0xFFFF) {
            mTravelSpeed = static_cast<f32>(railNextNode->mValues[0]) / 100.0f;
        } else {
            mTravelSpeed = 10.0f;
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
