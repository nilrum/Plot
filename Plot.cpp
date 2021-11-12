//
// Created by user on 03.06.2020.
//

#include "Plot.h"
#include "PlotItems.h"

namespace Plot {

    TMarginGroup::TMarginGroup(TOrientation orientation):orient(orientation)
    {

    }

    int TMarginGroup::Offset(TRawLayoutElement current) const
    {
        int curVal = 0;
        int maxVal = 0;
        for(const auto& it : elements)//пройдемся по всем элементам которые состоят в группе
        {
            int sum = 0;
            for(const auto& check : it.second)
            {
                if(check->MaxSize(orient) != INT_MAX)
                    sum += check->MaxSize(orient);
            }
            if(sum > maxVal) maxVal = sum;
            if(it.first == current)
                curVal = sum;
        }
        return maxVal - curVal;
    }

    void TMarginGroup::Add(TRawLayoutElement value, TPtrLayoutElement check)
    {
        if(check == nullptr)
        {
            elements[value] = TVecLayoutElements();
            return;
        }
        auto it = elements.find(value);
        if(it == elements.end())
            elements[value].push_back(check);
        else
        {
            auto& checks = it->second;
            for(const auto& c : checks)
                if(c == check) return;
            checks.push_back(check);
        }
    }

    void TMarginGroup::Del(TRawLayoutElement value)
    {
        auto it = elements.find(value);
        if(it != elements.end())
            elements.erase(it);
    }
//----------------------------------------------------------------------------------------------------------------------
TLayoutElement::TLayoutElement(TRawPlot plt, const TString& layerName): TLayerable(plt, layerName)
{

}

TLayoutElement::~TLayoutElement()
{
    for(size_t i = atLeft; i < atCount; i++)
        if(marginGroups[i])
        {
            marginGroups[i]->Del(this);
            marginGroups[i] = nullptr;
        }
}

void TLayoutElement::SetMinSize(const TSize &value)
{
    minSize = {TStateInt::Value(value.width()), TStateInt::Value(value.height())};
}

void TLayoutElement::SetMaxSize(const TSize &value)
{
    maxSize = {TStateInt::Value(value.width()), TStateInt::Value(value.height())};
}

void TLayoutElement::SetOutRect(const TRect &value)
{
    outRect = value;
}

void TLayoutElement::SetMargins(const TMargins &value)
{
    if(value != margins)
        margins = value;
}

const TPtrMarginGroup &TLayoutElement::MarginGroup(TAxisType type) const
{
    return marginGroups[type];
}

void TLayoutElement::SetMarginGroup(TAxisType type, const TPtrMarginGroup& value, const TPtrLayoutElement& check)
{
    if(marginGroups[type])
        marginGroups[type]->Del(this);
    marginGroups[type] = value;
    if(marginGroups[type])
        marginGroups[type]->Add(this, check);
}

double TLayoutElement::SelectTest(const TPointF &pos, bool onlySelectable)
{
    if(onlySelectable)
        return -1;
    if(outRect.contains(pos.toPoint()))
    {
        if(plot) return plot->SelectionTolerance() * 0.99;
    }
    return -1;
}

void TLayoutElement::SetFixedSize(const TSize &value)
{
    SetFixedWidth(value.width());
    SetFixedHeight(value.height());
}

void TLayoutElement::SetFixedWidth(int value)
{
    SetMinMaxWidth(value, value);
    isAutoMinMax.setWidth(false);
}

void TLayoutElement::SetFixedHeight(int value)
{
    SetMinMaxHeight(value, value);
    isAutoMinMax.setHeight(false);
}

void TLayoutElement::SetMinMaxWidth(int min, int max)
{
    minSize.setWidth(TStateInt::Value(min));
    maxSize.setWidth(TStateInt::Value(max));
}

void TLayoutElement::SetMinMaxHeight(int min, int max)
{
    minSize.setHeight(TStateInt::Value(min));
    maxSize.setHeight(TStateInt::Value(max));
}

bool TLayoutElement::IsFixedWidth() const
{
    return minSize.width().get() == maxSize.width().get();
}

bool TLayoutElement::IsFixedHeight() const
{
    return minSize.height().get() == maxSize.height().get();
}

void TLayoutElement::ResetFixedWidth()
{
    minSize.setWidth(TStateInt::Default(0));
    maxSize.setWidth(TStateInt::Default(INT_MAX));
    isAutoMinMax.setWidth(true);
}

void TLayoutElement::ResetFixedHeight()
{
    minSize.setHeight(TStateInt::Default(0));
    maxSize.setHeight(TStateInt::Default(INT_MAX));
    isAutoMinMax.setHeight(true);
}

void TLayoutElement::SetLayout(const TPtrLayout &value)
{
    parentLayerable = value;
    parentLayout = value;
}

TMinMaxSizeState TLayoutElement::CalcMinMaxSizes()
{
    return {minSize, maxSize};
}

void TLayoutElement::Update()
{
    CalcMinMaxSizes();
    CalcInnerRect();
    CalcSizes();
}

void TLayoutElement::CalcInnerRect()
{
    innerRect = outRect.adjusted(margins.left(), margins.top(), -margins.right(), -margins.bottom());

    if(marginGroups[atLeft])
        innerRect.setLeft(innerRect.left() + marginGroups[atLeft]->Offset(this));
    if(marginGroups[atTop])
        innerRect.setTop(innerRect.top() + marginGroups[atTop]->Offset(this));
    if(marginGroups[atRight])
        innerRect.setRight(innerRect.right() - marginGroups[atRight]->Offset(this));
    if(marginGroups[atBottom])
        innerRect.setBottom(innerRect.bottom() - marginGroups[atBottom]->Offset(this));
}

//----------------------------------------------------------------------------------------------------------------------
    TLayout::TLayout(TRawPlot plt, const TString& layerName):TLayoutElement(plt, layerName)
    {

    }

    void TLayout::UpdateSizeConstraints()
    {
        if(isAutoSize)
        {
            /*if(outRect.width() < minSize.width())
                outRect.setWidth(minSize.width());
            if(outRect.height() < minSize.height())
                outRect.setHeight(minSize.height());

            if(outRect.width() > maxSize.width())
                outRect.setWidth(maxSize.width());
            if(outRect.height() > maxSize.height())
                outRect.setHeight(maxSize.height());*/
            plot->UpdateSize();
        }
    }

    void TLayout::Clear()
    {
        for(int i = int(CountElements() - 1); i >= 0; --i)
            Remove(i);
    }

    bool TLayout::Remove(size_t index)
    {
        TPtrLayoutElement el = Take(index);
        return el != nullptr;
    }

    bool TLayout::Remove(TPtrLayoutElement const &value)
    {
        return Remove(IndexElement(value));
    }

    TPtrLayoutElement TLayout::Take(const TPtrLayoutElement &value)
    {
        return Take(IndexElement(value));
    }
//----------------------------------------------------------------------------------------------------------------------
    TLayoutStack::TLayoutStack(TRawPlot plt, TOrientation orientation, const TString &layerName): TLayoutGrid(plt, layerName)
    {
        if(orientation == orVert)
            SetMaxAddCols(1);
    }

    double TLayoutStack::Stretch(size_t index)
    {
        if(maxAddCols == 1)
        {
            if (index < rowStretches.size())
                return rowStretches[index];
        }
        else
        {
            if (index < colStretches.size())
                return colStretches[index];
        }
        return 0;
    }

    void TLayoutStack::SetStretch(size_t index, double value)
    {
        if(maxAddCols == 1)
        {
            if (index < rowStretches.size())
                rowStretches[index] = value;
        }
        else
        {
            if (index < colStretches.size())
                colStretches[index] = value;
        }
    }
//----------------------------------------------------------------------------------------------------------------------
    TLayoutGrid::TLayoutGrid(TRawPlot plt, const TString &layerName) : TLayout(plt, layerName)
    {

    }

