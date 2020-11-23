#include "lib.h"
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

enum ESTADO{estSaida, estPreMenu, estMenu, estPreJogo, estJogo};

//velocidade das entidades no geral
#define VELOCIDADE 5.0;
//vida das entidades no geral
#define HP 100.0;
//ataque das entidades no geral
#define ATAQUE 10.0;
//defesa da entidades no geral
#define DEFESA 5.0;
//total de fases no jogo
#define FASES 3;

int LARGURA; int ALTURA;
int LARGURA_F; int ALTURA_F;
//diz respeito ao fluxo do jogo, com -1 sendo a saída
int estado = 1;
//diz respeito à opção no menu
int opcao;
//numero de entidades totais;
int nEntidades=0;

ALLEGRO_DISPLAY *janela; //janela de saida padrao
ALLEGRO_EVENT_QUEUE *filaEventos; //fila de eventos padrao
ALLEGRO_TIMER *timer; //timer padrao
ALLEGRO_FONT *retroFont; //fonte padrao (deve ser amplificado)
ALLEGRO_BITMAP *fundo[3]; //fundo do jogo, coloquei '3' pois o valor deve ser constante

//estrutura geral das entidades do jogo, talvez seja alterado
typedef struct{
    float vx; float vy; //velocidades
    int px; int py; //posiçoes
    float hp; //HP total
    float atk; //dano de ataque
    float def; //defesa
    ALLEGRO_BITMAP *sprite; //sprite da entidade
    int alturaSprite; //auto-explicativos
    int larguraSprite;
} Entidade;

Entidade player;
Entidade *entidades=NULL;

//destroi o que der
void destroi(){
    al_destroy_timer(timer);
    al_destroy_display(janela);
    al_destroy_event_queue(filaEventos);
    al_destroy_font(retroFont);
    for (int i = 0; i < nEntidades; i++){
        al_destroy_bitmap(entidades[i].sprite);
    }
    al_destroy_bitmap(player.sprite);
    
}

//mensagem de erro, deve ser usado na verificação de inicializações
void msgErro(char *t){
    al_show_native_message_box(NULL,"ERRO","Ocorreu o seguinte erro:",t,NULL,ALLEGRO_MESSAGEBOX_ERROR);
    estado = 0;
}

//inicializa os addons
int inic(){
    if(!al_init()){
        msgErro("Falha ao inicializar a Allegro");
        return 0;
    }
    if(!al_init_font_addon()){
        msgErro("Falha ao inicializar o addon de fontes");
        return 0;
    }
    if(!al_init_ttf_addon()){
        msgErro("Falha ao inicializar o addon ttf");
        return 0;
    }
    if (!al_init_image_addon()){
        msgErro("Falha ao inicializar o addon de imagens");
        return 0;
    }
    if (!al_install_keyboard()){
        msgErro("Falha ao inicializar o teclado");
        return 0;
    }
    if(!al_install_audio()){
        msgErro("Falha ao inicializar o audio");
        return 0;
    }
    if(!al_init_acodec_addon()){
        msgErro("Falha ao inicializar o codec de audio");
        return 0;
    }
     if(!al_install_mouse()){
        msgErro("Falha ao iniciar o mouse");
        return 0;
    }
    if (!al_init_primitives_addon()){
        msgErro("Falha ao inicializar as primitivas");
        return 0;
    }
    return 1;
}

//cria tudo que será necessário no jogo
int cria(){
    timer = al_create_timer(1.0 / FPS);
    if(!timer) {
        msgErro("Falha ao criar temporizador");
        return 0;
    }
    al_set_new_display_flags(ALLEGRO_FULLSCREEN_WINDOW); //flag do fullscreen
    janela = al_create_display(1280,720);
    if(!janela) {
        msgErro("Falha ao criar janela");
        return 0;
    }

    ALTURA = al_get_display_height(janela);
    LARGURA = al_get_display_width(janela);

    al_set_window_title(janela, "Prototipo");

    filaEventos = al_create_event_queue();
    if(!filaEventos) {
        msgErro("Falha ao criar fila de eventos");
        return 0;
    }
    retroFont = al_load_font("fonts/retroGaming.ttf", 20, 0);
    if(!retroFont){
        msgErro("Falha ao carregar fonte");
        return 0;
    }
    al_register_event_source(filaEventos, al_get_display_event_source(janela));
    al_register_event_source(filaEventos, al_get_timer_event_source(timer));
    al_register_event_source(filaEventos, al_get_keyboard_event_source());
    al_start_timer(timer);
    return 1;
}

