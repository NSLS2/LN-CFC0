#ifndef IQMATH_H
#define IQMATH_H

#include <algorithm>
#include <limits>

#include <epicsMath.h>
#include <epicsTypes.h>

struct IQ {
    double I, Q;
};

static inline
double magnitude(double I, double Q)
{
    return sqrt(I*I+Q*Q);
}

static inline
double angle(double I, double Q)
{
    // I>0 and Q==0 gives 0 angle
    // result in radians
    return atan2(Q,I);
}

static inline
IQ AP2IQ(double amp, double pha)
{
    IQ ret;
    ret.I = amp*cos(pha);
    ret.Q = amp*sin(pha);
    return ret;
}

static inline
epicsInt16 squash16(epicsInt32 v)
{
    return std::max((epicsInt32)std::numeric_limits<epicsInt16>::min(),
                    std::min(v,
                             (epicsInt32)std::numeric_limits<epicsInt16>::max()));
}

static inline
epicsInt16 squash16(double v)
{
    return std::max((epicsInt32)std::numeric_limits<epicsInt16>::min(),
                    std::min((epicsInt32)lround(v),
                             (epicsInt32)std::numeric_limits<epicsInt16>::max()));
}


#endif // IQMATH_H
