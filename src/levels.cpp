#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "affichage.hpp"
#include "main.hpp"


void lire_ligne(const char *data)
{
	char nom_objet[32] = {0};
	Niveau::TableauParametres parametres;
	parametres.init();

	char str[32];
	const char *start = data;

	while(1)
	{
		if(*data == 0 || *data == '\r' || *data == '\n' || *data == ' ')
		{
			int len = data-start;
			memcpy(str, start, len);
			str[len] = 0;
			start = data+1;
			if(nom_objet[0])
				parametres.ajouter(atoi(str));
			else
				strcpy(nom_objet, str);
			if(*data == 0 || *data == '\r' || *data == '\n')
				break;
		}
		data++;
	}

	if(nom_objet[0])
		niveau.ajouter_objet(nom_objet, &parametres);

	if(*data == 0)
		return;
	else
	{
		while(*data == '\r' || *data == '\n')
			data++;
		lire_ligne(data);
	}
}

void charger_niveau(int id)
{
	char nom_fichier[256];
	sprintf(nom_fichier, "levels/level%d.txt", id);
	printf("loading %s\n", nom_fichier);

	FILE *fichier = fopen(nom_fichier, "rb");

	if(!fichier)
	{
		if(id != 1)
			charger_niveau(1);
		return;
	}

	char data[4096];
	int len = fread(data, 1, sizeof(data), fichier);
	data[len] = 0;
	fclose(fichier);

	niveau.reset();
	niveau.id = id;

	lire_ligne(data);

	scroll_init();
}

void ecrire_ligne(FILE *fichier, char *ligne)
{
	fwrite(ligne, 1, strlen(ligne), fichier);
	fwrite("\r\n", 1, 2, fichier);
}

void sauver_niveau()
{
	char nom_fichier[256];
	sprintf(nom_fichier, "levels/edit-level%d.txt", niveau.id);
	printf("saving %s\n", nom_fichier);

	FILE *fichier = fopen(nom_fichier, "wb");
	if(!fichier)
		return;

	char ligne[256];

	/*sprintf(ligne, "spawn %d %d", niveau.spawn.x, niveau.spawn.y);
	ecrire_ligne(fichier, ligne);*/

	fclose(fichier);
}
