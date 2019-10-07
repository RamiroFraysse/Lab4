#include "mraa.hpp"

#include <iostream>
using namespace std;

#define OBTENER_TEMP 10
#define OBTENER_MAX 20
#define OBTENER_MIN 30
#define OBTENER_PROM 40
#define OBTENER_TODO 50

#define PKG_START 0
#define PKG_OP 1
#define PKG_SIZE 2

#define RESPONDER_TEMP 11
#define RESPONDER_MAX 21
#define RESPONDER_MIN 31
#define RESPONDER_PROM 41
#define RESPONDER_TODO 51


#define START_BYTE 0x01
#define END_BYTE 0x02

// Variables Globales
uint8_t pkg_send[4];
uint8_t pkg_receive[256];
int tieneRespuesta;

// Inicializar led conectado a GPIO y controlador de I2C
// Comunicacion
mraa::Gpio* d_pin = NULL;
mraa::I2c* i2c;



// Funciones
int get_operacion();
void armar_mensaje(int op);
void send_mensaje();
int validar_respuesta();
void print_menu();
int opValido(uint8_t tipo);
void procesar_respuesta();

int main() {

	int operacion = 0;


	d_pin = new mraa::Gpio(3, true, true);

    i2c = new mraa::I2c(0);
    i2c->address(8);

    //Inicializacion del arreglo
	for(int i=0; i<256; i++)
		pkg_receive[i] = 0x00;

	print_menu();
	printf("\nOperacion (h for help) > ");
	operacion = get_operacion();

    // Indefinidamente
    for (;;) {
    	if (operacion == 6 || operacion == 'h')
    		print_menu();
    	else
    	{
			if (operacion > 0){

				armar_mensaje(operacion);

				send_mensaje();

				if(tieneRespuesta){
					if(validar_respuesta() < 0)
						printf("Invalid pkg receive \n");
					else
					{
						procesar_respuesta(); // Imprime por pantalla la respuesta
						// Luego de un segundo,
						sleep(1);
						//encender led e imprimir por stdout
						d_pin->write(1);
					}
				}
			}
			else printf("Ingrese una operacion valida\n");
    	}
    	// Forzar la salida de stdout
    	printf("\nOperacion (h for help) > ");
    	fflush(stdout);
    	fflush(stdin);
    	operacion = get_operacion();
    }
    return 0;
}

//Devuelve el codigo de operacion ingresado en caso de ser valido, de lo contrario retorna -1
int get_operacion(){
	char op;
	int res = scanf("%s",&op);
	if(res > 0){
		//Si esta entre 0 y 7
		if ((op > 48) && (op < 55)){
			tieneRespuesta = 1;
			return (int)(op - 48);
		}
		else{
			printf("Invalid Input \n");
		}
	}
	return (-1);
}

void armar_mensaje(int op){
	//start | codigo op | tamaño| end
	pkg_send[PKG_START] = START_BYTE; // Simbolo inicio
    switch (op){
    case 1:
        pkg_send[PKG_OP] = OBTENER_TEMP;
    	break;
    case 2:
    	pkg_send[PKG_OP] = OBTENER_MAX;
        break;
    case 3:
    	pkg_send[PKG_OP] = OBTENER_MIN;
        break;
    case 4:
    	pkg_send[PKG_OP] = OBTENER_PROM;
        break;
    case 5:
    	pkg_send[PKG_OP] = OBTENER_TODO;
        break;

    case '?':
    	printf("Invalid Operation \n");
    	break;
    }
    pkg_send[PKG_SIZE] = 0x04;	// Size = 4by
    pkg_send[3] = END_BYTE; // Simbolo Fin
}

void send_mensaje(){
	// Apagar led y envia por I2C
	sleep(1);
	d_pin->write(0);
	// Enviar mensaje data,size(data)
	i2c->write(pkg_send, pkg_send[2]);
}

