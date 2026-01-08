#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<stdbool.h>
#include<termios.h>
#include<unistd.h>
#include<fcntl.h>

#define TAILLE 12
#define NBDEP 500

typedef char typePlateau[TAILLE][TAILLE];
typedef char typeDeplacements[NBDEP];

int kbhit();
void chargerPartie(typePlateau plateau, char fichier[]);
void enregistrer_partie(typePlateau plateau, char fichier[]);
void afficher_plateau(typePlateau plateau);
void afficher_entete(char partie[20], int nbDeplacement);
void deplacer(typePlateau plateau, char touche, int ligneSokoban, int colonneSokoban, int *nbDeplacement, bool *inutile);
bool gagne(typePlateau plateau);
void trouver_sokoban(typePlateau plateau, int *ligne, int *colonne);
void chargerDeplacements(typeDeplacements t, char fichier[], int * nb);
void pas_jouee(typeDeplacements dep,typeDeplacements opti, int *i, bool *inutile, int * compteur);
pas_jouee(dep[i],&ii, &inutile, &compteurOpti);


int main(){
    char partie[20];
    char deplacement[20];
    typePlateau plateau;
    typeDeplacements dep;
    typeDeplacements opti;
    int nbDeplacement = 0, nbLettre = 0;
    bool inutile = false;
    printf("Entrez le nom du niveau (.sok) : ");
    scanf("%s", partie);
    printf("Entrez le nom du fichier de deplacement (.dep) : ");
    scanf("%s", deplacement);
    chargerPartie(plateau, partie);
    chargerDeplacements(dep,deplacement,&nbLettre);
    afficher_entete(partie, nbDeplacement);
    afficher_plateau(plateau);
    char touche = '\0';
    bool gagner = false;
    int i =0;
    int ligne = 0, colonne = 0;
    int ii = 0
    int compteurOpti = 0;
    trouver_sokoban(plateau, &ligne, &colonne);
    while ( nbLettre != i){
        usleep(200000);
        touche = dep[i];        
        trouver_sokoban(plateau, &ligne, &colonne);
        deplacer(plateau, touche, ligne, colonne, &nbDeplacement, &inutile);
        pas_jouee(dep[i],opti[],&ii, &inutile, &compteurOpti);
        afficher_entete(partie, nbDeplacement);
        afficher_plateau(plateau);
        i++;
    }
        gagner=gagne(plateau);
    if (gagner == true){
        printf("La suite de déplacements %s est bien une solution pour la partie %s. Elle contient %d déplacements.\n",deplacement,partie,nbDeplacement);
        char fichier[30];
        printf("Rentrer un nom de fichier(.dep)");
        scanf("%s",fichier);
        enregistrer_partie(typePlateau opti, char fichier[], int compteurOpti);
    }
    else{
        printf("La suite de déplacements %s N’EST PAS une solution pour la partie %s.\n", deplacement,partie);
    }
    return EXIT_SUCCESS;
}

void deplacer(typePlateau plateau, char touche, int ligneSokoban, int colonneSokoban, int *nbDeplacement){
    int bougeEnLigne = 0, bougeEnColonne = 0;
    if (touche == 'g' || touche == 'G') {
        bougeEnColonne = -1;
    }else if (touche == 'd' || touche == 'D'){
        bougeEnColonne = 1;
    }else if (touche == 'h' || touche == 'H'){
        bougeEnLigne = -1;
    }else if (touche == 'b' || touche == 'B') {
    bougeEnLigne = 1;
    }else{
        return; //retourne rien si une mauvaise touche est appuyé
    }
    int maj = -1;
    if (touche == 'B' || touche == 'H' || touche == 'G' || touche == 'D'){
        maj = 1;
    }
    int nouvelleLigne = ligneSokoban + bougeEnLigne;
    int nouvelleColonne = colonneSokoban + bougeEnColonne;
    char destination = plateau[nouvelleLigne][nouvelleColonne]; //la case où on souhaite aller
    char positionActuelle = plateau[ligneSokoban][colonneSokoban]; //la case où on est
    if (destination == '#'){
        *inutile = true;
        return;
    }
    if ((destination == ' ' && maj != 1) || (destination == '.'  && maj != 1)){
        if (positionActuelle == '@'){
            plateau[ligneSokoban][colonneSokoban] = ' ';
        } else {
            plateau[ligneSokoban][colonneSokoban] = '.';
        }
        
        if ((destination == ' ' && maj != 1)){
            plateau[nouvelleLigne][nouvelleColonne] = '@';
        } else {
            plateau[nouvelleLigne][nouvelleColonne] = '+';
        }
        (*nbDeplacement)++;
    }
    else if ((destination == '$'  && maj == 1) || (destination == '*'  && maj == 1)){
        int ligneApres = nouvelleLigne + bougeEnLigne;
        int colonneApres = nouvelleColonne + bougeEnColonne;
        char caseApres = plateau[ligneApres][colonneApres];
        if (caseApres == ' ' || caseApres == '.'){
            if (caseApres == ' '){
                plateau[ligneApres][colonneApres] = '$';
            } else {
                plateau[ligneApres][colonneApres] = '*';
            }
            
            if (destination == '$'){
                plateau[nouvelleLigne][nouvelleColonne] = '@';
            } else {
                plateau[nouvelleLigne][nouvelleColonne] = '+';
            }
            
            if (positionActuelle == '@'){
                plateau[ligneSokoban][colonneSokoban] = ' ';
            } else {
                plateau[ligneSokoban][colonneSokoban] = '.';
            }
            (*nbDeplacement)++;
        }
    }
    else{
        *inutile = true;
        return;
    }
}

void pas_jouee(char dep[],char opti[], ,bool *inutile, int * compteurOpti, int i){
    if (*inutile==flase){
        opti[*compteurOpti]=dep[];
        *inutile = false;
        *compteurOpti++;
    }
    
}

bool gagne(typePlateau plateau){
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


void trouver_sokoban(typePlateau plateau, int *ligne, int *colonne){
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

void afficher_entete(char partie[20], int nbDeplacement){
    system("clear");
    printf("-------------------------------------------------------\n");
    printf("Partie : %s \n", partie);
    printf("Déplacements : %d\n", nbDeplacement);
    printf("-------------------------------------------------------\n");
}

void afficher_plateau(typePlateau plateau){
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

void pas_jouee(char dep[], int i, bool inutile, int * compteur){
    if(inutile==true){
        dep[i]="";
    }
    else{
        (*compteur++);
    }
}

void chargerPartie(typePlateau plateau, char fichier[]){
    FILE * f;
    char finDeLigne;

    f = fopen(fichier, "r");
    if (f==NULL){
        printf("ERREUR SUR FICHIER");
        exit(EXIT_FAILURE);
    } else {
        for (int ligne=0 ; ligne<TAILLE ; ligne++){
            for (int colonne=0 ; colonne<TAILLE ; colonne++){
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

void enregistrer_partie(typeDeplacements opti, char fichier[], int compteurOpti){
    FILE * f;
    f = fopen(fichier, "w");
    for (int ligne =0; ligne < NBDEP; ligne++){
        fwrite(&opti[ligne], sizeof(char), 1, f);
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
