#ifndef EPSILON
#   define EPSILON (1e-6)
#endif

#ifndef PI
#   define PI (3.14159265358979)
#endif

#ifndef TAU
#   define TAU (6.283185307)
#endif

#ifndef TWO_PI
#   define TWO_PI TAU
#endif

#ifndef HALF_PI
#   define HALF_PI (1.5707963268)
#endif

/** Converts degree to radians */
float degree(float degree) { return degree * PI / 180.; }

