#include <s3c44b0x.h>
#include <s3cev40.h>
#include <fix_types.h>
#include <common_types.h>
#include <system.h>
#include <uart.h>
#include <keypad.h>
#include <lcd.h>
#include <timers.h>
#include "scaleEmulator.h"
#include <pbs.h>


#define QM 10;
#define FRUTA      ((uint8 *)0x0c210000)
#define VERDURA    ((uint8 *)0x0c220000)
#define MENU 	   ((uint8 *)0x0c230000)
#define SP1  	   ((uint8 *)0x0c240000)
#define SP2 	   ((uint8 *)0x0c250000)

static uint8 lcd_aux[LCD_BUFFER_SIZE];

void isr_timer( void ) __attribute__ ((interrupt ("IRQ")));
int n_ticks = 0;

typedef struct {
	char nombre[20];
	fix16 precio;
} tAlimentos;


int writelist_verduras( int num_list, int movelist);
int writelist_frutas(int num_list, int movelist);
int frutas_verduras();
void pantalla_peso(fix16 precio);
void funcion_peso(tAlimentos *alimentos);
void isr_keypad (void);
int countV = 23;
int countF = 23;


tAlimentos verduras[4][8] ={
		{{"Acelgas", TOFIX( fix16, 1.00, 10)},{"Ajos",TOFIX( fix16,0.60, 10)},{"Alcachofas",TOFIX( fix16,2.20, 10)},{"Berenjena",TOFIX( fix16,1.10, 10)},{"Brocoli",TOFIX( fix16,2.50, 10)},{"Calabacin",TOFIX( fix16,1.45, 10)},{"Calabaza",TOFIX( fix16,1.00, 10)},{"Cebolla",TOFIX( fix16,2.20, 10)}},
		{{"Maiz",TOFIX( fix16,1.10, 10)},{"Chirivia",TOFIX( fix16,1.40, 10)},{"Cogollos",TOFIX( fix16,1.25, 10)},{"Col Lombarda",TOFIX( fix16,1.50, 10)},{"Coliflor",TOFIX( fix16,1.50, 10)},{"Escarola",TOFIX( fix16,1.80, 10)},{"Esparrago",TOFIX( fix16,2.20, 10)},{"Espinacas",TOFIX( fix16,1.00, 10)}},
		{{"Judias Verdes",TOFIX( fix16,1.25, 10)},{"Lechuga",TOFIX( fix16,2.69, 10)},{"Nabo",TOFIX( fix16,0.85, 10)},{"Pepino",TOFIX( fix16,0.99, 10)},{"Pimiento",TOFIX( fix16,1.10, 10)},{"Puerro",TOFIX( fix16,1.50, 10)},{"Tomate",TOFIX( fix16,0.72, 10)},{"Zanahoria",TOFIX( fix16,2.00, 10)}},

};

tAlimentos frutas[4][8] ={
		{{"Chirimoyas",TOFIX( fix16,3.25, 10)},{"Limones",TOFIX( fix16,1.00, 10)},{"Mandarina ",TOFIX( fix16,1.10, 10)},{"Manzana ",TOFIX( fix16,0.80, 10)},{"Naranja",TOFIX( fix16,0.87, 10)},{"Platano",TOFIX( fix16,0.70, 10)},{"Higo",TOFIX( fix16,2.25, 10)},{"Cereza",TOFIX( fix16,1.25, 10)}},
		{{"Ciruelas",TOFIX( fix16,2.90, 10)},{"Fresas",TOFIX( fix16,2.50, 10)},{"Granada",TOFIX( fix16,1.50, 10)},{"Kaki",TOFIX( fix16,2.20, 10)},{"Persimon",TOFIX( fix16,1.90, 10)},{"Kiwi",TOFIX( fix16,2.45, 10)},{"Lima",TOFIX( fix16,1.45, 10)},{"Melon",TOFIX( fix16,3.00, 10)}},
		{{"Papaya",TOFIX( fix16,2.50, 10)},{"Piña",TOFIX( fix16,2.69, 10)},{"Sandia",TOFIX( fix16,1.15, 10)},{"Uvas",TOFIX( fix16,2.30, 10)},{"Melocoton",TOFIX( fix16,2.20, 10)},{"Níspero",TOFIX( fix16,1.50, 10)},{"Pomelo",TOFIX( fix16,0.72, 10)},{"Nectarina",TOFIX( fix16,1.10, 10)}},

};
tAlimentos *p = NULL;


