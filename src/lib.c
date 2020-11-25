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
//comentarios sobre globais estao todos no header

const float VELOCIDADE = 5.0;
const float HP=100.0;
const float ATAQUE=10.0;
const float DEFESA=5.0;
const int FASES=3;
const float RAIO_P=50;


int LARGURA, ALTURA;
int RAZAO_X, RAZAO_Y;
int opcao;
int estado = 1;
int nEntidades = 0; int nBlocos = 0;
bool sexo;
int faseAtual = 0;
int largTile[3]; int altTile[3];
float pxFundo = 0; float pyFundo = 0;
float escala = 1.0f; float escalaVelocidade = 0.0f;

ALLEGRO_DISPLAY *janela = NULL;
ALLEGRO_EVENT_QUEUE *filaEventos = NULL;
ALLEGRO_TIMER *timer = NULL;
ALLEGRO_TIMER *timerAlt = NULL;
ALLEGRO_TIMER *timerTeste=NULL;
ALLEGRO_FONT *retroFont = NULL;
ALLEGRO_FONT *retroFont32 = NULL;
ALLEGRO_BITMAP *fundo[3];
ALLEGRO_TRANSFORM camera;

Entidade player;
Entidade *entidades = NULL;
Bloco *blocos = NULL;
bool criaEnt; bool criaBloco;
bool mostraHitbox=false;
bool reinicio=false;

