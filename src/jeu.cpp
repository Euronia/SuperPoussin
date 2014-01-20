#include <stdio.h>

#include "spb/exec.hpp"
#include "main.hpp"


void update_jeu()
{
	CParameters parameters;

	CArray<CElement *> *objects = niveau.database->FindAllElements("object");
	for(CArray<CElement *>::CIterator o = objects->GetIterator(); o.Exists(); o.Next())
	{
		CElement *object = o.Get();
		if(!object->Removed())
			object->CallFunction("$on_tick", &parameters);
	}

	niveau.database->Cleanup();

	if(niveau.game_over_win)
		charger_niveau(niveau.id+1);
	else if(niveau.game_over_death)
		charger_niveau(niveau.id);
}
