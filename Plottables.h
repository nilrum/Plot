//
// Created by user on 19.05.2020.
//

#ifndef TESTQT_PLOTTABLES_H
#define TESTQT_PLOTTABLES_H

#include "BaseClasses.h"
#include <algorithm>
#include <functional>

namespace Plot {

    class TAxis;
    CLASS_PTRS(Axis)

    using TVecAxises = std::vector<TPtrAxis>;
    using TVecWAxises = std::vector<TWPtrAxis>;

    class TAbstractPlottable;
    CLASS_PTRS_TYPE(Plottable, TAbstractPlottable)

    double DistanceSquaredToLine(const TPointF& pos, const TPointF& start, const TPointF& stop);

    enum TScatterShape {
        ssNone,
        ssDot,
        ssCross,
        ssPlus,
        ssCircle,
        ssDisc,
        ssSquare,
        ssDiamond,
        ssStar,
        ssTriangle,
        ssTriangleInverted,
        ssCrossSquare,
        ssPlusSquare,
        ssCrossCircle,
        ssPlusCircle,
        ssPeace
    };

    enum TSelectionType { stNone, stWhole, stSingleData, stDataRange, stMultipleDataRanges};

    class TDataRange {
    public:
        TDataRange() : begin(0), end(0){};
        TDataRange(int b, int e) : begin(b), end(e){};

        bool operator==(const TDataRange &oth) const { return begin == oth.begin && end == oth.end; }
        bool operator!=(const TDataRange &oth) const { return !(*this == oth); }

        inline int Begin() const { return begin; }
        inline int End() const { return end; }
        inline int Size() const { return end - begin; }
        inline int Length() const { return Size(); }

        void SetBegin(int value) { begin = value; }
        void SetEnd(int value) { end = value; }

        bool IsValid() const { return end >= begin && begin >= 0; }
        bool Empty() const { return Length() == 0; }


        TDataRange Bounded(const TDataRange &other) const;
        TDataRange Expanded(const TDataRange &other) const;
        TDataRange Intersection(const TDataRange &other) const;
        TDataRange Adjusted(int changeBegin, int changeEnd) const { return TDataRange(begin + changeBegin, end + changeEnd); }
        void Adjust(int changeBegin, int changeEnd) { begin += changeBegin; end += changeEnd; }
        /*bool intersects(const TDataRange &other) const;*/
        bool Contains(const TDataRange &other) const;
    private:
        int begin;
        int end;
    };

    using TVecDataRange = std::vector<TDataRange>;

    template<typename T>
    T BoundValue(const T& minVal, const T& val, const T& maxVal)
    {
        return std::max(minVal, std::min(val, maxVal));
    }

    enum TSigDomain { sdBoth, sdNegative, sdPositive};

    template<typename T>
    inline bool LessThanSortKey(const T& a, const T& b) { return a.SortKey() < b.SortKey(); }
    template<typename T>
    class TDataContainer {
    public:
        using TConstIterator = typename std::vector<T>::const_iterator;
        using TIterator = typename std::vector<T>::iterator;
        using TVecData = std::vector<T>;
        void Set(const TDataContainer<T>& value)
        {
            Clear();
            Add(value);
        }

        void Set(const std::vector<T>& values, bool isSorted = false)
        {
            data = values;
            preallocSize = 0;
            preallocIteration = 0;
            if(isSorted == false)
                Sort();
        }

        void Add(const TDataContainer<T>& value)
        {
            if(value.IsEmpty()) return;

            int countAdd = value.Size();
            int oldSize = Size();
            if(oldSize > 0 && LessThanSortKey(*ConstBegin(), *(value.ConstEnd() - 1)) == false)
            {
                if(preallocSize < countAdd)
                    PreallocateGrow(countAdd);
                preallocSize -= countAdd;
                std::copy(value.ConstBegin(), value.ConstEnd(), Begin());
            }
            else
            {
                data.resize(data.size() + countAdd);
                std::copy(value.ConstBegin(), value.ConstEnd(), End() - countAdd);
                if(oldSize > 0 && LessThanSortKey(*(value.ConstEnd() - countAdd - 1), *(ConstEnd() - countAdd)) == false)
                    std::inplace_merge(Begin(), End() - countAdd, End(), LessThanSortKey<T>);
            }
        }

        void Add(const T& value)
        {
            if(IsEmpty() || LessThanSortKey(value, *(ConstEnd() - 1)) == false)
                data.emplace_back(value);
            else if(LessThanSortKey(value, *ConstBegin()))
            {
                if(preallocSize < 1)
                    PreallocateGrow(1);
                --preallocSize;
                *Begin() = data;
            }
            else
            {
                TConstIterator it = std::lower_bound(Begin(), End(), value, LessThanSortKey<T>);
                data.insert(it, value);
            }
        }

