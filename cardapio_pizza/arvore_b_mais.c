#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <limits.h>
#include <stdlib.h>

#include "arvore_b_mais.h"
#include "lista_pizzas.h"
#include "metadados.h"
#include "no_interno.h"
#include "no_folha.h"

int busca(int cod, char *nome_arquivo_metadados, char *nome_arquivo_indice, char *nome_arquivo_dados, int d){

    //LÊ O ARQUIVO DE INDICES
    FILE * arq_indice = fopen(nome_arquivo_indice, "rb");

    //RECEBE OS METADADOS
    TMetadados *metadados = le_arq_metadados(nome_arquivo_metadados);

    //TESTA SE RAIZ É FOLHA E RETORNA RAIZ PARA INSERÇÃO
    if (metadados->raiz_folha == 1){
        return metadados->pont_raiz;
    }
    else{
        //POSICIONA PONTEIRO NO INICIO DO ARQUIVO DE INDICES
        fseek(arq_indice, metadados->pont_raiz, SEEK_SET);

        int loop = 0;

        TNoInterno * noInterno = le_no_interno(d, arq_indice);

        if (noInterno != NULL){

            while(loop == 0){

                for (int i = 0; i < noInterno->m; i++){

                    if (noInterno->chaves[i] > cod){

                        if (noInterno->aponta_folha == 1){
                            return noInterno->p[i];
                        }
                        else{
                            fseek(arq_indice, noInterno->p[i], SEEK_SET);
                            noInterno = le_no_interno(d, arq_indice);
                            break;
                        }

                    }

                    if (i == (noInterno->m - 1)){

                        if (noInterno->aponta_folha == 1){
                            return noInterno->p[noInterno->m];
                        }

                        else{
                            fseek(arq_indice, noInterno->p[noInterno->m], SEEK_SET);
                            noInterno = le_no_interno(d, arq_indice);
                            break;
                        }
                    }
                }

            }
        }
    }
}

int insere(int cod, char *nome, char *categoria, float preco, char *nome_arquivo_metadados, char *nome_arquivo_indice, char *nome_arquivo_dados, int d)
{
    FILE * arq_indice = fopen(nome_arquivo_indice, "rb+");
    FILE * arq_dados = fopen(nome_arquivo_dados, "rb+");
    FILE * arq_metadados = fopen(nome_arquivo_metadados, "rb+");

    int var_busca = busca(cod, nome_arquivo_metadados, nome_arquivo_indice, nome_arquivo_dados, d);

    //CHAVE JÁ ESTÁ NA ARVORE
    if (var_busca == cod){
        return -1;
    }
    else {

        //SETA O PONTEIRO PARA O ENDEREÇO RETORNADO PELA BUSCA
        fseek(arq_dados, var_busca, SEEK_SET);

        //CRIA A PIZZA
        TPizza * p = pizza(cod, nome, categoria, preco);

        //LÊ O NÓ FOLHA DO ARQUIVO
        TNoFolha * noFolha = le_no_folha(d, arq_dados);

        //CASO AINDA HAJA ESPAÇO NA FOLHA PARA ADICIONAR
        if (noFolha->m < 2 * d) {

            //INSERE NO FINAL DO VETOR DE PIZZAS E INCREMENTA M EM UMA UNIDADE
            noFolha->pizzas[noFolha->m] = p;
            noFolha->m++;

            //REORDENA O VETOR DE PIZZAS POR CÓDIGO DA PIZZA
            for (int i = 1; i < noFolha->m; i++) {

                for (int j = 0; j < noFolha->m - i; j++) {
                    if (noFolha->pizzas[j]->cod > noFolha->pizzas[j + 1]->cod) {
                        TPizza *aux = noFolha->pizzas[j];
                        noFolha->pizzas[j] = noFolha->pizzas[j + 1];
                        noFolha->pizzas[j + 1] = aux;

                    }
                }
            }

            //SETA O PONTEIRO PARA O QUE FOI RETORNADO DA BUSCA
            fseek(arq_dados, var_busca, SEEK_SET);

            //SALVA O ARQUIVO DE DADOS E ENTÃO O FECHA
            salva_no_folha(d, noFolha, arq_dados);
            fclose(arq_dados);

            return var_busca;
        }
    }


    //RETORNO COM ERRO
    return -1;

}

