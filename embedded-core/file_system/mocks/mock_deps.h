#ifndef MOCK_DEPS_H
#define MOCK_DEPS_H

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stddef.h>

/* ── ESP-IDF stubs ─────────────────────────────────────── */
#define ESP_LOGI(tag, fmt, ...) ((void)0)
#define ESP_LOGE(tag, fmt, ...) ((void)0)
#define ESP_LOGW(tag, fmt, ...) ((void)0)

/* ── SPIFFS stub ───────────────────────────────────────── */
extern bool mock_spiffs_init_result;
static inline bool Spiffs_Initialize(void) { return mock_spiffs_init_result; }

/* ── FILE mock control ─────────────────────────────────── */
extern bool  mock_fopen_fail;
extern bool  mock_fprintf_fail;
extern bool  mock_fgets_fail;
extern char  mock_file_content[512];   /* content fgets "reads" */

#define fopen(path, mode)  mock_fopen((path),(mode))
#define fclose(f)          mock_fclose(f)
#define fprintf(f, ...)    mock_fprintf(f, __VA_ARGS__)
#define fgets(buf,n,f)     mock_fgets(buf,n,f)

FILE *mock_fopen(const char *path, const char *mode);
int   mock_fclose(FILE *f);
int   mock_fprintf(FILE *f, const char *fmt, ...);
char *mock_fgets(char *buf, int n, FILE *f);

#endif /* MOCK_DEPS_H */