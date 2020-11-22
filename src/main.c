#include <allegro5/allegro.h>
#include <allegro5/allegro_native_dialog.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_acodec.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <stdio.h>
#include <stdlib.h>

#define FPS 60.0
#define VEL 5
#define ESTADO_SAIDA -1
#define ESTADO_PRE_MENU 0
#define ESTADO_MENU 1
#define ESTADO_PRE_JOGO 2
#define ESTADO_JOGO 3
#define RETRO_TAMANHO 20

int LARGURA;
int ALTURA;
//diz respeito ao fluxo do jogo, com -1 sendo a saída
int estado=0;
//diz respeito à opção no menu
int opcao;

ALLEGRO_DISPLAY *janela; //janela de saida padrao
ALLEGRO_EVENT_QUEUE *fila_eventos; //fila de eventos padrao
ALLEGRO_TIMER *timer; //timer padrao
ALLEGRO_FONT *retro_font; //fonte padrao (deve ser amplificado)

//mensagem de erro, deve ser usado na verificação de inicializações
void msg_erro(char *t){
    al_show_native_message_box(NULL,"ERRO","Ocorreu o seguinte erro:",t,NULL,ALLEGRO_MESSAGEBOX_ERROR);
}

//inicializa os addons
int inic(){
    if(!al_init()){
        msg_erro("Falha ao inicializar a Allegro");
        return 0;
    }
    if(!al_init_font_addon()){
        msg_erro("Falha ao inicializar o addon de fontes");
        return 0;
    }
    if(!al_init_ttf_addon()){
        msg_erro("Falha ao inicializar o addon ttf");
        return 0;
    }
    if (!al_init_image_addon()){
        msg_erro("Falha ao inicializar o addon de imagens");
        return 0;
    }
    if (!al_install_keyboard()){
        msg_erro("Falha ao inicializar o teclado");
        return 0;
    }
    if(!al_install_audio()){
        msg_erro("Falha ao inicializar o audio");
        return 0;
    }
    if(!al_init_acodec_addon()){
        msg_erro("Falha ao inicializar o codec de audio");
        return 0;
    }
     if(!al_install_mouse()){
        msg_erro("Falha ao iniciar o mouse");
        return 0;
    }
    if (!al_init_primitives_addon()){
        msg_erro("Falha ao inicializar as primitivas");
        return 0;
    }
    return 1;
}

//cria tudo que será necessário no jogo
int cria(){
    timer = al_create_timer(1.0 / FPS);
    if(!timer) {
        msg_erro("Falha ao criar temporizador");
        return 0;
    }
    al_set_new_display_flags(ALLEGRO_FULLSCREEN_WINDOW); //flag do fullscreen
    janela = al_create_display(1280,720);
    if(!janela) {
        msg_erro("Falha ao criar janela");
        return 0;
    }

    ALTURA = al_get_display_height(janela);
    LARGURA = al_get_display_width(janela);

    al_set_window_title(janela, "Prototipo");

    fila_eventos = al_create_event_queue();
    if(!fila_eventos) {
        msg_erro("Falha ao criar fila de eventos");
        return 0;
    }
    retro_font = al_load_font("fonts/retroGaming.ttf", RETRO_TAMANHO, 0);
    if(!retro_font){
        msg_erro("Falha ao carregar fonte");
        return 0;
    }
    al_register_event_source(fila_eventos, al_get_display_event_source(janela));
    al_register_event_source(fila_eventos, al_get_timer_event_source(timer));
    al_register_event_source(fila_eventos, al_get_keyboard_event_source());
    al_start_timer(timer);
    return 1;
}

//seta tudo antes do fluxo do menu
void pre_menu(){
    opcao=0;
    estado++;
}

//fluxo do menu
void menu(){

}

//seta tudo antes do fluxo do jogo
void pre_jogo(){

}

//fluxo do jogo
void jogo(){

}

//destroi tudo que foi criado
void destroi(){
    al_destroy_timer(timer);
    al_destroy_display(janela);
    al_destroy_event_queue(fila_eventos);
    al_destroy_font(retro_font);
}

int main(){
    if(!inic()){
        msg_erro("Deu ruim 1!");
        destroi();
        return 0;
    }
    if(!cria()){
        msg_erro("Deu ruim 2!");
        destroi();
        return 0;
    }
    while (estado!=ESTADO_SAIDA){
        if (estado == ESTADO_PRE_MENU)
            pre_menu();
        else if (estado == ESTADO_MENU)
            menu();
        else if (estado == ESTADO_PRE_JOGO)
            pre_jogo();
        else if (estado == ESTADO_JOGO)
            jogo();
    }
    destroi();
    return 0;
}