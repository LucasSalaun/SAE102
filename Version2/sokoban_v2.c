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

typedef struct {
    int ligne;
    int colonne;
    typePlateau plateau;
} typeEtat;

typedef typeEtat typeHistorique[NBDEP];

int kbhit();
void chargerPartie(typePlateau plateau, char fichier[]);
void enregistrerPartie(typeDeplacements opti, char fichier[], int compteurOpti);
void afficherPlateau(typePlateau plateau);
void afficherEntete(char partie[20], int nbDeplacement);
void deplacer(typePlateau plateau, char touche, int ligneSokoban,
     int colonneSokoban, int *nbDeplacement, bool *inutile);
bool gagne(typePlateau plateau);
void trouverSokoban(typePlateau plateau, int *ligne, int *colonne);
void chargerDeplacements(typeDeplacements t, char fichier[], int * nb);
void copierPlateau(typePlateau source, typePlateau destination);
bool plateauxIdentiques(typePlateau p1, typePlateau p2);
int optimiserSequence(typePlateau plateauInitial, typeDeplacements dep, int nbLettre, typeDeplacements opti);

int main(){
    char partie[20];
    char deplacement[20];
    typePlateau plateau;
    typePlateau plateauInitial;
    typeDeplacements dep;
    typeDeplacements opti;
    int nbDeplacement = 0, nbLettre = 0;
    
    printf("Entrez le nom du niveau (.sok) : ");
    scanf("%s", partie);
    printf("Entrez le nom du fichier de deplacement (.dep) : ");
    scanf("%s", deplacement);
    
    chargerPartie(plateau, partie);
    copierPlateau(plateau, plateauInitial);
    chargerDeplacements(dep, deplacement, &nbLettre);
    afficherEntete(partie, nbDeplacement);
    afficherPlateau(plateau);
    
    char touche = '\0';
    bool gagner = false;
    int i = 0;
    int ligne = 0, colonne = 0;
    bool inutile = false;
    
    while (nbLettre != i){
        usleep(20000);
        touche = dep[i];
        trouverSokoban(plateau, &ligne, &colonne);
        deplacer(plateau, touche, ligne, colonne, &nbDeplacement, &inutile);
        afficherEntete(partie, nbDeplacement);
        afficherPlateau(plateau);
        i++;
    }
    
    gagner = gagne(plateau);
    if (gagner == true){
        int compteurOpti = optimiserSequence(plateauInitial, dep, nbLettre, opti);
        
        printf("La suite de déplacements %s est bien une solution pour la partie %s.\nElle contient initialement %d caractères.\n", deplacement, partie, nbLettre);
        printf("Après optimisation elle contient %d caractères. Souhaitez-vous l'enregistrer (o/n) ? ", compteurOpti);
        char choix;
        scanf(" %c", &choix);
        if (choix == 'o'){
            char fichier[30];
            printf("Rentrer un nom de fichier(.dep) : ");
            scanf("%s", fichier);
            enregistrerPartie(opti, fichier, compteurOpti);
        }
    } else {
        printf("La suite de déplacements %s N'EST PAS une solution pour la partie %s.\n", deplacement, partie);
    }
    return EXIT_SUCCESS;
}

int optimiserSequence(typePlateau plateauInitial, typeDeplacements dep, int nbLettre, typeDeplacements opti){
    typeDeplacements temp;
    typePlateau plateau;
    typeHistorique historique;
    bool inutile = false;
    int nbDeplacement = 0;
    int compteurOpti = 0;
    int ancienneOptimisation = -1;
    
    for (int i = 0; i < nbLettre; i++){
        temp[i] = dep[i];
    }
    int nbTemp = nbLettre;
    
    while (ancienneOptimisation != compteurOpti){
        ancienneOptimisation = compteurOpti;
        compteurOpti = 0;
        int compteurHistorique = 0;
        
        copierPlateau(plateauInitial, plateau);
        
        int ligne = 0, colonne = 0;
        trouverSokoban(plateau, &ligne, &colonne);
        historique[compteurHistorique].ligne = ligne;
        historique[compteurHistorique].colonne = colonne;
        copierPlateau(plateau, historique[compteurHistorique].plateau);
        compteurHistorique++;
        
        for (int i = 0; i < nbTemp; i++){
            inutile = false;
            nbDeplacement = 0;
            trouverSokoban(plateau, &ligne, &colonne);
            deplacer(plateau, temp[i], ligne, colonne, &nbDeplacement, &inutile);
            
            if (!inutile){
                trouverSokoban(plateau, &ligne, &colonne);
                
                bool sequenceInutile = false;
                int positionRetour = -1;
                
                for (int j = 0; j < compteurHistorique; j++){
                    if (historique[j].ligne == ligne && 
                        historique[j].colonne == colonne &&
                        plateauxIdentiques(historique[j].plateau, plateau)){
                        sequenceInutile = true;
                        positionRetour = j;
                        break;
                    }
                }
                
                if (sequenceInutile){
                    compteurOpti = positionRetour;
                    compteurHistorique = positionRetour + 1;
                } else {
                    opti[compteurOpti] = temp[i];
                    compteurOpti++;
                    
                    historique[compteurHistorique].ligne = ligne;
                    historique[compteurHistorique].colonne = colonne;
                    copierPlateau(plateau, historique[compteurHistorique].plateau);
                    compteurHistorique++;
                }
            }
        }
        
        for (int i = 0; i < compteurOpti; i++){
            temp[i] = opti[i];
        }
        nbTemp = compteurOpti;
    }
    
    return compteurOpti;
}

