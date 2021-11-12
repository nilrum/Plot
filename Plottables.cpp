//
// Created by user on 19.05.2020.
//


#include "Plottables.h"
#include "Plot.h"
#include <cstring>

namespace Plot {

    TDataRange TDataRange::Bounded(const TDataRange &other) const
    {
        TDataRange res(Intersection(other));
        if(res.Empty() == false) return res;

        if(end <= other.begin)
            return TDataRange(other.begin, other.begin);
        else
            return TDataRange(other.end, other.end);
    }

    TDataRange TDataRange::Expanded(const TDataRange &other) const
    {
        return TDataRange(std::min(begin, other.begin), std::max(end, other.end));
    }

    TDataRange TDataRange::Intersection(const TDataRange &other) const
    {
        TDataRange res(std::max(begin, other.begin), std::min(end, other.end));
        if(res.IsValid())
            return res;
        else
            return TDataRange();
    }

    bool TDataRange::Contains(const TDataRange &other) const
    {
        return begin <= other.begin && end >= other.end;
    }

    TAbstractPlottable::TAbstractPlottable(const TPtrAxis& key, const TPtrAxis& val) : TLayerable(key->Plot(), "main"),
                                                                                     keyAxis(key), valAxis(val)
    {
        SetParentLayerable(key->AxisRect());
    }

    TAbstractPlottable::~TAbstractPlottable()
    {
        if(selection) plot->OnSelectionChanged(false);
    }

    Plot::TPointF TAbstractPlottable::CoordToPixel(double key, double val)
    {
        if (keyAxis.expired() || valAxis.expired()) return Plot::TPointF();
        if (KeyAxis()->Orientation() == Plot::orVert)
            return Plot::TPointF(ValAxis()->CoordToPixel(val), KeyAxis()->CoordToPixel(key));
        else
            return Plot::TPointF(KeyAxis()->CoordToPixel(key), ValAxis()->CoordToPixel(val));
    }

    Plot::TRect TAbstractPlottable::ClipRect() const
    {
        if (keyAxis.expired() || valAxis.expired()) return TLayerable::ClipRect();
        auto key = keyAxis.lock();
        auto val = valAxis.lock();
        if (key->Orientation() == Plot::orVert)
            return Plot::TRect(Plot::TPoint(val->InLeft(), key->InTop()),
                               Plot::TPoint(val->InRight(), key->InBottom()));
        else
            return Plot::TRect(Plot::TPoint(key->InLeft(), val->InTop()),
                               Plot::TPoint(key->InRight(), val->InBottom()));
    }

    Plot::TPen TAbstractPlottable::Pen() const
    {
        return pen;
    }

    void TAbstractPlottable::SetPen(const Plot::TPen &value)
    {
        pen = value;
    }

    void TAbstractPlottable::SetColorPen(const TColor &value)
    {
        pen.color = value;
    }

    void TAbstractPlottable::SetWidthPen(size_t value)
    {
        pen.width = value;
        if(selDecorator)
            selDecorator->Pen().width = pen.width;
    }

    void TAbstractPlottable::SetStylePen(const TPenStyle &value)
    {
        pen.style = value;
        if(selDecorator)
        {
            if(pen.style == TPenStyle::psNone)
                selDecorator->Pen().style = psSolid;
            else
                selDecorator->Pen().style = pen.style;
        }
    }

    Plot::TBrush TAbstractPlottable::Brush() const
    {
        return brush;
    }

    void TAbstractPlottable::SetBrush(const Plot::TBrush &value)
    {
        brush = value;
    }

    void TAbstractPlottable::DrawPolyLine(const TUPtrPainter &painter, TVecPointF lineData)
    {
        int i = 0;
        int count = lineData.size();
        if(painter->Pen().style == psSolid && false &&
            painter->TestFlag(TPainter::pmVectorized) == false &&
            painter->TestFlag(TPainter::pmNoCaching) == false)
        {
            bool lastIsNan = false;
            while(i < count && (std::isnan(lineData[i].y()) || std::isnan(lineData[i].x())))
                ++i;
            ++i;
            while(i < count)
            {
                const TPointF& p = lineData[i];
                if(std::isnan(p.y()) == false && std::isnan(p.x()) == false)
                {
                    if(lastIsNan == false)
                        painter->DrawLine(lineData[i - 1], p);
                    else
                        lastIsNan = false;
                }
                else
                    lastIsNan = true;
                ++i;
            }
        }
        else
        {
            int segStart = 0;
            while(i < count)
            {
                if(std::isnan(lineData[i].y()) || std::isnan(lineData[i].x()))
                {
                    painter->DrawPolyline(&lineData[segStart], i - segStart);
                    segStart = i + 1;
                }
                ++i;
            }
            if(segStart < count)
                painter->DrawPolyline(&lineData[segStart], count - segStart);
        }
    }

    bool TAbstractPlottable::SelectEvent(TMouseInfo &info, bool additive)
    {
        if(selectable != stNone)
        {
            if(additive)
            {
                if(selectable == stWhole)
                {
                    if(IsSelected())
                        return SetSelection(TDataSelection());
                    else
                        return SetSelection(bufferSelect);
                }
                else
                {
                    if(selection.Contains(bufferSelect))
                        return SetSelection(selection - bufferSelect);
                    else
                        return SetSelection(selection + bufferSelect);
                }

            }
            return SetSelection(bufferSelect);
        }
        return false;
    }

    bool TAbstractPlottable::DeselectEvent()
    {
        if(selectable != stNone)
            return SetSelection(TDataSelection());
        return false;
    }

    bool TAbstractPlottable::SetSelection(TDataSelection value)
    {
        value.EnforceType(selectable);
        return SelectionChanged(selection, value);
    }

    void TAbstractPlottable::PixelToCoords(const TPointF &pos, double &key, double &val)
    {
        if(keyAxis.expired() || valAxis.expired()) return;

        if(KeyAxis()->Orientation() == orVert)
        {
            key = KeyAxis()->PixelToCoord(pos.y());
            val = ValAxis()->PixelToCoord(pos.x());
        }
        else
        {
            key = KeyAxis()->PixelToCoord(pos.x());
            val = ValAxis()->PixelToCoord(pos.y());
        }
    }

    double TAbstractPlottable::PixelToKey(const TPointF &pos)
    {
        if(keyAxis.expired()) return 0;
        if(KeyAxis()->Orientation() == orVert)
            return KeyAxis()->PixelToCoord(pos.y());
        else
            return KeyAxis()->PixelToCoord(pos.x());
    }

    void TAbstractPlottable::RescaleValueAxis(bool onlyErlange, bool inKeyRange)
    {
        if(keyAxis.expired() || valAxis.expired()) return;
        TSigDomain sig = sdBoth;
        auto key = KeyAxis();
        auto val = ValAxis();
        if(val->ScaleType() == stLog)
            sig = (val->Range().upper < 0) ? sdNegative : sdPositive;
        bool founded = false;
        TRange valRange = GetValRange(&founded, sig, inKeyRange ? key->Range() : TRange() );

        if(founded)
        {
            if(onlyErlange)
                valRange.Expand(val->Range());

            if(valRange.IsValid())
                val->SetRange(valRange, true);
            else
                val->SetRange(valRange.lower, asCenter);
        }
    }