void lcd_putfix( uint16 a, uint16 b, uint8 color, fix16 i ){
	int parte_entera;
	int parte_decimal;
	fix16 diez = 1010 << 10;
	int num_digitos = 1;
	uint16 x = a;
	uint16 y = b;

	parte_entera = i >> 10;
	lcd_putint_x3(x,y,color,parte_entera);


	while (parte_entera > 10){
		parte_entera = parte_entera/10;
		num_digitos++;
	}

	x = x + (num_digitos*24);

	lcd_puts_x3(x,y,color,",");
	x = x+24;

	i &= 0x3FF; //solo se queda la parte decimal
	parte_decimal = FMULI( 10, (fix32)i); //multiplicamos por 10
	lcd_putint_x3(x,y,color, parte_decimal >>10);
	x = x+24;

	i = FMULI( 10, (fix32)i);

	i &= 0x3FF; //solo se queda la parte decimal
	parte_decimal = FMULI( 10, (fix32)i); //multiplicamos por 10
	lcd_putint_x3(x,y,color, parte_decimal >> 10);

}

int writelist_verduras( int num_list, int movelist){

	int j = 7, i=0, posicion_movelist = 0,move = movelist;
	uint8 scancode;

	scancode = KEYPAD_KEYA;

	while( i < 4){

		if( i == move )
			posicion_movelist = j;

		lcd_puts( 50, 20+j, BLACK, verduras[num_list][i].nombre ); // he cambiado verduras por alimentos
		i++;
		j=j+43;
	}

	j=7;

	while( i < 8 ){
		if (i == move)
			posicion_movelist = j;
		lcd_puts( 200, 20+j, BLACK, verduras[num_list][i].nombre ); // lo mismo que arriba :D
		i++;
		j=j+43;
	}

	while(scancode != KEYPAD_KEYE && scancode != KEYPAD_KEYC && scancode != KEYPAD_KEYD
			&& scancode != KEYPAD_KEYF && scancode != KEYPAD_KEY0 && scancode != KEYPAD_KEY1 ){

		if(move <= 3)
			lcd_puts( 50, 20+posicion_movelist, BLACK, verduras[num_list][move].nombre ); //mas de lo mismo
		else
			lcd_puts( 200, 20+posicion_movelist, BLACK, verduras[num_list][move].nombre ); //mas de lo mismo

		scancode = keypad_scan();
		sw_delay_ms(250);
		scancode = keypad_scan();

		if(move < 3 || move == 3)
			lcd_puts( 50, 20+posicion_movelist, BLACK, "              ");
		else
			lcd_puts( 200, 20+posicion_movelist, BLACK, "             " );

		scancode = keypad_scan();
		sw_delay_ms(250);
		scancode = keypad_scan();
	}


	if(scancode == KEYPAD_KEYC){  //pulsa anterior
		return 1;
	}
	if(scancode == KEYPAD_KEYD){ //pulsa cancelar
		return 0;
	}
	if(scancode == KEYPAD_KEYE){ //Pulsa aceptar
		funcion_peso(&verduras[num_list][move]);
		return 4;
	}
	if(scancode == KEYPAD_KEYF){ //Pulsa siguiente
		return 2;
	}
	if(scancode == KEYPAD_KEY0){  //pulsa pantalla anterior
		lcd_clear();
		return 5;
	}
	if(scancode == KEYPAD_KEY1){  //pulsa pantalla siguiente
		lcd_clear();
		return 6;
	}
}

