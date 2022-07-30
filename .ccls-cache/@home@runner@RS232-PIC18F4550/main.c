/*
 * File:   Main_18F.c
 * Author: mario
 *
 * Created on 1 de febrero de 2022, 09:47 PM
 */

#include <xc.h>
#include <math.h>               //Libreria para Raiz Cuadrada Y arcotangente
#define _XTAL_FREQ 8000000      //Frecuencia de trabajo
#pragma config FOSC=INTOSC_EC   //Configuración de fuente de reloj interna
#pragma config WDT=OFF          //Desactivación del perro guardian
#pragma PBADEN=OFF              //
#pragma LVP=OFF                 //Programación de Bajo voltaje Activada

#define LED_CPU LATE0           //Pin del led de CPU asociado al funcionamiento








int ax,ay,az,gx,gy,gz;          //Variables de la IMU
int aux2,aux3 ;                      //Variable temporal
float Ax,Ay,Az;
float Ayz,Angx,Axz,Angy;
char Comando, Comando2 ;

char b =0;
char BufferR2[10];
unsigned char BufferR;          //Registro de resepción
void ImprimirDecimal (float);   //Función para imprimir un valor decimal en el puerto serial
void MensajeRS232(char *);    //Función para imprimir cadena de Caracteres

void Transmitir(unsigned char); //Función encargada de enviar datos Seriales
unsigned char Recibir(void);    //Función encargada de Resivir datos Seriales


void __interrupt () ISR (void); //Interrupción 
void main(void) {
    
    
  //unsigned char aux;            //variables de la IMU
  //float Ax,Ay,Az; //,gx,gy,gz;        //Variables de la IMU
  OSCCON=0b01110000;            //Configuración del oscilador 
  __delay_ms(1);                //Tiempo de establecimiento para el oscilador
  TRISE=0;
  LED_CPU=1;
  TRISD=0;
  LATD2=1;
  LATD3=1;
  //LATD|=0;
  LATD0=1;                  //lED DE ARRANQUE
  
  /* Configuración del puerto Serial*/
  TXSTA=0b00100000;     //Habilitación de trasnmición de datos
  RCSTA=0b10010000;     //Encendido del Puerto Serial y habilitación de Recepción
  BAUDCON=0b00000000;   //Configuración por Tabla 20-3 (9600Baudio)
  SPBRG=12;             //Configuración por Tabla 20-3 (9600Baudio)
  
  
  
  
  /* Configuración del PWM */
  TRISC2=0;             //Configuración del pin del ccp1 como salida
  CCP1CON=0b00001100;   //Configuración del modulo ccp1 como PWM
  TMR2=0;               //Inicializar el timer 2 en un valor de 0
  T2CON=0;              //Configurar el timer 2 Sin preescalar y sin posescalar
  PR2=133;              //Valor concerniente el periodo del PR2=((PWM fBus*Tpwm)/TMR2Prescal)-1
  CCPR1L=0;             //Valor Inicial de ciclo Util CCPR1L = (113+1)*n%
  TMR2ON=1;             //Encendido del Timer
  /* Configuración de interrupciones */
  RCIF=0;   //Bandera de Interrupción en Cero
  RCIE=1;   //Activación de la Interrupción para recepción de datos
  PEIE=1;   //Habilitaciónon de las interrupciones por periferico
  GIE=1;    //Habilitación de las interrupciones generales
  /* Comienzo del programa principal */
  LED_CPU=0;                  //lED DE ARRANQUE
  
  while(1){
    __delay_ms(500);
    LED_CPU=1;
    __delay_ms(500);
    LED_CPU=0;
    
        if(BufferR2[0] == 'F'){
            LATD3=0;
            __delay_ms(1500);
            LATD3=1;
            BufferR2[0]=0;
        }
        if(BufferR2[0] == 'O'){
            LATD2=0;
            __delay_ms(500);
            LATD2=1;
            BufferR2[0]=0;
        }
        //RCIF = 0;
    if(BufferR2[0]-48>0 && BufferR2[0]-48<9){
            CCPR1L=((BufferR2[0]-48)*10)+20;
        }
    if(BufferR2[0]-48==9){
        CCPR1L=134;
    }
    if(BufferR2[0]-48==0){
        CCPR1L=0;
    }
    MensajeRS232("Comando");
    //Transmitir(BufferR);
    Transmitir(BufferR2[0]);
    //Transmitir(BufferR2[1]);
    //Transmitir(BufferR2[2]);
    //Transmitir(BufferR2[3]);
    //Transmitir(BufferR2[4]);
    MensajeRS232(BufferR2);
    Transmitir('\n');
  }  
}
/* Función de interrupción */
void __interrupt () ISR (void){
     if(RCIF==1){   //Bandera de Interrupción en Cero
         RCIF=0;
         BufferR2[b]=Recibir();
         b++;
         if(BufferR2[b-1]=='\n'||b==10){ 
             while(b!=10){
                 BufferR2[b]=0;
                 b++;
             }
             b=0;
         }
     }
}
/* Función imprimir Cadena de caracteres */
void MensajeRS232(char* a){
//Función que escribe una cadena de caracteres variable en la pantalla
//a es una cadena de caracteres guardada en una variable *char
//Ejemplo: char aux[4]="Hola"; MensajeLCD_Var(aux);	
	while (*a != '\0'){
		Transmitir(*a);
		a++;
	}		
}
/*  Funcion para imprimer el numero decimal  */
void ImprimirDecimal (float An){
    char Entero; //prueba de impresion decimal
    int Decimal,aux1,i;
    if(An<0){
        Transmitir('-');
        An=An*(-1);
    }
    Entero=(int)An;             //La Patre entera de la división se guarda en Entero
    An= An-Entero;              //Dejar solamente la parte decimal
    An= An*10000;               //Convierto los decimales a enteros para poder extraerlos
    Decimal = (int)An;          //La Parte decimal de la división se guarda en decimal
    //aux2 = Entero;
    //aux3 = Decimal;
    i=10;
    while(Entero%i!=Entero){
        i=i*10;
    }
    i=i/10;
    aux1=Entero;
    while(i>=1){
        Transmitir((aux1/i)+48);
        aux1=aux1%i;
        i=i/10;
    }
    Transmitir('.');
    i=1000;
    aux1=Decimal;
    while(i>=1){
        Transmitir((aux1/i)+48);
        aux1=aux1%i;
        i=i/10;
    }
    Transmitir(' ');
}





/*  Rs232 Funciones de transmición y recepción*/
void Transmitir(unsigned char BufferT){
  while(TRMT==0);
  TXREG=BufferT;  
}
unsigned char Recibir(void){
    //while(RCIF==0);
    return RCREG;
}