#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

#define DEPLACEMENT 500
#define TAILLE 12
#define HAUT 'z'
#define BAS 's'
#define GAUCHE 'q'
#define DROITE 'd'
#define ABANDON 'x'
#define RESET 'r'
#define UNDO 'u'
#define ZOOM_PLUS '+'
#define ZOOM_MOINS '-'
#define CAISSE '$'
#define CIBLE '.'
#define SOKOBAN '@'
#define MUR '#'
#define CAISSE_CIBLE '*'
#define SOKOBAN_CIBLE '+'

typedef char t_Plateau[TAILLE][TAILLE];
typedef char t_tabDeplacement[DEPLACEMENT];

void charger_partie(t_Plateau plateau, char fichier[])
{
    FILE *f;
    char finDeLigne;

    f = fopen(fichier, "r");
    if (f == NULL)
    {
        printf("ERREUR SUR FICHIER");
        exit(EXIT_FAILURE);
    }
    else
    {
        for (int ligne = 0; ligne < TAILLE; ligne++)
        {
            for (int colonne = 0; colonne < TAILLE; colonne++)
            {
                fread(&plateau[ligne][colonne], sizeof(char), 1, f);
            }
            fread(&finDeLigne, sizeof(char), 1, f);
        }
        fclose(f);
    }
}

void enregistrer_partie(t_Plateau plateau, char fichier[])
{
    FILE *f;
    char finDeLigne = '\n';

    f = fopen(fichier, "w");
    for (int ligne = 0; ligne < TAILLE; ligne++)
    {
        for (int colonne = 0; colonne < TAILLE; colonne++)
        {
            fwrite(&plateau[ligne][colonne], sizeof(char), 1, f);
        }
        fwrite(&finDeLigne, sizeof(char), 1, f);
    }
    fclose(f);
}

int kbhit()
{
    // la fonction retourne :
    // 1 si un caractere est present
    // 0 si pas de caractere présent
    int unCaractere = 0;
    struct termios oldt, newt;
    int ch;
    int oldf;

    // mettre le terminal en mode non bloquant
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    ch = getchar();

    // restaurer le mode du terminal
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    if (ch != EOF)
    {
        ungetc(ch, stdin);
        unCaractere = 1;
    }
    return unCaractere;
}

void enregistrerDeplacements(t_tabDeplacement t, int nb, char fic[])
{
    FILE *f;

    f = fopen(fic, "w");
    fwrite(t, sizeof(char), nb, f);
    fclose(f);
}

void afficher_entete(char fichier[], int compteurActions)
{
    system("clear");
    printf("Niveau : %s\nHaut (%c)\nBas (%c) \nDroite (%c)\nGauche (%c)\n",
           fichier, HAUT, BAS, DROITE, GAUCHE);
    printf("Recommencer (%c)\nAbandoner (%c)\nUndo (%c)\nZoom (%c/%c)\nNombres d'action : %d\n",
           RESET, ABANDON, UNDO, ZOOM_PLUS, ZOOM_MOINS, compteurActions);
}

void afficher_plateau(t_Plateau plateau, int zoom)
{
    for (int i = 0; i < TAILLE; i++)
    {
        for (int x = 0; x < zoom; x++)
        {
            for (int j = 0; j < TAILLE; j++)
            {
                char c = plateau[i][j];

                if (c == MUR)
                {
                    for (int y = 0; y < zoom; y++)
                        printf("#");
                }
                else if (c == CAISSE || c == CAISSE_CIBLE)
                {
                    for (int y = 0; y < zoom; y++)
                        printf("$");
                }
                else if (c == SOKOBAN || c == SOKOBAN_CIBLE)
                {
                    for (int y = 0; y < zoom; y++)
                        printf("@");
                }
                else if (c == CIBLE)
                {
                    for (int y = 0; y < zoom; y++)
                        printf(".");
                }
                else
                {
                    for (int y = 0; y < zoom; y++)
                        printf(" ");
                }
            }
            printf("\n");
        }
    }
}

void recup_touche(char *adrTouche)
{
    *adrTouche = getchar();
}

void copierPlateau(t_Plateau src, t_Plateau dst)
{
    for (int i = 0; i < TAILLE; i++)
        for (int j = 0; j < TAILLE; j++)
            dst[i][j] = src[i][j];
}

