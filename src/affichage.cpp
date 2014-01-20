#include <stdio.h>
#include <SDL/SDL.h>

#include <base/string.hpp>
#include <base/tree.hpp>

#include "spb/exec.hpp"
#include "spb/parse.hpp"
#include "affichage.hpp"
#include "editeur.hpp"
#include "main.hpp"


#define DATADIR_GUI "data"
#define DATADIR_SPB "spb/data"

#define IMAGE_FOND_MENU "fond_menu"
#define IMAGE_MENUSELECT "menuselect"
#define IMAGE_FOND_JEU "fond_jeu"
#define IMAGE_ITEM_SLOT "item_slot"


enum
{
	TAILLE_CASE = 32
};

struct ImageComparator
{
	const char *nom;
	int image_id;
};

struct Image
{
	CString nom;
	int image_id;
	SDL_Surface *img;
	inline int Compare(const ImageComparator &comp)
	{
		int n = strcmp(nom, comp.nom);
		if(n == 0)
			return image_id - comp.image_id;
		else
			return n;
	}
};


SDL_Surface *ecran;
CTree<Image> images;

int scroll_x;
int scroll_y;


SDL_Surface *charger_image(const char *dir, const char *nom, int image_id=-1)
{
	ImageComparator comp = {nom, image_id};
	Image *image = images.Find(comp);
	if(image)
		return image->img;

	char filename[256];
	if(image_id >= 0)
		sprintf(filename, "%s/%s%d.bmp", dir, nom, image_id);
	else
		sprintf(filename, "%s/%s.bmp", dir, nom);

	image = &images.Insert(comp);
	image->nom = nom;
	image->image_id = image_id;
	image->img = SDL_LoadBMP(filename);

#ifndef EMSCRIPTEN
	if(image->img)
		SDL_SetColorKey(image->img, SDL_SRCCOLORKEY, SDL_MapRGB(image->img->format, 0, 0, 255));
	else
		printf("Image '%s' non chargee\n", filename);
#endif

	return image->img;
}

void decharger_images()
{
	images.Clear();
}


void afficher(SDL_Surface *img, int x, int y)
{
	SDL_Rect pos_image;
	pos_image.x = x;
	pos_image.y = y;
	SDL_BlitSurface(img, 0, ecran, &pos_image);
}

void afficher_gui(const char *image, int x, int y)
{
	SDL_Surface *img = charger_image(DATADIR_GUI, image);
	if(img)
		afficher(img, x, y);
}


void afficher_menu(int selection)
{
	afficher_gui(IMAGE_FOND_MENU, 0, 0);

	if(selection == MENUSELECT_PLAY)
		afficher_gui(IMAGE_MENUSELECT, 20, 425);
	else if(selection == MENUSELECT_LEVELS)
		afficher_gui(IMAGE_MENUSELECT, 230, 425);
	else if(selection == MENUSELECT_QUIT)
		afficher_gui(IMAGE_MENUSELECT, 425, 425);
}


void afficher_objet(const CElement *objet)
{
	if(!objet->FindAttribute("$image_visible")->m_Value)
		return;

	int image_id = objet->FindAttribute("$image_id")->m_Value;

	SDL_Surface *img = 0;
	const CElementModel *model = objet->GetModel();
	while(!img && model)
	{
		img = charger_image(DATADIR_SPB, model->GetName(), image_id);
		model = model->GetParentModel();
	}
	if(!img)
		return;

	int pos_x = objet->FindAttribute("$x")->m_Value;
	int pos_y = objet->FindAttribute("$y")->m_Value;
	int sol_y = 512 - TAILLE_CASE;
	SDL_Rect pos_image;
	pos_image.x = scroll_x + pos_x*TAILLE_CASE;
	pos_image.y = scroll_y + sol_y - pos_y*TAILLE_CASE;
	SDL_BlitSurface(img, 0, ecran, &pos_image);
}

int item_index;

void afficher_item(const CElement *item)
{
	if(!item->FindAttribute("$image_visible")->m_Value)
		return;

	int image_id = item->FindAttribute("$image_id")->m_Value;

	SDL_Surface *img = 0;
	const CElementModel *model = item->GetModel();
	while(!img && model)
	{
		img = charger_image(DATADIR_SPB, model->GetName(), image_id);
		model = model->GetParentModel();
	}
	if(!img)
		return;

	afficher_gui(IMAGE_ITEM_SLOT, 5+item_index*(TAILLE_CASE+1), 5);

	SDL_Rect pos_image;
	pos_image.x = 6 + item_index*(TAILLE_CASE+1);
	pos_image.y = 6;
	SDL_BlitSurface(img, 0, ecran, &pos_image);

	item_index++;
}

void afficher_jeu()
{
	afficher_gui(IMAGE_FOND_JEU, 0, 0);

	// objets non solides
	CArray<CElement *> *objets = niveau.database->FindAllElements("object");
	for(CArray<CElement *>::CIterator o = objets->GetIterator(); o.Exists(); o.Next())
	{
		const CElement *objet = o.Get();
		if(!objet->Removed() && !objet->FindAttribute("$solid")->m_Value)
			afficher_objet(objet);
	}

	// objets solides
	for(CArray<CElement *>::CIterator o = objets->GetIterator(); o.Exists(); o.Next())
	{
		const CElement *objet = o.Get();
		if(!objet->Removed() && objet->FindAttribute("$solid")->m_Value)
			afficher_objet(objet);
	}

	// items
	item_index = 0;
	CArray<CElement *> *items = niveau.database->FindAllElements("item");
	for(CArray<CElement *>::CIterator i = items->GetIterator(); i.Exists(); i.Next())
	{
		const CElement *item = i.Get();
		if(!item->Removed())
			afficher_item(item);
	}
}


void afficher_objet_souris(Position souris, int objet)
{
	Position pos = case_souris(souris);

	/*afficher(img, pos);*/
}


void afficher_palette()
{
	/*afficher_gui(IMAGE_PALETTE, 0, 0);*/
}



void scroll_init()
{
	scroll_x = 0;
	scroll_y = 0;
}

void scroll_gauche()
{
	scroll_x += TAILLE_CASE;
}

void scroll_droite()
{
	scroll_x -= TAILLE_CASE;
}

void scroll_haut()
{
	scroll_y += TAILLE_CASE;
}

void scroll_bas()
{
	if(scroll_y - TAILLE_CASE > 0)
		scroll_y -= TAILLE_CASE;
	else
		scroll_y = 0;
}

void scroll_centrer()
{
	int pos_x = 0;
	int pos_y = 0;

	CArray<CElement *> *units = niveau.database->FindAllElements("object");
	for(CArray<CElement *>::CIterator u = units->GetIterator(); u.Exists(); u.Next())
	{
		const CElement *unit = u.Get();
		if(unit->Removed())
			continue;
		if(strcmp(unit->GetModel()->GetName(), "Poussin") != 0)
			continue;
		pos_x = unit->FindAttribute("$x")->m_Value;
		pos_y = unit->FindAttribute("$y")->m_Value;
		break;
	}

	scroll_x = -pos_x*TAILLE_CASE + 640/2;
	if(pos_y*TAILLE_CASE > 512/2)
		scroll_y = pos_y*TAILLE_CASE - 512/2;
	else
		scroll_y = 0;
}



Position case_souris(Position souris)
{
	return Position((souris.x - scroll_x)/32, ((512-souris.y) + scroll_y)/32);
}