        void Add(const std::vector<T>& values, bool isSorted = false)
        {
            if(values.empty()) return;

            if(IsEmpty())
            {
                Set(values, isSorted);
                return;
            }
            int countAdd = values.size();
            int oldCount = Size();
            if(isSorted && LessThanSortKey(*ConstBegin(), values.back()) == false)
            {
                if(preallocSize < countAdd)
                    PreallocateGrow(countAdd);
                preallocSize -= countAdd;
                std::copy(values.begin(), values.end(), Begin());
            }
            else
            {
                data.resize(data.size() + countAdd);
                std::copy(values.begin(), values.end(), End() - countAdd);
                if(isSorted == false)
                    std::sort(End() - countAdd, End(), LessThanSortKey<T>);
                if(LessThanSortKey(*(ConstEnd() - countAdd - 1), *(ConstEnd() - countAdd)) == false)
                    std::inplace_merge(Begin(), End() - countAdd, End(), LessThanSortKey<T>);
            }
        }

        void Clear()
        {
            data.clear();
            preallocSize = 0;
            preallocIteration = 0;
        }

        inline int Size() const { return data.size() - preallocSize; }
        inline bool IsEmpty() const { return Size() == 0; }

        inline TConstIterator ConstBegin() const { return data.begin() + preallocSize; }
        inline TConstIterator ConstEnd() const { return data.end(); }

        inline TIterator Begin() { return data.begin() + preallocSize; }
        inline TIterator End() { return data.end(); }

        inline TIterator begin() { return Begin(); }
        inline TIterator end() { return End(); }

        inline void Delete(size_t start, size_t stop) { data.erase(Begin() + start, Begin() + stop); }

        void Sort()
        {
            std::sort(Begin(), End(), LessThanSortKey<T>);
        }
        TDataRange DataRange() const { return TDataRange(0, Size()); }

        TIterator ConstTo(TConstIterator value) { return Begin() + (value - Begin());}

        TConstIterator FindBegin(double sortKey, bool ExpandedRange = true) const
        {
            if(IsEmpty()) return ConstEnd();
            TConstIterator it = std::lower_bound(ConstBegin(), ConstEnd(), T::FromSortKey(sortKey), LessThanSortKey<T>);
            if(ExpandedRange && it != ConstBegin())
                --it;
            return it;
        }

        TConstIterator FindEnd(double sortKey, bool ExpandedRange = true) const
        {
            if(IsEmpty()) return ConstEnd();
            TConstIterator it = std::upper_bound(ConstBegin(), ConstEnd(), T::FromSortKey(sortKey), LessThanSortKey<T>);
            if(ExpandedRange && it != ConstEnd())
            ++it;
            return it;
        }
        void LimitIterators(TConstIterator& begin, TConstIterator& end, const TDataRange& dataRange)
        {
            TDataRange iteratorRange(begin - ConstBegin(), end - ConstBegin());
            iteratorRange = iteratorRange.Bounded(dataRange.Bounded(DataRange()));
            begin = ConstBegin() + iteratorRange.Begin();
            end = ConstBegin() + iteratorRange.End();
        }

        TRange KeyRange(bool* founded = nullptr, TSigDomain sig = sdBoth)
        {
            if(IsEmpty())
            {
                if(founded) *founded = false;
                return TRange();
            }
            bool haveLower = false;
            bool haveUpper = false;
            TRange res;
            if(sig == sdBoth)
            {
                if (T::SortKeyIsMainKey())
                {
                    for (auto it = ConstBegin(); it != ConstEnd(); it++)
                        if (isnan(it->MainValue()) == false)
                        {
                            res.lower = it->MainKey();
                            haveLower = true;
                            break;
                        }
                    for (auto it = ConstEnd() - 1; it != ConstBegin(); it--)
                        if (isnan(it->MainValue()) == false)
                        {
                            res.upper = it->MainKey();
                            haveUpper = true;
                            break;
                        }
                }
                else
                {
                    double cur = 0;
                    for(auto it = ConstBegin(); it != ConstEnd(); it++)
                    {
                        if (isnan(it->MainValue()))
                        {
                            cur = it->MainKey();
                            if ((cur < res.lower || haveLower == false) && cur < 0)
                            {
                                res.lower = cur;
                                haveLower = true;
                            }
                            if ((cur > res.upper || haveUpper == false) && cur < 0)
                            {
                                res.upper = cur;
                                haveUpper = true;
                            }
                        }
                    }
                }
            }
            else
            {
                double cur = 0;
                if(sig == sdNegative)
                {
                    for(auto it = ConstBegin(); it != ConstEnd(); it++)
                    {
                        if(isnan(it->MainValue()))
                        {
                            cur = it->MainKey();
                            if((cur < res.lower || haveLower == false) && cur < 0)
                            {
                                res.lower = cur;
                                haveLower = true;
                            }
                            if((cur > res.upper || haveUpper == false) && cur < 0)
                            {
                                res.upper = cur;
                                haveUpper = true;
                            }
                        }
                    }
                }
                else
                {
                    for(auto it = ConstBegin(); it != ConstEnd(); it++)
                    {
                        if(isnan(it->MainValue()))
                        {
                            cur = it->MainKey();
                            if((cur < res.lower || haveLower == false) && cur > 0)
                            {
                                res.lower = cur;
                                haveLower = true;
                            }
                            if((cur > res.upper || haveUpper == false) && cur > 0)
                            {
                                res.upper = cur;
                                haveUpper = true;
                            }
                        }
                    }
                }
            }
            if(founded) *founded = haveLower && haveUpper;
            return res;
        }

