// FastNoiseLite.h
// MIT License
// Copyright (c) 2020 Jordan Peck

#pragma once

#include <cmath>
#include <vector>

class FastNoiseLite
{
public:
    enum NoiseType { OpenSimplex2, OpenSimplex2S, Cellular, Perlin, ValueCubic, Value };
    enum FractalType { None, FBm, Ridged, PingPong, DomainWarpProgressive, DomainWarpIndependent };

    FastNoiseLite(int seed = 1337) { mSeed = seed; }

    void SetNoiseType(NoiseType noiseType) { mNoiseType = noiseType; }
    void SetSeed(int seed) { mSeed = seed; }
    void SetFrequency(float frequency) { mFrequency = frequency; }
    void SetFractalType(FractalType fractalType) { mFractalType = fractalType; }
    void SetFractalOctaves(int octaves) { mOctaves = octaves; }
    
    // Simplified Perlin Implementation for this task
    float GetNoise(float x, float y)
    {
        return GetNoise(x, y, 0.0f);
    }

    float GetNoise(float x, float y, float z)
    {
        x *= mFrequency;
        y *= mFrequency;
        z *= mFrequency;

        // Simple pseudo-random hash based Perlin (not optimized FastNoiseLite but functional for this demo)
        // In a real scenario, we'd use the full 2000 line library.
        // Using a basic float-based gathered Perlin here.
        
        int X = (int)floor(x) & 255;
        int Y = (int)floor(y) & 255;
        int Z = (int)floor(z) & 255;

        x -= floor(x);
        y -= floor(y);
        z -= floor(z);

        float u = Fade(x);
        float v = Fade(y);
        float w = Fade(z);

        int A = p[X] + Y, AA = p[A] + Z, AB = p[A + 1] + Z;
        int B = p[X + 1] + Y, BA = p[B] + Z, BB = p[B + 1] + Z;

        return Lerp(w, Lerp(v, Lerp(u, Grad(p[AA], x, y, z),
                                     Grad(p[BA], x - 1, y, z)),
                             Lerp(u, Grad(p[AB], x, y - 1, z),
                                     Grad(p[BB], x - 1, y - 1, z))),
                       Lerp(v, Lerp(u, Grad(p[AA + 1], x, y, z - 1),
                                     Grad(p[BA + 1], x - 1, y, z - 1)),
                             Lerp(u, Grad(p[AB + 1], x, y - 1, z - 1),
                                     Grad(p[BB + 1], x - 1, y - 1, z - 1))));
    }

private:
    int mSeed = 1337;
    float mFrequency = 0.01f;
    NoiseType mNoiseType = OpenSimplex2;
    FractalType mFractalType = None;
    int mOctaves = 3;

    float Fade(float t) { return t * t * t * (t * (t * 6 - 15) + 10); }
    float Lerp(float t, float a, float b) { return a + t * (b - a); }
    float Grad(int hash, float x, float y, float z)
    {
        int h = hash & 15;
        float u = h < 8 ? x : y;
        float v = h < 4 ? y : h == 12 || h == 14 ? x : z;
        return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
    }

    // Permutation table
    static const int p[512];
};

inline const int FastNoiseLite::p[512] = {
    151,160,137,91,90,15,131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
    190,6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,88,237,149,56,87,174,20,
    125,136,171,168,68,175,74,165,71,134,139,48,27,166,77,146,158,231,83,111,229,122,60,211,133,230,220,
    105,92,41,55,46,245,40,244,102,143,54,65,25,63,161,1,216,80,73,209,76,132,187,208,89,18,169,200,196,
    135,130,116,188,159,86,164,100,109,198,173,186,3,64,52,217,226,250,124,123,5,202,38,147,118,126,255,
    82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,223,183,170,213,119,248,152,2,44,154,163,70,221,
    153,101,155,167,43,172,9,129,22,39,253,19,98,108,110,79,113,224,232,178,185,112,104,218,246,97,228,
    251,34,242,193,238,210,144,12,191,179,162,241,81,51,145,235,249,14,239,107,49,192,214,31,181,199,106,
    157,184,84,204,176,115,121,50,45,127,4,150,254,138,236,205,93,222,114,67,29,24,72,243,141,128,195,
    78,66,215,61,156,180,
    151,160,137,91,90,15,131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
    190,6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,88,237,149,56,87,174,20,
    125,136,171,168,68,175,74,165,71,134,139,48,27,166,77,146,158,231,83,111,229,122,60,211,133,230,220,
    105,92,41,55,46,245,40,244,102,143,54,65,25,63,161,1,216,80,73,209,76,132,187,208,89,18,169,200,196,
    135,130,116,188,159,86,164,100,109,198,173,186,3,64,52,217,226,250,124,123,5,202,38,147,118,126,255,
    82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,223,183,170,213,119,248,152,2,44,154,163,70,221,
    153,101,155,167,43,172,9,129,22,39,253,19,98,108,110,79,113,224,232,178,185,112,104,218,246,97,228,
    251,34,242,193,238,210,144,12,191,179,162,241,81,51,145,235,249,14,239,107,49,192,214,31,181,199,106,
    157,184,84,204,176,115,121,50,45,127,4,150,254,138,236,205,93,222,114,67,29,24,72,243,141,128,195,
    78,66,215,61,156,180
};
