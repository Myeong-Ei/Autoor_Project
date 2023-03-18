#define IO_USERNAME  "MyeongEi"
#define IO_KEY       "aio_mlYR03kpbVTuyRuGlbG4gIE2dZlb"

#define WIFI_SSID "Kang_1101"
#define WIFI_PASS "$*^audtjr1021"

#include "AdafruitIO_WiFi.h"

#if defined(USE_AIRLIFT) || defined(ADAFRUIT_METRO_M4_AIRLIFT_LITE) ||         \
    defined(ADAFRUIT_PYPORTAL)
#if !defined(SPIWIFI_SS)

#define SPIWIFI SPI
#define SPIWIFI_SS 10 
#define NINA_ACK 9    
#define NINA_RESETN 6 
#define NINA_GPIO0 -1 
#endif
AdafruitIO_WiFi io(IO_USERNAME, IO_KEY, WIFI_SSID, WIFI_PASS, SPIWIFI_SS,
                   NINA_ACK, NINA_RESETN, NINA_GPIO0, &SPIWIFI);
#else
AdafruitIO_WiFi io(IO_USERNAME, IO_KEY, WIFI_SSID, WIFI_PASS);
#endif