        TRange ValRange(bool* founded = nullptr, TSigDomain sig = sdBoth, const TRange& inRange = TRange())
        {
            if(IsEmpty())
            {
                if(founded) *founded = false;
                return TRange();
            }
            bool isInRange = TRange::ValidRange(inRange);
            auto begin = ConstBegin();
            auto end = ConstEnd();
            TRange res;
            TRange cur;

            bool haveLower = false;
            bool haveUpper = false;

            if(T::SortKeyIsMainKey() && isInRange)
            {
                begin = FindBegin(inRange.lower);
                end = FindEnd(inRange.upper);
            }
            if(sig == sdBoth)
            {
                for(auto it = begin; it != end; it++)
                {
                    if (isInRange && (it->MainKey() < inRange.lower || it->MainKey() > inRange.upper))
                        continue;
                    cur = it->ValueRange();
                    if((cur.lower < res.lower || haveLower == false) && isnan(cur.lower) == false)
                    {
                        res.lower = cur.lower;
                        haveLower = true;
                    }
                    if((cur.upper > res.upper || haveUpper == false) && isnan(cur.upper) == false)
                    {
                        res.upper = cur.upper;
                        haveUpper = true;
                    }
                }
            }
            else
            {
                if(sig == sdNegative)
                {
                    for(auto it = begin; it != end; it++)
                    {
                        if (isInRange && (it->MainKey() < inRange.lower || it->MainKey() > inRange.upper))
                            continue;
                        cur = it->ValueRange();
                        if((cur.lower < res.lower || haveLower == false) && isnan(cur.lower) == false && cur.lower < 0)
                        {
                            res.lower = cur.lower;
                            haveLower = true;
                        }
                        if((cur.upper > res.upper || haveUpper == false) && isnan(cur.upper) == false  && cur.upper < 0)
                        {
                            res.upper = cur.upper;
                            haveUpper = true;
                        }
                    }
                }
                else
                {
                    for(auto it = begin; it != end; it++)
                    {
                        if (isInRange && (it->MainKey() < inRange.lower || it->MainKey() > inRange.upper))
                            continue;
                        cur = it->ValueRange();
                        if((cur.lower < res.lower || haveLower == false) && isnan(cur.lower) == false && cur.lower > 0)
                        {
                            res.lower = cur.lower;
                            haveLower = true;
                        }
                        if((cur.upper > res.upper || haveUpper == false) && isnan(cur.upper) == false && cur.upper > 0)
                        {
                            res.upper = cur.upper;
                            haveUpper = true;
                        }
                    }
                }
            }
            if(founded) *founded = haveLower && haveUpper;
            return res;
        }
    protected:
        std::vector<T> data;
        int preallocSize = 0;
        int preallocIteration = 0;


        void PreallocateGrow(int value)
        {
            if(value <= preallocSize)
                return;
            int newPreallocSize = value;
            newPreallocSize += (1u << BoundValue(4, preallocIteration + 4, 15)) - 12;
            ++preallocIteration;

            int sizeDiff = newPreallocSize - preallocSize;
            data.resize(data.size() + sizeDiff);
            std::copy_backward(data.begin() + preallocSize, data.end() - sizeDiff, data.end());
            preallocSize = newPreallocSize;
        }
    };

