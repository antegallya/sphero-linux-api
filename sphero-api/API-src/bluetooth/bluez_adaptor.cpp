/*************************************************************************
                           bluez_adaptor  -  description
                             -------------------
    début                : mar. 17 mars 2015
*************************************************************************/

//------ Réalisation de la classe <bluez_adaptor> (fichier bluez_adaptor.cpp) --

//---------------------------------------------------------------- INCLUDE

//-------------------------------------------------------- Include système
#include <iostream>
#include <sys/select.h>
#include <fcntl.h>

using namespace std;
//------------------------------------------------------ Include personnel
#include "bluez_adaptor.h"


//------------------------------------------------------------- Constantes

//---------------------------------------------------- Variables de classe

//----------------------------------------------------------- Types privés


//----------------------------------------------------------------- PUBLIC
//-------------------------------------------------------- Fonctions amies

//----------------------------------------------------- Méthodes publiques
int bluez_adaptor::connection(const char* address)
{
	if(_connected)
	{
		fprintf(stderr, "Connection attempt was made but peripheral was already connected\n");
		return _bt_socket;
	}


	bdaddr_t bt_address;

	//Conversion de l'adresse 
	if(str2ba(address, &bt_address))
	{
		perror("BT address conversion");
		return -1;
	}

	struct sockaddr_rc dest_addr;
	dest_addr.rc_bdaddr = bt_address;
	dest_addr.rc_family = AF_BLUETOOTH; 
	dest_addr.rc_channel = (uint8_t) 1;

	//Création du socket de communication
	_bt_socket = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);	
	fcntl(_bt_socket, F_SETFL, O_NONBLOCK);

	if(connect(_bt_socket, (sockaddr*) &dest_addr, sizeof(dest_addr)) < 0)
	{
		perror("BT connection");
		return -1;
	}
	_connected = true;
	return _bt_socket;
}

bool bluez_adaptor::isConnected()
{
	return _connected;
}

int bluez_adaptor::disconnect(void)
{
	_connected = false;	
	pthread_cancel(_listening_thread);
	return close(_bt_socket);
}

//------------------------------------------------- Surcharge d'opérateurs


//-------------------------------------------- Constructeurs - destructeur


bluez_adaptor::bluez_adaptor ():
	bluetooth_connector(),
	_bt_socket(0),
	_connected(false)
{
#ifdef MAP
    cout << "Appel au constructeur de <bluez_adaptor>" << endl;
#endif
} //----- Fin de bluez_adaptor


bluez_adaptor::~bluez_adaptor ( )
// Algorithme :
//
{
#ifdef MAP
    cout << "Appel au destructeur de <bluez_adaptor>" << endl;
#endif
	if(_connected)
	{
		disconnect();
	}
} //----- Fin de ~bluez_adaptor


//------------------------------------------------------------------ PRIVE

//----------------------------------------------------- Méthodes protégées

//------------------------------------------------------- Méthodes privées


