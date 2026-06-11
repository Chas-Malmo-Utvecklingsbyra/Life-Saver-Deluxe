#ifndef CAMERA_H
#define CAMERA_H

#include "esp_camera.h"

#define DEBUG_MOTION 0

// size of image blocks used for motion sensing (20)
#define BLOCK_SIZE_X 20
#define BLOCK_SIZE_Y 20

// motion sensing frame size
#define FWIDTH 640 
#define FHEIGHT 480

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

/// @brief Initialize motion detection
/// @return true if initialization was successful, false otherwise
bool camera_motion_init(void);

/// @brief Rebuild the block mask based on the current mask_frame configuration
void rebuild_block_mask(void);

/// @brief Update the previous frame with the current frame data
void update_frame();

/// @brief Capture a still image and update the current frame data for motion detection
/// @return true if the capture was successful, false otherwise
bool capture_still();

/// @brief Perform motion detection on the current frame
/// @return the number of changed blocks detected
uint16_t motion_detect();

#endif // CAMERA_H