    bool TAbstractPlottable::SetIsSelect(bool value)
    {
        return SetSelection(value ? TDataRange(0, 1) : TDataRange(0, 0));
    }

    void TAbstractPlottable::SetEditInter(const TPtrPlottableEditInterface &value)
    {
        if(editInter)
            editInter->SetPlottable(nullptr);
        editInter = value;
        if(editInter) editInter->SetPlottable(this);
    }




//----------------------------------------------------------------------------------------------------------------------
    TGraph::TGraph(const TPtrAxis &key, const TPtrAxis &val, TTypeGraph type) : TAbstractPlottable1D<TGraphData>(key, val), typeGraph(type)
    {
    }

    TGraph::~TGraph()
    {

    }

    void TGraph::Draw(const TUPtrPainter &painter)
    {
        if(typeGraph == tgLine)
            DrawTemplate<true>(painter, this, &TGraph::DrawLine, &TGraph::DrawLine);
        else
            DrawTemplate<false>(painter, this, &TGraph::DrawColumn, &TGraph::DrawColumn);
    }

    TVecPointF TGraph::GetLines(const TDataRange &dataRange)
    {
        TDataContainerGraph::TConstIterator begin, end;

        GetVisibleDataBounds(begin, end, dataRange);
        if(begin == end) return Plot::TVecPointF();

        std::vector<TGraphData> lineData;
        if(typeGraph != tgNone)
            lineData = GetOptimizedLineData(begin, end);
        auto key = keyAxis.lock().get();
        if(key->IsRangeReversed() == (key->Orientation() == orVert))
            std::reverse(lineData.begin(), lineData.end());

        switch (typeGraph)
        {
            case tgLine: return DataToLines(lineData);
            case tgColumn: return DataToColumn(lineData);
            case tgCoupling : return DataToCoupling(lineData);
            default:
                break;
        }

        return Plot::TVecPointF();
    }

    void TGraph::GetVisibleDataBounds(TDataContainerGraph::TConstIterator &begin, TDataContainerGraph::TConstIterator &end,
                                      const TDataRange &dataRange)
    {
        if(dataRange.Empty())
        {
            begin = data->ConstEnd();
            end = begin;
            return;
        }

        if(keyAxis.expired()) return;
        auto key = keyAxis.lock().get();
        begin = data->FindBegin(key->Range().lower);
        end = data->FindEnd(key->Range().upper);

        data->LimitIterators(begin, end, dataRange);
    }

    TVecGraphData TGraph::GetOptimizedLineData(const TDataContainerGraph::TConstIterator &begin,
                                               const TDataContainerGraph::TConstIterator &end)
    {
        if(keyAxis.expired() || valAxis.expired() || begin == end) return TVecGraphData();
        int dataCount = end - begin;
        int maxCount = (std::numeric_limits<int>::max)();
        auto key = keyAxis.lock().get();
        if (isAdaptive)
        {
            double keyPixelSpan = std::fabs(key->CoordToPixel(begin->key) - key->CoordToPixel((end-1)->key));
            if (2 * keyPixelSpan + 2 < static_cast<double>((std::numeric_limits<int>::max)()))
                maxCount = int(2 * keyPixelSpan + 2);
        }
        if(isAdaptive && dataCount >= maxCount)
        {
            auto it = begin;
            double minVal = it->val;
            double maxVal = it->val;
            auto currentIntervalFirstPoint = it;

            int reversedFactor = key->PixelOrientation(); // is used to calculate keyEpsilon pixel into the correct direction
            int reversedRound = reversedFactor == -1 ? 1 : 0; // is used to switch between floor (normal) and ceil (reversed) rounding of currentIntervalStartKey
            double currentIntervalStartKey = key->PixelToCoord((int)(key->CoordToPixel(begin->key) + reversedRound));
            double lastIntervalEndKey = currentIntervalStartKey;
            double keyEpsilon = std::fabs(currentIntervalStartKey - key->PixelToCoord(key->CoordToPixel(currentIntervalStartKey) + 1.0 * reversedFactor)); // interval of one pixel on screen when mapped to plot key coordinates
            bool keyEpsilonVariable = key->ScaleType() == stLog; // indicates whether keyEpsilon needs to be updated after every interval (for log axes)
            int intervalDataCount = 1;
            TVecGraphData res;
            ++it; // advance iterator to second data point because adaptive sampling works in 1 point retrospect
            while (it != end)
            {
                if (it->key < currentIntervalStartKey + keyEpsilon) // data point is still within same pixel, so skip it and expand value span of this cluster if necessary
                {
                    if (it->val < minVal)
                        minVal = it->val;
                    else if (it->val > maxVal)
                        maxVal = it->val;
                    ++intervalDataCount;
                }
                else // new pixel interval started
                {
                    if (intervalDataCount >= 2) // last pixel had multiple data points, consolidate them to a cluster
                    {
                        if (lastIntervalEndKey < currentIntervalStartKey - keyEpsilon) // last point is further away, so first point of this cluster must be at a real data point
                            res.push_back(TGraphData(currentIntervalStartKey + keyEpsilon * 0.2, currentIntervalFirstPoint->val));
                        res.push_back(TGraphData(currentIntervalStartKey + keyEpsilon * 0.25, minVal));
                        res.push_back(TGraphData(currentIntervalStartKey + keyEpsilon * 0.75, maxVal));
                        if (it->key > currentIntervalStartKey + keyEpsilon * 2) // new pixel started further away from previous cluster, so make sure the last point of the cluster is at a real data point
                            res.push_back(TGraphData(currentIntervalStartKey + keyEpsilon * 0.8, (it - 1)->val));
                    } else
                        res.push_back(TGraphData(currentIntervalFirstPoint->key, currentIntervalFirstPoint->val));
                    lastIntervalEndKey = (it - 1)->key;
                    minVal = it->val;
                    maxVal = it->val;
                    currentIntervalFirstPoint = it;
                    currentIntervalStartKey = key->PixelToCoord((int)(key->CoordToPixel(it->key) + reversedRound));
                    if (keyEpsilonVariable)
                        keyEpsilon = std::fabs(currentIntervalStartKey - key->PixelToCoord(key->CoordToPixel(currentIntervalStartKey) + 1.0 * reversedFactor));
                    intervalDataCount = 1;
                }
                ++it;
            }
            // handle last interval:
            if (intervalDataCount >= 2) // last pixel had multiple data points, consolidate them to a cluster
            {
                if (lastIntervalEndKey < currentIntervalStartKey - keyEpsilon) // last point wasn't a cluster, so first point of this cluster must be at a real data point
                    res.push_back(TGraphData(currentIntervalStartKey + keyEpsilon * 0.2, currentIntervalFirstPoint->val));
                res.push_back(TGraphData(currentIntervalStartKey+keyEpsilon * 0.25, minVal));
                res.push_back(TGraphData(currentIntervalStartKey+keyEpsilon * 0.75, maxVal));
            }
            else
                res.push_back(TGraphData(currentIntervalFirstPoint->key, currentIntervalFirstPoint->val));
            return res;
        }
        else
        {
            TVecGraphData res(dataCount);
            std::copy(begin, end, res.begin());
            return res;
        }
    }

