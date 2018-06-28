#include <GL/freeglut.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include "glm.h"
///Carregando obj:
GLMmodel* iguana;
GLuint iguanalist;
void inicializaIguana(){
    iguana = glmReadOBJ("iguana.obj");
    iguanalist = glmList(iguana, GLM_SMOOTH);
}
///ESTRUTURAS:

typedef struct vetor{
    double x,y,z;
} Vetor;

typedef struct arvore{
    Vetor posicao;
    int altura;
    int tipo;
    Vetor rotacao;
    Vetor cor;
    struct arvore *proximo;
    struct arvore *anterior;

} Arvore;

typedef struct listaEncadeadaArvores{
    struct arvore *inicial;
    struct arvore *ultima;
    int tamanho;
} ListaEncadeadaArvores;


 ///Variáveis Globais:

int keys[256]; /// Vetor de teclas, 1 para tecla pressionada e 0 para tecla solta
int tAnterior=0, tAtual;


double eyeX=0, eyeY=5, eyeZ=1, centerX=0, centerY=1, centerZ=20, upX=0, upY=1, upZ=0.2; ///Variáveis do look at
int slices = 32;
int stacks = 32;

double velocidadeZ = 0.07, aceleracaoZ = 0.000002, velocidadeX = 0, rotateZ = 0, rotCount = 1.5, velRotCount = 0.005, C=0;
double porDoSol = 0, subindo = 0;

Vetor origem = {0, 0, 0};
Vetor vi = {1, 0, 0};
Vetor vj = {0, 1, 0};
Vetor vk = {0, 0, 1};
Vetor posicaoAtual = {0, 0, 0};
Vetor corDoCeu = {0.3,0.5,1};
Vetor rotArvore = {1, 0, 0};

ListaEncadeadaArvores arvores;


void desenhaEsfera(Vetor posicao, double r, double slices, double stacks){
glPushMatrix();
        glTranslated(posicao.x,posicao.y,posicao.z);
        glRotated(60,1,0,0);
        glutSolidSphere(r,slices,stacks);
    glPopMatrix();
}

void desenhaCilindro(Vetor posicao, double r, double h, double slices, double stacks){
    glPushMatrix();
        glTranslated(posicao.x,posicao.y,posicao.z);
        glRotated(90,1,0,0);
        glutSolidCylinder(r,h,slices,stacks);
    glPopMatrix();
}

void desenhaSuperficie(Vetor o, Vetor t){
    glPushMatrix();
    glTranslated(o.x, o.y, o.z);
    glBegin(GL_TRIANGLE_FAN);
            glVertex3f(-t.x/2,  0, -t.z/2);
            glVertex3f(-t.x/2, 0, t.z/2);
            glVertex3f(t.x/2,  0, t.z/2);
            glVertex3f(t.x/2,  0, -t.z/2);
        glEnd();
    glPopMatrix();
}

void desenhaMontanhas(){
    glPushMatrix();
    glTranslated(centerX+150, -10, centerZ+600);
    glBegin(GL_TRIANGLE_FAN);
        glColor3f(0.3+corDoCeu.x/5, 0.3+corDoCeu.y/5, 0.3+corDoCeu.z/5);
        glVertex3f(0, 0, 0);
        glColor3f(2, 2, 2);
        glVertex3f(25, 50, 0);
        glColor3f(0.3+corDoCeu.x/5, 0.3+corDoCeu.y/5, 0.3+corDoCeu.z/5);
        glVertex3f(50, 0, 0);
    glEnd();
    glBegin(GL_TRIANGLE_FAN);
        glColor3f(0.3+corDoCeu.x/5, 0.3+corDoCeu.y/5, 0.3+corDoCeu.z/5);
        glVertex3f(30, 0, 0);
        glColor3f(2, 2, 2);
        glVertex3f(60, 30, 0);
        glColor3f(0.3+corDoCeu.x/5, 0.3+corDoCeu.y/5, 0.3+corDoCeu.z/5);
        glVertex3f(90, 0, 0);
    glEnd();
    glBegin(GL_TRIANGLE_FAN);
        glColor3f(0.3+corDoCeu.x/5, 0.3+corDoCeu.y/5, 0.3+corDoCeu.z/5);
        glVertex3f(-80, 0, 0);
        glColor3f(2, 2, 2);
        glVertex3f(-45, 80, 0);
        glColor3f(0.3+corDoCeu.x/5, 0.3+corDoCeu.y/5, 0.3+corDoCeu.z/5);
        glVertex3f(-10, 0, 0);
    glEnd();
    glBegin(GL_TRIANGLE_FAN);
        glColor3f(0.3+corDoCeu.x/5, 0.3+corDoCeu.y/5, 0.3+corDoCeu.z/5);
        glVertex3f(-30, 0, 0);
        glColor3f(2, 2, 2);
        glVertex3f(5, 40, 0);
        glColor3f(0.3+corDoCeu.x/5, 0.3+corDoCeu.y/5, 0.3+corDoCeu.z/5);
        glVertex3f(40, 0, 0);
    glEnd();
    glPopMatrix();

}

