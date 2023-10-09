#include <SPI.h>
#include <EthernetENC.h>

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };  // ethernet module MAC address
IPAddress server(10,1,1,219);                        // server IP address, without DNS
//char server[] = "www.site.com";                     // if your server has a DNS name, use this variable for connection

// static configuration variables
IPAddress staticIp(10, 1, 1, 167);
IPAddress staticDnsServer(8, 8, 8, 8);

EthernetClient client;                                // ethernet client object

void setup() {
  // configure CS pin
  Ethernet.init(8);

  // begin serial connection
  Serial.begin(9600);

  // wait some seconds to let the module start
  delay(5000);
  
  Serial.println("");
  Serial.println("========Start=========");

  // initialize ethernet module with static configuration
  Ethernet.begin(mac, staticIp, staticDnsServer);

  // wait to let the module properly initialize
  delay(1000);

  // connection to the server
  Serial.print("Connecting to: ");
  Serial.print(server);
  Serial.println("...");
  
  // notify successful connection
  while (!client.connect(server, 80)) {
    Serial.print("Connection failed, trying again...");
    delay(1000);
  }

  Serial.print("Connected to ");
  Serial.println(client.remoteIP());

  // simple GET request
  client.println("GET /test/ HTTP/1.1");
  client.println("Host: 10.1.1.219");
  client.println("User-Agent: Arduino/1.0");
  client.println("Connection: close");
  client.println();

}

void loop() {

  // if I got a response from the server, read and print to serial
  int len = client.available();
  if (len > 0) {
    byte buffer[80];
    if (len > 80) len = 80;
    client.read(buffer, len);
    Serial.write(buffer, len);
  }

  // disconnection
  if (!client.connected()) {

    Serial.println();
    Serial.println("Disconnected");
    client.stop();

    // stop execution
    while (true) {
      delay(1);
    }
  }
}