    TMinMaxSizeState TLayoutGrid::CalcMinMaxSizes()
    {
        if(isAutoMinMax.width() == false && isAutoMinMax.height() == false)
            return {minSize, maxSize};
        TMinMaxSizeState res{};

        //Для grid layout необходимо найти
        //  для каждой строки необходимо найти (а именно пройтись по всем столбам строки)
        //      сумму минимальных ширин(sumMinWidth)
        //      максимальную из минимальных высот(maxMinHeight)
        //      сумму максимальных ширин(sumMaxWidth)
        //      минимальную из максимальных высот(minMaxHeight)
        //      если максимальная ширина или высота равна INT_MAX то результат INT_MAX
        for(auto r = 0; r < CountRow(); r++)
        {
            TStateInt sumMinWidth;                          //по умолчанию нет
            TStateInt maxMinHeight;

            TStateInt sumMaxWidth;
            TStateInt minMaxHeight;

            for(auto c = 0; c < CountCol(); c++)
            {
                if(HasElement(r, c) == false || Element(r, c)->RealVisibility() == false)
                    continue;

                auto [min, max] = Element(r, c)->CalcMinMaxSizes();

                sumMinWidth = sumMinWidth + min.width();            //находим сумму минимальных ширин строки

                if(maxMinHeight < min.height() && min.height().IsNull() == false)
                    maxMinHeight = min.height();                    //находим максимальную высоту из мин высот

                sumMaxWidth = sumMaxWidth + max.width();            //находим сумму максимальных ширин

                if (minMaxHeight > max.height() && max.height().IsNull() == false)                    //находим минимальную высоту из макс высот
                    minMaxHeight = max.height();

            }
            if(sumMinWidth > res.first.width() && sumMinWidth.IsNull() == false)
                res.first.setWidth(sumMinWidth);//находим максимальную сумму минимальных широт
            res.first.setHeight(res.first.height() + maxMinHeight);             //находим сумму максимальных минимумов строки

            if (res.second.width() > sumMaxWidth && sumMaxWidth.IsNull() == false || sumMaxWidth.IsDefault())
                res.second.setWidth(sumMaxWidth);//находим из всех строк самую минимальную макс ширину

            res.second.setHeight(res.second.height() + minMaxHeight);

        }
        if(isAutoMinMax.width() && isAutoMinMax.height())
        {
            minSize = res.first;
            maxSize = res.second;
        }
        else
        {
            if(isAutoMinMax.width())
            {
                minSize.setWidth(res.first.width());
                maxSize.setWidth(res.second.width());
            }
            else
            {
                int y  = 0;
            }
            if(isAutoMinMax.height())
            {
                minSize.setHeight(res.first.height());
                maxSize.setHeight(res.second.height());
            }
        }

        UpdateSizeConstraints();
        return {minSize, maxSize};
    }

    void TLayoutGrid::CalcSizes()
    {
        if(CountElements() == 0) return;

        TVecInt minColWidths, maxColWidths;
        TVecInt minRowHeights, maxRowHeights;

        CalcMinMaxSizes<std::greater<TStateInt>>(minColWidths, minRowHeights, 0);
        CalcMinMaxSizes<std::less<TStateInt>>(maxColWidths, maxRowHeights, INT_MAX);

        int totalRowSpacing = int(minRowHeights.size() - 1) * rowSpacing;
        int totalColSpacing = int(minColWidths.size() - 1) * colSpacing;

        TVecDouble colSizes = CalcSectionSizes(minColWidths, maxColWidths, colStretches,
                                               innerRect.width() - totalColSpacing);
        TVecDouble rowSizes = CalcSectionSizes(minRowHeights, maxRowHeights, rowStretches,
                                               innerRect.height() - totalRowSpacing);

        int yOffset = innerRect.top();
        for (size_t row = 0; row < rowSizes.size(); row++)
        {
            int xOffset = innerRect.left();
            for (size_t col = 0; col < colSizes.size(); col++)
            {
                const auto& el = elements[row][col];
                if (el && el->RealVisibility())
                {
                    el->SetOutRect(TRect(xOffset, yOffset, int(colSizes[col]), int(rowSizes[row])));
                    el->CalcInnerRect();
                    el->CalcSizes();
                }
                xOffset = xOffset + int(colSizes[col]) + colSpacing;
            }
            yOffset = yOffset + int(rowSizes[row]) + rowSpacing;
        }
    }

    void TLayoutGrid::SetRowStretch(size_t row, double value)
    {
        if(row < CountRow() && value > 0)
            rowStretches[row] = value;
    }

    void TLayoutGrid::SetColStretch(size_t col, double value)
    {
        if(col < CountRow() && value > 0)
            colStretches[col] = value;
    }

    void TLayoutGrid::SetRowStretches(const TVecDouble &value)
    {
        if(value.size() == rowStretches.size())
        {
            rowStretches = value;
            for(double& r : rowStretches)
                if(r <= 0.) r = 1.;
        }
    }

    void TLayoutGrid::SetColStretches(const TVecDouble &value)
    {
        if(value.size() == colStretches.size())
        {
            colStretches = value;
            for(double& r : colStretches)
                if(r <= 0.) r = 1.;
        }
    }

    void TLayoutGrid::SetRowSpacing(int value)
    {
        rowSpacing = value;
    }

    void TLayoutGrid::SetColSpacing(int value)
    {
        colSpacing = value;
    }

    size_t TLayoutGrid::CountElements() const
    {
        return CountRow() * CountCol();
    }

    TPtrLayoutElement TLayoutGrid::Element(size_t index) const
    {
        int row = 0;
        int col = 0;
        if(IndexToRowCol(index, row, col) == false) return TPtrLayoutElement();
        return elements[row][col];
    }

    TPtrLayoutElement TLayoutGrid::Take(size_t index)
    {
        int row = 0;
        int col = 0;
        if(IndexToRowCol(index, row, col) == false || elements[row][col] == nullptr)
            return TPtrLayoutElement();

        TPtrLayoutElement el = elements[row][col];
        elements[row][col] = nullptr;
        el->SetLayout(TPtrLayout());

        return el;
    }

    int TLayoutGrid::IndexElement(TPtrLayoutElement const &value)
    {
        for(size_t i = 0; i < CountElements(); ++i)
            if(Element(i) == value) return i;
        return -1;
    }

    bool TLayoutGrid::IndexToRowCol(size_t index, int &row, int &col) const
    {
        if(index >= CountElements()) return false;
        row = index % CountRow();
        col = index / CountRow();
        return true;
    }

    TPtrLayoutElement TLayoutGrid::Element(size_t row, size_t col) const
    {
        if(row  < elements.size())
        {
            if(col < elements.front().size())
                return elements[row][col];
        }
        return TPtrLayoutElement();
    }

    const TPtrLayoutElement & TLayoutGrid::AddElement(size_t row, size_t col, const TPtrLayoutElement &value)
    {
        if(HasElement(row, col) == false)
            ExpandTo(row + 1, col + 1);
        elements[row][col] = value;
        if(value)
            value->SetLayout(ThisShared<TLayout>());
        return elements[row][col];
    }

    const TPtrLayoutElement& TLayoutGrid::AddElement(const TPtrLayoutElement& value)
    {
        int col = 0; int row = 0;
        while(HasElement(row, col))
        {
            col++;
            if(col >= maxAddCols)
            {
                col = 0;
                row++;
            }
        }
        return AddElement(row, col, value);
    }

    bool TLayoutGrid::HasElement(size_t row, size_t col) const
    {
        if(row < CountRow() && col < CountCol())
            return elements[row][col] != nullptr;
        return false;
    }

    void TLayoutGrid::ExpandTo(size_t newRowCount, size_t newColCount)
    {
        int maxColumn = (newColCount > CountCol())? newColCount : CountCol();
        while(CountRow() < newRowCount)
        {
            elements.emplace_back(maxColumn);
            rowStretches.push_back(1.);
        }
        for(auto& row : elements)
            row.resize(maxColumn);
        colStretches.resize(maxColumn, 1.);
    }

    void TLayoutGrid::InsertRow(size_t row)
    {
        elements.insert(elements.begin() + row, TVecLayoutElements(CountCol()));
        rowStretches.insert(rowStretches.begin() + row, 1.);
    }

