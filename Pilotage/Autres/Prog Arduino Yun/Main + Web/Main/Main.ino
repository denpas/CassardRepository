#include <Bridge.h>
#include <BridgeServer.h>
#include <BridgeClient.h> 
 
BridgeServer server;
 
void setup() {

  pinMode(13, OUTPUT);
  pinMode(12, OUTPUT);
  pinMode(11, OUTPUT);
  pinMode(10, OUTPUT);
  digitalWrite(13, LOW);
  digitalWrite(12, LOW);
  digitalWrite(11, LOW);
  digitalWrite(10, LOW);
  
  Bridge.begin();
  server.listenOnLocalhost();
  server.begin();

  Serial.begin(9600);       // for debugging
}
 
void loop() {
  BridgeClient client = server.accept();
  Serial.println("Attente connexion client");
  digitalWrite(13, HIGH);
  
  if (client.connected()){
    digitalWrite(13, LOW);
    Serial.println("1 client connecté");
    String command = client.readString();
    command.trim();
    if (command == "temperature") {
      int val = analogRead(A1);
      client.print(val);
    }
    client.stop();
  }
 
  delay(10);
}