    TVecPointF TGraph::DataToLines(const TVecGraphData &value)
    {
        auto key = keyAxis.lock().get();
        auto val = valAxis.lock().get();

        if(key == nullptr || val == nullptr) return TVecPointF();

        TVecPointF res(value.size());
        if(key->Orientation() == orVert)
            for(size_t i = 0; i < value.size(); i++)
            {
                res[i].setX(val->CoordToPixel(value[i].val));
                res[i].setY(key->CoordToPixel(value[i].key));
            }
        else
            for(size_t i = 0; i < value.size(); i++)
            {
                res[i].setX(key->CoordToPixel(value[i].key));
                res[i].setY(val->CoordToPixel(value[i].val));
            }
        return res;
    }

    TVecPointF TGraph::DataToColumn(const TVecGraphData &value)
    {
        auto key = keyAxis.lock().get();
        auto val = valAxis.lock().get();

        if(key == nullptr || val == nullptr) return TVecPointF();
        size_t count = value.size();//количество точек в колонне
        TVecPointF res(count * 2 + 1);//столько же точек с другой стороны плюс одна, чтобы закрыть крышку
        if(key->Orientation() == orVert)
        {
            for (size_t i = 0; i < count; i++)//считаем по положительной стороне
            {
                res[i].setX(val->CoordToPixel(value[i].val));
                res[i].setY(key->CoordToPixel(value[i].key));
            }
            for (size_t i = 0; i < count; i++)//считаем по отриц стороне
            {
                res[i + count].setX(val->CoordToPixel(-value[count - 1 - i].val));
                res[i + count].setY(res[count - 1 - i].y());
            }
            res.back() = res.front();//последнюю точку дублируем на начальную(закрываем крышку)
        }
        else
        {
            for (size_t i = 0; i < count; i++)
            {
                res[i].setX(key->CoordToPixel(value[i].key));
                res[i].setY(val->CoordToPixel(value[i].val));
            }
            for (size_t i = 0; i < count; i++)
            {
                res[i + count].setX(res[count - 1 - i].x());
                res[i + count].setY(val->CoordToPixel(-value[count - 1 - i].val));
            }
            res.back() = res.front();
        }
        return res;
    }

    TVecPointF TGraph::DataToCoupling(const TVecGraphData &value)
    {
        auto key = keyAxis.lock().get();
        auto val = valAxis.lock().get();

        if(key == nullptr || val == nullptr) return TVecPointF();

        TVecPointF res(value.size() * 4);
        double x1 = 0, x2 = 0;
        double y1 = 0, y2 = 0;
        double maxVal = 500;//если кривой от которой зависим нет то берем константу по ширине
        if(fillGraph.expired() == false)
            maxVal = fillGraph.lock()->data->ValRange().upper * 1.2;

        if(key->Orientation() == orVert)
            for(size_t i = 0; i < value.size(); i++)//0 3
        {                                           //1 2
                x1 = val->CoordToPixel(-maxVal);
                x2 = val->CoordToPixel(maxVal);
                y1 = key->CoordToPixel(value[i].key - value[i].val / 2.);
                y2 = key->CoordToPixel(value[i].key + value[i].val / 2.);
                res[i * 4 + 0].setX(x1);
                res[i * 4 + 0].setY(y1);
                res[i * 4 + 1].setX(x1);
                res[i * 4 + 1].setY(y2);
                res[i * 4 + 2].setX(x2);
                res[i * 4 + 2].setY(y2);
                res[i * 4 + 3].setX(x2);
                res[i * 4 + 3].setY(y1);
            }
        else
            {//TODO

            }
        return res;
    }

    void TGraph::DrawFill(const TUPtrPainter &painter, const TVecPointF& lineData)
    {
        if((brush.style == bsNone && otherBrush.style == bsNone) || typeGraph != tgLine)
            return;
        painter->SetBrush(brush);
        TVecDataRange segs = GetNonNanSegments(lineData, keyAxis.lock()->Orientation());
        if(fillGraph.expired())
        {
            for(size_t i = 0; i < segs.size(); i++)
                painter->DrawPolygon(GetFillPolygon(lineData, segs[i]));

            if(brush.color != otherBrush.color)
            {
                painter->SetBrush(otherBrush);
                for(size_t i = 0; i < segs.size(); i++)
                {
                    painter->Save();
                    painter->SetClipRect(TRectF(TPointF{0, 0}, GetFillBasePoint(lineData[segs[i].End() - 1], -1)));//минус один пиксель чтобы грань не рисовать
                    painter->DrawPolygon(GetFillPolygon(lineData, segs[i]));
                    painter->Restore();
                }
            }
        }
        else
        {
            auto fill = FillGraph().get();
            TVecPointF otherLines = fill->GetLines(TDataRange(0, fill->CountData()));
            if(otherLines.empty() == false)
            {
                TVecDataRange otherSegments = GetNonNanSegments(otherLines, fill->KeyAxis()->Orientation());
                TVecPairDataRange segmentPairs = GetOverlappingSegments(segs, lineData, otherSegments, otherLines);

                for(size_t i = 0; i < segmentPairs.size(); i++)
                    painter->DrawPolygon(GetChannelFillPolygon(lineData, segmentPairs[i].first, otherLines, segmentPairs[i].second));

                if(brush.color != otherBrush.color)
                {
                    painter->SetBrush(otherBrush);
                    for(size_t i = 0; i < segmentPairs.size(); i++)
                    {
                        painter->Save();
                        painter->SetClipPolygon(ClipPolygon(otherLines, segmentPairs[i].second), true);
                        painter->DrawPolygon(GetChannelFillPolygon(lineData, segmentPairs[i].first, otherLines,
                                                                   segmentPairs[i].second));
                        painter->Restore();
                    }
                }
            }
        }
    }

    TVecPointF TGraph::ClipPolygon(const TVecPointF& lines, const TDataRange& r)
    {
        TVecPointF res(lines.begin() + r.Begin(), lines.begin() + r.End());
        if(keyAxis.lock()->Orientation() == orVert)
        {
            res.push_back(TPointF(0, res.back().y()));
            res.push_back(TPointF(0, res.front().y()));
        }
        else
        {
            res.push_back(TPointF(res.back().x(), 0));
            res.push_back(TPointF(res.front().x(), 0));
        }
        return res;
    }

    TVecDataRange TGraph::GetNonNanSegments(const TVecPointF &lineData, TOrientation orientation)
    {
        TVecDataRange res;
        TDataRange cur(-1, -1);
        int i = 0;
        int count = lineData.size();
        if(orientation == orVert)
        {
            while (i < count)
            {
                while(i < count && std::isnan(lineData[i].x()))
                    ++i;
                if(i == count) break;
                cur.SetBegin(i++);
                while(i < count && std::isnan(lineData[i].x()) == false)
                    ++i;
                cur.SetEnd(i++);
                res.push_back(cur);
            }
        }
        else
        {
            while (i < count)
            {
                while(i < count && std::isnan(lineData[i].y()))
                    ++i;
                if(i == count) break;
                cur.SetBegin(i++);
                while(i < count && std::isnan(lineData[i].y()) == false)
                    ++i;
                cur.SetEnd(i++);
                res.push_back(cur);
            }
        }
        return res;
    }

