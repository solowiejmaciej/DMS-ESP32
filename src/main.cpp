#include <DMSManager.h>

const char *DMS_URL = "https://dms.solowiejmaciej.com";
const char *DMS_API_KEY = "yourkey";
const char *SSID = "INEA-73EF_2.4G";
const char *SSID_PASSWORD = "8689450570";

DMSManager dmsManager;

void setup()
{
  Serial.begin(115200);
  delay(100);
  dmsManager.setup(DMSOptions{
      DMS_URL,
      DMS_API_KEY,
      SSID,
      SSID_PASSWORD,
  });
  Serial.println("Device is ready!");
}
void loop()
{
  dmsManager.loop();
}
