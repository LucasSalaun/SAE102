#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>

#define TAILLE 12
#define NBDEP 500

typedef char typePlateau[TAILLE][TAILLE];
typedef char typeDeplacements[NBDEP];

/* ================= PROTOTYPES ================= */
void chargerPartie(typePlateau plateau, char fichier[]);
void chargerDeplacements(typeDeplacements t, char fichier[], int *nb);
void enregistrer_partie(typeDeplacements opti, char fichier[], int nb);
void afficher_plateau(typePlateau plateau);
void afficher_entete(char partie[], int nbDeplacement);
void trouver_sokoban(typePlateau plateau, int *l, int *c);
bool gagne(typePlateau plateau);

void deplacer(typePlateau plateau, char touche, int l, int c,
              bool *inutile, bool *caisseBougee);

/* ================= MAIN ================= */
int main() {
    typePlateau plateau;
    typeDeplacements dep, opti, buffer;
    int posX[NBDEP], posY[NBDEP];

    char sok[20], depFile[20];
    int nbLettre = 0;
    int nbDeplacement = 0;

    printf("Nom du niveau (.sok) : ");
    scanf("%s", sok);
    printf("Nom du fichier deplacements (.dep) : ");
    scanf("%s", depFile);

    chargerPartie(plateau, sok);
    chargerDeplacements(dep, depFile, &nbLettre);

    int i = 0;
    int compteurOpti = 0;
    int nbBuffer = 0;
    int nbPos = 0;

    bool inutile, caisseBougee;
    int l, c;

    while (i < nbLettre) {

        afficher_entete(sok, nbDeplacement);
        afficher_plateau(plateau);
        usleep(200000);

        trouver_sokoban(plateau, &l, &c);

        deplacer(plateau, dep[i], l, c, &inutile, &caisseBougee);

        if (inutile) {
            i++;
            continue;
        }

        trouver_sokoban(plateau, &l, &c);

        /* ===== SI UNE CAISSE A BOUGÉ ===== */
        if (caisseBougee) {
            for (int j = 0; j < nbBuffer; j++)
                opti[compteurOpti++] = buffer[j];

            opti[compteurOpti++] = dep[i];

            nbBuffer = 0;
            nbPos = 0;
            nbDeplacement++;
            i++;
            continue;
        }

        /* ===== DEPLACEMENT SANS CAISSE ===== */
        buffer[nbBuffer] = dep[i];
        posX[nbPos] = l;
        posY[nbPos] = c;
        nbBuffer++;
        nbPos++;

        /* ===== DETECTION DE BOUCLE ===== */
        for (int k = 0; k < nbPos - 1; k++) {
            if (posX[k] == l && posY[k] == c) {
                nbBuffer = k;
                nbPos = k;
                break;
            }
        }

        nbDeplacement++;
        i++;
    }

    /* ===== FIN : COPIE DU RESTE ===== */
    for (int j = 0; j < nbBuffer; j++)
        opti[compteurOpti++] = buffer[j];

    /* ===== BILAN ===== */
    if (gagne(plateau)) {
        printf("\nLa suite %s est une solution.\n", depFile);
        printf("Longueur initiale : %d\n", nbLettre);
        printf("Longueur optimisee : %d\n", compteurOpti);

        char out[20];
        printf("Nom du fichier optimise (.dep) : ");
        scanf("%s", out);
        enregistrer_partie(opti, out, compteurOpti);
    } else {
        printf("\nLa suite %s N'EST PAS une solution.\n", depFile);
    }

    return EXIT_SUCCESS;
}

/* ================= DEPLACER ================= */
void deplacer(typePlateau plateau, char touche, int l, int c,
              bool *inutile, bool *caisseBougee) {

    int dl = 0, dc = 0;
    *inutile = false;
    *caisseBougee = false;

    if (touche == 'g' || touche == 'G') dc = -1;
    else if (touche == 'd' || touche == 'D') dc = 1;
    else if (touche == 'h' || touche == 'H') dl = -1;
    else if (touche == 'b' || touche == 'B') dl = 1;
    else {
        *inutile = true;
        return;
    }

    bool pousser = (touche >= 'A' && touche <= 'Z');

    int nl = l + dl, nc = c + dc;
    char cur = plateau[l][c];
    char dest = plateau[nl][nc];

    if (dest == '#') {
        *inutile = true;
        return;
    }

    /* DEPLACEMENT SIMPLE */
    if (!pousser && (dest == ' ' || dest == '.')) {
        plateau[l][c] = (cur == '+' ? '.' : ' ');
        plateau[nl][nc] = (dest == '.' ? '+' : '@');
        return;
    }

    /* POUSSER UNE CAISSE */
    if (pousser && (dest == '$' || dest == '*')) {
        int nl2 = nl + dl, nc2 = nc + dc;
        char dest2 = plateau[nl2][nc2];

        if (dest2 == ' ' || dest2 == '.') {
            plateau[nl2][nc2] = (dest2 == '.' ? '*' : '$');
            plateau[nl][nc] = (dest == '*' ? '+' : '@');
            plateau[l][c] = (cur == '+' ? '.' : ' ');
            *caisseBougee = true;
            return;
        }
    }

    *inutile = true;
}

/* ================= UTILITAIRES ================= */
bool gagne(typePlateau plateau) {
    for (int i = 0; i < TAILLE; i++)
        for (int j = 0; j < TA
