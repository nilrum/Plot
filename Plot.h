//
// Created by user on 03.06.2020.
//

#ifndef NEO_PLOT_H
#define NEO_PLOT_H

#include <climits>
#include "Plottables.h"
#include "Algorithms.h"


namespace Plot {

    class TLayoutElement;
    CLASS_PTRS(LayoutElement)
    using TVecLayoutElements = std::vector<TPtrLayoutElement>;

    class TLayout;
    CLASS_PTRS(Layout)

    class TTitle;
    CLASS_PTRS(Title)

    class TLegend;
    CLASS_PTRS(Legend)

    class TLegendItem;
    CLASS_PTRS(LegendItem)

    class TLayoutStack;
    CLASS_PTRS(LayoutStack)

    class TLayoutGrid;
    CLASS_PTRS(LayoutGrid)

    class TSelectRect;
    CLASS_PTRS(SelectRect);

    class TMarginGroup{
    public:
        TMarginGroup(TOrientation orientation);
        int Offset(TRawLayoutElement current) const;//считает разницу между текущим элементом и максимальным значением в группе
    private:
        friend class TLayoutElement;
        TOrientation orient;
        std::map<TRawLayoutElement, TVecLayoutElements> elements;

        void Add(TRawLayoutElement value, TPtrLayoutElement check);
        void Del(TRawLayoutElement value);
    };
    enum class TTypePhase{MinMax, Group, Sizes};

    using TPtrMarginGroup = std::shared_ptr<TMarginGroup>;
    using TMinMaxSize = std::pair<TSize, TSize>;
    using TMinMaxSizeState = std::pair<TSizeState, TSizeState>;

    class TLayoutElement : public TLayerable{
    public:
        ~TLayoutElement() override;

        void Update();

        void CalcInnerRect();
        virtual TMinMaxSizeState CalcMinMaxSizes();
        virtual void CalcSizes(){};
        virtual bool ValidSizes() const { return true; }

        inline bool HasParent() const { return parentLayout.expired() == false; }
        inline TPtrLayout Layout() const { return parentLayout.lock(); }
        void SetLayout(const TPtrLayout& value);

        inline TSize MinSize() const { return {minSize.width().get(), minSize.height().get()}; }
        inline TSize MaxSize() const { return {maxSize.width().get(), maxSize.height().get()}; }
        inline const TSizeState& MinOrMaxSize(bool isMin) { return (isMin) ? minSize : maxSize; }
        inline const TRect& OutRect() const { return outRect; }
        inline const TRect& InnerRect() const { return innerRect; }
        inline const TMargins& Margins() const { return margins; }

        inline int MinSize(TOrientation orient) { return (orient == orVert) ? MinSize().height() : MinSize().width(); }
        inline int MaxSize(TOrientation orient) { return (orient == orVert) ? MaxSize().height() : MaxSize().width(); }

        inline int InLeft() const   { return innerRect.left(); }
        inline int InTop() const    { return innerRect.top(); }
        inline int InRight() const  { return innerRect.right(); }
        inline int InBottom() const { return innerRect.bottom(); }
        inline int InWidth() const  { return innerRect.width();}
        inline int InHeight() const { return innerRect.height();}

        void SetMinSize(const TSize& value);
        void SetMaxSize(const TSize& value);
        void SetOutRect(const TRect& value);
        void SetMargins(const TMargins& value);

        bool IsFixedWidth() const;
        bool IsFixedHeight() const;
        void SetFixedSize(const TSize& value);
        void SetFixedWidth(int value);
        void SetFixedHeight(int value);
        void ResetFixedWidth();
        void ResetFixedHeight();

        void SetMinMaxWidth(int min, int max);
        void SetMinMaxHeight(int min, int max);

        //обновлять автоматически мин и макс размеры(актуально для наследников)
        inline TSizeBool IsAutoMinMax() const { return isAutoMinMax; }
        void SetIsAutoMinMax(const TSizeBool& value) { isAutoMinMax = value; }

