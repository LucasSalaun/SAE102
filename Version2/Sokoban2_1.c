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
void afficher_plateau(typePlateau plateau);
void afficher_entete(char partie[20], int nbDeplacement);
void deplacer(typePlateau plateau, char touche, int ligneSokoban, int colonneSokoban, int *nbDeplacement, bool *inutile, bool *caisseDeplacee);
bool gagne(typePlateau plateau);
void trouver_sokoban(typePlateau plateau, int *ligne, int *colonne);
void chargerDeplacements(typeDeplacements t, char fichier[], int * nb);
void pas_jouee(char dep[], bool *inutile, int * compteur, int i);
void optimiserSuite(typePlateau plateauDepart, typeDeplacements dep, int nb, typeDeplacements opti, int *nbOpti);

int main(){
    char partie[20];
    char deplacement[20];
    typePlateau plateau;
    typeDeplacements dep;
    typeDeplacements opti;
    int nbDeplacement = 0, nbLettre = 0, nbOpti = 0;
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
    int i = 0;
    int ligne = 0, colonne = 0;
    int compteurOpti = 0;

    trouver_sokoban(plateau, &ligne, &colonne);

    // Parcours de tous les déplacements
    while (i < nbLettre){
        usleep(200000);
        touche = dep[i];        
        trouver_sokoban(plateau, &ligne, &colonne);

        bool caisseDeplacee = false;
        deplacer(plateau, touche, ligne, colonne, &nbDeplacement, &inutile, &caisseDeplacee);

        pas_jouee(dep, &inutile, &compteurOpti ,i);
        afficher_entete(partie, nbDeplacement);
        afficher_plateau(plateau);
        i++;
    }

    gagner = gagne(plateau);

    if (gagner){
        printf("La suite de déplacements %s est bien une solution pour la partie %s.\n", deplacement, partie);
        printf("Elle contient initialement %d déplacements.\n", nbDeplacement);

        // Optimisation finale
        optimiserSuite(plateau, dep, nbLettre, opti, &nbOpti);
        printf("Après optimisation, elle contient %d déplacements.\n", nbOpti);

        // Proposer l'enregistrement
        char choix;
        printf("Souhaitez-vous enregistrer la suite optimisée ? (O/N) : ");
        scanf(" %c", &choix);
        if (choix == 'O' || choix == 'o'){
            FILE *f = fopen("optimise.dep", "w");
            for (int j = 0; j < nbOpti; j++){
                fputc(opti[j], f);
            }
            fclose(f);
            printf("Suite optimisée enregistrée dans 'optimise.dep'\n");
        }
    } else {
        printf("La suite de déplacements %s N’EST PAS une solution pour la partie %s.\n", deplacement, partie);
    }

    return EXIT_SUCCESS;
}

// Déplace le Sokoban
void deplacer(typePlateau plateau, char touche, int ligneSokoban, int colonneSokoban, int *nbDeplacement, bool *inutile, bool *caisseDeplacee){
    int bougeEnLigne = 0, bougeEnColonne = 0;
    if (touche == 'g' || touche == 'G') bougeEnColonne = -1;
    else if (touche == 'd' || touche == 'D') bougeEnColonne = 1;
    else if (touche == 'h' || touche == 'H') bougeEnLigne = -1;
    else if (touche == 'b' || touche == 'B') bougeEnLigne = 1;
    else return;

    int maj = (touche >= 'A' && touche <= 'Z') ? 1 : -1;
    int nouvelleLigne = ligneSokoban + bougeEnLigne;
    int nouvelleColonne = colonneSokoban + bougeEnColonne;

    char destination = plateau[nouvelleLigne][nouvelleColonne];
    char positionActuelle = plateau[ligneSokoban][colonneSokoban];

    if (destination == '#'){
        *inutile = true;
        return;
    }

    if ((destination == ' ' && maj != 1) || (destination == '.' && maj != 1)){
        plateau[ligneSokoban][colonneSokoban] = (positionActuelle == '@') ? ' ' : '.';
        plateau[nouvelleLigne][nouvelleColonne] = (destination == ' ') ? '@' : '+';
        (*nbDeplacement)++;
        *caisseDeplacee = false;
    }
    else if ((destination == '$' && maj == 1) || (destination == '*' && maj == 1)){
        int ligneApres = nouvelleLigne + bougeEnLigne;
        int colonneApres = nouvelleColonne + bougeEnColonne;
        char caseApres = plateau[ligneApres][colonneApres];
        if (caseApres == ' ' || caseApres == '.'){
            plateau[ligneApres][colonneApres] = (caseApres == ' ') ? '$' : '*';
            plateau[nouvelleLigne][nouvelleColonne] = (destination == '$') ? '@' : '+';
            plateau[ligneSokoban][colonneSokoban] = (positionActuelle == '@') ? ' ' : '.';
            (*nbDeplacement)++;
            *caisseDeplacee = true;
        } else {
            *inutile = true;
        }
    } else {
        *inutile = true;
    }
}

