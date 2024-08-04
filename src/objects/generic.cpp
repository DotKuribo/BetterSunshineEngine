#include <Dolphin/GX.h>
#include <Dolphin/MTX.h>
#include <Dolphin/types.h>

#include <JSystem/J3D/J3DModel.hxx>
#include <JSystem/J3D/J3DModelLoaderDataBase.hxx>
#include <SMS/Camera/PolarSubCamera.hxx>
#include <SMS/Enemy/Conductor.hxx>
#include <SMS/M3DUtil/MActor.hxx>
#include <SMS/M3DUtil/MActorKeeper.hxx>
#include <SMS/MSound/MSoundSESystem.hxx>
#include <SMS/Manager/ModelWaterManager.hxx>
#include <SMS/Map/BGCheck.hxx>
#include <SMS/Map/MapCollisionData.hxx>
#include <SMS/MapObj/MapObjBall.hxx>
#include <SMS/MapObj/MapObjInit.hxx>
#include <SMS/MarioUtil/ShadowUtil.hxx>
#include <SMS/Player/Mario.hxx>
#include <SMS/Player/Watergun.hxx>
#include <SMS/raw_fn.hxx>

#include "libs/constmath.hxx"
#include "objects/generic.hxx"

static void clampRotation(TVec3f &rotation) {
    auto clampPreserve = [](f32 degrees) {
        if (degrees > 360.0f)
            degrees -= 360.0f;
        else if (degrees < -360.0f)
            degrees += 360.0f;
        return degrees;
    };

    rotation.x = clampPreserve(rotation.x);
    rotation.y = clampPreserve(rotation.y);
    rotation.z = clampPreserve(rotation.z);
}

void TGenericRailObj::load(JSUMemoryInputStream &in) {
    JDrama::TActor::load(in);

    mRegisterName = "GenericRailObj";
    mModelName    = in.readString();

    char railNameBuf[256];
    in.readString(railNameBuf, 256);

    in.readData(&mBaseRotation.x, 4);
    in.readData(&mBaseRotation.y, 4);
    in.readData(&mBaseRotation.z, 4);
    in.readData(&mFrameRate, 4);
    in.readData(&mSoundID, 4);
    in.readData(&mSoundSpeed, 4);
    in.readData(&mSoundStrength, 4);
    in.skip(3);
    in.readData(&mContactAnim, 1);
    in.skip(3);
    in.readData(&mCullModel, 1);

    mSoundCounter = mSoundSpeed;

    mInitialPosition = mTranslation;
    mInitialRotation = mRotation;

    {
        TGraphWeb *web = gpConductor->getGraphByName(railNameBuf);
        initGraphTracer(web);
    }

    initMapObj();
    makeObjAppeared();
}

void TGenericRailObj::makeMActors() {
    mActorKeeper = new TMActorKeeper(mLiveManager, 1);
    if (mModelLoadFlags & 0x8000)
        mActorKeeper->mModelFlags = 0x11220000;
    else
        mActorKeeper->mModelFlags = 0x10220000;

    char modelPath[128];
    snprintf(modelPath, 128, "%s.bmd", mModelName);

    mActorData = mActorKeeper->createMActor(modelPath, getSDLModelFlag());

    if (mModelLoadFlags & 0x4000) {
        mActorData->setLightID(0);
        mActorData->_40 = 0;
    }

    calcRootMatrix();
    mActorData->calc();
    mActorData->viewCalc();
}

void TGenericRailObj::initMapCollisionData() {
    char colPath[128];
    snprintf(colPath, 128, "/scene/MapObj/%s.col", mModelName);

    if (!JKRArchive::getGlbResource(colPath))
        return;

    mCollisionManager = new TMapCollisionManager(1, "mapObj", this);
    mCollisionManager->init(mModelName, 1, nullptr);
}

void TGenericRailObj::initMapObj() {
    TRailMapObj::initMapObj();
    // mActorData->setLightType(2);

    if (mCollisionManager) {
        mCollisionManager->changeCollision(0);
        mCollisionManager->mCurrentMapCollision->setAllActor(this);
    }

    playAnimations(J3DFrameCtrl::LOOP);
}

