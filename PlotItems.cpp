//
// Created by user on 22.05.2020.
//

#include "PlotItems.h"
#include "Plottables.h"

namespace Plot {
    TItemPosition::TItemPosition(TRawPlot value):TItemAnchor(value, nullptr)
    {

    }

    void TItemPosition::SetType(TTypePosition value)
    {
        SetTypeX(value);
        SetTypeY(value);
    }

    void TItemPosition::SetTypeX(TTypePosition value)
    {
        if(typePositionX == value) return;
        typePositionX = value;
        //TODO setpixelposition
    }

    void TItemPosition::SetTypeY(TTypePosition value)
    {
        if(typePositionY == value) return;
        typePositionY = value;
        //TODO setpixelposition
    }

    void TItemPosition::SetAxises(const TPtrAxis& key,const TPtrAxis& val)
    {
        keyAxis = key;
        valAxis = val;
    }

    void TItemPosition::SetCoords(double k, double v)
    {
        key = k;
        val = v;
    }

    TPointF TItemPosition::PixelPosition() const
    {
        TPointF res(0, 0);
        switch(typePositionX)
        {
            case TTypePosition::Absolute:
                res.rx() = key;
                if(parentAnchorX.expired() == false)
                    res.rx() += parentAnchorX.lock()->PixelPosition().x();
                break;
            case TTypePosition::ViewportRatio:
                res.rx() = key * plot->Viewport().width();
                if(parentAnchorX.expired() == false)
                    res.rx() += parentAnchorX.lock()->PixelPosition().x();
                else
                    res.rx() += plot->Viewport().left();
                break;
            case TTypePosition::AxisRectRatio:
                //TODO axisRects
                break;
            case TTypePosition::PlotCoords:
                if(keyAxis.expired() == false && keyAxis.lock()->Orientation() == orHorz)
                    res.rx() += keyAxis.lock()->CoordToPixel(key);
                else
                    if(valAxis.expired() == false && valAxis.lock()->Orientation() == orHorz)
                        res.rx() += valAxis.lock()->CoordToPixel(val);
        }

        switch(typePositionY)
        {
            case TTypePosition::Absolute:
                res.ry() = val;
                if(parentAnchorY.expired() == false)
                    res.ry() += parentAnchorY.lock()->PixelPosition().y();
                break;
            case TTypePosition::ViewportRatio:
                res.ry() = val * plot->Viewport().height();
                if(parentAnchorY.expired() == false)
                    res.ry() += parentAnchorY.lock()->PixelPosition().y();
                else
                    res.ry() += plot->Viewport().top();
                break;
            case TTypePosition::AxisRectRatio:
                //TODO axisRects
                break;
            case TTypePosition::PlotCoords:
                if(keyAxis.expired() == false && keyAxis.lock()->Orientation() == orVert)
                    res.ry() += keyAxis.lock()->CoordToPixel(key);
                else
                if(valAxis.expired() == false && valAxis.lock()->Orientation() == orVert)
                    res.ry() += valAxis.lock()->CoordToPixel(val);
                break;
        }

        return res;
    }

    double TItemPosition::PixelToKey(const TPoint &value)
    {
        if(keyAxis.expired() == false)
        {
            auto key = keyAxis.lock().get();
            if(key->Orientation() == orVert)
            {
                if(typePositionY == TTypePosition::PlotCoords)
                    return key->PixelToCoord(value.y());
            }
            else
            {
                if(typePositionX == TTypePosition::PlotCoords)
                    return key->PixelToCoord(value.x());
            }
        }
        return value.x();
    }

    double TItemPosition::PixelToVal(const TPoint &value)
    {
        if(valAxis.expired() == false)
        {
            auto val = valAxis.lock().get();
            if(val->Orientation() == orVert)
            {
                if(typePositionY == TTypePosition::PlotCoords)
                    return val->PixelToCoord(value.y());
            }
            else
            {
                if(typePositionX == TTypePosition::PlotCoords)
                    return val->PixelToCoord(value.x());
            }
        }
        return value.y();
    }

