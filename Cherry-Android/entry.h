#pragma once
#include <jni.h>
#include <errno.h>

#include <string.h>
#include <unistd.h>
#include <sys/resource.h>

#include <android/log.h>
#include <thread>


#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "Cherry_Android", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "Cherry_Android", __VA_ARGS__))


class Cherry_Android
{
public:
	const char * getPlatformABI();
	Cherry_Android();
	~Cherry_Android();
};

