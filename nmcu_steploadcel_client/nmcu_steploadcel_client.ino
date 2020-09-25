

/*WARNING!
 * while choosing GPIO pins on Node_MCU(NMCU)esp2866dev_brd care has to be taken 
 * for explaination check https://randomnerdtutorials.com/esp8266-pinout-reference-gpios/ with heading"Best Pins to Use – ESP8266"
 * NOTE: GPIO pin naming has been used and not the nomenclature on NodeMCU-ESP2866.Check above link for finding corresponding relation btwn GPIOnumberin and NMCU naming
*/
#include <SPI.h>                  //using SPI comm btwn Node_MCU(NMCU) and stepper motor driver for stepping and change in dir
#include <loopTimer.h>            //to check single loop time (for debugging)
#include <HighPowerStepperDriver.h> //https://github.com/pololu/high-power-stepper-driver-arduino
//#include <HX711.h>                //ADC for loadcell  https://github.com/bogde/HX711
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

#include <ESP8266WiFiMulti.h>
ESP8266WiFiMulti WiFiMulti;
/*#define calibration_factor 820    //This value is obtained using the SparkFun_HX711_Calibration sketch
#define DOUT  5                   // digi values from ADC to NMCU input    
#define CLK  4                    //NMCU outputs pulses to PD_SCK(CLK) of the ADC http://image.dfrobot.com/image/data/SEN0160/hx711_english.pdf
*/
//PINS 14,12,13,15 FOR NMCU_ESP CLK,MISO,MOSI,CS
const uint8_t CSPin = 15;         
/*declaring range parameters used to define the range where stepper motor should step()
btwn L & H no step()
below L step() in counter clock dir
above H step() in clock dir
*/
#define L  300              
#define H  500         

char ssid[] = "default";//"ISIS-IC-Guest";               // SSID of your home WiFi
char pass[] = "";               // password of your home WiFi
//WiFiServer server(80); 
                   
//Your IP address or domain name with URL path
const char* serverNameTension = "http://192.168.0.80/load_sens";
byte ledPin = 2;
String load_sens;
unsigned long previousMillis = 0;
const long interval = 100; 

/*IPAddress ip(192, 168, 0, 80);            // IP address of the server
IPAddress gateway(192, 168, 0, 1);           // gateway of your network
IPAddress subnet(255,255,255,0);          // subnet mask of your network*/
IPAddress server(192,168,0,80);       // the fix IP address of the server
WiFiClient client;

//HX711 scale;
HighPowerStepperDriver sd;  
// This period is the length of the delay in microseconds between steps, which controls the
// stepper motor's speed.  You can increase the delay to make the stepper motor
// go sLower.  If you decrease the delay, the stepper motor will go faster, but
// there is a limit to how fast it can go before it starts missing steps.
const uint16_t StepPeriodUs = 2;
const uint16_t StepPeriodUsL = 20;
const uint8_t SLPpullup =0; //tx

/*diff>100 release half rotation
 * diff(diff) within a range ---   do nothing
 * if diff(diff)
 */
void setup() 
{
  //pull sleep pin high to wake up the board
 pinMode(SLPpullup, OUTPUT);
 digitalWrite(SLPpullup, HIGH);
 
  //Serial.begin(9600);                //uncomment to see sensor values
  Serial.begin(115200);                   // only for debug
  /*WiFi.mode(WIFI_STA);
  WiFi.config(ip, gateway, subnet);       // forces to use the fix IP*/
  WiFi.begin(ssid, pass);                 // connects to the WiFi router
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
    
  }
  Serial.println("");
  Serial.println("Connected to WiFi");
  pinMode(ledPin, OUTPUT);
  /*server.begin();                         // starts the server

  Serial.print("IP: ");     Serial.println(WiFi.localIP());*/

