#ifndef AFFICHAGE_HPP
#define AFFICHAGE_HPP

#include "general.hpp"


extern struct SDL_Surface *ecran;


void decharger_images();

void afficher_menu(int selection);
void afficher_jeu();
void afficher_palette();
void afficher_objet_souris(Position souris, int objet);

void scroll_init();
void scroll_gauche();
void scroll_droite();
void scroll_haut();
void scroll_bas();
void scroll_centrer();

Position case_souris(Position souris);


#endif
