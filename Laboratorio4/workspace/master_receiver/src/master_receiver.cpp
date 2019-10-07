#include "mraa.hpp"

#include <iostream>
using namespace std;

int main() {
	// Inicializar led conectado a GPIO y controlador de I2C
	mraa::Gpio* d_pin = NULL;

	d_pin = new mraa::Gpio(3, true, true);

    mraa::I2c* i2c;
    i2c = new mraa::I2c(0);
    //setea la direccion con la que se va a comunicar con el slave.
    i2c->address(8);

    uint8_t rx_tx_buf[32];

    // Indefinidamente
    for (;;) {

    	// Apagar led y recibir por I2C
    	sleep(1);
    	d_pin->write(0);
    	//lee 6 bytes del puntero rx_tx_buf
    	i2c->read(rx_tx_buf, 6);

    	// Luego de un segundo, encender led e imprimir por stdout
    	sleep(1);
    	d_pin->write(1);
    	printf((char *) rx_tx_buf, "%s\n");

    	// Forzar la salida de stdout
    	fflush(stdout);
    }

    return 0;
}