    void TLayoutGrid::InsertCol(size_t col)
    {
        if(col < CountCol())
        {
            for (auto &row : elements)
                row.insert(row.begin() + col, nullptr);
            colStretches.insert(colStretches.begin() + col, 1.);
        }
        else
        {
            for (auto &row : elements)
                row.resize(col + 1);
            colStretches.resize(col + 1, 1.);
        }

    }

    void TLayoutGrid::Simplify()
    {
        for(int row = CountRow() - 1; row >= 0; --row)
        {
            bool has = false;
            for(int col = 0; col < int(CountCol()); ++col)
                if(elements[row][col] != nullptr)
                {
                    has = true;
                    break;
                }
            if(has == false)
            {
                elements.erase(elements.begin() + row);
                rowStretches.erase(rowStretches.begin() + row);
            }
        }
        for(int col = CountCol() - 1; col >= 0; --col)
        {
            bool has = false;
            for(size_t row = 0; row < CountRow(); ++row)
                if(elements[row][col] != nullptr)
                {
                    has = true;
                    break;
                }
            if(has == false)
            {
                for(size_t row = 0; row < CountRow(); ++row)
                    elements[row].erase(elements[row].begin() + col);
                colStretches.erase(colStretches.begin() + col);
            }
        }
    }


    TVecDouble TLayout::CalcSectionSizes(const TVecInt &minSizes, const TVecInt &maxSizes, const TVecDouble &stretch,
                                         int totalSize)
    {
        TVecDouble res(stretch.size());
        if(res.size())
        {
            double sumStretch = 0.;
            for(const auto& s : stretch)
                sumStretch += s;

            TVecInt needCheck(res.size());//индексы ячеек которые надо проверить
            for(size_t i = 0; i < res.size(); i++)
            {
                res[i] = totalSize * stretch[i] / sumStretch;
                needCheck[i] = i;
            }
            double freeSize = 0.;
            while(needCheck.size())
            {
                size_t i = 0;
                while (i < needCheck.size())
                {
                    int index = needCheck[i];
                     if (res[index] < minSizes[index])
                    {
                        double delta = minSizes[index] - res[index];
                        res[index] = minSizes[index];
                        freeSize = freeSize - delta;
                        if(freeSize <= 0)//если пустого места не осталось, то удаляемся
                            needCheck.erase(needCheck.begin() + i);
                    }
                    else if (res[index] > maxSizes[index])
                    {
                        double delta = res[index] - maxSizes[index];
                        res[index] = maxSizes[index];
                        freeSize += delta;
                        needCheck.erase(needCheck.begin() + i);
                    }
                    else
                        ++i;
                }
                if (freeSize != 0 && needCheck.size())
                {
                    double localStretch = 0;

                    for (const auto &i : needCheck)
                        localStretch += stretch[i];

                    for (const auto &i : needCheck)
                        res[i] = res[i] + freeSize * stretch[i] / localStretch;
                    freeSize = 0;
                }
                else
                    break;
            }

        }
        return res;
    }


//----------------------------------------------------------------------------------------------------------------------
    TAxisRect::TAxisRect(TRawPlot plt, bool isDefault):TLayoutGrid(plt, "main"), isDefaultInit(isDefault)
    {
        colSpacing = 0;
        rowSpacing = 0;
        //margins = TMargins(10, 10, 10, 10);

        axisLay[atTop] = plot->CreateLayout<TLayoutGrid>();
        axisLay[atLeft] = plot->CreateLayout<TLayoutGrid>();
        axisLay[atRight] = plot->CreateLayout<TLayoutGrid>();
        axisLay[atBottom] = plot->CreateLayout<TLayoutGrid>();

        //подстраивать свои мин макс размеры под мин макс размеры вложенных элементов
        axisLay[atTop]->SetMaxAddCols(1);
        axisLay[atBottom]->SetMaxAddCols(1);
    }

    void TAxisRect::SetThisWeak(TWPtrLayoutElement value)
    {
        TLayoutElement::SetThisWeak(value);

        AddElement(0, 1, axisLay[atTop]);
        AddElement(1, 0, axisLay[atLeft]);
        AddElement(1, 2, axisLay[atRight]);
        AddElement(2, 1, axisLay[atBottom]);

        if(isDefaultInit)
        {
            auto leftAxis = AddAxis(atLeft);
            auto bottomAxis = AddAxis(atBottom);
            AddDragAxes(bottomAxis, leftAxis);
            AddZoomAxes(bottomAxis, leftAxis);
            grid = std::make_unique<TGridAxes>(bottomAxis, leftAxis);
        }
    }

    void TAxisRect::SetAxisRectGroup(TAxisType type, const TPtrMarginGroup& value)
    {
        SetMarginGroup(type, value, axisLay[type]);
    }

    const TPtrAxis& TAxisRect::Axis(TAxisType type, size_t index)
    {
        if(index < axes[type].size())
            return axes[type][index];
        return Single<TPtrAxis>();
    }

    const TPtrAxis& TAxisRect::AddAxis(TAxisType type)
    {
        auto ax = TPtrAxis(new TAxis(type, plot));
        axes[type].emplace_back(ax);
        axisLay[type]->AddElement(ax);
        ax->axisRect = ThisShared<TAxisRect>();
        return axes[type].back();
    }

    void TAxisRect::RemoveAxis(const TPtrAxis& value)
    {
        if(value == nullptr) return;
        if(RemoveFrom(axes[value->TypeAxis()], value))
            axisLay[value->TypeAxis()]->Remove(value);
    }

    size_t TAxisRect::CountAxis(TAxisType type) const
    {
        return axes[type].size();
    }

    void TAxisRect::MousePress(TMouseInfo &info)
    {
        if(info.IsLeftButton() && info.IsMoving())
        {
            if(isHorzResizable)
            {
                if (outRect.right() - info.pos.x() <= 10)
                {
                    dragSize = outRect.width();
                    plot->SetCursor(ctHDrag);
                    return;
                }
            }

            if(isVertResizable)
            {
                if (outRect.right() - info.pos.y() <= 10)
                {
                    dragSize = outRect.height();
                    plot->SetCursor(ctVDrag);
                    return;
                }
            }
            dragSize = 0;

            isDragging = true;
            for(const auto& ax : vertDragAxes)
                if(ax.expired() == false)
                {
                    auto lock = ax.lock();
                    if(lock->IsDraggable())
                        lock->SaveDrag();
                }
            for(const auto& ax : horzDragAxes)
                if(ax.expired() == false)
                {
                    auto lock = ax.lock();
                    if(lock->IsDraggable())
                        lock->SaveDrag();
                }
        }
    }

    void TAxisRect::MouseMove(TMouseInfo &info, const TPoint &startPos)
    {
        if(info.IsMoving() == false)
        {
            info.Ignore();
            return;
        }
        if(dragSize)
        {
            if(isHorzResizable)
            {
                //если еще не установили фикс размеры, то берем от первонач. значения
                //иначе от текущего фиксированного
                double curWidth = isAutoMinMax.width() ? dragSize : minSize.width().get();
                double newWidth = dragSize + (info.pos.x() - startPos.x());
                const auto& al = axes[atTop];
                for(size_t i = 1; i < al.size(); i++)
                    al[i]->SetStep(al[i]->Step() * (curWidth / newWidth));

                SetFixedWidth(newWidth);

            }
            else
            {
                SetFixedHeight(dragSize + (info.pos.y() - startPos.y()));
            }

            plot->NativePaintLater();
            plot->Replot();
            return;
        }
        if(isDragging)
        {
            for(const auto& ax : vertDragAxes)
                if(ax.expired() == false)
                {
                    auto lock = ax.lock();
                    if(lock->IsDraggable())
                        lock->SetDrag(startPos, info.pos);
                }

            for(const auto& ax : horzDragAxes)
                if(ax.expired() == false)
                {
                    auto lock = ax.lock();
                    if(lock->IsDraggable())
                        lock->SetDrag(startPos, info.pos);
                }
            plot->Replot();
        }
    }

