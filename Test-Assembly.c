#include <stdio.h>
#include <time.h>
#include <math.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#define LARG_IMG 2560
#define ALTU_IMG 1600

struct Pixel {
  unsigned char r, g, b;
};

struct Pixel imagem[ALTU_IMG][LARG_IMG];
struct Pixel imagemSai[ALTU_IMG][LARG_IMG];

char* leLinha(FILE*f) {
  static char str[1000];
  unsigned char c;
  int j=0;

  c = fgetc(f);
  while (c == '#') {
    do {
      c = fgetc(f);
    } while (c != '\n');
    while (isspace(c = fgetc(f)));
  }

  do {
    str[j] = c;
    j++;
  } while (!isspace(c = fgetc(f)));

  str[j] = 0;
  return str;
}

void  lePPm(char nomeArq[], struct Pixel img[ALTU_IMG][LARG_IMG]) {
  int larg, altu, cores, tam;
  char* p;
  FILE* f = fopen(nomeArq, "rb");
  if (!f) {
    printf("O arquivo da imagem não pode ser aberto para leitura.\n");
    exit(1);
  }
  if (strcmp(leLinha(f), "P6")) {
    printf("Não é um arquivo no formato PPM (P6) de imagem.\n");
    exit(1);
  }
  p = leLinha(f);
  larg = atoi(p);
  if (LARG_IMG != larg) {
    printf("A imagem apresenta largura %d, mas o valor desejado e' %d.\n", larg, LARG_IMG);
    exit(1);
  }
  p = leLinha(f);
  altu = atoi(p);
  if (ALTU_IMG != altu) {
    printf("A imagem apresenta altura %d, mas o valor desejado e' %d.\n", altu, ALTU_IMG);
    exit(1);
  };
  p = leLinha(f);
  cores = atoi(p);
  if (cores != 255) {
    printf("A imagem tem um valor máximo de %d, mas a valor de cor é de 255.\n", cores);
    exit(1);
  }
  tam = LARG_IMG * ALTU_IMG;
  if (fread(&img[0][0], 3, tam, f) != tam) {
    printf("O arquivo da imagem parace nao estar completo.\n");
    exit(1);
  }
  fclose(f);
}

void salvaPPM(char nomeArq[], struct Pixel img[ALTU_IMG][LARG_IMG]) {
  int tam;
  FILE *f = fopen(nomeArq, "wb");
  fprintf(f, "P6\n%d %d\n255\n", LARG_IMG, ALTU_IMG);
  tam = LARG_IMG * ALTU_IMG;
  if (fwrite(&img[0][0], 3, tam, f) != tam) {
    printf("O arquivo de imagem nao foi gravado corretamente.\n");
    exit(1);
  }
  fclose(f);
}

void minMax(struct Pixel img[ALTU_IMG][LARG_IMG], int *min, int *max) {
  unsigned char mx[16]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
  unsigned char mn[16]={255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255};
  void *p= &img[0][0];
  int k,l;

  _asm{
    //_atribuindo aos registradores

    mov ecx,4096000/16   ;contador=4096000/16
    mov ebx,p            ;ebx=*img
    movups xmm1,mx[0]    ;xmm1 recebe 16bytes alinhados de mx
    movups xmm2,mn[0]    ;xmm2 recebe 16bytes alinhados de mn

//scan array for min & max values
    loop1:
        movups xmm0,[ebx] ;xmm0=recebe 16bytes alinhados de img
        pmaxub xmm1,xmm0  ;função max ,armazena em xmm1
        pminub xmm2,xmm0  ;função min ,armazena em xmm2
        add ebx,16        ;adiciona ao ponteiro de img mais 16(o que representa a qte de bytes)
        sub ecx,1         ;subtrai o contador
        jnz loop1         ;enquanto nao ligar o flag z ,ele volta

        movups mn[0],xmm2   ;armazena 16bytes em mn
        movups mx[0],xmm1   ;armazena 16bytes em mx

//determine final maximum value
        pshufd xmm0,xmm1,‭00001110b ;desloca =final 16 max valores
        pmaxub xmm1,xmm0           ;função max ,armazena em xmm1
        pshufd xmm0,xmm1,00000001b ;desloca =final 8 max valores
        pmaxub xmm1,xmm0           ;função max ,armazena em xmm1
        pshufd xmm0,xmm1,‭00000001b ;desloca =final 4 max valores
        pmaxub xmm1,xmm0           ;função max ,armazena em xmm1
        pextrw eax,xmm0,0          ;ax=2 max values
        cmp al,ah                  ;max valor
        jae pula
        mov al,ah
    pula:
        mov edx,0                  ;convert dl to edx
        mov dl,al
        mov [k],edx                ;[k]=edx

//determine final minimum value
        pshufd xmm0,xmm2,‭00001110b ;desloca =final 16 min valores
        pminub xmm2,xmm0           ;função min ,armazena em xmm2
        pshufd xmm0,xmm2,‭00000001b ;desloca =final 8 min valores
        pminub xmm2,xmm0           ;função min ,armazena em xmm2
        pshufd xmm0,xmm2,‭00000001b ;desloca =final 4 min valores
        pminub xmm1,xmm0           ;função min ,armazena em xmm2
        pextrw eax,xmm0,0          ;ax=2 min values
        cmp al,ah                  ;min valor
        jae pula2
        mov al,ah
    pula2:
        mov edx,0                  ;convert dl to edx
        mov dl,al
        mov [l],edx                ;[l]=edx

  }

*max=k;
*min=l;

}