//seta tudo antes do fluxo do menu
void pre_menu(){
    opcao = 0;
    estado++;
}

//fluxo do menu
void menu(){
    estado++;
}

void aumentaEntidades(){
    nEntidades++;
    entidades = (Entidade*)realloc(entidades,nEntidades*sizeof(Entidade));
}

//seta tudo antes do fluxo do jogo
void pre_jogo(){
    player.hp=HP;
    player.atk=ATAQUE;
    player.def=DEFESA;
    player.sprite=al_load_bitmap("bin/entities/player.bmp");
    //player.larguraSprite = ...
    //player.alturaSprite = ... ambos so podem ser definidos quando eu arranjar os sprites
    if(!player.sprite) msgErro("Erro no sprite do player");
    aumentaEntidades();
    fundo[0]=al_load_bitmap("bin/background/fase0.bmp");
    fundo[1]=al_load_bitmap("bin/background/fase1.bmp");
    fundo[2]=al_load_bitmap("bin/background/fase2.bmp");
    fundo[3]=al_load_bitmap("bin/background/fase3.bmp");
    //LARGURA_F=... 
    //ALTURA_F=... altura e largura do fundo
    estado++;
}

//fluxo do jogo
void jogo(){
    //sai do loop do while
    bool sair=false;
    //desenha a proxima tela
    bool desenhe=false;
    while (!sair){
        ALLEGRO_EVENT evento;
        al_wait_for_event(filaEventos,&evento);
        switch (evento.type){
            case ALLEGRO_EVENT_KEY_DOWN:
                switch(evento.keyboard.keycode){
                    case ALLEGRO_KEY_UP:
                        player.vx-=VELOCIDADE;
                        break;
                    case ALLEGRO_KEY_DOWN:
                        player.vx+=VELOCIDADE;
                        break;
                    case ALLEGRO_KEY_RIGHT:
                        player.vy+=VELOCIDADE;
                        break;
                    case ALLEGRO_KEY_LEFT:
                        player.vy-=VELOCIDADE;
                        break;
                    case ALLEGRO_KEY_ESCAPE:
                        //a função retorna 0, 1 ou 2. 0 se a janela for fechada, 1 se for OK e 2 se for cancelar
                        if(al_show_native_message_box(janela,"Saída","Deseja sair do jogo?","Aperte OK para sair ou Cancelar para retornar ao estado do jogo",NULL,ALLEGRO_MESSAGEBOX_OK_CANCEL)%2!=0){
                            sair=true;
                            estado = 0;
                        }
                        break;
                    case ALLEGRO_KEY_ENTER:
                        pause_jogo();
                        break;
                    case ALLEGRO_KEY_SPACE:
                        jogador_ataque();
                        break;
                }
                break;
            case ALLEGRO_EVENT_KEY_UP:
                break;
            case ALLEGRO_EVENT_TIMER:
                desenhe=true;
                break;
        }
    }
}


int main(){
    if(!inic()){
        msgErro("Deu ruim 1!");
        destroi();
        return 0;
    }
    if(!cria()){
        msgErro("Deu ruim 2!");
        destroi();
        return 0;
    }
    while (estado!=estSaida){
        if (estado == estPreMenu)
            pre_menu();
        else if (estado == estMenu)
            menu();
        else if (estado == estPreJogo)
            pre_jogo();
        else if (estado == estJogo)
            jogo();
    }
    destroi();
    return 0;
}
