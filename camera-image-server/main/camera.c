#include "camera.h"
#include "esp_log.h"
#include "math.h"

static const char *TAG = "Motion";

// Camera image settings
float cameraImageGain = 0;
float cameraImageExposure = 0;
float thresholdGainCompensation = 0; // motion detection level compensation for increased noise in image when gain increased (i.e. in darker conditions)

// frame stores (blocks)
uint16_t prev_frame[FH][FW] = {0};    // previously captured frame
uint16_t current_frame[FH][FW] = {0}; // current frame

// detection parameters
uint16_t averagePix = 0;        // average pixel reading from captured image (bright day = around 120)
uint16_t targetBrightness = 120;
uint16_t blockThreshold = 10;   // average pixel variation in block required to count as changed - range 0 to 255
uint16_t imageThresholdL = 1;  // min changed blocks in image required to count as motion detected in percent
uint16_t imageThresholdH = 255; // max changed blocks in image required to count as motion detected in percent

// misc
uint16_t tCounter = 0;        // count number of consecutive triggers
uint16_t tCounterTrigger = 2; // number of consequitive triggers required to count as movement detected
const uint16_t blocksPerMaskUnit = 16; // number of blocks in each of the 12 detection mask units

// store most current motion detection reading for display on main page
uint16_t latestChanges = 0;

// Image detection mask (i.e. if area of image is enabled for use when motion sensing, 1=active)
// 4 x 3 grid results in mask areas of 16 blocks (4x4) - image = 320x240 pixels, blocks = 20x20 pixels
#define MASK_COLUMNS 4
#define MASK_ROWS    3
uint16_t mask_active = 12; // number of mask sections active
const uint16_t maskBlockWidth = FW / MASK_COLUMNS;
const uint16_t maskBlockHeight = FH / MASK_ROWS;
bool mask_frame[MASK_COLUMNS][MASK_ROWS] = {{true, true, true},
                                            {true, true, true},
                                            {true, true, true},
                                            {true, true, true}};

/// @brief Update the previous frame with the current frame
void update_frame()
{
    memcpy(prev_frame, current_frame, sizeof(prev_frame));
}

/// @brief Check if a block is active in the detection mask
/// @param x Block x coordinate
/// @param y Block y coordinate
/// @return True if the block is active, false otherwise
bool block_active(uint16_t x, uint16_t y)
{
    // Which mask area is this block in
    uint16_t mask_x = floor(x / maskBlockWidth);  // x mask area (0 to 3)
    uint16_t mask_y = floor(y / maskBlockHeight); // y mask area (0 to 2)

    return mask_frame[mask_x][mask_y];
}

bool capture_still()
{
    uint32_t temp_frame[FH][FW] = {0};
    camera_fb_t *frame_buffer = esp_camera_fb_get();
    
    if (!frame_buffer)
    { 
        frame_buffer = esp_camera_fb_get();
        if (!frame_buffer)
            return false; // failed to capture image
    }

    // down-sample image in to blocks, step through all pixels in image
    for (uint32_t i = 0; i < (FWIDTH * FHEIGHT); i++)
    {                                  
        const uint16_t x = i % FWIDTH;
        const uint16_t y = floor(i / FWIDTH);
        const uint8_t block_x = floor(x / BLOCK_SIZE_X);
        const uint8_t block_y = floor(y / BLOCK_SIZE_Y);
        //const uint8_t pixel = frame_buffer->buf[i]; // get the pixels brightness (0 to 255)
        temp_frame[block_y][block_x] += frame_buffer->buf[i]; // add this pixel to the blocks running total
    }
    esp_camera_fb_return(frame_buffer);

    //bool frameChanged = false;
    uint16_t TempAveragePix = 0; // average pixel reading (used for calculating image brightness)
    for (int y = 0; y < FH; y++)
    {
        for (int x = 0; x < FW; x++)
        {
            uint16_t currentBlock = temp_frame[y][x] / (BLOCK_SIZE_X * BLOCK_SIZE_Y); // average pixel brightness in the block
            if (current_frame[y][x] != currentBlock)
            {
                //frameChanged = true; // not currently used but could be used to skip motion detection if image is the same as previous frame
            }
            current_frame[y][x] = currentBlock; //save 
            TempAveragePix += currentBlock; //used to calculate average brightness of whole image
        }
    }

    averagePix = TempAveragePix / (FH * FW); // calculate the average pixel brightness in whole image

    return true;
}

float motion_detect()
{
    uint16_t changes = 0;
    //const uint16_t blocks = (FWIDTH * FHEIGHT) / (BLOCK_SIZE_X * BLOCK_SIZE_Y); //total number of blocks in image

    //adjust block_threshold for gain setting (to compensate for noise introduced with gain)
    uint16_t tThreshold = blockThreshold + (float)(cameraImageGain * thresholdGainCompensation);

    // go through all blocks in current frame and check for changes since previous frame
    for (int y = 0; y < FH; y++)
    {
        for (int x = 0; x < FW; x++)
        {
            uint16_t current = current_frame[y][x];
            uint16_t prev = prev_frame[y][x];
            uint16_t pChange = abs(current - prev);
            //check if change in block is above threshold and block is active in detection mask
            if (pChange >= tThreshold)
            { 
                if (block_active(x, y))
                    changes += 1;

            }

        }
    }

    if (changes > latestChanges)
        latestChanges = changes;
    
    //check if number of changed blocks is above motion detection threshold
    if (changes >= imageThresholdL && changes <= imageThresholdH)
        tCounter++;
    else
        tCounter = 0;

    return changes; // return number of changed blocks
}