//destroi o que der
void destroi(){
    al_destroy_timer(timer);
    al_destroy_timer(timerAlt);
    al_destroy_timer(timerTeste);
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
    for (int i = 0; i < nBlocos; i++){
        al_destroy_bitmap(blocos[i].sprite);
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
    //addon de audio aparentemente bugado
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
    //pega a largura de cada tile, pra função de desenho
    largTile[i]=al_get_bitmap_width(fundo[i]);
    //pega a altura de cada tile , pra função de desenho
    altTile[i]=al_get_bitmap_height(fundo[i]);
    //defino as razoes, utilidade explicada na declaração
    RAZAO_X=LARGURA/largTile[i]; 
    RAZAO_Y=ALTURA/altTile[i];
}

//cria tudo que será necessário no jogo
int cria(){
    timer = al_create_timer(1.0 / FPS);
    timerAlt = al_create_timer(8.0 / FPS);
    timerTeste = al_create_timer(60.0 / FPS);
    if((!timer) || (!timerAlt)){
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
    al_register_event_source(filaEventos, al_get_timer_event_source(timerAlt));
    al_register_event_source(filaEventos,al_get_timer_event_source(timerTeste));
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
void aumentaBlocos(){
    nBlocos++;
    blocos = (Bloco*)realloc(blocos,nBlocos*sizeof(Bloco));
    if(blocos==NULL){
        msgErro("Deu ruim na alocação!");
        estado=0;
    }
}

//seta tudo antes do fluxo do menu
void preMenu(){
    int i = 0;
    bool sair = false;
    al_start_timer(timerAlt);
    while(!sair){
        ALLEGRO_EVENT evento;
        al_wait_for_event(filaEventos,&evento);
        switch (evento.type){
            case ALLEGRO_EVENT_TIMER:
                al_draw_multiline_text(retroFont32, al_map_rgb(255, 255, 255), LARGURA / 2, ALTURA - i, LARGURA * 0.9, al_get_font_line_height(retroFont32) * 1.1, ALLEGRO_ALIGN_CENTER, "O ano é 20XX, e o mundo está no mais completo caos. Há meses, surgiram boatos de que a infame [...]--uma organização criminosa vinda do outro lado do mundo--havia liberado a Ameaça Vermelha, um vírus altamente contagioso que causava fome e frio em tamanha magnitude que chegava a ser letal. Entretanto, como ela havia sido extinta há quase 30 anos, a população desconsiderou o risco, pensando que aquela era apenas mais uma fake news.\n\nA solução para tal crise se encontra em Brasilia, onde o primeiro-ministro Adolf Solnorabo Mussolini é secretamente recrutado para proteger seus queridos cidadãos da terrível Ameaça. Ao questionar sobre o motivo de ser escolhido para realizar árdua tarefa, nosso bravo herói descobre que seu histórico de atleta o imunizou contra a nefária doença, tornando-o a pessoa mais qualificada para combatê-la.\n\n\n\nCabe a você salvar o mundo dessa terrível ameaça.");
                al_flip_display();
                al_clear_to_color(al_map_rgb(0, 0, 0));
                i += 3;
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
    al_stop_timer(timerAlt);
    al_flush_event_queue(filaEventos);
    estado++;
}

//fluxo do menu
void menu(){
    ALLEGRO_MOUSE_STATE estadoMouse;
    opcao = 0; //indica qual botão que tá com highlight
    int i = 255, j = 120, temp;
    bool sair = false;
    al_start_timer(timer);
    while(!sair){
        ALLEGRO_EVENT evento;
        al_wait_for_event(filaEventos,&evento);
        switch (evento.type){
            case ALLEGRO_EVENT_TIMER:
                al_draw_text(retroFont32, al_map_rgb(i, i, i), LARGURA / 2, ALTURA / 2, ALLEGRO_ALIGN_CENTER, "Jogar");
                al_draw_text(retroFont32, al_map_rgb(j, j, j), LARGURA / 2, ALTURA / 2 + al_get_font_line_height(retroFont32), ALLEGRO_ALIGN_CENTER, "Sair");
                al_flip_display();
                al_clear_to_color(al_map_rgb(0, 0, 0));
                break;
            case ALLEGRO_EVENT_MOUSE_AXES:
                al_get_mouse_state(&estadoMouse);
                if((estadoMouse.x > 0) && (estadoMouse.x < 100)){
                    printf("Ae\n");
                }
                al_flip_display();
                break;
            case ALLEGRO_EVENT_KEY_DOWN: //aperta uma tecla
                switch(evento.keyboard.keycode){
                    case ALLEGRO_KEY_DOWN: 
                        opcao++;
                        temp = i;
                        i = j;
                        j = temp;
                        break;
                    case ALLEGRO_KEY_UP: 
                        opcao++;
                        temp = i;
                        i = j;
                        j = temp;
                        break;
                    case ALLEGRO_KEY_ENTER:
                        sair = true;
                }
                break;
        }
    }
    al_stop_timer(timer);
    al_flush_event_queue(filaEventos);
    if(opcao % 2 == 0){
        estado++;
    }
    else{
        estado = 0;
    }
}

//seta tudo antes do fluxo do jogo, talvez possa ser repetido a cada mudança de fase(?)
void preJogo(){
    //pergunta como a pessoa quer jogar
    if(al_show_native_message_box(janela,"Escolha do Sexo","Escolha com cautela:","Você prefere jogar com macho ou fêmea?","M|F",ALLEGRO_MESSAGEBOX_OK_CANCEL)%2!=0){
        sexo=1;
    }
    else sexo=0;
    //setando os atributos iniciais
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
    //0 utilidade, so pra ficar inicializado
    player.inimigo=false;
    player.escalaEntidade=1;
    //raio da hitbox de contato/hitbox de dano (com entidades)
    player.raio=player.alturaSprite/4+player.larguraSprite/4;
    if(reinicio){ //se reinicio for true, reseta as entidades e blocos
        nEntidades=0;
        nBlocos=0;
        blocos=NULL;
        entidades=NULL;
    }
    

    if(!player.sprite) msgErro("Erro no sprite do player");
    al_convert_mask_to_alpha(player.sprite,al_map_rgb(0,0,0)); //isso aqui define a transparencia do sprite

    al_flush_event_queue(filaEventos); //da um wipe na fila inteira
    estado++;
}
void fimDeJogo(){
    //bem besta
    if(al_show_native_message_box(janela,"Você morreu!","Que pena", "Os inimigos venceram, tentar novamente?","Claro!|Tô Fora",ALLEGRO_MESSAGEBOX_YES_NO)%2==0){
        estado=0;
    }
    //caso ele reinicie
    else{
        al_flush_event_queue(filaEventos);
        reinicio=1;
        estado=3;
    }
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
    al_draw_scaled_bitmap(janelaPause,0,0, //essa função desenha um bitmap na escala desejada
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

    for (int i = 0; i < nEntidades; i++){
        if(entidades[i].px+entidades[i].larguraSprite >= pxFundo && entidades[i].py+entidades[i].alturaSprite >= pyFundo && entidades[i].px<=LARGURA+pxFundo && entidades[i].py<=ALTURA+pyFundo){
            entidades[i].naTela=true;
        }
        else entidades[i].naTela=false;
        
    }
    for (int i = 0; i < nBlocos; i++){
        if(blocos[i].px+blocos[i].larguraSprite >= pxFundo && blocos[i].py+blocos[i].alturaSprite >= pyFundo && blocos[i].px<=LARGURA+pxFundo && blocos[i].py<=ALTURA+pyFundo){
            blocos[i].naTela=true;
        }
        else blocos[i].naTela=false;
    }

    //algoritmo de colisao medindo em um raio do player se tem alguma entidade, vasculhando as posiçoes das entidades
    for (int i = 0; i < nEntidades; i++){ 
        if(entidades[i].naTela){//se a entidade ta na tela
            //teorema de pitagoras (serio)
            double distancia=sqrt(pow(player.px+player.larguraSprite/2-entidades[i].px-entidades[i].larguraSprite/2,2)+pow(player.py+player.alturaSprite/2-entidades[i].py-entidades[i].alturaSprite/2,2));
            if(distancia<=player.raio+entidades[i].raio){ //isso significa que as circunferencias são secantes
                if(entidades[i].inimigo){ //se for inimigo, apanha
                    player.hp-=10;
                    //animação de dano feita nas coxas, simplesmente empurra o player
                    player.px-=3*player.vx;
                    player.py-=3*player.vy;
                }
            }
            if(distancia<=RAIO_P*entidades[i].escalaEntidade){ //se ele tiver dentro do raio de procura
                float seno=(player.py-entidades[i].py)/distancia; //olha ele ai
                float cosseno=(player.px-entidades[i].px)/distancia;
                entidades[i].vx=+VELOCIDADE*cosseno/1.2; //geometria 
                entidades[i].vy=+VELOCIDADE*seno/1.2;
            }
            else{
                entidades[i].vx=0; //se n tiver no raio, fica quietinho
                entidades[i].vy=0;
            }
            entidades[i].px+=entidades[i].vx; //atualiza a posição
            entidades[i].py+=entidades[i].vy; 
        }
    }
    //colisao do jogador com os blocos
    
    for(int j=0; j<nBlocos;j++){
        bool colisao=false;
        if(j<0) j = 0; //evitar bugs no j--
        if(blocos[j].naTela){  //n tem mt o que explicar aqui, so desenhando msm (serio)
            if (player.px+player.larguraSprite > blocos[j].px && player.px < blocos[j].px+blocos[j].larguraSprite && player.py+player.alturaSprite > blocos[j].py && player.py < blocos[j].py+blocos[j].alturaSprite) colisao=true;
            else colisao=false;
        }
        if((player.direcao[dCima] || player.direcao[dBaixo]) && colisao){
                player.py-=player.vy/5; //uma lenta porem eficiente solução ao problema das colisões
                j--;
        } 
        if((player.direcao[dDireita] || player.direcao[dEsquerda]) && colisao){
                player.px-=player.vx/5;
                j--;
        }
    }

}
int initEntidade(){
    if(rand()%2==0){
        entidades[nEntidades-1].sprite=al_load_bitmap("./bin/entities/Char/NPC.png");
        if(!entidades[nEntidades-1].sprite){
            msgErro("Deu ruim nos blocos!");
            return 0;
        } //inicializando tudo (obs, nada definitivo)
        entidades[nEntidades-1].larguraSprite = 1.7*al_get_bitmap_width(entidades[nEntidades-1].sprite); 
        entidades[nEntidades-1].alturaSprite =  1.7*al_get_bitmap_height(entidades[nEntidades-1].sprite);
        entidades[nEntidades-1].px=LARGURA/6+rand()%LARGURA; entidades[nEntidades-1].py=ALTURA/7+rand()%ALTURA;
        entidades[nEntidades-1].vx=0; entidades[nEntidades-1].vy=0;
        entidades[nEntidades-1].inimigo=false;
        entidades[nEntidades-1].escalaEntidade=1.7;
        entidades[nEntidades-1].raio=entidades[nEntidades-1].larguraSprite/4+entidades[nEntidades-1].alturaSprite/4;

    }
    else{
        entidades[nEntidades-1].sprite=al_load_bitmap("./bin/entities/Char/NPC1.png");
        if(!entidades[nEntidades-1].sprite){
            msgErro("Deu ruim nos blocos!");
            return 0;
        } //inicializando tudo (obs, nada definitivo)
        entidades[nEntidades-1].larguraSprite = 1.7*al_get_bitmap_width(entidades[nEntidades-1].sprite); 
        entidades[nEntidades-1].alturaSprite =  1.7*al_get_bitmap_height(entidades[nEntidades-1].sprite);
        entidades[nEntidades-1].px=LARGURA/3+rand()%LARGURA; entidades[nEntidades-1].py=ALTURA/4+rand()%ALTURA;
        entidades[nEntidades-1].vx=VELOCIDADE/5; entidades[nEntidades-1].vy=VELOCIDADE/20;
        entidades[nEntidades-1].inimigo=true;
        entidades[nEntidades-1].escalaEntidade=1.7;
        entidades[nEntidades-1].raio=entidades[nEntidades-1].larguraSprite/4+entidades[nEntidades-1].alturaSprite/4;
    }
    return 1;
    //aqui teoricamente deveriam ficar todas as possibilidades e tal
}

int initBloco(){
    if(rand()%2==0){
        //inicializando tudo
        blocos[nBlocos-1].sprite=al_load_bitmap("./bin/entities/Clutter/lake.png");
        if(!blocos[nBlocos-1].sprite){
            msgErro("Deu ruim nos blocos!");
            return 0;
        }
        blocos[nBlocos-1].larguraSprite = 5*al_get_bitmap_width(blocos[nBlocos-1].sprite);
        blocos[nBlocos-1].alturaSprite = 5*al_get_bitmap_height(blocos[nBlocos-1].sprite);
        blocos[nBlocos-1].px=LARGURA/9+rand()%LARGURA; blocos[nBlocos-1].py=ALTURA/2+rand()%ALTURA;
        bool colisao=false;
        for (int i = 0; i < nBlocos-1; i++){ //verifica se tem algum outro bloco na mesma area
            if(blocos[nBlocos-1].px+blocos[nBlocos-1].larguraSprite < blocos[i].px || blocos[nBlocos-1].px > blocos[i].px+blocos[i].larguraSprite || blocos[nBlocos-1].py+blocos[nBlocos-1].alturaSprite<blocos[i].py || blocos[nBlocos-1].py> blocos[i].py+blocos[i].alturaSprite) colisao=false;
            else colisao=true;
            if (colisao){
                blocos[nBlocos-1].px+=3*blocos[i].larguraSprite;
                blocos[nBlocos-1].py+=3*blocos[i].alturaSprite;
                i--; //roda denovo pra ver se saiu mesmo kkkkkkkkkkkkkkkkkkkk mt quebra cabeça fazer exatamente
            }
        }
        blocos[nBlocos-1].escalaEntidade = 5;
    }
    else{
        //2 tipo de bloco
        blocos[nBlocos-1].sprite=al_load_bitmap("./bin/entities/Clutter/Barrel.png");
        if(!blocos[nBlocos-1].sprite){
            msgErro("Deu ruim nos blocos!");
            return 0;
        }
        blocos[nBlocos-1].larguraSprite = 1.5*al_get_bitmap_width(blocos[nBlocos-1].sprite);
        blocos[nBlocos-1].alturaSprite = 1.5*al_get_bitmap_height(blocos[nBlocos-1].sprite);
        blocos[nBlocos-1].px=LARGURA/8+rand()%LARGURA; blocos[nBlocos-1].py=ALTURA/9+rand()%ALTURA;
         bool colisao=false;
        for (int i = 0; i < nBlocos-1; i++){
            if(blocos[nBlocos-1].px+blocos[nBlocos-1].larguraSprite < blocos[i].px || blocos[nBlocos-1].px > blocos[i].px+blocos[i].larguraSprite || blocos[nBlocos-1].py+blocos[nBlocos-1].alturaSprite<blocos[i].py || blocos[nBlocos-1].py> blocos[i].py+blocos[i].alturaSprite) colisao=false;
            else colisao=true;
            if (colisao){
                blocos[nBlocos-1].px+=3*blocos[i].larguraSprite;
                blocos[nBlocos-1].py+=3*blocos[i].alturaSprite;
                i--;
            }
        }
        blocos[nBlocos-1].escalaEntidade = 1.5;
    }
    return 1;
}

void geraEntidades(){ //gera as entidades, pode ser preciso usar varios casos
    if(rand()%200==0){ 
        aumentaEntidades();
        if(!initEntidade()) estado=0;

    }
    if(rand()%200==0){
        aumentaBlocos();
        if(!initBloco()) estado=0;

    }
}

void atualizaEntidades(){
    for (int i = 0; i < nEntidades; i++){
        if(entidades[i].naTela){ //desenha entidades escaladas e seus hitboxes
            al_draw_scaled_bitmap(entidades[i].sprite,0,0,
                                entidades[i].larguraSprite/entidades[i].escalaEntidade,entidades[i].alturaSprite/entidades[i].escalaEntidade,
                                entidades[i].px,entidades[i].py,
                                entidades[i].larguraSprite,
                                entidades[i].alturaSprite,0);
            if(mostraHitbox) al_draw_circle(entidades[i].px+entidades[i].larguraSprite/2,entidades[i].alturaSprite/2+entidades[i].py,entidades[i].raio,al_map_rgb(0,0,i),1);
            if(mostraHitbox) al_draw_circle(entidades[i].px+entidades[i].larguraSprite/2,entidades[i].alturaSprite/2+entidades[i].py,entidades[i].escalaEntidade*RAIO_P,al_map_rgb(0,0,i),1);

        }
    }
    for (int i = 0; i < nBlocos; i++){
        al_draw_scaled_bitmap(blocos[i].sprite,0,0,
                                blocos[i].larguraSprite/blocos[i].escalaEntidade,blocos[i].alturaSprite/blocos[i].escalaEntidade,
                                blocos[i].px,blocos[i].py,
                                blocos[i].larguraSprite,
                                blocos[i].alturaSprite,0);
            if(mostraHitbox) al_draw_rectangle(blocos[i].px,blocos[i].py,blocos[i].px+blocos[i].larguraSprite,blocos[i].py+blocos[i].alturaSprite,al_map_rgb(0,0,i),1);
    }
}

void atualizaJogador(bool anda){ //desenha e anima o jogador
    if(anda){ //anima o sprite usando vx, vy, e a estrutura do player (dps eu implemento)

    }
    al_draw_bitmap_region(player.sprite, //muito feio filho
                          player.pDesenhox,player.pDesenhoy,
                          player.larguraSprite,player.alturaSprite,
                          player.px,player.py,0);
    if(mostraHitbox) al_draw_circle(player.px+player.larguraSprite/2,player.py+player.larguraSprite/2,player.raio,al_map_rgb(0,0,0),1);
    
}



//fluxo do jogo
void jogo(){
    al_start_timer(timer);
    //sai do loop do while
    bool sair=false;
    bool teste=false; //modo de teste, frame unico
    bool anda=false; //unica serventia desse bool é pra animação de movimento do personagem
    bool desenhe=false; //desenha o proximo frame
    while (!sair && estado==4){
        ALLEGRO_EVENT evento;
        al_wait_for_event(filaEventos,&evento);
        switch (evento.type){
            case ALLEGRO_EVENT_KEY_DOWN: //aperta uma tecla
                switch(evento.keyboard.keycode){
                    case ALLEGRO_KEY_UP:
                        player.vy-=VELOCIDADE;
                        player.direcao[dCima]=true;
                        anda=true;
                        break;
                    case ALLEGRO_KEY_DOWN:
                        player.vy+=VELOCIDADE;
                        player.direcao[dBaixo]=true;
                        anda=true;
                        break;
                    case ALLEGRO_KEY_RIGHT:
                        player.vx+=VELOCIDADE;
                        player.direcao[dDireita]=true;
                        anda=true;
                        break;
                    case ALLEGRO_KEY_LEFT:
                        player.vx-=VELOCIDADE;
                        player.direcao[dEsquerda]=true;
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
                    case ALLEGRO_KEY_C:
                        if(!mostraHitbox) mostraHitbox=true;
                        else mostraHitbox=false;
                        break;
                    case ALLEGRO_KEY_T: //entra no modo de teste, bom pra desbugar as coisas
                        if(!teste){
                            al_stop_timer(timer);
                            al_flush_event_queue(filaEventos);
                            al_start_timer(timerTeste);
                            teste=true;
                        }              
                        else{
                            al_stop_timer(timerTeste);
                            al_flush_event_queue(filaEventos);
                            al_start_timer(timer);
                            teste=false;
                        }
                    }
                break;
            case ALLEGRO_EVENT_KEY_UP: //solta a tecla
                switch(evento.keyboard.keycode){
                    case ALLEGRO_KEY_UP:
                        if(player.vy!=0)player.vy+=VELOCIDADE;
                        player.direcao[dCima]=false;
                        break;
                    case ALLEGRO_KEY_DOWN:
                        if(player.vy!=0)player.vy-=VELOCIDADE;
                        player.direcao[dBaixo]=false;
                        break;
                    case ALLEGRO_KEY_RIGHT:
                        if(player.vx!=0)player.vx-=VELOCIDADE;
                        player.direcao[dDireita]=false;
                        break;
                    case ALLEGRO_KEY_LEFT:
                        if(player.vx!=0)player.vx+=VELOCIDADE;
                        player.direcao[dEsquerda]=false;
                        break;
                    case ALLEGRO_KEY_Q:
                        escalaVelocidade-= 0.1f;
                        break;
                    case ALLEGRO_KEY_E:
                        escalaVelocidade+= 0.1f;
                        break;
                }
                break;
            case ALLEGRO_EVENT_TIMER:
                desenhe=true;
                
                escala+=escalaVelocidade; //soma a velocidade da escala à escala, analogo ao movimento normal
                if(escala<1.0f) escala=1.0f; //limita o zoom pra n bugar 
                if(escala>5.0f) escala=5.0f; //zoom maximo
        
                geraEntidades();
                player.px+=player.vx;
                player.py+=player.vy;
                if(player.py<0)player.py=0;
                if(player.px<0)player.px=0;
                atualizaCamera(); //aqui começa a magia negra
                colisaoJogador();
                al_identity_transform(&camera);
                al_translate_transform(&camera,-(player.px+player.larguraSprite/2),-(player.py+player.larguraSprite/2)); //basicamente transforma tudo que ta na tela de acordo com esses parametros, eu vou mandar os videos que eu vi ensinando isso pq admito que nem eu entendi direito kkkkkk 
                al_scale_transform(&camera,escala,escala); //esse é o mais simples
                al_translate_transform(&camera,-pxFundo+(player.px+player.larguraSprite/2),-pyFundo+(player.py+player.larguraSprite/2)); 
                al_use_transform(&camera); //simplesmente torna a transformação "canonica"
                if(player.hp<=0) fimDeJogo();
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
            al_draw_textf(retroFont,al_map_rgb(0,0,0),player.px+player.larguraSprite/2,player.py+player.alturaSprite,ALLEGRO_ALIGN_CENTER,"PROTÓTIPO");
            al_flip_display();
            desenhe=0;
        }
    }
}