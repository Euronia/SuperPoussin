#include <stdio.h>
#include <time.h>
#include <SDL/SDL.h>

#ifdef EMSCRIPTEN
#include <emscripten.h>
#endif

#include <base/memory.hpp>

#include "affichage.hpp"
#include "collision.hpp"
#include "editeur.hpp"
#include "event.hpp"
#include "main.hpp"


bool charger_sdl()
{
	SDL_Init(SDL_INIT_VIDEO);
	ecran = SDL_SetVideoMode(640, 512, 32, SDL_HWSURFACE);
	if(!ecran)
		return false;
	SDL_WM_SetCaption("Super Poussin + Bacon VS This Ugly World", 0);
	return true;
}

void quitter_sdl()
{
	SDL_Quit();
}


int run_menu()
{
	int selection = MENUSELECT_PLAY;

	while(1)
	{
		Event event;
		while(recuperer_event(&event))
		{
			if(event.type == EVENT_TYPE_QUIT)
			{
				return MENUSELECT_QUIT;
			}
			else if(event.type == EVENT_TYPE_KEYPRESS)
			{
				if(event.sdl_key == SDLK_RETURN)
					return selection;
				else if(event.sdl_key == SDLK_RIGHT)
				{
					if(selection+1 < NUM_MENUSELECTS)
						selection++;
				}
				else if(event.sdl_key == SDLK_LEFT)
				{
					if(selection-1 >= MENUSELECT_PLAY)
						selection--;
				}
			}
		}
		push_touches_pressees();

		afficher_menu(selection);

		SDL_Flip(ecran);

		SDL_Delay(10);
	}
}

int run_jeu()
{
	bool mode_observation = false;
	int tick = 0;
	int start = clock();
#ifndef EMSCRIPTEN
	while(1)
	{
#endif
		Event event;
		while(recuperer_event(&event))
		{
			if(event.type == EVENT_TYPE_QUIT)
			{
				return MENUSELECT_QUIT;
			}
			else if(event.type == EVENT_TYPE_KEYPRESS)
			{
				if(event.sdl_key == SDLK_ESCAPE)
					return MENUSELECT_MENU;
				else if(event.sdl_key == SDLK_o)
					mode_observation = !mode_observation;

				if(!mode_observation)
				{
					if(event.sdl_key == SDLK_RETURN)
						charger_niveau(niveau.id);
					else if(event.sdl_key == SDLK_SPACE)
						activer_objet();
					else if(event.sdl_key == SDLK_RIGHT)
						deplacer_poussin(DIRECTION_DROITE);
					else if(event.sdl_key == SDLK_LEFT)
						deplacer_poussin(DIRECTION_GAUCHE);
					else if(event.sdl_key == SDLK_UP)
						deplacer_poussin(DIRECTION_HAUT);
					else if(event.sdl_key == SDLK_DOWN)
						deplacer_poussin(DIRECTION_BAS);
				}
			}
			else if(event.type == EVENT_TYPE_KEYPRESSING)
			{
				if(mode_observation)
				{
					if(event.sdl_key == SDLK_RIGHT)
						scroll_droite();
					else if(event.sdl_key == SDLK_LEFT)
						scroll_gauche();
					else if(event.sdl_key == SDLK_UP)
						scroll_haut();
					else if(event.sdl_key == SDLK_DOWN)
						scroll_bas();
				}
			}
		}

		push_touches_pressees();

		update_jeu();

		/*Uint32 fond = SDL_MapRGB(ecran->format, 128, 228, 240);
		SDL_FillRect(ecran, 0, fond);*/
		if(!mode_observation)
			scroll_centrer();
		afficher_jeu();
		SDL_Flip(ecran);

		tick++;
		if(tick%100 == 0)
			printf("%f\n", (float)CLOCKS_PER_SEC/((clock()-start)/(float)tick));
#ifndef EMSCRIPTEN
	}
#endif
	return 0;
}


int run_editeur()
{
	int objet = OBJET_INVALIDE;
	Position souris;
	bool palette_visible = false;
	while(1)
	{
		Event event;
		while(recuperer_event(&event))
		{
			if(event.type == EVENT_TYPE_QUIT)
			{
				return MENUSELECT_QUIT;
			}
			else if(event.type == EVENT_TYPE_KEYPRESS)
			{
				if(event.sdl_key == SDLK_ESCAPE)
					return MENUSELECT_MENU;
				else if(event.sdl_key == SDLK_RETURN)
					sauver_niveau();
				else if(event.sdl_key == SDLK_SPACE)
					palette_visible = true;
			}
			else if(event.type == EVENT_TYPE_KEYRELEASE)
			{
				if(event.sdl_key == SDLK_SPACE)
					palette_visible = false;
			}
			else if(event.type == EVENT_TYPE_KEYPRESSING)
			{
				if(event.sdl_key == SDLK_RIGHT)
					scroll_droite();
				else if(event.sdl_key == SDLK_LEFT)
					scroll_gauche();
				else if(event.sdl_key == SDLK_UP)
					scroll_haut();
				else if(event.sdl_key == SDLK_DOWN)
					scroll_bas();
			}
			else if(event.type == EVENT_TYPE_MOUSEPRESS)
			{
				if(event.sdl_button == SDL_BUTTON_LEFT)
				{
					if(palette_visible)
						objet = choisir_objet(event.souris);
					else
						ajouter_objet(event.souris, objet);
				}
				else if(event.sdl_button == SDL_BUTTON_RIGHT)
				{
					if(objet != OBJET_INVALIDE)
						objet = OBJET_INVALIDE;
					else
						supprimer_objets(event.souris);
				}
			}

			if(event.type == EVENT_TYPE_MOUSEPRESS || event.type == EVENT_TYPE_MOUSERELEASE || event.type == EVENT_TYPE_MOUSEMOTION)
				souris = event.souris;
		}
		push_touches_pressees();

		/*Uint32 fond = SDL_MapRGB(ecran->format, 128, 228, 240);
		SDL_FillRect(ecran, 0, fond);*/
		afficher_jeu();
		if(palette_visible)
			afficher_palette();
		else if(objet != OBJET_INVALIDE)
			afficher_objet_souris(souris, objet);
		SDL_Flip(ecran);

		SDL_Delay(10);
	}
}


int run()
{
	if(!charger_sdl())
		return 1;

#ifndef EMSCRIPTEN
	int select = MENUSELECT_MENU;
	while(1)
	{
		if(select == MENUSELECT_MENU)
		{
			select = run_menu();
		}
		else if(select == MENUSELECT_PLAY)
		{
			charger_niveau(0); //////////////////////////////////////////////////////////////////////////
			select = run_jeu();
		}
		else if(select == MENUSELECT_LEVELS)
		{
			charger_niveau(1); //////////////////////////////////////////////////////////////////////////
			select = run_editeur();
		}
		else
			break;
	}

	niveau.dispose();

	decharger_images();
	quitter_sdl();
#else
	charger_niveau(1);
	printf("level loaded!\n");
	emscripten_set_main_loop([]() { run_jeu(); }, 60, true);
	niveau.dispose();
#endif

	MEM_DUMP("memory.txt");
	MEM_CHECK();

	return 0;
}
