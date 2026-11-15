#pragma once
#include <SDL2/SDL.h>
#include <cstdint>

class Renderer {
public:
    bool init(SDL_Renderer* sdl_renderer, int w, int h);
    void destroy();

    void draw_framebuffer();
    void draw_splash(SDL_Window* win);
    void on_resize(int w, int h);

    // Upload raw RGBA framebuffer to GPU texture
    void upload_framebuffer(const uint8_t* rgba, int fb_w, int fb_h);

private:
    SDL_Renderer* renderer = nullptr;
    SDL_Texture*  fb_tex   = nullptr;
    int screen_w = 0;
    int screen_h = 0;

    // NX2 internal framebuffer resolution
    static constexpr int FB_W = 1280;
    static constexpr int FB_H = 720;
};