int writelist_frutas( int num_list, int movelist){

	int j = 7, i=0, posicion_movelist = 0,move = movelist;
	uint8 scancode;

	scancode = KEYPAD_KEYA;

	while( i < 4){

		if( i == move )
			posicion_movelist = j;

		lcd_puts( 50, 20+j, BLACK, frutas[num_list][i].nombre ); // he cambiado verduras por alimentos
		i++;
		j=j+43;
	}

	j=7;

	while( i < 8 ){

		if( i == move )
			posicion_movelist = j;

		lcd_puts( 200, 20+j, BLACK, frutas[num_list][i].nombre ); // lo mismo que arriba :D
		i++;
		j=j+43;
	}

	while(scancode != KEYPAD_KEYE && scancode != KEYPAD_KEYC && scancode != KEYPAD_KEYD
			&& scancode != KEYPAD_KEYF && scancode != KEYPAD_KEY0 && scancode != KEYPAD_KEY1 ){

		if(move <= 3)
			lcd_puts( 50, 20+posicion_movelist, BLACK, frutas[num_list][move].nombre ); //mas de lo mismo
		else
			lcd_puts( 200, 20+posicion_movelist, BLACK, frutas[num_list][move].nombre ); //mas de lo mismo

		scancode = keypad_scan();
		sw_delay_ms(250);
		scancode = keypad_scan();

		if(move < 3 || move == 3)
			lcd_puts( 50, 20+posicion_movelist, BLACK, "            ");
		else
			lcd_puts( 200, 20+posicion_movelist, BLACK, "           " );

		scancode = keypad_scan();
		sw_delay_ms(250);
		scancode = keypad_scan();
	}


	if(scancode == KEYPAD_KEYC){  //pulsa anterior
		return 1;
	}
	if(scancode == KEYPAD_KEYD){ //pulsa cancelar
		return 0;
	}
	if(scancode == KEYPAD_KEYE){ //Pulsa aceptar
		funcion_peso(&frutas[num_list][move]);
		return 4;
	}
	if(scancode == KEYPAD_KEYF){ //Pulsa siguiente
		return 2;
	}
	if(scancode == KEYPAD_KEY0){  //pulsa pantalla anterior
		lcd_clear();
		return 5;
	}
	if(scancode == KEYPAD_KEY1){  //pulsa pantalla siguiente
		lcd_clear();
		return 6;
	}

}

int frutas_verduras(){
	int i = 0; // i = 0 -> Fruta; i= 1-> verdura; i = 2 -> modo admin;
	uint8 scancode;
	lcd_putWallpaper( VERDURA );
	lcd_putWallpaper( FRUTA );
	scancode = keypad_getchar();
	while(scancode != KEYPAD_KEYE && scancode != KEYPAD_KEY0){ // scancode != ok o scandode != admin
		if(scancode == KEYPAD_KEYC || scancode== KEYPAD_KEYF){ //scancode == flechas
			if(i == 0){
				i = 1;
				lcd_putWallpaper( VERDURA );
			}
			else{
				i = 0;
				lcd_putWallpaper( FRUTA );
			}
		}
		scancode = keypad_getchar();
	}
	if (scancode == KEYPAD_KEY0)
		i = 2;

	return i;
}

void pantalla_peso(fix16 precio){//tAlimentos alimento){
	lcd_clear();
	lcd_draw_box( 10, 10, 310, 230, BLACK, 5 );
	lcd_draw_hline(10,310,115,BLACK,5);
	lcd_draw_vline(115,230,155,BLACK,5);
	lcd_puts( 16, 3, BLACK, "Peso:" );
	lcd_puts_x3( 240, 63, BLACK, "Kg" );
	lcd_puts( 162, 108, BLACK, "Total:" );
	lcd_puts( 16, 108, BLACK, "Precio/Kg:" );
	lcd_putfix(24,145, BLACK, precio);
	lcd_puts_x2(90,195, BLACK, "$/Kg" );
	lcd_puts_x2(290,195,BLACK,"$");

}

