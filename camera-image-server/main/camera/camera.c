#include "camera.h"
#include "esp_log.h"
#include "math.h"
#include "jpeg_decoder.h"
#include "esp_heap_caps.h"

static const char *TAG = "Motion";

// Camera image settings
float cameraImageGain = 0;
float cameraImageExposure = 0;

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
uint16_t tCounterTrigger = 2; // number of consecutive triggers required to count as movement detected

// store most current motion detection reading for display on main page
uint16_t latestChanges = 0;

// PSRAM buffer for JPEG decode output (RGB888, FWIDTH * FHEIGHT * 3 bytes)
#define DECODE_BUF_SIZE (FWIDTH * FHEIGHT * 3)
static uint8_t *s_decode_buf = NULL;

bool camera_motion_init(void)
{
    s_decode_buf = heap_caps_malloc(DECODE_BUF_SIZE, MALLOC_CAP_SPIRAM);
    if (!s_decode_buf)
    {
        ESP_LOGE(TAG, "Failed to allocate JPEG decode buffer (%d bytes) in PSRAM", DECODE_BUF_SIZE);
        return false;
    }
    rebuild_block_mask();
    return true;
}

// Image detection mask (i.e. if area of image is enabled for use when motion sensing, 1=active)
// 5 x 5 grid results in mask areas of 25 blocks (5x5) - image = 640x480 pixels, blocks = 20x20 pixels
#define MASK_COLUMNS 5
#define MASK_ROWS    5
uint16_t mask_active = MASK_COLUMNS * MASK_ROWS; // number of mask sections active
const uint16_t maskBlockWidth = FW / MASK_COLUMNS;
const uint16_t maskBlockHeight = FH / MASK_ROWS;
bool mask_frame[MASK_COLUMNS][MASK_ROWS] = {{true, true, true, true, true},
                                            {true, true, true, true, true},
                                            {true, true, true, true, true},
                                            {true, true, true, true, true},
                                            {true, true, true, true, true}};

// Precomputed per-block mask lookup — rebuilt by rebuild_block_mask() whenever mask_frame changes
static bool block_mask[FH][FW];

void rebuild_block_mask(void)
{
    for (int y = 0; y < FH; y++)
    {
        for (int x = 0; x < FW; x++)
        {
            uint16_t mask_x = x / maskBlockWidth;
            uint16_t mask_y = y / maskBlockHeight;
            if (mask_x >= MASK_COLUMNS) mask_x = MASK_COLUMNS - 1;
            if (mask_y >= MASK_ROWS)    mask_y = MASK_ROWS - 1;
            block_mask[y][x] = mask_frame[mask_x][mask_y];
        }
    }
}

void update_frame()
{
    memcpy(prev_frame, current_frame, sizeof(prev_frame));
}

bool capture_still()
{
    if (!s_decode_buf)
        return false;

    camera_fb_t *frame_buffer = esp_camera_fb_get();
    if (!frame_buffer)
        return false;

    // Decode VGA JPEG into RGB888 at full resolution
    esp_jpeg_image_cfg_t cfg = {
        .indata      = frame_buffer->buf,
        .indata_size = frame_buffer->len,
        .outbuf      = s_decode_buf,
        .outbuf_size = DECODE_BUF_SIZE,
        .out_format  = JPEG_IMAGE_FORMAT_RGB888,
        .out_scale   = JPEG_IMAGE_SCALE_0,
    };
    esp_jpeg_image_output_t out_img;
    esp_err_t err = esp_jpeg_decode(&cfg, &out_img);
    esp_camera_fb_return(frame_buffer);

    if (err != ESP_OK)
        return false;

    // Down-sample decoded RGB888 pixels into motion blocks using luma approximation
    uint32_t temp_frame[FH][FW] = {0};
    const uint8_t *p = s_decode_buf;
    for (uint16_t y = 0; y < out_img.height; y++)
    {
        const uint8_t block_y = y / BLOCK_SIZE_Y;
        for (uint16_t x = 0; x < out_img.width; x++, p += 3)
        {
            const uint8_t block_x = x / BLOCK_SIZE_X;
            // Fast luma approximation: (R + 2G + B) / 4
            const uint8_t luma = (p[0] + ((uint16_t)p[1] << 1) + p[2]) >> 2;
            temp_frame[block_y][block_x] += luma;
        }
    }

    uint16_t TempAveragePix = 0;
    for (int y = 0; y < FH; y++)
    {
        for (int x = 0; x < FW; x++)
        {
            uint16_t currentBlock = temp_frame[y][x] / (BLOCK_SIZE_X * BLOCK_SIZE_Y);
            current_frame[y][x] = currentBlock;
            TempAveragePix += currentBlock;
        }
    }

    averagePix = TempAveragePix / (FH * FW);
    return true;
}

uint16_t motion_detect()
{
    uint16_t changes = 0;

    // go through all blocks in current frame and check for changes since previous frame
    for (int y = 0; y < FH; y++)
    {
        for (int x = 0; x < FW; x++)
        {
            if (!block_mask[y][x])
                continue;

            const uint16_t current = current_frame[y][x];
            const uint16_t prev    = prev_frame[y][x];
            const uint16_t pChange = (current > prev) ? current - prev : prev - current;

            if (pChange >= blockThreshold)
                changes++;
        }
    }

    if (changes > latestChanges)
        latestChanges = changes;

    if (changes >= imageThresholdL && changes <= imageThresholdH)
        tCounter++;
    else
        tCounter = 0;

    return changes;
}

