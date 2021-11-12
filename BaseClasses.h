//
// Created by user on 03.09.2020.
//

#ifndef NEO_BASECLASSES_H
#define NEO_BASECLASSES_H

#include <sigslot/signal.hpp>
#include <memory>
#include <map>
#include <set>
#include <math.h>
#include "Primitives.h"

namespace Plot {

    class TConsts{
    public:
        static TColor TransparentColor();
        static TColor WhiteColor();
        static TColor BlackColor();
        static TColor BlueColor();
        static TColor RandColor();
        static TColor CreateColor(int r, int g, int b, int a = 255);

        static int GetAlpha(const TColor& value);
        static TColor SetAlpha(const TColor& value, int alpha);
    };

    enum TPenStyle { psNone, psSolid, psDash, psDot, psDashDot, psDashDotDot };
    enum TBrushStyle { bsNone, bsSolid};
    enum TFontStyle { fsNormal, fsBold, fsItalic};

    using TVecPointF = std::vector<TPointF>;

    class TPlot;
    CLASS_PTRS(Plot)

    class TLayer;
    CLASS_PTRS(Layer)

    class TLayerable;
    CLASS_PTRS(Layerable)
    using TVecLayerable = std::vector<TRawLayerable>;

    class TPixmap;
    class TRegion;

    class TPainter;
    CLASS_PTRS(Painter)

    class TPaintBuffer;
    CLASS_PTRS(PaintBuffer)

    template<typename T>
    class TPointer{
    public:
        TPointer() = default;
        TPointer(T* value)
        {
            Set(value);
        }
        TPointer(const TPointer<T>& oth)
        {
            Set(oth.ptr);
        }

        void Clear()
        {
            ptr = nullptr;
            id.disconnect();
        }
        void Set(T* value)
        {
            Clear();
            ptr = value;
            if(ptr)
                id = ptr->OnDestroy.connect(&TPointer<T>::Clear, this);
        }
        T* Get() const { return ptr; }
        T* operator ->() const { return ptr; }
        T& operator * () const { return *ptr; }
        bool IsEmpty() const { return ptr == nullptr; }
        bool IsValid() const { return ptr != nullptr; }
    private:
        T* ptr = nullptr;
        sigslot::scoped_connection id;
    };

    template<typename T>
    bool RemoveFrom(T& cont, const typename T::value_type& value)
    {
        for(auto it = cont.begin(); it != cont.end(); it++)
            if(*it == value)
            {
                cont.erase(it);
                return true;
            }
        return false;
    }

    struct TBrush{
        TColor color = TConsts::BlackColor();
        TBrushStyle style = bsSolid;
        TBrush() = default;
        TBrush(const TBrushStyle& s) : style(s){}
        TBrush(const TColor& c, const TBrushStyle& s = bsSolid): color(c), style(s){};
    };

    struct TPen{
        TColor color = TConsts::BlackColor();
        int width = 0;
        TPenStyle style = psSolid;
        TPen() = default;
        TPen(const TPenStyle& s): style(s){}
        TPen(const TColor& c, int w = 0, const TPenStyle& s = psSolid):color(c), width(w), style(s){}
    };

    struct TFont{
        TString name;
        int size = 11;              //размер в пикселях
        TFontStyle style = fsNormal;
        TFont(){}
        TFont(const TString& n, int si, TFontStyle st):name(n), size(si), style(st){}
        TFont(TFontStyle value): style(style){};
        TFont(int value): size(value){};
    };
    enum class TAlignText { Begin, Center, End};

    class TPainter{
    public:
        enum TPainterMode { pmDefault, pmVectorized, pmNoCaching, pmNonCosmetic};

        TPainter(){ mode.insert(pmDefault); }
        virtual ~TPainter() = default;
        virtual void Save() = 0;
        virtual void Restore() = 0;
        virtual void SetClipRect(const TRect& value) = 0;
        virtual void SetClipPolygon(const TVecPointF& points, bool isSub) = 0;
        virtual TRegion ClipRegion() const = 0;
        //virtual void SetClipRegion(const TRegion& value) = 0;
        virtual bool IsActive() const = 0;

        virtual bool Antialiasing() const = 0;
        virtual void SetAntialiasing(bool value) = 0;
        virtual void Offset(int x, int y) = 0;

        virtual TPen Pen() const = 0;
        virtual TBrush Brush() const = 0;