void funcion_peso(tAlimentos *alimentos){
	ufix16 weight = 0;
	ufix16 lastWeight = 1;

	scale_init();
	lcd_clear();
	pbs_init();

	pantalla_peso(alimentos->precio);
	lcd_putint_x2(110,70,BLACK, 0);
	timer3_delay_ms(200);



	pb_wait_any_keydown();
	while( weight != lastWeight )
	 {
		timer3_delay_ms(250);
		lastWeight = weight;
	    weight = scale_getWeight();                         // lee el sensor de peso
	    lcd_putfix(110,63,BLACK, weight);
	    lcd_putfix(170,145,BLACK, FMUL(alimentos->precio, weight, 10));

	 }
	imprimeticket(alimentos->nombre, weight, alimentos->precio);
}

char *strcpy(char *dest, const char *src)
{
  unsigned i;
  for (i=0; src[i] != '\0'; ++i)
    dest[i] = src[i];
  dest[i] = '\0';
  return dest;
}

tAlimentos nuevoAlimento(){
	tAlimentos nuevo;
	char   new_name[20];
	int32 i;
	fix16 new_precio;
	fix16 cien = TOFIX(fix16, 100, 10);
	uart0_puts( "Escriba el Nombre del nuevo Producto: " );
	uart0_gets( new_name );
	uart0_putchar( '\n' );
	strcpy(nuevo.nombre, new_name);
	uart0_puts( "Escriba el Precio del nuevo Producto: " );
	uart0_putchar( '\n' );
	uart0_puts("escribiendo los dos decimales a continuacion del numero");
	uart0_putchar( '\n' );
	uart0_puts("PE: 1e = 100; 2,23e = 223"); uart0_putchar( '\n' );
	i = uart0_getint();

	new_precio = TOFIX(fix16, (float)i/100, 10);
	//new_precio = FDIV(new_precio, cien,10);
	nuevo.precio = new_precio;
	return nuevo;
}

void anadir(){
	lcd_clear();
	//mostrar por lcd:
	uint8 scancode;
	tAlimentos nuevo;

	lcd_puts_x2(70, 50,BLACK," 0 - Frutas");
	lcd_puts_x2(70,110,BLACK," 1 - Verdura");
	lcd_puts_x2(70,170,BLACK," 2 - Cancelar");

	uart0_puts( " ---------------------------------- " );uart0_putchar( '\n' );
	uart0_puts( "|      AÑADIR NUEVO PRODUCTO       |" );uart0_putchar( '\n' );
	uart0_puts( " ---------------------------------- " );uart0_putchar( '\n' );

	while (1){
		scancode = keypad_getchar();
		if (scancode == KEYPAD_KEY0){
			lcd_clear();
			countF++; //nueva fruta
			nuevo = nuevoAlimento();
			strcpy(frutas[countF/8][countF%8].nombre, nuevo.nombre);
			frutas[countF/8][countF%8].precio = nuevo.precio;
			uart0_puts( "-------Producto añadido--------" );uart0_putchar( '\n' );
			return;
		}
		else if (scancode == KEYPAD_KEY1){
			lcd_clear();
			countV++; //nueva verdura
			nuevo = nuevoAlimento();
			strcpy(verduras[countV/8][countV%8].nombre, nuevo.nombre);
			verduras[countV/8][countV%8].precio = nuevo.precio;
			uart0_puts( "-------Producto añadido--------" );uart0_putchar( '\n' );
			return;
		}
		else if (scancode == KEYPAD_KEY2){
			uart0_puts( "----------Cancelado-----------" );uart0_putchar( '\n' );
			return;
		}
	}
}

void uart0_putfix(fix16 num){
	int parte_entera;
	int parte_decimal;
	fix16 diez = 1010 << 10;


	parte_entera = num >> 10;
	uart0_putint(parte_entera);

	uart0_puts( "," );//poner la coma

	num &= 0x3FF; //solo se queda la parte decimal
	parte_decimal = FMULI( 100, (fix32)num); //multiplicamos por 100 (dos decimales)
	uart0_putint(parte_decimal >> 10);
}

