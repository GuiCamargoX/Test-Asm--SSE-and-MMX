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
  unsigned int i, j;
  *min = 255;
  *max = 0;
  for (i = 0; i < ALTU_IMG; i++) {//2-trocar linha por coluna
    for (j = 0; j < LARG_IMG; j++) {
      struct Pixel p = img[i][j];
      if (p.r > *max)
        *max = p.r;
      if (p.g > *max)
        *max = p.g;
      if (p.b > *max)
        *max = p.b;
      if (p.r < *min)
        *min = p.r;
      if (p.g < *min)
        *min = p.g;
      if (p.b < *min)
        *min = p.b;
    }
  }
}

void processa(struct Pixel img[ALTU_IMG][LARG_IMG], struct Pixel imgSai[ALTU_IMG][LARG_IMG], int min, int max) {
  int i, j;
  short int f = 255.0 / (max - min) * 1.3 * 32; // ganho 1.3; para evitar o float: * 32 / 32
  min += (max - min) / 100;                     // reduz o nivel do preto
  for (i = 0; i < ALTU_IMG; i++) {
    for (j = 0; j < LARG_IMG; j++ ) {//2-trocar linha por coluna
      struct Pixel p = img[i][j];
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

  for (j=0; j<3; j++) {//1-nao necessita fazer tres vezes
    minMax(imagem, &min, &max);
    printf("%d %d",min,max);
    processa(imagem, imagemSai, min, max);
}

  final=clock();
  duracao = (double) (final - inicio) / CLOCKS_PER_SEC;
  printf("Tempo utilizado no processamento = %f\n", duracao);
  salvaPPM("LapisSai.ppm", imagemSai);
  return 0;
}