    bool TItemPosition::CheckPlotCoords() const
    {
        if(keyAxis.expired() || valAxis.expired()) return false;
        return keyAxis.lock()->Range().Contains(key) && valAxis.lock()->Range().Contains(val);
    }

//----------------------------------------------------------------------------------------------------------------------
    Plot::TAbstractItem::TAbstractItem(Plot::TRawPlot value, const TString &nameLayer):TLayerable(value, nameLayer)
    {

    }

    Plot::TAbstractItem::~TAbstractItem()
    {

    }

    void Plot::TAbstractItem::SetIsSelectable(bool value)
    {
        isSelectable = value;
    }

    bool Plot::TAbstractItem::SetIsSelected(bool value)
    {
        return SelectionChanged(isSelected, value);
    }

    bool Plot::TAbstractItem::SelectEvent(Plot::TMouseInfo &info, bool additive)
    {
        if(isSelectable)
            return SetIsSelected(additive? !isSelected : true);
        return false;
    }

    bool Plot::TAbstractItem::DeselectEvent()
    {
        return SetIsSelected(false);
    }

//----------------------------------------------------------------------------------------------------------------------
    TItemTracer::TItemTracer(TRawPlot value) : TAbstractItem(value, "overlay"),
        position(new TItemPosition(value))
    {
        isVisible = false;
        plot->OnMouseMove.connect(&TItemTracer::UpdateTracer, this);
    }

    bool TItemTracer::UpdatePosition()
    {
        if(graph.expired()) return false;
        auto coords = Graph()->TracerValue(graphKey);
        if(std::isnan(coords.x()) == false)
        {
            position->SetCoords(coords.x(), coords.y());
            return true;
        }
        return false;
    }

    double TItemTracer::SelectTest(const TPointF &pos, bool onlySelectable)
    {
        if(onlySelectable && isSelectable == false)
            return -1;
         //TODO if need select tracer
         return -1;
    }

    void TItemTracer::Draw(const TUPtrPainter &painter)
    {
        if(UpdatePosition() == false || style == tsNone)
            return;
        painter->SetPen(pen);
        painter->SetBrush(brush);

        TPointF center(position->PixelPosition());

        double w = size / 2.;
        TRect clip = ClipRect();
        switch(style)
        {
            case tsPlus :
                if (clip.intersects(TRectF(center - TPointF(w, w), center + TPointF(w, w)).toRect()))
                {
                    painter->DrawLine(center + TPointF(-w, 0), center + TPointF(w, 0));
                    painter->DrawLine(center + TPointF(0, -w), center + TPointF(0, w));
                }
                break;
            default:
                break;
        }
        auto gr = graph.lock();
        if(gr && gr->TracerMask().empty() == false)
        {
            TString text;
            if(std::isnan(position->Val()) == false)
            {
                if(gr->ConverterTraserValue())
                    text = STDFORMAT(STR(gr->TracerMask()), position->Key(), STR(gr->ConverterTraserValue()(position->Val())));
                else
                    text = STDFORMAT(STR(gr->TracerMask()), position->Key(), position->Val());
            }
            else
                text = STDFORMAT(STR(gr->TracerMask(false)), position->Key());

            if(textPosition.x() != INT_MAX) center = textPosition;

            auto widthText = painter->WidthText(text);
            //по умолчанию текст центруется и выше курсора на 20 пикселей
            TPointF offset(-widthText / 2., -20);
            if(gr && gr-> ValAxis() && gr->ValAxis()->AxisRect())
            {
                TRect r = gr->ValAxis()->AxisRect()->CentralRect();
                if(center.y() + offset.y() - 10 < r.top())
                    offset.setY(10);
                if(center.x() + offset.x() - 10 < r.left())
                    offset.setX(10);
                if(center.x() - offset.x() + 10 > r.right())
                    offset.setX(-(widthText + 10));
            }
            painter->SetBrush(TBrush(TConsts::WhiteColor()));
            painter->DrawText(center + offset, text);
        }
    }

