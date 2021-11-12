//
// Created by user on 02.06.2020.
//

#include "Primitives.h"
namespace Plot {

    bool TRect::contains(const TPoint &p, bool proper) const
    {
        int l, r;
        if (x2 < x1 - 1)
        {
            l = x2;
            r = x1;
        }
        else
        {
            l = x1;
            r = x2;
        }
        if (proper)
        {
            if (p.x() <= l || p.x() >= r)
                return false;
        }
        else
        {
            if (p.x() < l || p.x() > r)
                return false;
        }
        int t, b;
        if (y2 < y1 - 1)
        {
            t = y2;
            b = y1;
        }
        else
        {
            t = y1;
            b = y2;
        }
        if (proper)
        {
            if (p.y() <= t || p.y() >= b)
                return false;
        }
        else
        {
            if (p.y() < t || p.y() > b)
                return false;
        }
        return true;
    }

    bool TRect::intersects(const TRect &r) const
    {
        if (isNull() || r.isNull())
            return false;

        int l1 = x1;
        int r1 = x1;
        if (x2 - x1 + 1 < 0)
            l1 = x2;
        else
            r1 = x2;

        int l2 = r.x1;
        int r2 = r.x1;
        if (r.x2 - r.x1 + 1 < 0)
            l2 = r.x2;
        else
            r2 = r.x2;

        if (l1 > r2 || l2 > r1)
            return false;

        int t1 = y1;
        int b1 = y1;
        if (y2 - y1 + 1 < 0)
            t1 = y2;
        else
            b1 = y2;

        int t2 = r.y1;
        int b2 = r.y1;
        if (r.y2 - r.y1 + 1 < 0)
            t2 = r.y2;
        else
            b2 = r.y2;

        if (t1 > b2 || t2 > b1)
            return false;

        return true;
    }

    TRectF TRectF::normalized() const
    {
        TRectF r = *this;
        if (r.w < 0)
        {
            r.xp += r.w;
            r.w = -r.w;
        }
        if (r.h < 0)
        {
            r.yp += r.h;
            r.h = -r.h;
        }
        return r;
    }

    bool TRectF::contains(const TPoint &p, bool proper) const
    {
        double l = xp;
        double r = xp;
        if (w < 0)
            l += w;
        else
            r += w;
        if (l == r) // null rect
            return false;

        if (p.x() < l || p.x() > r)
            return false;

        double t = yp;
        double b = yp;
        if (h < 0)
            t += h;
        else
            b += h;
        if (t == b) // null rect
            return false;

        if (p.y() < t || p.y() > b)
            return false;

        return true;
    }
}