    TVecPointF TGraph::GetFillPolygon(const TVecPointF &lineData, const TDataRange &segment)
    {
        if(segment.Size() < 2) return TVecPointF();
        TVecPointF res(segment.Size() + 2);                 //буфер под результат плюс две точки под ось
        res.front() = GetFillBasePoint(lineData[segment.Begin()]);
        std::copy(lineData.begin() + segment.Begin(), lineData.begin() + segment.End(), res.begin() + 1);
        res.back() = GetFillBasePoint(lineData[segment.End() - 1]);
        return res;
    }

    TPointF TGraph::GetFillBasePoint(const TPointF &matchPoint, double offset)
    {
        if(keyAxis.expired() || valAxis.expired()) return Plot::TPointF();
        auto key = keyAxis.lock().get();
        auto val = valAxis.lock().get();
        if(val->ScaleType() == stLinear)
        {
            if(key->Orientation() == orVert)
                return TPointF(val->CoordToPixel(baseFillPoint) + offset, matchPoint.y());
            else
                return TPointF( matchPoint.x(), val->CoordToPixel(baseFillPoint) + offset);
        }
        else
        {
            //TODO logScale
            return Plot::TPointF();
        }
    }

    TVecPairDataRange TGraph::GetOverlappingSegments(const TVecDataRange &thisSegs, const TVecPointF &thisData,
                                                     const TVecDataRange &otherSegs, const TVecPointF &otherData)
    {
        TVecPairDataRange res;
        if(thisData.empty() || otherData.empty() || thisSegs.empty() || otherSegs.empty())
            return res;
        size_t thisIndex = 0;
        size_t otherIndex = 0;
        bool isVertKey = keyAxis.lock()->Orientation() == orVert;
        while(thisIndex < thisSegs.size() && otherIndex < otherSegs.size())
        {
            const TDataRange& thisSeg = thisSegs[thisIndex];
            if(thisSeg.Size() < 2)
            {
                ++thisIndex;
                continue;
            }

            const TDataRange& otherSeg = otherSegs[otherIndex];
            if(otherSeg.Size() < 2)
            {
                ++otherIndex;
                continue;
            }
            double thisLower, thisUpper, otherLower, otherUpper;

            if(isVertKey)
            {
                thisLower = thisData[thisSeg.Begin()].y();
                thisUpper = thisData[thisSeg.End() - 1].y();
                otherLower = otherData[otherSeg.Begin()].y();
                otherUpper = otherData[otherSeg.End() - 1].y();
            }
            else
            {
                thisLower = thisData[thisSeg.Begin()].x();
                thisUpper = thisData[thisSeg.End() - 1].x();
                otherLower = otherData[otherSeg.Begin()].x();
                otherUpper = otherData[otherSeg.End() - 1].x();
            }
            int precedence = 0;
            if(SegmentIntersect(thisLower, thisUpper, otherLower, otherUpper, precedence))
                res.push_back(TPairDataRange(thisSeg, otherSeg));

            if(precedence <= 0)
                ++otherIndex;
            else
                ++thisIndex;
        }
        return res;
    }

    bool
    TGraph::SegmentIntersect(double thisLower, double thisUpper, double otherLower, double otherUpper, int &precedence)
    {
        precedence = 0;
        if(thisLower > otherUpper)
        {
            precedence = -1;
            return false;
        }
        else if(otherLower > thisUpper)
        {
            precedence = 1;
            return false;
        }
        else
        {
            if(thisUpper > otherUpper)
                precedence = -1;
            else if(thisUpper < otherUpper)
                precedence = 1;
            return true;
        }
    }

    bool FuzzyCompare(double p1, double p2)
    {
        return (std::fabs(p1 - p2) * 1000000000000. <= std::min(std::fabs(p1), std::fabs(p2)));
    }

    int FindIndexAboveY(const TVecPointF* data, double y)
    {
        for(int i = data->size() - 1; i >= 0; --i)
            if(data->at(i).y() < y)
            {
                if(i < int(data->size() - 1))
                    return i + 1;
                else
                    return data->size() - 1;
            }
        return -1;
    }

    int FindIndexBelowY(const TVecPointF* data, double y)
    {
        for(size_t i = 0; i < data->size(); i++)
            if(data->at(i).y() > y)
            {
                if(i > 0)
                    return i - 1;
                else
                    return 0;
            }
        return -1;
    }

    TVecPointF
    TGraph::GetChannelFillPolygon(const TVecPointF &thisData, const TDataRange &thisRange, const TVecPointF &otherData,
                                  const TDataRange &otherRange)
    {
        if(keyAxis.expired() || valAxis.expired() || FillGraph()->KeyAxis() == nullptr)
            return Plot::TVecPointF();

        if(keyAxis.lock()->Orientation() != FillGraph()->KeyAxis()->Orientation() || thisData.empty())
            return Plot::TVecPointF();

        TVecPointF thisSegData(thisRange.Size());
        TVecPointF otherSegData(otherRange.Size());
        std::copy(thisData.begin() + thisRange.Begin(), thisData.begin() + thisRange.End(), thisSegData.begin());
        std::copy(otherData.begin() + otherRange.Begin(), otherData.begin() + otherRange.End(), otherSegData.begin());

        TVecPointF* staticData = &thisSegData;
        TVecPointF* croppedData = &otherSegData;
        if(keyAxis.lock()->Orientation() == orVert)
        {
            if(staticData->front().y() < croppedData->front().y())
                std::swap(staticData, croppedData);

            int lowBound = FindIndexBelowY(croppedData, staticData->front().y());//ищем индекс в массиве croppedData данные которые меньше массива staticData по высоте
            if (lowBound == -1) return TVecPointF();//значит массивы не перекрывают друг друга
            croppedData->erase(croppedData->begin(), croppedData->begin() + lowBound);//удаляем не перекрывающийся интервал
            if (croppedData->size() < 2) return TVecPointF();//если массив почти опустел, то выходим

            //находим точную точку начала вот втором массиве
            double slope = 0;
            if (FuzzyCompare(croppedData->at(1).y(), croppedData->at(0).y()) == false)
                slope = (croppedData->at(1).x() - croppedData->at(0).x()) / (croppedData->at(1).y() - croppedData->at(0).y());
            croppedData->at(0).setX(croppedData->at(0).x() + slope * (staticData->front().y() - croppedData->front().y()));
            croppedData->at(0).setY(staticData->front().y());

            if(staticData->back().y() > croppedData->back().y())
                std::swap(staticData, croppedData);

            int highBound = FindIndexAboveY(croppedData, staticData->back().y());
            if(highBound == -1) return TVecPointF();
            croppedData->erase(croppedData->begin() + highBound + 1, croppedData->end());
            if(croppedData->size() < 2) return TVecPointF();
            int li = croppedData->size() - 1;
            if(FuzzyCompare(croppedData->at(li).y(), croppedData->at(li - 1).y()) == false)
                slope = (croppedData->at(li).x() - croppedData->at(li - 1).x()) / (croppedData->at(li).y() - croppedData->at(li - 1).y());
            else
                slope = 0;
            croppedData->at(li).setX(croppedData->at(li - 1).x() + slope * (staticData->back().y() - croppedData->at(li - 1).y()));
            croppedData->at(li).setY(staticData->back().y());
        }
        else
        {
            //TODO orGor
        }
        int begSet = thisSegData.size();
        thisSegData.resize(begSet + otherSegData.size());
        for(int i = otherSegData.size() - 1; i >= 0; --i)
            thisSegData[begSet++] = otherSegData[i];
        return thisSegData;
    }


