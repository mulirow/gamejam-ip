#ifndef LIB_H
#define LIB_H

#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_video.h>

#define FPS 60.0

enum ESTADO{estSaida, estPreMenu, estMenu, estPreJogo, estCutscene, estTutorial, estJogo, estFinal}; //Organizar num enum porque esse monte de define ficaria feio
enum DIRECOES{dBaixo,dCima,dDireita,dEsquerda}; //esse enum é pra nao ter que ficar lembrando que numero é cada posição no vetor do struct de entidades

//velocidade das entidades no geral
extern const float VELOCIDADE;
//vida das entidades no geral
extern const float HP;
//ataque das entidades no geral
extern const float ATAQUE;
//defesa da entidades no geral
extern const float DEFESA;
//

//raio de procura do player
extern const float RAIO_P;
extern bool obj1; extern bool obj2; extern bool obj3;
extern bool comecou;
extern int completou1;
extern int completou2;
extern int completou3;
extern int numObj1;
extern int numObj2;
extern int numObj3;
extern int limObj1;
extern int limObj2;
extern bool *acertou;

extern int LARGURA; extern int ALTURA;

//diz respeito à opção no menu
extern int opcao;

//diz respeito ao fluxo do jogo, com -1 sendo a saída
extern int estado;
//o numero de entidades e o numero de blocos
extern int nEntidades; extern int nBlocos; 
extern int nBalas;
extern float limEntidades; //total de fases
extern int nPassivos;
extern bool mostrarCaixa;
extern int bCaixa;
extern float dificuldade;
//( ͡° ͜ʖ ͡°)
extern bool sexo;
extern bool atirando;

//altura e largura de cada tile, pode ser redundante
extern int largFase; extern int altFase;
//usado no desenho do bitmap do fundo e na camera
extern float pxFundo; extern float pyFundo;
//escala da camera e velocidade com que ela escala
extern float escala; extern float escalaVelocidade;
extern int contaFrames;

extern ALLEGRO_DISPLAY *janela; //janela de saida padrao
extern ALLEGRO_EVENT_QUEUE *filaEventos; //fila de eventos padrao
extern ALLEGRO_TIMER *timer; //timer padrao
extern ALLEGRO_TIMER *timerAlt; //timer que roda a 7.5 fps
extern ALLEGRO_FONT *retroFont; //fonte padrao (tamanho 20)
extern ALLEGRO_FONT *retroFont32; //fonte tamanho 32
extern ALLEGRO_BITMAP *fundo; //fundo do jogo
extern ALLEGRO_BITMAP *caixaDialogo;
extern ALLEGRO_BITMAP *barracaPastel;
extern ALLEGRO_BITMAP *solnoraboPastel;
extern ALLEGRO_BITMAP *capuz;
extern ALLEGRO_BITMAP *salmao;
extern ALLEGRO_BITMAP *lula;
extern ALLEGRO_TRANSFORM camera; //usado pra movimentar a camera
extern ALLEGRO_VIDEO *marinha;
extern ALLEGRO_VOICE *voice;
extern ALLEGRO_MIXER *mixer;
extern ALLEGRO_SAMPLE *blip;
extern ALLEGRO_SAMPLE *lancha;
extern ALLEGRO_SAMPLE *objetivo;
extern ALLEGRO_SAMPLE *passos;
extern ALLEGRO_SAMPLE_ID passosJ;
extern ALLEGRO_SAMPLE_ID passosE;
extern ALLEGRO_SAMPLE *tiro;
extern ALLEGRO_AUDIO_STREAM *loopMenu;
extern ALLEGRO_AUDIO_STREAM *creditos;
extern ALLEGRO_AUDIO_STREAM *audiojogo;

//estrutura geral das entidades do jogo, talvez seja alterado
typedef struct{
    float vx; float vy; //velocidades
    float px; float py; //posiçoes
    float hp; //HP total
    float atk; //dano de ataque
    float def; //defesa
    ALLEGRO_BITMAP *sprite; //sprite da entidade
    //auto-explicativos
    float alturaSprite; float larguraSprite;
    //raio de contato
    float raio;
    //pulo de linha pra uma direcao pra outra e pulo de coluna de um sprite pro outro
    int puloLinha; int puloColuna;
    //quantos frames tem a animação do sprite
    int totalFrames; int frameAtual;
    //talvez seja reduntante, mas pra deixar mais legivel, guarda em que linha cada sprite de direção se encontra
    int nDirecao[4];
    //posiçao x e y do desenho do sprime no bitmap(bmp) original
    int pDesenhox; int pDesenhoy;
    //aqui sabemos se a entidade está ou não na tela
    bool naTela;
    //se for inimigo = true
    bool inimigo;
    // escala do bitmap
    float escalaEntidade;
    //bool que diz em qual (ou quais) direções o player se move
    bool direcao[4];
    //se esta tomando dano ou não
    bool dano;
    bool boss;
} Entidade;

typedef struct{
    float vx; float vy; //velocidades
    float px; float py; //posiçoes
    ALLEGRO_BITMAP *sprite; //sprite da entidade
    //auto-explicativos
    float alturaSprite; float larguraSprite;
    //raio de contato
    float raio;
    //aqui sabemos se a entidade está ou não na tela
    bool atingiu;

    // escala do bitmap
    float escalaEntidade;
    //bool que diz em qual (ou quais) direções o player se move
    bool direcao[4];
} Balas;

typedef struct{
    //analogos à estrutura acima
    float px; float py;
    ALLEGRO_BITMAP *sprite;
    int alturaSprite; int larguraSprite;
    //altura e largura de onde acontece a colisão
    int alturaHitbox; int larguraHitbox;
    bool naTela;
    // escala do bitmap
    float escalaEntidade;
    bool interac;

} Bloco; //um tipo de entidade que não se move

extern Entidade player;
extern Balas balasPlayer;
extern Entidade *entidades;
extern Balas *balasEntidades;
extern Bloco *blocos;
extern bool criaEnt; // aqui é um boleano que responde se deve ou nao ser criada uma entidade ou um bloco
extern bool mostraHitbox; //boleano que diz se deve ou nao demonstrar as hitboxes
extern bool reinicio; //boleano que pergunta se o jogo esta reiniciando
extern bool interac;

void destroi();
int min();
int max();

//Auxiliar para erros
void msgErro(char *t);

//Funções de inicialização
int initEntidade();
int initBloco();
int initBalas();
int inic();
void geraMundo();
int cria();

//Funções de atualização
void UI();
void UIent(int i);
void geraEntidades();
void aumentaEntidades();
void aumentaBlocos();
void atualizaCamera();
void desenhaMundo();
void jogadorAtaque();
void colisaoBalasP();
void colisaoBalasE();
void colisaoEntidades(int i);
void caixaTexto(int j);
void colisaoJogador();
void atualizaBalas();
void atualizaJogador();
void atualizaEntidades();

//Funções dos estados
void preMenu();
void menu();
void preJogo();
void cutscene();
void tutorial();
void pauseJogo();
void fimDeJogo();
void jogo();
void final();

#endif
