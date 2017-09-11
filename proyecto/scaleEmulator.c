#include <s3c44b0x.h>
#include <s3cev40.h>
#include <leds.h>
#include <common_types.h>
#include "scaleEmulator.h"

static void pbs_isr( void )    __attribute__ ((interrupt ("IRQ")));
static void timer5_isr( void ) __attribute__ ((interrupt ("IRQ")));

static uint32 random( uint32 seed );

#define RANDOM ( (uint16)random( 0 ) )

#define TICKS2STABLE (15625)

/*******************************************************************/

static volatile ufix16 weightTo;      // Valor hacia el que convergeran las lecturas del sensor tras un deposito/retirada emulada de mercancia
static volatile ufix16 weightFrom;    // Valor desde el que convergeran las lecturas del sensor tras un deposito/retirada emulada de mercancia

/*******************************************************************/

void scale_init( void )
{
    weightFrom = 0;
    weightTo   = 0;

    random( (BCDMIN << 8) | BCDSEC );                          // Define la semilla del generador de numeros psudoaleatorios

    CLKCON |= (1<<10) | (1<<7) | (1 << 3);                     // Activa GPIO, BDMA y PWM

    PCONB &= ~(3<<9);                                          // PB[10] = out, PF[9] = out

    TCFG0 = (TCFG0 & ~(0xff << 16)) | (255 << 16);             // T4-T5 PRESCALER = 255 (1/(255-1))
    TCFG1 = (TCFG1 & ~(0xf << 16)) | (3 << 16);                // T4 DIVISOR = 16 (1/16)
    TCFG1 = (TCFG1 & ~(0xf << 20)) | (3 << 20);                // T5 DIVISOR = 16 (1/16)
    
    TCNTB4  = 0;                                               // T4 = 0
    TCON    = (TCON & ~(0xf << 20)) |  (1 << 21);              // Modo one shot, carga TCNT4, stop T4
    INTMSK |= BIT_TIMER5;                                      // Enmascara la interrupcion por fin de T4

    TCNTB5  = 0;                                               // T5 = 0
    TCON    = (TCON & ~(0xf << 24)) | (1 << 25);               // Modo one shot, carga TCNT5, stop T5
    INTMSK |= BIT_TIMER5;                                      // Enmascara la interrupcion por fin de T5
   
    PCONG    |= (3<<14) | (3<<12);                             // PG[7] = EINT7, PG[6] = EINT6
    PUPG     &= ~(0x3<<6);                                     // Pull-up enable
    EXTINT    = (EXTINT & ~(0xff<<24)) | (2<<28) | (2<<24);    // Indica que las interrupciones de pulsador se generen a flanco de bajada  
    pISR_PB   = (uint32) pbs_isr;                              // Instala la funcion pbs_down_isr como RTI por presion de pulsador
    EXTINTPND = BIT_LEFTPB | BIT_RIGHTPB;                      // Borra los bits de interrupción pendiente en el registro EXTINTPND
    I_ISPC    = BIT_PB;                                        // Borra el bit de interrupción pendiente
    INTMSK   &= ~( BIT_GLOBAL | BIT_PB );                      // Desenmascara la interrupción por presion de pulsador y las interrupciones globales
}

ufix16 scale_getWeight( void )
{
    if( TCNTO4 )                                                          // El peso no se estabiliza mientras T4 este contando
    {
        if( weightTo )
            return ((TICKS2STABLE-TCNTO4)*weightTo/TICKS2STABLE) >> 4;    // El peso converge al valor aleatorio (emula el desposito de mercancia)
        else
            return (TCNTO4*weightFrom/TICKS2STABLE) >> 4;                 // El peso converge a 0 (emula la retirada de mercancia)
    }
    else
        return weightTo >> 4;                                             // El peso se ha estabilizado
}

/*******************************************************************/

static void pbs_isr( void )
{
    INTMSK |= BIT_PB;                             // Enmascara la interrupcion por pulsacion del pushbuttons

    TCNTB5 = 15625;                               // T5 = 1000 ms = (15625*(255+1)*16)/64000000
    TCON   = (TCON & ~(0xf << 24)) | (1 << 25);   // Modo one shot, carga TCNT5, stop T5
    TCON   = (TCON & ~(0xf << 24)) | (1 << 24);   // Modo one shot, no carga TCNT5, start T5
    pISR_TIMER5 = (uint32) timer5_isr;            // Instala la función timer5_down_isr como RTI por fin de cuenta de T5
    I_ISPC      = BIT_TIMER5;                     // Borra el bit de interrupción pendiente
    INTMSK     &= ~( BIT_GLOBAL | BIT_TIMER5 );   // Desenmascara la interrupción por fin del T5 y las interrupciones globales
   
    EXTINTPND = BIT_LEFTPB | BIT_RIGHTPB;         // Borra los bits de interrupción pendiente en el registro EXTINTPNDD
    I_ISPC = BIT_PB;                              // Borra el bit de interrupcion pendiente comun a ambos pulsadores

    TCNTB4 = TICKS2STABLE;                        // T4 = (TICKS2STABLE*(255+1)*16)/64000000
    TCON   = (TCON & ~(0xf << 20)) |  (1 << 21);  // Modo one shot, carga TCNT4, stop T4
    TCON   = (TCON & ~(0xf << 20)) |  (1 << 20);  // Modo one shot, no carga TCNT4, start T4
    while( !TCNTO4 );                             // Necesario para compensar el retardo en actualización de TCNTO4 <- TCNT4 <- TCNTB4

    if( !weightTo )
    {
        weightFrom = 0;
        weightTo = RANDOM;                        // Emula el deposito de mercancia
        PDATB &= ~(3 << 9);                       // Enciende leds
    }
    else
    {
        weightFrom = weightTo;
        weightTo = 0;                             // Emula la retirada de mercancia
        PDATB |= (3 << 9);                        // Apaga leds
    }
}

static void timer5_isr( void )
{
    pISR_PB   = (uint32) pbs_isr;                 // Instala la funcion pbs_up_isr como RTI por depresion de pulsador
    EXTINTPND = BIT_LEFTPB | BIT_RIGHTPB;         // Borra los bits de interrupción pendiente en el registro EXTINTPNDD
    I_ISPC    = BIT_PB;                           // Borra el bit de interrupción pendiente
    INTMSK   &= ~( BIT_GLOBAL | BIT_PB );         // Desenmascara la interrupción por presion de pulsador y las interrupciones globales

    I_ISPC  = BIT_TIMER5;                         // Borra el bit de interrupcion pendiente por fin de T5
}

static uint32 random( uint32 seed )
{
    static uint32 num;
    uint32 bit;

    if( seed )
        num = seed;
    bit = 0x1 & ~((num >> 31) ^ (num >> 21) ^ (num >> 1) ^ num);
    num = (num << 1) | bit;
    return num;
}
