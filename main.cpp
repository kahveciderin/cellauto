#include <stdlib.h>

#include <SDL2/SDL.h>
#include <cmath>
#include <iostream>
#include <fstream>
#include <string>
using namespace std;

//#define MOVEFIRST 1
#define GRIDSIZE 256
#define SCALE 2

#define STARTX 128
#define STARTY 128

#define ALWAYSRENDER 1

#define REFRESH 1

#define RENDER 2000000
//#define RGB

//#define DEBUGLABELS 1

int main(int argc, char *argv[])
{

    ifstream file(argv[1]);
    // string line ;
    // while( getline( file, line )){}

    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_CreateWindowAndRenderer(GRIDSIZE * SCALE, GRIDSIZE * SCALE, 0, &window, &renderer);

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);

    class state
    {
    public:
        int8_t b_move;
        int8_t b_turn;
        int8_t b_alter;
        int b_nextstate;

        int8_t w_move;
        int8_t w_turn;
        int8_t w_alter;
        int w_nextstate;

        void add(int8_t bmo, int8_t btu, int8_t bal, int bns, int8_t wmo, int8_t wtu, int8_t wal, int wns)
        {
            b_move = bmo;
            b_turn = btu;
            b_alter = bal;
            b_nextstate = bns;
            w_move = wmo;
            w_turn = wtu;
            w_alter = wal;
            w_nextstate = wns;
        }
    };

    class ant
    {
    public:
        int state;
        int32_t x, y;
        uint8_t rot = 64;

        void rotate(int8_t rotation)
        {
            rot = (rot + rotation);
        }

        void move(int8_t move)
        {
            switch (rot % 4)
            {
            case 0:
                x += move;
                break;

            case 1:
                y -= move;
                break;

            case 2:
                x -= move;
                break;

            case 3:
                y += move;
                break;
            }

            if (x > GRIDSIZE - 1)
                x = 0;
            if (x < 0)
                x = GRIDSIZE - 1;
            if (y > GRIDSIZE - 1)
                y = 0;
            if (y < 0)
                y = GRIDSIZE - 1;
        }
    };

    state states[0x10000];

#include "adder.h"
#include "debug.h"

    ant machine;

    machine.x = STARTX;
    machine.y = STARTY;
    machine.state = 0;
    machine.rot = 64;

    class line
    {

    public:
        uint32_t chunks[GRIDSIZE / 32];
        void init()
        {
            for (int i = 0; i < GRIDSIZE / 32; i++)
            {
                chunks[i] = 0x0;
            }
        }

        bool getbit(uint16_t bit)
        {
            return (chunks[bit >> 5] & (1 << (bit & 0x1F)));
        }

        void togglebit(uint16_t bit)
        {
            chunks[bit >> 5] ^= 1 << (bit & 0x1F);
        }
    };

    SDL_RenderSetScale(renderer,
                       SCALE,
                       SCALE);

    line lines[GRIDSIZE];

    for (int i = 0; i < GRIDSIZE; i++)
    {
        lines[i].init();
    }

    bool run = 1;
    SDL_Event evt;
    uint64_t step;
    while (run)
    {


        
        bool render = ALWAYSRENDER;
#ifdef DEBUGLABELS
        printf("x: %d y: %d rot: %d state: %s (%d) STEP: %lu\n", machine.x, machine.y, machine.rot, debuglabels[machine.state], machine.state, step);
#endif

        bool blackpixelsmatter = lines[machine.y].getbit(machine.x);

        

#ifdef MOVEFIRST
        machine.move(blackpixelsmatter ? states[machine.state].w_move : states[machine.state].b_move);
        machine.rotate(blackpixelsmatter ? states[machine.state].w_turn : states[machine.state].b_turn);
#else
        machine.rotate(blackpixelsmatter ? states[machine.state].w_turn : states[machine.state].b_turn);
        machine.move(blackpixelsmatter ? states[machine.state].w_move : states[machine.state].b_move);
#endif


        if ((blackpixelsmatter ? states[machine.state].w_alter : states[machine.state].b_alter))
        {
            lines[machine.y].togglebit(machine.x);
            render = 1;
        }

        machine.state = blackpixelsmatter ? states[machine.state].w_nextstate : states[machine.state].b_nextstate;
        
        if(step % RENDER == 0){
        if (render)
        {

#if ALWAYSRENDER
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);
#ifdef RGB
            srand(step);
#endif
            for (int i = 0; i < GRIDSIZE; i++)
            {
                for (int j = 0; j < GRIDSIZE; j++)
                {
                    if (lines[i].getbit(j))
                    {
#ifdef RGB

                        SDL_SetRenderDrawColor(renderer, rand() % 256, rand() % 256, rand() % 256, 255);
#else
                        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
#endif
                        SDL_RenderDrawPoint(renderer, j, i);
                    }
                }
            }
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 0);
            SDL_RenderDrawPoint(renderer, machine.x, machine.y);
#else
            //SDL_RenderClear(renderer);

            if (step % REFRESH == 0)
            {
                for (int i = 0; i < GRIDSIZE; i++)
                {
                    for (int j = 0; j < GRIDSIZE; j++)
                    {
                        if (lines[i].getbit(j))
                        {
#ifdef RGB

                            SDL_SetRenderDrawColor(renderer, rand() % 256, rand() % 256, rand() % 256, 255);
#else
                            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
#endif
                            SDL_RenderDrawPoint(renderer, j, i);
                        }
                    }
                }
            }
            else
            {
                if (lines[machine.y].getbit(machine.x))
                {
#ifdef RGB
                    srand(step);
                    SDL_SetRenderDrawColor(renderer, rand() % 256, rand() % 256, rand() % 256, 255);
#else
                    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
#endif
                }
                else
                {
                    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
                }
                SDL_RenderDrawPoint(renderer, machine.x, machine.y);
            }

#endif

            SDL_RenderPresent(renderer);
        }

        }
        step++;
        while (SDL_PollEvent(&evt))
        {
            switch (evt.type)
            {
            case SDL_QUIT:
                run = 0;
                break;
            }
        }
        //char a;
        //scanf("%c", &a);
    }
}