    double TGraph::SelectTest(const TPointF &pos, bool onlySelectable)
    {
        if((onlySelectable && selectable == stNone) || data->IsEmpty())
            return -1;

        if(keyAxis.expired() || valAxis.expired())
            return -1;

        if(keyAxis.lock()->Contains(pos) && valAxis.lock()->Contains(pos))
        {
            auto pointIter = data->ConstEnd();
            double dist = PointDistance(pos, pointIter);

            int pointIndex = pointIter - data->Begin();
            bufferSelect = TDataSelection(TDataRange(pointIndex, pointIndex + 1));

            return dist;
        }
        return -1;
    }

    double LengthSquared(const TPointF& value){ return value.x() * value.x() + value.y() * value.y(); }
    double Dot(const TPointF& pos, const TPointF& vec) { return pos.x() * vec.x() + pos.y() * vec.y(); }
    double DistanceSquaredToLine(const TPointF& pos, const TPointF& start, const TPointF& stop)
    {
        TPointF v = stop - start;
        double lengthSqrt = LengthSquared(v);
        if(fabs(lengthSqrt) > 0.000000000001)
        {
            double mu = Dot(v, pos - start) / lengthSqrt;
            if(mu < 0)
                return LengthSquared(pos - start);
            else if(mu > 1)
                return LengthSquared(pos - stop);
            else
                return LengthSquared((start + mu * v) - pos);
        }
        else
            return LengthSquared(pos - start);
    }

    double TGraph::PointDistance(const TPointF &pos, TDataContainerGraph::TConstIterator &iter)
    {
        iter = data->ConstEnd();
        if(data->IsEmpty() || (typeGraph == tgNone && scatter.IsNone()))
            return -1;

        double minDist = std::numeric_limits<double>::max();
        double posMin = PixelToKey(pos - plot->TolerancePoint());
        double posMax = PixelToKey(pos + plot->TolerancePoint());
        if(posMin > posMax) std::swap(posMin, posMax);

        auto begin = data->FindBegin(posMin, true);
        auto end = data->FindEnd(posMax, true);

        for(auto it = begin; it != end; it++)
        {
            double curDist = LengthSquared(CoordToPixel(it->key, it->val) - pos);
            if(curDist < minDist)
            {
                minDist = curDist;
                iter = it;
            }
        }
        if(typeGraph != tgNone)
        {
            TVecPointF lineData = GetLines(data->DataRange());
            if(typeGraph != tgCoupling)
                for(size_t i = 0; i < lineData.size() - 1; i++)
                {
                    double curDist = DistanceSquaredToLine(pos, lineData[i], lineData[i + 1]);
                    if(curDist < minDist)
                        minDist = curDist;
                }
            else
                for(size_t i = 0; i < lineData.size(); i += 4)
                    if(TRectF(lineData[i], lineData[i + 2]).contains(pos))
                    {
                        return plot->SelectionTolerance() * 0.99;
                    }

        }
        return std::sqrt(minDist);
    }

    void TGraph::MousePress(TMouseInfo &info)
    {
        if(editInter == nullptr)
            TLayerable::MousePress(info);
        else
            editInter->MousePress(info);
    }

    void TGraph::MouseMove(TMouseInfo &info, const TPoint &startPos)
    {
        if(editInter == nullptr)
            TLayerable::MouseMove(info, startPos);
        else
            editInter->MouseMove(info, startPos);
    }

    void TGraph::MouseUp(TMouseInfo &info, const TPoint &startPos)
    {
        if(editInter == nullptr)
            TLayerable::MouseUp(info, startPos);
        else
            editInter->MouseUp(info, startPos);
    }

    bool TGraph::CheckIsDraw() const
    {
        if(keyAxis.expired() || valAxis.expired()) return false;
        if(typeGraph == tgNone && scatter.IsNone()) return false;
        if(keyAxis.lock()->Range().Size() <= 0 || data->IsEmpty()) return false;
        return true;
    }

    void TGraph::DrawLegendIcon(const TUPtrPainter &painter, const TRectF iconRect)
    {
        if(scatter.IsNone() == false)
        {
            painter->SetPen(pen);
            painter->DrawLine(TPointF(iconRect.left(), iconRect.CenterVert()), TPointF(iconRect.right() + 5, iconRect.CenterVert()));
            auto border = iconRect;
            border.adjust(0, 0, -2, -2);
            painter->DrawRect(border);
            scatter.Draw(painter, {iconRect.center()});
        }
        else
            painter->FillRect(iconRect, pen.color);
    }

    TPointF TGraph::TracerValue(double key) const
    {
        if(data->Size() > 1)
        {
            auto front = data->ConstBegin();
            auto back = data->ConstEnd() - 1;

            if(key <= front->key)
                return TPointF(front->key, front->val);
            else
            if(key >= back->key)
                return TPointF(back->key, back->val);
            else
            {
                auto it = data->FindBegin(key);
                if(it != data->ConstEnd())
                {
                    TDataContainerGraph::TConstIterator prev = it++;// won't advance to constEnd because we handled that case (graphKey >= back->key) before
                    if (key < (prev->key + it->key) * 0.5)
                        return TPointF(prev->key, prev->val);
                    else
                        return TPointF(it->key, it->val);
                }
            }
        }
        else
        {
            if(data->Size() == 1)
            {
                auto it = data->ConstBegin();
                return TPointF(it->key, it->val);
            }
        }
        return TAbstractPlottable::TracerValue(key);
    }

    double TGraph::BaseFillPoint() const
    {
        return baseFillPoint;
    }

    void TGraph::SetBaseFillPoint(double value)
    {
        baseFillPoint = value;
    }

    TBrush TGraph::OtherBrush() const
    {
        return otherBrush;
    }

    void TGraph::SetOtherBrush(const TBrush &value)
    {
        otherBrush = value;
    }

//----------------------------------------------------------------------------------------------------------------------
    TCouplingPlottable::TCouplingPlottable(const TPtrAxis& key, const TPtrAxis& val) : TGraph(key, val, tgCoupling)
    {

    }

    void TCouplingPlottable::MousePress(TMouseInfo &info)
    {
        if(info.IsLeftButton() && IsSelected() && info.IsMoving())
        {
            plot->SetCursor(ctVDrag);
            indDragging = selection.DataRanges()[0].Begin();
            valDragging = (data->ConstBegin() + indDragging)->key;
        }
        else
            indDragging = -1;
    }

