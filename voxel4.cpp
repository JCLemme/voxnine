/*
Copyright (c) 2004-2007, Lode Vandevenne

All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <cmath>
#include <string>
#include <vector>
#include <iostream>

#include "quickcg.h"
using namespace QuickCG;

/*
g++ *.cpp -lSDL -O3 -W -Wall -ansi -pedantic
g++ *.cpp -lSDL
*/

//place the example code below here:

#define mapWidth 96
#define mapHeight 96

struct LineSeg
{
    int start, end;
    ColorRGB color;
};

int worldMap[mapWidth][mapHeight][12];

int main(int /*argc*/, char */*argv*/[])
{
    double posX = 40, posY = 40;  //x and y start position
    double dirX = -1, dirY = 0; //initial direction vector
    double planeX = 0, planeY = 0.66; //the 2d raycaster version of camera plane

    int pitch = 200; //tilt of camera
    
    double time = 0; //time of current frame
    double oldTime = 0; //time of previous frame

    bool stepmode = false;
    
    for(int i=0;i<mapHeight;i++)
    {
        for(int j=0;j<mapWidth;j++)
        {
            if(j==0||j==mapWidth-1||i==0||i==mapHeight-1)
            {
                worldMap[j][i][0] = 2;
                worldMap[j][i][1] = 1;
                worldMap[j][i][2] = 1;
                worldMap[j][i][3] = 1;
                worldMap[j][i][4] = 1;
                worldMap[j][i][5] = 1;
                worldMap[j][i][6] = 1;
                worldMap[j][i][7] = 1;
                worldMap[j][i][8] = 1;
                worldMap[j][i][9] = 1;
                worldMap[j][i][10] = 1;
                worldMap[j][i][11] = 2;
            }
            else
            {
                worldMap[j][i][0] = 2;
                worldMap[j][i][1] = 0;
                worldMap[j][i][2] = 0;
                worldMap[j][i][3] = 0;
                worldMap[j][i][4] = 0;
                worldMap[j][i][5] = 0;
                worldMap[j][i][6] = 0;
                worldMap[j][i][7] = 0;
                worldMap[j][i][8] = 0;
                worldMap[j][i][9] = 0;
                worldMap[j][i][11] = 2;
            }
        }
    }

    worldMap[60][60][0] = 2;
    worldMap[60][60][1] = 0;
    worldMap[60][60][2] = 0;
    worldMap[60][60][3] = 0;
    worldMap[60][60][4] = 3;
    worldMap[60][60][5] = 4;
    worldMap[60][60][6] = 4;
    worldMap[60][60][7] = 3;
    worldMap[60][60][8] = 0;
    worldMap[60][60][9] = 0;
    worldMap[60][60][10] = 0;
    worldMap[60][60][11] = 2;

    worldMap[60][61][0] = 2;
    worldMap[60][61][1] = 0;
    worldMap[60][61][2] = 0;
    worldMap[60][61][3] = 0;
    worldMap[60][61][4] = 0;
    worldMap[60][61][5] = 4;
    worldMap[60][61][6] = 4;
    worldMap[60][61][7] = 0;
    worldMap[60][61][8] = 0;
    worldMap[60][61][9] = 0;
    worldMap[60][61][10] = 0;
    worldMap[60][61][11] = 2;

    worldMap[61][60][0] = 2;
    worldMap[61][60][1] = 0;
    worldMap[61][60][2] = 0;
    worldMap[61][60][3] = 0;
    worldMap[61][60][4] = 3;
    worldMap[61][60][5] = 0;
    worldMap[61][60][6] = 0;
    worldMap[61][60][7] = 3;
    worldMap[61][60][8] = 0;
    worldMap[61][60][9] = 0;
    worldMap[61][60][10] = 0;
    worldMap[61][60][11] = 2;

    worldMap[62][60][0] = 2;
    worldMap[62][60][1] = 0;
    worldMap[62][60][2] = 0;
    worldMap[62][60][3] = 0;
    worldMap[62][60][4] = 3;
    worldMap[62][60][5] = 0;
    worldMap[62][60][6] = 0;
    worldMap[62][60][7] = 3;
    worldMap[62][60][8] = 0;
    worldMap[62][60][9] = 0;
    worldMap[62][60][10] = 0;
    worldMap[62][60][11] = 2;

    worldMap[63][60][0] = 2;
    worldMap[63][60][1] = 0;
    worldMap[63][60][2] = 0;
    worldMap[63][60][3] = 0;
    worldMap[63][60][4] = 3;
    worldMap[63][60][5] = 0;
    worldMap[63][60][6] = 0;
    worldMap[63][60][7] = 3;
    worldMap[63][60][8] = 0;
    worldMap[63][60][9] = 0;
    worldMap[63][60][10] = 0;
    worldMap[63][60][11] = 2;

    worldMap[63][66][0] = 2;
    worldMap[63][66][1] = 0;
    worldMap[63][66][2] = 0;
    worldMap[63][66][3] = 5;
    worldMap[63][66][4] = 0;
    worldMap[63][66][5] = 4;
    worldMap[63][66][6] = 0;
    worldMap[63][66][7] = 3;
    worldMap[63][66][8] = 0;
    worldMap[63][66][9] = 0;
    worldMap[63][66][10] = 0;
    worldMap[63][66][11] = 2;

    screen(512, 384, 0, "Raycaster");
    while(!done())
    {
        for(int x = 0; x < w; x++)
        {
            //calculate ray position and direction
            double cameraX = 2 * x / double(w) - 1; //x-coordinate in camera space
            double rayPosX = posX;
            double rayPosY = posY;

            double rayDirX = dirX + planeX * cameraX;
            double rayDirY = dirY + planeY * cameraX;

            //which box of the map we're in
            int mapX = int(rayPosX);
            int mapY = int(rayPosY);

            //length of ray from current position to next x or y-side
            double sideDistX;
            double sideDistY;

            //length of ray from one x or y-side to next x or y-side
            double deltaDistX = sqrt(1 + (rayDirY * rayDirY) / (rayDirX * rayDirX));
            double deltaDistY = sqrt(1 + (rayDirX * rayDirX) / (rayDirY * rayDirY));
            double perpWallDist;

            //what direction to step in x or y-direction (either +1 or -1)
            int stepX;
            int stepY;

            int hit = 0; //was there a wall hit?
            int side; //was a NS or a EW wall hit?
            int depth[384];
            std::fill_n(depth, 384, 100000); //memset(&depth, 1, sizeof(double)*384);
            int count = 0;

            std::vector<LineSeg> spans;
            
            //calculate step and initial sideDist
            if (rayDirX < 0)
            {
                stepX = -1;
                sideDistX = (rayPosX - mapX) * deltaDistX;
            }
            else
            {
                stepX = 1;
                sideDistX = (mapX + 1.0 - rayPosX) * deltaDistX;
            }
            if (rayDirY < 0)
            {
                stepY = -1;
                sideDistY = (rayPosY - mapY) * deltaDistY;
            }
            else
            {
                stepY = 1;
                sideDistY = (mapY + 1.0 - rayPosY) * deltaDistY;
            }
            //std::cout << "X: " << x << "\n";
            //perform DDA
            while (mapX < 0 || mapY < 0 || hit < 100 /*|| count < 384*/)
            {
                //jump to next map square, OR in x-direction, OR in y-direction
                if(sideDistX < sideDistY)
                {
                    sideDistX += deltaDistX;
                    mapX += stepX;
                    side = 0;
                }
                else
                {
                    sideDistY += deltaDistY;
                    mapY += stepY;
                    side = 1;
                }

                if(mapX < 0) mapX = 0;
                if(mapY < 0) mapY = 0;

                if(mapX > mapHeight-1) mapX = mapHeight-1;
                if(mapY > mapWidth-1) mapY = mapWidth-1;

                //Check if ray has hit a wall
                if (1) //It should always collide check
                {
                    hit += 1;
                    //Calculate distance projected on camera direction (oblique distance will give fisheye effect!)
                    if (side == 0) perpWallDist = (mapX - rayPosX + (1 - stepX) / 2) / rayDirX;// /2; //why did I have these
                    else           perpWallDist = (mapY - rayPosY + (1 - stepY) / 2) / rayDirY;// /2;

                    //Calculate height of line to draw on screen
                    int lineHeight = (int)(h / perpWallDist); //This needs to be divided by 2 to make square voxels, but then it crashes. ??? wait nvm
                    //std::cout << "  Height for [" << mapX << "," << mapY << "]: " << lineHeight << "\n";
                    for(int b=0;b<12;b++)
                    {
                        //calculate lowest and highest pixel to fill in current stripe
                        int drawStart = ((lineHeight)*(b-6)) + pitch;
                        if(drawStart < 0)drawStart = 0;
                        int drawEnd = lineHeight + ((lineHeight)*(b-6)) + pitch;
                        if(drawEnd >= h)drawEnd = h - 1;
                        //std::cout << drawStart << " " << drawEnd << "\n";
                        
                        //choose wall color
                        ColorRGB color;
                        switch(worldMap[mapX][mapY][b])
                        {
                            case 0:  color = RGB_Black;  break; //red
                            case 1:  color = RGB_Grey;   break; //red
                            case 2:  color = RGB_Green;  break; //green
                            case 3:  color = RGB_Blue;   break; //blue
                            case 4:  color = RGB_White;  break; //white
                            case 5:  color = RGB_Red;    break; //white
                            default: color = RGB_Yellow; break; //yellow
                        }

                        //give x and y sides different brightness
                        if (side == 1) {color = color / 2;}

                        //draw the pixels of the stripe as a vertical line
                        if(color != RGB_Black)
                        {
                            verLineDepth(x, drawStart, drawEnd, color, depth, 384, &count);
                            //redracw();
                            //sleep();
                        }
                    }    
                }
            }
        }
        
        //timing for input and FPS counter
        oldTime = time;
        time = getTicks();
        double frameTime = (time - oldTime) / 1000.0; //frameTime is the time this frame has taken, in seconds
        print(1.0 / frameTime); //FPS counter
        redraw();
        cls();

        //speed modifiers
        double moveSpeed = frameTime * 10.0; //the constant value is in squares/second
        double rotSpeed = frameTime * 3.0; //the constant value is in radians/second
        readKeys();
        
        //move forward if no wall in front of you
        if (keyDown(SDLK_UP))
        {
            if(worldMap[int(posX + dirX * moveSpeed)][int(posY)][10] == false) posX += dirX * moveSpeed;
            if(worldMap[int(posX)][int(posY + dirY * moveSpeed)][10] == false) posY += dirY * moveSpeed;
        }
        
        //move backwards if no wall behind you
        if (keyDown(SDLK_DOWN))
        {
            if(worldMap[int(posX - dirX * moveSpeed)][int(posY)][10] == false) posX -= dirX * moveSpeed;
            if(worldMap[int(posX)][int(posY - dirY * moveSpeed)][10] == false) posY -= dirY * moveSpeed;
        }
        
        //rotate to the right
        if (keyDown(SDLK_RIGHT))
        {
            //both camera direction and camera plane must be rotated
            double oldDirX = dirX;
            dirX = dirX * cos(-rotSpeed) - dirY * sin(-rotSpeed);
            dirY = oldDirX * sin(-rotSpeed) + dirY * cos(-rotSpeed);
            double oldPlaneX = planeX;
            planeX = planeX * cos(-rotSpeed) - planeY * sin(-rotSpeed);
            planeY = oldPlaneX * sin(-rotSpeed) + planeY * cos(-rotSpeed);
           // std::cout << dirX << " " << dirY << " " << planeX << " " << planeY << "\n";
            
        }
        
        //rotate to the left
        if (keyDown(SDLK_LEFT))
        {
            //both camera direction and camera plane must be rotated
            double oldDirX = dirX;
            dirX = dirX * cos(rotSpeed) - dirY * sin(rotSpeed);
            dirY = oldDirX * sin(rotSpeed) + dirY * cos(rotSpeed);
            double oldPlaneX = planeX;
            planeX = planeX * cos(rotSpeed) - planeY * sin(rotSpeed);
            planeY = oldPlaneX * sin(rotSpeed) + planeY * cos(rotSpeed);
        }
        
        //pitch up
        if (keyDown(SDLK_i))
        {
            pitch += 1;
        }
        
        //pitch down
        if (keyDown(SDLK_k))
        {
            pitch -= 1;
        }
        
        //stepmode
        if (keyDown(SDLK_p))
        {
            stepmode = (stepmode)?false:true;
        }
    }
}

