/*************************
 This code is for the cat door
 Joel S
 TODO: bashed
 */

//Library's to include
#include <ServoWrap.h>
#include "values.h"
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <Adafruit_NeoPixel.h>
#include <ArduinoOTA.h>
#include <BlynkSimpleEsp8266.h>
#include <JSON_Decoder.h>
#include <OpenWeather.h>
#include <TimeLib.h>
#include <TimeAlarms.h>

//Instances to create
ESP8266WiFiMulti wifi;
WidgetTerminal terminal(TERMINAL_VPIN);
//BlynkTimer timer;
OW_Weather ow; // Weather forecast library instance
ServoWrap insideServo(INSIDE_SERVO_PIN,180,70);  //inside open 180, inside close 70
ServoWrap outsideServo(OUTSIDE_SERVO_PIN,30,140); //outside open 30, outside closed 140
Adafruit_NeoPixel onePxl = Adafruit_NeoPixel(1, ONE_LED_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel eightPxls = Adafruit_NeoPixel(8, EIGHT_LED_PIN, NEO_GRB + NEO_KHZ800);
uint32_t lowRed = onePxl.Color(50,0,0);
uint32_t lowGreen = onePxl.Color(0,50,0);
AlarmId nightAlarmId;
AlarmId morningAlarmId;
AlarmId dayWeatherCompairId;
time_t nightCurfewTime = AlarmHMS(22,0,0);
time_t morningCurfewTime = AlarmHMS(8,0,0);

//enabling the reset function
void(* resetFunc) (void) = 0;

/*************************************** SETUP **********************************************************/
void setup() {
  Serial.begin(SERIALBAUD);
  while (!Serial) ; // wait for Arduino Serial Monitor
  setupLEDs();
  insideServo.setup();
  outsideServo.setup();
  delay (2000);
  basicStartup();  // Start debug console, connect wifi, setup OTA, connect Blynk, turn on terminal, set timer for onboard LED and Scheduler Check

  //set the timer for getWeather and then make the first call to fill values
  getWeather();
  Blynk.syncAll();

  //************Setting up alarms************//
  Alarm.timerRepeat(LED_BLINK_INTERVAL, ledBlynk);
  Alarm.timerRepeat(WEATHER_GET_INTERVAL, getWeather);
  delay (5000); //wait 5 sec before setting the next alarms
  dayWeatherCompairId = Alarm.timerRepeat(DAY_WEATHER_COMPAIR_INTERVAL, daytimeWeatherCompair);
  nightAlarmId        = Alarm.alarmRepeat(nightCurfewTime, nightTimeCheck);
  morningAlarmId      = Alarm.alarmRepeat(morningCurfewTime, morningTimeCheck);
}

/******************************** LOOP *********************************************************/
//loop variables
bool catsGoOut;
bool catsCanComeIn;
bool warmEnoughToGoOutForcast;
bool warmEnoughToGoOutNow;
bool overrideEverything;
bool itsNightTime;
float minTempCatsGoOut = 5;
float currentTemp;
float minTemp;
time_t sunsetTime;
time_t sunriseTime;
time_t t;

void loop() {
  basicLoop(); //Checks WiFi connection, runs OTA, runs Blynk loop
  Alarm.delay(0); //Enables TimerAlarms to work
}

//**************************************************************************//
//                              FUNCTIONS                                   //
//                this is where the locig is programmed                     //
//**************************************************************************//

/*Ran at the nightCurfewTime,
  checks if door is closed and
  weather is going to me warm enough*/
void nightTimeCheck(){
  Serial.println("\nNight time activated");
  if (!overrideEverything){
    if (catsGoOut) {
      Serial.println (" Cats can currently go out, checking if its going to be cold.");

      if (!warmEnoughToGoOutForcast) {
        p("It's past night curfew and its going to be cold\n  Closing the cat door",true);
        catsGoOut = !catsGoOut;
        Blynk.virtualWrite(CATS_GO_OUT_VPIN,catsGoOut);
        Blynk.notify("It's going to be chilly, keeping the cats in tonight.");
      }else{
        Blynk.notify("It's a low of " + String(minTemp) + "c, Party On!");
        Serial.println("    It's not going to be cold cats can go out all night.");
      }//end not warm enough to go out

    }else{
      Blynk.notify("Just Checked, Door is closed, keeping it that way");
      Serial.println("  Cats can't go out right now, changing nothing.");
    }//end check if door open
  }//end overrideEverything
  Serial.println("  Turning off day time weather compair");
  itsNightTime = true;
}//end checkTime

/*Ran at the morningCurfewTime,
  checks if it is warm enough to go out
  and if door is already open*/
void morningTimeCheck(){
  if (!overrideEverything){
    Serial.println("\nMorning time activated");
    if (warmEnoughToGoOutNow){
      Serial.println("  It's warm enough to go out right now");

      if (!catsGoOut){
        p("It's warm enough and after morning curfew.\n Letting the cats out",true);
        catsGoOut = !catsGoOut;
        Blynk.virtualWrite(CATS_GO_OUT_VPIN,catsGoOut);
        Blynk.notify("The cat's are now welcome outside");

      }else{
        Serial.println("  Cats can already go out. Changing nothing.");
      }// end if cats can go out
    }//end if warmEnoughToGoOutNow
    Serial.println("  Starting weather compair for the day.");
    itsNightTime = false;
  }
}

/*ran at daytimeWeatherCompair interval
  checks if its night timer
  and if cats can already go out
  and if its warm enough then lets cats go out*/
void daytimeWeatherCompair(){
  if !itsNightTime{
    p("\nCompairing Weather:",true);
    if (!catsGoOut){
      Serial.println("  Cat's can't go out yet, Checking if its warm enough now:");
      if (warmEnoughToGoOutNow) {
        p("It's now warm enough for the cats to go out. Opening the door",true);
        catsGoOut = !catsGoOut;
        Blynk.virtualWrite(CATS_GO_OUT_VPIN,catsGoOut);
        Blynk.notify("The weather is warm enough. Cat door opening!");

      }else{
        Serial.println("    Not warm enough yet, try again later.");
      }
    }else{
      Serial.println("  Cats can already go out. No need to check Temp");
    }
  }//end if itsNightTime
}

//Opens the inside servo
void openInside(){
  insideServo.openIt();
  eightPxls.fill(lowGreen,7);
  eightPxls.show();
  Serial.println (" -> Cats can come in");
  terminal.println(" -> Cats can come in");
}

//Opens the outside servo
void openOutside(){
  outsideServo.openIt();
  onePxl.fill(lowGreen,0);
  onePxl.show();
  Serial.println (" -> Cats can go out");
  terminal.println(" -> Cats can go out");
}

//Closes the inside servo
void closeInside(){
  insideServo.closeIt();
  eightPxls.fill(lowRed,7);
  eightPxls.show();
  Serial.println (" -> Cats can't come in");
  terminal.println(" -> Cats can't come in");
}

//Closes the outside servo
void closeOutside(){
  outsideServo.closeIt();
  onePxl.fill(lowRed,0);
  onePxl.show();
  Serial.println (" -> Cats can't go out");
  terminal.println(" -> Cats can't go out");
}
//**************************************************************************//
//                          VPIN Fuctions                                   //
//                    these get called by the app                           //
//**************************************************************************//
// letting the cats go out by opening outside servo
BLYNK_WRITE(CATS_GO_OUT_VPIN){
  p("V3 has been switched",false);
  catsGoOut = param.asInt();
  catsGoOut ? openOutside() : closeOutside();
}

//Letting the cats come in by opening the inside servos
//yet to fully be used. will use with weight scale
BLYNK_WRITE(CATS_COME_IN_VPIN){
  p ("V4 has been switched",false);
  catsCanComeIn = param.asInt(); // assigning incoming value from pin V4 to a variable
  catsCanComeIn ? openInside() : closeInside();
}

//Set the temp threshold and check bools
BLYNK_WRITE(MIN_TEMP_ALLOWED_VPIN){
  minTempCatsGoOut = param.asInt();
  p("The cats can now go out above " + String(minTempCatsGoOut) + "c",true);
  checkTempBools();
}

//This function is to see when the exterior door will automatically open or close at night.
BLYNK_WRITE(INPUT_NIGHT_CURFEW_TIME_VPIN){
  String curfewTime = param.asStr();
  curfewTime.toLowerCase();

  if (curfewTime == "sunset"){
    nightCurfewTime = AlarmHMS(hour(sunsetTime),minute(sunsetTime),0);
  }else{
    String hvalue = getValue(curfewTime,':',0);
    String mvalue = getValue(curfewTime,':',1);
    nightCurfewTime = AlarmHMS(hvalue.toInt(), mvalue.toInt(),0);
  }

  Alarm.write(nightAlarmId,nightCurfewTime);
  p("The cats can go out at night until " + strTime(time_t(nightCurfewTime)),true);
}

//This function is to see when the exterior door will automatically open or close in the morning.
BLYNK_WRITE(INPUT_MORNING_CURFEW_TIME_VPIN){
  String tempCurfewTime = param.asStr();
  tempCurfewTime.toLowerCase();

  if (tempCurfewTime == "sunrise"){
    morningCurfewTime = AlarmHMS(hour(sunriseTime), minute(sunriseTime),0);
  }else{
    String hvalue = getValue(tempCurfewTime,':',0);
    String mvalue = getValue(tempCurfewTime,':',1);
    morningCurfewTime = AlarmHMS(hvalue.toInt(),hvalue.toInt(),0);
  }

  Alarm.write(morningAlarmId,morningCurfewTime);
  p("The cats can go out in the morning at " + strTime(time_t(morningCurfewTime)),true);
}

BLYNK_WRITE(TOTAL_OVERRIDE_VPIN){
  overrideEverything = param.asInt();
  // if (overrideEverything) {
  //   Alarm.disable(nightAlarmId);
  //   Alarm.disable(morningAlarmId);
  //   Alarm.disable(dayWeatherCompairId);
  // }else{
  //   Alarm.enable(nightAlarmId);
  //   Alarm.enable(morningAlarmId);
  //   Alarm.enable(dayWeatherCompairId);
  // }
}

//This fuction gets the weather from openweathermap and places it into the values below. To find out what you can get look at the openweathertest2 sketch
void getWeather(){
   // Create the structures that hold the retrieved weather
  OW_current *current = new OW_current;
  OW_hourly *hourly = new OW_hourly;
  OW_daily  *daily = new OW_daily;

  Serial.println("\nRequesting weather information from OpenWeather... ");
  ow.getForecast(current, hourly, daily, OPEN_WEATHER_API_KEY, LAT, LONG, UNITS, LANGUAGE);
  terminal.clear();
  p("Weather from Open Weather is now in system",true);

  setTime(current->dt + TIME_OFFSET);

  currentTemp = current->temp;
  minTemp = (daily->temp_min[0])<(daily->temp_min[1])?daily->temp_min[0]:daily->temp_min[1];
  checkTempBools();
  Blynk.virtualWrite(V0,minTemp);

  sunsetTime = current->sunset + TIME_OFFSET;
  sunriseTime = current->sunrise + TIME_OFFSET;

  delete current;
  delete hourly;
  delete daily;
}

String strTime(time_t unixTime){
  unixTime;
  return ctime(&unixTime);
}

//******************* check to see if the new temp is > than the allowed temp ************
void checkTempBools(){
  warmEnoughToGoOutForcast = minTemp > minTempCatsGoOut? true : false;
  warmEnoughToGoOutNow = currentTemp > minTempCatsGoOut? true : false;
}
//******************************************************************************//
//                        Startup Functions                                     //
//******************************************************************************//

//********* Keeping the code out of the startup call **************************//
void basicStartup(){
  connectWiFi();
  Blynk.config(SECRET_BLYNK_TOKEN); //,SECRET_SSID,SECRET_PSW connect to the internet and get blink running
  Blynk.connect();
  OTASetup();

  // Blynk Terminal startup - clear the terminal content
  terminal.clear();

  // This will print Blynk Software version to the Terminal Widget when
  // your hardware gets connected to Blynk Server
  terminal.println(F("Blynk v" BLYNK_VERSION ": Device started"));
  terminal.println(F("-------------"));
  terminal.print(F("Connected to: "));
  terminal.println((WiFi.SSID()));
  terminal.print(F("IP address: "));
  terminal.println((WiFi.localIP()));
  terminal.println(F("-------------"));
  terminal.println(F("Type 'Help' and get possible inputs"));
  terminal.flush();

  //configure LED and Timer
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
}

//********* Seting up the LEDs for Colours ***************//
void setupLEDs(){
  //Set up the one pxl
  onePxl.begin();
  onePxl.fill(lowRed,0);
  onePxl.show(); // Initialize all pixels to red

  //Set up the 8 pxl ring
  eightPxls.begin();
  eightPxls.fill(lowRed,7);
  eightPxls.show(); // Initialize all pixels to red

  Serial.println ("LED's Initialized");
}

//********* Make sure the device and app are on the same page ********************//
BLYNK_CONNECTED() {
  // Request Blynk server to re-send latest values for all pins
  Blynk.syncAll();
}

//********* Toggle LED*************************************//
void ledBlynk(){
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  terminal.flush();
}


//********************************************************************************************************//
//                                      Loop Function Are Here                                            //
//********************************************************************************************************//
//Keeping the basic stuff here
void basicLoop(){
  if (wifi.run() != WL_CONNECTED) {
    Serial.println("WiFi not connected!");
    delay(1000);
  }
  ArduinoOTA.handle();  //Don't even think about getting rid of this!
  Blynk.run();
}

void OTASetup(){
  ArduinoOTA.setHostname(OTA_HOSTNAME);
  ArduinoOTA.setPassword(OTA_PSW);

  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
  Serial.println("OTA ready");
}

// This function tries to connect to your WiFi network
void connectWiFi(){
  wifi.addAP(SECRET_SSID1, SECRET_PSW1); //Home
  #ifdef SECRET_SSID2
    wifi.addAP(SECRET_SSID2, SECRET_PSW2); //Guest
  #endif
  wifi.addAP(SECRET_SSID3, SECRET_PSW3); //Phone
  WiFi.hostname(DEVICE_NAME);//assign host name

  Serial.println("Connecting Wifi...");
  while (wifi.run() != WL_CONNECTED) {
    delay(250);
    Serial.print('.');
  }
    Serial.println("");
    Serial.println("Connected to ");
    Serial.println(WiFi.SSID());
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

}

// Terminal menu and input options
BLYNK_WRITE(V1){
  // if you type "help" into Terminal Widget - it will respond win what you can do
  String tempStr = param.asStr();
  tempStr.toLowerCase();

  // do somethin different depending on the tempStr
  if (tempStr == "ip") {
      terminal.print("Your IP address is: ") ;
      terminal.println(WiFi.localIP()) ;
  } else if (tempStr == "wifi"){
      terminal.print("You are conneted to: ") ;
      terminal.println(WiFi.SSID()) ;
  } else if (tempStr == "u"){
      String tempOpenClose = catsGoOut? "True":"False";
      terminal.println("Cats can go out? " + tempOpenClose);
      tempOpenClose = catsCanComeIn? "True":"False";
      terminal.println("Cats can come in? " + tempOpenClose);
      terminal.println("It is currently " + String(currentTemp) + "c");
      terminal.println("It will go down to " + String(minTemp) + "c tonight");
  } else if (tempStr == "c"){
      terminal.clear();
  } else if (tempStr == "r"){
      terminal.clear();
      terminal.println("Resetting arduino");
      resetFunc();
  } else if (tempStr == "help"){
      terminal.clear();
      terminal.println("For IP address type 'IP'");
      terminal.println("For Wifi SSID type 'WiFi'");
      terminal.println("For an update type 'U'");
      terminal.println("To clear the terminal type 'C'");
      terminal.println("To reset the arduino type 'R'");
      terminal.println("To see this again type 'Help'");
  } else {
      // Send it back
      terminal.print("You said:");
      terminal.write(param.getBuffer(), param.getLength());
      terminal.println();
      terminal.println("Type 'help' for more");
  }//End if else

  // Ensure everything is sent
  terminal.flush();
}// end BLYNK_WRITE

//**************************************************************************************************//
//                                      HELPER FUNCITONS                                            //
//**************************************************************************************************//

// returns string after seperator
String getValue(String data, char separator, int index){
    int found = 0;
    int strIndex[] = { 0, -1 };
    int maxIndex = data.length() - 1;

    for (int i = 0; i <= maxIndex && found <= index; i++) {
        if (data.charAt(i) == separator || i == maxIndex) {
            found++;
            strIndex[0] = strIndex[1] + 1;
            strIndex[1] = (i == maxIndex) ? i+1 : i;
        }
    }
    return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

//Print to both Blynk Terminal and Serial Monitor
void p(String txt,bool newLine){
  if (!newLine){
    Serial.print(txt);
    terminal.print(txt);
  }else{
    Serial.println(txt);
    terminal.println(txt);
  }
}
