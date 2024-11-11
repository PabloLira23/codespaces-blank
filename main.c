#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <locale.h>

#define USERNAME_LENGTH 50
#define PASSWORD_LENGTH 80
#define MAX_USERS 100

typedef struct {
    char username[USERNAME_LENGTH];
    char password[PASSWORD_LENGTH];
} User;

User users[MAX_USERS];
int userCount = 0;
int generatedCode = 0; // Armazena o código gerado para validação
bool isTwoFactorVerified = false; // Controle de verificação de dois fatores

// Função de criptografia da senha
void criptografia(char string[]) {
    time_t agora;
    struct tm *relogio;
    char relogioString[9];

    agora = time(NULL);
    relogio = localtime(&agora);
    strftime(relogioString, sizeof(relogioString), "%H%M%S", relogio);

    char hor[3], min[3], seg[3];
    strncpy(hor, relogioString, 2); hor[2] = '\0';
    strncpy(min, relogioString + 2, 2); min[2] = '\0';
    strncpy(seg, relogioString + 4, 2); seg[2] = '\0';

    int ascii;
    for (int i = 0; i < strlen(string); i++) {
        ascii = (int)string[i];
        string[i] = (char)(ascii + atoi(hor) - atoi(min) + atoi(seg));
    }

    int horInt = atoi(hor), minInt = atoi(min), segInt = atoi(seg);
    string[strlen(string)] = (char)(horInt + 30 + strlen(string));
    string[strlen(string) + 1] = (char)(minInt + 30 + (strlen(string) - 1));
    string[strlen(string) + 2] = (char)(segInt + 30 + (strlen(string) - 2));
    string[strlen(string) + 3] = '\0';
}

// Função de descriptografia da senha
void descriptografia(char string[]) {
    int ascii, descSeg, descMin, descHor;
    char descriptChar;

    char asciiSeg = string[strlen(string) - 1];
    descSeg = (int)asciiSeg - 30 - (strlen(string) - 3);
    string[strlen(string) - 1] = '\0';

    char asciiMin = string[strlen(string) - 1];
    descMin = (int)asciiMin - 30 - (strlen(string) - 2);
    string[strlen(string) - 1] = '\0';

    char asciiHor = string[strlen(string) - 1];
    descHor = (int)asciiHor - 30 - (strlen(string) - 1);
    string[strlen(string) - 1] = '\0';

    for (int i = 0; i < strlen(string); i++) {
        ascii = (int)string[i];
        descriptChar = ascii - descHor + descMin - descSeg;
        string[i] = (char)descriptChar;
    }
}

// Função para salvar os usuários no arquivo
void saveToFile() {
    FILE *file = fopen("bancoDados.txt", "w");
    if (file == NULL) {
        printf("Erro ao abrir o arquivo!\n");
        return;
    }

    for (int i = 0; i < userCount; i++) {
        fprintf(file, "%s,%s\n", users[i].username, users[i].password);
    }

    fclose(file);
}

// Função para carregar os usuários do arquivo
void loadFromFile() {
    FILE *file = fopen("bancoDados.txt", "r");
    if (file == NULL) {
        printf("Arquivo não encontrado. Um novo arquivo será criado ao salvar dados.\n");
        return;
    }

    userCount = 0;
    while (fscanf(file, "%[^,],%[^\n]\n", users[userCount].username, users[userCount].password) != EOF) {
        userCount++;
    }

    fclose(file);
}

// Função para criar um novo usuário
void createUser() {
    if (userCount >= MAX_USERS) {
        printf("Limite de usuários atingido.\n");
        return;
    }

    char username[USERNAME_LENGTH], password[PASSWORD_LENGTH];
    printf("Digite o nome de usuário: ");
    scanf("%s", username);

    printf("Digite a senha: ");
    scanf("%s", password);

    criptografia(password);

    strcpy(users[userCount].username, username);
    strcpy(users[userCount].password, password);
    userCount++;

    saveToFile();
    printf("Usuário criado com sucesso!\n");
}

// Função para listar todos os usuários com senha criptografada
void listUsers() {
    printf("Lista de usuários:\n");
    for (int i = 0; i < userCount; i++) {
        printf("Usuário: %s | Senha Criptografada: %s\n", users[i].username, users[i].password);
    }
}