    void TAxisRect::MouseUp(TMouseInfo &info, const TPoint &startPos)
    {
        isDragging = false;
        if(dragSize)
        {
            plot->SetCursor(ctDefault);
            const auto &al = axes[atTop];
            for (size_t i = 1; i < al.size(); i++)
                if (al[i]->IsRoundRange())
                    al[i]->SetStep(std::round(al[i]->Step()));
            dragSize = 0;
            plot->Replot();
        }
    }

    void TAxisRect::MouseWheel(TMouseInfoWheel &info)
    {
        double wheelSteps = info.delta / 120.;
        double factor = std::pow(0.85, wheelSteps);//TODO 0.85 - zoomFactor

        for(const auto& ax : vertZoomAxes)
            if(ax.expired() == false)
            {
                auto lock = ax.lock();
                if(lock->IsScalable())
                {
                    double center = (lock->IsDraggable()) ? lock->PixelToCoord(info.pos.y()) : 0;
                    lock->ScaleRange(factor, center);
                }
            }

        for(const auto& ax : horzZoomAxes)
            if(ax.expired() == false)
            {
                auto lock = ax.lock();
                if(lock->IsScalable())
                {
                    double center = (lock->IsDraggable()) ? lock->PixelToCoord(info.pos.x()) : 0;
                    lock->ScaleRange(factor, center);
                }
            }
        plot->Replot();
    }

    void TAxisRect::ClearDragAxes()
    {
        horzDragAxes.clear();
        vertDragAxes.clear();
    }

    void TAxisRect::ClearZoomAxes()
    {
        horzZoomAxes.clear();
        vertZoomAxes.clear();
    }

    void TAxisRect::AddDragAxes(const TPtrAxis &horz, const TPtrAxis &vert)
    {
        if(horz)
            horzDragAxes.emplace_back(horz);
        if(vert)
            vertDragAxes.emplace_back(vert);
    }

    void TAxisRect::AddZoomAxes(const TPtrAxis &horz, const TPtrAxis &vert)
    {
        if(horz)
            horzZoomAxes.emplace_back(horz);
        if(vert)
            vertZoomAxes.emplace_back(vert);
    }

    TRect TAxisRect::CentralRect() const
    {
        if(HasElement(1, 1)) return Element(1, 1)->OutRect();
        return InnerRect();
    }

    bool TAxisRect::IsHorzResizable() const
    {
        return isHorzResizable;
    }

    void TAxisRect::SetIsHorzResizable(bool value)
    {
        isHorzResizable = value;
    }

    bool TAxisRect::IsVertResizable() const
    {
        return isVertResizable;
    }

    void TAxisRect::SetIsVertResizable(bool value)
    {
        isVertResizable = value;
    }

//----------------------------------------------------------------------------------------------------------------------
    TAxis::TAxis(TAxisType typeAxis, TRawPlot plt): TLayoutElement(plt, "axes"),
                                                      type(typeAxis), orient(Orientation(type))
    {
        if(type == atLeft || type == atRight)
            SetFixedWidth(40);
        else
            SetFixedHeight(40);
    }

    void TAxis::CalcSizes()
    {
        if(isFixedPix)//если шаг в пикселях фиксированный, то автоматически пересчитаем максимальное значение шкалы
            range.upper = range.lower + step * CalcCountFixed();

        InitTickVectors();
        CalcTickVectors();
        if(axisBuffer == nullptr && plot && isUseBuffer)
            axisBuffer = plot->CreatePaintBuffer(outRect.size());
        if(axisBuffer)
            axisBuffer->SetSize(outRect.size());
    }

    void TAxis::Draw(const TUPtrPainter & painter)
    {
        if(outRect.width() == 0 || outRect.height() == 0) return;
        if(axisBuffer == nullptr)
        {
            DrawAxis(painter);
            return;
        }
        if(axisBuffer->Invalidate())
        {
            auto bufferPainter = axisBuffer->StartPaint();
            bufferPainter->Offset(-outRect.left(), -outRect.top());
            DrawAxis(bufferPainter);
            axisBuffer->StopPaint();
            axisBuffer->SetInvalidated(false);
        }
        axisBuffer->Draw(painter, outRect.left(), outRect.top());
    }

    void TAxis::DrawAxis(const TUPtrPainter &painter)
    {
        painter->FillRect(outRect, TConsts::WhiteColor());

        TPointF corr = (type != atTop) ? TPointF(0, 0) : TPointF(0, -1);
        int coef = (type == atLeft || type == atTop) ? -1 : 1;
        TPointF begin = corr, end = corr;
        switch (type)
        {
            case atLeft:    begin += innerRect.bottomRight();   end += innerRect.topRight();    break;
            case atRight:   begin += innerRect.bottomLeft();    end += innerRect.topLeft();     break;

            case atTop:     begin += innerRect.bottomLeft();    end += innerRect.bottomRight(); break;
            case atBottom:  begin += innerRect.topLeft();       end += innerRect.topRight();    break;
        }
        painter->SetPen((isSelected ? selPenAxis : penAxis));
        painter->DrawLine(begin, end);

        if(tickPositions.size())
        {
            if(orient == orVert)
            {
                for (size_t i = 0; i < tickPositions.size(); i++)
                {
                    const double &pos = tickPositions[i];
                    painter->DrawLine(TPointF(begin.x(), pos), TPointF(begin.x() + tickLen * coef, pos));
                }
                for (size_t i = 0; i < subTickPositions.size(); i++)
                {
                    const double &pos = subTickPositions[i];
                    painter->DrawLine(TPointF(begin.x(), pos), TPointF(begin.x() + subTickLen * coef, pos));
                }
            }
            else
            {
                for (size_t i = 0; i < tickPositions.size(); i++)
                {
                    const double &pos = tickPositions[i];
                    painter->DrawLine(TPointF(pos, begin.y()), TPointF(pos, begin.y() + tickLen * coef));
                }
                for (size_t i = 0; i < subTickPositions.size(); i++)
                {
                    const double &pos = subTickPositions[i];
                    painter->DrawLine(TPointF(pos, begin.y()), TPointF(pos, begin.y() + subTickLen * coef));
                }
            }

            if(tickLabels.empty() == false)
            {
                painter->SetFont(fontTicks);
                size_t countLabels = std::min(tickLabels.size(), tickPositions.size());
                switch (type)
                {
                    case atLeft:
                        for (size_t i = 0; i < countLabels; i++)
                            painter->DrawText(TPointF(begin.x() + (tickLen + tickLabelOffset) * coef, tickPositions[i]),
                                              tickLabels[i], TAlignText::End, alignText);
                        break;

                        case atRight:
                            for (size_t i = 0; i < countLabels; i++)
                                painter->DrawText(TPointF(begin.x() + (tickLen + tickLabelOffset) * coef, tickPositions[i]),
                                                  tickLabels[i], TAlignText::Begin, alignText);
                            break;

                            case atTop:
                                for (size_t i = 0; i < countLabels; i++)
                                    painter->DrawText(TPointF(tickPositions[i], begin.y() + (tickLen + tickLabelOffset) * coef ),
                                                      tickLabels[i], alignText, TAlignText::End);
                                break;

                                case atBottom:
                                    for (size_t i = 0; i < countLabels; i++)
                                        painter->DrawText(TPointF(tickPositions[i], begin.y() + (tickLen + tickLabelOffset) * coef ),
                                                          tickLabels[i], alignText, TAlignText::Begin);
                                    break;
                }
            }
        }
        if(label.empty() == false)
        {
            painter->SetFont(fontLabel);
            int offset = (tickLen + tickLabelOffset + tickLabelHeight + labelOffset) * coef;
            switch (type)
            {
                case atLeft:
                    painter->DrawText(TPointF(begin.x() + offset, begin.y() + (end.y() - begin.y()) / 2.), label.c_str(), TAlignText::Begin, TAlignText::Begin, -90);
                    break;
                case atTop:
                    painter->DrawText(TPointF(begin.x() + (end.x() - begin.x()) / 2., begin.y() + offset), label.c_str(), TAlignText::Center, TAlignText::End);
                    break;
                case atRight:
                    painter->DrawText(TPointF(begin.x() + offset, begin.y() + (end.y() - begin.y()) / 2.), label.c_str(), TAlignText::Begin, TAlignText::Begin, 90);
                    break;
                case atBottom:
                    painter->DrawText(TPointF(begin.x() + (end.x() - begin.x()) / 2., begin.y() + offset), label.c_str(), TAlignText::Center);
                    break;
            }

        }
        OnExtPaint(painter);
    }

