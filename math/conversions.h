/*
    Collection of conversions between mainly
    SI and imperial units of measurement.
    
    Phvli 2017-11-03
*/

#ifndef _MATH_CONVERSIONS_H
#define _MATH_CONVERSIONS_H

namespace math
{
    namespace convert
    {
        const float
            // Angles
            DEG_TO_RAD   = 0.01745329252f,     // degrees         -> radians
            RAD_TO_DEG   = 57.29577951f,       // radians         -> degrees

            // Common units of speed
            MS_TO_KMH    = 3.6f,               // m/s             -> km/h (exact)
            MS_TO_MPH    = 2.23693629f,        // m/s             -> miles per hour
            MS_TO_FTS    = 3.280839895f,       // m/s             -> feet per second
            MS_TO_MACH   = 0.002949f,          // m/s             -> mach (at sea level)
            
            KMH_TO_MS    = 0.277777778f,       // km/h            -> m/s
            KMH_TO_MPH   = 0.621371192f,       // km/h            -> miles per hour
            KMH_TO_MACH  = 0.000817661488f,    // km/h            -> mach (at sea level)

            MPH_TO_MS    = 0.44704f,           // miles per hour  -> m/s (exact)
            MPH_TO_KMH   = 1.609344f,          // miles per hour  -> km/h (exact)
            MPH_TO_MACH  = 0.0013183296f,      // miles per hour  -> mach (at sea level)
            FTS_TO_MS    = 0.3048f,            // feet per second -> m/s (exact)

            MACH_TO_KMH  = 1223.0f,            // mach            -> km/h (at sea level)
            MACH_TO_MS   = 339.722f,           // mach            -> m/s (at sea level)
            MACH_TO_MPH  = 758.5406212f,       // mach            -> miles per hour (at sea level)

            // Imperial units of length
            FT_TO_M      = 0.3048f,            // foot            -> meters (exact)
            IN_TO_M      = 0.0254f,            // inch            -> meters (exact)
            YD_TO_M      = 0.9144f,            // yard            -> meters (exact)
            MI_TO_M      = 1609.344f,          // statute mile    -> meters (exact)
            
            M_TO_IN      = 39.37007874f,       // meter           -> inches
            M_TO_FT      = 3.280839895f,       // meter           -> feet
            M_TO_YD      = 1.093613298f,       // meter           -> yards
            M_TO_MI      = 0.000621371192f,    // meter           -> statute miles

            // (Aero)nautical units
            NM_TO_M      = 1852.0f,            // nautical mile   -> meter (exact)
            NM_TO_KM     = 1.852f,             // nautical mile   -> kilometer (exact)
            NM_TO_MI     = 1.1508f,            // nautical mile   -> mile
            NM_TO_FT     = 6076.1f,            // nautical mile   -> foot

            M_TO_NM      = (1.0f / NM_TO_M),   // nautical mile   -> meter
            KM_TO_NM     = (1.0f / NM_TO_KM),  // nautical mile   -> km
            MI_TO_NM     = (1.0f / NM_TO_MI),  // nautical mile   -> statute mile
            FT_TO_NM     = (1.0f / NM_TO_FT),  // nautical mile   -> feet
            
            KN_TO_MS     = (1.852f / 3.6f),    // knot            -> m/s
            KN_TO_KMH    = 1.852f,             // knot            -> km/h (exact)
            KN_TO_MPH    = 1.150779f,          // knot            -> miles per hour
            
            MS_TO_KN     = (1.0f / KN_TO_MS),  // knot            -> m/s
            KMH_TO_KN    = (1.0f / KN_TO_KMH), // knot            -> km/h
            MPH_TO_KN    = (1.0f / KN_TO_MPH); // knot            -> miles per hour
            
        const long int
            // Time
            SEC_IN_MIN   = 60,                 // seconds in minute
            SEC_IN_HOUR  = 3600,               // seconds in hour
            SEC_IN_DAY   = 86400,              // seconds in day
            SEC_IN_MONTH = 2592000,            // seconds in 30 days
            SEC_IN_YEAR  = 31557600;           // seconds in 365.25 days
    }
}

#endif
