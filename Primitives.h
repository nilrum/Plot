//
// Created by user on 02.06.2020.
//

#ifndef NEO_PRIMITIVES_H
#define NEO_PRIMITIVES_H

#include <type_traits>
#include <functional>
#include <cmath>
#include "Types.h"
namespace Plot{

//TODO exclude this
using TColor = uint64_t;

template<typename T>
    class TSizeTempl{
    public:
        TSizeTempl() = default;
        TSizeTempl(T w, T h):wh(w), ht(h){};
        template<typename T2>
        TSizeTempl(const T2& value):wh(value.width()), ht(value.height()){};

        inline const T& height() const { return ht; }
        inline const T& width() const { return wh; }

        inline void setWidth(const T& value){ wh = value; }
        inline void setHeight(const T& value){ ht = value; }
        inline bool IsEmpty() const { return wh == 0 && ht == 0; }
    private:
        T wh = {};
        T ht = {};
    };

template<typename T>
inline bool operator == (const TSizeTempl<T>& one, const TSizeTempl<T>& two)
{
    return one.width() == two.width() && one.height() == two.height();
}
template<typename T>
inline bool operator != (const TSizeTempl<T>& one, const TSizeTempl<T>& two)
{
    return !(one == two);
}

template<typename T>
class TPointTempl{
public:
    TPointTempl() = default;
    TPointTempl(T posX, T posY):xp(posX), yp(posY){};

    template<typename T2>
    TPointTempl(const T2& oth): xp(T(oth.x())), yp(T(oth.y())){}

    inline const T& x() const { return xp; }
    inline const T& y() const { return yp; }

    inline T& rx() { return xp; }
    inline T& ry() { return yp; }

    inline void setX(T value) { xp = value; }
    inline void setY(T value) { yp = value; }

    template<typename T2>
    inline TPointTempl<T>& operator += (const TPointTempl<T2>& oth)
    {
        xp += oth.x();
        yp += oth.y();
        return *this;
    }

    inline TPointTempl<int> toPoint() const { return TPointTempl<int>( int(std::round(xp)), int(std::round(yp))); }

    inline void Set(T posX, T posY){ xp = posX, yp = posY; }
private:
    T xp = 0;
    T yp = 0;
};

template<typename T>
TPointTempl<T> operator + (const TPointTempl<T>& a, const TPointTempl<T>& b)
{
    return TPointTempl<T>(a.x() + b.x(), a.y() + b.y());
}

template<typename T>
TPointTempl<T> operator - (const TPointTempl<T>& a, const TPointTempl<T>& b)
{
    return TPointTempl<T>(a.x() - b.x(), a.y() - b.y());
}


template<typename T>
TPointTempl<T> operator * (const TPointTempl<T>& a, double factor)
{
    if constexpr( std::is_same<T, double>::value == true)
        return TPointTempl<T>(a.x() * factor, a.y() * factor);
    else
        return TPointTempl<T>(std::round(a.x() * factor), std::round(a.y() * factor));
}

template<typename T>
TPointTempl<T> operator * (double factor, const TPointTempl<T>& a)
{
    if constexpr( std::is_same<T, double>::value == true)
        return TPointTempl<T>(a.x() * factor, a.y() * factor);
    else
        return TPointTempl<T>(std::round(a.x() * factor), std::round(a.y() * factor));
}

using TSize = TSizeTempl<int>;
using TSizeF = TSizeTempl<double>;
using TPoint = TPointTempl<int>;
using TPointF = TPointTempl<double>;

class TRect{
public:

    TRect():x1(0), y1(0), x2(-1), y2(-1){}
    TRect(int left, int top, int width, int height):
            x1(left), y1(top), x2(left + width - 1), y2(top + height - 1){}

    TRect(const TPoint& topLeft, const TPoint& bottomRight):
            x1(topLeft.x()), y1(topLeft.y()), x2(bottomRight.x()), y2(bottomRight.y()){}
    TRect(const TPoint& topLeft, const TSize& size):
            x1(topLeft.x()), y1(topLeft.y()), x2(x1 + size.width() - 1), y2(y1 + size.height() - 1){}

    /*TRect(const TSize& size):
            x1(0), y1(0), x2(size.width() - 1), y2(size.height() - 1){}*/

    template<typename T2>
    TRect(const T2& oth):x1(oth.left()), y1(oth.top()), x2(oth.right()), y2(oth.bottom()){}


    inline int left() const { return x1; }
    inline int top() const { return y1; }
    inline int right() const { return x2; }
    inline int bottom() const { return y2; }
    inline int width() const { return x2 - x1 + 1; }
    inline int height() const { return y2 - y1 + 1; }

