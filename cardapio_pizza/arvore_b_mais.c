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

        fseek(arq_dados, var_busca, SEEK_SET);

        TPizza * p = pizza(cod, nome, categoria, preco);

        TNoFolha * noFolha = le_no_folha(d, arq_dados);

        if (noFolha->m < 2 * d) {

            //INSERE NO FINAL DO VETOR DE PIZZAS E INCREMENTA M EM UMA UNIDADE
            noFolha->pizzas[noFolha->m] = p;
            noFolha->m++;

            //REORDENA O VETOR DE PIZZAS
            for (int i = 1; i < noFolha->m; i++) {

                for (int j = 0; j < noFolha->m - i; j++) {
                    if (noFolha->pizzas[j]->cod > noFolha->pizzas[j + 1]->cod) {
                        TPizza *aux = noFolha->pizzas[j];
                        noFolha->pizzas[j] = noFolha->pizzas[j + 1];
                        noFolha->pizzas[j + 1] = aux;

                    }
                }
            }
            fseek(arq_dados, var_busca, SEEK_SET);

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
	//TODO: Inserir aqui o codigo do algoritmo de remocao
    return INT_MAX;
}


void carrega_dados(int d, char *nome_arquivo_entrada, char *nome_arquivo_metadados, char *nome_arquivo_indice, char *nome_arquivo_dados){

    FILE * arq_entrada = fopen(nome_arquivo_entrada, "rb");
    FILE * arq_indice = fopen(nome_arquivo_indice, "wb");
    FILE * arq_dados = fopen(nome_arquivo_dados, "wb");

    TMetadados *tabMetadados = metadados(d, 0, 1, 0, 1 * tamanho_no_folha(d));
    salva_arq_metadados(nome_arquivo_metadados, tabMetadados);

    TNoInterno *noInterno = no_interno_vazio(d);
    salva_no_interno(d, noInterno, arq_indice);

    TNoFolha *noFolha = no_folha_vazio(d);
    salva_no_folha(d, noFolha, arq_dados);

    fclose(arq_dados);
    fclose(arq_indice);

    TPizza * p;
    p = le_pizza(arq_entrada);

    while(p != NULL){
        p = pizza(p->cod, p->nome, p->categoria, p->preco);
        insere(p->cod, p->nome, p->categoria, p->preco, nome_arquivo_metadados, nome_arquivo_indice, nome_arquivo_dados, d);
        free(p);
        p = le_pizza(arq_entrada);
    }

    fclose(arq_entrada);
}