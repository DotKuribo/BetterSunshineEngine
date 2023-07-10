#pragma once

#include <Dolphin/OS.h>
#include <Dolphin/types.h>

#include <JSystem/JUtility/JUTTexture.hxx>
#include <JSystem/J2D/J2DPicture.hxx>


class SimpleTexAnimator {
public:
    SimpleTexAnimator()
        : mTextures(nullptr), mTexCount(0), mRotation(0.0f), mSpin(0.0f), mLastTime(0),
          mCurrentFrame(0.0f), mFrameRate(30.0f) {}
    SimpleTexAnimator(const ResTIMG **textures, size_t texCount)
        : mTextures(textures), mTexCount(texCount), mRotation(0.0f), mSpin(0.0f), mLastTime(0),
          mCurrentFrame(0.0f), mFrameRate(30.0f) {}
    SimpleTexAnimator(const u8 **textures, size_t texCount)
        : mTexCount(texCount), mRotation(0.0f), mSpin(0.0f), mLastTime(0),
          mCurrentFrame(0.0f), mFrameRate(30.0f) {
        mTextures = reinterpret_cast<const ResTIMG **>(textures);
    }
    ~SimpleTexAnimator() {}

    size_t getCurrentFrame() const { return static_cast<size_t>(mCurrentFrame); }
    size_t getFrameCount() const { return mTexCount; }
    f32 getRotation() const { return mRotation; }

    void setTextures(const ResTIMG **textures, size_t texCount) {
        mTextures     = textures;
        mTexCount     = texCount;
        mCurrentFrame = texCount;
    }
    void setCurrentFrame(u32 frame) { mCurrentFrame = static_cast<f32>(frame); }
    void setFrameRate(f32 rate) { mFrameRate = rate; }
    void setRotation(f32 rotation) { mRotation = rotation; }
    void setSpin(f32 degreesPerFrame) { mSpin = degreesPerFrame; };

    void process(J2DPicture *picture) {
        if (mLastTime == 0)
            mLastTime = OSTicksToSeconds(f64(OSGetTime()));

        f64 currentTime = OSTicksToSeconds(f64(OSGetTime()));
        f64 diff        = currentTime - mLastTime;
        mLastTime       = currentTime;

        mCurrentFrame += diff * mFrameRate;
        mRotation += diff * mFrameRate * mSpin;

        if (mCurrentFrame >= mTexCount)
            mCurrentFrame -= mTexCount - 1.0;

        picture->changeTexture(mTextures[static_cast<size_t>(mCurrentFrame) % mTexCount], 0);
        picture->mRotation = mRotation;
    }

    void resetAnimation() {
        mRotation     = 0.0f;
        mLastTime     = 0;
        mCurrentFrame = 0;
    }

private:
    const ResTIMG **mTextures;
    size_t mTexCount;
    f32 mRotation;
    f32 mSpin;
    f64 mLastTime;
    f32 mCurrentFrame;
    f32 mFrameRate;
};