int deplacer_original(t_Plateau plateau, char touche, int sokobanX, int sokobanY,
                      int *adrNbD)
{
    int dx = 0;
    int dy = 0;
    int deplaceEffectue = 0;

    if (touche == HAUT)
        dx = -1;
    else if (touche == BAS)
        dx = 1;
    else if (touche == GAUCHE)
        dy = -1;
    else if (touche == DROITE)
        dy = 1;
    else
        return 0;

    int nx = sokobanX + dx;
    int ny = sokobanY + dy;
    int nx2 = sokobanX + 2 * dx;
    int ny2 = sokobanY + 2 * dy;

    if (nx < 0 || nx >= TAILLE || ny < 0 || ny >= TAILLE)
        return 0;
    if (plateau[nx][ny] == MUR)
        return 0;

    if (plateau[nx][ny] == CAISSE ||
        plateau[nx][ny] == CAISSE_CIBLE)
    {
        if (nx2 < 0 || nx2 >= TAILLE || ny2 < 0 || ny2 >= TAILLE)
            return 0;

        if (plateau[nx2][ny2] == MUR ||
            plateau[nx2][ny2] == CAISSE ||
            plateau[nx2][ny2] == CAISSE_CIBLE)
            return 0;

        if (plateau[nx2][ny2] == CIBLE)
            plateau[nx2][ny2] = CAISSE_CIBLE;
        else
            plateau[nx2][ny2] = CAISSE;

        if (plateau[nx][ny] == CAISSE_CIBLE)
            plateau[nx][ny] = CIBLE;
        else
            plateau[nx][ny] = ' ';

        deplaceEffectue = 1;
    }
    else
    {
        deplaceEffectue = 1;
    }

    if (plateau[nx][ny] == CIBLE)
        plateau[nx][ny] = SOKOBAN_CIBLE;
    else
        plateau[nx][ny] = SOKOBAN;

    if (plateau[sokobanX][sokobanY] == SOKOBAN_CIBLE)
        plateau[sokobanX][sokobanY] = CIBLE;
    else
        plateau[sokobanX][sokobanY] = ' ';

    if (deplaceEffectue)
        (*adrNbD)++;

    return deplaceEffectue;
}

char code_deplacement(char t, int pousser)
{
    char rep = '\0';
    if (t == GAUCHE && pousser == 0)
        rep = 'g';
    if (t == HAUT && pousser == 0)
        rep = 'h';
    if (t == BAS && pousser == 0)
        rep = 'b';
    if (t == DROITE && pousser == 0)
        rep = 'd';
    if (t == GAUCHE && pousser == 1)
        rep = 'G';
    if (t == HAUT && pousser == 1)
        rep = 'H';
    if (t == BAS && pousser == 1)
        rep = 'B';
    if (t == DROITE && pousser == 1)
        rep = 'D';
    return rep;
}

int deplacer_memo(t_Plateau plateau, char touche, int sokobanX, int sokobanY,
                  int *adrNbD, char *dernierDepl)
{
    int dx = 0;
    int dy = 0;
    int deplaceEffectue = 0;
    int pousse = 0;

    if (touche == HAUT)
        dx = -1;
    else if (touche == BAS)
        dx = 1;
    else if (touche == GAUCHE)
        dy = -1;
    else if (touche == DROITE)
        dy = 1;
    else
        return 0;

    int nx = sokobanX + dx;
    int ny = sokobanY + dy;
    int nx2 = sokobanX + 2 * dx;
    int ny2 = sokobanY + 2 * dy;

    if (nx < 0 || nx >= TAILLE || ny < 0 || ny >= TAILLE)
        return 0;
    if (plateau[nx][ny] == MUR)
        return 0;

    if (plateau[nx][ny] == CAISSE ||
        plateau[nx][ny] == CAISSE_CIBLE)
    {
        if (nx2 < 0 || nx2 >= TAILLE || ny2 < 0 || ny2 >= TAILLE)
            return 0;

        if (plateau[nx2][ny2] == MUR ||
            plateau[nx2][ny2] == CAISSE ||
            plateau[nx2][ny2] == CAISSE_CIBLE)
            return 0;

        if (plateau[nx2][ny2] == CIBLE)
            plateau[nx2][ny2] = CAISSE_CIBLE;
        else
            plateau[nx2][ny2] = CAISSE;

        if (plateau[nx][ny] == CAISSE_CIBLE)
            plateau[nx][ny] = CIBLE;
        else
            plateau[nx][ny] = ' ';

        pousse = 1;
        deplaceEffectue = 1;
    }
    else
    {
        deplaceEffectue = 1;
    }

    if (plateau[nx][ny] == CIBLE)
        plateau[nx][ny] = SOKOBAN_CIBLE;
    else
        plateau[nx][ny] = SOKOBAN;

    if (plateau[sokobanX][sokobanY] == SOKOBAN_CIBLE)
        plateau[sokobanX][sokobanY] = CIBLE;
    else
        plateau[sokobanX][sokobanY] = ' ';

    if (deplaceEffectue)
    {
        (*adrNbD)++;
        *dernierDepl = code_deplacement(touche, pousse);
    }

    return deplaceEffectue;
}

