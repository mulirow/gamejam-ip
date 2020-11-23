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

#define FPS 60.0

enum ESTADO{estSaida, estPreMenu, estMenu, estPreJogo, estJogo};
enum DIRECOES{dBaixo,dCima,dDireita,dEsquerda}; //esse enum é pra nao ter que ficar lembrando que numero é cada posição no vetor do struct de entidades

//velocidade das entidades no geral
const float VELOCIDADE=5.0;
//vida das entidades no geral
const float HP=100.0;
//ataque das entidades no geral
const float ATAQUE=10.0;
//defesa da entidades no geral
const float DEFESA=5.0;
//total de fases no jogo
const int FASES=3;

int LARGURA; int ALTURA;
//as razões entre o tamanho dos tiles e a altura da tela, essencial pra desenhar na tela
int RAZAO_X; int RAZAO_Y;
//diz respeito ao fluxo do jogo, com -1 sendo a saída
int estado = 1;
//diz respeito à opção no menu
int opcao;
//numero de entidades totais;
int nEntidades=0;
//( ͡° ͜ʖ ͡°)
bool sexo;
//fase atual
int faseAtual=0;
//altura e largura de cada tile, pode ser redundante
int largTile[3]; int altTile[3];
//usado no desenho do bitmap do fundo e na camera
float pxFundo=0; float pyFundo=0;
//escala da camera e velocidade com que ela escala
float escala=1.0f; float escalaVelocidade=0.0f;


ALLEGRO_DISPLAY *janela=NULL; //janela de saida padrao
ALLEGRO_EVENT_QUEUE *filaEventos=NULL; //fila de eventos padrao
ALLEGRO_TIMER *timer=NULL; //timer padrao
ALLEGRO_FONT *retroFont=NULL; //fonte padrao (deve ser amplificado)
ALLEGRO_BITMAP *fundo[3]; //fundo do jogo
ALLEGRO_TRANSFORM camera; //usado pra movimentar a camera

