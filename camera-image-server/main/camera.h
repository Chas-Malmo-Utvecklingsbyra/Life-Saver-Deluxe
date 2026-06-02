#ifndef CAMERA_H
#define CAMERA_H

#include "esp_camera.h"

#define DEBUG_MOTION 0

#define BLOCK_SIZE_X 20 // size of image blocks used for motion sensing (20)
#define BLOCK_SIZE_Y 20

#define FWIDTH 320 // motion sensing frame size
#define FHEIGHT 240

#define FW (FWIDTH / BLOCK_SIZE_X) // number of blocks in image
#define FH (FHEIGHT / BLOCK_SIZE_Y)

// Detection parameters (adjustable)
extern uint16_t blockThreshold;
extern uint16_t imageThresholdL;
extern uint16_t imageThresholdH;
extern uint16_t tCounter;
extern uint16_t tCounterTrigger;
extern uint16_t averagePix;
extern uint16_t latestChanges;

// Camera image settings (adjustable)
extern float cameraImageGain;
extern float cameraImageExposure;

void update_frame();
bool capture_still();
float motion_detect();

#endif // CAMERA_H