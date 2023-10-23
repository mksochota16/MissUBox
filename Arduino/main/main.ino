#include <SPI.h>
#include <EthernetENC.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>

const char TEXT_MESSAGE = '0';
const char SPIN = '1';

const byte BUZZER_PIN = 9;
const byte SERVO_PIN = 10;


byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };  // ethernet module MAC address
IPAddress server(10,1,1,219);                        // server IP address, without DNS

EthernetClient client;
LiquidCrystal_I2C lcd(0x27, 16, 2); // I2C address 0x27, 16 column and 2 rows
Servo servo;

char responseBodyBuffer[128];
char textBuffer[32];
int sinceLCDUpdated = 0;
int servoPos = 0;

void setup() {
  // configure CS pin
  Ethernet.init(8);

  // begin serial connection
  Serial.begin(9600);

  // wait some seconds to let the module start
  delay(5000);
  
  Serial.println("========Start=========");

  
  servo.attach(SERVO_PIN);
  delay(100);
  servo.write(0);
  delay(200);
  servo.detach();


  lcd.init();
  lcd.clear();
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("init...");

  // initialize ethernet module with static configuration
  if(Ethernet.begin(mac) == 0){
    Serial.println("Error ethernet init");
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("ETH ERROR");
    while(true){
      delay(5000);
    }
  }

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Brak wiadomosci");
  lcd.setCursor(0,1);
  lcd.print("I tak kocham <3");

  // wait to let the module properly initialize
  delay(1000);

  // connection to the server
  Serial.print("Connecting to: ");
  Serial.print(server);
  Serial.println("...");
}

void ethDelay(unsigned long milliseconds) {
  const unsigned d = 1;
  while (milliseconds > d) {
    Ethernet.maintain();
    delay(d);
    milliseconds -= d;
  }
  Ethernet.maintain();
  delay(milliseconds);
}


void makeRequest() {
  // notify successful connection
  while (!client.connect(server, 80)) {
    Serial.println("Connection failed, trying again...");
    delay(1000);
  }

  //Serial.print("Connected to ");
  //Serial.println(client.remoteIP());

  // simple GET request
  client.println("GET /?api_key=123 HTTP/1.1");
  client.println("Host: 10.1.1.219");
  client.println("User-Agent: Arduino/1.0");
  client.println("Connection: close");
  client.println();

}

void messageSound(){
  tone(BUZZER_PIN, 900);
  ethDelay(120);      
  noTone(BUZZER_PIN);
  ethDelay(30);    
  tone(BUZZER_PIN, 1050);
  ethDelay(120);        
  noTone(BUZZER_PIN);
  ethDelay(30);    
  tone(BUZZER_PIN, 1200); 
  ethDelay(120);        
  noTone(BUZZER_PIN);
}

void handleResponse() {
  for(int i=1; i< strlen(responseBodyBuffer); i++){
    if(responseBodyBuffer[i] == '{') { // this marks the start of an action
      char type = responseBodyBuffer[i+8]; //this is the place of the type char
      switch (type) { 

        case SPIN:
          Serial.println("SPIN");
          servo.attach(SERVO_PIN);
          for (servoPos = 0; servoPos <= 180; servoPos += 1) { // goes from 0 degrees to 180 degrees
            servo.write(servoPos);              // tell servo to go to position in variable 'pos'
            ethDelay(15);                       // waits 15ms for the servo to reach the position
          }

          for (servoPos = 180; servoPos >= 0; servoPos -= 1) { // goes from 180 degrees to 0 degrees
            servo.write(servoPos);              // tell servo to go to position in variable 'pos'
            ethDelay(15);                       // waits 15ms for the servo to reach the position

          }
          servo.detach();
          i = i+20;
          break; 

        case TEXT_MESSAGE:
          Serial.println("MESSAGE");
          for(int a=i+19; a< strlen(responseBodyBuffer); a++){
            if (responseBodyBuffer[a] == '\"' || a-i-19>30){
              textBuffer[a-i-19] = '\0';
              i=a+2;
              break;
            }
            textBuffer[a-i-19] = responseBodyBuffer[a];
          }
          lcd.clear();
          lcd.backlight();
          sinceLCDUpdated = 0;
          for(int c=0; c<strlen(textBuffer);c++){
            if(c==15 && textBuffer[c] != ' '){
              lcd.print('-');
              lcd.setCursor(0,1);
              lcd.print(textBuffer[c]);
            }
            else if (c==15){// the last character in this line is space
              lcd.print(' ');
              lcd.setCursor(0,1);
            }
            else{
              lcd.print(textBuffer[c]);
            }
            
          }

          messageSound();
          break;
      }


    }
  }


}

void readAndStop() {
// connectLoop controls the hardware fail timeout
  int connectLoop = 0;
  char inChar;
  bool isResponseBody = false;
  int responseBodyStartsAt = 0;

  while(client.connected()) {

    int availableBytes = client.available();
    for(int i=0; i<availableBytes; i++) {
      inChar = client.read();
      if (inChar == '{' && !isResponseBody) {
        isResponseBody = true;
        responseBodyStartsAt = i;
      }
      if (isResponseBody) {
        if(i - responseBodyStartsAt > 127) {
          break;
        }
        responseBodyBuffer[i - responseBodyStartsAt] = inChar;
      }
      //Serial.print(inChar);
      // set connectLoop to zero if a packet arrives
      connectLoop = 0;
    }

    if (isResponseBody){
      if (availableBytes-responseBodyStartsAt > 127) {
        responseBodyBuffer[127] = '\0';
      }
      else {
        responseBodyBuffer[availableBytes-responseBodyStartsAt] = '\0';
      }
      Serial.println("Received response");
      handleResponse();
    }

    connectLoop++;

    // if more than 10000 milliseconds since the last packet
    if(connectLoop > 10000) {
      // then close the connection from this end.
      Serial.println(F("\r\nTimeout"));
      client.stop();
    }
    // this is a delay for the connectLoop timing
    ethDelay(1);
  }

  //Serial.println(F("\r\ndisconnecting."));
  // close client end
  client.stop();
}

void loop() {
  makeRequest();
  readAndStop();
  sinceLCDUpdated++;
  if (sinceLCDUpdated>100){
    sinceLCDUpdated=101;
    lcd.noBacklight();
  }
  ethDelay(15000);
}