void trouver_sokoban(t_Plateau plateau, int *adrSokobanX, int *adrSokobanY)
{
    int x;
    int y;
    *adrSokobanX = -1;
    *adrSokobanY = -1;

    for (x = 0; x < TAILLE; x++)
    {
        for (y = 0; y < TAILLE; y++)
        {
            if (plateau[x][y] == SOKOBAN ||
                plateau[x][y] == SOKOBAN_CIBLE)
            {
                *adrSokobanX = x;
                *adrSokobanY = y;
                return;
            }
        }
    }
}

bool gagne(t_Plateau plateau)
{
    for (int i = 0; i < TAILLE; i++)
    {
        for (int j = 0; j < TAILLE; j++)
        {
            if (plateau[i][j] == CIBLE)
                return false;
            if (plateau[i][j] == SOKOBAN_CIBLE)
                return false;
        }
    }
    return true;
}

void undo_operation(t_Plateau plateau, char code, int *adrNbD)
{
    int dx = 0;
    int dy = 0;
    int pousser = 0;

    if (code == 'g')
        dy = 1;
    if (code == 'h')
        dx = 1;
    if (code == 'b')
        dx = -1;
    if (code == 'd')
        dy = -1;
    if (code == 'G')
    {
        dy = 1;
        pousser = 1;
    }
    if (code == 'H')
    {
        dx = 1;
        pousser = 1;
    }
    if (code == 'B')
    {
        dx = -1;
        pousser = 1;
    }
    if (code == 'D')
    {
        dy = -1;
        pousser = 1;
    }

    int sokX;
    int sokY;
    trouver_sokoban(plateau, &sokX, &sokY);

    int targetX = sokX + dx;
    int targetY = sokY + dy;

    if (pousser == 1)
    {
        int pushX = sokX + dx * 2;
        int pushY = sokY + dy * 2;

        if (plateau[targetX][targetY] == CAISSE)
            plateau[sokX][sokY] = CAISSE;
        if (plateau[targetX][targetY] == CAISSE_CIBLE)
            plateau[sokX][sokY] = CAISSE_CIBLE;

        if (plateau[pushX][pushY] == CAISSE)
            plateau[targetX][targetY] = ' ';
        if (plateau[pushX][pushY] == CAISSE_CIBLE)
            plateau[targetX][targetY] = CIBLE;
    }

    if (plateau[targetX][targetY] == ' ')
        plateau[targetX][targetY] = SOKOBAN;
    if (plateau[targetX][targetY] == CIBLE)
        plateau[targetX][targetY] = SOKOBAN_CIBLE;

    if (plateau[sokX][sokY] == SOKOBAN)
        plateau[sokX][sokY] = ' ';
    if (plateau[sokX][sokY] == SOKOBAN_CIBLE)
        plateau[sokX][sokY] = CIBLE;

    if (*adrNbD > 0)
        (*adrNbD)--;
}