Arvore* criaArvore(Vetor posicao, int altura, int tipo, Vetor rotacao, Vetor cor){
    Arvore *nova = (Arvore*) malloc(sizeof(Arvore));
    nova->posicao = posicao;
    nova->rotacao = rotacao;
    nova->cor = cor;
    nova->tipo = tipo;
    nova->altura = altura;
    nova->proximo = NULL;
    return nova;
}

void *adicionaArvore(ListaEncadeadaArvores *lista, Arvore* nova){

    if (lista->tamanho == 0){
    	lista->inicial = nova;
    } else {
    	lista->ultima->proximo = nova;
    }
    lista->ultima = nova; // atualiza atalho para ultima arvore da lista

    lista->tamanho++;
}


int colidiu(Arvore *arvore, double centerX, double centerZ){
    if(   centerX > arvore->posicao.x-(arvore->altura-1)/3-1
       && centerX < arvore->posicao.x+(arvore->altura-1)/3+1
       && centerZ > arvore->posicao.z-(arvore->altura-1)/3-1
       && centerZ < arvore->posicao.z+(arvore->altura-1)/3+1){
        return 1;
    }
    return 0;
}

void inicializa(void){
    srand(time(NULL));

   inicializaIguana();

    int i, posicaoXArvore, posicaoZArvore;
    arvores.tamanho = 0;
    Vetor posicaoArvore;
    Vetor rotacaoArvore;
    Vetor cor;

    Arvore * arvore;
    for(i=0; i<300; i++){
        posicaoXArvore=rand()%500-250;
        posicaoZArvore=rand()%600+120;

        posicaoArvore.x = posicaoXArvore;
        posicaoArvore.y = 0;
        posicaoArvore.z = posicaoZArvore;

        rotacaoArvore.x = rand()%30-15;
        rotacaoArvore.y = rand()%360-180;
        rotacaoArvore.z = rand()%30-15;

        cor.x = (rand()%100-50)/300.0;
        cor.y = (rand()%100-50)/600.0;
        cor.z = (rand()%100-50)/1000.0;

        arvore = criaArvore(posicaoArvore, rand()%8+7, rand()%2+2,rotacaoArvore, cor );
        adicionaArvore(&arvores, arvore);
    }
}

void detectaColisoes(){
    Arvore *iterador = arvores.inicial;
    while (iterador != NULL){
        if(colidiu(iterador, centerX, centerZ)==1){
            eyeX=0, eyeY=5, eyeZ=1, centerX=0, centerY=1, centerZ=20, upX=0, upY=1, upZ=0.2;
	    rotateZ = 0;
            velocidadeZ = 0.07;
	    velocidadeX = 0;
            inicializa();
            printf("Faliceu\n");
        }
        iterador = iterador->proximo;
    }
}

void removeArvore(Arvore *arvoreAnterior, Arvore* arvoreRemover){
    arvoreAnterior->proximo = arvoreRemover->proximo;
    free (arvoreRemover);
    arvores.tamanho--;
}