        const TPtrMarginGroup& MarginGroup(TAxisType type) const;
        void SetMarginGroup(TAxisType type, const TPtrMarginGroup& value, const TPtrLayoutElement& check);
        double SelectTest(const TPointF& pos, bool onlySelectable) override;
    protected:
        TLayoutElement(TRawPlot plt, const TString& layerName = TString());

        TWPtrLayout parentLayout;
        TSizeState minSize = {TStateInt::Default(0), TStateInt::Default(0)};
        TSizeState maxSize = {TStateInt::Default(INT_MAX), TStateInt::Default(INT_MAX)};
        TRect innerRect = TRect(0, 0, 0, 0);
        TRect outRect = TRect(0, 0, 0, 0);
        TMargins margins;
        TPtrMarginGroup marginGroups[atCount];
        //автоматически подстраивать мин макс размеры под мин макс размеры вложенных элементов
        TSizeBool isAutoMinMax = {true, true};
        template<typename T>
        std::shared_ptr<T> ThisShared()
        {
            return std::dynamic_pointer_cast<T>(thisWeak.lock());
        }
        virtual void SetThisWeak(TWPtrLayoutElement value){ thisWeak = value; };
        friend class TPlot;

    private:
        TWPtrLayoutElement thisWeak;
    };

    class TLayout : public TLayoutElement{
    public:
        void UpdateSizeConstraints();   //обновить ограничения размеров
        bool ValidSizes() const { return CountElements(); }
        virtual size_t CountElements() const = 0;
        virtual TPtrLayoutElement Element(size_t index) const = 0;
        virtual int IndexElement(const TPtrLayoutElement& value) = 0;

        virtual TPtrLayoutElement Take(size_t index) = 0;
        TPtrLayoutElement Take(const TPtrLayoutElement& value);
        bool Remove(size_t index);
        bool Remove(const TPtrLayoutElement& value);

        void Clear();
        virtual void Simplify(){};

        inline bool IsAutoSize() const { return isAutoSize; }
        void SetIsAutoSize(bool value) { isAutoSize = value; }
    protected:
        //автоматически подстраивать свои размеры чтобы вместить мин размеры
        bool isAutoSize = false;
        TLayout(TRawPlot plt, const TString& layerName = TString());

        TVecDouble CalcSectionSizes(const TVecInt& minSizes, const TVecInt& maxSizes, const TVecDouble& stretch, int totalSize);
    private:
        friend class TLayerable;
    };

    class TLayoutGrid : public TLayout{
    public:
        void CalcSizes() override;
        TMinMaxSizeState CalcMinMaxSizes() override;

        inline size_t CountRow() const { return elements.size(); };
        inline size_t CountCol() const { return elements.empty() ? 0 : elements[0].size(); };

        inline const TVecDouble& RowStretches() const { return rowStretches; }
        inline const TVecDouble& ColStretches() const { return colStretches; }

        int RowSpacing() const { return rowSpacing; }
        int ColSpacing() const { return colSpacing; }

        void SetRowStretch(size_t row, double value);
        void SetColStretch(size_t col, double value);
        void SetRowStretches(const TVecDouble& value);
        void SetColStretches(const TVecDouble& value);

        void SetRowSpacing(int value);
        void SetColSpacing(int value);

        size_t CountElements() const override;
        TPtrLayoutElement Element(size_t index) const override;
        TPtrLayoutElement Take(size_t index) override;
        int IndexElement(const TPtrLayoutElement& value) override;

        template<bool isCol = true>
        void MoveColOrRow(size_t opos, size_t npos, size_t fix)
        {
            TakeAndMove(elements, opos, npos, TIndexerMatrix<decltype(elements), isCol>{fix});
        }

        inline size_t MaxAddCols() const { return maxAddCols; }
        inline void SetMaxAddCols(size_t value){ maxAddCols = value; }

        TPtrLayoutElement Element(size_t row, size_t col) const;

        //добавляет элемент по индексу строки столбца, если элемент уже есть заменяет его
        const TPtrLayoutElement& AddElement(size_t row, size_t col, const TPtrLayoutElement& value);

