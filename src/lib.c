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

const float VELOCIDADE = 5.0;
const float HP=100.0;
const float ATAQUE=10.0;
const float DEFESA=5.0;
const int FASES=3;

int LARGURA, ALTURA;
int RAZAO_X, RAZAO_Y;
int opcao;
int estado = 1;
int nEntidades = 0; int nEntiParadas = 0;
bool sexo;
int faseAtual = 0;
int largTile[3]; int altTile[3];
float pxFundo = 0; float pyFundo = 0;
float escala = 1.0f; float escalaVelocidade = 0.0f;

ALLEGRO_DISPLAY *janela = NULL;
ALLEGRO_EVENT_QUEUE *filaEventos = NULL;
ALLEGRO_TIMER *timer = NULL;
ALLEGRO_TIMER *timer15 = NULL;
ALLEGRO_FONT *retroFont = NULL;
ALLEGRO_FONT *retroFont32 = NULL;
ALLEGRO_BITMAP *fundo[3];
ALLEGRO_TRANSFORM camera;

Entidade player;
Entidade *entidades = NULL;
EntidadeParada *entParadas = NULL;
//variaveis pra saber se deve ou não ser criado entidades na tela
bool criaEnt; bool criaEntParada;

//destroi o que der
void destroi(){
    al_destroy_timer(timer);
    al_destroy_timer(timer15);
    al_destroy_display(janela);
    al_destroy_event_queue(filaEventos);
    al_destroy_font(retroFont);
    al_destroy_font(retroFont32);
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
            fundo[i]=al_load_bitmap("./bin/backgrounds/tile1.bmp");
            if(!fundo[i]){
                msgErro("Deu ruim no fundo 0!");
            }
            break;
        case 1:
            fundo[i]=al_load_bitmap("./bin/backgrounds/tile2.bmp");
            if(!fundo[i]){
                msgErro("Deu ruim no fundo 1!");
            }
            break;
        default: //so pra n dar merda
            fundo[i]=al_load_bitmap("./bin/backgrounds/tile1.bmp");
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
    timer15 = al_create_timer(4.0 / FPS);
    if((!timer) || (!timer15)){
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
    retroFont = al_load_font("./fonts/retroGaming.ttf", 20, 0);
    retroFont32 = al_load_font("./fonts/retroGaming.ttf", 32, 0);
    if((!retroFont) || (!retroFont32)){
        msgErro("Falha ao carregar fonte");
        return 0;
    }
    al_register_event_source(filaEventos, al_get_display_event_source(janela));
    al_register_event_source(filaEventos, al_get_timer_event_source(timer));
    al_register_event_source(filaEventos, al_get_timer_event_source(timer15));
    al_register_event_source(filaEventos, al_get_keyboard_event_source());
    
    return 1;
}

void aumentaEntidades(){
    nEntidades++;
    entidades = (Entidade*)realloc(entidades,nEntidades*sizeof(Entidade));
    if(entidades==NULL){
        msgErro("Deu ruim na alocação!");
        estado=0;
    }
}
void aumentaEntidadesParadas(){
    nEntiParadas++;
    entParadas = (EntidadeParada*)realloc(entParadas,nEntiParadas*sizeof(EntidadeParada));
    if(entidades==NULL){
        msgErro("Deu ruim na alocação!");
        estado=0;
    }
}

//seta tudo antes do fluxo do menu
void preMenu(){
    int i = 0;
    bool sair = false;
    al_start_timer(timer15);
    while(!sair){
        ALLEGRO_EVENT evento;
        al_wait_for_event(filaEventos,&evento);
        switch (evento.type){
            case ALLEGRO_EVENT_TIMER:
                al_clear_to_color(al_map_rgb(0, 0, 0));
                al_draw_multiline_text(retroFont32, al_map_rgb(255, 255, 255), LARGURA / 2, ALTURA - i, LARGURA * 0.9, al_get_font_line_height(retroFont32) * 1.1, ALLEGRO_ALIGN_CENTER, "O ano é 20XX, e o mundo está no mais completo caos. Há meses, surgiram boatos de que a infame [...]--uma organização criminosa vinda do outro lado do mundo--havia liberado a Ameaça Vermelha, um vírus altamente contagioso que causava fome e frio em tamanha magnitude que chegava a ser letal. Entretanto, como ela havia sido extinta há quase 30 anos, a população desconsiderou o risco, pensando que aquela era apenas mais uma fake news.\n\nA solução para tal crise se encontra em Brasilia, onde o primeiro-ministro Adolf Solnorabo Mussolini é secretamente recrutado para proteger seus queridos cidadãos da terrível Ameaça. Ao questionar sobre o motivo de ser escolhido para realizar árdua tarefa, nosso bravo herói descobre que seu histórico de atleta o imunizou contra a nefária doença, tornando-o a pessoa mais qualificada para combatê-la.\n\n\n\nCabe a você salvar o mundo dessa terrível ameaça.");
                al_flip_display();
                i += 2;
                if(i >= 1.5 * ALTURA) sair = true;
                break;
            case ALLEGRO_EVENT_KEY_DOWN: //aperta uma tecla
                switch(evento.keyboard.keycode){
                    case ALLEGRO_KEY_ENTER:
                        sair = true;
                        break;
                }
                break;
        }
    }
    al_stop_timer(timer15);
    opcao = 0;
    estado++;
}

//fluxo do menu
void menu(){
    estado++;
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
    if(sexo) player.sprite=al_load_bitmap("./bin/entities/Char/Mchar.bmp");
    else player.sprite=al_load_bitmap("./bin/entities/Char/Fchar.bmp");
    //tudo isso aqui debaixo é bem hardcoded, depende do sprite msm infelizmente (na vdd eu tenho preguiça de fazer usando matematica entao eh isso)
    player.larguraSprite = 25; player.alturaSprite =  31;
    player.puloColuna=32; player.puloLinha=32;
    player.totalFrames=3;
    player.nDirecao[dBaixo]=0; player.nDirecao[dCima]=4;
    player.nDirecao[dEsquerda]=1; player.nDirecao[dDireita]=2;

    //zera as posiçoes e velocidades
    player.px=0; player.py=0;
    player.vx=0; player.vy=0;
    //o player sempre esta na tela
    player.naTela=true;


    if(!player.sprite) msgErro("Erro no sprite do player");
    al_convert_mask_to_alpha(player.sprite,al_map_rgb(0,0,0)); //isso aqui define a transparencia do sprite

    al_flush_event_queue(filaEventos); //da um wipe na fila inteira
    estado++;
}

void pauseJogo(){
    al_flush_event_queue(filaEventos);
    ALLEGRO_BITMAP *janelaPause;
    retroFont = al_load_font("./fonts/retroGaming.ttf", 50/escala, 0);
    janelaPause=al_load_bitmap("./bin/misc/UI/TextBox.bmp"); //carrega um bitmap de uma caixa de texto
    if(!janelaPause || !retroFont){
        msgErro("Deu ruim no pause!");
    }
    int alturaPause=al_get_bitmap_height(janelaPause);
    int larguraPause=al_get_bitmap_width(janelaPause);
    al_draw_scaled_bitmap(janelaPause,0,0,
                        larguraPause,alturaPause,(player.px-5*larguraPause/(2*escala)),(player.py-5*alturaPause/(2*escala)), //largura do bitmap, altura do bitmap,posicao x na tela, posicao y na tela
                        5*larguraPause/escala,5*alturaPause/escala,0); //tamanho desejado, altura desejada, flag
    al_draw_text(retroFont,al_map_rgb(0,0,0),player.px,player.py,ALLEGRO_ALIGN_CENTRE,"PAUSADO");
    al_flip_display(); //atualiza a tela
    bool despausa=false;
    while(!despausa){
        ALLEGRO_EVENT pausa;
        al_wait_for_event(filaEventos,&pausa);
        if(pausa.type==ALLEGRO_EVENT_KEY_DOWN) despausa=true; //se apertar enter sai do loop
    }
    al_flush_event_queue(filaEventos);
    retroFont = al_load_font("./fonts/retroGaming.ttf", 20, 0);
    al_destroy_bitmap(janelaPause);
    player.vx=0;player.vy=0;escalaVelocidade=0;
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
    bool colisaox=false; bool colisaoy=false;
    for (int i = 0; i < nEntidades; i++){
        if(entidades[i].px >= pxFundo && entidades[i].py >= pyFundo && entidades[i].px<=LARGURA+pxFundo && entidades[i].py<=ALTURA+pyFundo){
            entidades[i].naTela=true;
        }
        else entidades[i].naTela=false;
        
    }
    for (int i = 0; i < nEntiParadas; i++){
        if(entParadas[i].px >= pxFundo && entParadas[i].py >= pyFundo && entParadas[i].px<=LARGURA+pxFundo && entParadas[i].py<=ALTURA+pyFundo){
            entParadas[i].naTela=true;
        }
        else entParadas[i].naTela=false;
    }
    //algoritmo de colisao medindo em um raio do player se tem alguma entidade, vasculhando as posiçoes das entidades (pensei num jeito melhor de fazer isso mas por enquanto nao)
    if(colisaox) player.vx=0;
    if(colisaoy) player.vy=0;
    player.px+=player.vx;
    player.py+=player.vy;
}
int initEntidade(){
    if(rand()%2==0){
        entidades[nEntidades-1].sprite=al_load_bitmap("./bin/entities/Char/NPC.png");
        if(!entidades[nEntidades-1].sprite){
            msgErro("Deu ruim na entidade parada!");
            return 0;
        } //inicializando tudo (obs, nada definitivo)
        entidades[nEntidades-1].larguraSprite = al_get_bitmap_width(entidades[nEntidades-1].sprite); 
        entidades[nEntidades-1].alturaSprite =  al_get_bitmap_height(entidades[nEntidades-1].sprite);
        entidades[nEntidades-1].px=LARGURA/6+rand()%LARGURA; entidades[nEntidades-1].py=ALTURA/7+rand()%ALTURA;
        entidades[nEntidades-1].vx=0; entidades[nEntidades-1].vy=0;
    }
    else{
        entidades[nEntidades-1].sprite=al_load_bitmap("./bin/entities/Char/NPC1.png");
        if(!entidades[nEntidades-1].sprite){
            msgErro("Deu ruim na entidade parada!");
            return 0;
        } //inicializando tudo (obs, nada definitivo)
        entidades[nEntidades-1].larguraSprite = al_get_bitmap_width(entidades[nEntidades-1].sprite); 
        entidades[nEntidades-1].alturaSprite =  al_get_bitmap_height(entidades[nEntidades-1].sprite);
        entidades[nEntidades-1].px=LARGURA/3+rand()%LARGURA; entidades[nEntidades-1].py=ALTURA/4+rand()%ALTURA;
        entidades[nEntidades-1].vx=0; entidades[nEntidades-1].vy=0;
    }
    return 1;
    //aqui teoricamente deveriam ficar todas as possibilidades e tal
}

int initEntidadeParada(){
    if(rand()%2==0){
        entParadas[nEntiParadas-1].sprite=al_load_bitmap("./bin/entities/Clutter/lake.png");
        if(!entParadas[nEntiParadas-1].sprite){
            msgErro("Deu ruim na entidade parada!");
            return 0;
        }
        entParadas[nEntiParadas-1].larguraSprite = al_get_bitmap_width(entParadas[nEntiParadas-1].sprite);
        entParadas[nEntiParadas-1].alturaSprite = al_get_bitmap_height(entParadas[nEntiParadas-1].sprite);
        entParadas[nEntiParadas-1].px=LARGURA/9+rand()%LARGURA; entParadas[nEntiParadas-1].py=ALTURA/2+rand()%ALTURA;

    }
    else{
        entParadas[nEntiParadas-1].sprite=al_load_bitmap("./bin/entities/Clutter/Barrel.png");
        if(!entParadas[nEntiParadas-1].sprite){
            msgErro("Deu ruim na entidade parada!");
            return 0;
        }
        entParadas[nEntiParadas-1].larguraSprite = al_get_bitmap_width(entParadas[nEntiParadas-1].sprite);
        entParadas[nEntiParadas-1].alturaSprite = al_get_bitmap_height(entParadas[nEntiParadas-1].sprite);
        entParadas[nEntiParadas-1].px=LARGURA/8+rand()%LARGURA; entParadas[nEntiParadas-1].py=ALTURA/9+rand()%ALTURA;
    }
    return 1;
}

void geraEntidades(){ //gera as entidades, pode ser preciso usar varios casos
    if(rand()%178==0){ 
        aumentaEntidades();
        if(!initEntidade()) estado=0;

    }
    if(rand()%154==0){
        aumentaEntidadesParadas();
        if(!initEntidadeParada()) estado=0;

    }
}

void atualizaEntidades(){
    for (int i = 0; i < nEntidades; i++){
        if(entidades[i].naTela){
            al_draw_bitmap(entidades[i].sprite,entidades[i].px,entidades[i].py,0);
        }
        
    }
    for (int i = 0; i < nEntiParadas; i++){
        if(entParadas[i].naTela){
            al_draw_bitmap(entParadas[i].sprite,entParadas[i].px,entParadas[i].py,0);
        }
    }
}

void atualizaJogador(bool anda){ //desenha e anima o jogador
    if(anda){ //anima o sprite usando vx, vy, e a estrutura do player (dps eu implemento)

    }
    al_draw_bitmap_region(player.sprite, //muito feio filho
                          player.pDesenhox,player.pDesenhoy,
                          player.larguraSprite,player.alturaSprite,
                          player.px,player.py,0);
    
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
                        player.vx=0;player.vy=0;escalaVelocidade=0;
                        al_flush_event_queue(filaEventos);
                        break;
                    case ALLEGRO_KEY_Q:
                        escalaVelocidade+= 0.1f;
                        break;
                    case ALLEGRO_KEY_E:
                        escalaVelocidade-=0.1f;
                        break;
                    case ALLEGRO_KEY_ENTER:
                        al_stop_timer(timer);
                        pauseJogo();
                        al_start_timer(timer);
                        break;
                    case ALLEGRO_KEY_SPACE:
                        //jogadorAtaque();
                        break;
                }
                break;
            case ALLEGRO_EVENT_KEY_UP: //solta a tecla
                switch(evento.keyboard.keycode){
                    case ALLEGRO_KEY_UP:
                        if(player.vy!=0) player.vy=0;
                        break;
                    case ALLEGRO_KEY_DOWN:
                        if(player.vy!=0) player.vy=0;
                        break;
                    case ALLEGRO_KEY_RIGHT:
                        if(player.vx!=0) player.vx=0;
                        break;
                    case ALLEGRO_KEY_LEFT:
                        if(player.vx!=0) player.vx=0;
                        break;
                    case ALLEGRO_KEY_Q:
                        if(escalaVelocidade!=0) escalaVelocidade=0;
                        break;
                    case ALLEGRO_KEY_E:
                        if(escalaVelocidade!=0) escalaVelocidade=0;
                        break;
                }
                break;
            case ALLEGRO_EVENT_TIMER:
                desenhe=true;
                
                escala+=escalaVelocidade; //soma a velocidade da escala à escala, analogo ao movimento normal
                if(escala<1.0f) escala=1.0f; //limita o zoom pra n bugar 
                if(escala>5.0f) escala=5.0f; //zoom maximo
                geraEntidades();
                colisaoJogador(); //botei a colisão aqui pra ficar mais bonitinho, atualizo o px e py
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