void encontraArvoresPerdidas(){
    int posicaoXArvore, posicaoZArvore;
    Vetor posicaoArvore;
    Vetor rotacaoArvore;
    Vetor cor;
    Arvore *anterior = arvores.inicial;
    Arvore *iterador = anterior->proximo;


    Arvore *arvore;
    while (iterador != NULL){
        if(iterador->posicao.z<centerZ-20 || iterador->posicao.x>centerX+300 || iterador->posicao.x<centerX-300){
                posicaoXArvore=rand()%550-275+centerX;
                posicaoZArvore=rand()%250+300+centerZ;

                posicaoArvore.x = posicaoXArvore;
                posicaoArvore.y = 0;
                posicaoArvore.z = posicaoZArvore;

                rotacaoArvore.x = rand()%15-100;
                rotacaoArvore.y = rand()%360-180;
                rotacaoArvore.z = rand()%30-15;

                cor.x = (rand()%100-50)/300.0;
                cor.y = (rand()%100-50)/600.0;
                cor.z = (rand()%100-50)/1000.0;

                arvore = criaArvore(posicaoArvore, rand()%8+7, rand()%3+2,rotacaoArvore, cor);
                adicionaArvore(&arvores, arvore);
            removeArvore(anterior,iterador);
        } else {
           anterior = iterador;
        }
        iterador = anterior->proximo;
    }
}

void desenhaArvore(Arvore *arvore){
    glColor3f(0.4, 0.2, 0);
    double altura = arvore->altura;
    glPushMatrix();
    glTranslated(arvore->posicao.x, arvore->posicao.y, arvore->posicao.z);
    glRotated(15,arvore->rotacao.x,arvore->rotacao.y,arvore->rotacao.z);
    desenhaCilindro(vj, altura/10, 3, slices, stacks);
    int i, tipo;
    tipo = arvore->tipo;
    for(i=1; i<arvore->altura; i++){
    
    glRotated(rotCount,rotArvore.x + arvore->rotacao.x/20,0,rotArvore.z + arvore->rotacao.z/20);

    glColor3f(0+i/(altura+1)+arvore->cor.x+corDoCeu.x/5, 0.3+i/(altura+1)+arvore->cor.y+corDoCeu.y/5, 0.05+i/(altura+1)+arvore->cor.z+corDoCeu.z/5);
    glBegin(GL_TRIANGLE_FAN); //base vista de baixo
        glVertex3f(-(altura-i)/3,  i, -(altura-i)/3);
        glVertex3f((altura-i)/3,  i, -(altura-i)/3);
        glVertex3f((altura-i)/3,  i, (altura-i)/3);
        glVertex3f(-(altura-i)/3,  i, (altura-i)/3);
    glEnd();
    glColor3f(0.08+i/(altura+1)+arvore->cor.x+corDoCeu.x/5, 0.5+i/(altura+1)+arvore->cor.y+corDoCeu.y/5, 0.08+i/(altura+1)+arvore->cor.z+corDoCeu.z/5);
    glBegin(GL_TRIANGLE_FAN); //base vista de cima
        glVertex3f(-(altura-i+1)*tipo/10,  i, -(altura-i+1)*tipo/10);
        glVertex3f(-(altura-i+1)*tipo/10,  i, (altura-i+1)*tipo/10);
        glVertex3f((altura-i+1)*tipo/10,  i, (altura-i+1)*tipo/10);
        glVertex3f((altura-i+1)*tipo/10,  i, -(altura-i+1)*tipo/10);
    glEnd();
    glColor3f(0.05+i/(altura+1)+arvore->cor.x+corDoCeu.x/5, 0.4+i/(altura+1)+arvore->cor.y+corDoCeu.y/5, 0.075+i/(altura+1)+arvore->cor.z+corDoCeu.z/5);
    glBegin(GL_TRIANGLE_FAN); //lado da frente
        glVertex3f(-(altura-i)/3,  i, -(altura-i)/3);
        glVertex3f(-(altura-i)*tipo/10,  i+1, -(altura-i)*tipo/10);
        glVertex3f((altura-i)*tipo/10,  i+1, -(altura-i)*tipo/10);
        glVertex3f((altura-i)/3,  i, -(altura-i)/3);
    glEnd();
    glColor3f(0.1+i/(altura+1)+arvore->cor.x+corDoCeu.x/5, 0.8+i/(altura+1)+arvore->cor.y+corDoCeu.y/5, 0.15+i/(altura+1)+arvore->cor.z+corDoCeu.z/5);
    glBegin(GL_TRIANGLE_FAN); //ladro de tras
        glVertex3f(-(altura-i)/3,  i, (altura-i)/3);
        glVertex3f(-(altura-i)*tipo/10,  i+1, (altura-i)*tipo/10);
        glVertex3f((altura-i)*tipo/10,  i+1, (altura-i)*tipo/10);
        glVertex3f((altura-i)/3,  i, (altura-i)/3);
    glEnd();
    glColor3f(0.04+i/(altura+1)+arvore->cor.x+corDoCeu.x/5, 0.3+i/(altura+1)+arvore->cor.y+corDoCeu.y/5, 0.06+i/(altura+1)+arvore->cor.z+corDoCeu.z/5);
    glBegin(GL_TRIANGLE_FAN); //direita
        glVertex3f(-(altura-i)/3,  i, -(altura-i)/3);
        glVertex3f(-(altura-i)/3,  i, (altura-i)/3);
        glVertex3f(-(altura-i)*tipo/10,  i+1, (altura-i)*tipo/10);
        glVertex3f(-(altura-i)*tipo/10,  i+1, -(altura-i)*tipo/10);
    glEnd();
    glColor3f(0.06+i/(altura+1)+arvore->cor.x+corDoCeu.x/5, 0.45+i/(altura+1)+arvore->cor.y+corDoCeu.y/5, 0.08+i/(altura+1)+arvore->cor.z+corDoCeu.z/5);
    glBegin(GL_TRIANGLE_FAN); //esquerda
        glVertex3f((altura-i)/3,  i, -(altura-i)/3);
        glVertex3f((altura-i)*tipo/10,  i+1, -(altura-i)*tipo/10);
        glVertex3f((altura-i)*tipo/10,  i+1, (altura-i)*tipo/10);
        glVertex3f((altura-i)/3,  i, (altura-i)/3);
    glEnd();
    }
    glPopMatrix();
}

