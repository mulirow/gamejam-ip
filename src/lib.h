#ifndef LIB_H
#define LIB_H

#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>

#define FPS 60.0

enum ESTADO{estSaida, estPreMenu, estMenu, estPreJogo, estJogo}; //Organizar num enum porque esse monte de define ficaria feio
enum DIRECOES{dBaixo,dCima,dDireita,dEsquerda}; //esse enum é pra nao ter que ficar lembrando que numero é cada posição no vetor do struct de entidades

//velocidade das entidades no geral
extern const float VELOCIDADE;
//vida das entidades no geral
extern const float HP;
//ataque das entidades no geral
extern const float ATAQUE;
//defesa da entidades no geral
extern const float DEFESA;
//total de fases no jogo
extern const int FASES;

extern int LARGURA; extern int ALTURA;
//as razões entre o tamanho dos tiles e a altura da tela, essencial pra desenhar na tela
extern int RAZAO_X; extern int RAZAO_Y;
//diz respeito à opção no menu
extern int opcao;
//numero de entidades totais;
//diz respeito ao fluxo do jogo, com -1 sendo a saída
extern int estado;
extern int nEntidades;
//( ͡° ͜ʖ ͡°)
extern bool sexo;
//fase atual
extern int faseAtual;
//altura e largura de cada tile, pode ser redundante
extern int largTile[3]; extern int altTile[3];
//usado no desenho do bitmap do fundo e na camera
extern float pxFundo; extern float pyFundo;
//escala da camera e velocidade com que ela escala
extern float escala; extern float escalaVelocidade;

extern ALLEGRO_DISPLAY *janela; //janela de saida padrao
extern ALLEGRO_EVENT_QUEUE *filaEventos; //fila de eventos padrao
extern ALLEGRO_TIMER *timer; //timer padrao
extern ALLEGRO_TIMER *timer15; //timer que roda a 15 fps
extern ALLEGRO_FONT *retroFont; //fonte padrao (tamanho 20)
extern ALLEGRO_FONT *retroFont32; //fonte tamanho 32
extern ALLEGRO_BITMAP *fundo[3]; //fundo do jogo
extern ALLEGRO_TRANSFORM camera; //usado pra movimentar a camera

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

extern Entidade player;
extern Entidade *entidades;

void destroi();

//Auxiliar para erros
void msgErro(char *t);

//Funções de inicialização
int inic();
void geraMundo(int i);
int cria();

//Funções de atualização
void aumentaEntidades();
void atualizaCamera();
void desenhaMundo();
void colisaoJogador();
void atualizaJogador(bool anda);
void atualizaEntidades();

//Funções dos estados
void preMenu();
void menu();
void preJogo();
void pauseJogo();
void jogo();

#endif
