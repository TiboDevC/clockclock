#ifndef CLOCKCLOCK_NTP_SYNC_HPP
#define CLOCKCLOCK_NTP_SYNC_HPP

#include <stdint.h>

enum class SyncResult { SUCCESS, WIFI_FAILED, NTP_FAILED, RTC_UPDATE_FAILED };

// Predefined timezone offsets (in seconds)
namespace TimeZone
{
static constexpr int32_t UTC = 0;
static constexpr int32_t CET = 3600;   // Central European Time
static constexpr int32_t CEST = 7200;  // Central European Summer Time
static constexpr int32_t EST = -18000; // Eastern Standard Time
static constexpr int32_t PST = -28800; // Pacific Standard Time
} // namespace TimeZone

// One-shot NTP sync for development only
SyncResult sync_rtc_once(const char *ssid, const char *password, int32_t timezone_offset = TimeZone::UTC);

#endif /* CLOCKCLOCK_NTP_SYNC_HPP */