    inline bool isNull() const { return x2 == x1 - 1 && y2 == y1 - 1; }

    inline void setLeft(int value) { x1 = value; }
    inline void setTop(int value) { y1 = value; }
    inline void setRight(int value) { x2 = value; }
    inline void setBottom(int value) { y2 = value; }

    inline void setWidth(int value) { x2 = x1 + value - 1; }
    inline void setHeight(int value) { y2 = y1 + value - 1; }

    inline TPoint topLeft() const { return TPoint(x1, y1);}
    inline TPoint bottomLeft() const { return TPoint(x1, y2);}

    inline TPoint topRight() const { return TPoint(x2, y1);}
    inline TPoint bottomRight() const { return TPoint(x2, y2);}

    inline TRect translated(int dx, int dy){ return TRect(TPoint(x1 + dx, y1 + dy), TPoint(x2 + dx, y2 + dy)); }
    inline TRect adjusted(int dx1, int dy1, int dx2, int dy2){ return TRect(TPoint(x1 + dx1, y1 + dy1), TPoint(x2 + dx2, y2 + dy2)); }

    inline TSize size() const { return TSize(width(), height());}

    inline TPoint center() const { return {CenterHorz(), CenterVert()}; }
    inline int CenterHorz() const { return x1 + width() / 2.; }
    inline int CenterVert() const { return y1 + height() / 2.; }

    inline void translate(const TPoint& p) { x1 += p.x(); y1 += p.y(); x2 += p.x(); y2 += p.y(); }

    bool contains(const TPoint &p, bool proper = false) const;
    bool intersects(const TRect &r) const;
private:
    int x1;
    int y1;
    int x2;
    int y2;
};


class TRectF{
public:
    TRectF():xp(0.), yp(0.), w(0.), h(0.){}
    TRectF(double left, double top, double width, double height):
            xp(left), yp(top), w(width), h(height){}
    TRectF(const TPointF& topLeft, const TPointF& bottomRight):
            xp(topLeft.x()), yp(topLeft.y()), w(bottomRight.x() - topLeft.x()), h(bottomRight.y() - topLeft.y()){}

    template<typename T2>
    TRectF(const T2& oth):xp(oth.left()), yp(oth.top()), w(oth.width()), h(oth.height()){}


    inline double left() const { return xp; }
    inline double top() const { return yp; }
    inline double right() const { return xp + w; }
    inline double bottom() const { return yp + h; }
    inline double width() const { return w; }
    inline double height() const { return h; }

    inline bool isNull() const { return w == 0. && h == 0.; }

    //inline void setLeft(double value) { double diff = value - xp; xp += diff; w -= diff;}
    //inline void setTop(double value) { double diff = value - yp; yp += diff; h -= diff; }
    inline void setLeft(double value) { xp = value; }
    inline void setTop(double value) { yp = value; }
    inline void setRight(double value) { w = value - xp; }
    inline void setBottom(double value) { h = value - yp; }

    inline void setWidth(double value) { w = value; }
    inline void setHeight(double value) { h = value; }

    inline TPointF topLeft() const { return TPointF(xp, yp);}
    inline TPointF bottomLeft() const { return TPointF(xp, yp + h);}

    inline TPointF topRight() const { return TPointF(xp + w, yp);}
    inline TPointF bottomRight() const { return TPointF(xp + w, yp + h);}

    TRect toRect() const { return TRect(int(std::round(xp)), int(std::round(yp)), int(std::round(xp + w) - 1), int(std::round(yp + h) - 1)); }
    TSizeF size() const { return TSizeF(width(), height()); }

    TRectF normalized() const;

    inline TPointF center() const { return {CenterHorz(), CenterVert()}; }
    inline double CenterHorz() const { return xp + w / 2.; }
    inline double CenterVert() const { return yp + h / 2.; }

    bool contains(const TPoint &p, bool proper = false) const;
    void adjust(double xp1, double yp1, double xp2, double yp2){ xp += xp1; yp += yp1; w += xp2 - xp1; h += yp2 - yp1; }
private:
    double xp;
    double yp;
    double w;
    double h;
};

class TMargins{
public:
    TMargins():l(0), t(0), r(0), b(0){}
    TMargins(int all):l(all), t(all), r(all), b(all){}
    TMargins(int left, int top, int right, int bottom):l(left), t(top), r(right), b(bottom){}