void TGenericRailObj::control() {
    TMapObjBase::control();

    if (mCullModel)
        mStateFlags.asU32 |= 0x100;
    else
        mStateFlags.asU32 &= ~0x100;

    if (checkMarioRiding()) {
        if (mContactAnim > 0) {
            auto *frameCtrl = mActorData->getFrameCtrl(MActor::BCK);
            if (frameCtrl)
                frameCtrl->mFrameRate = mFrameRate * (SMSGetAnmFrameRate() / 2);
        }
    } else {
        if (mContactAnim == 1) {
            auto *frameCtrl = mActorData->getFrameCtrl(MActor::BCK);
            if (frameCtrl)
                frameCtrl->mFrameRate = -mFrameRate * (SMSGetAnmFrameRate() / 2);
        } else if (mContactAnim == 2) {
            auto *frameCtrl = mActorData->getFrameCtrl(MActor::BCK);
            if (frameCtrl)
                frameCtrl->mAnimState = J3DFrameCtrl::ONCE_RESET;
        }
    }

    if (++mSoundCounter >= mSoundSpeed) {
        if (mSoundID != 0xFFFFFFFF && gpMSound->gateCheck(mSoundID))
            mCurrentSound =
                MSoundSESystem::MSoundSE::startSoundActor(mSoundID, mTranslation, 0, nullptr, 0, 4);
        mSoundCounter = 0;
    }

    if (mCurrentSound) {
        // TODO: add when exp2f is resolved
        //
        // mCurrentSound->setVolume(powf(0.01f, PSVECDistance(mTranslation, gpCamera->mTranslation)
        // /
        //                                          (8000.0f * mSoundStrength)),
        //                          0, 0);

        mCurrentSound->setVolume(
            lerp<f32>(1.0f, 0.0f,
                      clamp(PSVECDistance(mTranslation, gpCamera->mTranslation) /
                                (8000.0f * mSoundStrength),
                            0.0f, 1.0f)),
            0, 0);
    }

    TGraphWeb *graph = mGraphTracer->mGraph;
    if (graph->isDummy()) {
        mRotation.add(mBaseRotation);
        return;
    }

    TVec3f nodePos;
    if (moveToNextNode(mTravelSpeed)) {
        readRailFlag();
        {
            int nextIndex = graph->getShortestNextIndex(mGraphTracer->mCurrentNode,
                                                         mGraphTracer->mPreviousNode, -1);
            mGraphTracer->moveTo(nextIndex);
        }

        TRailNode *node = graph->mNodes[mGraphTracer->mCurrentNode].mRailNode;
        if (node->mValues[0] != 0xFFFF)
            mTravelSpeed = static_cast<f32>(node->mValues[0]) / 100.0f;

        nodePos         = graph->indexToPoint(mGraphTracer->mCurrentNode);
        mDistanceToNext = PSVECDistance(nodePos, mTranslation) / mTravelSpeed;
        mPathDistance   = mDistanceToNext;
    } else {
        nodePos = graph->indexToPoint(mGraphTracer->mCurrentNode);
    }

    f32 distanceLerp = 1.0f - (static_cast<f32>(mDistanceToNext) / static_cast<f32>(mPathDistance));

    if (graph->mNodes[mGraphTracer->mCurrentNode].mRailNode->mFlags & 0x1000) {
        if (distanceLerp > 1.0f - (mTravelSpeed / static_cast<f32>(mPathDistance))) {
            resetPosition();

            mGroundY =
                gpMapCollisionData->checkGround(mTranslation.x, mTranslation.y, mTranslation.z, 0,
                                                const_cast<const TBGCheckData **>(&mFloorBelow));

            return;
        }
    }

    OSReport("Next: %f, Length: %f\n", (f32)mDistanceToNext, (f32)mPathDistance);

    mRotation = {
        lerp<f32>(mCurrentNodeRotation.x, mTargetNodeRotation.x,
                  1.0f - (static_cast<f32>(mDistanceToNext) / static_cast<f32>(mPathDistance))),
        lerp<f32>(mCurrentNodeRotation.y, mTargetNodeRotation.y,
                  1.0f - (static_cast<f32>(mDistanceToNext) / static_cast<f32>(mPathDistance))),
        lerp<f32>(mCurrentNodeRotation.z, mTargetNodeRotation.z,
                  1.0f - (static_cast<f32>(mDistanceToNext) / static_cast<f32>(mPathDistance))),
    };

    mRotation.add(mCurrentRotation);
    mCurrentRotation.add(mBaseRotation);
    clampRotation(mCurrentRotation);

    mGroundY = gpMapCollisionData->checkGround(mTranslation.x, mTranslation.y, mTranslation.z, 0,
                                               const_cast<const TBGCheckData **>(&mFloorBelow));
}

