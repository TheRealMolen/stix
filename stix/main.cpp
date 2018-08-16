#include <algorithm>
#include <cmath>
#include <iostream>
#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>

#include "config.h"
#include "text.h"

using namespace std;


int rezmul = 110;
int swidth = width * rezmul;
int sheight = height * rezmul;


uint32_t ticks_left = 0;
uint32_t ticks_left_beat = 0;

const uint32_t max_steps = 16;

uint32_t steps[max_steps];
uint32_t currstep = ~0u;    // we don't want to miss the first beat!!

bool steptrig[steps_per_beat];

Mix_Chunk* samples[12];
uint32_t numsamples = 12;


uint32_t gfx[width*height];

string cheatcode = "iwin";
uint32_t cheatpos = 0;

Message msg;


class Level
{
    uint32_t len;
    uint32_t maxsample;
    uint32_t pattern[max_steps];

public:
    Level(const char* pat)
    {
        len = (uint32_t)strlen(pat);

        maxsample = 0;
        for (uint32_t i = 0; i < len; ++i)
        {
            char c = pat[i];

            uint32_t val = 0;
            if (c >= '0')
            {
                val = 1 + (c - '0');
                maxsample = max(maxsample, (uint32_t)(c - '0'));
            }

            pattern[i] = val;
        }
    }

    uint32_t getlen() const
    {
        return len;
    }
    const uint32_t* getsteps() const
    {
        return pattern;
    }
    uint32_t getmax() const
    {
        return maxsample;
    }

    bool matches() const
    {
        for (uint32_t i = 0; i < len; ++i)
        {
            if (steps[i] != pattern[i])
                return false;
        }
        return true;
    }
};

Level levels[] = 
{
    Level("0   "),
    Level("0 1 "),
    Level("0 1 0 0 "),
    Level("0 120 0 "),
    Level("0 120   0 1 0 2 "),
    Level("0 0 0  00 1 0 2 "),
};

uint32_t numlevels = sizeof(levels) / sizeof(levels[0]);
uint32_t currlevel = 0;


uint32_t cols[12] = {
    0x445279,   //kick
    0xF3B998,   //hihatclosed
    0xE455D4,   //cowbell
    0xFFE8D1,   //hahatopen
    0x63BAAB,   //snare
    0x00A647,   //midtom
    0x007015,   //lotom
    0xFFF6EB,   //clap
    0x4C8077,   //kick2
    0xB14E80,   //bounce
    0xD9525C,   //bass
    0x7180FF,   //robot
};


uint32_t lighter(uint32_t rgb)
{
    uint32_t r = (rgb >> 16) & 0xff;
    uint32_t g = (rgb >> 8) & 0xff;
    uint32_t b = rgb & 0xff;

    const uint32_t lightness = 125;
    r = min(255u, (r * lightness) / 100);
    g = min(255u, (g * lightness) / 100);
    b = min(255u, (b * lightness) / 100);

    return ((r << 16) | (g << 8) | b);
}


enum EPhase { PHASE_LISTEN, PHASE_RECORD, PHASE_MESSAGE, PHASE_FREE };
EPhase phase = PHASE_MESSAGE;


void init()
{
    memset(steps, 0, sizeof(steps));

    msg = Message("aadbdbda1 2 3 4 STIX bdaGO aGO a");
}


void handle_step_change(int32_t step, bool reverse, bool reset)
{
    uint32_t maxsample = numsamples;
    if (phase != PHASE_FREE)
        maxsample = 1 + levels[currlevel].getmax();

    if (reset)
        steps[step] = 0;
    else if( reverse )
        steps[step] = (steps[step] + maxsample) % (maxsample + 1);
    else
        steps[step] = (steps[step] + 1) % (maxsample + 1);
}


void update_cheat(int32_t key)
{
    if (cheatpos < cheatcode.length())
    {
        if (key == cheatcode[cheatpos])
        {
            cheatpos++;
            if (cheatpos == cheatcode.length())
            {
                currlevel = numlevels;
                phase = PHASE_MESSAGE;
                currstep = ~0u;
                msg = Message("CHEATbdb");
            }
        }
        else
            cheatpos = 0;
    }
}