/*  //for loadcell ADC HX711
  scale.begin(DOUT, CLK);              //Initializing library HX711.h with data output pin, clock input pin 
  scale.set_scale(calibration_factor); //This value is obtained by using the SparkFun_HX711_Calibration sketch
  scale.tare();                        //Assuming there is no weight on the scale at start up, reset the scale to 0
*/
  //for stepper motor
  SPI.begin();
  sd.setChipSelectPin(CSPin);

  // Give the driver some time to power up.
  delay(1);

  // Reset the driver to its default settings and clear latched status
  // conditions.
  sd.resetSettings();
  sd.clearStatus();

  // Select auto mixed decay.  TI's DRV8711 documentation recommends this mode
  // for most applications, and we find that it usually works well.
  sd.setDecayMode(HPSDDecayMode::AutoMixed);

  // Set the current limit. You should change the number here to an appropriate
  // value for your particular system.
  sd.setCurrentMilliamps36v4(3000);

  // Set the number of microsteps that correspond to one full step. page- 28:MODE,34:8.2.2.1 https://www.ti.com/lit/ds/symlink/drv8711.pdf
  sd.setStepMode(HPSDStepMode::MicroStep256);

  // Enable the motor outputs.
  sd.enableDriver();
}

void loop() {
  client.connect(server, 80);   // Connection to the server
  digitalWrite(ledPin, LOW);    // to show the communication only (inverted logic)
  /*WiFiClient client = server.available();
    if (client) {
      if (client.connected()) {
        //digitalWrite(ledPin, LOW);  // to show the communication only (inverted logic)
        //Serial.println(".");
        float T = client.read();    // receives the message from the client
        Serial.print("From client: "); Serial.println(T);
        client.flush();
        //client.println("Hi client! No, I am listening.\r"); // sends the answer to the client
        //digitalWrite(ledPin, HIGH);*/

        unsigned long currentMillis = millis();
        //Serial.println(currentMillis);
        if(currentMillis - previousMillis >= interval) {
          if((WiFi.status() == WL_CONNECTED)) {
             load_sens = httpGETRequest(serverNameTension);
             Serial.println("Tension: " + load_sens + " *g");
             // save the last HTTP GET Request
          previousMillis = currentMillis;
          }
          else{Serial.println("WiFi Disconnected");
          }
        }

          
         float T=load_sens.toFloat();
         manage_T(T);  
        //client.flush();
        digitalWrite(ledPin, HIGH);
         //task that sets direction and steps based on T value and pre-defined L & H values 
      }
      //client.stop();                // tarminates the connection with the client
  

  
  // put your main code here, to run repeatedly:
  //loopTimer.check(&Serial);        //FOR DEBUG
  //float T= scale.get_units();        //get loadcell values in g
  //Serial.println(T);               // for printing on serial monitor
  //manage_T(T);                       //task that sets direction and steps based on T value and pre-defined L & H values 
 
  

//stepping func using SPI 
void stepup()
  {
    { for(unsigned int x = 0; x < 25601; x++) //"25601"STEPS RESULT IN HALF ROTATION OF SHAFT WITH MICROSTEP256
      //for(unsigned int x = 0; x < 3201; x++)    //"3201"STEPS RESULT IN HALF ROTATION OF SHAFT WITH MICROSTEP32
      {sd.step(); 
      //Serial.println("herestepup");
      delayMicroseconds(StepPeriodUs); }
    }
  }

  void stepupL()
  {
    { for(unsigned int x = 0; x < 25601; x++) //"25601"STEPS RESULT IN HALF ROTATION OF SHAFT WITH MICROSTEP256
      //for(unsigned int x = 0; x < 3201; x++)    //"3201"STEPS RESULT IN HALF ROTATION OF SHAFT WITH MICROSTEP32
      {sd.step(); 
      delayMicroseconds(StepPeriodUs); }
    }
  }

void manage_T(float T){
  Serial.println("here");
 if(T < 0) {T= 0;}
if(T < L) { Serial.println("here2");sd.setDirection(0);stepup();}
else if (T > H) { Serial.println("here3");sd.setDirection(1);stepup();
}
}

String httpGETRequest(const char* serverName) {
          WiFiClient client;
          HTTPClient http;
            
          // Your IP address with path or Domain name with URL path 
          http.begin(client, serverName);
          
          // Send HTTP POST request
          int httpResponseCode = http.GET();
          
          String payload = "--"; 
          
          if (httpResponseCode>0) {
            Serial.print("HTTP Response code: ");
            Serial.println(httpResponseCode);
            payload = http.getString();
          }
          else {
            Serial.print("Error code: ");
            Serial.println(httpResponseCode);
          }
          // Free resources
          http.end();
        
          return payload;
}
