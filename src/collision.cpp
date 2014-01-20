#include <stdio.h>

#include "spb/exec.hpp"
#include "spb/parse.hpp"
#include "main.hpp"
#include "collision.hpp"


void deplacer_poussin(DIRECTION dir)
{
	CParameters parametres;
	parametres.Add(dir);

	CArray<CElement *> *objects = niveau.database->FindAllElements("object");
	for(CArray<CElement *>::CIterator o = objects->GetIterator(); o.Exists(); o.Next())
	{
		CElement *object = o.Get();
		if(!object->Removed() && strcmp(object->GetModel()->GetName(), "Poussin") == 0)
			object->CallFunction("$on_control_move", &parametres);
	}
}

void activer_objet()
{
	CParameters parametres;

	CArray<CElement *> *units = niveau.database->FindAllElements("object");
	for(CArray<CElement *>::CIterator u = units->GetIterator(); u.Exists(); u.Next())
	{
		CElement *unit = u.Get();
		if(!unit->Removed() && strcmp(unit->GetModel()->GetName(), "Poussin") == 0)
			unit->CallFunction("$on_control_use", &parametres);
	}
}