// Marque un déplacement non joué
void pas_jouee(char dep[], bool *inutile, int * compteur, int i){
    if (*inutile){
        dep[i]='?';
        *inutile = false;
        (*compteur)++;
    }
}

// Vérifie si la partie est gagnée
bool gagne(typePlateau plateau){
    for (int i = 0; i < TAILLE; i++){
        for (int j = 0; j < TAILLE; j++){
            if (plateau[i][j] == '.' || plateau[i][j] == '+')
                return false;
        }
    }
    return true;
}

// Trouve la position du Sokoban
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
    if (f==NULL){
        printf("ERREUR SUR FICHIER");
        exit(EXIT_FAILURE);
    }
    for (int ligne=0 ; ligne<TAILLE ; ligne++){
        for (int colonne=0 ; colonne<TAILLE ; colonne++){
            fread(&plateau[ligne][colonne], sizeof(char), 1, f);
        }
        fread(&finDeLigne, sizeof(char), 1, f);
    }
    fclose(f);
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
    if (f==NULL){
        printf("FICHIER NON TROUVE\n");
        return;
    }
    fread(&dep, sizeof(char), 1, f);
    if (feof(f)){
        printf("FICHIER VIDE\n");
        fclose(f);
        return;
    }
    while (!feof(f)){
        t[*nb] = dep;
        (*nb)++;
        fread(&dep, sizeof(char), 1, f);
    }
    fclose(f);
}

// Optimisation finale : supprime non joués et séquences inutiles
void optimiserSuite(typePlateau plateauDepart, typeDeplacements dep, int nb, typeDeplacements opti, int *nbOpti){
    *nbOpti = 0;
    int ligne, colonne;
    typePlateau plateauTemp;
    for(int i=0;i<TAILLE;i++)
        for(int j=0;j<TAILLE;j++)
            plateauTemp[i][j] = plateauDepart[i][j];
    trouver_sokoban(plateauTemp, &ligne, &colonne);

    int i=0;
    while(i<nb){
        if(dep[i]=='?'){ i++; continue; }

        // Début d'une séquence
        int seqStart = i;
        int seqEnd = i;
        int ligneSeq = ligne, colonneSeq = colonne;
        typePlateau plateauSeq;
        for(int r=0;r<TAILLE;r++)
            for(int c=0;c<TAILLE;c++)
                plateauSeq[r][c] = plateauTemp[r][c];
        bool caisseBougee = false;

        // Parcours jusqu'à la fin de séquence utile
        while(seqEnd<nb && dep[seqEnd]!='?'){
            bool inutile=false;
            bool caisse=false;
            deplacer(plateauSeq, dep[seqEnd], ligneSeq, colonneSeq, &seqEnd, &inutile, &caisse);
            if(caisse) caisseBougee = true;
            trouver_sokoban(plateauSeq, &ligneSeq, &colonneSeq);
            seqEnd++;
        }

        // Si séquence inutile, ne rien ajouter
        if(caisseBougee || ligneSeq!=ligne || colonneSeq!=colonne){
            // séquence utile
            for(int k=seqStart;k<seqEnd;k++){
                if(dep[k]!='?'){
                    opti[*nbOpti] = dep[k];
                    (*nbOpti)++;
                    // appliquer sur plateauTemp
                    bool inutile2=false, caisse2=false;
                    deplacer(plateauTemp, dep[k], ligne, colonne, &i, &inutile2, &caisse2);
                    trouver_sokoban(plateauTemp, &ligne, &colonne);
                }
            }
        }
        i = seqEnd;
    }
}
