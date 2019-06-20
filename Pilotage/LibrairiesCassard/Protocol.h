
//---------------------------------------------------------------------------------------------------------------------------------------
//MODULE CONSTANTS
//---------------------------------------------------------------------------------------------------------------------------------------

const byte MODULE_TOURELLE = 1;
const byte MODULE_PROPULSION = 2;

//---------------------------------------------------------------------------------------------------------------------------------------
//PROTOCOL CONSTANTS
//---------------------------------------------------------------------------------------------------------------------------------------

const byte TOURELLE_SETUP 					= 1;
const byte TOURELLE_SETID 					= 2;
const byte TOURELLE_SETCHANGESPEED 			= 3;
const byte TOURELLE_SETAZIMUTELEVATION 		= 4;

const byte PAP_SETUP 						= 10;
const byte BRU_SETUP 						= 11;
const byte PROPULSION_FORWARDENGINE			= 12;
const byte PROPULSION_REVERSEENGINE			= 13;
const byte PROPULSION_STOPENGINE			= 14;
const byte PROPULSION_TURNRIGHT				= 15;
const byte PROPULSION_TURNLEFT				= 16;
const byte PROPULSION_GOSTRAIGHT			= 17;
const byte PROPULSION_SETPROPORTIONALMODE	= 18;

const byte END_OF_TRAME	   					= 255;