    void TItemTracer::SetGraph(const TPtrPlottable& value)
    {
        if(value != nullptr)
        {
            if(value->Plot() == plot)
            {
                position->SetType(TTypePosition::PlotCoords);
                position->SetAxises(value->KeyAxis(), value->ValAxis());
                graph = value;
                UpdatePosition();
                return;
            }
        }
        graph.reset();
    }

    void TItemTracer::SetGraphKey(double value)
    {
        graphKey = value;
    }

    void TItemTracer::UpdateTracer(const TMouseInfo &info)
    {
        if(graph.expired()) return;
        auto ax = Graph()->KeyAxis();
        SetGraphKey(ax->PixelToCoord((ax->Orientation() == orVert) ? info.pos.y() : info.pos.x()));
        SetTextPosition(info.pos);
        Layer()->Replot();
    }

    TPoint TItemTracer::TextPostion() const
    {
        return textPosition;
    }

    void TItemTracer::SetTextPosition(const TPoint &value)
    {
        textPosition = value;
    }
//----------------------------------------------------------------------------------------------------------------------

    TItemBorder::TItemBorder(const TPtrAxis& key, const TPtrAxis& val) : TAbstractItem(key->Plot(), "overlay"), position(key->Plot())
    {
        position.SetType(TTypePosition::PlotCoords);
        position.SetAxises(key, val);
        isSelectable = true;
    }

    void TItemBorder::SetKeyVal(double key, double val)
    {
        if(position.Key() == key) return;
        position.SetCoords(key, val);
        OnPositionChanged(position.Key());
        plot->Replot();
    }

    void TItemBorder::Draw(const TUPtrPainter &painter)
    {
        painter->SetPen(isSelected ? selPen : itemPen);
        painter->SetBrush(TBrush());
        TPointF begin, end;
        GetPosLine(begin, end);
        painter->DrawLine(begin, end);
        if(title.empty() == false)
        {
            TString t = STDFORMAT(title, Key());
            painter->SetBrush(TBrush(TConsts::TransparentColor()));

            switch (alignTitle)
            {
                case TAlignText::Begin:
                    begin += TPointF(10, 0);
                    painter->DrawText(begin, t, TAlignText::Begin, TAlignText::End);
                break;

                case TAlignText::Center:
                    begin += TPointF(begin.x() + (end.x() - begin.x()) / 2., 0);
                    painter->DrawText(begin, t, TAlignText::Center, TAlignText::End);
                break;

                case TAlignText::End:
                    end += TPointF(-10, 0);
                    painter->DrawText(end, t, TAlignText::End, TAlignText::End);
                break;
            }
        }
    }

    double TItemBorder::SelectTest(const TPointF &pos, bool onlySelectable)
    {
        if(onlySelectable && isSelectable == false)
            return -1;
        TPointF begin, end;
        GetPosLine(begin, end);
        return std::sqrt(DistanceSquaredToLine(pos, begin, end));
    }

    void TItemBorder::GetPosLine(TPointF &begin, TPointF &end)
    {
        TPointF pos = position.PixelPosition();
        TRectF r = isMaskRect ? position.KeyAxis()->AxisRect()->CentralRect() : plot->Viewport();
        if(position.KeyAxis()->Orientation() == orVert)
        {
            begin = TPointF(r.left(), pos.y()),
            end = TPointF(r.right(), pos.y());
        }
        else
        {
            begin = TPointF(pos.x(), r.top());
            end = TPointF(pos.x(), r.bottom());
        }
    }

    void TItemBorder::MousePress(TMouseInfo &info)
    {
        if(info.IsLeftButton() && info.IsMoving())
        {
            isEditing = true;
            beginMoving = position.Key();
            plot->Replot();
        }
    }

