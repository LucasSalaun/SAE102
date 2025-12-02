#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

#define TAILLE 12
#define HAUT 'z'
#define BAS 's'
#define GAUCHE 'q'
#define DROITE 'd'
#define ABANDON 'x'
#define RESET 'r'
#define CAISSE '$'
#define CIBLE  '.'
#define SOKOBAN '@'
#define MUR '#'
#define CAISSE_CIBLE '*'
#define SOKOBAN_CIBLE '+'


typedef char t_Plateau[TAILLE][TAILLE];


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

void afficher_entete(char fichier[], int compteurActions)
{
    system("clear");
    printf("Niveau : %s\nHaut (%c)\nBas (%c) \nDroite (%c)\nGauche (%c)\n",
           fichier, HAUT, BAS, DROITE, GAUCHE);
    printf("Recommencer (%c)\nAbandoner (%c)\nNombres d'action : %d\n",
           RESET, ABANDON, compteurActions);
}

void afficher_plateau(t_Plateau plateau)
{
    for (int i = 0; i < TAILLE; i++)
    {
        for (int j = 0; j < TAILLE; j++)
        {
            char c = plateau[i][j];

            if (c == MUR)
            {
                printf("#");
            }
            else if (c == CAISSE || c == CAISSE_CIBLE)
            {
                printf("$");
            }
            else if (c == SOKOBAN || c == SOKOBAN_CIBLE)
            {
                printf("@");
            }
            else if (c == CIBLE)
            {
                printf(".");
            }
            else
            {
                printf(" ");
            }
        }
        printf("\n");
    }
}

void recup_touche(char *adrTouche)
{
    *adrTouche = getchar();
}

void deplacer(t_Plateau plateau, char touche, int sokobanX, int sokobanY,
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
        return;

    int nx = sokobanX + dx;
    int ny = sokobanY + dy;
    int nx2 = sokobanX + 2 * dx;
    int ny2 = sokobanY + 2 * dy;

    if (nx < 0 || nx >= TAILLE || ny < 0 || ny >= TAILLE)
        return;
    if (plateau[nx][ny] == MUR)
        return;

    if (plateau[nx][ny] == CAISSE ||
        plateau[nx][ny] == CAISSE_CIBLE)
    {
        if (nx2 < 0 || nx2 >= TAILLE || ny2 < 0 || ny2 >= TAILLE)
            return;

        if (plateau[nx2][ny2] == MUR ||
            plateau[nx2][ny2] == CAISSE ||
            plateau[nx2][ny2] == CAISSE_CIBLE)
            return;

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

int main()
{
    char nomFichier[20];
    char touche = '\0';
    int compteur = 0;
    int sokobanX;
    int sokobanY;
    t_Plateau plateau;
    t_Plateau tableau;

    printf("Nom du fichier : ");
    scanf("%s", nomFichier);

    charger_partie(tableau, nomFichier);
    charger_partie(plateau, nomFichier);

    trouver_sokoban(plateau, &sokobanX, &sokobanY);

    afficher_entete(nomFichier, compteur);
    afficher_plateau(plateau);

    while (touche != ABANDON && touche != RESET)
    {
        usleep(100);
        if (kbhit())
        {
            recup_touche(&touche);
            trouver_sokoban(plateau, &sokobanX, &sokobanY);
            deplacer(plateau, touche, sokobanX, sokobanY, &compteur);
            afficher_entete(nomFichier, compteur);
            afficher_plateau(plateau);

            if (gagne(plateau))
            {
                printf("Bravo ! Vous avez gagné la partie !\n");
                return 0;
            }
        }

        if (touche == ABANDON)
        {
            char rep;
            printf("Voulez-vous abandonner la partie ? (o/n) : ");

            do
            {
                rep = getchar();
            } while (rep == '\n');

            if (rep == 'o')
            {
                char choixSave;
                printf("Voulez-vous sauvegarder la partie avant de quitter ? (o/n) : ");

                do
                {
                    choixSave = getchar();
                } while (choixSave == '\n');

                if (choixSave == 'o')
                {
                    char fichierSave[30];
                    printf("Nom du fichier de sauvegarde (.sok) : ");
                    scanf("%s", fichierSave);

                    enregistrer_partie(plateau, fichierSave);

                    printf("Partie sauvegardée dans '%s'.\n", fichierSave);
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
            char rep;
            printf("Voulez-vous vraiment recommencer la partie ? (o/n) : ");

            do
            {
                rep = getchar();
            } while (rep == '\n');

            if (rep == 'o')
            {
                compteur = 0;
                for (int i = 0; i < TAILLE; i++)
                {
                    for (int j = 0; j < TAILLE; j++)
                    {
                        plateau[i][j] = tableau[i][j];
                    }
                }
                trouver_sokoban(plateau, &sokobanX, &sokobanY);
                afficher_entete(nomFichier, compteur);
                afficher_plateau(plateau);

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