// Função para listar todos os usuários com a senha descriptografada
void listDecryptedUsers() {
    printf("\nLista de usuários com senha descriptografada:\n");
    for (int i = 0; i < userCount; i++) {
        char decryptedPassword[PASSWORD_LENGTH];
        strcpy(decryptedPassword, users[i].password); // Copia a senha criptografada
        descriptografia(decryptedPassword); // Descriptografa a cópia
        printf("Usuário: %s | Senha Descriptografada: %s\n", users[i].username, decryptedPassword); // Exibe a senha correta
    }
}

// Função para atualizar a senha de um usuário existente
void updateUser() {
    char username[USERNAME_LENGTH], password[PASSWORD_LENGTH];
    printf("Digite o nome de usuário para atualizar a senha: ");
    scanf("%s", username);

    int found = -1;
    for (int i = 0; i < userCount; i++) {
        if (strcmp(users[i].username, username) == 0) {
            found = i;
            break;
        }
    }

    if (found != -1) {
        printf("Digite a nova senha: ");
        scanf("%s", password);
        criptografia(password);
        strcpy(users[found].password, password);
        saveToFile();
        printf("Senha atualizada com sucesso!\n");
    } else {
        printf("Usuário não encontrado.\n");
    }
}

// Função para excluir um usuário
void deleteUser() {
    char username[USERNAME_LENGTH];
    printf("Digite o nome de usuário para excluir: ");
    scanf("%s", username);

    int found = -1;
    for (int i = 0; i < userCount; i++) {
        if (strcmp(users[i].username, username) == 0) {
            found = i;
            break;
        }
    }

    if (found != -1) {
        for (int i = found; i < userCount - 1; i++) {
            users[i] = users[i + 1];
        }
        userCount--;
        saveToFile();
        printf("Usuário excluído com sucesso!\n");
    } else {
        printf("Usuário não encontrado.\n");
    }
}

// Funcao para gerar o codigo de verificacao de dois fatores
int generateTwoFactorCode() {
    srand(time(0));
    return rand() % 900000 + 100000; // Gera um numero entre 100000 e 999999
}

// Funcao para mostrar o codigo ao usuario (Opcao 5)
void showTwoFactorCode() {
    generatedCode = generateTwoFactorCode();
    printf("Seu codigo de verificacao de dois fatores e: %d\n", generatedCode);
}

// Menu principal
int main() {
    setlocale(LC_ALL,"");
    loadFromFile();
    int choice;
    int userInputCode;

    do {
        printf("\nMenu:\n");
        printf("1. Criar usuario\n");
        printf("2. Listar usuarios\n");
        printf("3. Atualizar senha do usuario\n");
        printf("4. Excluir usuario\n");
        printf("5. Gerar codigo de verificacao em dois fatores\n");

        // Mostra a opcao 6 apenas se a verificacao em dois fatores foi realizada
        if (isTwoFactorVerified) {
            printf("6. Listar usuarios descriptografados\n");
        }

        printf("0. Sair\n");
        printf("Escolha uma opcao ou insira o codigo de verificacao: ");
        scanf("%d", &choice);

        // Verifica se o usuario digitou o codigo de dois fatores
        if (choice == generatedCode && generatedCode != 0) {
            printf("Codigo correto! Opcao 6 desbloqueada.\n");
            isTwoFactorVerified = true;
            generatedCode = 0; // Reseta o codigo para evitar reuso
            continue; // Volta ao menu para mostrar a opcao 6 desbloqueada
        }

        switch (choice) {
            case 1:
                createUser();
                break;
            case 2:
                listUsers();
                break;
            case 3:
                updateUser();
                break;
            case 4:
                deleteUser();
                break;
            case 5:
                showTwoFactorCode();
                break;
            case 6:
                if (isTwoFactorVerified) {
                    listDecryptedUsers();
                } else {
                    printf("Opcao invalida. Tente novamente.\n");
                }
                break;
            case 0:
                printf("Saindo...\n");
                break;
            default:
                printf("Opcao invalida. Tente novamente.\n");
                break;
        }
    } while (choice != 0);

    return 0;
}
