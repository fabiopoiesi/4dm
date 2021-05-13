/*
 * log.h
 *
 *  Created on: May 16, 2019
 *      Author: mbortolon
 */

#ifndef SRC_LIB_LOG_H_
#define SRC_LIB_LOG_H_

#ifdef __ANDROID__

#include <android/log.h>

#define LOG_TAG "frame_sender"

#ifndef LOGD
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#endif

#ifndef LOGI
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#endif

#ifndef LOGE
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#endif

#else

#ifndef LOGD
#define LOGD(...) printf(__VA_ARGS__); printf("\n");
#endif

#ifndef LOGI
#define LOGI(...) printf(__VA_ARGS__); printf("\n");
#endif

#ifndef LOGE
#define LOGE(...) fprintf(stderr, __VA_ARGS__); fprintf(stderr, "\n");
#endif

#endif

#endif /* SRC_LIB_LOG_H_ */
