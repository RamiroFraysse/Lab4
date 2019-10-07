// Wire Slave Sender
// by Nicholas Zambetti <http://www.zambetti.com>

// Demonstrates use of the Wire library
// Sends data as an I2C/TWI slave device
// Refer to the "Wire Master Reader" example for use with this

// Created 29 March 2006

// This example code is in the public domain.


#include <Wire.h>

void setup() {
  Serial.begin(9600);
  //Inicia la libreria wire
  Wire.begin(8);                // join i2c bus with address #8//Unirse al bus i2c con la direccion 8.
  Wire.onRequest(requestEvent); // register event.Llama a esta funcion cuando el maestro solicite datos de un esclavo.
}

void loop() {
  delay(100);
  
}

// function that executes whenever data is requested by master
// this function is registered as an event, see setup()
void requestEvent() {
  Serial.println("Hola");
  int x = Wire.write("helloo"); // respond with message of 6 bytes
  Serial.println(x);
  // as expected by master
}
