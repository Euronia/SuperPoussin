#ifndef EVENT_HPP
#define EVENT_HPP

#include "general.hpp"


struct Event
{
	int type;
	int sdl_key;
	int sdl_button;
	Position souris;
};


enum
{
	EVENT_TYPE_INVALID=0,
	EVENT_TYPE_QUIT,
	EVENT_TYPE_KEYPRESS,
	EVENT_TYPE_KEYRELEASE,
	EVENT_TYPE_KEYPRESSING,
	EVENT_TYPE_MOUSEPRESS,
	EVENT_TYPE_MOUSERELEASE,
	EVENT_TYPE_MOUSEMOTION
};


bool touches_pressees[SDLK_LAST];

bool recuperer_event(Event *event)
{
	SDL_Event e;
	int ok = SDL_PollEvent(&e);
	if(!ok)
		return false;

	if(e.type == SDL_QUIT)
		event->type = EVENT_TYPE_QUIT;
	else if(e.type == SDL_KEYDOWN)
		event->type = EVENT_TYPE_KEYPRESS;
	else if(e.type == SDL_KEYUP)
		event->type = EVENT_TYPE_KEYRELEASE;
	else if(e.type == SDL_USEREVENT)
		event->type = EVENT_TYPE_KEYPRESSING;
	else if(e.type == SDL_MOUSEBUTTONDOWN)
		event->type = EVENT_TYPE_MOUSEPRESS;
	else if(e.type == SDL_MOUSEBUTTONUP)
		event->type = EVENT_TYPE_MOUSERELEASE;
	else if(e.type == SDL_MOUSEMOTION)
		event->type = EVENT_TYPE_MOUSEMOTION;
	else
		event->type = EVENT_TYPE_INVALID;

	if(e.type == SDL_KEYDOWN || e.type == SDL_KEYUP)
		event->sdl_key = e.key.keysym.sym;
	else if(e.type == SDL_USEREVENT)
		event->sdl_key = e.user.code;
	else
		event->sdl_key = SDLK_UNKNOWN;

	if(e.type == SDL_MOUSEBUTTONDOWN || e.type == SDL_MOUSEBUTTONUP)
		event->souris = Position(e.button.x, e.button.y);
	else if(e.type == SDL_MOUSEMOTION)
		event->souris = Position(e.motion.x, e.motion.y);
	else
		event->souris = Position(0, 0);

	if(e.type == SDL_MOUSEBUTTONDOWN || e.type == SDL_MOUSEBUTTONUP)
		event->sdl_button = e.button.button;
	else
		event->sdl_button = 0;

	if(e.type == SDL_KEYDOWN)
		touches_pressees[e.key.keysym.sym] = true;
	else if(e.type == SDL_KEYUP)
		touches_pressees[e.key.keysym.sym] = false;

	return true;
}

void push_touches_pressees()
{
	for(int t = 0; t < SDLK_LAST; t++)
	{
		if(!touches_pressees[t])
			continue;
		SDL_Event e;
		e.type = SDL_USEREVENT;
		e.user.code = t;
		SDL_PushEvent(&e);
	}
}


#endif
