#include <iostream>
#include <cstdlib>
#include <vector>
#include <string>
#include <ctime>

#include <SDL2/SDL.h>

int main(int argc, char **argv)
{
    SDL_Init(SDL_INIT_EVERYTHING);
    
    SDL_Window *window = SDL_CreateWindow("Voxel Test", 0, 0, 320, 200, 0);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    
    bool running = true;
    SDL_Event event;
    
    // Color palettes
    struct palette_color
    {
        uint8_t r, g, b;
    };
    
    palette_color palette[128];
    int32_t zpal;
    
    for(zpal=0;zpal<64;zpal++)
    {
        palette[zpal].r = zpal * 4;
        palette[zpal].g = zpal * 4;
        palette[zpal].b = zpal * 2;
    }
    
    for(zpal=64;zpal<128;zpal++)
    {
        palette[zpal].r = zpal;
        palette[zpal].g = zpal * 2;
        palette[zpal].b = zpal * 2;
    }
    
    // Map generation
    int16_t height[4096], color[4096]; // Might have to be 4095-elements wide
    
    int16_t mgy, mgx, mgp = 0;
    
    for(mgy=0;mgy<64;mgy++)
    {
        for(mgx=0;mgx<64;mgx++)
        {
            int16_t d = 15 * 15 - pow(((mgx & 31) - 16), 2) - pow(((mgy & 31) - 16), 2);
            
            if(d > 0 && ((mgx ^ mgy) & 32))
            {
                height[mgp] = 64 - sqrt(d);
                color[mgp] = (mgx + mgy) * .5;
            }
            else
            {
                height[mgp] = 64;
                color[mgp] = (cos(mgx * .2) + sin(mgy * .3)) * 3 + 88;
            }
            mgp++;
        }
    }
    
    std::cout << "Hi\n";
    
    // Starting positions
    double posx = 0, posy = 0, posz = 40 << 16;
    double ang = 0.0;
    double horiz = -50;
    int32_t x, y, p;
    
    // Appears to be constants
    double r160 = 1.0 / 160.0;
    
    double d = 0;
    double dd = 65536.0 * r160;
    double de = dd * 128;
    double sdz = (100 - horiz) * 65536 * r160;
    
    while(running)
    {
        while(SDL_PollEvent(&event))
        {
            switch(event.type)
            {
                case(SDL_QUIT): running = false; break;
                case(SDL_KEYDOWN):
                {
                    if(event.key.keysym.sym == SDLK_q)
                    {
                        running = false;
                    }
                }
                break;
            }
        }
        
        // SDL - clear the background
        SDL_SetRenderDrawColor(renderer, 0xAD, 0xEF, 0xFF, 255);
        SDL_RenderClear(renderer);
        
        // More math stuff
        double cosang = cos(ang) * 65536.0, sinang = sin(ang) * 65536.0;
        double dx = sinang + cosang;
        double dxi = -sinang * r160;
        double dy = sinang - cosang; 
        double dyi = cosang * r160;
        
        // For each column...
        int16_t sx;
        
        for(sx=0;sx<320;sx++)
        {
            // Raytrace without floats
            int32_t x = posx, y = posy, z = posz;
            double dz = sdz, p = sx; // p = sx + 63680
            
            // We start at the bottom of the screen and work our way up (320x200)
            int32_t dep = 199;
            
            for(d=0;d<de+1;d=d+dd)
            {
                x = x + dx; 
                y = y + dy;
                z = z + dz;
                
                int32_t i = (((x << 6) & 0xFC00000) + (y & 0x3F0000)) >> 16;
                double h = height[i] << 16;

                while(h < (double)z) 
                {
                    // Draw the pixel at the current location
                    SDL_SetRenderDrawColor(renderer, palette[color[i]].r, palette[color[i]].g, palette[color[i]].b, 255);
                    
                    if(dep > dep-1)
                    {
                        SDL_RenderDrawPoint(renderer, p, dep);
                    }
                    
                    // Move up the screen by one pixel
                    dep = dep - 1; 
                    z = z - d;
                    dz = dz - dd;
                } 
            }
            
            dx = dx + dxi;
            dy = dy + dyi;
            
            // The hill is done drawing, so fill the rest of the line with sky color
            while(dep >= 0)
            {
                SDL_SetRenderDrawColor(renderer, 0xAD, 0xEF, 0xFF, 255);
                SDL_RenderDrawPoint(renderer, p, dep);
                dep = dep - 1;
            }
        }
        
        // Move the camera around
        posx = posx + cosang * .2;
        posy = posy + sinang * .2;
        ang = ang + .002;
        
        // SDL - show it, with a pause for clarity
        SDL_RenderPresent(renderer);
        SDL_Delay(10);
    }
    
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    
    return 0;
}


