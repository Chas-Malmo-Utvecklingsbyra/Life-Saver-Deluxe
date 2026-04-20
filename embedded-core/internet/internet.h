#ifndef INTERNET_H
#define INTERNET_H

void Internet_Initialize(const char *ssid, const char *password);

// this is thread safe
bool Internet_Is_Connected();

#endif