    inline int left() const{ return l; }
    inline int top() const{ return t; }
    inline int right() const{ return r; }
    inline int bottom() const{ return b; }
    friend inline bool operator==(const TMargins &m1, const TMargins &m2);
private:
    int l;
    int t;
    int r;
    int b;
};

inline bool operator==(const TMargins &m1, const TMargins &m2)
{
    return m1.l == m2.l && m1.t == m2.t && m1.r == m2.r && m1.b == m2.b;
};

inline bool operator!=(const TMargins &m1, const TMargins &m2)
{
    return !(m1 == m2);
};

template<typename T>
class TStateValue{
    T data;
    T def;
    bool isInit = false;
    bool isNull = false;
    TStateValue(const T& defValue):data(defValue), def(defValue){}
    TStateValue(const T& value, const T& defValue):data(value), def(defValue), isInit(true){}
public:
    using type_data = T;
    static TStateValue Default(const T& defValue) { return TStateValue(defValue); }
    static TStateValue Value(const T& value) { return TStateValue(value, 0); }

    TStateValue(): isInit(true), isNull(true), data(0), def(0){}
    TStateValue(const TStateValue<T>& oth): data(oth.data), def(oth.def), isInit(oth.isInit), isNull(oth.isNull){}
    T get() const
    {
        if(isInit == false) return def;
        if(isNull) return 0.;
        return data;
    }

    bool IsNull() const { return isNull; }
    bool IsInit() const { return isInit; }
    bool IsDefault() const { return !isInit; }

    inline void operator = (const T& value)
    {
        data = value;
        isInit = true;
        isNull = false;
    }
    inline void operator = (const TStateValue<T>& value)
    {
        //if(value.isInit == false) return;
        //if(value.isNull && isInit == true) return;
        def = value.def;
        data = value.data;
        isInit = value.isInit;
        isNull = value.isNull;
    }
    template<typename TOth>
    friend bool operator < (const TStateValue<TOth>& left, const TStateValue<TOth>& right);

    template<typename TOth>
    friend bool operator > (const TStateValue<TOth>& left, const TStateValue<TOth>& right);

    template<typename TOth>
    friend TStateValue<TOth> operator + (const TStateValue<TOth>& left, const TStateValue<TOth>& right);

    template<typename TOth>
    friend TStateValue<TOth> operator - (const TStateValue<TOth>& left, const TStateValue<TOth>& right);

    template<typename TOth, typename TArithmetic>
    friend TStateValue<TOth> Arithmetic(const TStateValue<TOth>& left, const TStateValue<TOth>& right);

    template<typename TOth, typename TLogic>
    friend bool Logic(const TStateValue<TOth>& left, const TStateValue<TOth>& right);
};

template<typename T, typename TLogic>
    bool Logic(const TStateValue<T>& left, const TStateValue<T>& right)
    {
        if(left.isNull || right.isNull)
            return true;     //если значение не имеется то оно всегда вернет true

        if(left.isInit == false && right.isInit == false)
            return true;//если значение не инициализированно то оно всегда возвращает true

        if(left.isInit == false)
            return TLogic()(left.def, right.data);

        if(right.isInit == false)
            return TLogic()(left.data, right.def);

        return TLogic()(left.data, right.data);            //только когда есть реальное значение происходит проверка
    }

template<typename T>
    bool operator < (const TStateValue<T>& left, const TStateValue<T>& right)
    {
        return Logic<T, std::less<T>>(left, right);
    }

template<typename T>
    bool operator > (const TStateValue<T>& left, const TStateValue<T>& right)
    {
        return Logic<T, std::greater<T>>(left, right);
    }

template<typename T, typename TArithmetic>
    TStateValue<T> Arithmetic(const TStateValue<T>& left, const TStateValue<T>& right)
{
        if(left.isInit == false && right.isInit == false)//если оба не инициал
            return TStateValue<T>::Default(left.def);//возвращаем значение по умолчанию

        if(left.isNull && right.isNull)
            return TStateValue<T>();

        if(left.isNull)
            return right;

        if(right.isNull)
            return left;

        if(left.isInit == false)
        {
            if(left.def == INT_MAX)
                return left;
            else
                TStateValue<T>::Value(TArithmetic()(left.def, right.data));
        }

        if(right.isInit == false)
        {
            if(right.def == INT_MAX)
                return right;
            else
                TStateValue<T>::Value(TArithmetic()(left.data, right.def));
        }

        return TStateValue<T>::Value(TArithmetic()(left.data, right.data));
}

template<typename T>
    TStateValue<T> operator + (const TStateValue<T>& left, const TStateValue<T>& right)
    {
        return Arithmetic<T, std::plus<T>>(left, right);
    }

template<typename T>
    TStateValue<T> operator - (const TStateValue<T>& left, const TStateValue<T>& right)
{
        return Arithmetic<T, std::minus<T>>(left, right);
}

    using TStateInt = TStateValue<int>;
    using TSizeState = TSizeTempl<TStateInt>;

    using TSizeBool = TSizeTempl<bool>;
}



#endif //NEO_PRIMITIVES_H
