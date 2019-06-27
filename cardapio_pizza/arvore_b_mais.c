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

    //FAZ A BUSCA E RETORNA PONTEIRO PRO NÓ QUE O CÓDIGO ESTÁ PRESENTE
    int var_busca = busca(cod, nome_arquivo_metadados, nome_arquivo_indice, nome_arquivo_dados, d);

    //COLOCA O CURSOS NO DEVIDO LOCAL
    fseek(arq_dados, var_busca, SEEK_SET);

    //LÊ O NÓ FOLHA
    TNoFolha * noFolha = le_no_folha(d, arq_dados);


    for (int i = 0; i < noFolha->m; i++){
        //VERIFICA SE A PIZZA JÁ ESTÁ NO NÓ
        if (cod == noFolha->pizzas[i]->cod)
            return -1;
    }

    //CRIA A PIZZA A SER INSERIDA POSTERIORMENTE
    TPizza * p = pizza(cod, nome, categoria, preco);

    //VERIFICA SE POSSUI ESPAÇO NO NÓ PARA INSERÇÃO
    if (noFolha->m < (2 * d)){

        //INSERE PIZZA NO FINAL DO NÓ E INCREMENTA M EM UMA UNIDADE
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

        //SALVA O ARQUIVO DE DADOS E FECHA ARQUIVOS
        salva_no_folha(d, noFolha, arq_dados);
        fclose(arq_dados);
        fclose(arq_indice);
        fclose(arq_metadados);

        return var_busca;

    }
    //CASO NÃO HAJA ESPAÇO EM NÓ E NECESSITE PARTICIONAMENTO
    else{

        //POSICIONA O CURSOR NO PAI
        int endereco_pai = fseek(arq_indice, noFolha->pont_pai, SEEK_SET);

        //LÊ O NÓ INTERNO
        TNoInterno * noInterno = le_no_interno(d, arq_indice);

        //CRIA NOVO NÓ FOLHA
        TNoFolha * novoNo = no_folha_vazio(d);

        //PERCORRE A NO FOLHA E VERIFICA ONDE DEVE SER INSERIDO A NOVA PIZZA
        int controle = 0;
        for (int i = 0; i < noFolha->m; i++){

            if (cod < noFolha->pizzas[i]->cod)
                break;

            controle++;
        }

        //POSIÇÕES MAIORES QUE D VÃO PARA O NOVO NÓ
        if (controle >= d){
            int aux_index = 0;
            int aux_tam = noFolha->m;
            for (int i = d; i < aux_tam; i++){
                novoNo->pizzas[aux_index] = pizza(noFolha->pizzas[i]->cod, noFolha->pizzas[i]->nome, noFolha->pizzas[i]->categoria, noFolha->pizzas[i]->preco);
                noFolha->pizzas[i] = NULL;
                noFolha->m--;
                novoNo->m++;
                aux_index++;
            }
            //ADICIONA NOVA PIZZA NO FINAL DO NOVO NÓ
            novoNo->pizzas[novoNo->m] = p;
            novoNo->m++;
        }
        //INSERE NO NÓ FOLHA E MOVE OUTROS PARA NOVO NÓ
        else{
            int aux_index = 0;
            int aux_tam = noFolha->m;
            for (int i = 0; i < aux_tam; i++){
                if (i >= d - 1){
                    novoNo->pizzas[aux_index] = pizza(noFolha->pizzas[i]->cod, noFolha->pizzas[i]->nome, noFolha->pizzas[i]->categoria, noFolha->pizzas[i]->preco);
                    noFolha->pizzas[i] = NULL;
                    noFolha->m--;
                    novoNo->m++;
                    aux_index++;
                }
            }
            noFolha->pizzas[noFolha->m] = p;
            noFolha->m++;

        }

        //ORDENA NO FOLHA

        for (int i = 1; i < noFolha->m; i++) {
            for (int j = 0; j < noFolha->m - i; j++) {
                if (noFolha->pizzas[j]->cod > noFolha->pizzas[j + 1]->cod) {
                    TPizza *aux = noFolha->pizzas[j];
                    noFolha->pizzas[j] = noFolha->pizzas[j + 1];
                    noFolha->pizzas[j + 1] = aux;
                }
            }
        }

        //ORDENA NOVO NO
        for (int i = 1; i < novoNo->m; i++) {
            for (int j = 0; j < novoNo->m - i; j++) {
                if (novoNo->pizzas[j]->cod > novoNo->pizzas[j + 1]->cod) {
                    TPizza *aux = novoNo->pizzas[j];
                    novoNo->pizzas[j] = novoNo->pizzas[j + 1];
                    novoNo->pizzas[j + 1] = aux;
                }
            }
        }
        
        //AINDA HÁ ESPAÇO PARA ADICIONAR CHAVES NO PAI
//        if (noInterno->m < 2*d){
//
//            for (int i = noInterno->m - 1; i >= 0; i--){
//                if (controle < i){
//                    noInterno->chaves[i] = noInterno->chaves[i - 1];
//                    noInterno->p[i] = noInterno->p[i - 1];
//                }
//                if (controle == i){
//                    noInterno->chaves[i] = chave;
//                    noInterno->p[i] = noFolha->pont_prox;
//                }
//            }
//        }


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

	        // LIMPA O NÓ AUXILIAR
	        libera_no_folha(d, noFolha);

	        // RETORNA A PIZZA REMOVIDA
	        return var_busca;

	    }
	    else {
	        // CASO SEJA NECESSÁRIO FAZER CONCATENAÇÃO OU REDISTRIBUIÇÃO

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

	        // LÊ O PRÓXIMO NÓ FOLHA
	        fseek(arq_dados, noFolha->pont_prox, SEEK_SET);
	        TNoFolha* noFolhaProx = le_no_folha(d, arq_dados);

	        // CASO SEJA POSSÍVEL FAZER A REDISTRIBUIÇÃO COM O NÓ FOLHA DA DIREITA
	        if (noFolha->m + noFolhaProx->m >= (2 * d)) {

                // PASSA AS D PRIMEIRAS PIZZAS DO PRÓXIMO NÓ FOLHA PARA O NÓ EM QUE OCORREU A REMOÇÃO
	            for(int i = 0; noFolha->m < d; i++) {
	                noFolha->pizzas[i + noFolha->m] = noFolhaProx->pizzas[i];
	                noFolha->m++;

	                noFolhaProx->pizzas[i] = NULL;
	                noFolhaProx->m--;
	            }

	            // REORGANIZA O PRÓXIMO NÓ FOLHA
	            while(noFolhaProx->pizzas[0] == NULL) {
	                for(int i = 0; i < (2*d - 1); i++) {
	                    noFolhaProx->pizzas[i] = noFolhaProx->pizzas[i+1];
	                }
	            }
                noFolhaProx->pizzas[noFolhaProx->m] = NULL;

	            // LÊ O NÓ INTERNO QUE APONTA PARA O PRÓXIMO NÓ E ALTERA O VALOR DA CHAVE
	            fseek(arq_indice, noFolhaProx->pont_pai, SEEK_SET);
	            TNoInterno* noInterno = le_no_interno(d, arq_indice);

	            // VARIÁVEL PARA ARMAZENAR QUAL DOS PONTEIROS DO NÓ INTERNO À SER ALTERADA
	            int controle = 0;
	            for (int i = 0; i < noInterno->m; i++){
	                if (noInterno->p[i] == noFolha->pont_prox){
	                    break;
	                }
	                controle++;
	            }

	            // ATUALIZA O NÓ INTERNO
	            noInterno->chaves[controle-1] = noFolhaProx->pizzas[0]->cod;

	            // SALVA AS ALTERAÇÕES NO DISCO
	            fseek(arq_dados, var_busca, SEEK_SET);
	            salva_no_folha(d, noFolha, arq_dados);

	            fseek(arq_dados, noFolha->pont_prox, SEEK_SET);
	            salva_no_folha(d, noFolhaProx, arq_dados);

	            fseek(arq_indice, noFolha->pont_pai, SEEK_SET);
	            salva_no_interno(d, noInterno, arq_indice);

	            // FECHA ARQUIVOS
	            fclose(arq_metadados);
	            fclose(arq_indice);
	            fclose(arq_dados);

	            // LIMPA OS NÓS AUXILIARES
	            libera_no_folha(d, noFolha);
	            libera_no_folha(d, noFolhaProx);
	            libera_no_interno(noInterno);

	            return var_busca;
	        }

	        // CASO SEJA NECESSÁRIA A CONCATENAÇÃO
	        else if (noFolha->m + noFolhaProx->m < 2*d) {
//	            printf("\n-- Concatenação incompleta --\n");

	            fseek(arq_indice, noFolha->pont_pai, SEEK_SET);
	            TNoInterno * noInterno = le_no_interno(d, arq_indice);

                // MOVE AS PIZZAS DO PRÓXIMO NÓ PARA O NÓ NO QUAL OCORREU A REMOÇÃO
                for(int i = 0; noFolhaProx->m > 0; i++) {
                    noFolha->pizzas[noFolha->m] = noFolhaProx->pizzas[i];
                    noFolha->m++;
                    noFolhaProx->m--;
                }

                // VARIÁVEL PARA ARMAZENAR QUAL DOS PONTEIROS DO NÓ INTERNO À SER ALTERADA
                int controle = 0;
                for (int i = 0; i < noInterno->m; i++){
                    if (noInterno->p[i] == noFolha->pont_prox){
                        break;
                    }
                    controle++;
                }

                // ATUALIZA O NÓ INTERNO
                for(int i = 0; i < 2*d - 1; i++) {
                    if (i >= controle) {
                        noInterno->chaves[i-1] = noInterno->chaves[i];
                        noInterno->p[i] = noInterno->p[i+1];
                    }
                }
                noInterno->chaves[noInterno->m] = -1;
                noInterno->p[noInterno->m] = -1;
                noInterno->m--;

                // ATUALIZA O PONTEIRO DO NÓ FOLHA E LIMPA A PRÓX FOLHA
                noFolha->pont_prox = noFolhaProx->pont_prox;
                libera_no_folha(d, noFolhaProx);

                // SALVA ALTERAÇÕES NO DISCO
                fseek(arq_dados, var_busca, SEEK_SET);
                salva_no_folha(d, noFolha, arq_dados);

                fseek(arq_indice, noFolha->pont_pai, SEEK_SET);
                salva_no_interno(d, noInterno, arq_indice);

                // TESTES
//                TMetadados* meta = le_arq_metadados(nome_arquivo_metadados);
//                printf("\nMetadados:  ");
//                imprime_metadados(meta);
//                fseek(arq_indice, meta->pont_raiz, SEEK_SET);
//                TNoInterno* raiz = le_no_interno(d, arq_indice);
//                printf("\nRaiz:  ");
//                imprime_no_interno(d, raiz);
//                TNoFolha* f1, *f2;
//                fseek(arq_dados, raiz->p[0], SEEK_SET);
//                f1 = le_no_folha(d, arq_dados);
//                fseek(arq_dados, raiz->p[1], SEEK_SET);
//                f2 = le_no_folha(d, arq_dados);
//                printf("\nFolha 0:\n");
//                imprime_no_folha(d, f1);
//                printf("\nFolha 1:\n");
//                imprime_no_folha(d, f2);
                return var_busca;

            }

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