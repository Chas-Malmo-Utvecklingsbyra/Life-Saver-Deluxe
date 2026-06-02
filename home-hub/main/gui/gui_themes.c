#include "gui_themes.h"


const theme_t themes[4] = {
    { .name = "Dracula", .bg = 0x282A36, .sidebar = 0x44475A, .button = 0xBD93F9, .button_pressed = 0xFF79C6, .content_bg = 0x383A59, .sensor_bg = 0x6272A4, .text = 0xF8F8F2, .font_large = &lv_font_montserrat_22, .font_normal = &lv_font_montserrat_18, .font_small = &lv_font_montserrat_14, },
    { .name = "Nord", .bg = 0x2E3440, .sidebar = 0x3B4252, .button = 0x5E81AC, .button_pressed = 0x88C0D0, .content_bg = 0x373E4D, .sensor_bg = 0x4C566A, .text = 0xECEFF4, .font_large = &lv_font_montserrat_22, .font_normal = &lv_font_montserrat_18, .font_small = &lv_font_montserrat_14, },
    { .name = "Gruvbox", .bg = 0x282828, .sidebar = 0x3C3836, .button = 0xD79921, .button_pressed = 0xFABD2F, .content_bg = 0x32302F, .sensor_bg = 0x504945, .text = 0xEBDBB2, .font_large = &lv_font_montserrat_22, .font_normal = &lv_font_montserrat_18, .font_small = &lv_font_montserrat_14, },
    { .name = "Solarized", .bg = 0x002B36, .sidebar = 0x073642, .button = 0x268BD2, .button_pressed = 0x2AA198, .content_bg = 0x073642, .sensor_bg = 0x094652, .text = 0x839496, .font_large = &lv_font_montserrat_22, .font_normal = &lv_font_montserrat_18, .font_small = &lv_font_montserrat_14, },
};

/* 
    { .name = "Pokemon", .bg = 0xF47174, .sidebar = 0xCC0000, .content_bg = 0xCC0000, .sensor_bg = 0xEEEEEE, .button = 0xFFCC00, .button_pressed = 0xD4A200, .text = 0x222222, .text_secondary = 0x555555, .logo_img = &pokemon_logo, .font_large = &lv_font_montserrat_22, .font_normal = &lv_font_montserrat_18, .font_small = &lv_font_montserrat_14, },    
    { .name = "TMNT", .bg = 0x1A1A1A, .sidebar = 0x145A32, .content_bg = 0x222222, .sensor_bg = 0x2E2E2E, .button = 0x4CAF50, .button_pressed = 0x2E7D32, .text = 0xF5F5F5, .text_secondary = 0xAAAAAA, .logo_img = &turtles_logo, .font_large = &lv_font_montserrat_22, .font_normal = &lv_font_montserrat_18, .font_small = &lv_font_montserrat_14, },
    { .name = "Jurassic Park", .bg = 0x111111, .sidebar = 0x8B0000, .content_bg = 0x1C1C1C, .sensor_bg = 0x2C2C2C, .button = 0xD4AF37, .button_pressed = 0xB8860B, .text = 0xF5E6A9, .text_secondary = 0xC0B283, .logo_img = &jurassic_logo, .font_large = &lv_font_montserrat_22, .font_normal = &lv_font_montserrat_18, .font_small = &lv_font_montserrat_14, },
    { .name = "Star Wars", .bg = 0x0B0B0B, .sidebar = 0xFFE81F, .content_bg = 0x000000, .sensor_bg = 0x222222, .button = 0x000000, .button_pressed = 0xB8860B, .text = 0xFFE81F, .text_secondary = 0xC0B283, .logo_img = &jurassic_logo, .font_large = &lv_font_montserrat_22, .font_normal = &lv_font_montserrat_18, .font_small = &lv_font_montserrat_14, },
*/