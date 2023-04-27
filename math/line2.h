#ifndef _MATH_LINE2_H
#define _MATH_LINE2_H

#include "vec2.h"

namespace math
{
    class Line2;

    class Line2
    {
        public:
            Vec2 a, b;

            Line2();
            Line2(const Vec2 &a, const Vec2 &b) { this->a = Vec2(a); this->b = Vec2(b); }

            float
            length(void) const;

            float
            length_sq(void) const;

            Vec2
            direction(void) const;
            
            // Comparisons
            bool operator==(const Line2& l) const { return this->a == l.a && this->b == l.b; }
            bool operator!=(const Line2& l) const { return !(*this == l); }
            bool operator> (const Line2 &l) const { return this->length_sq() >  l.length_sq(); }
            bool operator>=(const Line2 &l) const { return this->length_sq() >= l.length_sq(); }
            bool operator< (const Line2 &l) const { return this->length_sq() <  l.length_sq(); }
            bool operator<=(const Line2 &l) const { return this->length_sq() <= l.length_sq(); }

            Line2 operator-() const { return Line2(b, a); }

            bool
            intersects(const Line2 &l) const;

            Vec2
            get_intersection(const Line2 &l) const;
    };
}

#endif
