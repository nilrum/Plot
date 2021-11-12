//
// Created by user on 22.05.2020.
//

#ifndef TESTQT_PLOTITEMS_H
#define TESTQT_PLOTITEMS_H

#include "Plot.h"

namespace Plot {

    class TAbstractItem;
    CLASS_PTRS(AbstractItem)

    class TItemAnchor{
    public:
        TItemAnchor(TRawPlot value, TPtrAbstractItem parent, const TString& nameItem = ""):
            plot(value), parentItem(parent), name(nameItem) { }
        virtual ~TItemAnchor() = default;

        virtual TPointF PixelPosition() const { return TPointF(); };
    protected:
        TRawPlot plot;
        TWPtrAbstractItem parentItem;
        TString name;
    };

    CLASS_PTRS(ItemAnchor)

    enum class TTypePosition{  Absolute,    //абсолютная позиция в координатах, если есть parentAnchor, то исходя из его координат
        ViewportRatio,//значение указывается в коэфф от размеров viewport, если есть parentAnchor, то исходя из его координат
        AxisRectRatio,//значение указывается в коэфф от размеров axisRect, если есть parentAnchor, то исходя из его координат
        PlotCoords    //значение указывается в величинах axis и результат получается исходя из axis
    };
    class TItemPosition : public TItemAnchor{
    public:
        TItemPosition(TRawPlot value);

        inline TTypePosition Type() const { return typePositionX; }
        inline TTypePosition TypeX() const { return typePositionX; }
        inline TTypePosition TypeY() const { return typePositionY; }

        void SetType(TTypePosition value);
        void SetTypeX(TTypePosition value);
        void SetTypeY(TTypePosition value);

        void SetAxises(const TPtrAxis& key,const TPtrAxis& val);

        void SetCoords(double k, double v);
        void SetKey(double value) { key = value; }
        void SetVal(double value) { val = value; }

        virtual TPointF PixelPosition() const override;
        virtual double PixelToKey(const TPoint& value);
        virtual double PixelToVal(const TPoint& value);

        inline double Key() const { return key; }
        inline double Val() const { return val; }

        inline TPtrAxis KeyAxis() const { return keyAxis.lock(); }
        inline TPtrAxis ValAxis() const { return valAxis.lock(); }

        bool CheckPlotCoords() const;
    protected:
        TTypePosition typePositionX = TTypePosition::Absolute;
        TTypePosition typePositionY = TTypePosition::Absolute;

        double key = 0.;
        double val = 0.;

        TWPtrAxis keyAxis;
        TWPtrAxis valAxis;

        TWPtrItemAnchor parentAnchorX;
        TWPtrItemAnchor parentAnchorY;

    };

    class TAbstractItem : public TLayerable{
    public:
        TAbstractItem(TRawPlot value, const TString &nameLayer = "");
        ~TAbstractItem();

        inline bool IsSelectable() const { return isSelectable; }
        inline bool IsSelected() const { return isSelected; }

        void SetIsSelectable(bool value);
        bool SetIsSelected(bool value);

    protected:
        bool isSelectable = false;
        bool isSelected = false;

        bool SelectEvent(TMouseInfo& info, bool additive) override;
        bool DeselectEvent() override;
    };

    class TItemTracer : public TAbstractItem{
    public:
        TItemTracer(TRawPlot value);

        enum TTracerStyle { tsNone, tsPlus, tsCross, tsCircle, tsSquare };

        bool UpdatePosition();
        double SelectTest(const TPointF& pos, bool onlySelectable) override;

        inline TPtrPlottable Graph() const { return graph.lock(); }
        void SetGraph(const TPtrPlottable& value);
        void SetGraphKey(double value);

        inline void SetPen(const TPen& value){ pen = value; }
        inline void SetBrush(const TBrush& value){ brush = value; }

        TPoint TextPostion() const;
        void SetTextPosition(const TPoint& value);
    protected:
        TPen pen{TConsts::BlackColor(), 2};
        TBrush brush{ bsNone };
        double size = 6.;
        TTracerStyle style = tsPlus;
        TWPtrPlottable graph;
        double graphKey = 0.;
        bool isInterpolating = false;
        TPoint textPosition{INT_MAX, INT_MAX};
        std::unique_ptr<TItemPosition> position = nullptr;
        void Draw(const TUPtrPainter& painter) override;

        void UpdateTracer(const TMouseInfo &info);

    };

    class TItemBorder : public TAbstractItem{
    public:
        TItemBorder(const TPtrAxis& key, const TPtrAxis& val);

        void SetKeyVal(double key, double val = 0);
        inline double Key() const { return position.Key(); }
        inline double Val() const { return position.Val(); }

        inline TPen ItemPen() const { return itemPen; }
        void SetItemPen(const TPen& value) { itemPen = value; }

        inline TPen SelPen() const { return selPen; }
        void SetSelPen(const TPen& value) { selPen = value; }

        void MousePress(TMouseInfo& info) override;
        void MouseMove(TMouseInfo& info, const TPoint& startPos) override;
        void MouseUp( TMouseInfo& info, const TPoint& startPos) override;
        double SelectTest(const TPointF& pos, bool onlySelectable) override;

        inline void SetTitle(const TString& value, TAlignText align = TAlignText::Begin) { title = value; alignTitle = align; }
        inline TString Title() const { return title; }

        bool IsMaskRect() const;
        void SetIsMaskRect(bool value);

        sigslot::signal<double> OnPositionChanged;
    private:
        TString title;
        TAlignText alignTitle;
        bool isEditing = false;
        double beginMoving = 0.;
        TItemPosition position;
        TPen itemPen{ Plot::TConsts::BlueColor(), 1 };
        TPen selPen{ TConsts::BlueColor(), 2 };
        bool isMaskRect = false;
        void Draw(const TUPtrPainter& painter) override;

        void GetPosLine(TPointF& begin, TPointF& end);
        TRect ClipRect() const override;
    };

    CLASS_PTRS(ItemBorder)

    class TItemText : public TAbstractItem{
    public:
        TItemText(Plot::TRawPlot plt);

        void SetText(const TString& value);
        TString Text() const;

        TColor ColorText() const;
        void SetColorText(const TColor& value);

        TColor ColorBack() const;
        void SetColorBack(const TColor& value);

        size_t FontSize() const;
        void SetFontSize(size_t value);

        const TItemPosition& Position() const;
        TItemPosition& Position();

        double SelectTest(const TPointF& pos, bool onlySelectable) override;
        void MousePress(TMouseInfo& info) override;
        void MouseMove(TMouseInfo& info, const TPoint& startPos) override;
        void MouseUp  (TMouseInfo& info, const TPoint& startPos) override;

        sigslot::signal<TPointF> OnPositionChanged;
    protected:
        TItemPosition position;
        TString text;
        TColor colorText = TConsts::BlackColor();
        TColor colorBack = TConsts::WhiteColor();
        size_t fontSize = 10;
        TRectF textRect;

        bool isEditing = false;
        TPointF beginMoving;
        void Draw(const TUPtrPainter& painter) override;

    };

    CLASS_PTRS(ItemText)
}

#endif //TESTQT_PLOTITEMS_H
