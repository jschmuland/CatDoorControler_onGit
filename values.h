/* 
  These are the values used in the main sketch
  they are all kept in the same spot so I don't have to go searching to find them
*/

//************************************   Values for variables   *************************************
//Pin Definitions
#define TERMINAL_VPIN V1
#define MIN_TEMP_ALLOWED_VPIN V2
#define CATS_GO_OUT_VPIN V3
#define CATS_COME_IN_VPIN V4
#define TOTAL_OVERRIDE_VPIN V5
#define INPUT_NIGHT_CURFEW_TIME_VPIN V6
#define INPUT_MORNING_CURFEW_TIME_VPIN V7
#define ONE_LED_PIN 5 //D1 on mini
#define EIGHT_LED_PIN 4 //D2 on mini
#define INSIDE_SERVO_PIN 13 //D7 on mini
#define OUTSIDE_SERVO_PIN 15 //D8 on mini

//Intervals Definitions
#define LED_BLINK_INTERVAL    2 //2sec
#define WEATHER_GET_INTERVAL  60 * 10 //10min
#define DAY_WEATHER_COMPAIR_INTERVAL WEATHER_GET_INTERVAL/2

//Serial Monitor baud
#define SERIALBAUD 250000

//************************************   WiFi credentials    *************************************
#define SECRET_SSID1 "Ribland Wireless Inc"
#define SECRET_PSW1 "D0ct0rN3rdDuo2020!"
//#define SECRET_SSID2 "Guest Who?"
//#define SECRET_PSW2 "ribland191"
#define SECRET_SSID3 "joelsHotspot"
#define SECRET_PSW3 "kapinger"
#define DEVICE_NAME "Cat Door Controler"


//************************************   3rd Party Tokens and Passwords   *************************************
//Blynk Tokens
#define SECRET_BLYNK_TOKEN "2e5OZVoZxra-oxatmtqG9rOQcMKnSvOM"  //Cat Door Token
//#define SECRET_BLYNK_TOKEN "dAyAHXLzQ9Zs-PVMg8r_0Vh0vDhCZwoL"  //Basic Sketch Token

//OTA Values
#define OTA_HOSTNAME "Cat Door"
#define OTA_PSW "kapinger"

// OpenWeather API Details
#define OPEN_WEATHER_API_KEY  "24d27a7c2ee171e95d64b152bcb31f07" // Obtain this from your OpenWeather account
#define LAT   "45.4215" // 90.0000 to -90.0000 negative for Southern hemisphere
#define LONG  "-75.6972" // 180.000 to -180.000 negative for West
#define UNITS  "metric"  // or "imperial"
#define LANGUAGE  "en"   // See notes tab
#define TIME_OFFSET -4UL * 3600UL // UTC + 0 hour