void processa(struct Pixel img[ALTU_IMG][LARG_IMG], struct Pixel imgSai[ALTU_IMG][LARG_IMG], int min, int max) {
  int i, j;
  short int f = 255.0 / (max - min) * 1.3 * 32; // ganho 1.3; para evitar o float: * 32 / 32
  min += (max - min) / 100;                     // reduz o nivel do preto
  for (i = 0; i < ALTU_IMG; i++){
    for (j = 0; j < LARG_IMG; j+=4) {
      struct Pixel p = img[i][j];
      struct Pixel q = img[i][j+1];
      struct Pixel k = img[i][j+2];
      struct Pixel m = img[i][j+3];

      short int v;
      v = ((p.r - min) * f) >> 5; // deslocar 5 bits, significa dividir por 32
      if (v > 255) v = 255;
      if (v < 0) v = 0;
      p.r = v;
      v = ((p.g - min)  * f) >> 5;
      if (v > 255) v = 255;
      if (v < 0) v = 0;
      p.g = v;
      v = ((p.b - min)  * f) >> 5;
      if (v > 255) v = 255;
      if (v < 0) v = 0;
      p.b = v;
      imgSai[i][j] = p;

      v = ((q.r - min) * f) >> 5; // deslocar 5 bits, significa dividir por 32
      if (v > 255) v = 255;
      if (v < 0) v = 0;
      q.r = v;
      v = ((q.g - min)  * f) >> 5;
      if (v > 255) v = 255;
      if (v < 0) v = 0;
      q.g = v;
      v = ((q.b - min)  * f) >> 5;
      if (v > 255) v = 255;
      if (v < 0) v = 0;
      q.b = v;
      imgSai[i][j+1] = q;

      v = ((k.r - min) * f) >> 5; // deslocar 5 bits, significa dividir por 32
      if (v > 255) v = 255;
      if (v < 0) v = 0;
      k.r = v;
      v = ((k.g - min)  * f) >> 5;
      if (v > 255) v = 255;
      if (v < 0) v = 0;
      k.g = v;
      v = ((k.b - min)  * f) >> 5;
      if (v > 255) v = 255;
      if (v < 0) v = 0;
      k.b = v;
      imgSai[i][j+2] = k;

      v = ((m.r - min) * f) >> 5; // deslocar 5 bits, significa dividir por 32
      if (v > 255) v = 255;
      if (v < 0) v = 0;
      m.r = v;
      v = ((m.g - min)  * f) >> 5;
      if (v > 255) v = 255;
      if (v < 0) v = 0;
      m.g = v;
      v = ((m.b - min)  * f) >> 5;
      if (v > 255) v = 255;
      if (v < 0) v = 0;
      m.b = v;
      imgSai[i][j+3] = m;
    }
  }
}

int main() {
  int j, min, max;
  clock_t inicio, final;
  double duracao;

  lePPm("LapisFraco.ppm", imagem);
  printf("Iniciando processamento:\n");
  inicio=clock();
  for (j=0; j<3; j++) {
    minMax(imagem, &min, &max);
    processa(imagem, imagemSai, min, max);
  }
  final=clock();
  duracao = (double) (final - inicio) / CLOCKS_PER_SEC;
  printf("Tempo utilizado no processamento = %f\n", duracao);
  salvaPPM("LapisSai.ppm", imagemSai);
  return 0;
}