    class TSelectionDecorator{
    public:
        const TPen& Pen() const { return pen; }
        TPen& Pen() { return pen; }
        void SetPen(const TPen& value){ pen = value; }
        void SetBrush(const TBrush& value){ brush = value; }
        void ApplyBrush(const TUPtrPainter & painter)
        {
            painter->SetBrush(brush);
        };
        void ApplyPen(const TUPtrPainter & painter)
        {
            painter->SetPen(pen);
        };
    private:
        TPen pen{TConsts::BlueColor(), 2};
        TBrush brush{bsNone};
    };

    using TRawSelectionDecorator = TSelectionDecorator*;

    class TDataSelection{
    public:
        TDataSelection() = default;
        TDataSelection(const TDataRange& range){ ranges.emplace_back(range); }

        bool operator==(const TDataSelection& other) const;
        bool operator!=(const TDataSelection& other) const { return !(*this == other); }
        TDataSelection &operator+=(const TDataSelection& other);
        TDataSelection &operator-=(const TDataSelection& other);
        TDataSelection &operator+=(const TDataRange& other);
        TDataSelection &operator-=(const TDataRange& other);
        operator bool() const { return !Empty(); }

        inline size_t CountDataRange() const { return ranges.size(); }
        inline const TVecDataRange& DataRanges() const { return ranges; }
        inline TVecDataRange& DataRanges() { return ranges; }
        inline bool Empty() const { return ranges.empty(); }
        void AddDataRange(const TDataRange& other, bool simplify = true);
        bool Contains(const TDataSelection& oth);
        void Clear(){ ranges.clear(); };
        void Simplify();
        void EnforceType(TSelectionType value);
        TDataSelection Inverse(const TDataRange& outerRange);

        TDataRange Span() const;
    private:
        TVecDataRange ranges;
        inline static bool LessThanDataRangeBegin(const TDataRange &a, const TDataRange &b) { return a.Begin() < b.Begin(); }
    };

    inline const TDataSelection operator+(const TDataSelection& a, const TDataSelection& b)
    {
        TDataSelection result(a);
        result += b;
        return result;
    }

    inline const TDataSelection operator-(const TDataSelection& a, const TDataSelection& b)
    {
        TDataSelection result(a);
        result -= b;
        return result;
    }

    class TScatterStyle{
    public:
        TScatterStyle(){};
        TScatterStyle(TScatterShape value, size_t size = 6);

        inline size_t SizeScatter() const { return sizeScat; }
        inline void SetSizeScatter(size_t value) { sizeScat = value; }

        inline TScatterShape ShapeScatter() const { return shapeScat; }
        inline void SetShapeScatter(TScatterShape value) { shapeScat = value; }

        bool IsNone() const { return shapeScat == ssNone; }
        void Draw(const TUPtrPainter& painter, const TVecPointF& values);
    protected:
        size_t sizeScat = 6;
        TScatterShape shapeScat = ssNone;
        TPen penScat = psNone;
        TBrush brushScat = bsNone;
    };

    class TAbstractPlottable;

    class TPlottableEditInterface{
    public:
        virtual void MousePress(TMouseInfo& info)                       { isEditing = plottable != nullptr; };
        virtual void MouseMove(TMouseInfo& info, const TPoint& startPos){ info.Ignore(); };
        virtual void MouseUp( TMouseInfo& info, const TPoint& startPos) { isEditing = false; };
        virtual void MouseWheel(TMouseInfoWheel& info)                  { info.Ignore(); };

        virtual void SetPlottable(TAbstractPlottable* value)            { plottable = value; }
    protected:
        bool isEditing = false;
        TAbstractPlottable* plottable = nullptr;
    };

    using TPtrPlottableEditInterface = std::shared_ptr<TPlottableEditInterface>;
    using TFunConvertTraser = std::function<TString(double)>;

    class TAbstractPlottable : public TLayerable {
    public:
        TAbstractPlottable(const TPtrAxis& key, const TPtrAxis& val);
        ~TAbstractPlottable();

        inline TString PlottableName() const { return plottableName; }
        inline void SetPlottableName(const TString& value) { plottableName = value; }

        Plot::TPen Pen() const;
        void SetPen(const Plot::TPen &value);
        void SetColorPen(const TColor& value);
        void SetWidthPen(size_t value);
        void SetStylePen(const TPenStyle& value);

        Plot::TBrush Brush() const;
        void SetBrush(const Plot::TBrush &value);

        Plot::TPointF CoordToPixel(double key, double val);
        void PixelToCoords(const TPointF& pos, double& key, double& val);
        double PixelToKey(const TPointF& pos);

        inline TPtrAxis KeyAxis() const { return keyAxis.lock(); }
        inline TPtrAxis ValAxis() const { return valAxis.lock(); }

