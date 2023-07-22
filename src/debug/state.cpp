#include "module.hxx"

u32 XYZState     = 0xF000FFFF;
int gDebugUIPage = 1;
int gDebugState  = 0;

extern int gDebugState;
extern int gDebugUIPage;
extern bool gIsFastMovement;
extern bool gIsDebugActive;
extern int gMarioAnimation;
extern bool gIsDebugActive;
extern int gAnimationID;
extern float gAnimationSpeed;
extern bool gIsCameraTracking;
extern bool gIsCameraFrozen;
extern TVec3f gCamPosition, gCamRotation;
extern float gCamFOV;

BETTER_SMS_FOR_CALLBACK void resetDebugState(TMarDirector *director) {
    gCamPosition      = {0, 0, 0};
    gCamRotation      = {0, 0, 0};
    gCamFOV           = 70.0f;
    gMarioAnimation   = 0;
    gIsCameraFrozen   = false;
    gIsCameraTracking = false;
    gIsDebugActive    = false;
    gAnimationID      = 0;
    gAnimationSpeed   = 1.0f;
    gDebugState       = 0;
}