void desenhaIguana(){
    glPushMatrix();
    glTranslatef(centerX, centerY, centerZ);
    glRotated(-5, 1, 0, 0);
    glRotated(rotateZ, 0, 1, -0.3);
    glScalef(0.3, 0.3, 0.3);
    glColor3f(0.3, 0.3, 0.9);
    glCallList(iguanalist);
    glPopMatrix();
}

void desenhaCena(void){
    glClearColor(corDoCeu.x,corDoCeu.y,corDoCeu.z, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glColor3d(0.6+corDoCeu.x, 0.6+corDoCeu.y, 0.7+corDoCeu.z);

    Vetor tamanhoPlano = {1500, 0, 1500};
    desenhaSuperficie(posicaoAtual, tamanhoPlano);

    desenhaIguana();

    Arvore *iterador = arvores.inicial;
    while (iterador != NULL){
        desenhaArvore(iterador);
        iterador = iterador->proximo;
    }

    glColor3d(1-corDoCeu.x,1-corDoCeu.y,1-corDoCeu.z);
    Vetor posicaoEsfera = {centerX, centerY, centerZ};
    //desenhaEsfera(posicaoEsfera, 1, 16, 16);

    glColor3d(1,0.7,0);
    posicaoEsfera.x = centerX+300;
    posicaoEsfera.z = centerZ+700;
    posicaoEsfera.y = centerY+100-porDoSol;
        if(porDoSol<140 && porDoSol>-75){ //Sol se pondo
            if(corDoCeu.x<0.5)
                corDoCeu.x +=0.02/10;
            if(corDoCeu.y>0.3)
                corDoCeu.y -=0.006/10;
            if (corDoCeu.z>0)
                corDoCeu.z -=0.02/10;
        }
        else if(porDoSol>140){ //Sol ao longo do dia
            if (corDoCeu.x>0)
                corDoCeu.x -=0.007/10;
            if (corDoCeu.y>0)
                corDoCeu.y -=0.005/10;
            if (corDoCeu.z>0)
                corDoCeu.z -=0.003/10;
        }
        else if(porDoSol<-100){ //Sol posto
            if (corDoCeu.x<0.2)
                corDoCeu.x += 0.004/10;
            if (corDoCeu.x<0.4)
                corDoCeu.y += 0.003/10;
            if (corDoCeu.x<0.8)
                corDoCeu.z += 0.01/10;
        }
        if(corDoCeu.x<=0 && corDoCeu.y<=0 && corDoCeu.z<=0 && porDoSol>200){
            porDoSol=-230;
        }
    desenhaEsfera(posicaoEsfera, 120, 32, 32);
    desenhaMontanhas();


    glutSwapBuffers();
}

void posiciona(){
    posicaoAtual.x = centerX;
    posicaoAtual.y = centerY-1;
    posicaoAtual.z = centerZ;
    centerZ+=velocidadeZ*(tAtual-tAnterior);
    eyeZ+=velocidadeZ*(tAtual-tAnterior);
    centerX+=velocidadeX*(tAtual-tAnterior);
    eyeX+=velocidadeX*(tAtual-tAnterior);
    velocidadeZ+=aceleracaoZ*(tAtual-tAnterior);
    porDoSol += velocidadeZ*(tAtual-tAnterior)/20;
    rotCount += velRotCount*(tAtual-tAnterior)/50;
    if(rotCount>3 || rotCount<1){velRotCount=-velRotCount;}
    C++;
    rotArvore.x=cos(C/20)*(tAtual-tAnterior);
    rotArvore.z=sin(C/20)*(tAtual-tAnterior);
}

void comandos(){
    if(keys['d']==1 || keys['D']==1){
	if(velocidadeX > -velocidadeZ*2/5){
        	velocidadeX-=velocidadeZ/4;
	}
	if(rotateZ>-20){
		rotateZ-=3;
	}
    }
    else if(velocidadeX<0){
	if(rotateZ<0){
		rotateZ+=3;
	}
    }
    if(keys['A']==1 || keys['a']==1){
	if(velocidadeX < velocidadeZ*2/5){
        	velocidadeX+=velocidadeZ/4;
	}
	if(rotateZ<20){
		rotateZ+=3;
	}
    }
    else if(velocidadeX>0){
	if(rotateZ>0){
		rotateZ-=3;
	}
    }
    if(keys['d']==0 && keys['D']==0 && keys['A']==0 && keys['a']==0){
    	if(velocidadeX>0){
		velocidadeX-=velocidadeZ/20;
	}
	if(velocidadeX<0){
		velocidadeX+=velocidadeZ/20;
	}
    }
}

void atualiza(int idx){

    tAtual = glutGet(GLUT_ELAPSED_TIME);
    posiciona();
    comandos();
    tAnterior = glutGet(GLUT_ELAPSED_TIME);
    glLoadIdentity();

    detectaColisoes();
    encontraArvoresPerdidas();
    gluLookAt(eyeX,eyeY,eyeZ,centerX,centerY,centerZ,upX,upY,upZ);

    glPopMatrix();
    glutPostRedisplay();
    glutTimerFunc(17, atualiza, 0);
}

void resize(int width, int height){
    float razaoaspecto = (float) width / (float) height;

    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-razaoaspecto, razaoaspecto, -1.0, 1.0, 3.5, 1000.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity() ;
}

void pressiona(unsigned char key,int x, int y){
    keys[key]=1;
    switch(key){
        case 27:
            exit(0);
            break;
    }
}

void solta(unsigned char key,int x, int y){
    keys[key]=0;
}


float light_ambient[]  = { 0.5f, 0.5f, 0.5f, 1.0f };
float light_diffuse[]  = { 1.0f, 1.0f, 1.0f, 1.0f };
float light_specular[] = { 0.5f, 0.5f, 0.5f, 1.0f };
float light_position[] = { 10.0f, 50.0f, 50.0f, 1.0f };

float mat_ambient[]    = { 0.7f, 0.7f, 0.7f, 1.0f };
float mat_diffuse[]    = { 0.8f, 0.8f, 0.8f, 1.0f };
float mat_specular[]   = { 1.0f, 1.0f, 1.0f, 1.0f };
float high_shininess[] = { 100.0f };

int main(int argc, char *argv[]){
    glutInit(&argc, argv);
    glutInitWindowSize(1280,720);
    glutInitWindowPosition(10,10);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);

    glutCreateWindow("Almocreve");

    glutReshapeFunc(resize);
    glutDisplayFunc(desenhaCena);
    glutKeyboardFunc(pressiona);
    glutKeyboardUpFunc(solta);
    glutTimerFunc(0, atualiza, 0);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glEnable(GL_LIGHT0);
    glEnable(GL_NORMALIZE);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_LIGHTING);
    glShadeModel(GL_SMOOTH);

    glLightfv(GL_LIGHT0, GL_AMBIENT,  light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE,  light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);

    glMaterialfv(GL_FRONT, GL_AMBIENT,   mat_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE,   mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR,  mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, high_shininess);
    inicializa();
    glutMainLoop();

    return 0;
}
