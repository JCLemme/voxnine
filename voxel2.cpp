#include <iostream>
#include <algorithm>
#include <cstdlib>
#include <vector>
#include <string>
#include <ctime>
#include <cmath>

#include <SDL2/SDL.h>
#include "lodepng.h"

#define IMAGE_SIZE 176
#define IMAGE_HEIGHT 30976
#define IMAGE_WIDTH 176

struct Vector2
{
    double a, b;
};

struct Vector3
{
    double a, b, c;
};

struct IntersectData
{
    Vector2 a, b;
    double c;
};

IntersectData intersect(Vector2 start, Vector2 direction)
{
    Vector2 aabb[2];
	aabb[0].a = 0; aabb[0].b = 0;
	aabb[1].a = IMAGE_SIZE; aabb[1].b = IMAGE_SIZE;

    int xSign = (1/direction.a < 0.0) ? 1 : 0;
    int ySign = (1/direction.b < 0.0) ? 1 : 0;

    double txmin = (aabb[  xSign].a - start.a) * (1/direction.a);
    double txmax = (aabb[1-xSign].a - start.a) * (1/direction.a);
    double tymin = (aabb[  ySign].b - start.b) * (1/direction.b);
    double tymax = (aabb[1-ySign].b - start.b) * (1/direction.b);
    double tmin = std::max(txmin, tymin);
    double tmax = std::min(txmax, tymax);

    Vector2 begin = {start.a + direction.a*tmin, start.b + direction.b*tmin};
    Vector2 end   = {start.a + direction.a*tmax, start.b + direction.b*tmax};

    double count = 0;
    
    if(tmin < tmax)
    {
	    count = tmax-tmin;
    }

    return IntersectData{begin, end, trunc(count)};
}

