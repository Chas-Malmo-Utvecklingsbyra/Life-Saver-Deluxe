#include "gui_themes.h"


const theme_t themes[4] = {
    { .name = "Dracula", .bg = 0x282A36, .sidebar = 0x44475A, .button = 0xBD93F9, .button_pressed = 0xFF79C6, .content_bg = 0x383A59, .sensor_bg = 0x6272A4, .text = 0xF8F8F2, .font_large = &lv_font_montserrat_22, .font_normal = &lv_font_montserrat_18, .font_small = &lv_font_montserrat_14, },
    { .name = "Nord", .bg = 0x2E3440, .sidebar = 0x3B4252, .button = 0x5E81AC, .button_pressed = 0x88C0D0, .content_bg = 0x373E4D, .sensor_bg = 0x4C566A, .text = 0xECEFF4, .font_large = &lv_font_montserrat_22, .font_normal = &lv_font_montserrat_18, .font_small = &lv_font_montserrat_14, },
    { .name = "Gruvbox", .bg = 0x282828, .sidebar = 0x3C3836, .button = 0xD79921, .button_pressed = 0xFABD2F, .content_bg = 0x32302F, .sensor_bg = 0x504945, .text = 0xEBDBB2, .font_large = &lv_font_montserrat_22, .font_normal = &lv_font_montserrat_18, .font_small = &lv_font_montserrat_14, },
    { .name = "Solarized", .bg = 0x002B36, .sidebar = 0x073642, .button = 0x268BD2, .button_pressed = 0x2AA198, .content_bg = 0x073642, .sensor_bg = 0x094652, .text = 0xF8F8F2, .font_large = &lv_font_montserrat_22, .font_normal = &lv_font_montserrat_18, .font_small = &lv_font_montserrat_14, },
};