    void TCouplingPlottable::MouseMove(TMouseInfo &info, const TPoint &startPos)
    {
        if(indDragging != -1)
        {
            auto key = keyAxis.lock().get();
            (data->Begin() + indDragging)->key = valDragging + key->PixelToCoord(info.pos.y()) - key->PixelToCoord(startPos.y());

            plot->Replot();
        }
    }

    void TCouplingPlottable::MouseUp(TMouseInfo &info, const TPoint &startPos)
    {
        if(indDragging != -1)
        {
            double k = (data->Begin() + indDragging)->key;
            if (k != valDragging)
                OnValueDragged(indDragging, k, (data->Begin() + indDragging)->val);
            plot->SetCursor(ctDefault);
            indDragging = -1;
        }
    }

    void TCouplingPlottable::Draw(const TUPtrPainter &painter)
    {
        DrawTemplate<false>(painter, this, &TCouplingPlottable::DrawCoupling, &TCouplingPlottable::DrawCouplingSel);
    }

    void TCouplingPlottable::DrawCoupling(const TUPtrPainter &painter, const TDataRange &seg, const TVecPointF &lines)
    {
        if(isMaskColumn && fillGraph.expired() == false)
        {
            painter->Save();
            painter->SetClipPolygon(FillGraph()->GetLines(seg), true);
        }

        for(size_t j = 0; j < lines.size(); j += 4)
            painter->DrawPolygon(lines.data() + j, 4);

        if(isMaskColumn)
        {
            painter->Restore();
        }
    }

    void TCouplingPlottable::DrawCouplingSel(const TUPtrPainter &painter, const TDataRange &seg, const TVecPointF &lines)
    {
        DrawCoupling(painter, seg, lines);
        TPen p(painter->Pen().color, 1);

        painter->SetPen(p);
        painter->Save();
        painter->SetClipRect(plot->MainLayout()->InnerRect());
        int xe = this->plot->MainLayout()->InnerRect().width();
        int y = 0;
        for (size_t j = 0; j < lines.size(); j += 4)
        {
            y = int(lines[j].y() + (lines[j + 1].y() - lines[j].y()) / 2.);
            painter->DrawLine(TPoint(0, y), TPoint(xe, y));
        }
        painter->Restore();
    }

    void TCouplingPlottable::SetIsMaskColumn(bool value)
    {
        isMaskColumn = value;
    }

//----------------------------------------------------------------------------------------------------------------------
    TGridAxes::TGridAxes(const TPtrAxis& key, const TPtrAxis& val) : TAbstractPlottable(key, val)
    {
        pen.style = psSolid;//psDot;
        pen.color = TConsts::SetAlpha(TConsts::BlackColor(), 100);
        penSubTick.style = psDot;
        penSubTick.color = TConsts::SetAlpha(TConsts::BlackColor(), 100);
    }

    void TGridAxes::Draw(const TUPtrPainter &painter)
    {
        if(keyAxis.expired() || valAxis.expired()) return;
        auto key = KeyAxis();
        auto val = ValAxis();
        if(key->Orientation() == orVert)
        {
            DrawVert(painter, key, val);
            DrawHorz(painter, val, key);
        }
        else
        {
            DrawHorz(painter, key, val);
            DrawVert(painter, val, key);
        }
    }

    void TGridAxes::DrawVert(const TUPtrPainter &painter, const TPtrAxis& axis, const TPtrAxis& twoAxis)
    {
        double begin = twoAxis->InLeft();
        double end = twoAxis->InRight();
        const TVecDouble& ticks = axis->TickPositions();
        painter->SetPen(pen);
        for(const auto& p : ticks)
            painter->DrawLine(TPointF(begin, p), TPointF(end, p));
        if(axis->IsSubTicks() && isSubTick)
        {
            const TVecDouble& subticks = axis->SubTickPositions();
            painter->SetPen(penSubTick);
            for(const auto& p : subticks)
                painter->DrawLine(TPointF(begin, p), TPointF(end, p));
        }
    }

    void TGridAxes::DrawHorz(const TUPtrPainter &painter, const TPtrAxis& axis, const TPtrAxis& twoAxis)
    {
        double begin = twoAxis->InTop();
        double end = twoAxis->InBottom();
        const TVecDouble& ticks = axis->TickPositions();
        painter->SetPen(pen);
        for(const auto& p : ticks)
            painter->DrawLine(TPointF(p, begin), TPointF(p, end));
        if(axis->IsSubTicks() && isSubTick)
        {
            const TVecDouble& subTicks = axis->SubTickPositions();
            painter->SetPen(penSubTick);

            for(const auto& p : subTicks)
                painter->DrawLine(TPointF(p, begin), TPointF(p, end));
        }
    }

    bool TDataSelection::Contains(const TDataSelection &oth)
    {
        if(oth.Empty()) return false;

        size_t othIndex = 0;
        size_t thisIndex = 0;
        while (thisIndex < ranges.size() && othIndex < oth.ranges.size())
        {
            if (ranges[thisIndex].Contains(oth.ranges[othIndex]))
                ++othIndex;
            else
                ++thisIndex;
        }
        return thisIndex < ranges.size();
    }

    bool TDataSelection::operator==(const TDataSelection &other) const
    {
        if(ranges.size() != other.ranges.size())
            return false;
        for(size_t i = 0; i < ranges.size(); i++)
            if(ranges[i] != other.ranges[i])
                return false;
        return true;
    }

    TDataSelection &TDataSelection::operator+=(const TDataSelection &other)
    {
        ranges.insert(ranges.end(), other.ranges.begin(), other.ranges.end());
        Simplify();
        return *this;
    }

    TDataSelection &TDataSelection::operator-=(const TDataSelection &other)
    {
        for (size_t i = 0; i < other.CountDataRange(); ++i)
            *this -= other.ranges[i];
        return *this;
    }

    TDataSelection &TDataSelection::operator+=(const TDataRange &other)
    {
        AddDataRange(other);
        Simplify();
        return *this;
    }

    TDataSelection &TDataSelection::operator-=(const TDataRange &other)
    {
        if(other.Empty() || Empty())
            return *this;
        Simplify();
        size_t i = 0;
        while (i < ranges.size())
        {
            const TDataRange& cur = ranges[i];
            if(cur.Begin() >= other.End())
                break;
            if(cur.End() > other.Begin())
            {
                if(cur.Begin() >= other.Begin())
                {
                    if(cur.End() <= other.End())
                    {
                        ranges.erase(ranges.begin() + i);
                        continue;
                    }
                    else
                        ranges[i].SetBegin(other.End());
                }
                else
                {
                    if(cur.End() <= other.End())
                    {
                        ranges[i].SetEnd(other.Begin());
                    }
                    else
                    {
                        ranges[i].SetEnd(other.Begin());
                        ranges.insert(ranges.begin() + i, TDataRange(other.End(), cur.End()));
                    }
                }
            }
            ++i;
        }
        return *this;
    }

    void TDataSelection::AddDataRange(const TDataRange &other, bool simplify)
    {
        ranges.push_back(other);
        if(simplify)
            Simplify();
    }