        //добавляет элемент в следующий столбец
        const TPtrLayoutElement& AddElement(const TPtrLayoutElement& value);

        bool HasElement(size_t row, size_t col) const;
        void ExpandTo(size_t newRowCount, size_t newColCount);
        void InsertRow(size_t row);
        void InsertCol(size_t col);
        void Simplify() override;
        bool IndexToRowCol(size_t index, int& row, int& col) const;
    protected:
        TLayoutGrid(TRawPlot plt, const TString& layerName = TString());
        std::vector<TVecLayoutElements> elements;
        TVecDouble rowStretches;
        TVecDouble colStretches;
        int rowSpacing = 0;
        int colSpacing = 0;
        size_t maxAddCols = INT_MAX;

        template<typename T>
        void CalcMinMaxSizes(TVecInt& colWidths, TVecInt& rowHeights, int def);
        friend class TPlot;
    };
    template<typename T>
    void TLayoutGrid::CalcMinMaxSizes(TVecInt &colWidths, TVecInt &rowHeights, int def)
    {
        std::vector<TStateInt> heights(CountRow());
        std::vector<TStateInt> widths(CountCol());

        T pred;
        for(size_t i = 0; i < heights.size(); i++)
            for(size_t j = 0; j < widths.size(); j++)
            {
                const auto& cell = elements[i][j];
                if(cell == nullptr) continue;
                if(cell->RealVisibility() == false)
                {
                    //heights[i] = TStateInt();
                    //widths[j] = TStateInt();
                    continue;
                }
                const auto& s = cell->MinOrMaxSize(def == 0);
                if(pred(s.height(), heights[i]) && s.height().IsNull() == false)
                    heights[i] = s.height();
                if(pred(s.width(), widths[j]) && s.width().IsNull() == false)
                    widths[j] = s.width();
            }
        colWidths.resize(widths.size());
        rowHeights.resize(heights.size());
        for(auto i = 0; i < widths.size(); i++)
            colWidths[i] = widths[i].get();
        for(auto i = 0; i < heights.size(); i++)
            rowHeights[i] = heights[i].get();
    }


    class TLayoutStack : public TLayoutGrid{
    public:
        double Stretch(size_t index);
        void SetStretch(size_t index, double value);
    protected:
        TLayoutStack(TRawPlot plt, TOrientation orientation, const TString& layerName = TString());
        friend class TPlot;
    };

    class TAxisRect;
    CLASS_PTRS(AxisRect)

    enum TAlignSet{ asLower, asCenter, asUpper};

    using TExtPaint = sigslot::signal<const TUPtrPainter &>;

    class TAxis : public TLayoutElement{
    public:
        double CoordToPixel(double value);
        double PixelToCoord(double value);

        void CalcSizes() override;

        inline TAxisType TypeAxis() const { return type; }
        inline TScaleType ScaleType() const { return scaleType; }
        inline TString Label() const { return label; }
        inline const TRange& Range() const { return range; }
        inline double Lower() const { return range.lower; }
        inline double Upper() const { return range.upper; }
        double Step() const;
        inline bool IsRangeReversed() const { return isRangeReversed; }//если true то right->left or bottom->top
        inline bool IsTicks() const { return isTicks; }
        inline bool IsSubTicks() const { return isSubTicks; }
        inline int SubTickCount() const { return subTickCount; }
        inline bool IsTickLabels() const { return isTickLabels; }
        inline TColor ColorAxis() const { return penAxis.color; }
        inline TOrientation Orientation() const { return orient; }
        inline int PixelOrientation() const { return isRangeReversed != (orient == orVert) ? -1 : 1; }
        inline bool IsFixedPix() const { return isFixedPix; }
        inline bool IsRoundTicks() const { return isRoundTicks; }
        inline bool IsRoundRange() const { return isRoundRange; }
        inline bool IsUseBuffer() const { return isUseBuffer; }
        inline TAlignText AlignText() const { return alignText; }

