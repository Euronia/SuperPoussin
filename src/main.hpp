#ifndef MAIN_HPP
#define MAIN_HPP

#include "general.hpp"


enum
{
	MENUSELECT_MENU=0,
	MENUSELECT_PLAY,
	MENUSELECT_LEVELS,
	MENUSELECT_QUIT,
	NUM_MENUSELECTS
};

struct Niveau
{
	class CModelsDatabase *models_database;
	class CDatabase *database;
	int id;
	bool game_over_death;
	bool game_over_win;

	Niveau();
	void dispose();
	void parse_spb(const char *filename);
	void reset();

	typedef Tableau<10, int> TableauParametres;
	void ajouter_objet(const char *nom, TableauParametres *parametres);
};
extern Niveau niveau;


// main.cpp
int run();

// levels.cpp
void charger_niveau(int id);
void sauver_niveau();

// jeu.cpp
void update_jeu();


#endif