//estrutura geral das entidades do jogo, talvez seja alterado
typedef struct{
    float vx; float vy; //velocidades
    int px; int py; //posiçoes
    float hp; //HP total
    float atk; //dano de ataque
    float def; //defesa
    ALLEGRO_BITMAP *sprite; //sprite da entidade
    //auto-explicativos
    int alturaSprite; int larguraSprite;
    //pulo de linha pra uma direcao pra outra e pulo de coluna de um sprite pro outro
    int puloLinha; int puloColuna;
    //quantos frames tem a animação do sprite
    int totalFrames;
    //talvez seja reduntante, mas pra deixar mais legivel, guarda em que linha cada sprite de direção se encontra
    int nDirecao[4];
    //posiçao x e y do desenho do sprime no bitmap(bmp) original
    int pDesenhox; int pDesenhoy;
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
    for (int i = 0; i < FASES; i++){
        al_destroy_bitmap(fundo[i]);
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
    //if(!al_install_audio()){ msgErro("Falha ao inicializar o audio"); return 0;}
    //if(!al_init_acodec_addon()){msgErro("Falha ao inicializar o codec de audio");return 0;}
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

void geraMundo(int i){ 
    //aqui simplesmente carrega os fundos como cada tile, fiz um switch case com i e tudo mais so pra caso precise botar mais fases e etc, mas qualquer coisa a gente bota um load direto msm
    switch (i){
        case 0:
            fundo[i]=al_load_bitmap("bin/backgrounds/tile1.bmp");
            if(!fundo[i]){
                msgErro("Deu ruim no fundo 0!");
            }
            break;
        case 1:
            fundo[i]=al_load_bitmap("bin/backgrounds/tile2.bmp");
            if(!fundo[i]){
                msgErro("Deu ruim no fundo 1!");
            }
            break;
        default: //so pra n dar merda
            fundo[i]=al_load_bitmap("bin/backgrounds/tile1.bmp");
            if(!fundo[i]){
                msgErro("Deu ruim no fundo 2!");
            }
            break;
    }
    largTile[i]=al_get_bitmap_width(fundo[i]);
    altTile[i]=al_get_bitmap_height(fundo[i]);
    RAZAO_X=LARGURA/largTile[i]; //defino as razoes, utilidade explicada na declaração
    RAZAO_Y=ALTURA/altTile[i];

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

    al_set_window_title(janela, "Jogo");

    //bem redundante, mas so mesmo por enquanto
    for (int i = 0; i < FASES; i++){
        geraMundo(i);
    }
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
    
    return 1;
}

//seta tudo antes do fluxo do menu
void preMenu(){
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
//seta tudo antes do fluxo do jogo, talvez possa ser repetido a cada mudança de fase(?)
void preJogo(){
    //pergunta como a pessoa quer jogar
    if(al_show_native_message_box(janela,"Escolha do Sexo","Escolha com cautela:","Você prefere jogar com macho ou fêmea?","M|F",ALLEGRO_MESSAGEBOX_OK_CANCEL)%2!=0){
        sexo=1;
    }
    else sexo=0;
    player.hp=HP;
    player.atk=ATAQUE;
    player.def=DEFESA;
    if(sexo) player.sprite=al_load_bitmap("bin/entities/Char/Mchar.bmp");
    else player.sprite=al_load_bitmap("bin/entities/Char/Fchar.bmp");
    //tudo isso aqui debaixo é bem hardcoded, depende do sprite msm infelizmente (na vdd eu tenho preguiça de fazer usando matematica entao eh isso)
    player.larguraSprite = 25; player.alturaSprite =  31;
    player.puloColuna=32; player.puloLinha=32;
    player.totalFrames=3;
    player.nDirecao[dBaixo]=0; player.nDirecao[dCima]=4;
    player.nDirecao[dEsquerda]=1; player.nDirecao[dDireita]=2;

    //zera as posiçoes e velocidades
    player.px=0; player.py=0;
    player.vx=0; player.vy=0;


    if(!player.sprite) msgErro("Erro no sprite do player");
    al_convert_mask_to_alpha(player.sprite,al_map_rgb(0,0,0));
    aumentaEntidades();

    estado++;
}
void jogadorAtaque(){

}
void pauseJogo(){
    ALLEGRO_BITMAP *janelaPause;
    janelaPause=al_load_bitmap("bin/misc/UI/TextBox.bmp"); //carrega um bitmap de uma caixa de texto
    if(!janelaPause){
        msgErro("Deu ruim no pause!");
    }
    al_draw_bitmap(janelaPause,LARGURA/2,ALTURA/2,0); //desenha ela sobre a tela
    al_draw_text(retroFont,al_map_rgb(0,0,0),LARGURA/2,ALTURA/2,ALLEGRO_ALIGN_CENTER,"PAUSADO");
    al_flip_display(); //atualiza a tela
    ALLEGRO_EVENT pausa;
    al_wait_for_event(filaEventos,&pausa);
    bool despausa=0;
    while(!despausa){
        if(pausa.keyboard.keycode==ALLEGRO_KEY_ENTER) despausa=1; //se apertar enter sai do loop
    }
    al_destroy_bitmap(janelaPause);
    al_flip_display();
}
void atualizaCamera(){ //atualiza as posiçoes das coisas
    pxFundo=-(LARGURA/2)+(player.px+player.larguraSprite/2); //magia negra (geometria)
    pyFundo=-(ALTURA/2)+(player.py+player.alturaSprite/2);
    if(pxFundo<0) pxFundo=0; //n deixa passar do 0
    if(pyFundo<0) pyFundo=0;
}

void desenhaMundo(){
    al_draw_scaled_bitmap(fundo[faseAtual],0,0,largTile[faseAtual],altTile[faseAtual],
                           pxFundo,pyFundo,LARGURA,ALTURA,0);
}
void colisaoJogador(){
    //algoritmo de colisao medindo em um raio do player se tem alguma entidade, vasculhando as posiçoes das entidades (pensei num jeito melhor de fazer isso mas por enquanto nao)
    player.px+=player.vx;
    player.py+=player.vy;
}
void atualizaJogador(bool anda){ //desenha e anima o jogador
    if(anda){ //anima o sprite usando vx, vy, e a estrutura do player (dps eu implemento)

    }
    colisaoJogador(); //aqui eu atualizo px e py 
    al_draw_bitmap_region(player.sprite, //muito feio filho
                          player.pDesenhox,player.pDesenhoy,
                          player.larguraSprite,player.alturaSprite,
                          player.px,player.py,0);
    
}

void atualizaEntidades(){ //desenha e anima as entidades, pode ser preciso usar varios casos

}

//aqui reserva um espaço pra criar as funçoes que desenham monstros, coisas aleatorias, construçoes, npcs, etc

//fluxo do jogo
void jogo(){
    al_start_timer(timer);
    //sai do loop do while
    bool sair=false;
    bool anda=false; //unica serventia desse bool é pra animação de movimento do personagem
    bool desenhe=false; //desenha o proximo frame
    while (!sair){
        ALLEGRO_EVENT evento;
        al_wait_for_event(filaEventos,&evento);
        switch (evento.type){
            case ALLEGRO_EVENT_KEY_DOWN: //aperta uma tecla
                switch(evento.keyboard.keycode){
                    case ALLEGRO_KEY_UP:
                        player.vy-=VELOCIDADE;
                        anda=true;
                        break;
                    case ALLEGRO_KEY_DOWN:
                        player.vy+=VELOCIDADE;
                        anda=true;
                        break;
                    case ALLEGRO_KEY_RIGHT:
                        player.vx+=VELOCIDADE;
                        anda=true;
                        break;
                    case ALLEGRO_KEY_LEFT:
                        player.vx-=VELOCIDADE;
                        anda=true;
                        break;
                    case ALLEGRO_KEY_ESCAPE:
                        //a função retorna 0, 1 ou 2. 0 se a janela for fechada, 1 se for OK e 2 se for cancelar
                        if(al_show_native_message_box(janela,"Saída","Deseja sair do jogo?","Aperte OK para sair ou Cancelar para retornar ao estado do jogo",NULL,ALLEGRO_MESSAGEBOX_OK_CANCEL)%2!=0){
                            sair=true;
                            estado = 0;
                        }
                        break;
                    case ALLEGRO_KEY_Q:
                        escalaVelocidade+= 0.1f;
                        break;
                    case ALLEGRO_KEY_E:
                        escalaVelocidade-=0.1f;
                        break;
                    case ALLEGRO_KEY_ENTER:
                        pauseJogo();
                        break;
                    case ALLEGRO_KEY_SPACE:
                        jogadorAtaque();
                        break;
                }
                break;
            case ALLEGRO_EVENT_KEY_UP: //solta a tecla
                switch(evento.keyboard.keycode){
                    case ALLEGRO_KEY_UP:
                        player.vy+=VELOCIDADE;
                        break;
                    case ALLEGRO_KEY_DOWN:
                        player.vy-=VELOCIDADE;
                        break;
                    case ALLEGRO_KEY_RIGHT:
                        player.vx-=VELOCIDADE;
                        break;
                    case ALLEGRO_KEY_LEFT:
                        player.vx+=VELOCIDADE;
                        break;
                    case ALLEGRO_KEY_Q:
                        escalaVelocidade-= 0.1f;
                        break;
                    case ALLEGRO_KEY_E:
                        escalaVelocidade+= 0.1f;
                }
                break;
            case ALLEGRO_EVENT_TIMER:
                desenhe=true;
                escala+=escalaVelocidade; //soma a velocidade da escala à escala, analogo ao movimento normal
                if(escala<1.0f) escala=1.0f; //limita o zoom pra n bugar 
                if(escala>5.0f) escala=5.0f;
                atualizaCamera(); //aqui começa a magia negra
                al_identity_transform(&camera);
                al_translate_transform(&camera,-(player.px+player.larguraSprite/2),-(player.py+player.larguraSprite/2)); //basicamente transforma tudo que ta na tela de acordo com esses parametros, eu vou mandar os videos que eu vi ensinando isso pq admito que nem eu entendi direito kkkkkk
                al_scale_transform(&camera,escala,escala); //esse é o mais simples
                al_translate_transform(&camera,-pxFundo+(player.px+player.larguraSprite/2),-pyFundo+(player.py+player.larguraSprite/2)); 
                al_use_transform(&camera); //simplesmente torna a transformação "canonica"
                break;
            case ALLEGRO_EVENT_DISPLAY_CLOSE:
                sair = true;
                desenhe = false;
                break;
        }
        if(desenhe){
            desenhaMundo(); //desenha o fundo da fase atual
            if(player.vx==0 || player.vy==0) anda=false;
            atualizaJogador(anda);
            atualizaEntidades();
            al_draw_textf(retroFont,al_map_rgb(0,0,0),4*LARGURA/5,4*ALTURA/5,ALLEGRO_ALIGN_CENTER,"evento atual=%d",evento.type);
            al_flip_display();
            desenhe=0;
        }
    }
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
