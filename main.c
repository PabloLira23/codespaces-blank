#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <locale.h>

#define USERNAME_LENGTH 50
#define PASSWORD_LENGTH 80
#define MAX_USERS 100

#define RESET "\033[0m"
#define BOLD "\033[1m"
#define CYAN "\033[36m"
#define YELLOW "\033[33m"

typedef struct {
    char username[USERNAME_LENGTH];
    char password[PASSWORD_LENGTH];
} User;

User users[MAX_USERS];
int userCount = 0;
int generatedCode = 0;
bool isTwoFactorVerified = false;

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

    int horInt = atoi(hor), minInt = atoi(min), segInt = atoi(seg);

    for (int i = 0; i < strlen(string); i++) {
        int ascii = (int)string[i];
        string[i] = (char)(ascii + horInt - minInt + segInt);
    }

    int len = strlen(string);
    string[len] = (char)(horInt + 30);
    string[len + 1] = (char)(minInt + 30);
    string[len + 2] = (char)(segInt + 30);
    string[len + 3] = '\0';
}

void descriptografia(char string[]) {
    int len = strlen(string);

    int descSeg = (int)string[len - 1] - 30;
    string[len - 1] = '\0';

    int descMin = (int)string[len - 2] - 30;
    string[len - 2] = '\0';

    int descHor = (int)string[len - 3] - 30;
    string[len - 3] = '\0';

    for (int i = 0; i < strlen(string); i++) {
        int ascii = (int)string[i];
        string[i] = (char)(ascii - descHor + descMin - descSeg);
    }
}

void saveToFile() {
    FILE *file = fopen("bancoDados.txt", "w");
    if (file == NULL) {
        printf("erro ao abrir o arquivo!\n");
        return;
    }

    for (int i = 0; i < userCount; i++) {
        fprintf(file, "%s,%s\n", users[i].username, users[i].password);
    }

    fclose(file);
}

void loadFromFile() {
    FILE *file = fopen("bancoDados.txt", "r");
    if (file == NULL) {
        printf("arquivo nao encontrado. um novo arquivo sera criado ao salvar dados.\n");
        return;
    }

    userCount = 0;
    while (fscanf(file, "%[^,],%[^\n]\n", users[userCount].username, users[userCount].password) != EOF) {
        userCount++;
    }

    fclose(file);
}

void createUser() {
    if (userCount >= MAX_USERS) {
        printf("limite de usuarios atingido.\n");
        return;
    }

    char username[USERNAME_LENGTH], password[PASSWORD_LENGTH];
    printf("digite o nome de usuario: ");
    scanf("%s", username);

    printf("digite a senha: ");
    scanf("%s", password);

    criptografia(password);

    strcpy(users[userCount].username, username);
    strcpy(users[userCount].password, password);
    userCount++;

    saveToFile();
    printf("usuario criado com sucesso!\n");
}

void listUsers(bool decrypted) {
    printf("\nlista de usuarios:\n");
    for (int i = 0; i < userCount; i++) {
        char displayedPassword[PASSWORD_LENGTH];
        strcpy(displayedPassword, users[i].password);
        if (decrypted) {
            descriptografia(displayedPassword);
        }
        printf("usuario: %s | senha: %s\n", users[i].username, displayedPassword);
    }
}

void updateUser() {
    char username[USERNAME_LENGTH], password[PASSWORD_LENGTH];
    printf("digite o nome de usuario para atualizar a senha: ");
    scanf("%s", username);

    int found = -1;
    for (int i = 0; i < userCount; i++) {
        if (strcmp(users[i].username, username) == 0) {
            found = i;
            break;
        }
    }

    if (found != -1) {
        printf("digite a nova senha: ");
        scanf("%s", password);
        criptografia(password);
        strcpy(users[found].password, password);
        saveToFile();
        printf("senha atualizada com sucesso!\n");
    } else {
        printf("usuario nao encontrado.\n");
    }
}

void deleteUser() {
    char username[USERNAME_LENGTH];
    printf("digite o nome de usuario para excluir: ");
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
        printf("usuario excluido com sucesso!\n");
    } else {
        printf("usuario nao encontrado.\n");
    }
}

int generateTwoFactorCode() {
    srand(time(0));
    return rand() % 900000 + 100000;
}

void showTwoFactorCode() {
    generatedCode = generateTwoFactorCode();
    printf("seu codigo de verificacao de dois fatores e: %d\n", generatedCode);
}

void showMenu() {
    printf("\n%s%s=========================================%s\n", BOLD, CYAN, RESET);
    printf("%s%s       gerenciador de usuarios       %s\n", BOLD, CYAN, RESET);
    printf("%s%s=========================================%s\n", BOLD, CYAN, RESET);
    printf("%s%s [1]%s criar usuario\n", BOLD, YELLOW, RESET);
    printf("%s%s [2]%s listar usuarios\n", BOLD, YELLOW, RESET);
    printf("%s%s [3]%s atualizar senha do usuario\n", BOLD, YELLOW, RESET);
    printf("%s%s [4]%s excluir usuario\n", BOLD, YELLOW, RESET);
    printf("%s%s [5]%s gerar codigo de verificacao em dois fatores\n", BOLD, YELLOW, RESET);
    printf("%s%s [0]%s sair\n", BOLD, YELLOW, RESET);
    printf("%s%s=========================================%s\n", BOLD, CYAN, RESET);
    printf("%sescolha uma opcao ou insira o codigo de verificacao:%s ", BOLD, RESET);
}

int main() {
    setlocale(LC_ALL,"");
    loadFromFile();
    int choice;

    do {
        showMenu();
        scanf("%d", &choice);

        if (choice == generatedCode && generatedCode != 0) {
            printf("\n%s** codigo correto! senhas serao exibidas descriptografadas. **%s\n", BOLD, RESET);
            isTwoFactorVerified = true;
            generatedCode = 0;
            continue;
        }

        switch (choice) {
            case 1:
                createUser();
                break;
            case 2:
                listUsers(isTwoFactorVerified);
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
            case 0:
                printf("\n%s** saindo do programa... obrigado! **%s\n", BOLD, RESET);
                break;
            default:
                printf("\n%s** opcao invalida. tente novamente! **%s\n", BOLD, RESET);
                break;
        }
    } while (choice != 0);

    return 0;
}
