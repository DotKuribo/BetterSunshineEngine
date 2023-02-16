
#include "module.hxx"

float Hx_MotionUpdate(float* hxMotionStruct) {
	float scaleFactor = 1.0f / (60.0f / BetterSMS::getFrameRate());

	if(hxMotionStruct[0] <= hxMotionStruct[7]) {
	    if(hxMotionStruct[1] < hxMotionStruct[7]) {
			hxMotionStruct[6] += (hxMotionStruct[5] * scaleFactor);
	    }
	}
	else {
		hxMotionStruct[6] += (hxMotionStruct[3] * scaleFactor);
	}
	hxMotionStruct[7] += scaleFactor;
    hxMotionStruct[8] += hxMotionStruct[6];
	return hxMotionStruct[8];
}


SMS_PATCH_BL(SMS_PORT_REGION(0x8017ec20, 0x80290288, 0, 0), Hx_MotionUpdate);
SMS_PATCH_BL(SMS_PORT_REGION(0x8017ed20, 0x80290288, 0, 0), Hx_MotionUpdate);
SMS_PATCH_BL(SMS_PORT_REGION(0x8017edec, 0x80290288, 0, 0), Hx_MotionUpdate);
SMS_PATCH_BL(SMS_PORT_REGION(0x8017ee7c, 0x80290288, 0, 0), Hx_MotionUpdate);
SMS_PATCH_BL(SMS_PORT_REGION(0x8017efb0, 0x80290288, 0, 0), Hx_MotionUpdate);
SMS_PATCH_BL(SMS_PORT_REGION(0x8017f070, 0x80290288, 0, 0), Hx_MotionUpdate);
SMS_PATCH_BL(SMS_PORT_REGION(0x8017f160, 0x80290288, 0, 0), Hx_MotionUpdate);
SMS_PATCH_BL(SMS_PORT_REGION(0x8017f280, 0x80290288, 0, 0), Hx_MotionUpdate);
SMS_PATCH_BL(SMS_PORT_REGION(0x8017f5e0, 0x80290288, 0, 0), Hx_MotionUpdate);
SMS_PATCH_BL(SMS_PORT_REGION(0x8017facc, 0x80290288, 0, 0), Hx_MotionUpdate);
SMS_PATCH_BL(SMS_PORT_REGION(0x80180500, 0x80290288, 0, 0), Hx_MotionUpdate);
SMS_PATCH_BL(SMS_PORT_REGION(0x80180e10, 0x80290288, 0, 0), Hx_MotionUpdate);
SMS_PATCH_BL(SMS_PORT_REGION(0x80180eb8, 0x80290288, 0, 0), Hx_MotionUpdate);
SMS_PATCH_BL(SMS_PORT_REGION(0x80181b8c, 0x80290288, 0, 0), Hx_MotionUpdate);