    void TAxis::SetScaleType(TScaleType value)
    {
        if(value != scaleType)
        {
            scaleType = value;
            if(scaleType == stLog)
                range = range.CheckForLogScale();
            Invalidate();
        }
    }

    void TAxis::SetLabel(const TString& value)
    {
        if(value != label)
        {
            label = value;
            Invalidate();
        }
    }

    void TAxis::SetRange(const TRange &value, bool andStep)
    {
        if(range.lower == value.lower && range.upper == value.upper)
            return;
        if(TRange::ValidRange(value) == false)
            return;
        if(andStep)
            step = step / range.Size();
        if(scaleType == stLinear)
            range = value.CheckForLinScale();
        else
            range = value.CheckForLogScale();
        if(isRoundRange)
            range.lower = std::floor(range.lower);//округлим минимум в меньшую сторону
        if(andStep)
        {
            step = range.Size() * step;
            if(isRoundRange)
                step = std::ceil(step);//округлим шаг в большую сторону
        }
        Invalidate();
    }

    void TAxis::SetRange(double lower, double upper)
    {
        SetRange(TRange(lower, upper));
    }

    void TAxis::SetRangeLower(double value)
    {
        SetRange(TRange(value, range.upper));
    }

    void TAxis::SetRangeUpper(double value)
    {
        SetRange(TRange(range.lower, value));
    }

    void TAxis::SetRangeSize(double value)
    {
        SetRange(TRange(range.lower, range.lower + value));
    }

    void TAxis::SetRange(double pos, TAlignSet align)
    {
        switch(align)
        {
            case asLower: SetRange(pos, pos + range.Size()); break;
            case asCenter: SetRange(pos - range.Size() / 2., pos + range.Size() / 2.); break;
            case asUpper: SetRange(pos - range.Size(), pos); break;
        }
    }

    void TAxis::SetStep(double value)
    {
        step = value;
        if(isFixedPix)//если шаг фиксированный, то автоматически пересчитаем upper
            SetRange(range.lower, range.lower + value * CalcCountFixed());
        Invalidate();
    }

    void TAxis::SetIsRangeReversed(bool value)
    {
        isRangeReversed = value;
        Invalidate();
    }

    void TAxis::SetIsTicks(bool value)
    {
        if(value != isTicks)
        {
            isTicks = value;
            Invalidate();
        }
    }

    void TAxis::SetIsTickLabels(bool value)
    {
        if(value != isTickLabels)
        {
            isTickLabels = value;
            Invalidate();
        }
    }

    void TAxis::SetIsSubTicks(bool value)
    {
        isSubTicks = value;
        Invalidate();
    }

    void TAxis::SetColorAxis(TColor value)
    {
        penAxis.color = value;
        Invalidate();
    }

    void TAxis::SetTickLen(int value)
    {
        tickLen = value;
        Invalidate();
    }

    void TAxis::SetSubTickLen(int value)
    {
        subTickLen = value;
        Invalidate();
    }

    void TAxis::SetSubTickCount(int value)
    {
        subTickCount = value;
        Invalidate();
    }

    void TAxis::InitTickVectors()
    {
        if(plot == nullptr) return;
        if(isTicks && range.Size() > 0)
            CalculateTicker(tickVals, subTickVals, tickLabels);
        else
            if(tickVals.size())
            {
                tickVals.clear();
                subTickVals.clear();
                tickLabels.clear();
            }
    }

    void TAxis::CalcTickVectors()
    {
        if(tickVals.size())
        {
            tickPositions.resize(tickVals.size());
            for(size_t i = 0; i < tickVals.size(); i++)
                tickPositions[i] = CoordToPixel(tickVals[i]);
        }
        else
            tickPositions.clear();
        if(subTickVals.size())
        {
            subTickPositions.resize(subTickVals.size());
            for(size_t i = 0; i < subTickVals.size(); i++)
                subTickPositions[i] = CoordToPixel(subTickVals[i]);
        }
        else
            subTickPositions.clear();
    }

    double TAxis::CoordToPixel(double value)
    {
        if(orient == orVert)
        {
            if(scaleType == stLinear)
            {
                if(isRangeReversed == false)
                    return InTop() +    (value - range.lower) / range.Size() * InHeight();
                else
                    return InBottom() - (value - range.lower) / range.Size() * InHeight();
            }
            else
            {
                if(value > 0)
                {
                    if (isRangeReversed == false)
                        return InTop() +
                               std::log(value / range.lower) / std::log(range.upper / range.lower) * InHeight();
                    else
                        return InBottom() -
                               std::log(value / range.lower) / std::log(range.upper / range.lower) * InHeight();
                }
                else
                {
                    if (isRangeReversed == false)
                        return InTop() - 100;
                    else
                        return InBottom() + 100;
                }
            }
        }
        else
        {
            if(scaleType == stLinear)
            {
                if(isRangeReversed == false)
                    return InLeft() +  (value - range.lower) / range.Size() * InWidth();
                else
                    return InRight() - (value - range.lower) / range.Size() * InWidth();
            }
            else
            {
                if(isRangeReversed == false)
                    return InLeft() +  std::log(value / range.lower) / std::log(range.upper / range.lower) * InWidth();
                else
                    return InRight() - std::log(value / range.lower) / std::log(range.upper / range.lower) * InWidth();
            }
        }
    }

    double TAxis::PixelToCoord(double value)
    {
        if(orient == orVert)
        {
            if(scaleType == stLinear)
            {
                if(isRangeReversed == false)
                    return  range.lower + ( value - InTop() ) / InHeight() * range.Size();
                else
                    return  range.lower + ( InBottom() - value ) / InHeight() * range.Size();
            }
            else
            {
                if(isRangeReversed == false)
                    return std::pow(range.upper / range.lower, (value - InTop()) / InHeight()) * range.lower;
                else
                    return std::pow(range.upper / range.lower, (InBottom() - value) / InHeight()) * range.lower;
            }
        }
        else
        {
            if(scaleType == stLinear)
            {
                if(isRangeReversed == false)
                    return  range.lower + ( value - InLeft() ) / InWidth() * range.Size();
                else
                    return  range.lower + ( InRight() - value ) / InWidth() * range.Size();
            }
            else
            {
                if(isRangeReversed == false)
                    return std::pow(range.upper / range.lower, (value - InLeft()) / InWidth()) * range.lower;
                else
                    return std::pow(range.upper / range.lower, (InRight() - value) / InWidth()) * range.lower;
            }
        }
    }

    bool TAxis::Contains(const TPointF &value) const
    {
        if(orient == orVert)
            return value.y() >= InTop() && value.y() <= InBottom();
        else
            return value.x() >= InLeft() && value.x() <= InRight();
    }

    double TAxis::SelectTest(const TPointF &pos, bool onlySelectable)
    {
        if(plot == nullptr) return -1;
        if(InnerRect().contains(pos.toPoint()) == false || isSelectable == false)
            return -1;
        return plot->SelectionTolerance() * 0.99;
    }

    bool TAxis::SelectEvent(TMouseInfo &info, bool additive)
    {
        return SetIsSelected(additive ? !isSelected : true);
    }

    bool TAxis::DeselectEvent()
    {
        return SetIsSelected(false);
    }

    bool TAxis::SetIsSelected(bool value)
    {
        if(isSelected != value) Invalidate();
        return SelectionChanged(isSelected, value);
    }

    void TAxis::SetDrag(const TPoint &startPos, const TPoint &curPos)
    {
        if(scaleType == stLinear)
        {
            double diff = 0;
            if(orient == orVert) diff = PixelToCoord(startPos.y()) - PixelToCoord(curPos.y());
            else diff = PixelToCoord(startPos.x()) - PixelToCoord(curPos.x());

            SetRange(bufferDrag.lower + diff, asLower);
        }
        else
        {
            //TODO log scale
        }
        Invalidate();
    }