        bool IsSelected() const { return selection.Empty() == false; }
        inline const TDataSelection& Selection() const { return selection;};
        inline TDataSelection& Selection() { return selection; };
        bool SetSelection(TDataSelection value);
        bool SetIsSelect(bool value);

        void SetSelectable(TSelectionType value) { selectable = value; }

        void SetEditInter(const TPtrPlottableEditInterface& value);

        const TString& TracerMask(bool isData = true) const { return isData ? tracerMask : tracerMaskNoData; }
        void SetTracerMask(const TString& value, bool isData = true) { if(isData) tracerMask = value; else tracerMaskNoData  = value; }

        virtual TPointF TracerValue(double key) const { return TPointF(NAN, NAN);};

        TFunConvertTraser ConverterTraserValue() const { return convertValue; }
        void SetConverterTracerValue(const TFunConvertTraser& value) { convertValue = value; }

        virtual TRange GetKeyRange(bool* founded, TSigDomain sig = sdBoth) const { return TRange(); };
        virtual TRange GetValRange(bool* founded, TSigDomain sig = sdBoth, const TRange& inRange = TRange()) const { return TRange(); };

        virtual void RescaleValueAxis(bool onlyErlange = false, bool inKeyRange = false);
        bool IsAntiAliasing() const override { return true; }

        virtual bool SelectEvent(TMouseInfo& info, bool additive) override;
        virtual bool DeselectEvent() override;

        TRawSelectionDecorator SelDecorator() const { return selDecorator.get(); }

        virtual void DrawLegendIcon(const TUPtrPainter &painter, const TRectF iconRect){};
    protected:
        TString plottableName;
        TWPtrAxis keyAxis;
        TWPtrAxis valAxis;
        Plot::TPen pen;
        Plot::TBrush brush{bsNone};
        bool isAdaptive = true;
        TSelectionType selectable = stWhole;
        TDataSelection selection;
        TString tracerMask;
        TString tracerMaskNoData;
        TFunConvertTraser convertValue;
        TPtrPlottableEditInterface editInter;
        std::unique_ptr<TSelectionDecorator> selDecorator = std::make_unique<TSelectionDecorator>();

        Plot::TRect ClipRect() const override;

        void DrawPolyLine(const TUPtrPainter &painter, TVecPointF lineData);

        TDataSelection bufferSelect;
    };

    template<typename T>
    class TAbstractPlottable1D : public TAbstractPlottable {
    public:
        TAbstractPlottable1D(const TPtrAxis& key, const TPtrAxis& val);
        virtual size_t CountData() const { return data->Size(); }
        using TPlottableData = TDataContainer<T>;
        using TPtrPlottableData = std::shared_ptr<TPlottableData>;
        const TPtrPlottableData& Data() const { return data; };

        virtual TRange GetKeyRange(bool* founded, TSigDomain sig = sdBoth) const;
        virtual TRange GetValRange(bool* founded, TSigDomain sig = sdBoth, const TRange& inRange = TRange()) const;
    protected:
        std::shared_ptr<TDataContainer<T>> data;
        void GetDataSegments(TVecDataRange &selSegments, TVecDataRange &unselSegments) const;
    };

    template<typename T>
    TAbstractPlottable1D<T>::TAbstractPlottable1D(const TPtrAxis& key, const TPtrAxis& val):
        TAbstractPlottable(key, val), data(std::make_shared<TDataContainer<T>>())
    {

    }

    template<typename T>
    void TAbstractPlottable1D<T>::GetDataSegments(Plot::TVecDataRange &selSegments, Plot::TVecDataRange &unselSegments) const
    {
        selSegments.clear();
        unselSegments.clear();
        if (selectable == stWhole)
        {
            if(IsSelected())
                selSegments.emplace_back(TDataRange(0, CountData()));
            else
                unselSegments.emplace_back(TDataRange(0, CountData()));
        }
        else
        {
            TDataSelection sel(selection);
            sel.Simplify();
            selSegments = sel.DataRanges();
            unselSegments = sel.Inverse(TDataRange(0, CountData())).DataRanges();
        }
    }

    template<typename T>
    TRange TAbstractPlottable1D<T>::GetKeyRange(bool *founded, TSigDomain sig) const
    {
        return data->KeyRange(founded, sig);
    }

    template<typename T>
    TRange TAbstractPlottable1D<T>::GetValRange(bool *founded, TSigDomain sig, const TRange &inRange) const
    {
        return data->ValRange(founded, sig, inRange);
    }