    void TDataSelection::Simplify()
    {
        for(int i = ranges.size() - 1; i >= 0; --i)
            if(ranges[i].Empty())
                ranges.erase(ranges.begin() + i);

        if(Empty()) return;

        std::sort(ranges.begin(), ranges.end(), LessThanDataRangeBegin);

        size_t i = 1;
        while (i < ranges.size())
        {
            if(ranges[i - 1].End() >= ranges[i].Begin())
            {
                ranges[i - 1].SetEnd(std::max(ranges[i - 1].End(), ranges[i].End()));
                ranges.erase(ranges.begin() + i);
            }
            else
                ++i;
        }
    }

    void TDataSelection::EnforceType(TSelectionType value)
    {
        Simplify();
        if(value == stNone)
        {
            Clear();
        }
        else if(value == stSingleData)
        {
            if(Empty() == false)
            {
                if(ranges.size() > 1) ranges.resize(1);
                if(ranges.front().Length() > 1) ranges.front().SetEnd(ranges.front().Begin() + 1);
            }
        }
        else if(value == stDataRange)
        {
            if(Empty() == false)
            {
                ranges.clear();
                ranges.push_back(Span());
            }
        }

    }

    TDataRange TDataSelection::Span() const
    {
        if(Empty())
            return TDataRange();
        else
            return TDataRange(ranges.front().Begin(), ranges.back().End());
    }

    TDataSelection TDataSelection::Inverse(const TDataRange &outerRange)
    {
        if(Empty())
            return TDataSelection(outerRange);
        TDataRange fullRange = outerRange.Expanded(Span());
        TDataSelection res;
        if(ranges.front().Begin() != fullRange.Begin())
            res.AddDataRange(TDataRange(fullRange.Begin(), ranges.front().Begin()), false);
        for(size_t i = 1; i < ranges.size(); ++i)
            res.AddDataRange(TDataRange(ranges[i - 1].End(), ranges[i].Begin()), false);
        if(ranges.back().End() != fullRange.End())
            res.AddDataRange(TDataRange(ranges.back().End(), fullRange.End()), false);
        res.Simplify();
        return res;
    }

    TColorMapData::TColorMapData(int kSize, int vSize, const TRange &kRange, const TRange &vRange):
            keyRange(kRange), valRange(vRange)
    {
        SetSize(kSize, vSize);
        Fill(0);
    }

    TColorMapData::TColorMapData(const TColorMapData &oth)
    {
        *this = oth;
    }

    TColorMapData &TColorMapData::operator=(const TColorMapData &oth)
    {
        if(&oth != this)
        {
            if(oth.alpha.empty() && alpha.size())
                ClearAlpha();
            SetSize(oth.keySize, oth.valSize);
            if(oth.alpha.size() && alpha.empty())
                CreateAlpha(false);
            SetRange(oth.keyRange, oth.valRange);
            if(isEmpty == false)
            {
                memcpy(data.data(), oth.data.data(), oth.data.size());
                if(alpha.size())
                    memcpy(alpha.data(), oth.alpha.data(), oth.alpha.size());
            }
            dataBounds = oth.dataBounds;
            isDataModifed = true;
        }
        return *this;
    }

    void TColorMapData::SetSize(int kSize, int vSize)
    {
        if(keySize == kSize && valSize == vSize) return;
        keySize = kSize;
        valSize = vSize;
        isEmpty = keySize == 0 || valSize == 0;
        if(isEmpty == false)
        {
            data.resize(keySize * valSize);
            Fill(0);
            if(alpha.size()) CreateAlpha();
        }
        else
        {
            data.clear();
        }
        isDataModifed = true;
    }

    void TColorMapData::SetRange(const TRange &kRange, const TRange &vRange)
    {
        keyRange = kRange;
        valRange = vRange;
    }

    void TColorMapData::SetKeyRange(const TRange &value)
    {
        keyRange = value;
    }

    void TColorMapData::SetValRange(const TRange &value)
    {
        valRange = value;
    }

    void TColorMapData::SetCell(int keyIndex, int valIndex, double value)
    {
        if(keyIndex < 0 || keyIndex >= keySize || valIndex < 0 || valIndex >= valSize)
            return;
        data[valIndex * keySize + keyIndex] = value;
        if(value < dataBounds.lower)
            dataBounds.lower = value;
        if(value > dataBounds.upper)
            dataBounds.upper = value;
        isDataModifed = true;
    }

    void TColorMapData::SetCellNoCheck(int keyIndex, int valIndex, double value)
    {
        data[valIndex * keySize + keyIndex] = value;
    }

    void TColorMapData::SetAlpha(int keyIndex, int valIndex, unsigned char value)
    {
        if(keyIndex < 0 || keyIndex >= keySize || valIndex < 0 || valIndex >= valSize)
            return;
        if(alpha.size() || CreateAlpha())
        {
            alpha[valIndex * keySize + keyIndex] = value;
            isDataModifed = true;
        }
    }

    bool TColorMapData::CreateAlpha(bool initOpaque)
    {
        ClearAlpha();
        if(isEmpty) return false;
        alpha.resize(keySize * valSize);
        if(initOpaque)
            FillAlpha(255);
        return true;
    }

    void TColorMapData::Clear()
    {
        SetSize(0, 0);
    }

    void TColorMapData::ClearAlpha()
    {
        if(alpha.size())
        {
            alpha.clear();
            isDataModifed = true;
        }
    }

    void TColorMapData::Fill(double value)
    {
        for(auto& it : data)
            it = value;
        dataBounds.Set(0, 0);
        isDataModifed = true;
    }

    void TColorMapData::FillAlpha(unsigned char value)
    {
        if(alpha.size() || CreateAlpha(false))
        {
            for(auto& it : alpha)
                it = value;
            isDataModifed = true;
        }
    }

    void TColorMapData::RecalculateDataBounds()
    {
        if(keySize > 0 && valSize > 0)
        {
            double minHeight = data[0];
            double maxHeight = data[0];
            for(const auto& d : data)
            {
                if(d > maxHeight)
                    maxHeight = d;
                if(d < minHeight)
                    minHeight = d;
            }
            dataBounds.Set(minHeight, maxHeight, false);
        }
    }

//-------------------------------------------------------------------------------------------------------------
    TColorMapBase::TColorMapBase(const TPtrAxis& key, const TPtrAxis& val) : TAbstractPlottable(key, val)
    {

    }

    void TColorMapBase::RescaleDataRange(bool recalcDataBounds)
    {
        if(recalcDataBounds)
            mapData->RecalculateDataBounds();
        SetDataRange(mapData->DataBounds());
    }

    void TColorMapBase::SetDataRange(const TRange &value)
    {
        if(value.IsValid() == false) return;
        if(dataRange.lower != value.lower || dataRange.upper != value.upper)
        {
            if(scaleType == stLog)
                dataRange = value.CheckForLogScale();
            else
                dataRange = value.CheckForLinScale();
            isMapImageInvalidated = true;
        }
    }

    TRange TColorMapBase::GetKeyRange(bool *founded, TSigDomain sig) const
    {
        if(founded) *founded = true;
        TRange res = mapData->KeyRange();
        res.Normalize();
        if(sig == TSigDomain::sdPositive)
        {
            if(res.lower <= 0 && res.upper > 0)
                res.lower = res.upper * 1e-3;
            else
            if(res.lower <= 0 && res.upper <= 0 && founded)
                *founded = false;
        }
        else if(sig == TSigDomain::sdNegative)
        {
            if(res.upper >= 0 && res.lower < 0)
                res.upper = res.lower * 1e-3;
            else
            if(res.lower >= 0 && res.upper >= 0)
                *founded = false;
        }
        return res;
    }

