
/**
******************************************************
* @file ServerPic.h
* @brief Documentacion de Modulo Sensor
* @author Julian Salas Bartolome
* @version 1.0
* @date Julio 2020
*
*
* Compilado con Stino ESP 8266 2.5.2
* Opciones de compilacion
* 	Placa: 					Generic ESP8266 Module  ( para ESP01 y ESP11 )
* 	CPU Frecuency:			80 MHZ
* 	CRYSTAL Frecuency:		26 MHZ
*	Debul Level:			none
* 	Debug Pot:				DisabLedVerde
*	Esase Flash:			Only sketch
*	Expressid FW:			nonos-sdk 2.2.1 (legacy)
*   Exceptiosn:				DisabLedVerde	
*	Flash Frecuency:		40 MHZ
*	Flash Mode:				DOUT ( compatible )
*	Flash Size:				4M
*	IwIP Variant:			v2 Lower Memory
*	Reset Method:           ck
*	SSL SUport:             ALL SSL chiphers (most compatible)
*	Upload Speed:			115200
*	VTables:				Flash
*	
*	version: 1.0
*   Properties: xtal=80,CrystalFreq=26,lvl=None____,dbg=DisabLedVerde,wipe=none,sdk=nonosdk221,exception=disabLedVerde,FlashFreq=40,FlashMode=dout,eesz=1M,ip=lm2f,ResetMethod=ck,ssl=all,baud=115200,vt=flash 
*	version: 2.0
* Añadidos distinttos Hardwares
*******************************************************/

#ifndef SERVERPIC_H
	#define SERVERPIC_H

	//----------------------------------------------
	//Includes
	//----------------------------------------------
	#include <ESP8266WiFi.h>
	#include <WiFiClient.h> 
	#include <ESP8266WebServer.h>
	#include <WiFiUdp.h>
	#include <ArduinoOTA.h>



	#include "Global.h"
	#include "Configuracion.h"
	#include "Eprom.h"
	#include "ModoAP.h"
	#include "ModoSTA.h"
	#include "Servidor.h"


	#include "PZEM004Tv30.h"


	//------------------
	//Hardware Utilizado 
	//------------------

		#define Placa "NODEMCU"
		#define Modelo "ESP12"
		#define Ino "PowerMeter"        		


			


	//----------------------------
	//CARACTERISTICAS DISPOSITIVO
	//----------------------------
	#define VIno "1.0"						//Version del programa principal
	#define VBuild  "3"
	//---------------------------------
	//CARACTERISTICAS DE LA COMPILACION
	//--------------------------------
	#define Compiler "Stino";				//Compilador
	#define VCore "2.5.2";					//Versión del Core Arduino

	#include "IO.h";


	//----------------------------------------------
	//HOME KIT
	//----------------------------------------------
	#define HomeKit
	//----------------------------------------------
	//Teimpo de Test de conexion
	//----------------------------------------------
	#define TiempoTest	8000												//Tiempo en milisegundos para Test de conexion a servidor


	//----------------------------------------------
	//DEBUG
	//----------------------------------------------
	#define Debug



	//----------------------------
	//Declaracion de funciones Generales
	//----------------------------

	
	boolean GetDispositivo (void);
	void DispositivoOn (void);
	void DispositivoOff (void);

	//----------------------------
	//Declaracion de funciones PARTICULARES
	//----------------------------
	String LeeVoltaje (void);
	String LeeFrecuencia (void);
	String LeeCorriente (void);
	String LeePotencia(void);
	String LeeEnergia (void);
	String LeeFactorPotencia (void);

	//----------------------------
	//Declaracion de variables UNIVERSALES
	//----------------------------
	ESP8266WebServer server(80);											//Definimos el objeto Servidor para utilizarlo en Modo AP
	WiFiClient Cliente;														//Definimos el objeto Cliente para utilizarlo en Servidor
	Telegrama oMensaje;									 					//Estructura mensaje donde se almacenaran los mensajes recibidos del servidor

	String cSalida;															//Variable donde se deja el estado ( String ) para mandar en mensaje a ServerPic
	boolean lEstado = 0;													//Variable donde se deja el estado del dispositivo para reponer el estado en caso de reconexion
	boolean lConexionPerdida = 0;											//Flag de conexion perdida, se pone a 1 cuando se pierde la conexion. Se utiliza para saber si se ha perdido la conexion para restablecer estado anterior de la perdida

	boolean lHomeKit;													    //Flag para habililtar HomeKit ( 1 Habilitado, 0 deshabilitado )
	boolean lEstadisticas;													//Flag para habilitar Estadisticas ( 1 Habilitado, 0 Deshabilitado )
 	boolean lPush;															//Flag para habilitar Push ( 1 Habilitado, 0 Deshabilitado )

	long nMiliSegundosTest = 0;												//Variable utilizada para temporizar el Test de conexion del modulo a ServerPic
	long nMilisegundosRebotes = 0;                							//Variable utilizada para temporizar el tiempo de absorcion de rebotes
	String cDispositivo = String(' ');										//Nombre del dispositivo, utilizado para conocer el nombre de su pareja en homeKit			
	String cPush = String(' ');												//Nombre del cliente push			


	boolean lOnOff = 1;														//Flag que indica si el PIR está habilitado o no												
	boolean lDispositivo = 0;												//Flag que indica que se ha dado la orden al un dispositivo remoto para que actue


	//----------------------------
	//Declaracion de variables PARTICULARES
	//----------------------------
	PZEM004Tv30 pzem(PinTXSensor, PinRXSensor);

	float Voltaje = 0;
	float Corriente = 0;
	float Potencia = 0;
	float Energia = 0;
	float Frecuencia = 0;
	float FactorPotencia = 0;


	

	//Variables donde se almacenan los datos definidos anteriormente para pasarlos a Serverpic.h
	//para mandar la información del Hardware y Software utilizados
	//En la libreria ServerPic.h estan definidos como datos externos y se utilizan en la funcion
	//Mensaje () para responder al comando generico #Info.
	//Con ese comando, el dispositivo mandara toda esta información al cliente que se lo pida
	// ESTOS DATOS NO SON PARA MODIFICAR POR USUARIO
	//----------------------------
	//Datos del programa principal
	//----------------------------
	String VMain = VIno;
	String Main = Ino; 
	String VModulo = VBuild;
	//----------------------------
	//Datos de compilación
	//----------------------------	
	String Compilador = Compiler;
	String VersionCore = VCore;

	//----------------------------
	//Datos de Hardware
	//----------------------------	
	String ModeloESP = Modelo;
	String Board = Placa;