    struct TGraphData {
        double key, val;
        inline void Set(double k, double v){ key = k; val = v; }

        TGraphData() : key(0), val(0) {};
        TGraphData(double k, double v) : key(k), val(v) {}

        inline double SortKey() const { return key; }
        inline static TGraphData FromSortKey(double sortKey) { return TGraphData(sortKey, 0); }
        inline static bool SortKeyIsMainKey() { return true; }

        inline double MainKey() const { return key; }
        inline double MainValue() const { return val; }

        inline Plot::TRange ValueRange() const { return Plot::TRange(val, val); }
    };


    class TGraph;
    CLASS_PTRS(Graph);

    using TVecGraphData = std::vector<TGraphData>;
    using TPairDataRange = std::pair<TDataRange, TDataRange>;
    using TVecPairDataRange = std::vector<TPairDataRange>;
    using TOnValueDragged = sigslot::signal<int, double, double>;//index, key, value
    using TDataContainerGraph = TDataContainer<TGraphData>;

    enum TTypeGraph{tgNone, tgLine, tgColumn, tgCoupling};

    class TGraph : public TAbstractPlottable1D<TGraphData>{
    public:
        ~TGraph();
        virtual double SelectTest(const TPointF& pos, bool onlySelectable) override;

        TPtrGraph FillGraph() const { return fillGraph.lock(); }
        void SetFillGraph(const TPtrGraph& value){ fillGraph = value; }
        TTypeGraph TypeGraph() const { return typeGraph; }
        TOnValueDragged OnValueDragged;
        TVecPointF GetLines(const TDataRange& dataRange);

        inline TScatterStyle& Scatter() { return scatter; }

        double BaseFillPoint() const;
        void SetBaseFillPoint(double value);

        TBrush OtherBrush() const;
        void SetOtherBrush(const TBrush& value);

        void MousePress(TMouseInfo& info) override;
        void MouseMove(TMouseInfo& info, const TPoint& startPos) override;
        void MouseUp( TMouseInfo& info, const TPoint& startPos) override;

        TPointF TracerValue(double key) const;

        void DrawLegendIcon(const TUPtrPainter &painter, const TRectF iconRect) override;
    protected:
        TGraph(const TPtrAxis& key, const TPtrAxis& val, TTypeGraph type = tgLine);
        friend class TPlot;

        TTypeGraph typeGraph = tgLine;
        TScatterStyle scatter;
        double baseFillPoint = 0;
        TBrush otherBrush{bsNone};
        TWPtrGraph fillGraph;
        void Draw(const TUPtrPainter& painter) override;

        void GetVisibleDataBounds(TDataContainerGraph::TConstIterator &begin, TDataContainerGraph::TConstIterator &end,
                                  const TDataRange& dataRange);
        TVecGraphData GetOptimizedLineData(const TDataContainerGraph::TConstIterator &begin, const TDataContainerGraph::TConstIterator &end);
        TVecPointF DataToLines(const TVecGraphData& value);
        TVecPointF DataToColumn(const TVecGraphData& value);
        TVecPointF DataToCoupling(const TVecGraphData& value);

        TVecDataRange GetNonNanSegments(const TVecPointF& lineData, TOrientation orientation);
        void DrawFill(const TUPtrPainter &painter, const TVecPointF& lineData);
        TVecPointF GetFillPolygon(const TVecPointF& lineData, const TDataRange& segment);

        TPointF GetFillBasePoint(const TPointF &matchPoint, double offset = 0);
        TVecPairDataRange GetOverlappingSegments(const TVecDataRange& thisSegs, const TVecPointF& thisData, const TVecDataRange& otherSegs, const TVecPointF& otherData);

        TVecPointF ClipPolygon(const TVecPointF& lines, const TDataRange& r);

        bool SegmentIntersect(double thisLower, double thisUpper, double otherLower, double otherUpper, int &precedence);

        TVecPointF GetChannelFillPolygon(const TVecPointF &thisData, const TDataRange &thisRange, const TVecPointF &otherData,
                              const TDataRange &otherRange);
        double PointDistance(const TPointF& pos, TDataContainerGraph::TConstIterator& iter);

