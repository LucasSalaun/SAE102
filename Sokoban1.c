#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<stdbool.h>
#include<termios.h>
#include<unistd.h>
#include<fcntl.h>

#define TAILLE 12

typedef int t_plateau[TAILLE][TAILLE];

int kbhit();
void charger_partie(t_plateau plateau, char fichier[]);
void enregistrer_partie(t_plateau plateau, char fichier[]);
void afficher_plateau(t_plateau plateau);
void afficher_entete(char partie[20], int nb_deplacement);
void deplacer(t_plateau plateau, char touche, int ligne_sokoban, int colonne_sokoban, int *nb_deplacement);
bool gagne(t_plateau plateau);
void trouver_sokoban(t_plateau plateau, int *ligne, int *colonne);
void chargerDeplacements(typeDeplacements t, char fichier[], int * nb);

int main(){
    char partie[20];
    t_plateau plateau;
    int nb_deplacement = 0;
    printf("Entrez le nom du fichier (.sok) : ");

    scanf("%s", partie);
    charger_partie(plateau, partie);
    afficher_entete(partie, nb_deplacement);
    afficher_plateau(plateau);
    char touche = '\0';
    bool gagner = false;
    while (gagner == false){
        touche = '\0';
        if (kbhit()){
            touche = getchar();
        }
        if (touche == 'q' || touche == 'z' ||  touche == 's' || touche == 'd'){
            int ligne, colonne;
            trouver_sokoban(plateau, &ligne, &colonne);
            deplacer(plateau, touche, ligne, colonne, &nb_deplacement);
            afficher_entete(partie, nb_deplacement);
            afficher_plateau(plateau);
            gagner=gagne(plateau);
        }
    }
    if (gagner == true){
        printf("Bravo, partie terminée.");
        printf("Déplacements effectués : %d\n", nb_deplacement);
    } 
    return EXIT_SUCCESS;
}

void deplacer(t_plateau plateau, char touche, int ligne_sokoban, int colonne_sokoban, int *nb_deplacement){
    int bouge_en_ligne = 0, bouge_en_colonne = 0;
    if (touche == 'q') {
        bouge_en_colonne = -1;
    }else if (touche == 'd'){
        bouge_en_colonne = 1;
    }else if (touche == 'z'){
        bouge_en_ligne = -1;
    }else if (touche == 's') {
    bouge_en_ligne = 1;
    }else{
        return; //retourne rien si une mauvaise touche est appuyé
    }    
    int nouvelle_ligne = ligne_sokoban + bouge_en_ligne;
    int nouvelle_colonne = colonne_sokoban + bouge_en_colonne;
    char destination = plateau[nouvelle_ligne][nouvelle_colonne]; //la case où on souhaite aller
    char position_actuelle = plateau[ligne_sokoban][colonne_sokoban]; //la case où on est
    if (destination == '#'){
        return;
    }
    if (destination == ' ' || destination == '.'){
        if (position_actuelle == '@'){
            plateau[ligne_sokoban][colonne_sokoban] = ' ';
        } else {
            plateau[ligne_sokoban][colonne_sokoban] = '.';
        }
        
        if (destination == ' '){
            plateau[nouvelle_ligne][nouvelle_colonne] = '@';
        } else {
            plateau[nouvelle_ligne][nouvelle_colonne] = '+';
        }
        (*nb_deplacement)++;
    }
    else if (destination == '$' || destination == '*'){
        int ligne_apres = nouvelle_ligne + bouge_en_ligne;
        int colonne_apres = nouvelle_colonne + bouge_en_colonne;
        char case_apres = plateau[ligne_apres][colonne_apres];
        if (case_apres == ' ' || case_apres == '.'){
            if (case_apres == ' '){
                plateau[ligne_apres][colonne_apres] = '$';
            } else {
                plateau[ligne_apres][colonne_apres] = '*';
            }
            
            if (destination == '$'){
                plateau[nouvelle_ligne][nouvelle_colonne] = '@';
            } else {
                plateau[nouvelle_ligne][nouvelle_colonne] = '+';
            }
            
            if (position_actuelle == '@'){
                plateau[ligne_sokoban][colonne_sokoban] = ' ';
            } else {
                plateau[ligne_sokoban][colonne_sokoban] = '.';
            }
            (*nb_deplacement)++;
        }
    }
}

bool gagne(t_plateau plateau){
    for (int i = 0; i < TAILLE; i++)
    {
        for (int j = 0; j < TAILLE; j++)
        {
            if (plateau[i][j] == '.')
                return false;
            if (plateau[i][j] == '+')
                return false;
        }
    }
    return true;
}


void trouver_sokoban(t_plateau plateau, int *ligne, int *colonne){
    for (int i = 0; i < TAILLE; i++){
        for (int j = 0; j < TAILLE; j++){
            if (plateau[i][j] == '@' || plateau[i][j] == '+'){
                *ligne = i;
                *colonne = j;
                return;
            }
        }
    }
}

void afficher_entete(char partie[20], int nb_deplacement){
    system("clear");
    printf("-------------------------------------------------------\n");
    printf("Partie : %s \n", partie);
    printf("Déplacements : %d\n", nb_deplacement);
    printf("-------------------------------------------------------\n");
}

void afficher_plateau(t_plateau plateau){
    for (int i = 0; i < TAILLE; i++){
        for (int j = 0; j < TAILLE; j++){
            char c = plateau[i][j];
            if (c == '*') c = '$'; //on ne doit pas à l'affichage différencier les caisses des caisses sur objectif etc...
            if (c == '+') c = '@';
            printf("%c", c);
        }
        printf("\n");
    }
}

void charger_partie(t_plateau plateau, char fichier[]){
    FILE *f;
    char finDeLigne;
    f = fopen(fichier, "r");
    if (f == NULL){
        printf("ERREUR : Impossible d'ouvrir le fichier\n");
        exit(EXIT_FAILURE);
    } else {
        for (int ligne = 0; ligne < TAILLE; ligne++){
            for (int colonne = 0; colonne < TAILLE; colonne++){
                fread(&plateau[ligne][colonne], sizeof(char), 1, f);
            }
            fread(&finDeLigne, sizeof(char), 1, f);
        }
        fclose(f);
    }
}

int kbhit(){
    int unCaractere = 0;
    struct termios oldt, newt;
    int ch;
    int oldf;
    
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
 
    ch = getchar();
    
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);
 
    if (ch != EOF){
        ungetc(ch, stdin);
        unCaractere = 1;
    } 
    return unCaractere;
}

void enregistrer_partie(t_plateau plateau, char fichier[]){
    FILE * f;
    char finDeLigne = '\n';
    f = fopen(fichier, "w");
    for (int ligne =0; ligne < TAILLE; ligne++){
        for (int colonne = 0; colonne < TAILLE; colonne++){
            fwrite(&plateau[ligne][colonne], sizeof(char), 1, f);
        }
        fwrite(&finDeLigne, sizeof(char), 1, f);
    }
    fclose(f);
}

void chargerDeplacements(typeDeplacements t, char fichier[], int * nb){
    FILE * f;
    char dep;
    *nb = 0;

    f = fopen(fichier, "r");
    if (f==NULL){
        printf("FICHIER NON TROUVE\n");
    } else {
        fread(&dep, sizeof(char), 1, f);
        if (feof(f)){
            printf("FICHIER VIDE\n");
        } else {
            while (!feof(f)){
                t[*nb] = dep;
                (*nb)++;
                fread(&dep, sizeof(char), 1, f);
            }
        }
    }
    fclose(f);
}