        inline TPtrAxisRect AxisRect() const { return axisRect.lock(); }

        void SetScaleType(TScaleType value);
        void SetLabel(const TString& value);
        void SetStep(double value);
        void SetIsRangeReversed(bool value);
        void SetIsTicks(bool value);
        void SetIsTickLabels(bool value);
        void SetIsSubTicks(bool value);
        void SetColorAxis(TColor value);
        void SetTickLen(int value);
        void SetSubTickLen(int value);
        void SetSubTickCount(int value);
        void SetIsFixedPix(bool value);
        void SetIsRoundTicks(bool value);
        void SetIsRoundRange(bool value);
        void SetIsUseBuffer(bool value);
        void SetAlignText(TAlignText value);

        void SetRange(const TRange& value, bool andStep = false);
        void SetRange(double lower, double upper);
        void SetRangeLower(double value);
        void SetRangeUpper(double value);
        void SetRange(double lower, TAlignSet align);
        void SetRangeSize(double value);
        void ScaleRange(double factor, double center);

        const TFont& FontLabel() const;
        void SetFontLabel(const TFont& value);

        const TFont& FontTicks() const;
        void SetFontTicks(const TFont& value);

        const TVecDouble& TickPositions() const { return tickPositions; }

        inline TAxisType LabelPos() const { return labelPos; }
        inline void SetLabelPos(TAxisType value) { labelPos = value; }

        const TVecDouble& SubTickPositions() const { return subTickPositions; }

        bool Contains(const TPointF& value) const;

        inline bool IsSelected() const { return isSelected; }
        bool SetIsSelected(bool value);

        inline bool IsSelectable() const { return isSelectable; }
        inline void SetIsSelectable(bool value) { isSelectable = value; }

        static TOrientation Orientation(TAxisType value) { return (value == atLeft || value == atRight) ? orVert : orHorz; }

        double SelectTest(const TPointF& pos, bool onlySelectable) override;

        void SaveDrag() { bufferDrag = range; }
        void SetDrag(const TPoint& startPos, const TPoint& curPos);

        bool IsDraggable() const { return isDraggable; }
        void SetIsDraggable(bool value) { isDraggable = value; }

        bool IsScalable() const { return isScalable; }
        void SetIsScalable(bool value) { isScalable = value; }

        static double FixPix() { static double fixPix = 60; return fixPix; }
        double CalcCountFixed() const;

        inline size_t CountAfterPoint() const { return countAfterPoint; }
        inline void SetCountAfterPoint(size_t value) { countAfterPoint = value; }

        inline TPtrPaintBuffer AxisBuffer() { return axisBuffer; }
        inline void Invalidate() { if(axisBuffer) axisBuffer->SetInvalidated(true); }

        TExtPaint OnExtPaint; // событие после отрисовки шкалы
    protected:
        TAxis(TAxisType typeAxis, TRawPlot plt);
        friend class TAxisRect;
        friend class TPlot;

        bool isSelected = false;
        bool isSelectable = true;
        TAxisType type;
        TAxisType labelPos = atCount;
        TOrientation orient;
        TScaleType scaleType = stLinear;
        TString label;
        TRange range{0, 5};
        double step = 1.;
        bool isFixedPix = false;
        bool isRangeReversed = false;
        bool isTicks = true;
        bool isTickLabels = true;
        bool isSubTicks = true;
        bool isRoundTicks = false;
        bool isRoundRange = false;
        TPen penAxis;
        TPen selPenAxis{TConsts::BlueColor(), 2};
        TFont fontLabel;
        TFont fontTicks;
        TWPtrAxisRect axisRect;
        TPtrPaintBuffer axisBuffer;
        bool isUseBuffer = true;
        bool isDraggable = true;
        bool isScalable = true;
        TAlignText alignText = TAlignText::Center;
        void Draw(const TUPtrPainter& painter) override;
        void DrawAxis(const TUPtrPainter & painter);

