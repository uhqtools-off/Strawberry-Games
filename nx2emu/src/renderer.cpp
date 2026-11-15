#include "renderer.h"
#include <iostream>
#include <cstring>

bool Renderer::init(SDL_Renderer* sdl_renderer, int w, int h) {
    renderer = sdl_renderer;
    screen_w = w;
    screen_h = h;

    // Create streaming texture for VRAM → screen blit
    fb_tex = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_STREAMING,
        FB_W, FB_H
    );
    if (!fb_tex) {
        std::cerr << "[GFX] Failed to create framebuffer texture: " << SDL_GetError() << "\n";
        return false;
    }

    SDL_SetTextureBlendMode(fb_tex, SDL_BLENDMODE_NONE);
    std::cout << "[GFX] Framebuffer texture: " << FB_W << "x" << FB_H << "\n";
    return true;
}

void Renderer::destroy() {
    if (fb_tex) { SDL_DestroyTexture(fb_tex); fb_tex = nullptr; }
}

void Renderer::on_resize(int w, int h) {
    screen_w = w;
    screen_h = h;
}

void Renderer::upload_framebuffer(const uint8_t* rgba, int fb_w, int fb_h) {
    if (!fb_tex || !rgba) return;

    void*   pixels = nullptr;
    int     pitch  = 0;
    if (SDL_LockTexture(fb_tex, nullptr, &pixels, &pitch) != 0) return;

    int copy_w = (fb_w < FB_W) ? fb_w : FB_W;
    int copy_h = (fb_h < FB_H) ? fb_h : FB_H;

    for (int y = 0; y < copy_h; ++y) {
        const uint8_t* src = rgba + y * fb_w * 4;
        uint8_t*       dst = static_cast<uint8_t*>(pixels) + y * pitch;
        std::memcpy(dst, src, copy_w * 4);
    }

    SDL_UnlockTexture(fb_tex);
}

void Renderer::draw_framebuffer() {
    if (!fb_tex) return;

    // Letterbox / pillarbox to maintain 16:9
    float scale_x = (float)screen_w / FB_W;
    float scale_y = (float)screen_h / FB_H;
    float scale   = (scale_x < scale_y) ? scale_x : scale_y;

    int dst_w = (int)(FB_W * scale);
    int dst_h = (int)(FB_H * scale);
    int dst_x = (screen_w - dst_w) / 2;
    int dst_y = (screen_h - dst_h) / 2;

    SDL_Rect dst = { dst_x, dst_y, dst_w, dst_h };
    SDL_RenderCopy(renderer, fb_tex, nullptr, &dst);
}

void Renderer::draw_splash(SDL_Window* win) {
    // Dark background
    SDL_SetRenderDrawColor(renderer, 10, 10, 15, 255);
    SDL_RenderClear(renderer);

    // Center box
    int bw = 420, bh = 180;
    SDL_Rect box = {
        (screen_w - bw) / 2,
        (screen_h - bh) / 2,
        bw, bh
    };

    // Shadow
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180);
    SDL_Rect shadow = { box.x+4, box.y+4, box.w, box.h };
    SDL_RenderFillRect(renderer, &shadow);

    // Box fill
    SDL_SetRenderDrawColor(renderer, 20, 20, 32, 255);
    SDL_RenderFillRect(renderer, &box);

    // Red accent top bar
    SDL_Rect accent = { box.x, box.y, box.w, 4 };
    SDL_SetRenderDrawColor(renderer, 232, 0, 60, 255);
    SDL_RenderFillRect(renderer, &accent);

    // Border
    SDL_SetRenderDrawColor(renderer, 42, 42, 61, 255);
    SDL_RenderDrawRect(renderer, &box);

    // Pixel-art NX2 logo text (block squares)
    auto draw_pixel = [&](int px, int py) {
        SDL_Rect r = { box.x + 20 + px*8, box.y + 20 + py*8, 7, 7 };
        SDL_RenderFillRect(renderer, &r);
    };

    SDL_SetRenderDrawColor(renderer, 232, 0, 60, 255);
    // "N"
    int pts_N[][2] = {{0,0},{0,1},{0,2},{0,3},{0,4},{1,1},{2,2},{3,3},{4,0},{4,1},{4,2},{4,3},{4,4}};
    for (auto& p : pts_N) draw_pixel(p[0], p[1]);
    // "X"
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    int pts_X[][2] = {{6,0},{10,0},{7,1},{9,1},{8,2},{7,3},{9,3},{6,4},{10,4}};
    for (auto& p : pts_X) draw_pixel(p[0], p[1]);
    // "2"
    SDL_SetRenderDrawColor(renderer, 68, 138, 255, 255);
    int pts_2[][2] = {{12,0},{13,0},{14,0},{14,1},{12,2},{13,2},{14,2},{12,3},{12,4},{13,4},{14,4}};
    for (auto& p : pts_2) draw_pixel(p[0], p[1]);

    // Subtitle hint
    // Simple horizontal rule
    SDL_SetRenderDrawColor(renderer, 42, 42, 61, 255);
    SDL_Rect rule = { box.x + 20, box.y + 70, box.w - 40, 1 };
    SDL_RenderFillRect(renderer, &rule);

    // Dot indicators (like "no ROM" status)
    SDL_SetRenderDrawColor(renderer, 100, 100, 130, 255);
    int tx = box.x + 20, ty = box.y + 85;
    for (int i = 0; i < 3; ++i) {
        SDL_Rect dot = { tx + i*14, ty, 8, 8 };
        SDL_RenderFillRect(renderer, &dot);
    }

    // "No ROM loaded" label area — colored blocks
    SDL_SetRenderDrawColor(renderer, 232, 0, 60, 200);
    SDL_Rect pill = { box.x + 20, box.y + 105, 180, 24 };
    SDL_RenderFillRect(renderer, &pill);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_Rect pill_inner = { box.x + 24, box.y + 112, 60, 10 };
    SDL_RenderFillRect(renderer, &pill_inner);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 80);
    SDL_Rect pill_inner2 = { box.x + 90, box.y + 112, 100, 10 };
    SDL_RenderFillRect(renderer, &pill_inner2);

    // Footer
    SDL_SetRenderDrawColor(renderer, 50, 50, 70, 255);
    SDL_Rect footer = { box.x, box.y + bh - 32, bw, 32 };
    SDL_RenderFillRect(renderer, &footer);
    // green dot = running
    SDL_SetRenderDrawColor(renderer, 0, 230, 118, 255);
    SDL_Rect gdot = { box.x + 16, box.y + bh - 20, 8, 8 };
    SDL_RenderFillRect(renderer, &gdot);
}
