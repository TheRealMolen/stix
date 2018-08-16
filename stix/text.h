#pragma once

#include <string>
#include <SDL/SDL.h>

using namespace std;


bool init_text();

void draw_char(char c, int x, int y, SDL_Surface* screen);


class Message
{
    size_t pos;
    string text;

    static const size_t initpos = ((size_t)0) - 1;

public:

    Message() = default;
    Message(const string& msg) : pos(initpos), text(msg)
    {
    }

    void tick();

    void render(SDL_Surface* screen) const;

    bool finished() const
    {
        return (pos != initpos ) && (text.size() <= pos);
    }
};

