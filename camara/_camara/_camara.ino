// Arduino timer CTC interrupt example
// www.engblaze.com

// avr-libc library includes
#include <avr/io.h>
#include <avr/interrupt.h>

#define LEDPIN 13
#define SEGUNDOS_POR_MINUTO 1
const String ESPACIO = "   ";

// para la camara de fotos
#define PRENDER 7
#define FOTO 6

const int LIBERAR_PRENDER = 250;  // Setear en 500 ms
const int DELAY_ENCENDIDO = 500; // Tiempo entre el ENCENDIDO y la FOTO
const int DURACION_PULSO = 250;  // Setear en 500 ms


//------------------------------
int intervaloFotosEnMinutos = 5;
//------------------------------
int intervaloFotoEnSegundos = intervaloFotosEnMinutos * SEGUNDOS_POR_MINUTO;
int segundosPasadosDesdeUltimaFoto = 0;
boolean esMomentoDeSacarFoto = false;

void setup()
{
        // ---------- Setup de los Timers. NO CAMBIAR -------------
	// initialize Timer1
	cli();			// disable global interrupts
	TCCR1A = 0;		// set entire TCCR1A register to 0
	TCCR1B = 0;		// same for TCCR1B

	// set compare match register to desired timer count:
	OCR1A = 15624;
	// turn on CTC mode:
	TCCR1B |= (1 << WGM12);
	// Set CS10 and CS12 bits for 1024 prescaler:
	TCCR1B |= (1 << CS10);
	TCCR1B |= (1 << CS12);
	// enable timer compare interrupt:
	TIMSK1 |= (1 << OCIE1A);
	// enable global interrupts:
	sei();
        // ------------ Fin Setup de los Timers -----------------

        // Set up de camara
	pinMode(LEDPIN, OUTPUT); // led de prueba


        Serial.begin(9600);
}


///////////////////// LOOP PRINCIPAL ///////////////////////
void loop()
{
  if (esMomentoDeSacarFoto == true) {
    sacarFoto();
    // seteando en false ya que acabamos de sacar foto y no tenemos que sacar otra hasta el proximo intervalo
    esMomentoDeSacarFoto = false;
  }
    
}
/////////////////// FIN LOOP PRINCIPAL ////////////////////


/** 
  * Interrupcion invocada por cada segundo.
  * Lleva la cuenta de la cantidad de segundos que paso desde la ultima foto que se saco.  Si es momento de sacar una nueva foto, 
  * entonces levanta flag de sacar foto, permitiendo al loop principal sacar la foto.
  * Manter esta interrupcion lo mas 'rapida' posible dado que mientras se encuentra en esta interrupcion, ninguna otra interrupcion puede ser manejada.
**/
ISR(TIMER1_COMPA_vect) {
    segundosPasadosDesdeUltimaFoto++;
    if (segundosPasadosDesdeUltimaFoto == intervaloFotoEnSegundos) {
        Serial.println("Momento de sacar foto");
        esMomentoDeSacarFoto = true;
        segundosPasadosDesdeUltimaFoto = 0;
    }  
}
//-------------------------------------------
//-------------------------------------------
void sacarFoto() {
//   digitalWrite(LEDPIN, !digitalRead(LEDPIN)); 
  imprimirTiempo();
  
  prenderCamara();
  digitalWrite (FOTO , LOW);
  delay (DURACION_PULSO);
  digitalWrite (FOTO , HIGH);   
  ponerEnStandByCamara();
  
  Serial.println("Foto tomada");
}
//-------------------------------------------
void prenderCamara() {
    digitalWrite (PRENDER , LOW);  // Prende la camara
    delay (DELAY_ENCENDIDO); 
}
//-------------------------------------------
void ponerEnStandByCamara() {
    delay (LIBERAR_PRENDER);
    digitalWrite (PRENDER , HIGH); // Apaga la camara
} 
//-------------------------------------------
void imprimirTiempo() {
    RTC.getTime();
    
    String tiempo = ESPACIO;
    tiempo = tiempo + RTC.hour, DEC;
    tiempo = tiempo + ESPACIO;
    tiempo = tiempo + RTC.minute, DEC;
    tiempo = tiempo + ESPACIO;
    tiempo = tiempo + RTC.second, DEC;
   
    Serial.println(tiempo);
}
//-------------------------------------------