        int tickLen = 4;
        int subTickLen = 2;
        int tickLabelOffset = 1;
        int labelOffset = 5;
        int tickLabelHeight = 8;

        TVecDouble tickVals;
        TVecDouble subTickVals;
        TVecString tickLabels;
        TVecDouble tickPositions;
        TVecDouble subTickPositions;

        size_t subTickCount = 3;
        size_t countAfterPoint = 2;

        virtual void CalculateTicker(TVecDouble& ticks, TVecDouble& subTicks, TVecString& labels);
        virtual TVecDouble CalcTicks(double tickStep);
        virtual TVecDouble CalcSubTicks(const TVecDouble& ticks);
        virtual TVecString CalcLabels(const TVecDouble& ticks);
        virtual void TrimTicks(TVecDouble& value, const TRange& range);

        void InitTickVectors();
        void CalcTickVectors();

        bool SelectEvent(TMouseInfo& info, bool additive) override;
        bool DeselectEvent() override;

        TRange bufferDrag;

    };

    class TAxisRect : public TLayoutGrid{
    public:
        void SetAxisRectGroup(TAxisType type, const TPtrMarginGroup& value);

        const TPtrAxis& Axis(TAxisType type, size_t index = 0);
        const TPtrAxis& AddAxis(TAxisType type);
        void RemoveAxis(const TPtrAxis& value);
        size_t CountAxis(TAxisType type) const;

        void MousePress(TMouseInfo& info) override;
        void MouseMove(TMouseInfo& info, const TPoint& startPos) override;
        void MouseUp( TMouseInfo& info, const TPoint& startPos) override;
        void MouseWheel(TMouseInfoWheel& info) override;

        void ClearDragAxes();
        void ClearZoomAxes();
        void AddDragAxes(const TPtrAxis& horz, const TPtrAxis& vert);
        void AddZoomAxes(const TPtrAxis& horz, const TPtrAxis& vert);
        TRect CentralRect() const;
        const TUPtrGridAxes& Grid() const { return grid; }

        TPtrLayoutGrid AxisLay(TAxisType type) { return axisLay[type]; }

        bool IsHorzResizable() const;
        void SetIsHorzResizable(bool value);
        bool IsVertResizable() const;
        void SetIsVertResizable(bool value);
    protected:
        TAxisRect(TRawPlot plt, bool isDefault = true);
        void SetThisWeak(TWPtrLayoutElement value) override;
        bool isDefaultInit;
        TPtrLayoutGrid axisLay[atCount];
        TVecAxises axes[atCount];
        TUPtrGridAxes grid;

        bool isDragging = false;
        bool isHorzResizable = true;
        bool isVertResizable = false;
        int dragSize = 0;
        TVecWAxises vertDragAxes;
        TVecWAxises horzDragAxes;
        TVecWAxises vertZoomAxes;
        TVecWAxises horzZoomAxes;
        friend class TPlot;
    };

    enum TCursorType {ctDefault, ctHDrag, ctVDrag};

    class TItemTracer;
    using TRawItemTracer = TItemTracer*;

    class TInitRef{
        bool value;
        TPlot* plot;

    public:
        TInitRef(const TInitRef&) = delete;
        void operator = (const TInitRef&) = delete;

        TInitRef(TPlot* p, bool v);
        TInitRef(TInitRef&& oth);
        ~TInitRef();
    };


    class TPlot {
    public:
        explicit TPlot(bool isDefault = true);
        virtual ~TPlot();

        virtual void SetViewport(const TRect& value);
        inline TRect Viewport() const { return viewport; }
        inline TPtrLayoutGrid PlotLayout() const { return plotLayout; }
        inline TPtrLayoutGrid MainLayout() const { return mainLayout; }
        virtual TRect KeyClipRect() const { return TRect(); }

        virtual void PlotPaint();
        void PlotPaint(const TUPtrPainter& painter);
        virtual void Replot();

        virtual void SetCursor(TCursorType value) = 0;

        void SetInitState(bool value);
        TInitRef InitRef(bool before, bool after);

