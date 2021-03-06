#ifndef _Sprite_H
#define _Sprite_H

#include <SDL2/SDL.h>

class Renderer;

class Sprite {
public:
    Sprite(const char *filename, const Renderer &renderer);

    ~Sprite();

    Sprite(const Sprite&) = delete;

    Sprite& operator =(const Sprite&) = delete;

    unsigned int getWidth() const {
        return width_;
    }

    unsigned int getHeight() const {
        return height_;
    }

    void draw(int x, int y, const double &angle, const Renderer &renderer) const;
    void draw(int x, int y, unsigned int width, unsigned int height, const double &angle, const Renderer &renderer) const;

private:
    void tidyup();

    SDL_Texture *texture_;
    unsigned int width_;
    unsigned int height_;
};

#endif  // _Sprite_H
