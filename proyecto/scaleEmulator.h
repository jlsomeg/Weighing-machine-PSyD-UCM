/*-------------------------------------------------------------------
**
**  Fichero:
**    scaleEmulator.h  29/8/2016
**
**    (c) J.M. Mendias
**    Programación de Sistemas y Dispositivos
**    Facultad de Informática. Universidad Complutense de Madrid
**
**  Propósito:
**    Contiene las definiciones de los prototipos de las funciones
**    necesarias para emular la lectura de un sensor de peso
**
**  Notas de diseño:
**    - Este emulador hace uso en exclusividad de los siguientes
**      recursos: TIMER4, TIMER5, LEDS y PBS
** 
**-----------------------------------------------------------------*/

#ifndef __SCALEEMULATOR_H__
#define __SCALEEMULATOR_H__

#include <fix_types.h>

/*
** Inicializa el emulador del sensor de peso
**
** Tras ejecutar esta funcion, con la presion de cualquier pulsador podran
** emularse alternativamente las acciones de depositar mercancia sobre la bascula
** y de retirar la marcancia de la bascula.
**
** La presencia emulada de mercancia sobre la bascula se visualiza manteniendo
** ambos leds encendidos.
*/
void scale_init( void );

/*
** Emula la lectura del sensor de peso y su traduccion a Kg en formato Q6.10
**
** Las lecturas del sensor de peso no se estabilizan hasta transcurrido un cierto
** tiempo desde el deposito/retirada emulada de mercancia (i.e. tras la presion 
** de un pulsador sucesivas llamadas a esta funcion devuelven valores distintos 
** durante un cierto tiempo ).
**
** En ausencia emulada de mercancia, la lectura del sensor se estabiliza a 0.
** En presencia emulada de mercancia, la lectura del sensor se estabiliza a un 
** valor aleatorio
*/
ufix16 scale_getWeight( void );

#endif 
