#include "lib.h"
#include <allegro5/allegro.h>
#include <allegro5/allegro_native_dialog.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

void jogadorAtaque(){

}

void atualizaEntidades(){ //desenha e anima as entidades, pode ser preciso usar varios casos

}

int main(){
    if(!inic()){
        msgErro("Deu ruim na inicialização!");
        destroi();
        return 0;
    }
    if(!cria()){
        msgErro("Deu ruim na criação!");
        destroi();
        return 0;
    }
    while (estado!=estSaida){
        if (estado == estPreMenu)
            preMenu();
        else if (estado == estMenu)
            menu();
        else if (estado == estPreJogo)
            preJogo();
        else if (estado == estJogo)
            jogo();
    }
    destroi();
    return 0;
}