void imprimeticket(char *nombre, fix16 peso, fix16 precio){
	uart0_puts( " -----------------------------" );uart0_putchar( '\n' );
	uart0_puts( "|     TICKET DE COMPRA        |" );uart0_putchar( '\n' );
	uart0_puts( " ----------------------------- " );uart0_putchar( '\n' );
	uart0_puts("NOMBRE                        e/Kg");uart0_putchar( '\n' );
	uart0_puts(nombre);
	uart0_puts("__________________");
	uart0_putfix(precio);
	uart0_putchar( '\n' );
	uart0_puts("-  TOTAL:");uart0_putchar( '\n' );

	uart0_putfix(FMUL(precio,peso,10));
}

strcmp(const char *s1, const char *s2)
{
    for ( ; *s1 == *s2; s1++, s2++)
	if (*s1 == '\0')
	    return 0;
    return ((*(unsigned char *)s1 < *(unsigned char *)s2) ? -1 : +1);
}

void borrar(){
	lcd_clear();
	//mostrar por lcd:
	uint8 scancode;
	char nombrelista [20];
	char nombre[20];
	boolean encontrado = 0;
	int i=0;
	int j= 0;

	lcd_puts_x2(70, 50,BLACK," 0 - Frutas");
	lcd_puts_x2(70,110,BLACK," 1 - Verdura");
	lcd_puts_x2(70,170,BLACK," 2 - Cancelar");

	uart0_puts( " ------------------------------ " );uart0_putchar( '\n' );
	uart0_puts( "|		BORRAR UN PRODUCTO		|" );uart0_putchar( '\n' );
	uart0_puts( " ------------------------------" );uart0_putchar( '\n' );

	while (1){
		scancode = keypad_getchar();
		if (scancode == KEYPAD_KEY0){
			lcd_clear();

			uart0_puts( "Escriba el Nombre del Producto a borrar: " );uart0_putchar( '\n' );
			uart0_gets( nombre );

			//buscar
			for(i=0; i < 4 && encontrado == 0; i++)
				for(j = 0; j < 8 && encontrado == 0; j++ ){
					 strcpy(nombrelista, frutas[i][j].nombre);
					 if ((strcmp(nombre,nombrelista)) == 0){
					    encontrado = 1;
						i--;
						j--;
					 }
				}
			if (encontrado == 1){

				strcpy(frutas[i][j].nombre, frutas[countF/8][countF%8].nombre); //copiamos el nombre
				frutas[i][j].precio = frutas[countF/8][countF%8].precio; //copiamos el precio
				strcpy(frutas[countF/8][countF%8].nombre, " "); //Esperemos que no pete D:
				frutas[countF/8][countF%8].precio = 0;
				countF--;
				uart0_puts( "-------Producto Borrado--------" );uart0_putchar( '\n' );
			}
			else
				uart0_puts( "-----Producto no encontrado------" );uart0_putchar( '\n' );
			return;
		}

		else if (scancode == KEYPAD_KEY1){
			lcd_clear();

			uart0_puts( "Escriba el Nombre del nuevo Producto: " );uart0_putchar( '\n' );
			uart0_gets( nombre );

			//buscar
			for(i=0; i < 4 && encontrado == 0; i++)
				for(j = 0; j < 8 && encontrado == 0; j++ ){
					strcpy(nombrelista, verduras[i][j].nombre);
					if ((strcmp(nombre,nombrelista)) == 0){
						encontrado = 1;
						i--;
						j--;
					}
				}
			if (encontrado == 1){
				strcpy(verduras[i][j].nombre, verduras[countV/8][countV%8].nombre); //copiamos el nombre
				verduras[i][j].precio = verduras[countV/8][countV%8].precio; //copiamos el precio
				strcpy(verduras[countV/8][countV%8].nombre, " "); //Esperemos que no pete D:
				verduras[countV/8][countV%8].precio = 0;
				countV--;
				uart0_puts( "-------Producto Borrado--------" );uart0_putchar( '\n' );
			}
			else
				uart0_puts( "-----Producto no encontrado------" );uart0_putchar( '\n' );
			return;
		}
		else if (scancode == KEYPAD_KEY2){
			uart0_puts( "----------Cancelado-----------" );uart0_putchar( '\n' );
			return;
		}
	}//while

}

