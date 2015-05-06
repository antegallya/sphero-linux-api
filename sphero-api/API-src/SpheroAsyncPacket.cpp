/*************************************************************************
                           SpheroAsyncPacket  -  description
                             -------------------
    début                : mar. 17 mars 2015
*************************************************************************/

//------ Réalisation de la classe <SpheroAsyncPacket> (fichier SpheroAsyncPacket.cpp) --

//---------------------------------------------------------------- INCLUDE

//-------------------------------------------------------- Include système
#include <iostream>
#include <sys/select.h>
#include <fcntl.h>

//------------------------------------------------------ Include personnel
#include "SpheroAsyncPacket.hpp"

//------------------------------------------------------------- Constantes

//---------------------------------------------------- Variables de classe

//----------------------------------------------------------- Types privés


//----------------------------------------------------------------- PUBLIC
//-------------------------------------------------------- Fonctions amies

//----------------------------------------------------- Méthodes publiques

//------------------------------------------------- Surcharge d'opérateurs

//-------------------------------------------- Constructeurs - destructeur
SpheroAsyncPacket::SpheroAsyncPacket(Sphero* sphero) : SpheroPacket(sphero)
{

}

SpheroAsyncPacket::~SpheroAsyncPacket()
{

}
//------------------------------------------------------------------ PRIVE

//----------------------------------------------------- Méthodes protégées

//------------------------------------------------------- Méthodes privées