void TGenericRailObj::setGroundCollision() {
    auto *model = getModel();
    if (mCollisionManager) {
        mCollisionManager->mCurrentMapCollision->_5C &= 0xFFFFFFFE;  // Enable
        mCollisionManager->mCurrentMapCollision->moveMtx(*model->mJointArray);
    }
}

void TGenericRailObj::readRailFlag() {
    auto currentIndex = mGraphTracer->mCurrentNode;
    auto prevIndex    = mGraphTracer->mPreviousNode;

    TGraphWeb *graph = mGraphTracer->mGraph;
    if (graph->isDummy())
        return;

    if (prevIndex == -1)
        mCurrentNodeRotation = mRotation;
    else {
        TGraphNode &graphNode = graph->mNodes[prevIndex];
        TRailNode *railNode   = graphNode.mRailNode;

        mCurrentNodeRotation = {convertAngleS16ToFloat(railNode->mValues[1]),
                                convertAngleS16ToFloat(railNode->mValues[2]),
                                convertAngleS16ToFloat(railNode->mValues[3])};
    }

    if (currentIndex == -1) {
        mTargetNodeRotation = mCurrentNodeRotation;
    } else {
        TGraphNode &graphNextNode = graph->mNodes[currentIndex];
        TRailNode *railNextNode   = graphNextNode.mRailNode;

        mTargetNodeRotation = {convertAngleS16ToFloat(railNextNode->mValues[1]),
                               convertAngleS16ToFloat(railNextNode->mValues[2]),
                               convertAngleS16ToFloat(railNextNode->mValues[3])};
    }
}

void TGenericRailObj::resetPosition() {
    mTranslation = mInitialPosition;
    mRotation    = mInitialRotation;
    mGraphTracer->setTo(mGraphTracer->mGraph->findNearestNodeIndex(mTranslation, -1));
    readRailFlag();
}

void TGenericRailObj::requestShadow() {
    /*TCircleShadowRequest request;
    request.mTranslation   = mTranslation;
    request.mTranslation.y = mGroundY;
    request.mOffsetY       = mShadowRadius;
    request.mOffsetY2      = mShadowRadius;
    request.mRotationY     = mRotation.y;
    request.mShadowType    = getShadowType();
    gpBindShadowManager->request(request, mObjectID);*/
}

u32 TGenericRailObj::getShadowType() { return 0; }

bool TGenericRailObj::checkMarioRiding() {
    if (gpMarioAddress->mFloorTriangle->mOwner != this)
        return false;

    return SMS_IsMarioTouchGround4cm__Fv();
}