    TRange TColorMapBase::GetValRange(bool *founded, TSigDomain sig, const TRange &inRange) const
    {
        if(inRange.IsValid())
        {
            const TRange& kr = mapData->KeyRange();
            if(kr.upper < inRange.lower || kr.lower > inRange.upper)
            {
                if(founded) *founded = false;
                return TRange();
            }
        }
        if(founded) *founded = true;
        TRange res = mapData->ValRange();
        res.Normalize();
        if(sig == TSigDomain::sdPositive)
        {
            if(res.lower <= 0 && res.upper > 0)
                res.lower = res.upper * 1e-3;
            else
                if(res.lower <= 0 && res.upper <= 0 && founded)
                    *founded = false;
        }
        else if(sig == TSigDomain::sdNegative)
        {
            if(res.upper >= 0 && res.lower < 0)
                res.upper = res.lower * 1e-3;
            else
                if(res.lower >= 0 && res.upper >= 0)
                    *founded = false;
        }
        return res;
    }

    TGradientPreset TColorMapBase::Preset() const
    {
        return preset;
    }

    void TColorMapBase::SetPreset(TGradientPreset value)
    {
        preset = value;
    }

    double TColorMapBase::SelectTest(const TPointF &pos, bool onlySelectable)
    {
        if((onlySelectable && selectable == stNone) || mapData->IsEmpty())
            return -1;

        if(keyAxis.expired() || valAxis.expired())
            return -1;

        if(keyAxis.lock()->Contains(pos) && valAxis.lock()->Contains(pos) && mapData->KeyRange().Contains(PixelToKey(pos)))
        {
            bufferSelect = TDataSelection(TDataRange(0, 1));
            return plot->SelectionTolerance() - 1;
        }
        return -1;
    }

    size_t TColorMapBase::HeaderHeight() const
    {
        return headerHeight;
    }

    void TColorMapBase::SetHeaderHeight(size_t value)
    {
        headerHeight = value;
    }

    TScatterStyle::TScatterStyle(TScatterShape value, size_t size):shapeScat(value), sizeScat(size)
    {

    }

    void TScatterStyle::Draw(const TUPtrPainter &painter, const TVecPointF &values)
    {
        if(penScat.style != psNone)
            painter->SetPen(penScat);
        for(const auto& v : values)
        {
            const auto& x = v.x();
            const auto& y = v.y();
            double w = sizeScat / 2.;
            switch (shapeScat)
            {
                case ssNone:
                    break;
                case ssDot:
                {
                    painter->DrawLine(TPointF(x, y), TPointF(x + 0.0001, y));
                    break;
                }
                case ssCross:
                {
                    painter->DrawLine(TPointF(x - w, y - w), TPointF(x + w, y + w));
                    painter->DrawLine(TPointF(x - w, y + w), TPointF(x + w, y - w));
                    break;
                }
                case ssPlus:
                {
                    painter->DrawLine(TPointF(x - w, y), TPointF(x + w, y));
                    painter->DrawLine(TPointF(x, y + w), TPointF(x, y - w));
                    break;
                }
                case ssCircle:
                {
                    painter->DrawEllipse(TPointF(x, y), w, w);
                    break;
                }
                case ssDisc:
                {
                    TBrush b = painter->Brush();
                    painter->SetBrush(painter->Pen().color);
                    painter->DrawEllipse(TPointF(x, y), w, w);
                    painter->SetBrush(b);
                    break;
                }
                case ssSquare:
                {
                    painter->DrawRect(TRectF(x - w, y - w, sizeScat, sizeScat));
                    break;
                }
                case ssDiamond:
                {
                    TPointF lineArray[4] = {TPointF(x - w, y),
                                            TPointF(x, y - w),
                                            TPointF(x + w, y),
                                            TPointF(x, y + w)};
                    painter->DrawPolygon(lineArray, 4);
                    break;
                }
                case ssStar:
                {
                    painter->DrawLine(TPointF(x - w, y), TPointF(x + w, y));
                    painter->DrawLine(TPointF(x, y + w), TPointF(x, y - w));
                    painter->DrawLine(TPointF(x - w * 0.707, y - w * 0.707), TPointF(x + w * 0.707, y + w * 0.707));
                    painter->DrawLine(TPointF(x - w * 0.707, y + w * 0.707), TPointF(x + w * 0.707, y - w * 0.707));
                    break;
                }
                case ssTriangle:
                {
                    TPointF lineArray[3] = {TPointF(x - w, y + 0.755 * w),
                                            TPointF(x + w, y + 0.755 * w),
                                            TPointF(x, y - 0.977 * w)};
                    painter->DrawPolygon(lineArray, 3);
                    break;
                }
                case ssTriangleInverted:
                {
                    TPointF lineArray[3] = {TPointF(x - w, y - 0.755 * w),
                                            TPointF(x + w, y - 0.755 * w),
                                            TPointF(x, y + 0.977 * w)};
                    painter->DrawPolygon(lineArray, 3);
                    break;
                }
                case ssCrossSquare:
                {
                    painter->DrawRect(TRectF(x - w, y - w, sizeScat, sizeScat));
                    painter->DrawLine(TPointF(x - w, y - w), TPointF(x + w * 0.95, y + w * 0.95));
                    painter->DrawLine(TPointF(x - w, y + w * 0.95), TPointF(x + w * 0.95, y - w));
                    break;
                }
                case ssPlusSquare:
                {
                    painter->DrawRect(TRectF(x - w, y - w, sizeScat, sizeScat));
                    painter->DrawLine(TPointF(x - w, y), TPointF(x + w * 0.95, y));
                    painter->DrawLine(TPointF(x, y + w), TPointF(x, y - w));
                    break;
                }
                case ssCrossCircle:
                {
                    painter->DrawEllipse(TPointF(x, y), w, w);
                    painter->DrawLine(TPointF(x - w * 0.707, y - w * 0.707), TPointF(x + w * 0.670, y + w * 0.670));
                    painter->DrawLine(TPointF(x - w * 0.707, y + w * 0.670), TPointF(x + w * 0.670, y - w * 0.707));
                    break;
                }
                case ssPlusCircle:
                {
                    painter->DrawEllipse(TPointF(x, y), w, w);
                    painter->DrawLine(TPointF(x - w, y), TPointF(x + w, y));
                    painter->DrawLine(TPointF(x, y + w), TPointF(x, y - w));
                    break;
                }
                case ssPeace:
                {
                    painter->DrawEllipse(TPointF(x, y), w, w);
                    painter->DrawLine(TPointF(x, y - w), TPointF(x, y + w));
                    painter->DrawLine(TPointF(x, y), TPointF(x - w * 0.707, y + w * 0.707));
                    painter->DrawLine(TPointF(x, y), TPointF(x + w * 0.707, y + w * 0.707));
                    break;
                }
            }
        }
    }
}