#include <stdio.h>
#include <string.h>

#include <base/memory.hpp>

#include "spb/exec.hpp"
#include "spb/parse.hpp"
#include "main.hpp"


Niveau niveau;

static void PrintCallback(int Line, const char *pFilename, const CParameters *paParameters)
{
	printf("called print at line %d from file '%s': ", Line, pFilename);
	for(int p = 0; p < paParameters->Size(); p++)
		printf("%d ", paParameters->GetAt(p));
	printf("\n");
}

static void GameOverCallback(int Line, const char *pFilename, const CParameters *paParameters)
{
	if(paParameters->Size() != 1)
	{
		printf("invalid game over call at line %d from file '%s'\n", Line, pFilename);
		return;
	}

	int Result = paParameters->GetAt(0);
	printf("game over %d\n", Result);
	if(Result)
		niveau.game_over_win = true;
	else
		niveau.game_over_death = true;
}


Niveau::Niveau()
{
	models_database = 0;
	database = 0;
}

void Niveau::dispose()
{
	if(models_database)
	{
		MEM_DELETE(models_database);
		models_database = 0;
	}
	if(database)
	{
		MEM_DELETE(database);
		database = 0;
	}
}

void Niveau::parse_spb(const char *filename)
{
	CDataPointer ptr(filename);
	bool success = models_database->Parse(&ptr);
	printf("parse success '%s' %d\n", filename, success);
}

void Niveau::reset()
{
	dispose();
	game_over_death = false;
	game_over_win = false;

	models_database = MEM_NEW(CModelsDatabase);

	models_database->AddConstant("true", 1);
	models_database->AddConstant("false", 0);
	models_database->AddConstant("right", DIRECTION_DROITE);
	models_database->AddConstant("left", DIRECTION_GAUCHE);
	models_database->AddConstant("up", DIRECTION_HAUT);
	models_database->AddConstant("down", DIRECTION_BAS);

	models_database->AddElementType("object");
	models_database->AddElementType("item");

	CDataPointer DataPtr("spb/list.txt");
	while(!DataPtr.EndReached())
	{
		char ligne[256];
		DataPtr.ReadLine(ligne);
		if(ligne[0] && strncmp(ligne, "//", 2) != 0)
		{
			char filename[256];
			sprintf(filename, "spb/scripts/%s", ligne);
			parse_spb(filename);
		}
	}

	models_database->Finish();

	database = MEM_NEW(CDatabase(models_database));

	database->AddSysFunction("print", PrintCallback);
	database->AddSysFunction("game_over", GameOverCallback);

	database->AddDefaultAttributeParameter("object", "$x", false);
	database->AddDefaultAttributeParameter("object", "$y", false);
	database->AddDefaultAttribute("object", "$image_id", -1, false);
	database->AddDefaultAttribute("object", "$image_visible", 1, false);
	database->AddDefaultAttribute("object", "$solid", 1, false);

	// TODO: remove these
	database->AddDefaultAttribute("object", "$gravite_x", 0, false);
	database->AddDefaultAttribute("object", "$gravite_y", 0, false);
	database->AddDefaultAttribute("object", "$climbable", 0, false);
	database->AddDefaultAttribute("object", "$pushable", 0, false);
	database->AddDefaultAttribute("object", "$fall", 0, false);
	database->AddDefaultAttribute("object", "$heavy", 0, false);
	//

	database->AddDefaultAttribute("item", "$image_id", -1, false);
	database->AddDefaultAttribute("item", "$image_visible", 1, false);

	printf("done\n");
}

void Niveau::ajouter_objet(const char *nom, TableauParametres *parametres)
{
	int t = parametres->taille();
	CParameters add_parameters;
	for(int p = 0; p < t; p++)
		add_parameters.Add(parametres->get(p));
	database->AddElement("object", nom, &add_parameters);
}