    void TAxis::ScaleRange(double factor, double center)
    {
        if(scaleType == stLinear)
        {
            TRange newRange;

            newRange.lower = (range.lower - center) * factor + center;
            newRange.upper = (range.upper - center) * factor + center;

            SetRange(newRange, true);
        }
        else
        {
            //TODO log scale
        }
        Invalidate();
    }

    void TAxis::SetIsFixedPix(bool value)
    {
        isFixedPix = value;
    }

    double TAxis::CalcCountFixed() const
    {
        return (orient == orVert ? InHeight() : InWidth()) / FixPix();
    }

    double TAxis::Step() const
    {
        return step;
    }

    void TAxis::SetIsUseBuffer(bool value)
    {
        isUseBuffer = value;
        if(isUseBuffer == false && axisBuffer)
            axisBuffer.reset();
    }

    void TAxis::SetAlignText(TAlignText value)
    {
        alignText = value;
    }

    void TAxis::CalculateTicker(TVecDouble &ticks, TVecDouble &subTicks, TVecString &labels)
    {
        ticks = CalcTicks(step);
        //расчитываем subTicks до удаления лишних ticks для полного заполнения
        if(isSubTicks) subTicks = CalcSubTicks(ticks);
        //удаляем метки выходящие за размеры range
        TrimTicks(ticks, range);
        TrimTicks(subTicks, range);
        //получаем строки меток уже проверенные на границы range
        if(isTickLabels) labels = CalcLabels(ticks);
    }

    TVecDouble TAxis::CalcTicks(double tickStep)
    {
        const TRange& r = range;
        TVecDouble res;
        if(scaleType == stLinear)
        {
            if(isRoundTicks == false)
            {
                res.resize(int(r.Size() / tickStep) + 2);
                for (size_t i = 0; i < res.size(); i++)
                    res[i] = r.lower + i * tickStep;
            }
            else
            {
                auto first = std::floor(range.lower / tickStep);
                auto last = std::ceil(range.upper / tickStep);
                auto tickCount = last - first + 1;
                res.resize(tickCount < 0 ? 0 : tickCount);
                for(auto i = 0; i < res.size(); i++)
                    res[i] = (first + i) * tickStep;
            }

        }
        else
        {
            double low = r.lower;
            while(low <= r.upper)
            {
                res.push_back(low);
                low *= 10.;
            }
        }
        return res;
    }

    TVecDouble TAxis::CalcSubTicks(const TVecDouble &ticks)
    {
        if(ticks.empty()) return TVecDouble();
        TVecDouble res;
        res.reserve((ticks.size() - 1) * subTickCount);
        if(scaleType == stLinear)
            for(size_t i = 1; i < ticks.size(); i++)
            {
                double step = (ticks[i] - ticks[i - 1]) / double(subTickCount + 1);
                for(size_t j = 1; j <= subTickCount; j++)
                    res.emplace_back(ticks[i - 1] + j * step);
            }
        else
            for(size_t i = 1; i < ticks.size(); i++)
            {
                double a = ticks[i];
                double b = ticks[i - 1];
                a = std::log(a);
                b = std::log(b);
                a = a - b;
                a = a / double(subTickCount + 1);

                double step = (std::log(ticks[i]) - std::log(ticks[i - 1])) / double(subTickCount + 1);
                for(size_t j = 1; j <= subTickCount; j++)
                    res.emplace_back(std::exp(std::log(ticks[i - 1]) + j * step));
            }
        return res;
    }

    TVecString TAxis::CalcLabels(const TVecDouble &ticks)
    {
        TVecString res(ticks.size());
        if(scaleType == stLinear)
        {
            double delta = 0;
            if (ticks.size() > 1)
                delta = ticks[1] - ticks[0];
            TFormatDouble frmt(countAfterPoint, 4);
            for (size_t i = 0; i < ticks.size(); i++)
                res[i] = frmt.Format(ticks[i], delta);
        }
        else
        {
            TFormatDouble frmt(3);
            for (size_t i = 0; i < ticks.size(); i++)
                res[i] = frmt.Format(ticks[i], ticks[i]);
        }
        return res;
    }

    void TAxis::TrimTicks(TVecDouble &value, const TRange& range)
    {
        //удаляем тики, что меньше минимума
        while (value.size())
        {
            if(value.front() < range.lower)
                value.erase(value.begin());
            else
                break;
        }
        //удаляем тики, что больше максимума
        while (value.size())
            if(value.back() > range.upper)
                value.pop_back();
            else
                break;
    }

    void TAxis::SetIsRoundTicks(bool value)
    {
        isRoundTicks = value;
    }

    const TFont &TAxis::FontLabel() const
    {
        return fontLabel;
    }

    void TAxis::SetFontLabel(const TFont &value)
    {
        fontLabel = value;
    }

    const TFont &TAxis::FontTicks() const
    {
        return fontTicks;
    }

    void TAxis::SetFontTicks(const TFont &value)
    {
        fontTicks = value;
    }

    void TAxis::SetIsRoundRange(bool value)
    {
        isRoundRange = value;
    }


//----------------------------------------------------------------------------------------------------------------------
    TPlot::TPlot(bool isDefault)
    {
        TString layNames[] = {"grid", "axes", "main", "legend", "overlay"};
        for(const auto& name : layNames)
            AddLayer(name);
        layers.back()->SetMode(TLayerMode::Buffered);

        currentLayer = layers[2];//main layout

        plotLayout = CreateLayout<TLayoutGrid>();
        mainLayout = CreateLayout<TLayoutGrid>();
        plotLayout->AddElement(1, 0, mainLayout);
        if(isDefault)
            AddAxisRect(isDefault);
    }

    TPlot::~TPlot()
    {
        layers.clear();
        buffers.clear();
    }

    size_t TPlot::CountLayers() const
    {
        return layers.size();
    }

    const TPtrLayer &TPlot::Layer(size_t index) const
    {
        if(index < layers.size())
            return layers[index];
        return Single<TPtrLayer>();
    }

    const TPtrLayer &TPlot::Layer(const TString &nameLayer) const
    {
        if(nameLayer.empty())
            return Single<TPtrLayer>();

        for(const auto& l : layers )
            if(l->Name() == nameLayer)
                return l;
        return Single<TPtrLayer>();
    }

    const TPtrLayer &TPlot::AddLayer(const TString &nameLayer)
    {
        layers.emplace_back(new TLayer(this, nameLayer));
        return layers.back();
    }

    void TPlot::MoveLayerAfter(const TString &moveLayer, const TString &afterLayer)
    {
        int move = -1;
        auto after = -1;
        int i = 0;
        for(const auto& l : layers)
        {
            if(l->Name() == moveLayer)
                move = i;
            if(l->Name() == afterLayer)
                move = i;
            i++;
        }
        if(move == -1 || after == -1 || move == after) return;
        std::swap(layers[move], layers[after]);
    }

    const TPtrLayer &TPlot::CurrentLayer() const
    {
        return currentLayer;
    }

    void TPlot::SetViewport(const TRect &value)
    {
        viewport = value;
        plotLayout->SetOutRect(viewport);
    }

    void TPlot::Replot()
    {
        if(replotting || isInitState != 0) return;
        replotting = true;
        plotLayout->Update();
        InitPaintBuffers();
        for(const auto& lay : layers)
            lay->DrawToBuffer();
        for(const auto& buffer : buffers)
            buffer->SetInvalidated(false);
        //TODO need repaint
        NativePaintLater();
        replotting = false;
    }