        size_t CountLayers() const;                                 //количество слоев
        const TPtrLayer& Layer(size_t index) const;                //получить слой по индексу
        const TPtrLayer& Layer(const TString& nameLayer) const;    //найти слой по имени
        const TPtrLayer& CurrentLayer() const;                     //текущий слой по умолчанию
        const TPtrLayer& AddLayer(const TString& nameLayer);       //добавить слой с именем
        void MoveLayerAfter(const TString& moveLayer, const TString& after);//перемещает слой после слоя after

        template<typename T = TLayoutElement, typename... TArgs>
            std::shared_ptr<T> CreateLayout(TArgs&&... args);

        template<typename T, typename... TArgs>
            std::shared_ptr<T> AddPlottable(TArgs&&... args);

        const TPtrPlottable& AddPlottable(const TPtrPlottable& value);
        bool DelPlottable(const TPtrPlottable& value);
        const TPtrPlottable& Plottable(size_t index) const;
        size_t CountPlottables() const;
        TPtrPlottable FindPlottable(const TString& value);

        const TPtrAxisRect& AddAxisRect(bool isDefault = true);
        const TPtrAxisRect& AddAxisRect(const TPtrAxisRect& value);
        void DelAxisRect(const TPtrAxisRect& value);
        const TPtrAxisRect& AxisRect(size_t index) const;
        size_t CountAxisRect() const;

        const TPtrTitle& Title() const;
        const TPtrTitle& AddTitle(const TString& text = TString());
        void DeleteTitle();

        TPtrLegend CreateLegend();
        const TPtrLegend& AddLegend();
        const TPtrLegend& Legend() const;
        void DeleteLegend();

        inline double SelectionTolerance() const { return selectionTolerance; };
        inline TPointF TolerancePoint() const { return TPointF(selectionTolerance, selectionTolerance); }

        Plot::TRawItemTracer Tracer() const { return tracer.get(); }
        void SetTracer(TRawItemTracer value);

        bool ResetSelected(TRawLayerable clicked = nullptr);

        const TUPtrSelectRect& SelectRect() const;

        virtual TPtrPaintBuffer CreatePaintBuffer(const Plot::TSize &sizeBuffer) = 0;

        //функция вызывает срабатывание события перерисовать от базовой библиотеки немедлено
        virtual void NativePaintNow() = 0;

        //функция вызывает событие перерисовки позже в подходящий момент
        virtual void NativePaintLater() = 0;

        //вызывает нативную установку размеров из базовой библиотеки
        virtual void NativeResize(const TSize& newSize) = 0;

        //устанавливает минимальные размеры для базовой библиотеки
        virtual void NativeSetMinSize(const TSize& newSize) = 0;

        virtual void UpdateSize(){ NativeResize(plotLayout->MinSize()); };

        sigslot::signal<const TMouseInfo&> OnMousePress;
        sigslot::signal<const TMouseInfo&> OnMouseMove;
        sigslot::signal<const TMouseInfo&> OnMouseUp;
        sigslot::signal<const TMouseInfo&> OnMouseDblClick;
        sigslot::signal<bool> OnSelectionChanged;
        sigslot::signal<> OnUserChanged;
        sigslot::signal<> OnDestroy;

        inline TPoint MousePressPos() const { return mousePressPos; }
        inline TPoint MouseUpPos() const { return mouseUpPos; }

        inline const TPtrAxis& LeftAxis(size_t axisRect = 0, size_t index = 0) const   { return AxisRect(axisRect)->Axis(atLeft, index); }
        inline const TPtrAxis& RightAxis(size_t axisRect = 0, size_t index = 0) const  { return AxisRect(axisRect)->Axis(atRight, index); }
        inline const TPtrAxis& TopAxis(size_t axisRect = 0, size_t index = 0) const    { return AxisRect(axisRect)->Axis(atTop, index); }
        inline const TPtrAxis& BottomAxis(size_t axisRect = 0, size_t index = 0) const { return AxisRect(axisRect)->Axis(atBottom, index); }

