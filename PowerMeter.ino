/**
******************************************************
* @file Sensor.ino
* @brief Sensor para temperatura, viento, ....
* @author Julian Salas Bartolome
* @version 1.0
* @date Julio 2020
*
*
*******************************************************/

#include "ServerPic.h"
#include "IO.h"

int nVuelta = 0;

void setup() {
  	#ifdef Debug														//Usamos el puereto serie solo para debugar	
		Serial.begin(9600);												//Si no debugamos quedan libres los pines Tx, Rx para set urilizados
		Serial.println("Iniciando........");
	#endif
	EEPROM.begin(256);													//Reservamos zona de EEPROM
																		//Ampliamos la zona de EEPROM para almacenar umbrales de todos los sensores	

	//BorraDatosEprom ( 0, 256);											//Borramos 128 bytes empezando en la posicion 0		

  	pinMode(PinReset, INPUT_PULLUP);           							//Configuramos el pin de reset como entrada



	if ( LeeByteEprom ( FlagConfiguracion ) == 0 )						//Comprobamos si el Flag de configuracion esta a 0
	{																	// y si esta
		ModoAP();														//Lo ponemos en modo AP
	}else{																//Si no esta
		#ifdef LedRojoVerde
			EnciendeLedRojo();
		#endif	
		if ( ClienteSTA() )												//Lo poenmos en modo STA y nos conectamos a la SSID
		{																//Si ha conseguido conectarse a ls SSID en modo STA
	        if ( ClienteServerPic () )									//Intentamos conectar a ServerPic
    		{															//Si se consigue
//				CheckFirmware();    									//Comprobamos si el firmware esta actualizado a la ultima version
		    	#ifdef Debug
        			Serial.println(" ");
        			Serial.println("Conectado al servidor");
      			#endif 
      			cSalida = " " ;

				DataConfig aCfg = EpromToConfiguracion ();     							 //Leemos la configuracin de la EEprom
				char USUARIO[1+aCfg.Usuario.length()]; 
				(aCfg.Usuario).toCharArray(USUARIO, 1+1+aCfg.Usuario.length());          //Almacenamos en el array USUARIO el nombre de usuario 
				cDispositivo = USUARIO;
				lHomeKit = aCfg.lHomeKit;												 //Almacenamos el estado de habilitacion de HomeKit	
				lPush = aCfg.lPush;														 //Almacenamos el estado de habilitacion de Push
				char CLIENTEPUSH[1+aCfg.Push.length()]; 							
				(aCfg.Push).toCharArray(CLIENTEPUSH, 1+1+aCfg.Push.length());            //Almacenamos en el array CLIENTEPUSH el nombre del cliente push 
				cPush = CLIENTEPUSH;
				lEstadisticas = aCfg.lEstadisticas;										 //Almacenamos el estado de habilitacion de Estadisticas 							 



				if ( lEstadisticas )													 //Si están habilitadas las estadisticas, actualizamos el numero de inicios
				{
					GrabaVariable ("inicios", 1 + LeeVariable("inicios") );
				}
				if (lPush)																 //Si esta habilitado el Push, notificamos el arranque
				{
					cSalida = "sendpush-:-"+cPush+"-:-"+cDispositivo+"-:-Inicio del modulo ";
					MensajeServidor(cSalida);			
					cSalida = String(' ');	
				}

    		}
    	}	
         wdt_reset();							                                          //Refrescamos el perro guardia n para que no se resetee
	}
}


