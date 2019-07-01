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
#include <string.h>

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

    //ABRE OS ARQUIVOS NECESSÁRIOS
    FILE * arq_indice = fopen(nome_arquivo_indice, "rb+");
    FILE * arq_dados = fopen(nome_arquivo_dados, "rb+");
    FILE * arq_metadados = fopen(nome_arquivo_metadados, "rb+");

    //BUSCA O CÓDIGO A TENTAR SER INSERIDO E RETORNA O PONTEIRO PARA O NÓ ONDE ESTÁ OU DEVERIA ESTAR
    int var_busca = busca(cod, nome_arquivo_metadados, nome_arquivo_indice, nome_arquivo_dados, d);
    //COLOCA O CURSOR NA POSIÇÃO RETORNADA
    fseek(arq_dados, var_busca, SEEK_SET);


    TNoFolha * noFolha = le_no_folha(d, arq_dados);

    if(noFolha->m < (2 * d)){

        TPizza * aux = pizza(cod, nome, categoria, preco);

        if(noFolha->m != 0){

            TPizza *aux_2 = pizza(cod, nome, categoria, preco);

            for(int i = 0; i < noFolha->m; i++){

                //CASO O NÓ JÁ EXISTA NA ARVORE
                if(noFolha->pizzas[i]->cod == cod){
                    return -1;
                }

                //CASO CONTRÁRIO, INSERE NO DEVIDO LOCAL
                if(noFolha->pizzas[i]->cod > aux->cod){

                    //TROCA O VALOR DO AUX PELO DA PIZZA MAIOR
                    aux->cod = noFolha->pizzas[i]->cod;
                    strcpy(aux->nome, noFolha->pizzas[i]->nome);
                    strcpy(aux->categoria, noFolha->pizzas[i]->categoria);
                    aux->preco = noFolha->pizzas[i]->preco;

                    //TROCA O VALOR DA PIZZA PELA QUE DEVE SER INSERIDA
                    noFolha->pizzas[i]->cod = aux_2->cod;
                    strcpy(noFolha->pizzas[i]->nome, aux_2->nome);
                    strcpy(noFolha->pizzas[i]->categoria, aux_2->categoria);
                    noFolha->pizzas[i]->preco = aux_2->preco;

                    //ACERTA O AUX 2
                    aux_2->cod = aux->cod;
                    strcpy(aux_2->nome, aux->nome);
                    strcpy(aux_2->categoria, aux->categoria);
                    aux_2->preco = aux->preco;
                }
            }

            free(aux_2);
        }

        noFolha->pizzas[noFolha->m] = aux;
        noFolha->m++;

        TNoFolha * noFolha_aux = no_folha(d, noFolha->m, noFolha->pont_pai, noFolha->pont_prox);
        for(int i = 0; i < noFolha->m; i++){

            noFolha_aux->pizzas[i] = pizza(noFolha->pizzas[i]->cod, noFolha->pizzas[i]->nome, noFolha->pizzas[i]->categoria, noFolha->pizzas[i]->preco);
        }

        fseek(arq_dados, var_busca, SEEK_SET);
        salva_no_folha(d, noFolha_aux, arq_dados);

        fclose(arq_dados);
        fclose(arq_indice);


        free(aux);
        free(noFolha);
        free(noFolha_aux);

        return var_busca;
    }
    else{

        //DA O SEEK NO ARQUIVO DE INDICE ATÉ O CORRESPONDENTE
        fseek(arq_indice, noFolha->pont_pai, SEEK_SET);

        TPizza *aux = pizza(cod, nome, categoria, preco);

        //CRIAR NOVO NÓ FOLHA VAZIO
        TNoFolha * novo_noFolha = no_folha_vazio(d);
        int troca = 0;

        //ADICIONAR AS PRIMEIRAS D PIZZAS(ORDENADAS POR CODIGO) NO noFolha
        TPizza *aux_2 = pizza(cod, nome, categoria, preco);

        for(int i = 0; i < noFolha->m; i++){

            //CASO O NÓ JÁ EXISTA NA ARVORE
            if(noFolha->pizzas[i]->cod == cod){
                return -1;
            }

            //CASO CONTRÁRIO, INSERE NO DEVIDO LOCAL
            if(noFolha->pizzas[i]->cod > aux->cod){

                //TROCA O VALOR DO AUX PELO DA PIZZA MAIOR
                aux->cod = noFolha->pizzas[i]->cod;
                strcpy(aux->nome, noFolha->pizzas[i]->nome);
                strcpy(aux->categoria, noFolha->pizzas[i]->categoria);
                aux->preco = noFolha->pizzas[i]->preco;

                //TROCA O VALOR DA PIZZA PELA QUE DEVE SER INSERIDA
                noFolha->pizzas[i]->cod = aux_2->cod;
                strcpy(noFolha->pizzas[i]->nome, aux_2->nome);
                strcpy(noFolha->pizzas[i]->categoria, aux_2->categoria);
                noFolha->pizzas[i]->preco = aux_2->preco;

                //ACERTA O AUX 2
                aux_2->cod = aux->cod;
                strcpy(aux_2->nome, aux->nome);
                strcpy(aux_2->categoria, aux->categoria);
                aux_2->preco = aux->preco;
            }
        }

        free(aux_2);

        noFolha->pizzas[noFolha->m] = aux;
        noFolha->m++;

        int trocou_folha = 0;

        //ADICIONAR AS OUTRAS PIZZAS NO novo_noFolha
        for(int i = d; i < (2*d + 1); i++){

            if(noFolha->pizzas[i]->cod == cod) trocou_folha = 1;

            novo_noFolha->pizzas[i - d] = pizza(noFolha->pizzas[i]->cod, noFolha->pizzas[i]->nome, noFolha->pizzas[i]->categoria, noFolha->pizzas[i]->preco);
            noFolha->pizzas[i] = NULL;
        }

        noFolha->m = d;
        novo_noFolha->m = d + 1;

        //ABRE O ARQUIVO DE METADADOS PARA ATUALIZAR AS REFERENCIAS
        TMetadados *metadados = le_arq_metadados(nome_arquivo_metadados);

        int ret;

        if(trocou_folha == 0){
            ret = var_busca;
        }
        else{
            ret = metadados->pont_prox_no_folha_livre;
        }

        if(metadados->raiz_folha == 0){

            //MODIFICAR O ARQUIVO DE INDICE PARA TER UM PONTEIRO AO novo_noFolha
            //   lembrete: O NOVO NO FOLHA FICARA NECESSÁRIAMENTE APOS O ANTIGO NO ARQUIVO

            fseek(arq_indice, noFolha->pont_pai, SEEK_SET);
            TNoInterno * noInterno = le_no_interno(d, arq_indice);
            int chave = novo_noFolha->pizzas[0]->cod;
            int aux_chave = chave;

            //CASO O NO INTERNO TENHA ESPAÇO PARA INSERÇÃO
            //if(noInterno->m < 2*d){

            int flag = 0;
            int aux_pont; int aux_pont_02;

            //ACERTA OS PONTEIROS DO NÓ INTERNO
            //O ESPAÇO PARA O NOVO PONTEIRO FICA NO LUGAR ONDE O -2 FOI INSERIDO
            for(int i = 0; i < noInterno->m; i++){

                if(noInterno->chaves[i] > chave){

                    if(flag == 0){
                        aux_pont = noInterno->p[i + 1];
                        noInterno->p[i + 1] = -2;
                        flag = 1;
                    }
                    else{
                        aux_pont_02 = noInterno->p[i + 1];
                        noInterno->p[i + 1] = aux_pont;
                        aux_pont = aux_pont_02;
                    }
                    aux_chave = noInterno->chaves[i];
                    noInterno->chaves[i] = chave;
                    chave = aux_chave;

                }
            }
            //AUMENTO DO NUMERO DE CHAVES NO NO INTERNO
            noInterno->chaves[noInterno->m] = chave;
            noInterno->m ++;

            //ULTIMO ACERTO DO PONTEIRO PARA A TROCA
            if(flag == 0){
                noInterno->p[noInterno->m] = -2;
            }
            else{
                noInterno->p[noInterno->m] = aux_pont;
            }

            int pont_novo;

            pont_novo = metadados->pont_prox_no_folha_livre;

            novo_noFolha->pont_pai = noFolha->pont_pai;
            novo_noFolha->pont_prox = noFolha->pont_prox;
            noFolha->pont_prox = pont_novo;

            //SALVAR ARQUIVO DE DADOS
            fseek(arq_dados, var_busca, SEEK_SET);
            salva_no_folha(d, noFolha, arq_dados);

            fseek(arq_dados, pont_novo, SEEK_SET);
            salva_no_folha(d, novo_noFolha, arq_dados);

            //FECHA ARQUIVO DE DADOS
            //fclose(fd);

            if(flag == 0){
                noInterno->p[noInterno->m] = pont_novo;
            }
            else{
                for(int i = 0; i < noInterno->m; i++){

                    if(noInterno->p[i] == -2){
                        noInterno->p[i] = pont_novo;
                        break;
                    }
                }
            }

            //FLAG DO LOOP
            int flag_loop = 0;

            //ATUALIZAR ARQUIVO METADADOS
            metadados->pont_prox_no_folha_livre = noFolha->pont_prox + tamanho_no_folha(d);

            //CASO O NO INTERNO ESTEJA CHEIO, PARTICIONA
            if(noInterno->m > 2 * d){

                int pont_noInterno = noFolha->pont_pai;    int loop = 0;
                int ref_pai = -1;

                TNoInterno * novo_noInterno;
                TNoInterno * pai_noInterno;

                while(loop == 0){

                    ref_pai = noInterno->chaves[d];

                    //CRIA UM NOVO NO VAZIO
                    novo_noInterno = no_interno_vazio(d);

                    //CASO NAO TENHA NO PAI, CRIA UM
                    if(noInterno->pont_pai == -1){
                        pai_noInterno = no_interno_vazio(d);
                        loop = 1;
                    }
                    else{
                        //RECEBE O PAI
                        fseek(arq_indice, noInterno->pont_pai, SEEK_SET);
                        pai_noInterno = le_no_interno(d, arq_indice);
                    }

                    //TRANSFERE OS PONTEIROS PARA O NOVO NO INTERNO E ATUALIZA O NO INTERNO
                    for(int i = d + 1; i <= noInterno->m; i++){

                        novo_noInterno->p[i - d - 1] = noInterno->p[i];
                        noInterno->p[i] = -1;
                        if(noInterno->aponta_folha == 0 && i < noInterno->m){
                            novo_noInterno->chaves[i - d - 1] = noInterno->chaves[i];
                        }
                        noInterno->chaves[i - 1] = -1;
                        novo_noInterno->m = novo_noInterno->m + 1;
                    }

                    noInterno->m = d;

                    //COMO O M FOI ESTABELECIDO COM PONTEIROS E SEMPRE TEM M+1 P
                    novo_noInterno->m = novo_noInterno->m - 1;
                    novo_noInterno->aponta_folha = noInterno->aponta_folha;

                    //ATUALIZA AS CHAVES DO NOVO NO INTERNO
                    for(int i = 0; i < novo_noInterno->m; i++){

                        if(novo_noInterno->aponta_folha == 1){
                            fseek(arq_dados, novo_noInterno->p[i + 1], SEEK_SET);
                            TNoFolha * aux_folha = le_no_folha(d, arq_dados);
                            novo_noInterno->chaves[i] = aux_folha->pizzas[0]->cod;
                        }
                    }

                    //ATUALIZA OS PONTEIROS PAI DOS NO FOLHAS/INTERNOS
                    for(int i = 0; i < novo_noInterno->m + 1; i++){

                        if(novo_noInterno->aponta_folha == 1){
                            fseek(arq_dados, novo_noInterno->p[i], SEEK_SET);
                            TNoFolha * aux_folha = le_no_folha(d, arq_dados);

                            //ATUALIZA PONTEIRO DO PAI DA FOLHA
                            aux_folha->pont_pai = metadados->pont_prox_no_interno_livre;
                            fseek(arq_dados, novo_noInterno->p[i], SEEK_SET);
                            salva_no_folha(d, aux_folha, arq_dados);

                            //free(aux_folha);
                        }
                        else{
                            fseek(arq_indice, novo_noInterno->p[i], SEEK_SET);
                            TNoInterno * aux_interno = le_no_interno(d, arq_indice);

                            //ATUALIZA PONTEIRO DO PAI DO NO INTERNO
                            aux_interno->pont_pai = metadados->pont_prox_no_interno_livre;
                            fseek(arq_indice, novo_noInterno->p[i], SEEK_SET);
                            salva_no_interno(d, aux_interno, arq_indice);

                            //free(aux_interno);
                        }
                    }
                    //O PAI É UM NOVO NO
                    if(loop == 1){

                        pai_noInterno->p[0] = pont_noInterno;
                        pai_noInterno->p[1] = metadados->pont_prox_no_interno_livre;
                        pai_noInterno->chaves[0] = ref_pai;  //novo_noInterno->chaves[0];
                        pai_noInterno->m = 1;

                        novo_noInterno->pont_pai = metadados->pont_prox_no_interno_livre + tamanho_no_interno(d);
                        fseek(arq_indice, metadados->pont_prox_no_interno_livre, SEEK_SET);
                        salva_no_interno(d, novo_noInterno, arq_indice);

                        noInterno->pont_pai = metadados->pont_prox_no_interno_livre + tamanho_no_interno(d);
                        fseek(arq_indice, pont_noInterno, SEEK_SET);
                        salva_no_interno(d, noInterno, arq_indice);

                        fseek(arq_indice, metadados->pont_prox_no_interno_livre + tamanho_no_interno(d), SEEK_SET);
                        salva_no_interno(d, pai_noInterno, arq_indice);

                        metadados->pont_raiz = metadados->pont_prox_no_interno_livre + tamanho_no_interno(d);
                        metadados->pont_prox_no_interno_livre = metadados->pont_prox_no_interno_livre + (2*tamanho_no_interno(d));

                        salva_arq_metadados(nome_arquivo_metadados, metadados);
                    }
                    else{
                        int aux_pos = -2;

                        for(int i = 0; i < pai_noInterno->m; i++){
                            if(pai_noInterno->chaves[i] > noInterno->chaves[0]){
                                aux_pos = i;
                                break;
                            }
                        }

                        if(aux_pos != -2){

                            for(int i = pai_noInterno->m - 1; i > aux_pos; i++){
                                pai_noInterno->chaves[i + 1] = pai_noInterno->chaves[i];
                                pai_noInterno->p[i + 2] = pai_noInterno->p[i + 1];
                            }

                            pai_noInterno->p[aux_pos + 1] = metadados->pont_prox_no_interno_livre;
                            pai_noInterno->chaves[aux_pos] = ref_pai;

                        }
                            //O NO INTERNO É A ULTIMA CHAVE, LOGO É ´SO ADICIONAR NO FINAL
                        else{
                            pai_noInterno->chaves[pai_noInterno->m] = ref_pai;
                            pai_noInterno->p[pai_noInterno->m + 1] = metadados->pont_prox_no_interno_livre;
                            pai_noInterno->m = pai_noInterno->m + 1;
                        }

                        novo_noInterno->pont_pai = noInterno->pont_pai;
                        fseek(arq_indice, metadados->pont_prox_no_interno_livre, SEEK_SET);
                        salva_no_interno(d, novo_noInterno, arq_indice);

                        fseek(arq_indice, pont_noInterno, SEEK_SET);
                        salva_no_interno(d, noInterno, arq_indice);

                        fseek(arq_indice, novo_noInterno->pont_pai, SEEK_SET);
                        salva_no_interno(d, pai_noInterno, arq_indice);

                        metadados->pont_prox_no_interno_livre = metadados->pont_prox_no_interno_livre + tamanho_no_interno(d);

                        if(pai_noInterno->m <= 2*d){
                            loop = 1;
                        }
                            //RECOMEÇA O LOOP
                        else{
                            flag_loop = 1;

                            pont_noInterno = noInterno->pont_pai;
                            noInterno = pai_noInterno;
                        }
                    }
                }
            }
            else{
                //SALVAR ARQUIVO DE INDICE
                fseek(arq_indice, noFolha->pont_pai, SEEK_SET);
                salva_no_interno(d, noInterno, arq_indice);
            }

            fclose(arq_indice);
            fclose(arq_dados);

            if(flag_loop == 0){
                //FECHA ARQUIVO DE METADADOS
                salva_arq_metadados(nome_arquivo_metadados, metadados);
            }

            return ret;
        }
        else{

            TNoInterno * noInterno = no_interno_vazio(d);
            noInterno->p[0] = var_busca;
            noInterno->p[1] = metadados->pont_prox_no_folha_livre;
            noInterno->chaves[0] = novo_noFolha->pizzas[0]->cod;
            noInterno->m = 1;
            noInterno->aponta_folha = 1;

            fseek(arq_indice, 0, SEEK_SET);
            salva_no_interno(d, noInterno, arq_indice);
            free(noInterno);

            //SALVAR ARQUIVO DE DADOS
            noFolha->pont_prox = metadados->pont_prox_no_folha_livre;
            noFolha->pont_pai = 0;

            fseek(arq_dados, var_busca, SEEK_SET);
            salva_no_folha(d, noFolha, arq_dados);

            novo_noFolha->pont_pai = 0;

            fseek(arq_dados, metadados->pont_prox_no_folha_livre, SEEK_SET);
            salva_no_folha(d, novo_noFolha, arq_dados);

            fclose(arq_dados);
            fclose(arq_indice);

            free(noFolha);
            free(novo_noFolha);

            metadados->pont_raiz = 0;
            metadados->raiz_folha = 0;
            metadados->pont_prox_no_interno_livre = tamanho_no_interno(d);
            metadados->pont_prox_no_folha_livre = metadados->pont_prox_no_folha_livre + tamanho_no_folha(d);

            salva_arq_metadados(nome_arquivo_metadados, metadados);
            free(metadados);

            return ret;
        }
    }
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

            // LÊ O NÓ INTERNO QUE APONTA PARA O PRÓXIMO NÓ PARA ALTERAR O VALOR DA CHAVE
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

                // MOVE AS PIZZAS DO PRÓXIMO NÓ PARA O NÓ NO QUAL OCORREU A REMOÇÃO
                int cont = noFolhaProx->m;
                for(int i = 0; cont > 0; i++) {
                    noFolha->pizzas[noFolha->m] = noFolhaProx->pizzas[i];
                    noFolha->m++;
//                    noFolhaProx->m--;
                    cont--;
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

                // CHECA SE É PRECISO PROPAGAR A CONCATENAÇÃO
                if(noInterno->pont_pai != -1 && noInterno->m < d) {

                    // LÊ O PAI DO NÓ INTERNO
                    fseek(arq_indice, noInterno->pont_pai, SEEK_SET);
                    TNoInterno* noInternoPai = le_no_interno(d, arq_indice);

                    // BUSCA O PONTEIRO PARA O NÓ INTERNO QUE PRECISA SER CONCATENADO
                    int controle = 0;
                    for(int i = 0; i < noInternoPai->m; i++) {
                        if(noInternoPai->p[i] == noFolha->pont_pai)
                            break;
                        controle++;
                    }

                    // LÊ O PRÓXIMO NÓ INTERNO
                    fseek(arq_indice, noInternoPai->p[controle+1], SEEK_SET);
                    TNoInterno* noInternoProx = le_no_interno(d, arq_indice);

                    // CHECA SE É POSSÍVEL FAZER A CONCATENAÇÃO
                    if (noInterno->m + noInternoProx->m < 2*d) {

                        // ATUALIZA O PRIMEIRO ELEMENTO DO NOINTERNOPROX PARA APONTAR PRO NÓ CERTO
                        if(noInternoProx->aponta_folha == 1) {
                            fseek(arq_dados, noInternoProx->p[0], SEEK_SET);
                            TNoFolha* noFolhaAux = le_no_folha(d, arq_dados);

                            noFolhaAux->pont_pai = noFolha->pont_pai;

                            fseek(arq_dados,noInternoProx->p[0], SEEK_SET);
                            salva_no_folha(d, noFolhaAux, arq_dados);
                            libera_no_folha(d, noFolhaAux);
                        }
                        else if (noInternoProx->aponta_folha == 0) {
                            fseek(arq_indice, noInternoProx->p[0], SEEK_SET);
                            TNoInterno* noInternoAux = le_no_interno(d, arq_indice);

                            noInternoAux->pont_pai = noInternoPai->p[controle];

                            fseek(arq_indice,noInternoProx->p[0], SEEK_SET);
                            salva_no_interno(d, noInternoAux, arq_indice);
                            libera_no_interno(noInternoAux);
                        }

                        // PASSA A CHAVE DO NÓ PAI E O PRIMEIRO PONTEIRO DO NOINTERNOPROX PRO NÓ ONDE OCORREU A REMOÇÃO
                        noInterno->p[noInterno->m+1] = noInternoProx->p[0];
                        noInterno->chaves[noInterno->m] = noInternoPai->chaves[controle];
                        noInterno->m++;

                        // ATUALIZA O NÓ PAI
                        noInternoPai->p[controle+1] = -1;
                        noInternoPai->chaves[controle] = -1;
                        noInternoPai->m--;

                        for(int i = 0; i < 2*d - 1; i++) {
                            if (i >= controle) {
                                noInternoPai->chaves[i] = noInternoPai->chaves[i+1];
                                noInternoPai->p[i+1] = noInternoPai->p[i+2];
                            }
                        }

                        // CASO O NÓ PAI ERA RAIZ DA ÁRVORE E ELE AGORA ESTÁ VAZIO
                        if (noInternoPai->m == 0 && noInternoPai->pont_pai == -1) {

                            // LÊ ARQUIVO DE METADADOS PARA ATUALIZAR O NÓ RAIZ
                            TMetadados* meta = le_arq_metadados(nome_arquivo_metadados);
                            meta->pont_raiz = noInternoPai->p[controle];
                            salva_arq_metadados(nome_arquivo_metadados, meta);
                            noInterno->pont_pai = -1;
                        }

                        // PASSA AS CHAVES E SEUS PONTEIROS DO PRÓXIMO NÓ PARA O NÓ INTERNO ATUAL
                        for(int i = 0; i < noInternoProx->m; i++) {

                            // ATUALIZA O PONTEIRO PARA O PAI
                            if(noInternoProx->aponta_folha == 1) {
                                fseek(arq_dados, noInternoProx->p[i+1], SEEK_SET);
                                TNoFolha* noFolhaAux = le_no_folha(d, arq_dados);

                                noFolhaAux->pont_pai = noFolha->pont_pai;

                                fseek(arq_dados,noInternoProx->p[i+1], SEEK_SET);
                                salva_no_folha(d, noFolhaAux, arq_dados);
                                libera_no_folha(d, noFolhaAux);
                            }
                            else if (noInternoProx->aponta_folha == 0) {
                                fseek(arq_indice, noInternoProx->p[i+1], SEEK_SET);
                                TNoInterno* noInternoAux = le_no_interno(d, arq_indice);

                                noInternoAux->pont_pai = noInternoPai->p[controle];

                                fseek(arq_indice,noInternoProx->p[i+1], SEEK_SET);
                                salva_no_interno(d, noInternoAux, arq_indice);
                                libera_no_interno(noInternoAux);
                            }

                            noInterno->p[noInterno->m+1] = noInternoProx->p[i+1];
                            noInterno->chaves[noInterno->m] = noInternoProx->chaves[i];
                            noInterno->m++;
                        }

                    }

                }

                // ATUALIZA O PONTEIRO DO NÓ FOLHA
                noFolha->pont_prox = noFolhaProx->pont_prox;

                // SALVA ALTERAÇÕES NO DISCO
                fseek(arq_dados, var_busca, SEEK_SET);
                salva_no_folha(d, noFolha, arq_dados);

                fseek(arq_indice, noFolha->pont_pai, SEEK_SET);
                salva_no_interno(d, noInterno, arq_indice);

                fclose(arq_dados);
                fclose(arq_indice);
                fclose(arq_metadados);

                // LIBERA NÓS AUXILIARES
                libera_no_folha(d, noFolhaProx);
//                libera_no_folha(d, noFolha);
                libera_no_interno(noInterno);

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