void update(uint32_t tickdelta)
{
    // have we just finished a message?
    if (phase == PHASE_MESSAGE && msg.finished())
    {
        if (currlevel < numlevels)
            phase = PHASE_LISTEN;
        else
            phase = PHASE_FREE;
    }

    // reset all of our trigs
    memset(steptrig, 0, sizeof(steptrig));

    ticks_left -= tickdelta;
    if (ticks_left + 0xf0000 < 0xf0000)
    {
        uint32_t patternlength = (currlevel < numlevels) ? levels[currlevel].getlen() : max_steps;

        ticks_left += ticks_per_step;
        currstep = (currstep + 1) % patternlength;

        // if we've just finished a pattern, switch phase
        if (currstep == 0)
        {
            if (phase == PHASE_RECORD)
            {
                if (levels[currlevel].matches())
                {
                    phase = PHASE_MESSAGE;
                    currlevel = currlevel + 1;

                    if (currlevel >= numlevels) {
                        msg = Message("YOU  WIN  abdb FREE PLAY");
                    }
                    else {
                        msg = Message("YEAHbd NEXTLEVEL");
                    }
                }
                else
                    phase = PHASE_LISTEN;
            }
            else if( phase == PHASE_LISTEN )
                phase = PHASE_RECORD;
        }

        steptrig[currstep % steps_per_beat] = true;
        //cout << "trigs: " << (int)steptrig[0] << (int)steptrig[1] << (int)steptrig[2] << (int)steptrig[3];

        const uint32_t* srcpat = steps;
        if (phase == PHASE_LISTEN || (phase == PHASE_MESSAGE && currlevel < numlevels))
        {
            srcpat = levels[currlevel].getsteps();
        }
        if (srcpat[currstep])
        {
            uint32_t sample = srcpat[currstep] - 1;
            Mix_PlayChannel(-1, samples[sample], 0);
            //cout << "   play " << currstep;
        }

        //cout << endl;
    }

    msg.tick();

    // also update the beat ticker
    ticks_left_beat -= tickdelta;
    if (ticks_left_beat + 0xf0000 < 0xf0000)
        ticks_left_beat += ticks_per_beat;
}

void draw()
{
    uint32_t bgcol = 0x54433A;
    if (phase == PHASE_RECORD || phase == PHASE_FREE)
        bgcol = 0xdf7126;
    for (uint32_t i = 0; i < width*height; ++i)
        gfx[i] = bgcol;

    // copy steps into gfx
    uint32_t length = max_steps;
    if (currlevel < numlevels)
        length = levels[currlevel].getlen();

    uint32_t yoffs = (length <= 8) ? 1 : 0;
    for (uint32_t y = 0; y < length/4; ++y)
    {
        for (uint32_t x = 0; x < 4; ++x)
        {
            uint32_t stepnum = (y * 4) + x;
            uint32_t step = steps[stepnum];
            uint32_t col = (step > 0) ? cols[step - 1] : 0x402E32;
            if( stepnum == currstep && (phase != PHASE_LISTEN) )
                col = lighter(col);

            gfx[7 + ((y+yoffs) * 6) + x] = col;
        }
    }

    float metro = (float)ticks_left_beat / (float)ticks_per_beat;
    uint32_t met_i = 55 + (uint32_t)(200 * powf(metro, 1.5f));
    uint32_t met = met_i | (met_i << 8) | (met_i << 16);
    gfx[0] = gfx[5] = gfx[30] = gfx[35] = met;
}


int get_step_from_sym(SDL_Keycode sym)
{
    switch (sym)
    {
    case SDLK_1:    return 0;
    case SDLK_2:    return 1;
    case SDLK_3:    return 2;
    case SDLK_4:    return 3;
    case SDLK_q:    return 4;
    case SDLK_w:    return 5;
    case SDLK_e:    return 6;
    case SDLK_r:    return 7;
    case SDLK_a:    return 8;
    case SDLK_s:    return 9;
    case SDLK_d:    return 10;
    case SDLK_f:    return 11;
    case SDLK_z:    return 12;
    case SDLK_x:    return 13;
    case SDLK_c:    return 14;
    case SDLK_v:    return 15;
    }
    return -1;
}