int main()
{
    char nomFichier[64];
    char touche = '\0';
    int compteurActions = 0;
    int sokobanX;
    int sokobanY;
    int zoom = 1;
    t_Plateau plateau;
    t_Plateau plateauOriginal;

    t_tabDeplacement tabDeplacements;
    int nbDeplacements = 0;

    t_Plateau historiquePlateau[DEPLACEMENT + 1];
    int indiceHistorique = 0;

    printf("Nom du fichier : ");
    scanf("%s", nomFichier);

    charger_partie(plateauOriginal, nomFichier);
    charger_partie(plateau, nomFichier);

    copierPlateau(plateau, historiquePlateau[0]);

    trouver_sokoban(plateau, &sokobanX, &sokobanY);

    afficher_entete(nomFichier, compteurActions);
    afficher_plateau(plateau, zoom);

    int caractere;
    while ((caractere = getchar()) != '\n' && caractere != EOF)
    {
    }

    while (touche != ABANDON && touche != RESET)
    {
        usleep(100);
        if (kbhit())
        {
            recup_touche(&touche);

            if (touche == ZOOM_PLUS)
            {
                if (zoom < 3)
                    zoom++;
                afficher_entete(nomFichier, compteurActions);
                afficher_plateau(plateau, zoom);
                touche = '\0';
                continue;
            }

            if (touche == ZOOM_MOINS)
            {
                if (zoom > 1)
                    zoom--;
                afficher_entete(nomFichier, compteurActions);
                afficher_plateau(plateau, zoom);
                touche = '\0';
                continue;
            }

            if (touche == UNDO)
            {
                if (indiceHistorique > 0)
                {
                    indiceHistorique--;
                    copierPlateau(historiquePlateau[indiceHistorique], plateau);
                    if (compteurActions > 0)
                        compteurActions--;
                    if (nbDeplacements > 0)
                        nbDeplacements--;
                }
                afficher_entete(nomFichier, compteurActions);
                afficher_plateau(plateau, zoom);
                touche = '\0';
                continue;
            }

            trouver_sokoban(plateau, &sokobanX, &sokobanY);

            char dernierDeplacement;
            int deplacementValide = deplacer_memo(plateau, touche, sokobanX, sokobanY, &compteurActions,
                                                  &dernierDeplacement);

            if (deplacementValide)
            {
                if (nbDeplacements < DEPLACEMENT)
                {
                    tabDeplacements[nbDeplacements] = dernierDeplacement;
                    nbDeplacements++;
                }
                if (indiceHistorique < DEPLACEMENT)
                {
                    indiceHistorique++;
                    copierPlateau(plateau, historiquePlateau[indiceHistorique]);
                }
                afficher_entete(nomFichier, compteurActions);
                afficher_plateau(plateau, zoom);
                if (gagne(plateau))
                {
                    printf("Bravo ! Vous avez gagné la partie !\n");

                    char reponseUtilisateur;
                    printf("Voulez-vous sauvegarder la liste des déplacements ? (o/n) : ");
                    do
                    {
                        reponseUtilisateur = getchar();
                    } while (reponseUtilisateur == '\n');

                    if (reponseUtilisateur == 'o')
                    {
                        char fichierDeplacements[128];
                        printf("Nom du fichier de déplacements (.dep) : ");
                        scanf("%s", fichierDeplacements);
                        while ((caractere = getchar()) != '\n' && caractere != EOF)
                        {
                        }
                        enregistrerDeplacements(tabDeplacements, nbDeplacements, fichierDeplacements);
                        printf("Déplacements sauvegardés dans '%s'.\n", fichierDeplacements);
                    }
                    return 0;
                }
            }

            afficher_entete(nomFichier, compteurActions);
            afficher_plateau(plateau, zoom);
        }

        if (touche == ABANDON)
        {
            char reponseUtilisateur;
            printf("Voulez-vous abandonner la partie ? (o/n) : ");

            do
            {
                reponseUtilisateur = getchar();
            } while (reponseUtilisateur == '\n');

            if (reponseUtilisateur == 'o')
            {
                char choixSauvegarde;
                printf("Voulez-vous sauvegarder la partie avant de quitter ? (o/n) : ");

                do
                {
                    choixSauvegarde = getchar();
                } while (choixSauvegarde == '\n');

                if (choixSauvegarde == 'o')
                {
                    char fichierSauvegarde[128];
                    printf("Nom du fichier de sauvegarde (.sok) : ");
                    scanf("%s", fichierSauvegarde);
                    enregistrer_partie(plateau, fichierSauvegarde);
                    printf("Partie sauvegardée dans '%s'.\n", fichierSauvegarde);
                }

                char choixDep;
                printf("Voulez-vous sauvegarder les déplacements ? (o/n) : ");
                do
                {
                    choixDep = getchar();
                } while (choixDep == '\n');

                if (choixDep == 'o')
                {
                    char fichierDeplacements[128];
                    printf("Nom du fichier de déplacements (.dep) : ");
                    scanf("%s", fichierDeplacements);
                    enregistrerDeplacements(tabDeplacements, nbDeplacements, fichierDeplacements);
                    printf("Déplacements sauvegardés dans '%s'.\n", fichierDeplacements);
                }

                printf("Vous avez abandonné la partie.\n");
                return 0;
            }
            else
            {
                touche = '\0';
                continue;
            }
        }

        if (touche == RESET)
        {
            char reponseUtilisateur;
            printf("Voulez-vous vraiment recommencer la partie ? (o/n) : ");

            do
            {
                reponseUtilisateur = getchar();
            } while (reponseUtilisateur == '\n');

            if (reponseUtilisateur == 'o')
            {
                compteurActions = 0;
                nbDeplacements = 0;
                indiceHistorique = 0;
                copierPlateau(plateauOriginal, plateau);
                copierPlateau(plateau, historiquePlateau[0]);
                afficher_entete(nomFichier, compteurActions);
                afficher_plateau(plateau, zoom);
                touche = '\0';
                continue;
            }
            else
            {
                touche = '\0';
                continue;
            }
        }
    }

    return 0;
}
