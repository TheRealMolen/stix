#pragma once

#include <iostream>
#include "text.h"

#include "config.h"

using namespace std;


// layout for our font

const char* font_layout =
"0123456789  "
"ABCDEFGHIJKL"
"MNOPQRSTUVWX"
"YZ-_!.,abd  "
"            ";
const int32_t glyphs_per_row = 12;
size_t num_glyphs = -1;

const int32_t num_chars = 128;
int32_t font_sprites[num_chars];


SDL_Surface* font_surface = NULL;


void print_font_table()
{
    cout << "font table........................." << endl;
    for (int32_t c = 0; c < num_chars; ++c)
    {
        if (c >= 32)
            cout << "char " << char(c) << "  @ " << font_sprites[c] << '\n';
        else
            cout << "char #" << c << " @ " << font_sprites[c] << '\n';
    }
    cout << "-------------------------------" << endl;
}


bool init_text()
{
    font_surface = SDL_LoadBMP("data/font.bmp");
    if (!font_surface)
    {
        cout << "couldn't load font: " << SDL_GetError() << endl;
        return false;
    }

    // build font sprite table from font layout
    memset(font_sprites, 0xff, sizeof(font_sprites));
    num_glyphs = strlen(font_layout);
    for (int32_t c = 0; c < num_chars; ++c)
    {
        auto pos = strchr(font_layout, c);
        if (pos)
            font_sprites[c] = (int32_t)distance(font_layout, pos);
        else
            font_sprites[c] = -1;
    }

    return true;
}

void draw_char(char c, int x, int y, SDL_Surface* screen)
{
    if (c >= num_chars || font_sprites[c] < 0)
        return;

    int32_t sprite = font_sprites[c];

    SDL_Rect glyph;
    glyph.x = width * (sprite % glyphs_per_row);
    glyph.y = height * (sprite / glyphs_per_row);
    glyph.w = width;
    glyph.h = height;

    SDL_Rect dest;
    dest.x = x;
    dest.y = y;
    dest.w = width;
    dest.h = height;

    SDL_BlitSurface(font_surface, &glyph, screen, &dest);
}



void Message::tick()
{
    if (finished()) return;

    if (steptrig[0] || steptrig[2])
    {
        ++pos;
        //cout << "tick to " << pos << ':' << text[pos] << endl;
    }
}

void Message::render(SDL_Surface* screen) const
{
    if (finished()) return;
    if (pos == initpos) return;

    char c = text[pos];
    draw_char(c, 0, 0, screen);
}


