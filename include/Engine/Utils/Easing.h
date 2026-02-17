#pragma once

#include <cmath>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace Easing {

    // --- Linear ---
    inline float Linear(float t) { return t; }

    // --- Sine ---
    inline float InSine(float t) { return 1 - std::cos((t * M_PI) / 2); }
    inline float OutSine(float t) { return std::sin((t * M_PI) / 2); }
    inline float InOutSine(float t) { return -(std::cos(M_PI * t) - 1) / 2; }

    // --- Quad ---
    inline float InQuad(float t) { return t * t; }
    inline float OutQuad(float t) { return 1 - (1 - t) * (1 - t); }
    inline float InOutQuad(float t) { return t < 0.5 ? 2 * t * t : 1 - std::pow(-2 * t + 2, 2) / 2; }

    // --- Cubic ---
    inline float InCubic(float t) { return t * t * t; }
    inline float OutCubic(float t) { return 1 - std::pow(1 - t, 3); }
    inline float InOutCubic(float t) { return t < 0.5 ? 4 * t * t * t : 1 - std::pow(-2 * t + 2, 3) / 2; }

    // --- Quart ---
    inline float InQuart(float t) { return t * t * t * t; }
    inline float OutQuart(float t) { return 1 - std::pow(1 - t, 4); }
    inline float InOutQuart(float t) { return t < 0.5 ? 8 * t * t * t * t : 1 - std::pow(-2 * t + 2, 4) / 2; }

    // --- Quint ---
    inline float InQuint(float t) { return t * t * t * t * t; }
    inline float OutQuint(float t) { return 1 - std::pow(1 - t, 5); }
    inline float InOutQuint(float t) { return t < 0.5 ? 16 * t * t * t * t * t : 1 - std::pow(-2 * t + 2, 5) / 2; }

    // --- Expo ---
    inline float InExpo(float t) { return t == 0 ? 0 : std::pow(2, 10 * t - 10); }
    inline float OutExpo(float t) { return t == 1 ? 1 : 1 - std::pow(2, -10 * t); }
    inline float InOutExpo(float t) {
        return t == 0 ? 0 : t == 1 ? 1 : t < 0.5 ? std::pow(2, 20 * t - 10) / 2 : (2 - std::pow(2, -20 * t + 10)) / 2;
    }

    // --- Circ ---
    inline float InCirc(float t) { return 1 - std::sqrt(1 - std::pow(t, 2)); }
    inline float OutCirc(float t) { return std::sqrt(1 - std::pow(t - 1, 2)); }
    inline float InOutCirc(float t) {
        return t < 0.5 ? (1 - std::sqrt(1 - std::pow(2 * t, 2))) / 2 : (std::sqrt(1 - std::pow(-2 * t + 2, 2)) + 1) / 2;
    }

    // --- Back ---
    inline float InBack(float t) {
        const float c1 = 1.70158;
        const float c3 = c1 + 1;
        return c3 * t * t * t - c1 * t * t;
    }
    inline float OutBack(float t) {
        const float c1 = 1.70158;
        const float c3 = c1 + 1;
        return 1 + c3 * std::pow(t - 1, 3) + c1 * std::pow(t - 1, 2);
    }
    inline float InOutBack(float t) {
        const float c1 = 1.70158;
        const float c2 = c1 * 1.525;
        return t < 0.5
            ? (std::pow(2 * t, 2) * ((c2 + 1) * 2 * t - c2)) / 2
            : (std::pow(2 * t - 2, 2) * ((c2 + 1) * (t * 2 - 2) + c2) + 2) / 2;
    }

    // --- Elastic ---
    inline float InElastic(float t) {
        const float c4 = (2 * M_PI) / 3;
        return t == 0 ? 0 : t == 1 ? 1 : -std::pow(2, 10 * t - 10) * std::sin((t * 10 - 10.75) * c4);
    }
    inline float OutElastic(float t) {
        const float c4 = (2 * M_PI) / 3;
        return t == 0 ? 0 : t == 1 ? 1 : std::pow(2, -10 * t) * std::sin((t * 10 - 0.75) * c4) + 1;
    }
    inline float InOutElastic(float t) {
        const float c5 = (2 * M_PI) / 4.5;
        return t == 0 ? 0 : t == 1 ? 1 : t < 0.5
            ? -(std::pow(2, 20 * t - 10) * std::sin((20 * t - 11.125) * c5)) / 2
            : (std::pow(2, -20 * t + 10) * std::sin((20 * t - 11.125) * c5)) / 2 + 1;
    }

    // --- Bounce ---
    inline float OutBounce(float t) {
        const float n1 = 7.5625;
        const float d1 = 2.75;
        if (t < 1 / d1) {
            return n1 * t * t;
        } else if (t < 2 / d1) {
            return n1 * (t -= 1.5 / d1) * t + 0.75;
        } else if (t < 2.5 / d1) {
            return n1 * (t -= 2.25 / d1) * t + 0.9375;
        } else {
            return n1 * (t -= 2.625 / d1) * t + 0.984375;
        }
    }
    inline float InBounce(float t) { return 1 - OutBounce(1 - t); }
    inline float InOutBounce(float t) { return t < 0.5 ? (1 - OutBounce(1 - 2 * t)) / 2 : (1 + OutBounce(2 * t - 1)) / 2; }

}