void TGenericRailObj::playAnimations(s8 state) {
    if (mActorData->checkAnmFileExist(mModelName, MActor::BCK)) {
        mActorData->setBck(mModelName);
        auto *frameCtrl = mActorData->getFrameCtrl(MActor::BCK);
        if (frameCtrl) {
            frameCtrl->mAnimState = mContactAnim > 0 ? J3DFrameCtrl::ONCE : state;
            frameCtrl->mFrameRate = mContactAnim > 0 ? 0.0f
                                                     : mFrameRate * (SMSGetAnmFrameRate() / 2);
        }
    }

    if (mActorData->checkAnmFileExist(mModelName, MActor::BLK)) {
        mActorData->setBlk(mModelName);
        auto *frameCtrl = mActorData->getFrameCtrl(MActor::BLK);
        if (frameCtrl) {
            frameCtrl->mAnimState = state;
            frameCtrl->mFrameRate = mFrameRate * (SMSGetAnmFrameRate() / 2);
        }
    }

    if (mActorData->checkAnmFileExist(mModelName, MActor::BRK)) {
        mActorData->setBrk(mModelName);
        auto *frameCtrl = mActorData->getFrameCtrl(MActor::BRK);
        if (frameCtrl) {
            frameCtrl->mAnimState = state;
            frameCtrl->mFrameRate = mFrameRate * (SMSGetAnmFrameRate() / 2);
        }
    }

    if (mActorData->checkAnmFileExist(mModelName, MActor::BPK)) {
        mActorData->setBpk(mModelName);
        auto *frameCtrl = mActorData->getFrameCtrl(MActor::BPK);
        if (frameCtrl) {
            frameCtrl->mAnimState = state;
            frameCtrl->mFrameRate = mFrameRate * (SMSGetAnmFrameRate() / 2);
        }
    }

    if (mActorData->checkAnmFileExist(mModelName, MActor::BTP)) {
        mActorData->setBtp(mModelName);
        auto *frameCtrl = mActorData->getFrameCtrl(MActor::BTP);
        if (frameCtrl) {
            frameCtrl->mAnimState = state;
            frameCtrl->mFrameRate = mFrameRate * (SMSGetAnmFrameRate() / 2);
        }
    }

    if (mActorData->checkAnmFileExist(mModelName, MActor::BTK)) {
        mActorData->setBtk(mModelName);
        auto *frameCtrl = mActorData->getFrameCtrl(MActor::BTK);
        if (frameCtrl) {
            frameCtrl->mAnimState = state;
            frameCtrl->mFrameRate = mFrameRate * (SMSGetAnmFrameRate() / 2);
        }
    }
}

void TGenericRailObj::stopAnimations() {
    J3DFrameCtrl *frameCtrl = mActorData->getFrameCtrl(MActor::BCK);
    if (frameCtrl)
        frameCtrl->mAnimState = J3DFrameCtrl::ONCE_RESET;

    frameCtrl = mActorData->getFrameCtrl(MActor::BLK);
    if (frameCtrl)
        frameCtrl->mAnimState = J3DFrameCtrl::ONCE_RESET;

    frameCtrl = mActorData->getFrameCtrl(MActor::BRK);
    if (frameCtrl)
        frameCtrl->mAnimState = J3DFrameCtrl::ONCE_RESET;

    frameCtrl = mActorData->getFrameCtrl(MActor::BPK);
    if (frameCtrl)
        frameCtrl->mAnimState = J3DFrameCtrl::ONCE_RESET;

    frameCtrl = mActorData->getFrameCtrl(MActor::BTP);
    if (frameCtrl)
        frameCtrl->mAnimState = J3DFrameCtrl::ONCE_RESET;

    frameCtrl = mActorData->getFrameCtrl(MActor::BTK);
    if (frameCtrl)
        frameCtrl->mAnimState = J3DFrameCtrl::ONCE_RESET;
}

ObjData generic_railobj_data{.mMdlName         = "GenericRailObj",
                             .mObjectID        = 0x40000F00,
                             .mLiveManagerName = reinterpret_cast<const char *>(
                                 gLiveManagerName),  // const_cast<char *>("木マネージャー")
                             .mObjKey = reinterpret_cast<const char *>(
                                 gUnkManagerName),  // const_cast<char *>("GenericRailObj"),
                             .mAnimInfo         = nullptr,
                             .mObjCollisionData = nullptr,
                             .mMapCollisionInfo = nullptr,
                             .mSoundInfo        = nullptr,
                             .mPhysicalInfo     = nullptr,
                             .mSinkData         = nullptr,
                             ._28               = nullptr,
                             .mBckMoveData      = nullptr,
                             ._30               = 50.0f,
                             .mUnkFlags         = 0x4 /*0x02130100*/,
                             .mKeyCode          = cexp_calcKeyCode("GenericRailObj")};