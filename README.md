# voxnine
A collection of voxel renderers inspired by Ken Silverman's Voxlap engine

I've been trying to reverse engineer Voxlap for years. This repo contains some of my attempts. 
QuickCG and the raycasting code inside these programs is borrowed from Lode Vandevenne and modified by me.

---

Voxel7 is the latest renderer. Compile with

`g++ -o voxel7 voxel7.cpp quickcg.cpp -lSDL`
    
Arrow keys move, U/J move up and down, I/K tilt camera up/down.
