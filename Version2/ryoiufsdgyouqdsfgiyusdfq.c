#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<stdbool.h>
#include<termios.h>
#include<unistd.h>
#include<fcntl.h>

#define TAILLE 12 
#define NBDEP 1000 // Augmenté pour supporter de longs fichiers .dep

typedef char typePlateau[TAILLE][TAILLE];
typedef char typeDeplacements[NBDEP];

// Prototypages
void chargerPartie(typePlateau plateau, char fichier[]);
void enregistrerPartie(typeDeplacements opti, char fichier[], int compteurOpti);
void afficherPlateau(typePlateau plateau);
void afficherEntete(char partie[20], int nbDeplacement);
void deplacer(typePlateau plateau, char touche, int ligneSokoban, int colonneSokoban, int *nbDeplacement, bool *inutile, bool *caisseBougee);
bool gagne(typePlateau plateau);
void trouverSokoban(typePlateau plateau, int *ligne, int *colonne);
void chargerDeplacements(typeDeplacements t, char fichier[], int * nb);

int main(){
    char partie[20];
    char deplacement[20];
    typePlateau plateau;
    typeDeplacements dep;
    typeDeplacements opti;
    typeDeplacements buffer; // Stockage temporaire avant validation
    
    // Historique des positions pour détecter les boucles
    int histoL[NBDEP], histoC[NBDEP];
    int nbBuffer = 0, nbPos = 0;

    int nbDeplacement = 0, nbLettre = 0;
    bool inutile = false, caisseBougee = false;
    
    printf("Entrez le nom du niveau (.sok) : ");
    scanf("%s", partie);
    printf("Entrez le nom du fichier de deplacement (.dep) : ");
    scanf("%s", deplacement);
    
    chargerPartie(plateau, partie);
    chargerDeplacements(dep, deplacement, &nbLettre);
    
    int i = 0;
    int ligne = 0, colonne = 0;
    int compteurOpti = 0;

    // Enregistrer la position initiale
    trouverSokoban(plateau, &ligne, &colonne);
    histoL[0] = ligne;
    histoC[0] = colonne;
    nbPos = 1;

    while (i < nbLettre){
        usleep(200000); // Pause pour suivre le déroulement [cite: 13]
        char touche = dep[i];        
        trouverSokoban(plateau, &ligne, &colonne);
        
        // On tente le déplacement
        deplacer(plateau, touche, ligne, colonne, &nbDeplacement, &inutile, &caisseBougee);

        // 1. Gestion des déplacements non joués [cite: 25, 27]
        if (inutile) {
            inutile = false; // Reset pour le prochain tour
            i++;
            continue; 
        }

        // Nouvelle position après mouvement
        int nl, nc;
        trouverSokoban(plateau, &nl, &nc);

        // 2. Gestion des séquences inutiles [cite: 29]
        if (caisseBougee) {
            // Une caisse a bougé : on valide tout le trajet précédent
            for (int j = 0; j < nbBuffer; j++) {
                opti[compteurOpti++] = buffer[j];
            }
            opti[compteurOpti++] = dep[i]; // On ajoute le coup de poussée actuel
            
            // On vide le buffer et on recommence l'historique à la nouvelle position
            nbBuffer = 0;
            histoL[0] = nl;
            histoC[0] = nc;
            nbPos = 1;
            caisseBougee = false;
        } 
        else {
            // Simple déplacement : on vérifie si on revient sur nos pas [cite: 29, 31]
            int indexBoucle = -1;
            for (int k = 0; k < nbPos; k++) {
                if (histoL[k] == nl && histoC[k] == nc) {
                    indexBoucle = k;
                    break;
                }
            }

            if (indexBoucle != -1) {
                // Boucle détectée : on tronque le buffer
                nbBuffer = indexBoucle; 
                nbPos = indexBoucle + 1;
            } else {
                // Pas de boucle : on continue d'enregistrer le trajet
                buffer[nbBuffer++] = dep[i];
                histoL[nbPos] = nl;
                histoC[nbPos] = nc;
                nbPos++;
            }
        }

        afficherEntete(partie, nbDeplacement);
        afficherPlateau(plateau);
        i++;
    }

    // Ajouter le reste du buffer (trajet final sans poussée de caisse)
    for (int j = 0; j < nbBuffer; j++) {
        opti[compteurOpti++] = buffer[j];
    }

    // Affichage du bilan [cite: 14, 15]
    if (gagne(plateau)){
        printf("\nLa suite %s est bien une solution.\n", deplacement);
        printf("Longueur initiale : %d\n", nbLettre);
        printf("Longueur optimisee : %d\n", compteurOpti);
        
        printf("Souhaitez-vous l’enregistrer (o/n) ? ");
        char choix;
        scanf(" %c", &choix);
        if (choix == 'o'){
            char fichier[30];
            printf("Rentrer un nom de fichier (.dep) : ");
            scanf("%s", fichier);
            enregistrerPartie(opti, fichier, compteurOpti); // [cite: 16]
        }
    } else {
        printf("\nLa suite %s N’EST PAS une solution.\n", deplacement); 
    }

    return EXIT_SUCCESS;
}

