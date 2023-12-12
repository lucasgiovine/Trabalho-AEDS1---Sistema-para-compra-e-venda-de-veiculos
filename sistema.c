#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_STRING_SIZE 50
#define MAX_LINE_SIZE 90000

struct Veiculo {
    float preco;
    int ano;
    char marca[MAX_STRING_SIZE];
    char cor[MAX_STRING_SIZE];
};

// Função para imprimir informações sobre um veículo
void printVeiculo(struct Veiculo veiculo) {
    printf("Preco: %.2f\n", veiculo.preco);
    printf("Ano: %d\n", veiculo.ano);
    printf("Marca: %s\n", veiculo.marca);
}

// Função para obter a data e hora atual formatada
void getCurrentDateTime(char *dateTime) {
    time_t t;
    struct tm *tm_info;

    time(&t);
    tm_info = localtime(&t);

    strftime(dateTime, 20, "%Y-%m-%d %H:%M:%S", tm_info);
}

// Função para escrever uma linha em um arquivo
void escreverArquivo(FILE *arquivo, const char *linha) {
    fprintf(arquivo, "%s", linha);
}

// Função para adicionar uma marca se ela não estiver presente no arquivo de marcas
void adicionarMarcaSeAusente(FILE *marcasFile, const char *marca) {
    fseek(marcasFile, 0, SEEK_SET);
    char line[MAX_LINE_SIZE];

    while (fgets(line, MAX_LINE_SIZE, marcasFile) != NULL) {
        char marcaExistente[MAX_STRING_SIZE];
        sscanf(line, "%[^,]", marcaExistente);

        if (strcmp(marcaExistente, marca) == 0) {
            return; // A marca já existe
        }
    }

    // Se a marca não está presente, adicione com taxa 0.0
    fprintf(marcasFile, "%s,0.0\n", marca);
}

// Função para comprar veículos e atualizar estoque, histórico de compras e marcas
void compraVeiculos(FILE *ofertasFile, FILE *estoqueFile, FILE *historicoComprasFile, FILE *marcasFile) {
    struct Veiculo veiculoCompra;
    char marcaDesejada[MAX_STRING_SIZE];
    int anoDesejado, precoDesejado;
    printf("Preco: ");
    scanf("%d", &precoDesejado);
    printf("Ano: ");
    scanf("%d", &anoDesejado);
    printf("Marca: ");
    scanf("%s", marcaDesejada);

    fseek(ofertasFile, 0, SEEK_SET);
    char line[MAX_LINE_SIZE];
    int veiculoEncontrado = 0;

    while (fgets(line, MAX_LINE_SIZE, ofertasFile) != NULL) {
        sscanf(line, "%f,%d,%[^,]", &veiculoCompra.preco, &veiculoCompra.ano, veiculoCompra.marca);

        if (precoDesejado == veiculoCompra.preco && anoDesejado == veiculoCompra.ano &&
            strcmp(marcaDesejada, veiculoCompra.marca) == 0) {
            // Atualização: removendo a parte da cor, já que não é necessário incluir
            fprintf(estoqueFile, "%.2f,%d,%s\n", veiculoCompra.preco, veiculoCompra.ano, veiculoCompra.marca);
            fprintf(historicoComprasFile, "Compra: %.2f,%d,%s", veiculoCompra.preco, veiculoCompra.ano, veiculoCompra.marca);

            char dateTime[20];
            getCurrentDateTime(dateTime);
            fprintf(historicoComprasFile, "%s\n", dateTime);

            veiculoEncontrado = 1;
            break;
        }
    }

    if (veiculoEncontrado) {
        printf("Veiculo encontrado e registrado em estoque.csv e historico_compras.csv:\n");
        printVeiculo(veiculoCompra);

        // Verificar se a marca já está no arquivo marcas.csv
        fseek(marcasFile, 0, SEEK_SET);
        int marcaEncontrada = 0;
        while (fgets(line, MAX_LINE_SIZE, marcasFile) != NULL) {
            char marca[MAX_STRING_SIZE];
            float taxa;
            sscanf(line, "%[^,],%f", marca, &taxa);

            if (strcmp(marca, veiculoCompra.marca) == 0) {
                marcaEncontrada = 1;
                break;
            }
        }

        if (!marcaEncontrada) {
            adicionarMarcaSeAusente(marcasFile, veiculoCompra.marca);
        }

        fflush(estoqueFile);
        fflush(historicoComprasFile);
        fflush(marcasFile);
    } else {
        printf("Carro nao encontrado!\n");
    }
}

