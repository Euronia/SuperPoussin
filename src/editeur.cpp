#include <stdio.h>

#include "affichage.hpp"
#include "editeur.hpp"
#include "main.hpp"


int choisir_objet(Position souris)
{
	int x1 = souris.x/32;
	int y1 = souris.y/32;
	if(x1%2 == 0 || y1%2 == 0)
		return OBJET_INVALIDE;
	return y1/2*10 + (x1+1)/2;
}

void ajouter_objet(Position souris, int objet)
{
	Position pos = case_souris(souris);

	/*if(objet == OBJET_BOITE_TOMBE)
		niveau.ajouter_boite_tombe(pos.x, pos.y);

	else if(objet == OBJET_BLOCTERRE1)
		niveau.ajouter_bloc_terre(pos.x, pos.y, 1);
    else if(objet == OBJET_BLOCTERRE2)
		niveau.ajouter_bloc_terre(pos.x, pos.y, 2);
    else if(objet == OBJET_BLOCTERRE3)
		niveau.ajouter_bloc_terre(pos.x, pos.y, 3);
    else if(objet == OBJET_BLOCTERRE4)
		niveau.ajouter_bloc_terre(pos.x, pos.y, 4);
	else if(objet == OBJET_BLOCTERRE5)
		niveau.ajouter_bloc_terre(pos.x, pos.y, 5);
	else if(objet == OBJET_BLOCTERRE6)
		niveau.ajouter_bloc_terre(pos.x, pos.y, 6);
	else if(objet == OBJET_BLOCTERRE7)
		niveau.ajouter_bloc_terre(pos.x, pos.y, 7);
    else if(objet == OBJET_BLOCTERRE8)
		niveau.ajouter_bloc_terre(pos.x, pos.y, 8);
	else if(objet == OBJET_BLOCTERRE9)
		niveau.ajouter_bloc_terre(pos.x, pos.y, 9);
	else if(objet == OBJET_BLOCTERRE10)
		niveau.ajouter_bloc_terre(pos.x, pos.y, 10);

	else if(objet == OBJET_BOITE_CLEF1)
		niveau.ajouter_boite_clef(pos.x, pos.y, 0);
	else if(objet == OBJET_BOITE_CLEF2)
		niveau.ajouter_boite_clef(pos.x, pos.y, 1);
	else if(objet == OBJET_BOITE_CLEF3)
		niveau.ajouter_boite_clef(pos.x, pos.y, 2);
	else if(objet == OBJET_BOITE_CLEF4)
		niveau.ajouter_boite_clef(pos.x, pos.y, 3);
    else if(objet == OBJET_BOITE_CLEF5)
		niveau.ajouter_boite_clef(pos.x, pos.y, 4);
    else if(objet == OBJET_BOITE_CLEF6)
		niveau.ajouter_boite_clef(pos.x, pos.y, 5);

	else if(objet == OBJET_CLEF1)
		niveau.ajouter_clef(pos.x, pos.y, 0);
	else if(objet == OBJET_CLEF2)
		niveau.ajouter_clef(pos.x, pos.y, 1);
	else if(objet == OBJET_CLEF3)
		niveau.ajouter_clef(pos.x, pos.y, 2);
	else if(objet == OBJET_CLEF4)
		niveau.ajouter_clef(pos.x, pos.y, 3);
    else if(objet == OBJET_CLEF5)
		niveau.ajouter_clef(pos.x, pos.y, 4);
    else if(objet == OBJET_CLEF6)
		niveau.ajouter_clef(pos.x, pos.y, 5);

	else if(objet == OBJET_SPAWN_DARKPOUSSIN_D)
		niveau.ajouter_spawn_darkpoussin(pos.x, pos.y);

	else if(objet == OBJET_FLEUR)
		niveau.ajouter_fleur(pos.x, pos.y);
	else if(objet == OBJET_POTDEFLEUR)
		niveau.ajouter_potdefleur(pos.x, pos.y);

	else if(objet == OBJET_LEVIER)
		niveau.ajouter_levier(pos.x, pos.y, 0);

	else if(objet == OBJET_OEUF)
		niveau.ajouter_oeuf(pos.x, pos.y);

	else if(objet == OBJET_VENTILATEUR_HAUT)
		niveau.ajouter_ventilateur(pos.x, pos.y, DIRECTION_HAUT, 3);
	else if(objet == OBJET_VENTILATEUR_DROITE)
		niveau.ajouter_ventilateur(pos.x, pos.y, DIRECTION_DROITE, 3);
	else if(objet == OBJET_VENTILATEUR_GAUCHE)
		niveau.ajouter_ventilateur(pos.x, pos.y, DIRECTION_GAUCHE, 3);

	else if(objet == OBJET_FUSEE)
		niveau.saisir_fusee(pos.x, pos.y);

	else if(objet == OBJET_SPAWN)
		niveau.saisir_spawn(pos.x, pos.y);*/
}

void supprimer_objets(Position souris)
{
	Position pos = case_souris(souris);

	/*int i = 0;
	while(i < niveau.blocs_terre.taille())
	{
		if(niveau.blocs_terre[i].pos == pos)
			niveau.blocs_terre.enlever(i);
		else
			i++;
	}

	i = 0;
	while(i < niveau.boites.taille())
	{
		if(niveau.boites[i].pos == pos)
			niveau.boites.enlever(i);
		else
			i++;
	}

	i = 0;
	while(i < niveau.oeufs.taille())
	{
		if(niveau.oeufs[i] == pos)
			niveau.oeufs.enlever(i);
		else
			i++;
	}

	i = 0;
	while(i < niveau.fleurs.taille())
	{
		if(niveau.fleurs[i].pos == pos)
			niveau.fleurs.enlever(i);
		else
			i++;
	}

	i = 0;
	while(i < niveau.spawns_darkpoussin.taille())
	{
		if(niveau.spawns_darkpoussin[i] == pos)
		{
			niveau.spawns_darkpoussin.enlever(i);
			jeu.enlever_darkpoussin(i);
		}
		else
			i++;
	}

	i = 0;
	while(i < Niveau::MAX_CLEFS)
	{
		if(niveau.clefs[i].pos == pos)
			niveau.clefs[i].existe = false;
		i++;
	}

	i = 0;
	while(i < niveau.ventilateurs.taille())
	{
		if(niveau.ventilateurs[i].pos == pos)
			niveau.ventilateurs.enlever(i);
		else
			i++;
	}

	i = 0;
	while(i < niveau.leviers.taille())
	{
		if(niveau.leviers[i].pos == pos)
			niveau.leviers.enlever(i);
		else
			i++;
	}*/
}