void deplacer(typePlateau plateau, char touche, int ligneSokoban, int colonneSokoban, int *nbDeplacement, bool *inutile, bool *caisseBougee){
    int bougeEnLigne = 0, bougeEnColonne = 0;
    *caisseBougee = false;

    if (touche == 'g' || touche == 'G') bougeEnColonne = -1;
    else if (touche == 'd' || touche == 'D') bougeEnColonne = 1;
    else if (touche == 'h' || touche == 'H') bougeEnLigne = -1;
    else if (touche == 'b' || touche == 'B') bougeEnLigne = 1;
    else { *inutile = true; return; }

    int maj = (touche == 'B' || touche == 'H' || touche == 'G' || touche == 'D') ? 1 : 0;
    
    int nouvelleLigne = ligneSokoban + bougeEnLigne;
    int nouvelleColonne = colonneSokoban + bougeEnColonne;
    char destination = plateau[nouvelleLigne][nouvelleColonne];
    char positionActuelle = plateau[ligneSokoban][colonneSokoban];

    if (destination == '#'){
        *inutile = true; // [cite: 26]
        return;
    }

    // Déplacement simple
    if (maj == 0 && (destination == ' ' || destination == '.')){
        plateau[ligneSokoban][colonneSokoban] = (positionActuelle == '@') ? ' ' : '.';
        plateau[nouvelleLigne][nouvelleColonne] = (destination == ' ') ? '@' : '+';
        (*nbDeplacement)++;
    }
    // Pousser une caisse
    else if (maj == 1 && (destination == '$' || destination == '*')){
        int ligneApres = nouvelleLigne + bougeEnLigne;
        int colonneApres = nouvelleColonne + bougeEnColonne;
        char caseApres = plateau[ligneApres][colonneApres];

        if (caseApres == ' ' || caseApres == '.'){
            plateau[ligneApres][colonneApres] = (caseApres == ' ') ? '$' : '*';
            plateau[nouvelleLigne][nouvelleColonne] = (destination == '$') ? '@' : '+';
            plateau[ligneSokoban][colonneSokoban] = (positionActuelle == '@') ? ' ' : '.';
            *caisseBougee = true;
            (*nbDeplacement)++;
        } else {
            *inutile = true;
        }
    }
    else {
        *inutile = true;
    }
}

void chargerPartie(typePlateau plateau, char fichier[]){
    FILE * f = fopen(fichier, "r");
    if (f==NULL) exit(EXIT_FAILURE);
    for (int ligne=0 ; ligne<TAILLE ; ligne++){
        for (int colonne=0 ; colonne<TAILLE ; colonne++){
            fread(&plateau[ligne][colonne], sizeof(char), 1, f);
        }
        fgetc(f); // skip \n
    }
    fclose(f);
}

void chargerDeplacements(typeDeplacements t, char fichier[], int * nb){
    FILE * f = fopen(fichier, "r");
    if (f==NULL) return;
    *nb = 0;
    char dep;
    while (fread(&dep, sizeof(char), 1, f) && *nb < NBDEP){
        if (dep != '\n' && dep != '\r') {
            t[*nb] = dep;
            (*nb)++;
        }
    }
    fclose(f);
}

void enregistrerPartie(typeDeplacements opti, char fichier[], int compteurOpti){
    FILE * f = fopen(fichier, "w");
    if (f) {
        for (int i=0; i < compteurOpti; i++) fputc(opti[i], f);
        fclose(f);
    }
}

bool gagne(typePlateau plateau){
    for (int i = 0; i < TAILLE; i++)
        for (int j = 0; j < TAILLE; j++)
            if (plateau[i][j] == '.' || plateau[i][j] == '+') return false;
    return true;
}

void trouverSokoban(typePlateau plateau, int *ligne, int *colonne){
    for (int i = 0; i < TAILLE; i++)
        for (int j = 0; j < TAILLE; j++)
            if (plateau[i][j] == '@' || plateau[i][j] == '+') {
                *ligne = i; *colonne = j; return;
            }
}

void afficherEntete(char partie[20], int nbDeplacement){
    system("clear");
    printf("-------------------------------------------------------\n");
    printf("Partie : %s | Déplacements : %d\n", partie, nbDeplacement);
    printf("-------------------------------------------------------\n");
}

void afficherPlateau(typePlateau plateau){
    for (int i = 0; i < TAILLE; i++){
        for (int j = 0; j < TAILLE; j++) printf("%c", plateau[i][j]);
        printf("\n");
    }
}