    void TItemBorder::MouseMove(TMouseInfo &info, const TPoint &startPos)
    {
        if(isEditing)
        {
            position.SetKey(beginMoving + position.PixelToKey(info.pos) - position.PixelToKey(startPos));
            OnPositionChanged(position.Key());
            plot->Replot();
        }
        else
            info.Ignore();
    }

    void TItemBorder::MouseUp(TMouseInfo &info, const TPoint &startPos)
    {
        isEditing = false;
    }

    TRect TItemBorder::ClipRect() const
    {
        return position.ValAxis()->Plot()->Viewport();
        //return position.ValAxis()->AxisRect()->InnerRect();
    }

    bool TItemBorder::IsMaskRect() const
    {
        return isMaskRect;
    }

    void TItemBorder::SetIsMaskRect(bool value)
    {
        isMaskRect = value;
    }

//--------------------------------------------------------------------------------------
    TItemText::TItemText(Plot::TRawPlot plt):TAbstractItem(plt, "main"), position(plt)
    {
        isSelectable = true;
    }

    void TItemText::SetText(const TString &value)
    {
        text = value;
    }

    TString TItemText::Text() const
    {
        return text;
    }

    TColor TItemText::ColorText() const
    {
        return colorText;
    }

    void TItemText::SetColorText(const TColor &value)
    {
        colorText = value;
    }

    TColor TItemText::ColorBack() const
    {
        return colorBack;
    }

    void TItemText::SetColorBack(const TColor &value)
    {
        colorBack = value;
    }

    const TItemPosition &TItemText::Position() const
    {
        return position;
    }

    TItemPosition &TItemText::Position()
    {
        return position;
    }

    void TItemText::Draw(const TUPtrPainter &painter)
    {
        if(position.Type() == TTypePosition::PlotCoords && position.CheckPlotCoords() == false)
                return;
        auto p = painter->Pen();
        p.color = colorText;
        painter->SetPen(p);

        painter->SetFont(fontSize);

        auto pos = position.PixelPosition();
        textRect = TRectF(pos.x(), pos.y(), painter->WidthText(text) * 1.2, painter->HeightText(text) * 1.2);
        textRect.setLeft(textRect.left() - textRect.width() / 2.);
        textRect.setTop(textRect.top() - textRect.height() / 2.);
        if(colorBack != TConsts::TransparentColor())
            painter->FillRect(textRect, colorBack);

        painter->DrawText(pos, text, TAlignText::Center, TAlignText::Center);
        if(isSelected)
            painter->DrawRect(textRect);
    }

    size_t TItemText::FontSize() const
    {
        return fontSize;
    }

    void TItemText::SetFontSize(size_t value)
    {
        fontSize = value;
    }

    double TItemText::SelectTest(const TPointF &pos, bool onlySelectable)
    {
        if(textRect.contains(pos) && plot)
            return plot->SelectionTolerance() * 0.99;
        return -1;
    }

    void TItemText::MousePress(TMouseInfo &info)
    {
        if(info.IsLeftButton() && info.IsMoving())
        {
            isEditing = true;
            beginMoving = TPointF(position.Key(), position.Val() );
            plot->Replot();
        }
    }

    void TItemText::MouseMove(TMouseInfo &info, const TPoint &startPos)
    {
        if(isEditing)
        {
            position.SetKey(beginMoving.x() + position.PixelToKey(info.pos) - position.PixelToKey(startPos));
            position.SetVal(beginMoving.y() + position.PixelToVal(info.pos) - position.PixelToVal(startPos));
            OnPositionChanged(TPointF(position.Key(), position.Val()));
            plot->Replot();
        }
        else
            info.Ignore();
    }

    void TItemText::MouseUp(TMouseInfo &info, const TPoint &startPos)
    {
        isEditing = false;
    }
}