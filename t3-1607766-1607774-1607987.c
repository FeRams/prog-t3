/*
	Por: 	Felipe Negrelli Wolter		1607766
		Fernando H. Ratusznei Caetano	1607774
		Pedro Vicente Alves Sokulski	1607987

*/

#include "trabalho3.h"
#include <stdlib.h>
#include <time.h>

LUT* criaLUT (Imagem3C* img, int bpp) {
	LUT *lut = (LUT *) malloc(sizeof(LUT));

	int n = 16;

	int i;

	lut->cores = (unsigned char **) malloc(n * sizeof(LUT));

	for (i = 0; i < n; i++) {
		lut->cores[i] = (unsigned char *) malloc(3 * sizeof(char));
	}	

	srand(clock());

	for (i = 0; i < n; i++) {
		lut->cores[i][0] = rand() % 256;
		lut->cores[i][1] = rand() % 256;
		lut->cores[i][2] = rand() % 256;
	}

	lut->n_pos = n;
	lut->bpp = bpp;

	return lut;
}

void destroiLUT (LUT* lut) {
	int i;

	for (i = 0; i < lut->n_pos; i++) {
		free(lut->cores[i]);
	}

	free(lut->cores);
	free(lut);
}

int distanciaQuadradaCor (unsigned char *cor1, unsigned char *cor2) {
	return 	(cor1[0] - cor2[0]) * (cor1[0] - cor2[0]) +
		(cor1[1] - cor2[1]) * (cor1[1] - cor2[1]) +
		(cor1[2] - cor2[2]) * (cor1[2] - cor2[2]);
}

int maisProximoNaLUT (LUT *lut, unsigned char *cor) {
	int i;
	
	int cor_mais_proxima = 0;
	int min_dist = distanciaQuadradaCor(lut->cores[0], cor);

	for (i = 1; i < lut->n_pos; i++) {
		int dist = distanciaQuadradaCor(lut->cores[i], cor);

		if (dist < min_dist) {
			cor_mais_proxima = i;
			min_dist = dist;
		}
	}

	return cor_mais_proxima;
}

void salvaCorDaLUT (unsigned char *buffer, int i_lut, int bpp, int pos) {
	int byte_pos = pos / 8;
	int bit_pos = pos % 8;

	int i;

	buffer[0] = 0;

	for (i = 0; i < 3; i++) {
		buffer[byte_pos] |= (i_lut << (8 - bpp)) >> bit_pos;

		bit_pos += bpp;
		if (bit_pos >= 8) {
			bit_pos -= 8;
			byte_pos++;
			buffer[byte_pos] = 0;
		}
	}
}

unsigned char* codificaImagem (Imagem3C* img, LUT* lut, int* n_bytes) {
	int i;
	int j;

	/* bits por cor * (Tamanho * canais / bits por cor padrao) */
	int n_pos = lut->bpp * (img->altura * img->largura * 3 / 24);
	unsigned char *bytes =
		(unsigned char *) malloc(n_pos * sizeof(unsigned char));


	int i_bytes = 0;
	int i_lut;

	/* Para cada pixel, acha cor mais proxima e salva na imagem nova */
	for (i = 0; i < img->altura; i++) {
		for (j = 0; j < img->largura; j++) {
			unsigned char cor[] = {
				img->dados[0][i][j],
				img->dados[1][i][j],
				img->dados[2][i][j],
			};

			i_lut = maisProximoNaLUT(lut, cor);

			salvaCorDaLUT(bytes, i_lut, lut->bpp, i_bytes);
			i_bytes += lut->bpp;
		}
	}

	*n_bytes = i_bytes / 8;

	return bytes;
}

unsigned char pegaCampoPorBit (unsigned char *buffer, int pos, int size) {
	int byte_pos = pos / 8;
	int bit_pos = pos % 8;

	return ((buffer[byte_pos] << bit_pos) & 0x0FF) >> (8 - size);
}

Imagem3C* decodificaImagem (unsigned char* codificada, int n_bytes,
		LUT* lut, int altura, int largura) {

	int i;
	int j;
	int i_codificada = 0;

	Imagem3C *img = criaImagem3C(largura, altura);

	for (i = 0; i < img->altura; i++) {
		for (j = 0; j < img->largura; j++) {
			int lut_pos = pegaCampoPorBit(codificada,
				i_codificada, lut->bpp);

			img->dados[0][i][j] = lut->cores[lut_pos][0];
			img->dados[1][i][j] = lut->cores[lut_pos][1];
			img->dados[2][i][j] = lut->cores[lut_pos][2];

			i_codificada += lut->bpp;
		}
	}

	return img;
}