        bool CheckIsDraw() const;
        template<bool IsFill, typename TPtr, typename TFun>
        void DrawUnsel(const TUPtrPainter &painter, TVecDataRange& unselSegments, TPtr* ptr, const TFun& drawUnsel)
        {
            TVecPointF lines;
            for(auto& seg : unselSegments)
            {
                seg.Adjust(-1, 1);
                lines = GetLines(seg);

                if constexpr (IsFill)
                {
                    painter->SetPen(psNone);
                    DrawFill(painter, lines);
                }
                painter->SetPen(pen);
                (ptr->*drawUnsel)(painter, seg, lines);
                if(scatter.IsNone() == false)
                {
                    painter->SetPen(TPen(pen.color, 1));
                    scatter.Draw(painter, lines);
                }
            }
        }
        template<bool IsFill, typename TPtr, typename TFun>
        void DrawSel(const TUPtrPainter &painter, TVecDataRange& selSegments, TPtr* ptr, const TFun& drawSel)
        {
            TVecPointF lines;
            for(const auto& seg : selSegments)
            {
                lines = GetLines(seg);
                if constexpr (IsFill)
                {
                    painter->SetPen(psNone);
                    DrawFill(painter, lines);
                }
                if(selDecorator) selDecorator->ApplyPen(painter);
                (ptr->*drawSel)(painter, seg, lines);
                if(scatter.IsNone() == false)
                    scatter.Draw(painter, lines);
            }
        }

        void DrawLine(const TUPtrPainter &painter, const TDataRange &seg, const TVecPointF& lines)
        {
            painter->SetBrush(bsNone);
            DrawPolyLine(painter, lines);
        }
        void DrawColumn(const TUPtrPainter &painter, const TDataRange &seg, const TVecPointF& lines)
        {
            painter->SetBrush(brush);
            painter->DrawPolygon(lines);
        }

        template<bool IsFill, typename TPtr, typename TFun>
        void DrawTemplate(const TUPtrPainter &painter, TPtr* ptr, const TFun& drawUnsel, const TFun& drawSel)
        {
            if(CheckIsDraw() == false) return;
            TVecDataRange selSegments, unselSegments;
            GetDataSegments(selSegments, unselSegments);
            DrawUnsel<IsFill>(painter, unselSegments, ptr, drawUnsel);
            DrawSel<IsFill>(painter, selSegments, ptr, drawSel);
        }
    };

    class TCouplingPlottable : public TGraph{
    public:
        void MousePress(TMouseInfo& info) override;
        void MouseMove(TMouseInfo& info, const TPoint& startPos) override;
        void MouseUp( TMouseInfo& info, const TPoint& startPos) override;

        inline bool IsMaskColumn() const { return isMaskColumn; }
        void SetIsMaskColumn(bool value);

    protected:
        TCouplingPlottable(const TPtrAxis& key, const TPtrAxis& val);
        friend TPlot;

        int indDragging = -1;
        double valDragging = 0.;
        bool isMaskColumn = true;

        void Draw(const TUPtrPainter& painter) override;
        void DrawCoupling(const TUPtrPainter &painter, const TDataRange &seg, const TVecPointF& lines);
        void DrawCouplingSel(const TUPtrPainter &painter, const TDataRange &seg, const TVecPointF& lines);
    };

    CLASS_PTRS(CouplingPlottable)

    class TGridAxes : public TAbstractPlottable{
    public:
        TGridAxes(const TPtrAxis& key, const TPtrAxis& val);
        bool IsSubTick() const { return isSubTick; }
        void SetIsSubTick(bool value) { isSubTick = value; }

        TPen SubTickPen() const  { return penSubTick; }
        void SetSubTickPen(const TPen& value){ penSubTick = value; };
    protected:
        bool isSubTick = true;
        TPen penSubTick;
        virtual void Draw(const TUPtrPainter& painter);
        void DrawVert(const TUPtrPainter& painter, const TPtrAxis& axis, const TPtrAxis& twoAxis);
        void DrawHorz(const TUPtrPainter& painter, const TPtrAxis& axis, const TPtrAxis& twoAxis);
    };

    CLASS_PTRS(GridAxes)


    class TColorMapData{
    public:
        TColorMapData(int kSize, int vSize, const TRange& kRange, const TRange& vRange);
        ~TColorMapData() = default;
        TColorMapData(const TColorMapData& oth);
        TColorMapData& operator=(const TColorMapData& oth);

        inline int KeySize() const { return keySize; }
        inline int ValSize() const { return valSize; }
        inline const TRange& KeyRange() const { return keyRange; }
        inline const TRange& ValRange() const { return valRange; }
        inline const TRange& DataBounds() const { return dataBounds; }
        inline bool IsEmpty() const { return isEmpty; }

        inline const double* RawData() const { return data.data(); }
        inline const unsigned char* RawAlpha() const { return alpha.data(); }

        inline double Cell(int kIndex, int vIndex){ return data[vIndex * keySize + kIndex]; }
        inline unsigned char Alpha(int kIndex, int vIndex){ int index = vIndex * keySize + kIndex; if(index >= 0 && index < int(alpha.size())) return alpha[index]; return 255; }

