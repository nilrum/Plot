//
// Created by user on 03.09.2020.
//

#include "BaseClasses.h"
#include "Plot.h"

namespace Plot {

    TPaintBuffer::TPaintBuffer(const TSize &value):size(value)
    {

    }

    void TPaintBuffer::SetSize(const TSize &value)
    {
        if(size == value) return;
        size = value;
        ReallocBuffer();
    }

    void TPaintBuffer::SetInvalidated(bool value)
    {
        invalidate = value;
    }

    //----------------------------------------------------------------------------------------------------------------------
    TLayer::TLayer(TRawPlot value, const TString &nameLay, int ind ): plot(value), name(nameLay), index(ind)
    {

    }

    TLayer::~TLayer()
    {

    }

    void TLayer::SetIsVisible(bool value)
    {
        isVisible = value;
    }

    void TLayer::SetMode(TLayerMode value)
    {
        if(mode == value) return;
        mode = value;
        if(buffer.expired() == false)
            buffer.lock()->SetInvalidated(true);
    }

    void TLayer::Replot()
    {
        if(mode == TLayerMode::Buffered)
        {
            if(plot->HasInvalidateBuffers()) return;
            if(buffer.expired() == false)
            {
                TPtrPaintBuffer buf = buffer.lock();
                buf->Clear(TConsts::TransparentColor());
                DrawToBuffer();
                buf->SetInvalidated(false);
                plot->NativePaintLater();
            }
        }
        else
            plot->Replot();
    }

    void TLayer::Draw(const TUPtrPainter& painter) const
    {
        auto p = painter.get();
        for(const TRawLayerable& child : children)
            if(child->RealVisibility())
            {
                p->Save();
                p->SetClipRect(child->ClipRect().translated(0, -1));
                bool oldAnti = p->Antialiasing();
                p->SetAntialiasing(child->IsAntiAliasing());
                child->Draw(painter);
                p->SetAntialiasing(oldAnti);
                p->Restore();
            }
    }

    void TLayer::DrawToBuffer() const
    {
        if(buffer.expired()) return;
        TPtrPaintBuffer buf = buffer.lock();
        TUPtrPainter painter = buf->StartPaint();
        if(painter && painter->IsActive())
        {
            Draw(painter);
        }
        buf->StopPaint();
    }

    void TLayer::AddChild(TRawLayerable value, bool prepend)
    {
        auto it = find(children.begin(), children.end(), value);
        if(it == children.end())
        {
            if(prepend)
                children.insert(children.begin(), value);
            else
                children.emplace_back(value);
            if(buffer.expired() == false)
                buffer.lock()->SetInvalidated(true);
        }
    }

    void TLayer::DelChild(TRawLayerable value)
    {
        auto it = find(children.begin(), children.end(), value);
        if(it != children.end())
        {
            children.erase(it);
            if(buffer.expired() == false)
                buffer.lock()->SetInvalidated(true);
        }
    }

    void TLayer::SetBuffer(const TPtrPaintBuffer &value)
    {
        buffer = value;
    }

    void TLayer::CheckIndexesInLayer()
    {
        if(children.size() < 2) return;
        int ib = -1;
        int ie = -1;
        for(size_t i = 1; i < children.size(); i++)
        {
            if(ib == -1)//если еще не нашли что менять
            {
                if (children[i - 1]->IndexInLayer() > children[i]->IndexInLayer())
                {
                    ib = i - 1;
                    ie = i;
                }
            }
            else
                {//проверим а не больше ли он и последующих
                    if (children[ib]->IndexInLayer() > children[i]->IndexInLayer())
                        ie = i;
                }
        }
        if(ib != -1)//если нашли индексы которые меняем
            std::swap(children[ib], children[ie]);
    }

//----------------------------------------------------------------------------------------------------------------------
    TLayerable::TLayerable(TRawPlot plt, const TPtrLayer& lay): plot(plt)
    {
        SetLayer(lay);
    }

    TLayerable::TLayerable(TRawPlot plt, const TString &nameLayer):TLayerable(plt, plt->Layer(nameLayer))
    {

    }

    TLayerable::~TLayerable()
    {
        OnDestroy();
        if(layer.expired() == false)
            Layer()->DelChild(this);
    }

    void TLayerable::SetLayer(const TPtrLayer &value, bool prepend)
    {
        if(layer.expired() == false)        //если назначили новый слой, а он уже был
            Layer()->DelChild(this);

        layer = value;

        if(value)
            value->AddChild(this, prepend);
    }

    void TLayerable::SetLayer(const TString &nameLayer, bool prepend)
    {
        SetLayer(plot->Layer(nameLayer), prepend);
    }

    void TLayerable::SetIsVisible(bool value)
    {
        isVisible = value;
    }


    bool TLayerable::RealVisibility() const
    {
        return isVisible &&
            (layer.expired() || Layer()->IsVisible()) &&
            (parentLayerable.expired() || ParentLayerable()->RealVisibility());
    }

    TRect TLayerable::ClipRect() const
    {
        if(plot) return plot->Viewport();
        return Plot::TRect();
    }

    void TLayerable::SetIndexInLayer(int index)
    {
        indexInLayer = index;
        if(layer.expired() == false) Layer()->CheckIndexesInLayer();
    }

    TPtrLayerable TLayerable::ParentLayerable() const
    {
        return parentLayerable.lock();
    }

    void TLayerable::SetParentLayerable(const TWPtrLayerable &value)
    {
        parentLayerable = value;
    }

    void TLayerable::Replot()
    {
        plot->Replot();
    }


}