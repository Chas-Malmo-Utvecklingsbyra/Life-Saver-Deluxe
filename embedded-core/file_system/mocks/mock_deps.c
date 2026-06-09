#include "mock_deps.h"
#include <stdarg.h>
#include <string.h>

/* ── Controllable state ──────────────────────────────────── */
bool mock_spiffs_init_result = true;
bool mock_fopen_fail         = false;
bool mock_fprintf_fail       = false;
bool mock_fgets_fail         = false;
char mock_file_content[512]  = {0};

/* Dummy non-NULL sentinel used as FILE* */
static FILE _dummy_file;

FILE *mock_fopen(const char *path, const char *mode)
{
    (void)path; (void)mode;
    if (mock_fopen_fail) return NULL;
    return &_dummy_file;
}

int mock_fclose(FILE *f)
{
    (void)f;
    return 0;
}

int mock_fprintf(FILE *f, const char *fmt, ...)
{
    (void)f;
    if (mock_fprintf_fail) return -1;
    /* just return a positive length to signal success */
    return (int)strlen(fmt);
}

char *mock_fgets(char *buf, int n, FILE *f)
{
    (void)f;
    if (mock_fgets_fail) return NULL;
    strncpy(buf, mock_file_content, n - 1);
    buf[n - 1] = '\0';
    return buf;
}