int exclui(int cod, char *nome_arquivo_metadados, char *nome_arquivo_indice, char *nome_arquivo_dados, int d)
{
	// ABRE OS ARQUIVOS NECESSARIOS
	FILE * arq_metadados = fopen(nome_arquivo_metadados, "rb+");
	FILE * arq_indice = fopen(nome_arquivo_indice, "rb+");
	FILE * arq_dados = fopen(nome_arquivo_dados, "rb+");

	int var_busca = busca(cod, nome_arquivo_metadados, nome_arquivo_indice, nome_arquivo_dados, d);

	// CASO A BUSCA RETORNE -1, A PIZZA NÃO FOI ENCONTRADA E POR ISSO NÃO PODE FAZER A REMOÇÃO
	if (var_busca == -1)
	    return -1;
	else {

	    // SETA O PONTEIRO PARA O ENDEREÇO RETORNADO NA BUSCA
	    fseek(arq_dados, var_busca, SEEK_SET);

	    // LÊ O NÓ FOLHA
	    TNoFolha* noFolha = le_no_folha(d, arq_dados);

	    // CASO A REMOÇÃO VÁ MANTER O NÓ FOLHA COM TAMANHO SUFICIENTE
	    if (noFolha->m > d) {

	        // REMOVE A PIZZA DA FOLHA
	        for(int i = 0; i < noFolha->m; i++) {

	            if (cod == noFolha->pizzas[i]->cod) {
                    for (int j = i; j < noFolha->m - 1; j++) {
                        noFolha->pizzas[j] = noFolha->pizzas[j + 1];
                    }

                    noFolha->pizzas[noFolha->m - 1] = NULL;
                    free(noFolha->pizzas[noFolha->m - 1]);

                    noFolha->m--;
                    break;
                }
	        }

	        // SALVA O NÓ FOLHA NO ARQUIVO DE DADOS E FECHA OS ARQUIVOS
	        fseek(arq_dados, var_busca, SEEK_SET);
	        salva_no_folha(d, noFolha, arq_dados);
	        fclose(arq_metadados);
	        fclose(arq_indice);
	        fclose(arq_dados);

	        // RETORNA A PIZZA REMOVIDA
	        return var_busca;

	    }

	}

    return INT_MAX;
}


void carrega_dados(int d, char *nome_arquivo_entrada, char *nome_arquivo_metadados, char *nome_arquivo_indice, char *nome_arquivo_dados){

    //ABRE OS ARQUIVOS NECESSARIOS
    FILE * arq_entrada = fopen(nome_arquivo_entrada, "rb");
    FILE * arq_indice = fopen(nome_arquivo_indice, "wb");
    FILE * arq_dados = fopen(nome_arquivo_dados, "wb");

    //CRIA UM ARQUIVO DE METADADOS E SALVA-O EM ARQUIVO
    TMetadados * Metadados = metadados(d, 0, 1, 0, 1 * tamanho_no_folha(d));
    salva_arq_metadados(nome_arquivo_metadados, Metadados);

    //CRIA UM NÓ INTERNO E SALVA-O EM ARQUIVO
    TNoInterno *noInterno = no_interno_vazio(d);
    salva_no_interno(d, noInterno, arq_indice);

    //CRIA UM NO FOLHA E SALVA-O EM ARQUIVO
    TNoFolha *noFolha = no_folha_vazio(d);
    salva_no_folha(d, noFolha, arq_dados);

    //FECHA OS ARQUIVOS QUE NÃO SERÃO MAIS UTILIZADOS
    fclose(arq_dados);
    fclose(arq_indice);

    //LÊ PIZZA DO ARQUIVO DE ENTRADA
    TPizza * p = le_pizza(arq_entrada);

    //FAZ ISSO ENQUANTO PIZZA FOR DIFERENTE DE NULL
    while(p != NULL){
        //CRIA UMA PIZZA COM O QUE FOI LIDO DO ARQUIVO DE ENTRADA
        p = pizza(p->cod, p->nome, p->categoria, p->preco);
        //INSERE A PIZZA
        insere(p->cod, p->nome, p->categoria, p->preco, nome_arquivo_metadados, nome_arquivo_indice, nome_arquivo_dados, d);
        //PIZZA RECEBE PROXIMA PIZZA DO ARQUIVO DE ENTRADA
        p = le_pizza(arq_entrada);
    }

    //FECHA ARQUIVO DE ENTRADA
    fclose(arq_entrada);
}