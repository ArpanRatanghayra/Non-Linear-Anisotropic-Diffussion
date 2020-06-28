#include "pm.h"
#include <math.h>   
#include <stdlib.h> 
#define PM_RED(rgb)     (( (rgb) >> 16) & 0xffu)
#define PM_GREEN(rgb)   (( (rgb) >> 8 ) & 0xffu)
#define PM_BLUE(rgb)    (  (rgb)        & 0xffu)
#define PM_RGB(r, g, b) (( (r) & 0xffu) << 16) | (( (g) & 0xffu) << 8) | ( (b) & 0xffu)

static int getChannel(uint rgb, int channel)
{
    switch(channel) {
        case 0:
            return PM_RED(rgb);
        case 1:
            return PM_GREEN(rgb);
        case 2:
            return PM_BLUE(rgb);
    }
    return 0;
}

static int applyChannel(img_data *idata, proc_data *pdata, int x, int y, int ch)
{
    int p = getChannel(idata->bits[x + y * idata->w], ch);
    int deltaW = getChannel(idata->bits[x + (y-1) * idata->w], ch) - p;
    int deltaE = getChannel(idata->bits[x + (y+1) * idata->w], ch) - p;
    int deltaS = getChannel(idata->bits[x+1 + y * idata->w], ch) - p;
    int deltaN = getChannel(idata->bits[x-1 + y * idata->w], ch) - p;
    float cN = pdata->conduction_ptr(abs(deltaN), pdata->thresh);
    float cS = pdata->conduction_ptr(abs(deltaS), pdata->thresh);
    float cE = pdata->conduction_ptr(abs(deltaE), pdata->thresh);
    float cW = pdata->conduction_ptr(abs(deltaW), pdata->thresh);
    return p + pdata->lambda * (cN * deltaN + cS * deltaS + cE * deltaE + cW * deltaW);
}

void pm(img_data *idata, proc_data *pdata)
{
    for(int it = 0; it < pdata->iterations; ++it) {
        for(int y = 1; y < idata->h-1; ++y) {
            for(int x = 1; x < idata->w-1; ++x) {
                int r = applyChannel(idata, pdata, x, y, 0);
                int g = applyChannel(idata, pdata, x, y, 1);
                int b = applyChannel(idata, pdata, x, y, 2);
                int a = getChannel(idata->bits[x+y*idata->w], 3);
                idata->bits[x+y*idata->w] = PM_RGB(r, g, b);
            }
        }
    }
}

float pm_quadric(int norm, float thresh)
{
    return 1.0f / (1.0f + norm * norm / (thresh * thresh));
}

float pm_exponential(int norm, float thresh)
{
    return exp(- norm * norm / (thresh * thresh));
}