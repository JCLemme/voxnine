/*

Voxel7 - a voxel renderer
Written by John Lemme (2018), based on the raycasting code from Lode Vandevenne's tutorial site.

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
#include <fstream>
#include "quickcg.h"
using namespace QuickCG;

#define mapWidth 96
#define mapHeight 96
#define mapDepth 12

#define windowWidth 512
#define windowHeight 384

typedef struct VoxelCell
{
    ColorRGB color;
    int runLength;
} VoxelCell;

VoxelCell worldMap[mapWidth][mapHeight][mapDepth];

void defaultMap();
void encodeMap();

int main(int argc, char** argv)
{
    double posX = 40, posY = 40, posZ = mapDepth/2;  //x, y, and z start position
    double dirX = -1, dirY = 0; //initial direction vector
    double planeX = 0, planeY = 0.66; //the 2d raycaster version of camera plane

    int pitch = 200; //tilt of camera
    
    double time = 0; //time of current frame
    double oldTime = 0; //time of previous frame
    
    std::vector<unsigned char> mapFile;
    
    if(argc > 1)
    {
        loadFile(mapFile, argv[1]);
        
        if(mapFile.size() < mapHeight*mapWidth*mapDepth*3)
        {
            std::cout << "File \"" << argv[1] << "\" is corrupt - using defaults\n";
            defaultMap();
        }
        else
        {
            int counter = 0;
            
            for(int x=0;x<mapWidth;x++)
            {
                for(int y=0;y<mapHeight;y++)
                {
                    for(int z=0;z<mapDepth;z++)
                    {
                        worldMap[x][y][z].color = ColorRGB{mapFile[counter], mapFile[counter+1], mapFile[counter+2]};
                        counter += 3;
                    }
                }
            }
            
            std::cout << "Loaded file \"" << argv[1] << "\"\n";
        }
    }
    else
    {
        std::cout << "No map loaded - using defaults\n";
        defaultMap();
    }

    encodeMap();

    screen(windowWidth, windowHeight, 0, "Vox7 Application");

    int* depth = (int*)malloc(windowHeight*sizeof(int)); //new int[384];
    int* depthrear = (int*)malloc(windowHeight*sizeof(int)); //new int[384];
            
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
            int mapX = int(rayPosX), tmapX;
            int mapY = int(rayPosY), tmapY;

            //length of ray from current position to next x or y-side
            double sideDistX, tsideDistX;
            double sideDistY, tsideDistY;

            //length of ray from one x or y-side to next x or y-side
            double deltaDistX = sqrt(1 + (rayDirY * rayDirY) / (rayDirX * rayDirX));
            double deltaDistY = sqrt(1 + (rayDirX * rayDirX) / (rayDirY * rayDirY));
            double perpWallDist, tperpWallDist;
            int lineHeight, tlineHeight;
            
            //what direction to step in x or y-direction (either +1 or -1)
            int stepX;
            int stepY;

            int hit = 1; //was there a wall hit?
            int side, tside; //was a NS or a EW wall hit?
            
            std::fill_n(depth, windowHeight, 100000); //memset(&depth, 1, sizeof(double)*384);
            std::fill_n(depthrear, windowHeight, 100000); //memset(&depth, 1, sizeof(double)*384);
            
            //std::cout << "Pointer : " << (uint64_t)depth << "\n";
            
            int count = 0;
            
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

            //First run
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
            
            //Calculate distance projected on camera direction (oblique distance will give fisheye effect!)
            if (side == 0) perpWallDist = (mapX - rayPosX + (1 - stepX) / 2) / rayDirX;// /2; //why did I have these
            else           perpWallDist = (mapY - rayPosY + (1 - stepY) / 2) / rayDirY;// /2;

            //Calculate height of line to draw on screen
            lineHeight = (int)(h / perpWallDist); //This needs to be divided by 2 to make square voxels, but then it crashes. ??? wait nvm
                
            //perform DDA
            while (mapX < 0 || mapY < 0 || (count < windowHeight && hit < 900))
            {
                hit += 1;
                
                //Calculate next DDA for horizontal fill
                tmapX = mapX;
                tmapY = mapY;
                tside = side;
                tsideDistX = sideDistX;
                tsideDistY = sideDistY;
                
                //jump to next map square, OR in x-direction, OR in y-direction
                if(sideDistX < sideDistY)
                {
                    tsideDistX += deltaDistX;    
                    tmapX += stepX;
                    tside = 0;
                }
                else
                {
                    tsideDistY += deltaDistY;
                    tmapY += stepY;
                    tside = 1;
                }

                if(tmapX < 0) tmapX = 0;
                if(tmapY < 0) tmapY = 0;

                if(tmapX > mapHeight-1) tmapX = mapHeight-1;
                if(tmapY > mapWidth-1) tmapY = mapWidth-1;
                
                //Calculate distance projected on camera direction (oblique distance will give fisheye effect!)
                if (tside == 0) tperpWallDist = (tmapX - rayPosX + (1 - stepX) / 2) / rayDirX;// /2; //why did I have these
                else           tperpWallDist = (tmapY - rayPosY + (1 - stepY) / 2) / rayDirY;// /2;

                //Calculate height of line to draw on screen
                int tlineHeight = (int)(h / tperpWallDist); //This needs to be divided by 2 to make square voxels, but then it crashes. ??? wait nvm
                    
                for(int b=0;b<mapDepth;b++)
                {
                    int ob = b;
                    
                    //calculate lowest and highest pixel to fill in current stripe
                    int drawStart = ((lineHeight)*(ob-posZ)) + pitch;
                    if(drawStart < 0)drawStart = 0;
                    
                    b += (worldMap[mapX][mapY][ob].runLength)-1;
                    if(b < 0) b = 0;
                    
                    int drawEnd = lineHeight + ((lineHeight)*(b-posZ)) + pitch;
                    if(drawEnd >= h)drawEnd = h - 1;
                    
                    //choose wall color
                    ColorRGB color = worldMap[mapX][mapY][b].color;

                    //give x and y sides different brightness
                    if (side == 1) {color = color / 2;}

                    //draw the pixels of the stripe as a vertical line
                    if(color != RGB_Black)
                    {
                        verLineTriDepth(x, drawStart, drawEnd, color, depth, windowHeight, &count, 0, depthrear);
                    }
                         
                    //calculate lowest and highest pixel to fill in current stripe
                    int tdrawStart = ((tlineHeight)*(ob-posZ)) + pitch;
                    if(tdrawStart < 0)tdrawStart = 0;
                    int tdrawEnd = tlineHeight + ((tlineHeight)*(b-posZ)) + pitch;
                    if(tdrawEnd >= h)tdrawEnd = h - 1;
                    
                    //choose wall color
                    ColorRGB tcolor = worldMap[mapX][mapY][b].color;

                    //give x and y sides different brightness
                    {tcolor = tcolor / 3;}

                    //draw the pixels of the stripe as a vertical line
                    if(tcolor != RGB_Black)
                    {
                        verLineTriDepth(x, (b<posZ)?drawStart:tdrawStart, (b<posZ)?tdrawEnd:drawEnd, tcolor, depth, windowHeight, &count, 1, depthrear);
                    }
                }
                
                memcpy(depth, depthrear, windowHeight * sizeof(int));
                lineHeight = tlineHeight;
                perpWallDist = tperpWallDist;
                sideDistX = tsideDistX;
                sideDistY = tsideDistY;
                side = tside;
                mapX = tmapX;
                mapY = tmapY;
            }
        }
        
        //timing for input and FPS counter
        oldTime = time;
        time = getTicks();
        double frameTime = (time - oldTime) / 1000.0; //frameTime is the time this frame has taken, in seconds
        print(1.0 / frameTime); //FPS counter
        print(std::string("X: " + std::to_string(posX) + "  Y: " + std::to_string(posY)), 300, 0);
        
        redraw();

        //EDIT MODE
        if(keyDown(SDLK_p))
        {
            print("EDIT MODE", 256, 120);
            redraw();
            std::cout << "Edit mode enabled.\n";
            
            bool editing = true;
            
            while(editing)
            {
                std::cout << ">";
                
                std::string input, part;
                std::vector<std::string> args;
                
                getline(std::cin, input);
                
                std::istringstream inp(input);
                while(getline(inp, part, ' '))
                    args.push_back(part);
                
                if(args.size() > 0)
                {
                    if(args[0] == "q")
                    {
                        editing = false;
                    }
                    else if(args[0] == "w")
                    {
                        worldMap[std::stoi(args[1])][std::stoi(args[2])][std::stoi(args[3])].color = ColorRGB{(unsigned char)std::stoi(args[4]), (unsigned char)std::stoi(args[5]), (unsigned char)std::stoi(args[6])};
                        std::cout << "Voxel written\n";
                    }
                    else if(args[0] == "s")
                    {
                        std::vector<unsigned char> wdata;
                        
                        for(int x=0;x<mapWidth;x++)
                        {
                            for(int y=0;y<mapHeight;y++)
                            {
                                for(int z=0;z<mapDepth;z++)
                                {
                                    wdata.push_back(worldMap[x][y][z].color.r);
                                    wdata.push_back(worldMap[x][y][z].color.g);
                                    wdata.push_back(worldMap[x][y][z].color.b);
                                }
                            }
                        }
                        
                        std::ofstream file;
                        file.open(args[1].c_str(), std::ios::out|std::ios::binary);
                        file.write((char*)&wdata[0], std::streamsize(wdata.size()));
                        file.close();
                        
                        std::cout << "Saved to " << args[1] << "\n";
                    }
                }
            }
        }
        
        cls();
        
        //speed modifiers
        double moveSpeed = frameTime * 10.0; //the constant value is in squares/second
        double rotSpeed = frameTime * 3.0; //the constant value is in radians/second
        readKeys();
        
        //move forward if no wall in front of you
        if (keyDown(SDLK_UP))
        {
            if(worldMap[int(posX + dirX * moveSpeed)][int(posY)][10].color == RGB_Black) posX += dirX * moveSpeed;
            if(worldMap[int(posX)][int(posY + dirY * moveSpeed)][10].color == RGB_Black) posY += dirY * moveSpeed;
        }
        
        //move backwards if no wall behind you
        if (keyDown(SDLK_DOWN))
        {
            if(worldMap[int(posX - dirX * moveSpeed)][int(posY)][10].color == RGB_Black) posX -= dirX * moveSpeed;
            if(worldMap[int(posX)][int(posY - dirY * moveSpeed)][10].color == RGB_Black) posY -= dirY * moveSpeed;
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
        
        //posZ up
        if (keyDown(SDLK_u))
        {
            posZ -= .1;
            if(posZ < 0) posZ = 0;
        }
        
        //posZ down
        if (keyDown(SDLK_j))
        {
            posZ += .1;
            if(posZ > mapDepth) posZ = mapDepth;
        }        
        
        //pitch up
        if (keyDown(SDLK_i))
        {
            pitch += 10;
        }
        
        //pitch down
        if (keyDown(SDLK_k))
        {
            pitch -= 10;
        }
    }
}

void defaultMap()
{
    // Default map. Grey walls, green ceiling and floor, and some weird statues.
    for(int i=0;i<mapHeight;i++)
    {
        for(int j=0;j<mapWidth;j++)
        {
            if(j==0||j==mapWidth-1||i==0||i==mapHeight-1)
            {
                worldMap[j][i][0] = {RGB_Green, 1};
                
                for(int r=0;r<mapDepth-2;r++)
                    worldMap[j][i][r+1] = {RGB_Grey, 1};
                    
                worldMap[j][i][11] = {RGB_Green, 1};
            }
            else
            {
                worldMap[j][i][0] = {RGB_Green, 1};
                
                for(int r=0;r<mapDepth-2;r++)
                    worldMap[j][i][r+1] = {RGB_Black, 1};
                    
                worldMap[j][i][11] = {RGB_Green, 1};
            }
        }
    }

    worldMap[60][60][4] = {RGB_Blue, 1};
    worldMap[60][60][5] = {RGB_White, 2};
    worldMap[60][60][6] = {RGB_White, 1};
    worldMap[60][60][7] = {RGB_Blue, 1};
    worldMap[60][61][5] = {RGB_White, 2};
    worldMap[60][61][6] = {RGB_White, 1};
    worldMap[61][60][4] = {RGB_Blue, 1};
    worldMap[61][60][7] = {RGB_Blue, 1};
    worldMap[62][60][4] = {RGB_Blue, 1};
    worldMap[62][60][7] = {RGB_Blue, 1};
    worldMap[63][60][4] = {RGB_Blue, 1};
    worldMap[63][60][7] = {RGB_Blue, 1};
    worldMap[63][66][3] = {RGB_Red, 1};
    worldMap[63][66][5] = {RGB_White, 1};
    worldMap[63][66][7] = {RGB_Blue, 1};
}

void encodeMap()
{
    for(int i=0;i<mapHeight;i++)
    {
        for(int j=0;j<mapWidth;j++)
        {
            for(int b=0;b<mapDepth;b++)
            {
                ColorRGB cache = worldMap[i][j][b].color;
                worldMap[i][j][b].runLength = 0;
                int nb = b;
                
                while(worldMap[i][j][nb].color == cache && nb < mapDepth)
                {
                    nb++;
                    worldMap[i][j][b].runLength++;
                }
            }
        }
    }
}
    
