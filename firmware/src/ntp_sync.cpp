#ifdef NTP_WIFI_SYNC

#include "ntp_sync.hpp"
#include "DS3231.h"
#include <NTPClient.h>
#include <WiFi.h>
#include <WiFiUdp.h>

SyncResult sync_rtc_once(const char *ssid, const char *password, int32_t timezone_offset)
{
	Serial.println("Starting one-time NTP synchronization...");

	// Connect to WiFi
	Serial.print("Connecting to WiFi: ");
	Serial.println(ssid);
	WiFi.begin(ssid, password);

	unsigned long start_time = millis();
	while (WiFi.status() != WL_CONNECTED) {
		if (millis() - start_time > 15000) { // 10 second timeout
			Serial.println("WiFi connection failed!");
			return SyncResult::WIFI_FAILED;
		}
		delay(500);
		Serial.print(".");
	}
	Serial.println("\nWiFi connected!");

	// Setup NTP client
	WiFiUDP ntpUDP;
	NTPClient timeClient(ntpUDP, "pool.ntp.org", timezone_offset, 60000);
	timeClient.begin();

	// Get time from NTP
	Serial.println("Getting time from NTP server...");
	if (!timeClient.forceUpdate()) {
		Serial.println("Failed to get time from NTP server!");
		WiFi.disconnect();
		return SyncResult::NTP_FAILED;
	}

	// Update RTC
	static constexpr int second_offset = -1; // Remove 1 second for hands to start turning a bit earlier
	unsigned long epoch_time = timeClient.getEpochTime() + second_offset;
	Serial.print("NTP time (epoch): ");
	Serial.println(epoch_time);

	timeClient.end();

	DS3231 Clock;
	Clock.setEpoch(epoch_time, false);

	Clock.setClockMode(false); // 24h mode

	Serial.println("RTC successfully synchronized!");
	WiFi.disconnect();
	Serial.println("WiFi disconnected");

	return SyncResult::SUCCESS;
}

#endif // NTP_WIFI_SYNC