int main(int argc, char **argv)
{
    SDL_Init(SDL_INIT_EVERYTHING);

    std::vector<unsigned char> image; //the raw pixels
    unsigned width, height;

    //decode
    unsigned error = lodepng::decode(image, width, height, "weevils.png");

    //if there's an error, display it
    if(error) std::cout << "decoder error " << error << ": " << lodepng_error_text(error) << std::endl;

    std::cout << image.size();
    
	int zSize = height / width;

	int renderHeight = width;
	int renderWidth = width * sqrt(2);

	int imageSize = width;

    uint32_t *renderedImage = new uint32_t[renderWidth*renderHeight];
    memset(renderedImage, 0, renderWidth*renderHeight);
    
	uint32_t** volumes = new uint32_t*[imageSize];
	int volumeSize = (image.size()/imageSize)/4;

	for(int z = 0; z < imageSize; z++)
	{
		volumes[z] = new uint32_t[volumeSize];
	}

	for(int x = 0; x < imageSize; x++)
	{
		for(int y = 0; y < imageSize; y++)
		{
			for(int z = 0; z < imageSize; z++)
			{
				volumes[z][x+y*imageSize] = image[(x+z*imageSize+y*imageSize*imageSize)*4] << 24;
				volumes[z][x+y*imageSize] |= image[(x+z*imageSize+y*imageSize*imageSize)*4+1] << 16;
				volumes[z][x+y*imageSize] |= image[(x+z*imageSize+y*imageSize*imageSize)*4+2] << 8;
				volumes[z][x+y*imageSize] |= image[(x+z*imageSize+y*imageSize*imageSize)*4+3];
				//std::cout << "Point [" << x << " " << y << " " << z << "] is " << (int)volumes[z][x+y*imageSize] << "\n";
			}
		}
	}

	double startAngle = 0;
	double turnsPerSecond = 0.1;
	
	double rayLength = width * sqrt(2);
	double samplingRate = 1;
	double sampleCount = samplingRate*rayLength;
	double sampleDistance = rayLength/sampleCount;

	double *squared = new double[256];
	double *cubed = new double[256];

	for(int i = 0; i < 256; i++)
	{
		squared[i] = pow(i/256, 2);
	}

	for(int i = 0; i < 256; i++)
	{
		cubed[i] = pow(i/256, 3);

	}
	
    SDL_Window *window = SDL_CreateWindow("Voxel Test", 0, 0, 3*renderWidth, 3*renderHeight, 0);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    SDL_RenderSetScale(renderer, 3, 3);
    
    SDL_Texture *viewTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, renderWidth, renderHeight);
 
    /*for(int z = 0; z < imageSize; z++)
    {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        
	    for(int x = 0; x < imageSize; x++)
	    {
		    for(int y = 0; y < imageSize; y++)
		    {
                SDL_SetRenderDrawColor(renderer, volumes[z][x+y*imageSize], volumes[z][x+y*imageSize], volumes[z][x+y*imageSize], 255);
                SDL_RenderDrawPoint(renderer, x, y);
		    }
	    }
	
        SDL_RenderPresent(renderer);
        SDL_Delay(1000);
    }*/
       
    bool running = true;
    SDL_Event event;
	uint32_t startTime = SDL_GetTicks();
	
	double angle = 0;
	
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
        //SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        //SDL_RenderClear(renderer);

        memset(renderedImage, 0, renderWidth*renderHeight);
            
	    angle += 0.01; // = ((SDL_GetTicks()-startTime)/1000)*(2*3.14159265*turnsPerSecond)+startAngle;
	
	    Vector2 increment = {sin(angle)*sampleDistance, cos(angle)*sampleDistance*1.4};
	    double baseX = sin(angle)*rayLength/2 + width/2;
	    double baseY = cos(angle)*rayLength/2 + width/2;
	    double cosAngle = cos(angle);
	    double sinAngle = sin(angle);

	    double *startXPositions = new double[(int)trunc(rayLength)];
	    double *startYPositions = new double[(int)trunc(rayLength)];
	    double *sampleCounts = new double[(int)trunc(rayLength)];

        //std::cout << "BaseX: " << baseX << "\nBaseY: " << baseY << "\nRayLength: " << rayLength << "\n\n";
        
	    for(int i = 0; i < trunc(rayLength); i++)
	    {
		    Vector2 pos = {baseX + cosAngle*((i-rayLength/2)/rayLength)*rayLength, (baseY - sinAngle*((i-rayLength/2)/rayLength)*rayLength)*1.4-imageSize/5};

		    IntersectData intersection = intersect(pos, increment);
		
		    startXPositions[i] = intersection.a.a;
		    startYPositions[i] = intersection.a.b;
		    sampleCounts[i] = intersection.c;
	    }

	    uint32_t* zVolume;

        //std::cout << zSize << "\n";
        
	    for(int z = 0; z < zSize; z++)
	    {
		    double imageZPos = z*renderWidth;
		    zVolume = volumes[z];
		    for(int s = 0; s < trunc(rayLength); s++)
		    {
			    Vector2 pos = {startXPositions[s], startYPositions[s]};

			    double outputr = 0.0, outputg = 0.0, outputb = 0.0;
			    double saturationr = 0.0, saturationg = 0.0, saturationb = 0.0;
			    double x = 0.0;
			    double y = 0.0;
			    int valuer = 0, valueg = 0, valueb = 0;
			    double vr = 0.0, vg = 0.0, vb = 0.0;
			    double ar = 0.0, ag = 0.0, ab = 0.0;
			
			    for(int i = 1; i < sampleCounts[s]; i++)
			    {
				    x = trunc(pos.a + i*increment.a);
				    y = trunc(pos.b + i*increment.b);
				    valuer = 0xFF & zVolume[(int)(x + y*imageSize)] >> 24;
				    valueg = 0xFF & zVolume[(int)(x + y*imageSize)] >> 16;
				    valueb = 0xFF & zVolume[(int)(x + y*imageSize)] >> 8;
				    
				    if(valuer < 50 && valueg < 50 && valueb < 50)
				    {
					    continue;
				    }

				    ar = pow(valuer/256.0, 2);//squared[value];
				    ag = pow(valueg/256.0, 2);//squared[value];
				    ab = pow(valueb/256.0, 2);//squared[value];
				    vr = pow(valuer/256.0, 3);//cubed[value];
				    vg = pow(valueg/256.0, 3);//cubed[value];
				    vb = pow(valueb/256.0, 3);//cubed[value];
				    
				    outputr =     outputr     + vr - vr*saturationr;
				    outputg =     outputg     + vg - vg*saturationg;
				    outputb =     outputb     + vb - vb*saturationb;
				    saturationr = 1;//saturationr + ar - ar*saturationr;
				    saturationg = 1;//saturationg + ag - ag*saturationg;
				    saturationb = 1;//saturationb + ab - ab*saturationb;
				    
				    if(saturationr >= 0.9)
				    {
					    break;
				    }
			    }

			    int index = imageZPos + s;
			    renderedImage[index/4] = ((0xFF&(int)trunc(outputr*256)) << 24) |  ((0xFF&(int)trunc(outputg*256)) << 16) | ((0xFF&(int)trunc(outputb*256)) << 8) | 0xFF;
			    
                SDL_SetRenderDrawColor(renderer, trunc(outputr*256), trunc(outputg*256), trunc(outputb*256), 255);
                SDL_RenderDrawPoint(renderer, index%renderWidth, index/renderWidth);
                //std::cout << "Placing point of " << trunc(output*256) << " to " << x << " " << y << "\n";
			    //cutImageData.data[index+0] = trunc(output*256);
			    //cutImageData.data[index+1] = trunc(output*256);
			    //cutImageData.data[index+2] = trunc(output*256);
			    //cutImageData.data[index+3] = 255;

		    }

	    }
	
        //SDL_LockTexture(viewTexture, NULL, (void**)&renderedImage, &renderWidth);
        //SDL_UnlockTexture(viewTexture);
        //SDL_RenderCopy(renderer, viewTexture, NULL, NULL);
        
        // SDL - show it, with a pause for clarity
        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }
    
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    
    return 0;
}