        inline int KeyCell(double value) const { return int((value - keyRange.lower) / keyRange.Size() * (keySize - 1) + 0.5); }
        inline int ValCell(double value) const { return int((value - valRange.lower) / valRange.Size() * (valSize - 1) + 0.5); }
        inline int KeyCell(double value, bool check) const
        {
            int res = KeyCell(value);
            if(res < 0) return 0;
            if(res >= keySize) return keySize - 1;
            return res;
        }

        inline bool IsDataModifed() const { return isDataModifed; }
        inline void SetIsDataModifed(bool value) { isDataModifed = value; }

        void SetSize(int kSize, int vSize);
        void SetRange(const TRange& kRange, const TRange& vRange);
        void SetKeyRange(const TRange& value);
        void SetValRange(const TRange& value);
        void SetCell(int keyIndex, int valIndex, double value);
        void SetCellNoCheck(int keyIndex, int valIndex, double value);
        void SetAlpha(int keyIndex, int valIndex, unsigned char value);

        void Clear();
        void ClearAlpha();

        void Fill(double value);
        void FillAlpha(unsigned char value);

        void RecalculateDataBounds();
        void SetDataBounds(double lower, double upper) { dataBounds.Set(lower, upper); }
    private:
        int keySize = 0;
        int valSize = 0;
        TRange keyRange;
        TRange valRange;
        TRange dataBounds;
        bool isEmpty = true;
        TVecDouble data;
        TVecUChar alpha;
        bool isDataModifed = true;
        bool CreateAlpha(bool initOpaque = true);
    };


    enum TColorInterpolation {
         ciRGB  ///< Color channels red, green and blue are linearly interpolated
        ,ciHSV ///< Color channels hue, saturation and value are linearly interpolated (The hue is interpolated over the shortest angle distance)
    };
    enum TGradientPreset { gpGrayscale  ///< Continuous lightness from black to white (suited for non-biased data representation)
        ,gpHot       ///< Continuous lightness from black over firey colors to white (suited for non-biased data representation)
        ,gpCold      ///< Continuous lightness from black over icey colors to white (suited for non-biased data representation)
        ,gpNight     ///< Continuous lightness from black over weak blueish colors to white (suited for non-biased data representation)
        ,gpCandy     ///< Blue over pink to white
        ,gpGeography ///< Colors suitable to represent different elevations on geographical maps
        ,gpIon       ///< Half hue spectrum from black over purple to blue and finally green (creates banding illusion but allows more precise magnitude estimates)
        ,gpThermal   ///< Colors suitable for thermal imaging, ranging from dark blue over purple to orange, yellow and white
        ,gpPolar     ///< Colors suitable to emphasize polarity around the center, with blue for negative, black in the middle and red for positive values
        ,gpSpectrum  ///< An approximation of the visible light spectrum (creates banding illusion but allows more precise magnitude estimates)
        ,gpJet       ///< Hue variation similar to a spectrum, often used in numerical visualization (creates banding illusion but allows more precise magnitude estimates)
        ,gpHues      ///< Full hue cycle, with highest and lowest color red (suitable for periodic data, such as angles and phases, see \ref setPeriodic)
    };

    class TColorMapBase : public TAbstractPlottable{
    public:
        TColorMapBase(const TPtrAxis& key, const TPtrAxis& val);
        void SetDataRange(const TRange& value);
        void RescaleDataRange(bool recalcDataBounds = false);
        TColorMapData* Data() const { return mapData.get(); }

        double SelectTest(const TPointF& pos, bool onlySelectable) override;

        TGradientPreset Preset() const;
        virtual void SetPreset(TGradientPreset value);

        size_t HeaderHeight() const;
        virtual void SetHeaderHeight(size_t value);

        virtual TRange GetKeyRange(bool* founded, TSigDomain sig = sdBoth) const;
        virtual TRange GetValRange(bool* founded, TSigDomain sig = sdBoth, const TRange& inRange = TRange()) const;
    protected:
        TRange dataRange;
        TScaleType scaleType = stLinear;
        bool isInterpolate = true;
        bool isTightBoundary = false;
        size_t headerHeight = 0;
        TGradientPreset preset = TGradientPreset::gpThermal;
        std::unique_ptr<TColorMapData> mapData = std::make_unique<TColorMapData>(10, 10, TRange(0, 5), TRange(0, 5));

        bool isMapImageInvalidated = true;

    };

    CLASS_PTRS(ColorMapBase)
}




#endif //TESTQT_PLOTTABLES_H