    void TPlot::InitPaintBuffers()
    {
        int indexBuffer = -1;
        IncrIndexCheck(indexBuffer);
        for(size_t i = 0; i < layers.size(); i++)
        {
            auto& lay = layers[i];
            if(lay->Mode() == TLayerMode::Logical)
                lay->SetBuffer(buffers[indexBuffer]);
            else
            {
                IncrIndexCheck(indexBuffer);
                lay->SetBuffer(buffers[indexBuffer]);
                if(i < layers.size() - 1 && layers[i + 1]->Mode() == TLayerMode::Logical )
                    IncrIndexCheck(indexBuffer);
            }
        }

        while(int(buffers.size()) > indexBuffer + 1)
            buffers.pop_back();

        for(const TPtrPaintBuffer& buf : buffers)
        {
            buf->SetSize(viewport.size());
            buf->Clear(TConsts::TransparentColor());
            buf->SetInvalidated(true);
        }
    }

    bool TPlot::HasInvalidateBuffers()
    {
        for(const TPtrPaintBuffer& buf : buffers)
            if(buf->Invalidate())
                return true;
        return false;
    }

    void TPlot::PlotPaint()
    {
        PlotPaint(CreatePainter());
    }

    void TPlot::PlotPaint(const TUPtrPainter& painter)
    {
        if(painter->IsActive())
        {
            painter->FillRect(viewport, background);
            for(const TPtrPaintBuffer& buf : buffers)
                buf->Draw(painter, 0, 0);
        }
    }

    const TPtrPlottable& TPlot::AddPlottable(const TPtrPlottable& value)
    {
        plottables.emplace_back(value);
        return plottables.back();
    }

    bool TPlot::DelPlottable(const TPtrPlottable& value)
    {
        return RemoveVal(plottables, value);
    }

    size_t TPlot::CountPlottables() const
    {
        return plottables.size();
    }

    const TPtrPlottable &TPlot::Plottable(size_t index) const
    {
        if(index < plottables.size())
            return plottables[index];
        return Single<TPtrPlottable>();
    }

    TPtrPlottable TPlot::FindPlottable(const TString &value)
    {
        for(const auto& p : plottables)
            if(p->PlottableName() == value)
                return p;
        return TPtrPlottable();
    }

    int ManhattanLength(const TPoint& value)
    {
        return std::abs(value.x()) + std::abs(value.y());
    }

    void TPlot::MousePress(TMouseInfo &info)
    {
        mouseHasMoved = false;
        mousePressPos = info.pos;

        TVecLayerable candidates = FindLayerableList(mousePressPos, false);
        if (candidates.empty() == false)
            signalLayerable.Set(candidates.front());

        for (size_t i = 0; i < candidates.size(); i++)
        {
            info.Accept();
            candidates[i]->MousePress(info);
            if (info.IsAccepted())
            {
                signalLayerable.Set(candidates[i]);
                break;
            }
        }
        OnMousePress(info);
    }

    void TPlot::MouseMove(TMouseInfo &info)
    {
        if (mouseHasMoved == false && ManhattanLength(mousePressPos - info.pos) > 3)
            mouseHasMoved = true;
        if (signalLayerable.IsEmpty() == false)
        {
            info.Accept();
            signalLayerable->MouseMove(info, mousePressPos);
        }
        OnMouseMove(info);
    }

    void TPlot::MouseUp(TMouseInfo &info)
    {
        mouseUpPos = info.pos;
        if(mouseHasMoved == false)
        {
            if(info.IsLeftButton())
                MouseSelect(info);
        }

        if(signalLayerable.IsEmpty() == false)
        {
            signalLayerable->MouseUp(info, mousePressPos);
            signalLayerable.Clear();
            OnUserChanged();
        }
        OnMouseUp(info);
    }

    void TPlot::MouseDblClick(TMouseInfo &info)
    {
        OnMouseDblClick(info);
    }

    void TPlot::MouseWheel(TMouseInfoWheel &info)
    {
        if(info.IsMoving() == false) return;
        TVecLayerable candidates = FindLayerableList(info.pos, false);
        for(const auto& can : candidates)
        {
            info.Accept();
            can->MouseWheel(info);
            if(info.IsAccepted())
            {
                OnUserChanged();
                break;
            }
        }
    }

    void TPlot::MouseSelect(TMouseInfo &info)
    {
        TRawLayerable clicked = FindLayerable(mousePressPos, true);
        bool selStateChanged = false;
        bool additive = info.IsMultiSelect();//TODO check multiSelect
        if(additive == false)
        {
            for(const auto& lay : layers)
            {
                const TVecLayerable& children = lay->Children();
                for(const auto& child : children)
                    if(child != clicked)
                        selStateChanged |= child->DeselectEvent();
            }
        }

        if(clicked)
            selStateChanged |= clicked->SelectEvent(info, additive);

        if(selStateChanged)
            Replot();
    }

    TVecLayerable TPlot::FindLayerableList(const TPointF &pos, bool onlySelectable)
    {
        TVecLayerable res;
        for(int i = layers.size() - 1; i >= 0; --i)
        {
            const TVecLayerable& children = layers[i]->Children();
            for(int j = children.size() - 1; j >= 0; --j)
            {
                if(children[j]->RealVisibility() == false) continue;

                double dist = children[j]->SelectTest(pos, onlySelectable);
                if(dist >= 0 && dist < SelectionTolerance())
                    res.push_back(children[j]);
            }
        }
        return res;
    }

    TRawLayerable TPlot::FindLayerable(const TPointF &pos, bool onlySelectable)
    {
        TVecLayerable vec = FindLayerableList(pos, onlySelectable);
        if(vec.empty()) return nullptr;
        else return vec.front();
    }

    void TPlot::SetInitState(bool value)
    {
        if(value) isInitState++;
        else isInitState--;

        if(isInitState == 0)
            Replot();
    }

    const TPtrAxisRect &TPlot::AddAxisRect(bool isDefault)
    {
        auto res = TPtrAxisRect(new TAxisRect(this, isDefault));
        res->SetThisWeak(res);
        return AddAxisRect(res);
    }
    const TPtrAxisRect& TPlot::AddAxisRect(const TPtrAxisRect& value)
    {
        if(value->Layout() == nullptr)
            mainLayout->AddElement(0, mainLayout->CountCol(), value);
        axisRects.push_back(value);
        return axisRects.back();
    }

    void TPlot::DelAxisRect(const TPtrAxisRect& value)
    {
        mainLayout->Remove(value);
        RemoveFrom(axisRects, value);
    }

    const TPtrAxisRect& TPlot::AxisRect(size_t index) const
    {
        if(index < axisRects.size())
            return axisRects[index];
        return Single<TPtrAxisRect>();
    }

    size_t TPlot::CountAxisRect() const
    {
        return axisRects.size();
    }

    void TPlot::SetTracer(TRawItemTracer value)
    {
        tracer.reset(value);
    }

    void TPlot::Draw(const TUPtrPainter &painter)
    {
        plotLayout->Update();
        for(const auto& lay : layers)
            lay->Draw(painter);
    }

    TInitRef TPlot::InitRef(bool before, bool after)
    {
        SetInitState(before);
        return TInitRef(this, after);
    }

    const TPtrLegend &TPlot::Legend() const
    {
        return legend;
    }

    TPtrLegend TPlot::CreateLegend()
    {
        return CreateLayout<TLegend>();
    }

    const TPtrLegend &TPlot::AddLegend()
    {
        legend = CreateLegend();
        plotLayout->AddElement(2, 0, legend);
        return legend;
    }

    void TPlot::DeleteLegend()
    {
        legend.reset();
        plotLayout->Remove(legend);
    }

    TPtrAxis TPlot::CreatePlotAxis(TAxisType typeAxis)
    {
        return TPtrAxis(new TAxis(typeAxis, this));
    }

    const TPtrTitle &TPlot::Title() const
    {
        return title;
    }

    const TPtrTitle &TPlot::AddTitle(const TString &text)
    {
        if(title == nullptr)
        {
            title = CreateLayout<TTitle>(text);
            plotLayout->AddElement(0, 0, title);
        }
        return title;
    }

    void TPlot::DeleteTitle()
    {
        plotLayout->Remove(title);
        title.reset();
    }



    TInitRef::TInitRef(TPlot *p, bool v):plot(p), value(v)
    {

    }

    TInitRef::TInitRef(TInitRef &&oth):plot(oth.plot), value(oth.value)
    {
        oth.plot = nullptr;
    }