void modo_admin(){
	lcd_clear();
	//mostrar por lcd:
	uint8 scancode;
	lcd_puts_x2(7, 50,BLACK," 0 - Añadir producto");
	lcd_puts_x2(7,110,BLACK," 1 - Borrar producto");
	lcd_puts_x2(7,170,BLACK," 2 - Cancelar");
	while (1){
		scancode = keypad_getchar();
		if (scancode == KEYPAD_KEY0){
			anadir();
			return;
		}
		else if (scancode == KEYPAD_KEY1){
			borrar();
			return;
		}
		else if (scancode == KEYPAD_KEY2)
			return;
	}
}

void main( void ){

	sys_init();
	uart0_init();
	lcd_init();
	keypad_init();
	uart0_init();
	timers_init();


	uart0_putchar('/n');
	n_ticks = 0;

	timer0_open_ms( isr_timer, 9999, 0 );



	int pantalla;
	int seleccion;
	int i=0,j = 0;

	lcd_clear();
	lcd_on();
	while(1){
		pantalla =0;
		seleccion = frutas_verduras();

		if(seleccion == 1){
			uart0_puts("Verdura seleccionada");
			uart0_putchar( '\n' );

			do{
				lcd_putWallpaper(MENU);
				i = writelist_verduras(pantalla,j);

				if(i==2)
					j = (j+1)%8;
				if(i==1)
					j = (j-1) %8;
				if(i==4){ 			// se ha dado a OK
					keypad_wait_any_keyup();
					i = 0; //para salir del bucle
				}
				if (i == 5) 		//pantalla anterior
					pantalla = (pantalla-1)%8;
				if (i == 6)			//siguiente pantalla
					pantalla = (pantalla+1)%8;
			} while( i != 0 );
		}

		else if (seleccion == 0){
			uart0_puts("Fruta seleccionada");uart0_putchar( '\n' );

			do{
				lcd_putWallpaper(MENU);
				i = writelist_frutas(pantalla,j);

				if(i==2) //busca en la siguiente fila
					j = (j+1)%8;

				if(i==1) //busca en la fila anterior
					j = (j-1)%8;
				if(i==4){ // se ha dado a ok
					keypad_wait_any_keyup();
					i = 0; //para salir del bucle
				}
				if (i == 5) 		//pantalla anterior
					pantalla = (pantalla-1)%8;
				if (i == 6)			//siguiente pantalla
					pantalla = (pantalla+1)%8;
			} while( i != 0 );
		}
		else{
			uart0_putchar('/n');
			modo_admin();
		}
	}

}

void isr_timer( void ){

	if (n_ticks < 3) // si hay menos de 10s se suma 1
		n_ticks++;

	else{
		I_ISPC = BIT_TIMER0;
		lcd_savescreen(lcd_aux); //guarda en lcd_aux el estado de la pantalla;
		while(keypad_scan() == KEYPAD_FAILURE){ //mientras no se pulse ninguna tecla
			lcd_putWallpaper(SP1);
			timer3_delay_s(2);
			lcd_putWallpaper(SP2);
			timer3_delay_s(2);
		}

		lcd_loadscreen(lcd_aux); //muestra por pantalla lo que estaba antes del salvapantallas
		sw_delay_ms(1000);
		n_ticks = 0;
		timer0_open_ms( isr_timer, 10000, 1 );

	}

}







/*void main( void )
{
    ufix16 weight = 0;
    ufix16 lastWeight = 0;

    sys_init();                                              // Inicializa sistema y dispositivos
    uart0_init();
    scale_init();                                            // Inicializa el emulador del sensor de peso

    while( 1 )                                               // Indefinidamente
    {
        lastWeight = weight;
        weight = scale_getWeight();                          // lee el sensor de peso
        if( weight != lastWeight )                           // y si la lectura difere de la anterior
        {
            uart0_puts( "Lectura del sensor de peso: " );
            uart0_putint( weight );                          // vuelca el valor por la uart0
            uart0_putchar( '\n' );
        }
    }
}
*/