void copierPlateau(typePlateau source, typePlateau destination){
    for (int i = 0; i < TAILLE; i++){
        for (int j = 0; j < TAILLE; j++){
            destination[i][j] = source[i][j];
        }
    }
}

bool plateauxIdentiques(typePlateau p1, typePlateau p2){
    for (int i = 0; i < TAILLE; i++){
        for (int j = 0; j < TAILLE; j++){
            if (p1[i][j] != p2[i][j]){
                return false;
            }
        }
    }
    return true;
}

void deplacer(typePlateau plateau, char touche, int ligneSokoban, int colonneSokoban, int *nbDeplacement, bool *inutile){
    int bougeEnLigne = 0, bougeEnColonne = 0;
    if (touche == 'g' || touche == 'G') {
        bougeEnColonne = -1;
    } else if (touche == 'd' || touche == 'D'){
        bougeEnColonne = 1;
    } else if (touche == 'h' || touche == 'H'){
        bougeEnLigne = -1;
    } else if (touche == 'b' || touche == 'B') {
        bougeEnLigne = 1;
    } else {
        return;
    }
    
    int maj = -1;
    if (touche == 'B' || touche == 'H' || touche == 'G' || touche == 'D'){
        maj = 1;
    }
    
    int nouvelleLigne = ligneSokoban + bougeEnLigne;
    int nouvelleColonne = colonneSokoban + bougeEnColonne;
    char destination = plateau[nouvelleLigne][nouvelleColonne];
    char positionActuelle = plateau[ligneSokoban][colonneSokoban];
    
    if (destination == '#'){
        *inutile = true;
        return;
    }
    
    if ((destination == ' ' && maj != 1) || (destination == '.' && maj != 1)){
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
    } else if ((destination == '$' && maj == 1) || (destination == '*' && maj == 1)){
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
        } else {
            *inutile = true;
        }
    } else {
        *inutile = true;
    }
}

void enregistrerPartie(typeDeplacements opti, char fichier[], int compteurOpti){
    FILE * f;
    f = fopen(fichier, "w");
    for (int i = 0; i < compteurOpti; i++){
        fwrite(&opti[i], sizeof(char), 1, f);
    }
    fclose(f);
}

bool gagne(typePlateau plateau){
    for (int i = 0; i < TAILLE; i++){
        for (int j = 0; j < TAILLE; j++){
            if (plateau[i][j] == '.'){
                return false;
            }
            if (plateau[i][j] == '+'){
                return false;
            }
        }
    }
    return true;
}

void trouverSokoban(typePlateau plateau, int *ligne, int *colonne){
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

void afficherEntete(char partie[20], int nbDeplacement){
    system("clear");
    printf("-------------------------------------------------------\n");
    printf("Partie : %s \n", partie);
    printf("Déplacements : %d\n", nbDeplacement);
    printf("-------------------------------------------------------\n");
}

void afficherPlateau(typePlateau plateau){
    for (int i = 0; i < TAILLE; i++){
        for (int j = 0; j < TAILLE; j++){
            char c = plateau[i][j];
            if (c == '*') c = '$';
            if (c == '+') c = '@';
            printf("%c", c);
        }
        printf("\n");
    }
}

void chargerPartie(typePlateau plateau, char fichier[]){
    FILE * f;
    char finDeLigne;

    f = fopen(fichier, "r");
    if (f == NULL){
        printf("ERREUR SUR FICHIER");
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

void chargerDeplacements(typeDeplacements t, char fichier[], int * nb){
    FILE * f;
    char dep;
    *nb = 0;

    f = fopen(fichier, "r");
    if (f == NULL){
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