void loop() {


	    long Inicio, Fin;

 		/*----------------
 		Comprobacion Reset
 		------------------*/

		TestBtnReset (PinReset);

 		/*----------------
 		Comprobacion Conexion
 		------------------*/
		if ( TiempoTest > 0 )
		{
			if ( millis() > ( nMiliSegundosTest + TiempoTest ) )			//Comprobamos si existe conexion  
			{

				nMiliSegundosTest = millis();
				if ( !TestConexion(lEstadisticas) )							//Si se ha perdido la conexion
				{
					lEstado = 0;
					lConexionPerdida = 1;									//Ponemos el flag de perdida conexion a 1


				}else{														//Si existe conexion
					if ( lConexionPerdida )									//Comprobamos si es una reconexion ( por perdida anterior )
					{														//Si lo es
						lConexionPerdida = 0;								//Reseteamos flag de reconexion

					}	
				}	

 	   		}	
    	}
 		/*----------------
 		Medida del snesor
 		------------------*/
 		if (lOnOff)															//Si tenemos habilitado el termostato										
 		{
 				switch(nVuelta) {

					Case 0:
						MensajeServidor("medida-:-Voltimetro-:-"+LeeVoltaje()); 	
						break;	
					Case 1:	
						MensajeServidor("medida-:-Amperimetro-:-"+LeeCorriente()); 	
						break;
 				}
 				nVuelta++:
 				if ( nVuelta == 2)
 				{
 					nVuelta = 0;
 				}
		}	
		delay(1000);
 		/*----------------
 		Analisis comandos
 		------------------*/
		oMensaje = Mensaje ();								 			//Iteractuamos con ServerPic, comprobamos si sigue conectado al servidor y si se ha recibido algun mensaje

		if ( oMensaje.lRxMensaje)										//Si se ha recibido ( oMensaje.lRsMensaje = 1)
		{
	    	#ifdef Debug				
				Serial.println(oMensaje.Remitente);						//Ejecutamos acciones
				Serial.println(oMensaje.Mensaje);
			#endif	
			//En este punto empieza el bloque de programa particular del dispositivo segun la utilizacion					
			//--------------------------------------------------------------------------------------------
			//Comandos generales de Dispositivo
			//
			// On.- Habilita el dispositivo
			// Off.- Deshabilita el dispositivo
			// Change.- Cambia el estado ( habilita/deshabilita ) del dispositivo
			// ChangeGet.- Cambia el estado ( habilita/deshabilita ) del dispositivo y responde del nuevo estado al remitente
			// Get.- Responde al remitente con el estado del dispositivo
			//--------------------------------------------------------------------------------------------

			if (oMensaje.Mensaje == "On")								//Si se recibe "On", se pone PinSalida a '1'
			{	
				DispositivoOn();	
				cSalida = "On";											//Se pone cSalida a 'On' para  se envie a HomeKit 
			}
			if (oMensaje.Mensaje == "Off")								//Si se recibe "Off", se pone PinSalida a '0'
			{	
				DispositivoOff();	
				cSalida = "Off";										//Se pone cSalida a 'Off' para  se envie a HomeKit 
			}
			if (oMensaje.Mensaje == "Change")							//Si se recibe 'Change', cambia el estado de PinSalida  y se pone cSalida con el nuevo estado para comunicar a Homekit
			{	
				if ( GetDispositivo() )
				{
					DispositivoOff();
					cSalida = "Off";					
				}else{
					DispositivoOn();
					cSalida = "On";					
				}
			}
			if (oMensaje.Mensaje == "ChangeGet")						//Si se recibe 'ChangeGet', cambia el estado de PinSalida, devuelve el nuevo estado al remitente  y se pone cSalida con el nuevo estado para comunicar a Homekit
			{	
				if ( GetDispositivo() )
				{
					DispositivoOff();
					cSalida = "Off";					
				}else{
					DispositivoOn();
					cSalida = "On";					
				}
				oMensaje.Mensaje = cSalida;								//Confeccionamos el mensaje a enviar hacia el servidor	
				oMensaje.Destinatario = oMensaje.Remitente;
				EnviaMensaje(oMensaje);									//Y lo enviamos
			}			
			if (oMensaje.Mensaje == "Get")								//Si se recibe 'Get', se devuelve el estado de PinSalida al remitente
			{	
				if ( GetDispositivo() )
				{
					cSalida = String('On');								//No ha habido cambio de estado, Vaciamos cSalida para que no se envie a HomeKit 
				}else{
					cSalida = String('Off');							//No ha habido cambio de estado, Vaciamos cSalida para que no se envie a HomeKit 
				}
				oMensaje.Mensaje = cSalida;
				oMensaje.Destinatario = oMensaje.Remitente;
				EnviaMensaje(oMensaje);	
				cSalida = String(' ');									//No ha habido cambio de estado, Vaciamos cSalida para que no se envie a HomeKit 
			}	
			if (oMensaje.Mensaje == "GetVoltaje")						//Si se recibe 'GetVoltaje', se devuelve el valor del voltaje al remitente
			{	
				cSalida = LeeVoltaje();
				oMensaje.Mensaje = cSalida;
				oMensaje.Destinatario = oMensaje.Remitente;
				EnviaMensaje(oMensaje);	
				cSalida = String(' ');									//No ha habido cambio de estado, Vaciamos cSalida para que no se envie a HomeKit 
			}	
			if (oMensaje.Mensaje == "GetCorriente")						//Si se recibe 'GetCorriente', se devuelve el valor de la corriente al remitente
			{	
				cSalida = LeeCorriente();
				oMensaje.Mensaje = cSalida;
				oMensaje.Destinatario = oMensaje.Remitente;
				EnviaMensaje(oMensaje);	
				cSalida = String(' ');									//No ha habido cambio de estado, Vaciamos cSalida para que no se envie a HomeKit 
			}				
			if (oMensaje.Mensaje == "GetFrecuencia")					//Si se recibe 'GetFrecuencia', se devuelve el valor de la frecuencia al remitente
			{	
				cSalida = LeeFrecuencia();
				oMensaje.Mensaje = cSalida;
				oMensaje.Destinatario = oMensaje.Remitente;
				EnviaMensaje(oMensaje);	
				cSalida = String(' ');									//No ha habido cambio de estado, Vaciamos cSalida para que no se envie a HomeKit 
			}	
			if (oMensaje.Mensaje == "GetPotencia")						//Si se recibe 'GetPotencia', se devuelve el valor de la potencia al remitente
			{	
				cSalida = LeePotencia();
				oMensaje.Mensaje = cSalida;
				oMensaje.Destinatario = oMensaje.Remitente;
				EnviaMensaje(oMensaje);	
				cSalida = String(' ');									//No ha habido cambio de estado, Vaciamos cSalida para que no se envie a HomeKit 
			}	
			if (oMensaje.Mensaje == "GetEnergia")						//Si se recibe 'GetEnergia', se devuelve el valor de la energia al remitente
			{	
				cSalida = LeeEnergia();
				oMensaje.Mensaje = cSalida;
				oMensaje.Destinatario = oMensaje.Remitente;
				EnviaMensaje(oMensaje);	
				cSalida = String(' ');									//No ha habido cambio de estado, Vaciamos cSalida para que no se envie a HomeKit 
			}
			if (oMensaje.Mensaje == "GetFactorPotencia")				//Si se recibe 'GetFactorPotencia', se devuelve el valor del factor de potencia al remitente
			{	
				cSalida = LeeFactorPotencia();
				oMensaje.Mensaje = cSalida;
				oMensaje.Destinatario = oMensaje.Remitente;
				EnviaMensaje(oMensaje);	
				cSalida = String(' ');									//No ha habido cambio de estado, Vaciamos cSalida para que no se envie a HomeKit 
			}							
	 		/*----------------
 			Actualizacion ultimo valor
 			------------------*/
			if ( cSalida != String(' ') )				//Si hay cambio de estado
			{	
				EnviaValor (cSalida);					//Actualizamos ultimo valor
			}


	 		/*----------------
 			Notificacion HomeKit
 			------------------*/
			#ifdef HomeKit
				if ( cSalida != String(' ') && oMensaje.Remitente != ( cDispositivo + String("_") ) && lHomeKit ) //Si está habililtadoHomeKit y hay cambio de estado en el dispositivo, se notifica a HomeKit
				{
					oMensaje.Destinatario = cDispositivo + String("_");
					oMensaje.Mensaje = cSalida;
					EnviaMensaje(oMensaje);
				}
			#endif		
			
			cSalida = String(' ');										//Limpiamos cSalida para iniciar un nuevo bucle

			if ( lEstadisticas )									 	//Si están habilitadas las estadisticas, actualizamos el numero de comandos recibidos
			{
				GrabaVariable ("comandos", 1 + LeeVariable("comandos") );
			}	

			nMiliSegundosTest = millis();		
	

		}

	    wdt_reset(); 													//Refrescamos WDT

}

