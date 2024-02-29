#include <8051.h>
#define true 1
#define false 0
#define T0_DAT 65535-921
#define TL_0 T0_DAT%256
#define TH_0 T0_DAT/256
#define T100 1000

// Klawisze oraz ich odpowiedznik w wartoœci
//LEFT = 1, RIGHT = 8 , ENTER = 2, ESC = 4, UP = 16, DOWN = 32
#define ENTER 0b00000010
#define ESC 0b00000100
#define RIGHT 0b00001000
#define UP 0b00010000
#define DOWN 0b00100000
#define LEFT 0b00000001
#define PRESSED P3_5

__code unsigned char wzory[10] = { 0b0111111, 0b0000110, 0b1011011, 0b1001111, 0b1100110, 0b1101101, 0b1111101, 0b0000111, 0b1111111, 0b1101111 };

// wybieranie wyœwietlacza
__xdata unsigned char* CSDS = ( __xdata unsigned char * ) 0xFF30;

//wybieranie segmentu
__xdata unsigned char* CSDB = ( __xdata unsigned char * ) 0xFF38;

__sfr __at (0x8C) TH0;
__sfr __at (0x8A) TL0;

// Flagi
__bit t0_flag;
__bit key_flag = 0;
__bit keyboard_flag = 0;
__bit edit_mode = 0;

unsigned int R2 = 240;
unsigned int R3 = 4;

// Wybranie odpowiedniego klawisza oraz odpowiedniej jednostki czasu do zmiany
unsigned char selected_key = 0; // wybrany przycisk
unsigned char time_unit = 0; // pos³u¿y do wybrania jednostki czasu do ustawienia

unsigned char led_index = 1, led_bit = 1; //indeks aktywnego wyœwietlacza, aktywny wyœwietlacz

// Ustawienie pocz¹tkowej godziny
unsigned char godzina = 23, minuta = 59, sekunda = 37;


// Zmiana czasu ogólnego

void inkrementacja_czasu(){
    sekunda++;

    if(sekunda == 60){
        sekunda = 0;
        minuta++;
    }
    if(minuta == 60){
        minuta = 0;
        godzina++;
    }
    if(godzina == 24){
        godzina = 0;
    }
}

// Inkrementacja godziny, minuty i sekundy

void inkrementacja_godziny(){
    godzina++;

    if(godzina == 24){
        godzina = 0;
    }
}

void inkrementacja_minuty(){
    minuta++;

    if(minuta == 60){
        minuta = 0;
    }
}

void inkrementacja_sekundy(){
    sekunda++;

    if(sekunda == 60){
        sekunda = 0;
    }
}

// Dekrementacja godziny, minuty i sekundy

void dekrementacja_godziny(){

    if(godzina == 0){
        godzina = 23;
    }else{
        godzina--;
    }
}

void dekrementacja_minuty(){

    if(minuta == 0){
        minuta = 59;
    }else{
        minuta--;
    }
}

void dekrementacja_sekundy(){

    if(sekunda == 0){
        sekunda = 59;
    }else{
        sekunda--;
    }
}

// TIMER

void t0_int(void)__interrupt(true) {
	TH0 = 226;
	TL0 = 0;
	t0_flag = 1;
}

void t0_serv(){

    R2--;
    if(R2==0){
        R3--;

        if(R3==0){
            R3=4;
            if(key_flag == 0){
                inkrementacja_czasu();
            }
        }

        R2=240;
    }
}

void reset_timer(){
    TH0 = 226;
    TL0 = 0;
    R2 = 240;
    R3 = 4;
    t0_flag = 0;
}

// Wyœwietlanie czsu na wyœwietlaczach

void wyswietl_czas(){

    if(led_index == 1){

        P1_6 = true;
        *CSDS = led_bit;
        *CSDB = wzory[sekunda%10];
        led_bit+= led_bit;
        led_index++;
        P1_6 = false;

    }else if(led_index == 2){

        P1_6 = true;
        *CSDS = led_bit;
        *CSDB = wzory[sekunda/10];
        led_bit+= led_bit;
        led_index++;
        P1_6 = false;

    }else if(led_index == 3){

        P1_6 = true;
        *CSDS = led_bit;
        *CSDB = wzory[minuta%10];
        led_bit+= led_bit;
        led_index++;
        P1_6 = false;

    }else if(led_index == 4){

        P1_6 = true;
        *CSDS = led_bit;
        *CSDB = wzory[minuta/10];
        led_bit+= led_bit;
        led_index++;
        P1_6 = false;

    }else if(led_index == 5){

        P1_6 = true;
        *CSDS = led_bit;
        *CSDB = wzory[godzina%10];
        led_bit+= led_bit;
        led_index++;
        P1_6 = false;

    }else if(led_index == 6){

        P1_6 = true;
        *CSDS = led_bit;
        *CSDB = wzory[godzina/10];
        led_bit = 1;
        led_index = 1;
        P1_6 = false;
    }
}

// Ustawienie wyœwietlanej godziny poprzez klawiature

void ustaw_zegarek(){

    if(keyboard_flag == 0){
        keyboard_flag = 1;

        if(selected_key == ESC){
            P1_7 = false;

            if(edit_mode == 0){
                key_flag = 1;
                edit_mode = 1;
            }

        }else if(selected_key == ENTER){
            P1_7 = true;

            if(edit_mode == 1){
	    	reset_timer();
                key_flag = 0;
                edit_mode = 0;
                time_unit = 0;
            }

        }else if(edit_mode == 1){

            if(selected_key == LEFT){
                time_unit++;

                if(time_unit > 2){
                    time_unit = 0;
                }

            }else if(selected_key == RIGHT){
                time_unit--;

                if(time_unit > 2){
                    time_unit = 2;
                }

            }else if(selected_key == UP){

                if(time_unit == 0){
                    inkrementacja_sekundy();

                }else if(time_unit == 1){
                    inkrementacja_minuty();

                }else if(time_unit == 2){
                    inkrementacja_godziny();
                }

            }else if(selected_key == DOWN){
                if(time_unit == 0){
                    dekrementacja_sekundy();

                }else if(time_unit == 1){
                    dekrementacja_minuty();

                }else if(time_unit == 2){
                    dekrementacja_godziny();
                }

            }
        }
    }
}

// Wartoœci inicjalizuj¹ce do timera

void ustaw_timer(){
    TH0=   226;
    TL0 = 0;
    TMOD = 0b01110000;
    IE   = 0b10000010;
    TR0  = 0b00000001;
}

// Funkcja g³ówna

void main(){

    ustaw_timer();

    while ( 1 ){

        if(t0_flag == 1){
            t0_flag = 0;
            wyswietl_czas();
            t0_serv();

            if(PRESSED == true){
                selected_key = led_bit;
                ustaw_zegarek();

            }else if(PRESSED == false && keyboard_flag == 1 && selected_key == led_bit){
                keyboard_flag = 0;
            }
        }
    }
}