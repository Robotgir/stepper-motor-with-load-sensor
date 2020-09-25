
/*WARNING!
 * while choosing GPIO pins on Node_MCU(NMCU)esp2866dev_brd care has to be taken 
 * for explaination check https://randomnerdtutorials.com/esp8266-pinout-reference-gpios/ with heading"Best Pins to Use – ESP8266"
 * NOTE: GPIO pin naming has been used and not the nomenclature on NodeMCU-ESP2866.Check above link for finding corresponding relation btwn GPIOnumberin and NMCU naming
*/
//#include <SPI.h>                  //using SPI comm btwn Node_MCU(NMCU) and stepper motor driver for stepping and change in dir
#include <loopTimer.h>            //to check single loop time (for debugging)
//#include <HighPowerStepperDriver.h> //https://github.com/pololu/high-power-stepper-driver-arduino
#include <HX711.h>                //ADC for loadcell  https://github.com/bogde/HX711
#include <ESP8266WiFi.h>
#include <ESPAsyncWebServer.h>
#define calibration_factor 820    //This value is obtained using the SparkFun_HX711_Calibration sketch
#define DOUT  5                   // digi values from ADC to NMCU input    
#define CLK  4                    //NMCU outputs pulses to PD_SCK(CLK) of the ADC http://image.dfrobot.com/image/data/SEN0160/hx711_english.pdf

char ssid[] = "default";           // SSID of your home WiFi
char pass[] = "";            // password of your home WiFi

unsigned long askTimer = 0;

//IPAddress server(192,168,0,80);       // the fix IP address of the server
//WiFiClient client;
AsyncWebServer server(80);
//WiFiServer server(80);
//PINS 14,12,13,15 FOR NMCU_ESP CLK,MISO,MOSI,CS
//const uint8_t CSPin = 15;         
/*declaring range parameters used to define the range where stepper motor should step()
btwn L & H no step()
below L step() in counter clock dir
above H step() in clock dir
*/
#define L  300              
#define H  500         
IPAddress ip(192, 168, 0, 80);            // IP address of the server
IPAddress gateway(192, 168, 0, 1);           // gateway of your network
IPAddress subnet(255,255,255,0);          // subnet mask of your network
HX711 scale;
//HighPowerStepperDriver sd;  
// This period is the length of the delay in microseconds between steps, which controls the
// stepper motor's speed.  You can increase the delay to make the stepper motor
// go sLower.  If you decrease the delay, the stepper motor will go faster, but
// there is a limit to how fast it can go before it starts missing steps.
//const uint16_t StepPeriodUs = 2;

String readtension() {
  return String(scale.get_units());
}

void setup() 
{
  //Serial.begin(9600);                //uncomment to see sensor values
  Serial.begin(115200);               // only for debug
  WiFi.config(ip, gateway, subnet);       // forces to use the fix IP
  WiFi.begin(ssid, pass);             // connects to the WiFi router
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500); 
  }
  //for loadcell ADC HX711
  scale.begin(DOUT, CLK);              //Initializing library HX711.h with data output pin, clock input pin 
  scale.set_scale(calibration_factor); //This value is obtained by using the SparkFun_HX711_Calibration sketch
  scale.tare();                        //Assuming there is no weight on the scale at start up, reset the scale to 0


  server.on("/load_sens", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", readtension().c_str());
  });

  server.begin();
 /* //for stepper motor
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
  sd.setCurrentMilliamps36v4(1000);

  // Set the number of microsteps that correspond to one full step. page- 28:MODE,34:8.2.2.1 https://www.ti.com/lit/ds/symlink/drv8711.pdf
  sd.setStepMode(HPSDStepMode::MicroStep32);

  // Enable the motor outputs.
  sd.enableDriver();*/
  }

void loop() {

 /* client.connect(server, 80);   // Connection to the server
  //digitalWrite(ledPin, LOW);    // to show the communication only (inverted logic)
  Serial.println(".");
  //client.println("Hello server! Are you sleeping?\r");  // sends the message to the server
  int T= scale.get_units();        //get loadcell values in g
  client.write(T,2);
  //String answer = client.readStringUntil('\r');   // receives the answer from the sever
  //Serial.println("from server: " + answer);
  client.flush();
  //digitalWrite(ledPin, HIGH);
  //delay(2000);                  // client will trigger the communication after two seconds
 
  
  // put your main code here, to run repeatedly:
  //loopTimer.check(&Serial);        //FOR DEBUG
  //float T= scale.get_units();        //get loadcell values in g
  //Serial.println(T);               // for printing on serial monitor
  //manage_T(T);                       //task that sets direction and steps based on T value and pre-defined L & H values 
 
  
*/}
//stepping func using SPI 
/*void stepup()
  {
    { //for(unsigned int x = 0; x < 25601; x++) //"25601"STEPS RESULT IN HALF ROTATION OF SHAFT WITH MICROSTEP256
      for(unsigned int x = 0; x < 3201; x++)    //"3201"STEPS RESULT IN HALF ROTATION OF SHAFT WITH MICROSTEP32
      {sd.step(); 
      delayMicroseconds(StepPeriodUs); }
    }
  }

void manage_T(float T){
 if(T < 0) {T= 0;}
if(T < L) { sd.setDirection(0);stepup();}
else if (T > H) { sd.setDirection(1);stepup();
}
}*/
