#include <DMSManager.h>

const char *DMS_URL = "http://192.168.1.172:3000";
const char *SSID = "INEA-73EF_2.4G";
const char *SSID_PASSWORD = "8689450570";

DMSManager dmsManager;

void setup()
{
  Serial.begin(115200);
  delay(100);
  dmsManager.setup(DMSOptions{
      DMS_URL,
      SSID,
      SSID_PASSWORD,
  });
  Serial.println("Device is ready!");
}
void loop()
{
  dmsManager.loop();
}