        virtual void SetPen(const TPen& value) = 0;
        virtual void SetBrush(const TBrush& value) = 0;
        virtual void SetFont(const TFont& value) = 0;
        virtual void DrawRect(const Plot::TRect& rect) = 0;
        virtual void FillRect(const TRect& rect, const TColor& color) = 0;

        virtual void DrawPixmap(int x, int y, const TPixmap& value) = 0;
        virtual void DrawLine(const TPointF& b, const TPointF& e) = 0;
        virtual void DrawLine(const TPoint& b, const TPoint& e) = 0;
        virtual void DrawEllipse(const TPointF& center, double rx, double ry) = 0;
        virtual void DrawText(const TPointF& p, const TString& value, TAlignText horz, TAlignText vert, double rotate) = 0;
        virtual void DrawPolyline(const TPointF* points, int pointCount) = 0;
        virtual void DrawPolygon(const TPointF* points, int pointCount) = 0;

        inline void DrawText(const TPointF& p, const TString& value, TAlignText horz = TAlignText::Begin, TAlignText vert = TAlignText::Begin)
        { DrawText(p, value, horz, vert, 0.); }

        void DrawPolyline(const TVecPointF& points){ DrawPolyline(&points[0], points.size()); }
        void DrawPolygon(const TVecPointF& points){ DrawPolygon(&points[0], points.size()); }
        void SetMode(TPainterMode value) { mode.insert(value); }
        void UnSetMode(TPainterMode value) { mode.erase(value); }
        bool TestFlag(TPainterMode value) { return mode.find(value) != mode.end(); }

        virtual int WidthText(const TString& value) = 0;
        virtual int HeightText(const TString& value) = 0;

    protected:
        std::set<TPainterMode> mode;

    };

    class TPaintBuffer{
    public:
        explicit TPaintBuffer(const TSize& value);
        virtual ~TPaintBuffer() = default;

        inline TSize Size() const { return size; }
        inline bool Invalidate() const { return invalidate; }

        void SetSize(const TSize& value);
        void SetInvalidated(bool value);

        virtual TUPtrPainter StartPaint() = 0;
        virtual void StopPaint(){};
        virtual void Draw(const TUPtrPainter& painter, int x, int y) = 0;
        virtual void Clear(const TColor& color) = 0;
    protected:
        TSize size;
        bool invalidate = true;
        virtual void ReallocBuffer() = 0;
    };
    enum class TLayerMode { Logical, Buffered };

    class TLayer{
    public:
        virtual ~TLayer();

        inline const TPlot& Plot() const { return *plot; }
        inline TString Name() const { return name; }
        inline int Index() const { return index; }
        inline bool IsVisible() const { return isVisible; }
        inline TLayerMode Mode() const { return mode; }

        void SetIsVisible(bool value);
        void SetMode(TLayerMode value);

        void SetBuffer(const TPtrPaintBuffer& value);
        void DrawToBuffer() const;

        const TVecLayerable& Children() const { return children; }
        void Replot();
    protected:
        TLayer(TRawPlot value, const TString& nameLay, int ind = 0);

        TRawPlot plot;
        TString name;
        int index = -1;
        bool isVisible = true;
        TLayerMode mode = TLayerMode::Logical;
        TVecLayerable children;
        TWPtrPaintBuffer buffer;

        void Draw(const TUPtrPainter& painter) const;
        void AddChild(TRawLayerable value, bool prepend);
        void DelChild(TRawLayerable value);

        void CheckIndexesInLayer();

    private:
        friend class TLayerable;
        friend class TPlot;
    };

    struct TMouseInfo{
        TMouseInfo() = default;
        TMouseInfo(const TPoint& p, const TPoint& gp):pos(p), gpos(gp){}
        TPoint pos;
        TPoint gpos;
        bool isLeftButton = false;
        bool isCtrl = false;
        bool isAlt = false;
        bool isShift = false;
        int delta = 0;
        inline bool IsLeftButton() const { return isLeftButton; }
        inline bool IsMultiSelect() const { return isCtrl; }
        inline bool IsMoving() const { return isCtrl; }

        inline void Accept() { isAccept = true; }
        inline void Ignore() { isAccept = false; }
        inline bool IsAccepted() const { return isAccept; }
    private:
        bool isAccept = false;
    };

    using TMouseInfoWheel = TMouseInfo;

    class TLayerable{
    public:
        virtual ~TLayerable();

        inline const TRawPlot Plot() const { return plot; }
        void Replot();

        inline TPtrLayer Layer() const { return layer.lock(); }
        void SetLayer(const TPtrLayer& value, bool prepend = false);
        void SetLayer(const TString& nameLayer, bool prepend = false);

