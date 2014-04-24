/** @file

    @brief math constants

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>

    <p>created 2014/04/21</p>
*/

#ifndef MOSRC_MATH_CONSTANTS_H
#define MOSRC_MATH_CONSTANTS_H

// -------------------- universal constants ----------------

/** the infamous PI */
#ifndef PI
#define PI (3.1415926535897932384626433832795)
#endif

/** 2.0 * PI */
#ifndef TWO_PI
#define TWO_PI (6.283185307179586476925286766559)
#endif

/** PI / 2.0 */
#ifndef HALF_PI
#define HALF_PI (1.5707963267948966192313216916398)
#endif

/** 2.0/3.0 * PI */
#ifndef TWOTHIRD_PI
#define TWOTHIRD_PI (2.0943951023931954923084289221863)
#endif

/** degree to 2*pi multiplier (TWO_PI/360.0) */
#ifndef DEG_TO_TWO_PI
#define DEG_TO_TWO_PI (PI/180.0)
#endif

/** 2*pi to degree multiplier (360.0/TWO_PI) */
#ifndef TWO_PI_TO_DEG
#define TWO_PI_TO_DEG (180.0/PI)
#endif

/** degree to radians multiplier (1.0/360.0) */
#ifndef DEG_TO_RAD
#define DEG_TO_RAD (0.0027777777777777777777777777777)
#endif

/** sqrt(2.0) */
#ifndef SQRT_2
#define SQRT_2 (1.4142135623730950488016887242097)
#endif

/** tanh(1.0) */
#ifndef TANH_1
#define TANH_1 (0.761594)
#endif

/** 1.0/tanh(1.0) */
#ifndef TANH_1_I
#define TANH_1_I (1.313035)
#endif



#endif // MOSRC_MATH_CONSTANTS_H
