/*
    Tetris Clone by Andynonomous -
     January 13th 2017
*/

#include "tetris.h"

static ALLEGRO_FONT *basicFont;
static unsigned char keys[3][ALLEGRO_KEY_MAX];

int main()
{
	ALLEGRO_DISPLAY *display = 0;
	ALLEGRO_EVENT_QUEUE *eventQueue = 0;

	ALLEGRO_TIMER *updateTimer = 0;
	ALLEGRO_TIMER  *displayTimer = 0;
	ALLEGRO_TIMER *dropTimer = 0;
	ALLEGRO_TIMER *moveTimer = 0;

	basicFont = 0;

	int initSuccess = al_init();
	int primitivesOK = al_init_primitives_addon();
	int gotKeyboard = al_install_keyboard();

	updateTimer = al_create_timer(1.0 / UPDATES_PER_SECOND);
	displayTimer = al_create_timer(1.0 / FRAMES_PER_SECOND);
	dropTimer = al_create_timer(DROP_INTERVAL);
	moveTimer = al_create_timer(DROP_INTERVAL * 0.5);

	display = al_create_display(DISPLAY_WIDTH, DISPLAY_HEIGHT);
	eventQueue = al_create_event_queue();

	al_register_event_source(eventQueue, al_get_display_event_source(display));
	al_register_event_source(eventQueue, al_get_keyboard_event_source());
	al_register_event_source(eventQueue, al_get_timer_event_source(updateTimer));
	al_register_event_source(eventQueue, al_get_timer_event_source(displayTimer));
	al_register_event_source(eventQueue, al_get_timer_event_source(dropTimer));
	al_register_event_source(eventQueue, al_get_timer_event_source(moveTimer));

	if (initSuccess && gotKeyboard && primitivesOK && display && eventQueue)
	{
		al_init_font_addon();
		al_init_ttf_addon();
		basicFont = al_load_font("DejaBold.ttf", 38, 0);
		int done = NOT_TRUE_AT_ALL;

		time_t seed;
		srand(time(&seed));

		Assert(GRID_WIDTH < DISPLAY_WIDTH && GRID_HEIGHT < DISPLAY_HEIGHT);

		al_start_timer(updateTimer);
		al_start_timer(displayTimer);

		game_state state = {};

		state.dropTimer = dropTimer;
		state.moveTimer = moveTimer;

		state.dropTime = NOT_TRUE_AT_ALL;
		state.moveTime = VERY_TRUE;

		while(!done)
		{
			int redrawTime = NOT_TRUE_AT_ALL;
			int updateTime = NOT_TRUE_AT_ALL;
			// ----------------------------------------------------- Events
			for(;;)
			{
				ALLEGRO_EVENT ev;
				al_wait_for_event(eventQueue, &ev);
				if(ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
				{
					done = VERY_TRUE;
					break;
				}
				else if(ev.type == ALLEGRO_EVENT_KEY_DOWN)
				{
					if(!keys[HELD_DOWN][ev.keyboard.keycode])
						keys[JUST_PRESSED][ev.keyboard.keycode] = 1;
					keys[HELD_DOWN][ev.keyboard.keycode] = 1;
				}
				else if(ev.type == ALLEGRO_EVENT_KEY_UP)
				{
					keys[HELD_DOWN][ev.keyboard.keycode] = 0;
					keys[RELEASED][ev.keyboard.keycode] = 1;
					if(ev.keyboard.keycode == ALLEGRO_KEY_ESCAPE)
					{
						done = VERY_TRUE;
						break;
					}
				}
				else if (ev.type == ALLEGRO_EVENT_TIMER)
				{
					if (ev.timer.source == updateTimer)
					{
						updateTime = VERY_TRUE;
					}
					else if(ev.timer.source == displayTimer)
					{
						redrawTime = VERY_TRUE;
					}
					else if(ev.timer.source == state.dropTimer)
					{
                        state.dropTime = VERY_TRUE;
					}
					else if(ev.timer.source == state.moveTimer)
					{
						state.moveTime = VERY_TRUE;
						al_stop_timer(state.moveTimer);
					}
				}

				if(al_is_event_queue_empty(eventQueue))
					break;
			}			

			// ----------------------------------------------------- Update & Render
			if(updateTime)
			{
            	done = UpdateGame(&state, keys);

            	memset(keys[RELEASED], 0, ALLEGRO_KEY_MAX);
            	memset(keys[JUST_PRESSED], 0, ALLEGRO_KEY_MAX);
            	state.dropTime = NOT_TRUE_AT_ALL;

            	DebugResetCycleCounts();
			}
			if(redrawTime)
			{
				al_clear_to_color(al_map_rgb(0, 0, 0));

				// ----------------------------------- Drawing

				RenderGame(basicFont, &state);

				// ------------------------------------------- //
  				al_flip_display();

  			}
  			
		}		// ------------------------------------------- Loop End

		al_destroy_display(display);
		al_destroy_event_queue(eventQueue);
		al_destroy_timer(displayTimer);
		al_destroy_timer(updateTimer);
		al_destroy_timer(dropTimer);
		al_destroy_font(basicFont);

		al_shutdown_font_addon();
		al_shutdown_ttf_addon();
		al_shutdown_primitives_addon();

		al_uninstall_keyboard();
	}
	else
	{
		printf("\nAllegro Failed To Initialize\n");
	}

    return 0;
}

void DebugResetCycleCounts()
{
	#if defined(WTF)

	for(unsigned int cycleCountIndex = 0; cycleCountIndex < ArrayCount(cycleCounts); ++cycleCountIndex)
	{
		uint64 *count = &cycleCounts[cycleCountIndex].cycleCount;
		int *hits = &cycleCounts[cycleCountIndex].hitCount;

		if(*count)
			printf("cycleCount[%d] : %lu    Hits : %d\n", cycleCountIndex, *count, *hits);

		*count = 0;
		*hits = 0;
	}
	printf("\n");

	#endif
}

uint64 rdtsc(){
    unsigned int lo,hi;
    __asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
    return ((uint64)hi << 32) | lo;
}