        inline bool IsVisible() const { return isVisible; }
        void SetIsVisible(bool value);

        TPtrLayerable ParentLayerable() const;
        void SetParentLayerable(const TWPtrLayerable& value);

        int IndexInLayer() const { return indexInLayer; }
        void SetIndexInLayer(int index);

        // Возвращает видимость объекта,
        // если есть Layer, то он должен быть виден
        // если есть родитель, то он должен быть виден
        bool RealVisibility() const;

        virtual void MousePress(TMouseInfo& info)                       { info.Ignore(); };
        virtual void MouseMove(TMouseInfo& info, const TPoint& startPos){ info.Ignore(); };
        virtual void MouseUp  (TMouseInfo& info, const TPoint& startPos){ info.Ignore(); };
        virtual void MouseWheel(TMouseInfoWheel& info)                  { info.Ignore(); };

        virtual double SelectTest(const TPointF& pos, bool onlySelectable){ return -1;}
        //выбор элемента, возвращает изменился ли статус выделения
        virtual bool SelectEvent(TMouseInfo& info, bool additive){ return false; };
        virtual bool DeselectEvent() { return false; }

        virtual bool IsAntiAliasing() const { return false; }

        sigslot::signal<> OnDestroy;
        sigslot::signal<bool> OnSelectionChanged;
    protected:
        TLayerable(TRawPlot plt, const TString& nameLayer = TString());
        TLayerable(TRawPlot plt, const TPtrLayer& lay);

        TRawPlot plot;
        TWPtrLayer layer;
        TWPtrLayerable parentLayerable;

        bool isVisible = true;
        int indexInLayer = 0;

        virtual TRect ClipRect() const;
        virtual void Draw(const TUPtrPainter& painter){};
        template<typename T>
        bool SelectionChanged(T& value, const T& newValue);

    private:
        friend class TLayer;
        friend class TPlot;
    };

    enum TOrientation{ orHorz, orVert};
    enum TAxisType {atLeft, atTop, atBottom, atRight, atCount};
    enum TScaleType { stLinear, stLog};

    struct TRange{
        double lower = 0.;
        double upper = 0.;

        TRange() = default;
        TRange(double l, double u, bool norm = true): lower(l), upper(u){ if(norm) Normalize(); }

        void Set(double l, double u, bool norm = true){ lower = l; upper = u; if(norm) Normalize();}
        inline double Size() const { return upper - lower; }
        inline double Center() const { return (upper + lower) * 0.5;}
        inline double Percent(double value) const { return lower + Size() * value; }

        inline void Normalize(){ if(lower > upper) std::swap(lower, upper); }
        inline bool Contains(double value) const
        {
            return value >= lower && value <= upper;
        }

        inline bool IsValid() const { return ValidRange(*this); }

        inline void Reset() { lower = 1.; upper = 0.; }

        TRange CheckForLinScale() const
        {
            TRange res(lower, upper);
            res.Normalize();
            return res;
        }

        TRange CheckForLogScale() const
        {
            TRange res(lower, upper);
            res.Normalize();
            double rangeFac = 0.1;
            if(res.lower <= 0.)
                res.lower = rangeFac;

            if(res.upper <= 0.)
                res.upper = rangeFac * 10.;

            res.Normalize();//
            double chk = rangeFac;
            while(res.lower > chk * 10.)
                chk = chk * 10.;
            res.lower = chk;

            chk = rangeFac;
            while(res.upper > chk)
                chk *= 10;
            res.upper = chk;

            return res;
        }

        void Expand(const TRange oth)
        {
            if(lower > oth.lower || isnan(lower))
                lower = oth.lower;
            if(upper < oth.upper || isnan(upper))
                upper = oth.upper;
        }

        static bool ValidRange(const TRange& value)
        {
            /*return (value.lower > -MaxRange() &&
                    value.upper < MaxRange() &&
                    std::fabs(value.lower-value.upper) > MinRange() &&
                    std::fabs(value.lower-value.upper) < MaxRange() &&
                    !(value.lower > 0 && std::isinf((value.upper/value.lower)) &&
                    !(value.upper < 0 && std::isinf(value.lower/value.upper))));*/
            return value.upper - value.lower > 0;
        }

        static double MinRange() { static double minRange = 1e-280; return minRange; }
        static double MaxRange() { static double maxRange = 1e250; return maxRange; }
    };
}




#endif //NEO_BASECLASSES_H