    TInitRef::~TInitRef()
    {
        if(plot)
            plot->SetInitState(value);
    }

    TTitle::TTitle(TRawPlot plt, const TString &title) : TLayoutElement(plt, "main")
    {
        SetText(title);
    }

    void TTitle::SetText(const TString &value)
    {
        SetText(TVecString{1, value});
    }

    void TTitle::SetText(const TVecString &values)
    {
        text = values;
        SetFixedHeight(17 * text.size());
    }

    void TTitle::AddText(const TString &value)
    {
        text.push_back(value);
        SetFixedHeight(text.size() * 17);
    }

    size_t TTitle::CountText() const
    {
        return text.size();
    }

    void TTitle::Draw(const TUPtrPainter &painter)
    {
        for(auto i = 0; i < text.size(); i++)
            painter->DrawText(innerRect.center() + TPoint(0, i * 17), text[i], TAlignText::Center, TAlignText::Center);
    }


    TLegendItem::TLegendItem(TRawLegend parent):TLayoutElement(parent->Plot(), "legend"),
        legend(parent)
    {
        SetMinMaxHeight(20, 20);
    }

    void TLegendItem::SetIsSelected(bool value)
    {
        SelectionChanged(isSelected, value);
    }

    void TLegendItem::SetIsSelectable(bool value)
    {
        isSelectable = value;
    }

    double TLegendItem::SelectTest(const TPointF &pos, bool onlySelectable)
    {
        if(onlySelectable && isSelectable == false)
            return -1;
        return TLayoutElement::SelectTest(pos, false);
    }

    bool TLegendItem::SelectEvent(TMouseInfo &info, bool additive)
    {
        if(isSelectable)
            return SelectionChanged(isSelected, (additive ? !isSelected : true));
        return TLayerable::SelectEvent(info, additive);
    }

    bool TLegendItem::DeselectEvent()
    {
        return SelectionChanged(isSelected, false);
    }

    TRect TLegendItem::ClipRect() const
    {
        return outRect;
    }


TLegendItemPlottable::TLegendItemPlottable(TRawLegend legend, const TPtrPlottable& ptrPlottable):
    TLegendItem(legend), plottable(ptrPlottable)
{
    ptrPlottable->OnSelectionChanged.connect([this](bool value){ SetIsSelected(value); });
    OnSelectionChanged.connect([p = ptrPlottable.get()](bool value){ p->SetIsSelect(value); });
}

void TLegendItemPlottable::Draw(const TUPtrPainter &painter)
{
    if(plottable.expired()) return;
    const auto& r = innerRect;
    TSize iconSize = legend->IconSize();
    auto center = r.center();

    TString outText = Plottable()->PlottableName();
    //общая ширина иконки и текста
    int widthRes = iconSize.width() + Legend()->TextPadding() + painter->WidthText(outText);
    if(widthRes > outRect.width())
    {
        outText = TrimBefore(outText, '/', 4);
        widthRes = iconSize.width() + Legend()->TextPadding() + painter->WidthText(outText);
    }
    TRect iconRect(TPoint(center.x() - widthRes / 2., center.y() - iconSize.height() / 2.), iconSize);

    painter->SetFont(isSelected ? fsBold : fsNormal);
    painter->DrawText(TPoint (iconRect.right() + Legend()->TextPadding(), iconRect.CenterVert()), outText,
                      TAlignText::Begin, TAlignText::Center);
    painter->Save();
    painter->SetClipRect(iconRect);
    plottable.lock()->DrawLegendIcon(painter, iconRect);
    painter->Restore();
}
//----------------------------------------------------------------------------------------------------------------------
    TLegend::TLegend(TRawPlot plt):TLayoutGrid(plt)
    {
        SetLayer("legend");
        SetMaxAddCols(3);
    }

    TSize TLegend::IconSize() const
    {
        return iconSize;
    }

    void TLegend::SetIconSize(const TSize &value)
    {
        iconSize = value;
    }

    int TLegend::TextPadding() const
    {
        return textPadding;
    }

    void TLegend::SetTextPadding(int value)
    {
        textPadding = value;
    }

    void TLegend::Draw(const TUPtrPainter &painter)
    {
        painter->SetBrush(brush);
        painter->SetPen(borderPen);
        painter->DrawRect(outRect);
    }

    void TLegend::SetBrush(const TBrush &value)
    {
        brush = value;
    }

    void TLegend::SetBorderPen(const TPen &value)
    {
        borderPen = value;
    }

    void TLegend::SetIconBorderPen(const TPen &value)
    {
        iconBorderPen = value;
    }

    size_t TLegend::CountItems() const
    {
        return items.size();
    }

    const TPtrLegendItem& TLegend::Item(size_t index) const
    {
        if(index < items.size())
            return items[index];
        return Single<TPtrLegendItem>();
    }

    void TLegend::AddItem(const TPtrLegendItem& value)
    {
        items.emplace_back(value);
        AddElement(value);
    }

    void TLegend::DelItem(size_t index)
    {
        items.erase(items.begin() + index);
    }
//----------------------------------------------------------------------------------------------------------------------
    TRuler::TRuler(size_t dpi, size_t size, size_t length):dpiPainter(dpi), sizeRuler(size), lengthRuler(length)
    {

    }

    void TRuler::Draw(const TUPtrPainter &painter)
    {
        painter->Save();
        painter->SetFont(TFont(10));
        painter->SetPen(TPen(tickColor));
        painter->FillRect(TRect(0, 0, lengthRuler, sizeRuler), background);

        if(typeRuler == TTypeRuler::Inch)
            DrawInch(painter);
        else
            DrawSm(painter);
        painter->DrawLine(TPointF(0, sizeRuler - mainLineOffset), TPointF(lengthRuler, sizeRuler - mainLineOffset));
        painter->Restore();
    }

    void TRuler::DrawSm(const TUPtrPainter &painter)
    {
        double smPoints = dpiPainter / 2.54;
        double mmPoints = smPoints / 10.;
        size_t num = 0;
        for(auto i = 0.; i < lengthRuler; i += smPoints)
        {
            //рисуем текст метки
            painter->DrawText(Plot::TPointF(i, 0) + offsetRuler, std::to_string(num++), Plot::TAlignText::Center);

            //рисуем главную метку под текстом
            painter->DrawLine(  Plot::TPointF(i, sizeRuler - mainLineOffset) + offsetRuler,
                                    Plot::TPointF(i, sizeRuler - mainLineOffset - 4) + offsetRuler);

            //рисуем метку в пол сантиметра
            painter->DrawLine(Plot::TPointF(i + smPoints / 2., sizeRuler - mainLineOffset) + offsetRuler,
                              Plot::TPointF(i + smPoints / 2., sizeRuler - mainLineOffset - 3)  + offsetRuler);

            //рисуем миллиметровые метки
            for(auto j = i; j < i + smPoints; j += mmPoints)
                painter->DrawLine(Plot::TPointF(j, sizeRuler - mainLineOffset) + offsetRuler,
                                  Plot::TPointF(j, sizeRuler - mainLineOffset - 1) + offsetRuler);
        }
    }

    void TRuler::DrawInch(const TUPtrPainter &painter)
    {
        size_t num = 0;
        double smInch = dpiPainter / 16.;
        for(auto i = 0; i < lengthRuler; i += dpiPainter)
        {
            //рисуем текст метки
            painter->DrawText(Plot::TPointF(i, 0) + offsetRuler, std::to_string(num++), Plot::TAlignText::Center);
            for(auto j = 0; j < 16; j++)
            {
                size_t lengthTick = j == 0 ? 7 : ( j % 8 == 0 ? 5 : 3);
                painter->DrawLine(Plot::TPointF(i + j * smInch, sizeRuler - mainLineOffset) + offsetRuler,
                                  Plot::TPointF(i + j * smInch, sizeRuler - mainLineOffset - lengthTick) + offsetRuler);
            }
        }
    }

    void TRuler::SetLength(size_t value)
    {
        lengthRuler = value;
    }

    void TRuler::SetSize(size_t value)
    {
        sizeRuler = value;
    }


}