#pragma once

#include <glog/logging.h>

#define GLOG_AUTO_CLEAN_AFTER_DAYS  30  // keep your logs for 30 days

inline void InitLogger(char* name)
{
    // Initialize Google’s logging library.
#ifdef NDEBUG
    FLAGS_logtostderr = 0;
#else
    FLAGS_logtostderr = 1;
#endif
    google::InitGoogleLogging(name);
    google::EnableLogCleaner(GLOG_AUTO_CLEAN_AFTER_DAYS);
}