// Função para vender veículos e atualizar estoque, histórico de vendas e marcas
void vendeVeiculos(FILE *estoqueFile, FILE *historicoVendasFile, FILE *marcasFile) {
    struct Veiculo veiculoVenda;
    printf("Informe os detalhes do veiculo a ser vendido:\n");
    printf("Preco: ");
    scanf("%f", &veiculoVenda.preco);
    printf("Ano: ");
    scanf("%d", &veiculoVenda.ano);
    printf("Marca: ");
    scanf("%s", veiculoVenda.marca);

    fseek(estoqueFile, 0, SEEK_SET);
    char line[MAX_LINE_SIZE];
    int veiculoEncontrado = 0;

    FILE *tempFile = fopen("temp.csv", "w");
    if (tempFile == NULL) {
        perror("Erro ao criar arquivo temporário");
        return;
    }

    while (fgets(line, MAX_LINE_SIZE, estoqueFile) != NULL) {
        float preco;
        int ano;
        char marca[MAX_STRING_SIZE];
        char cor[MAX_STRING_SIZE];

        sscanf(line, "%f,%d,%[^,],%s", &preco, &ano, marca, cor);

        if (preco == veiculoVenda.preco && ano == veiculoVenda.ano &&
            strcmp(marca, veiculoVenda.marca) == 0) {
            veiculoEncontrado = 1;

            fprintf(historicoVendasFile, "Venda: %.2f,%d,%s,%s,", preco, ano, marca, cor);

            char dateTime[20];
            getCurrentDateTime(dateTime);
            fprintf(historicoVendasFile, "%s\n", dateTime);

            printf("Veiculo encontrado e vendido:\n");
            printVeiculo(veiculoVenda);
        } else {
            fprintf(tempFile, "%.2f,%d,%s,%s\n", preco, ano, marca, cor);
        }
    }

    fclose(tempFile);

    fclose(estoqueFile);
    estoqueFile = fopen("veiculos_estoque.csv", "w");
    if (estoqueFile == NULL) {
        perror("Erro ao abrir o arquivo de estoque");
        return;
    }

    tempFile = fopen("temp.csv", "r");
    if (tempFile == NULL) {
        perror("Erro ao abrir o arquivo temporário");
        fclose(estoqueFile);
        return;
    }

    while (fgets(line, MAX_LINE_SIZE, tempFile) != NULL) {
        escreverArquivo(estoqueFile, line);
    }

    fclose(tempFile);

    // Verificar se não há mais veículos da marca no estoque
    fseek(estoqueFile, 0, SEEK_SET);
    int marcaPresente = 0;

    while (fgets(line, MAX_LINE_SIZE, estoqueFile) != NULL) {
        char marca[MAX_STRING_SIZE];
        sscanf(line, "%*[^,],%*d,%[^,]", marca);

        if (strcmp(marca, veiculoVenda.marca) == 0) {
            marcaPresente = 1;
            break;
        }
    }

    if (!marcaPresente) {
        // Se a marca não está presente no estoque, remova do arquivo marcas.csv
        // (A função adicionarMarcaSeAusente já trata essa situação, então não há necessidade de remover)
    }

    fflush(estoqueFile);
    fflush(historicoVendasFile);
    fflush(marcasFile);
}

int main() {
    FILE *ofertasFile = fopen("veiculos_ofertas.csv", "r");
    FILE *estoqueFile = fopen("estoque.csv", "a+");
    FILE *historicoComprasFile = fopen("historico_compras.csv", "a");
    FILE *historicoVendasFile = fopen("historico_vendas.csv", "a");
    FILE *marcasFile = fopen("marcas.csv", "a+");

    if (ofertasFile == NULL || estoqueFile == NULL || historicoComprasFile == NULL || historicoVendasFile == NULL || marcasFile == NULL) {
        perror("Erro ao abrir o arquivo");
        return 1;
    }

    int opcao;
    do {
        printf("\nMenu:\n");
        printf("1 - Compra de veiculos para estoque\n");
        printf("2 - Venda de veiculos\n");
        printf("3 - Sair\n");
        printf("Escolha uma opcao: ");
        scanf("%d", &opcao);

        switch (opcao) {
            case 1:
                compraVeiculos(ofertasFile, estoqueFile, historicoComprasFile, marcasFile);
                break;
            case 2:
                vendeVeiculos(estoqueFile, historicoVendasFile, marcasFile);
                break;
            case 3:
                break;
            default:
                printf("Opcao invalida. Tente novamente.\n");
        }
    } while (opcao != 3);

    fclose(ofertasFile);
    fclose(estoqueFile);
    fclose(historicoComprasFile);
    fclose(historicoVendasFile);
    fclose(marcasFile);

    return 0;
}