        inline bool SavePdf(const TString& path) { TSize s = CalcSaveSize(); return SavePdf(path, s.width(), s.height()); }
        inline bool SavePng(const TString& path) { TSize s = CalcSaveSize(); return SavePng(path, s.width(), s.height()); }

        virtual TSize CalcSaveSize() const { return TSize(); }
        virtual bool SavePdf(const TString& path, int newWidth, int newHeight){ return false; };
        virtual bool SavePng(const TString& path, int newWidth, int newHeight){ return false; };
        virtual TPoint Dpi() const { return TPoint(); }
    protected:
        int isInitState = 0;
        TRect viewport;
        TPtrLayoutGrid plotLayout;
        TPtrLayoutGrid mainLayout;
        std::vector<TPtrLayer> layers;
        std::vector<TPtrPaintBuffer> buffers;
        std::unique_ptr<TItemTracer> tracer;
        bool replotting = false;
        TColor background = TConsts::WhiteColor();
        TPtrLayer currentLayer;
        TUPtrSelectRect selectRect;

        std::vector<TPtrPlottable> plottables;
        std::vector<TPtrAxisRect> axisRects;
        TPtrTitle title;
        TPtrLegend legend;

        TPtrAxis CreatePlotAxis(TAxisType typeAxis);//создает шкалу принадлежащую только plot

        void Draw(const TUPtrPainter& painter);

        void InitPaintBuffers();
        virtual TUPtrPainter CreatePainter() = 0;
        void IncrIndexCheck(int& index)
        {
            ++index;
            if(index >= int(buffers.size()))
                buffers.emplace_back(CreatePaintBuffer(TSize()));
        }
        bool HasInvalidateBuffers();

        //for interraction
        bool mouseHasMoved = false;
        TPoint mousePressPos;
        TPoint mouseUpPos;
        TPointer<TLayerable> signalLayerable;
        double selectionTolerance = 8.;

        virtual void MousePress(TMouseInfo& info);
        virtual void MouseMove(TMouseInfo& info);
        virtual void MouseUp( TMouseInfo& info);
        virtual void MouseDblClick( TMouseInfo& info);
        virtual void MouseWheel(TMouseInfoWheel& info);

        TVecLayerable FindLayerableList(const TPointF& pos, bool onlySelectable);
        TRawLayerable FindLayerable(const TPointF& pos, bool onlySelectable);
        void MouseSelect(TMouseInfo& info);
        virtual TSize CheckSize(const TSize& newSize){ return newSize; }

        void SetSelectRect();
        void ResetSelectRect();
    private:
        friend class TLayer;
        friend class TLayerable;
    };

    template<typename T, typename... TArgs>
    std::shared_ptr<T> TPlot::CreateLayout(TArgs&&... args)
    {
        auto res = std::shared_ptr<T>(new T(this, std::forward<TArgs>(args)...));
        res->SetThisWeak(res);
        return res;
    }

    template<typename T, typename... TArgs>
     std::shared_ptr<T> TPlot::AddPlottable(TArgs&&... args)
    {
        auto res = std::shared_ptr<T>(new T(std::forward<TArgs>(args)...));
        AddPlottable(res);
        return res;
    }

    template<typename T>
    bool TLayerable::SelectionChanged(T &value, const T &newValue)
    {
        if(value == newValue) return false;

        value = newValue;
        OnSelectionChanged(value);
        if(plot) plot->OnSelectionChanged(value);
        return true;
    }

    class TTitle : public TLayoutElement{
    public:
        inline const TString& Text(size_t index = 0) const { return text[index]; };
        void SetText(const TString& value);
        void SetText(const TVecString& values);
        void AddText(const TString& value);
        size_t CountText() const;

    protected:
        TTitle(TRawPlot plt, const TString& title = TString());
        friend TPlot;
        virtual void Draw(const TUPtrPainter& painter);
        std::vector<TString> text;
    };

    class TLegendItem : public TLayoutElement{
    public:
        inline TRawLegend Legend() const { return legend; }

