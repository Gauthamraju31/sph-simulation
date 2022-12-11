#pragma once

namespace worldConstants
{
    /* Gravitational constant */
    const float G = 0.02f * .25;
    /* Spacing of the particles */
    const float spacing = 2.f;
    /* */
    const float k = spacing / 1000.0f;
    /* Far pressure weight */
    const float k_near = k * 10;
    /* Near pressure weight */
    const float rest_density = 3;
    /* Rest density */
    const float r = spacing * 1.25f;
    /* Radius of support */
    const float rsq = r * r;
    /* The size of the world */
    const float SIM_W = 50;
    /* The floor of the world */
    const float bottom = 0;
    /* The binning factor */
    const int numBuckets = 4093;
}