int validar_respuesta(){
	//Chequea que se reciba una trama como se menciona a continuacion.
	// start|codigoOperacion|tamaño|dato|end
	pkg_receive[0] = i2c->readByte();//Lee el primer byte
	if(pkg_receive[0] == START_BYTE){
		pkg_receive[1] = i2c->readByte(); // codigo op
		if(pkg_receive[1] != END_BYTE && opValido(pkg_receive[1])){
			pkg_receive[2] = i2c->readByte(); // SIZE
			if(pkg_receive[2] > 4 && pkg_receive[2] < 256 && pkg_receive[2] != END_BYTE){
				int i = 2;
				while(pkg_receive[i] != END_BYTE && i < 256){
					i += 1;
					pkg_receive[i] = i2c->readByte();
				}
				if(i >= 256){
					printf("Error reading pkg: No END_BYTE found\n");
					return (-1);
				}
				return (1);
			} else printf("Error reading pkg: invalid SIZE\n");
		} else printf("Error reading pkg: invalid TYPE\n");
	} else printf("Error reading pkg: No START_BYTE found\n");
	return (-1);
}

int opValido(uint8_t tipo){
	return RESPONDER_TEMP == tipo || RESPONDER_MAX == tipo || RESPONDER_MIN == tipo || RESPONDER_PROM == tipo || RESPONDER_TODO == tipo;
}

void procesar_respuesta(){
	// Longitud payload 3 to pkg_receive[2] - 4
	union{
	  float valorf;
	  int valorI;
	  uint8_t valorByte[4];
	} valorRespuesta;
	valorRespuesta.valorI = 0;
	//Tamaño del dato a leer.
	int payloadSize = pkg_receive[2] - 4;
	switch(pkg_receive[1]){
		case RESPONDER_TEMP:
			//almacena en el arreglo los datos obtenidos.
			for(int i=0; i < payloadSize; i++){
				//Lee la parte entera y la parte decimal recibida.
				valorRespuesta.valorByte[i] = pkg_receive[pkg_receive[2] - i - 2];
			}
			printf("Valor Temperatura actual: %d\n", valorRespuesta.valorI);
			break;
		case RESPONDER_MAX:
			for(int i=0; i < payloadSize; i++){
				valorRespuesta.valorByte[i] = pkg_receive[pkg_receive[2] - i - 2];
			}
			printf("Valor Temperatura maxima: %d\n", valorRespuesta.valorI);
			break;
		case RESPONDER_MIN:
			for(int i=0; i < payloadSize; i++){
				valorRespuesta.valorByte[i] = pkg_receive[pkg_receive[2] - i - 2];
			}
			printf("Valor Temperatura minima: %d\n", valorRespuesta.valorI);
			break;
		case RESPONDER_PROM:
			for(int i=0; i < payloadSize; i++){
				valorRespuesta.valorByte[i] = pkg_receive[pkg_receive[2] - i - 2];

			}
			printf("Valor Temperatura promedio: %f\n", valorRespuesta.valorf);
			break;
		case RESPONDER_TODO:
			for(int i=0; i < 4; i++){
				valorRespuesta.valorByte[i] = pkg_receive[pkg_receive[2] - i - 2];
			}
			printf("Valor Temperatura promedio: %f\n", valorRespuesta.valorf);
			valorRespuesta.valorI = 0;
			for(int i=0; i < 2; i++){
				valorRespuesta.valorByte[i] = pkg_receive[pkg_receive[2] - i - 6];
			}
			printf("Valor Temperatura minima: %d\n", valorRespuesta.valorI);
			for(int i=0; i < 2; i++){
				valorRespuesta.valorByte[i] = pkg_receive[pkg_receive[2] - i - 8];
			}
			printf("Valor Temperatura maxima: %d\n", valorRespuesta.valorI);
			for(int i=0; i < 2; i++){
				valorRespuesta.valorByte[i] = pkg_receive[pkg_receive[2] - i - 10];
			}
			printf("Valor Temperatura actual: %d\n", valorRespuesta.valorI);
			break;
	}
}

void print_menu(){
	printf("______Operaciones de Temperatura_____\n");
	printf("1) Obtener Temperatura Actual \n");
	printf("2) Obtener Temperatura Maxima \n");
	printf("3) Obtener Temperatura Minima \n");
	printf("4) Obtener Temperatura Promedio \n");
	printf("5) Obtener Todo \n");
}