        inline bool IsSelected() const { return isSelected; }
        void SetIsSelected(bool value);

        inline bool IsSelectable() const { return isSelectable; }
        void SetIsSelectable(bool value);

        double SelectTest(const TPointF& pos, bool onlySelectable) override;
    protected:
        TLegendItem(TRawLegend parent);
        TRawLegend legend;
        bool isSelected = false;    //выделен ли элемент легенды
        bool isSelectable = true;   //можно ли выделять элемент

        bool SelectEvent(TMouseInfo& info, bool additive) override;
        bool DeselectEvent() override;
        TRect ClipRect() const override;
    };

    class TLegendItemPlottable : public TLegendItem{
    public:
        inline TPtrPlottable Plottable() const { return plottable.lock(); }

    protected:
        TLegendItemPlottable(TRawLegend legend, const TPtrPlottable& value);
        friend TLegend;
        TWPtrPlottable plottable;
        void Draw(const TUPtrPainter& painter) override;
    };

    class TLegend : public TLayoutGrid{
    public:
        inline TSize IconSize() const;
        void SetIconSize(const TSize& value);

        inline int TextPadding() const;
        void SetTextPadding(int value);

        inline const TBrush& Brush() const { return brush; }
        void SetBrush(const TBrush& value);

        inline const TPen& BorderPen() const { return borderPen; }
        void SetBorderPen(const TPen& value);

        inline const TPen& IconBorderPen() const { return iconBorderPen; }
        void SetIconBorderPen(const TPen& value);

        size_t CountItems() const;
        const TPtrLegendItem& Item(size_t index) const;
        void AddItem(const TPtrLegendItem& value);
        void DelItem(size_t index);
        template<typename T, typename... TArgs>
        std::shared_ptr<T> AddItem(TArgs&&... args)
        {
            auto res = std::shared_ptr<T>(new T(this, std::forward<TArgs>(args)...));
            AddItem(res);
            return res;
        }
    protected:
        TLegend(TRawPlot plt);
        friend TPlot;
        TSize iconSize{12, 12};
        int textPadding = 3;

        TBrush brush{TConsts::WhiteColor()};
        TPen borderPen{psNone};
        TPen iconBorderPen{psNone};

        std::vector<TPtrLegendItem> items;

        void Draw(const TUPtrPainter& painter) override;

    };

    enum class TTypeRuler{ Meter, Inch};

    class TRuler{
    public:
        TRuler(size_t dpi, size_t size = 22, size_t length = 0);

        inline bool IsVisible() const { return isVisible; }
        void SetIsVisible(bool value) { isVisible = value;}

        inline TTypeRuler TypeRuler() const { return typeRuler; }
        void SetTypeRuler(TTypeRuler value);

        inline size_t Length() const { return lengthRuler; }
        void SetLength(size_t value);

        inline size_t Size() const { return isVisible ? sizeRuler : 0; }
        void SetSize(size_t value);

        void Draw(const TUPtrPainter& painter);     //нарисовать рулетку в зависимости от настроек

        static double PixToSm(size_t dpi, int pix);
        static int SmToPix(size_t dpi, double sm);
    private:
        bool isVisible = false;
        TTypeRuler typeRuler = TTypeRuler::Meter;//TTypeRuler::Inch;//
        TColor background = TConsts::WhiteColor();
        TColor tickColor = TConsts::BlackColor();
        TPointF offsetRuler{0, 0};
        size_t dpiPainter;
        size_t sizeRuler;                      //размер рулетки в пикселях (для горизонтальной это height)
        size_t lengthRuler;                    //длина рулетки в пикселях (для горизонтальной это width)
        int mainLineOffset = 2;                //смещение основной линии рулетки от низа

        void DrawSm(const TUPtrPainter& painter);   //нарисовать рулетку в сантиметрах
        void DrawInch(const TUPtrPainter& painter); //нарисовать рулетку в дюймах
    };
}

#endif //NEO_PLOT_H
