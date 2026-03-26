#pragma once

inline float normalize(float currentValue, float originalMin, float originalMax, float targetMin, float targetMax) {
    return(targetMin+((currentValue-originalMin)*(targetMax-targetMin))/(originalMax-originalMin));
}