bool loadsample(const char* file, int id)
{
    samples[id] = Mix_LoadWAV(file);
    if (!samples[id])
    {
        cout << "couldn't load sample " << file << ": " << Mix_GetError() << endl;
        return false;
    }
    return true;
}



int main(int argc, char * argv[])
{
    // ---------- the grownup bit ---------------
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) < 0)
    {
        cout << "SDL initialization failed. SDL Error: " << SDL_GetError();
        return -1;
    }

    // figure out appropriate window size
    SDL_DisplayMode displaymode;
    if (SDL_GetDesktopDisplayMode(0, &displaymode))
    {
        cout << "couldn't read the display mode: " << SDL_GetError() << endl;
        return -2;
    }

    // 80% of screen height seems good
    int desiredh = (80 * displaymode.h) / 100;
    rezmul = desiredh / height;
    swidth = width * rezmul;
    sheight = height * rezmul;

    if (!init_text())
    {
        cout << "no text = no game :/" << endl;
        return -4;
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 128) < 0)
    {
        cout << "Couldn't open audio device: " << Mix_GetError() << endl;
        return -2;
    }
    Mix_Init( 0 );

    if (!loadsample("data/kick.wav", 0) ||
        !loadsample("data/hihatclosed.wav", 1) ||
        !loadsample("data/cowbell.wav", 2) ||
        !loadsample("data/hahatopen.wav", 3) ||
        !loadsample("data/snare.wav", 4) ||
        !loadsample("data/midtom.wav", 5) ||
        !loadsample("data/lotom.wav", 6) ||
        !loadsample("data/clap.wav", 7) ||
        !loadsample("data/kick2.wav", 8) ||
        !loadsample("data/bounce.wav", 9) ||
        !loadsample("data/bass.wav", 10) ||
        !loadsample("data/robot.wav", 11) )
    {
        return -3;
    }

    Mix_AllocateChannels(16);

    auto window = SDL_CreateWindow("stix",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        swidth, sheight, 0);
    if (!window)
    {
        cout << "couldn't open window T_T -- " << SDL_GetError() << endl;
        return -100;
    }

    SDL_ShowCursor(SDL_DISABLE);

    auto backbuf = SDL_GetWindowSurface(window);
    auto screen = SDL_CreateRGBSurface(0, width, height, 32, 0, 0, 0, 0);

    //------------ the game bit! -------------

    init();

    uint32_t last_tick = SDL_GetTicks();
    bool play = true;
    while( play ) {
        // frametime
        uint32_t now_tick = SDL_GetTicks();
        uint32_t tickdelta = now_tick - last_tick;
        last_tick = now_tick;

        SDL_Event e;
        while (SDL_PollEvent(&e) != 0)
        {
            if (e.type == SDL_QUIT)
                play = false;
            else if (e.type == SDL_KEYDOWN)
            {
                int32_t step = get_step_from_sym(e.key.keysym.sym);
                bool shift = !!(e.key.keysym.mod & KMOD_SHIFT);
                bool alt = !!(e.key.keysym.mod & KMOD_ALT);
                if (step >= 0)
                    handle_step_change(step, shift, alt);
                else if (e.key.keysym.sym == SDLK_ESCAPE)
                    play = false;

                update_cheat(e.key.keysym.sym);
            }
        }

        update(tickdelta);
        draw();


        // flip
        SDL_LockSurface(screen);
        memcpy(screen->pixels, gfx, sizeof(gfx));
        SDL_UnlockSurface(screen);

        // draw text on top!
        if (!msg.finished())
            msg.render(screen);

        SDL_Rect fullscreen;
        fullscreen.x = 0;
        fullscreen.y = 0;
        fullscreen.w = swidth;
        fullscreen.h = sheight;
        SDL_BlitScaled(screen, NULL, backbuf, &fullscreen);
        SDL_UpdateWindowSurface(window);
    }

    // -------- the really grownup bit ----------------

    SDL_ShowCursor(SDL_ENABLE);
    SDL_FreeSurface(screen);
    SDL_FreeSurface(backbuf);
    Mix_Quit();
    SDL_Quit();

    return 0;
}