/**
******************************************************
* @brief Devuelve el estado del dispositivo
*
* @return devuelve <b>1</b> si el dispositivo esta habilitado o <b>0<\b> en caso de que no lo este
*/
boolean GetDispositivo (void)
{

	return ( lOnOff );

}
/**
******************************************************
* @brief Habilita el dispositivo
*
*/
void DispositivoOn (void)
{
	lOnOff=1;
}
/**
******************************************************
* @brief Deshabilita el dispositivo
*
*/
void DispositivoOff (void)
{
 	lOnOff=0;
}


String LeeVoltaje (void)
{
	String cValor;
	char buffer[5];	
    Voltaje = pzem.voltage();
    if( !isnan(Voltaje) ){
        cValor = dtostrf(Voltaje, 1, 1, buffer);
    } else {
        cValor = "Error";
    }	
	return ( cValor );
}

String LeeFrecuencia (void)
{
	String cValor;
	char buffer[5];		
    Frecuencia = pzem.frequency();
    if( !isnan(Frecuencia) ){
        cValor = dtostrf(Frecuencia, 1, 1, buffer);
    } else {
        Serial.println("Error");
    }
	return ( cValor );
}    
String LeeCorriente (void)
{
	String cValor;
	char buffer[5];		
    Corriente = pzem.current();
    if( !isnan(Corriente) ){
        cValor = dtostrf(Corriente, 1, 1, buffer);
    } else {
        Serial.println("Error");
    }
	return ( cValor );
}   
String LeePotencia (void)
{
	String cValor;
	char buffer[5];		
    Potencia = pzem.power();
    if( !isnan(Potencia) ){
        cValor = dtostrf(Potencia, 1, 1, buffer);
    } else {
        Serial.println("Error");
    }
	return ( cValor );
}   
String LeeEnergia (void)
{
	String cValor;
	char buffer[5];		
    Energia = pzem.energy();
    if( !isnan(Energia) ){
        cValor = dtostrf(Energia, 1, 1, buffer);
    } else {
        Serial.println("Error");
    }
	return ( cValor );
}   
String LeeFactorPotencia (void)
{
	String cValor;
	char buffer[5];		
    FactorPotencia = pzem.energy();
    if( !isnan(FactorPotencia) ){
        cValor = dtostrf(FactorPotencia, 1, 1, buffer);
    } else {
        Serial.println("Error");
    }
	return ( cValor );
}   
#endif