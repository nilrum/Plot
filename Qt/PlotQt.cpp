//
// Created by user on 07.05.2020.
//

#include "PlotQt.h"
#include <QDebug>

namespace Plot {

    TColor TConsts::TransparentColor()
    {
        return QColor(Qt::transparent).rgba64();
    }

    TColor TConsts::WhiteColor()
    {
        return QColor(Qt::white).rgba64();
    }

    TColor TConsts::BlackColor()
    {
        return QColor(Qt::black).rgba64();
    }

    TColor TConsts::BlueColor()
    {
        return QColor(Qt::blue).rgba64();
    }


    int TConsts::GetAlpha(const TColor &value)
    {
        return QRgba64::fromRgba64(value).alpha();
    }


    TColor TConsts::SetAlpha(const TColor &value, int alpha)
    {
        QColor r = QRgba64::fromRgba64(value);
        r.setAlpha(alpha);
        return r.rgba64();
    }

    TColor TConsts::RandColor()
    {
        return QColor(rand() % 255, rand() % 255, rand() % 255).rgba64();
    }

    TColor TConsts::CreateColor(int r, int g, int b, int a)
    {
        return QColor(r, g, b, a).rgba64();
    }
}
//----------------------------------------------------------------------------------------------------------------------
TPaintBufferPixmap::TPaintBufferPixmap(const Plot::TSize &size, double pixelRatio)
    :Plot::TPaintBuffer(size), devicePixelRatio(pixelRatio)
{
    TPaintBufferPixmap::ReallocBuffer();
}

Plot::TUPtrPainter TPaintBufferPixmap::StartPaint()
{
    return Plot::TUPtrPainter(new TPainterQt(&buffer));
}

void TPaintBufferPixmap::Draw(const Plot::TUPtrPainter &painter, int x, int y)
{
    if(painter && painter->IsActive())
        painter->DrawPixmap(x, y, buffer);
}

void TPaintBufferPixmap::Clear(const TColor &color)
{
    buffer.fill(QRgba64::fromRgba64(color));
}

void TPaintBufferPixmap::ReallocBuffer()
{
    SetInvalidated(true);
    buffer = Plot::TPixmap(size.width(), size.height());
}
//----------------------------------------------------------------------------------------------------------------------

TPainterQt::TPainterQt():QPainter()
{

}

TPainterQt::TPainterQt(QPaintDevice *device):QPainter(device)
{

}

void TPainterQt::Save()
{
    save();
}

void TPainterQt::Restore()
{
    restore();
}

void TPainterQt::SetClipRect(const Plot::TRect &value)
{
    setClipRect(To(value));
}

bool TPainterQt::IsActive() const
{
    return isActive();
}

bool TPainterQt::Antialiasing() const
{
    return antialiasing;
}


void TPainterQt::SetAntialiasing(bool value)
{
    setRenderHint(QPainter::Antialiasing, value);
    if(antialiasing != value)
    {
        antialiasing = value;
        if(TestFlag(pmVectorized) == false)
        {
            if (antialiasing)
                translate(0.5, 0.5);
            else
                translate(-0.5, -0.5);
        }

    }
}

void TPainterQt::Offset(int x, int y)
{
    translate(x, y);
}

Plot::TPen TPainterQt::Pen() const
{
    const QPen& p = pen();
    return Plot::TPen(p.color().rgba64(), p.width(), (Plot::TPenStyle)p.style());
}

Plot::TBrush TPainterQt::Brush() const
{
    const QBrush& b = brush();
    return Plot::TBrush(b.color().rgba64(), (Plot::TBrushStyle)b.style());
}


void TPainterQt::SetPen(const Plot::TPen &value)
{
    QPen p = pen();
    p.setColor(QRgba64::fromRgba64(value.color));
    p.setWidth(value.width);
    p.setStyle(Qt::PenStyle(value.style));
    setPen(p);
}

void TPainterQt::SetBrush(const Plot::TBrush &value)
{
    QBrush b = brush();
    b.setColor(QRgba64::fromRgba64(value.color));
    b.setStyle(Qt::BrushStyle(value.style));
    setBrush(b);
}

void TPainterQt::SetFont(const Plot::TFont &value)
{
    static int qtWeights[] = {QFont::Normal, QFont::Bold};
    auto f = font();
    f.setWeight(qtWeights[value.style]);
    f.setPixelSize(value.size);
    setFont(f);
}


void TPainterQt::DrawRect(const Plot::TRect &rect)
{
    drawRect(To(rect));
}

void TPainterQt::FillRect(const Plot::TRect &rect, const TColor &color)
{
    QColor qtColor = QRgba64::fromRgba64(color);
    fillRect(To(rect), qtColor);
}

void TPainterQt::DrawPixmap(int x, int y, const Plot::TPixmap &value)
{
    drawPixmap(x, y, value);
}

void TPainterQt::DrawLine(const Plot::TPointF &b, const Plot::TPointF &e)
{
    if(antialiasing && TestFlag(pmVectorized))
        drawLine(To(b), To(e));
    else
        drawLine(To(b.toPoint()), To(e.toPoint()));
}

void TPainterQt::DrawLine(const Plot::TPoint &b, const Plot::TPoint &e)
{
    drawLine(To(b), To(e));
}

void TPainterQt::DrawEllipse(const Plot::TPointF &center, double rx, double ry)
{
    drawEllipse(To(center), rx, ry);
}

void TPainterQt::DrawPolyline(const Plot::TPointF *points, int pointCount)
{
    drawPolyline((QPointF*)points, pointCount);
}

void TPainterQt::DrawPolygon(const Plot::TPointF *points, int pointCount)
{
    drawPolygon((QPointF*)points, pointCount);
}

void TPainterQt::DrawText(const Plot::TPointF &p, const TString &value, Plot::TAlignText horz, Plot::TAlignText vert, double rotateVal)
{
    QRectF textRect = fontMetrics().boundingRect(0, 0, 0, 0, Qt::TextDontClip, STR(value));

    if(horz != Plot::TAlignText::Begin)
    {
        switch (horz)
        {
            case Plot::TAlignText::Center: textRect.moveLeft(- textRect.width() / 2.); break;
            case Plot::TAlignText::End: textRect.moveLeft(- textRect.width()); break;
        }
    }
    if(vert != Plot::TAlignText::Begin)
    {
        switch (vert)
        {
            case Plot::TAlignText::Center: textRect.moveTop(- textRect.height() / 2.); break;
            case Plot::TAlignText::End: textRect.moveTop(textRect.top() - textRect.height()); break;
        }
    }
    textRect.translate(To(p));
    if(rotateVal != 0)
    {
        QTransform oldTransform = transform();
        if(rotateVal < 0)
            translate((textRect.left() - textRect.height()), textRect.top() + textRect.width() / 2.);
        else
            translate((textRect.left() + textRect.height()), textRect.top() - textRect.width() / 2.);
        rotate(rotateVal);
        drawText(0, 0, textRect.width(), textRect.height(), Qt::TextDontClip, STR(value));
        setTransform(oldTransform);
    }
    else
    {
        if(brush().style() != Qt::NoBrush)
            fillRect(textRect, brush());
        drawText(textRect.left(), textRect.top(), textRect.width(), textRect.height(), Qt::TextDontClip, QVariant(STR(value)).toString());
    }
}

int TPainterQt::WidthText(const TString &value)
{
    //return QFontMetrics(font()).horizontalAdvance(value.c_str());
    return fontMetrics().boundingRect(0, 0, 0, 0, 0, STR(value)).width();
}

int TPainterQt::HeightText(const TString &value)
{
    //return QFontMetrics(font()).height();
    return fontMetrics().boundingRect(0, 0, 0, 0, 0, STR(value)).height();
}

void TPainterQt::SetClipPolygon(const Plot::TVecPointF& points, bool isSub)
{
    QVector<QPoint> cl(points.size());
    for(size_t i = 0; i < points.size(); i++)
    {
        const Plot::TPointF& p = points[i];
        cl[i] = QPoint(p.x(), p.y());
    }
    if(isSub)
        setClipRegion(clipRegion().subtracted(QRegion(cl)));
    else
        setClipRegion(QRegion(cl));
}

Plot::TRegion TPainterQt::ClipRegion() const
{
    return clipRegion();
}



/*
void TPainterQt::SetClipRegion(const Plot::TRegion &value)
{
    setClipRegion(value);
}*/


namespace Plot{

TColorMap::TColorMap(const TPtrAxis& key, const TPtrAxis& val) : TColorMapBase(key, val)
{

}

void TColorMap::Draw(const TUPtrPainter &painter)
{
    if(keyAxis.expired() || valAxis.expired()) return;
    if(mapData->IsDataModifed() || isMapImageInvalidated)
        UpdateMapImage();
    bool isUseBuffer = painter->TestFlag(TPainter::pmVectorized);
    TPainterQt* localPainter = dynamic_cast<TPainterQt*>(painter.get());
    QRectF mapBufferTarget;
    QPixmap mapBuffer;
    if(isUseBuffer)
    {
        const double mapBufferPixelRatio = 3;
        mapBufferTarget = painter->ClipRegion().boundingRect();
        mapBuffer = QPixmap((mapBufferTarget.size() * mapBufferPixelRatio).toSize());
        mapBuffer.fill(Qt::transparent);
        localPainter = new TPainterQt(&mapBuffer);
        localPainter->scale(mapBufferPixelRatio, mapBufferPixelRatio);
        localPainter->translate(-mapBufferTarget.topLeft());
    }

    TRectF imageRect = TRectF(CoordToPixel(mapData->KeyRange().lower, mapData->ValRange().lower),
                              CoordToPixel(mapData->KeyRange().upper, mapData->ValRange().upper)).normalized();
    double halfCellWidth = 0;
    double halfCellHeight= 0;
    auto key = keyAxis.lock().get();
    auto val = valAxis.lock().get();
    if(key->Orientation() == orVert)
    {
        if(mapData->KeySize() > 1)
            halfCellHeight = 0.5 * imageRect.height() / double(mapData->KeySize() - 1);
        if(mapData->ValSize() > 1)
            halfCellWidth = 0.5 * imageRect.width() / double(mapData->ValSize() - 1);
    }
    else
    {
        if(mapData->KeySize() > 1)
            halfCellWidth = 0.5 * imageRect.width() / double(mapData->KeySize() - 1);
        if(mapData->ValSize() > 1)
            halfCellHeight = 0.5 * imageRect.height() / double(mapData->ValSize() - 1);
    }
    imageRect.adjust(-halfCellWidth, -halfCellHeight, halfCellWidth, halfCellHeight);
    bool mirrorX = ((key->Orientation() == orHorz) ? key : val)->IsRangeReversed();
    bool mirrorY = !((val->Orientation() == orVert) ? val : key)->IsRangeReversed();
    bool smoothBackup = localPainter->renderHints().testFlag(QPainter::SmoothPixmapTransform);
    localPainter->setRenderHint(QPainter::SmoothPixmapTransform, isInterpolate);
    QRegion clipBackup;
    if(isTightBoundary)
    {
        clipBackup = localPainter->clipRegion();
        TRectF tightClipRect = TRectF(CoordToPixel(mapData->KeyRange().lower, mapData->ValRange().lower),
                                      CoordToPixel(mapData->KeyRange().upper, mapData->ValRange().upper)).normalized();
        localPainter->setClipRect(To(tightClipRect), Qt::IntersectClip);
    }
    localPainter->drawImage(To(imageRect), mapImage.mirrored(mirrorX, mirrorY));
    if(isTightBoundary)
        localPainter->setClipRegion(clipBackup);
    localPainter->setRenderHint(QPainter::SmoothPixmapTransform, smoothBackup);
    if(isUseBuffer)
    {
        delete localPainter;
        ((TPainterQt*)painter.get())->drawPixmap(mapBufferTarget.toRect(), mapBuffer);
    }
}

void TColorMap::UpdateMapImage()
{
    if(keyAxis.expired() || mapData->IsEmpty()) return;
    QImage::Format f = QImage::Format_ARGB32_Premultiplied;

    int keySize = mapData->KeySize();
    int valSize = mapData->ValSize();
    int keyFactor = OversamplingFactor(keySize);
    int valFactor = OversamplingFactor(valSize);
    int resKey = keySize * keyFactor;
    int resVal = valSize * valFactor;

    TOrientation orient = keyAxis.lock()->Orientation();

    if(orient == orVert && (mapImage.width() != resVal || mapImage.height() != resKey))
        mapImage = QImage(QSize(resVal, resKey), f);
    else if(orient == orHorz && (mapImage.width() != resKey || mapImage.height() != resVal))
        mapImage = QImage(QSize(resKey, resVal + 1), f);

    if(mapImage.isNull())
    {
        mapImage = QImage(QSize(10, 10), f);
        mapImage.fill(Qt::black);
    }
    else
    {
        QImage* localImage = &mapImage;
        if(keyFactor > 1 || valFactor > 1)
        {
            if(orient == orVert && (underMapImage.width() != valSize || underMapImage.height() != keySize))
                underMapImage = QImage(QSize(valSize, keySize), f);
            else
                if(orient == orHorz && (underMapImage.width() != keySize || mapImage.height() != valSize))
                    underMapImage = QImage(QSize(keySize, valSize), f);
            localImage = &underMapImage;
        }
        else
        {
            if(underMapImage.isNull() == false)
                underMapImage = QImage();
        }

        const double* rawData = mapData->RawData();
        const unsigned char* rawAlpha = mapData->RawAlpha();

        if(orient == orVert)
        {
            const int& lineCount = keySize;
            const int& rowCount = valSize;
            for(int line = 0; line < lineCount; ++line)
            {
                QRgb* pixels = reinterpret_cast<QRgb*>(localImage->scanLine(lineCount - 1 - line));
                if(rawAlpha)
                    gradient.Colorize(rawData + line, rawAlpha + line, dataRange, pixels, rowCount, lineCount, scaleType == stLog);
                else
                    gradient.Colorize(rawData + line, dataRange, pixels, rowCount, lineCount, scaleType == stLog);
            }
        }
        else
        {
            const int& lineCount = valSize;
            const int& rowCount = keySize;
            for(int line = 0; line < lineCount; ++line)
            {
                QRgb* pixels = reinterpret_cast<QRgb*>(localImage->scanLine(lineCount - 1 - line));
                if(rawAlpha)
                    gradient.Colorize(rawData + line * rowCount, rawAlpha + line * rowCount, dataRange, pixels, rowCount, 1, scaleType == stLog);
                else
                    gradient.Colorize(rawData + line * rowCount, dataRange, pixels, rowCount, 1, scaleType == stLog);
            }
        }
        if(keyFactor > 1 || valFactor > 1)
        {
            if(orient == orVert)
                mapImage = underMapImage.scaled(resVal, resKey, Qt::IgnoreAspectRatio, Qt::FastTransformation);
        }
    }
    mapData->SetIsDataModifed(false);
    isMapImageInvalidated = false;
}

void TColorMap::SetPreset(TGradientPreset value)
{
    preset = value;
    gradient.LoadPreset(preset);
    isMapImageInvalidated = true;
}

//---------------------------------------------------------------------------------------------------------------------

void TOptimizedColorMap::DrawHeaderImage()
{
    int valSize = 20;//сделаем просто значение по умолчанию
    if(headerHeight && (headerImage.height() != headerHeight || gradient.IsBufferInvalidated()))
    {
        headerImage = QImage(QSize(valSize, headerHeight), QImage::Format_ARGB32_Premultiplied);
        TVecDouble headerVals(valSize);
        for(auto i = 0; i < headerVals.size(); i++)
            headerVals[i] = i * 1 / double(valSize - 1);
        TRange headerRange(0, 1);
        for(auto i = 0; i < headerHeight; i++)
        {
            QRgb* pixels = reinterpret_cast<QRgb*>(headerImage.scanLine(i));
            gradient.Colorize(headerVals.data(), headerRange, pixels, headerVals.size(), 1, scaleType == stLog);
        }
    }
}
    void TOptimizedColorMap::UpdateOptimizedImage()
    {//оптимизированная версия только для вертикального расположения keyAx
        DrawHeaderImage();
        if(keyAxis.expired() || mapData->IsEmpty()) return;
        int keySize = mapData->KeySize();
        int valSize = mapData->ValSize();

        QImage::Format f = QImage::Format_ARGB32_Premultiplied;


        int offsetRawData = 0;
        if(keySize > 10000)
        {
            const TRange &keyRange = KeyAxis()->Range();
            const TRange &mapRange = mapData->KeyRange();

            const double offset = keyRange.Size() * 0.5;
            bufferRange.Set(std::max(keyRange.lower - offset, mapRange.lower),
                            std::min(keyRange.upper + offset, mapRange.upper));
            int lowerKey = mapData->KeyCell(bufferRange.lower, true);
            int upperKey = mapData->KeyCell(bufferRange.upper, true);
            keySize = upperKey - lowerKey + 1;
            offsetRawData = lowerKey;
        }
        else
        {
            bufferRange.Set(0, 0, false);
        }

        if(mapImage.width() != valSize || mapImage.height() != keySize)
            mapImage = QImage(QSize(valSize, keySize), f);

        if(mapImage.isNull() == false)
        {
            const double* rawData = mapData->RawData();
            const unsigned char* rawAlpha = mapData->RawAlpha();

            int lineCount = mapImage.height();
            int keySize = mapData->KeySize();
            for(int line = 0; line < lineCount; ++line)
            {
                QRgb* pixels = reinterpret_cast<QRgb*>(mapImage.scanLine(lineCount - 1 - line));
                gradient.Colorize(rawData + line + offsetRawData, dataRange, pixels, valSize, keySize, scaleType == stLog);
            }
        }

        mapData->SetIsDataModifed(false);
        isMapImageInvalidated = false;

    }

    void TOptimizedColorMap::Draw(const TUPtrPainter &painter)
    {
        if(keyAxis.expired() || valAxis.expired() || mapData->KeySize() < 2 || mapData->ValSize() < 2) return;
        if(mapData->IsDataModifed() || isMapImageInvalidated || IsNeedUpdateBuffer())
            UpdateOptimizedImage();
        bool isUseBuffer = painter->TestFlag(TPainter::pmVectorized);
        TPainterQt* localPainter = dynamic_cast<TPainterQt*>(painter.get());
        QRectF mapBufferTarget;
        QPixmap mapBuffer;
        if(isUseBuffer)
        {
            const double mapBufferPixelRatio = 3;
            mapBufferTarget = painter->ClipRegion().boundingRect();
            mapBuffer = QPixmap((mapBufferTarget.size() * mapBufferPixelRatio).toSize());
            mapBuffer.fill(Qt::transparent);
            localPainter = new TPainterQt(&mapBuffer);
            localPainter->scale(mapBufferPixelRatio, mapBufferPixelRatio);
            localPainter->translate(-mapBufferTarget.topLeft());
        }

        TRectF imageRect;
        if(bufferRange.IsValid() == false)
            imageRect = TRectF(CoordToPixel(mapData->KeyRange().lower, mapData->ValRange().lower),
                                  CoordToPixel(mapData->KeyRange().upper, mapData->ValRange().upper)).normalized();
        else
            imageRect = TRectF(CoordToPixel(bufferRange.lower, mapData->ValRange().lower),
                               CoordToPixel(bufferRange.upper, mapData->ValRange().upper)).normalized();

        double halfCellWidth =  0.5 * imageRect.width() / double(mapImage.width() - 1);
        double halfCellHeight = 0.5 * imageRect.height() / double(mapImage.height() - 1);

        auto key = keyAxis.lock().get();
        auto val = valAxis.lock().get();

        imageRect.adjust(-halfCellWidth, -halfCellHeight, halfCellWidth, halfCellHeight);
        bool mirrorX = ((key->Orientation() == orHorz) ? key : val)->IsRangeReversed();
        bool mirrorY = !((val->Orientation() == orVert) ? val : key)->IsRangeReversed();
        bool smoothBackup = localPainter->renderHints().testFlag(QPainter::SmoothPixmapTransform);
        localPainter->setRenderHint(QPainter::SmoothPixmapTransform, isInterpolate);

        localPainter->drawImage(To(imageRect), mapImage.mirrored(mirrorX, mirrorY));

        localPainter->setRenderHint(QPainter::SmoothPixmapTransform, smoothBackup);
        if(isUseBuffer)
        {
            delete localPainter;
            ((TPainterQt*)painter.get())->drawPixmap(mapBufferTarget.toRect(), mapBuffer);
        }
    }

    bool TOptimizedColorMap::IsNeedUpdateBuffer()
    {
        if(bufferRange.IsValid() == false) return false;
        const TRange& keyRange = KeyAxis()->Range();
        const TRange& mapRange = mapData->KeyRange();

        return  (bufferRange.lower > keyRange.lower && bufferRange.lower != mapRange.lower) ||
                (bufferRange.upper < keyRange.upper && bufferRange.upper != mapRange.upper);
    }

void TOptimizedColorMap::SetHeaderHeight(size_t value)
{
    headerHeight = value;
    if(headerHeight)
    {
        auto ax = valAxis.lock().get();
        headerConnect = ax->OnExtPaint.connect([this](const TUPtrPainter& painter)
        {
            DrawHeaderImage();

            auto rect = valAxis.lock()->OutRect();
            rect.setTop(rect.bottom() - headerHeight);
            rect.setLeft(rect.left() + 1);
            rect.setRight(rect.right() - 1);
            rect.setBottom(rect.bottom() - 1);
            auto p = (TPainterQt*)painter.get();

            bool smoothBackup = p->renderHints().testFlag(QPainter::SmoothPixmapTransform);
            p->setRenderHint(QPainter::SmoothPixmapTransform, isInterpolate);
            p->drawImage(To(rect), headerImage);
            p->setRenderHint(QPainter::SmoothPixmapTransform, smoothBackup);
            p->SetFont(10);
            p->DrawText(rect.topLeft() + TPoint(2, -1), "Min", TAlignText::Begin, TAlignText::End, 0.);
            p->DrawText(rect.topRight() + TPoint(-2, -1), "Max", TAlignText::End, TAlignText::End, 0.);
        }
        );
    }
    else
        headerConnect.disconnect();
}

    void TOptimizedColorMap::SetPreset(TGradientPreset value)
    {
        TColorMap::SetPreset(value);
        if(valAxis.expired() == false)
            valAxis.lock()->Invalidate();
    }


//----------------------------------------------------------------------------------------------------------------
    TColorGradient::TColorGradient(TGradientPreset preset)
    {
        LoadPreset(preset);
    }

    void TColorGradient::LoadPreset(TGradientPreset preset)
    {
        ClearColorStops();
        switch(preset)
        {
            case gpGrayscale:
                SetInterpolation(ciRGB);
                SetColorStop(0, Qt::black);
                SetColorStop(1, Qt::white);
                break;
            case gpHot:
                SetInterpolation(ciRGB);
                SetColorStop(0.0, QColor(50, 0, 0));
                SetColorStop(0.2, QColor(180, 10, 0));
                SetColorStop(0.4, QColor(245, 50, 0));
                SetColorStop(0.6, QColor(255, 150, 10));
                SetColorStop(0.8, QColor(255, 255, 50));
                SetColorStop(1.0, QColor(255, 255, 255));
                break;
            case gpCold:
                SetInterpolation(ciRGB);
                SetColorStop(0.0, QColor(0, 0, 50));
                SetColorStop(0.2, QColor(0, 10, 180));
                SetColorStop(0.4, QColor(0, 50, 245));
                SetColorStop(0.6, QColor(10, 150, 255));
                SetColorStop(0.8, QColor(50, 255, 255));
                SetColorStop(1.0, QColor(255, 255, 255));
                break;
            case gpNight:
                SetInterpolation(ciHSV);
                SetColorStop(0, QColor(10, 20, 30));
                SetColorStop(1, QColor(250, 255, 250));
                break;
            case gpCandy:
                SetInterpolation(ciHSV);
                SetColorStop(0, QColor(0, 0, 255));
                SetColorStop(1, QColor(255, 250, 250));
                break;
            case gpGeography:
                SetInterpolation(ciRGB);
                SetColorStop(0, QColor(70, 170, 210));
                SetColorStop(0.20, QColor(90, 160, 180));
                SetColorStop(0.25, QColor(45, 130, 175));
                SetColorStop(0.30, QColor(100, 140, 125));
                SetColorStop(0.5, QColor(100, 140, 100));
                SetColorStop(0.6, QColor(130, 145, 120));
                SetColorStop(0.7, QColor(140, 130, 120));
                SetColorStop(0.9, QColor(180, 190, 190));
                SetColorStop(1, QColor(210, 210, 230));
                break;
            case gpIon:
                SetInterpolation(ciHSV);
                SetColorStop(0, QColor(50, 10, 10));
                SetColorStop(0.45, QColor(0, 0, 255));
                SetColorStop(0.8, QColor(0, 255, 255));
                SetColorStop(1, QColor(0, 255, 0));
                break;
            case gpThermal:
                SetInterpolation(ciRGB);
                SetColorStop(0, QColor(0, 0, 50));
                SetColorStop(0.15, QColor(20, 0, 120));
                SetColorStop(0.33, QColor(200, 30, 140));
                SetColorStop(0.6, QColor(255, 100, 0));
                SetColorStop(0.85, QColor(255, 255, 40));
                SetColorStop(1, QColor(255, 255, 255));
                break;
            case gpPolar:
                SetInterpolation(ciRGB);
                SetColorStop(0, QColor(50, 255, 255));
                SetColorStop(0.18, QColor(10, 70, 255));
                SetColorStop(0.28, QColor(10, 10, 190));
                SetColorStop(0.5, QColor(0, 0, 0));
                SetColorStop(0.72, QColor(190, 10, 10));
                SetColorStop(0.82, QColor(255, 70, 10));
                SetColorStop(1, QColor(255, 255, 50));
                break;
            case gpSpectrum:
                SetInterpolation(ciHSV);
                SetColorStop(0, QColor(50, 0, 50));
                SetColorStop(0.15, QColor(0, 0, 255));
                SetColorStop(0.35, QColor(0, 255, 255));
                SetColorStop(0.6, QColor(255, 255, 0));
                SetColorStop(0.75, QColor(255, 30, 0));
                SetColorStop(1, QColor(50, 0, 0));
                break;
            case gpJet:
                SetInterpolation(ciRGB);
                SetColorStop(0, QColor(0, 0, 100));
                SetColorStop(0.15, QColor(0, 50, 255));
                SetColorStop(0.35, QColor(0, 255, 255));
                SetColorStop(0.65, QColor(255, 255, 0));
                SetColorStop(0.85, QColor(255, 30, 0));
                SetColorStop(1, QColor(100, 0, 0));
                break;
            case gpHues:
                SetInterpolation(ciHSV);
                SetColorStop(0, QColor(255, 0, 0));
                SetColorStop(1.0/3.0, QColor(0, 0, 255));
                SetColorStop(2.0/3.0, QColor(0, 255, 0));
                SetColorStop(1, QColor(255, 0, 0));
                break;
        }
    }

    void TColorGradient::ClearColorStops()
    {
        colorStops.clear();
        isBufferInvalidated = true;
    }

    void TColorGradient::SetInterpolation(TColorInterpolation value)
    {
        if(interpolation != value)
        {
            interpolation = value;
            isBufferInvalidated = true;
        }
    }

    void TColorGradient::SetColorStop(double pos, const QColor &color)
    {
        colorStops.insert(pos, color);
        isBufferInvalidated = true;
    }

    void TColorGradient::Colorize(const double *data, const TRange &range, QRgb *scanLine, int n, int dataIndexFactor,
                                  bool logarithmic)
    {
        if(data == nullptr || scanLine == nullptr) return;
        if(isBufferInvalidated)
            UpdateBuffer();
        if(logarithmic == false)
        {
            double posToIndexFactor = (levelCount - 1) / range.Size();
            if(isPeriodic)
                for(int i = 0; i < n; ++i)
                {
                    int index = int((data[dataIndexFactor * i] - range.lower) * posToIndexFactor) % levelCount;
                    if(index < 0)
                        index += levelCount;
                    scanLine[i] = buffer[index];
                }
            else
                for(int i = 0; i < n; ++i)
                {
                    int index = (data[dataIndexFactor * i] - range.lower) * posToIndexFactor;
                    if(index < 0)
                        index = 0;
                    else if(index >= levelCount)
                        index = levelCount - 1;
                    scanLine[i] = buffer[index];
                }
        }
        else
        {
            //TODO log
        }
    }

    void
    TColorGradient::Colorize(const double *data, const unsigned char *alpha, const TRange &range, QRgb *scanLine, int n,
                             int dataIndexFactor, bool logarithmic)
    {
        if(data == nullptr || alpha == nullptr || scanLine == nullptr)
            return;
        if(isBufferInvalidated)
            UpdateBuffer();

        if(logarithmic == false)
        {
            double posToIndexFactor = (levelCount - 1) / range.Size();
            if(isPeriodic)
                for(int i = 0; i < n; ++i)
                {
                    int index = int((data[dataIndexFactor * i] - range.lower) * posToIndexFactor) % levelCount;
                    if(index < 0)
                        index += levelCount;
                    if(alpha[dataIndexFactor * i] == 255)
                        scanLine[i] = buffer[index];
                    else
                    {
                        QRgb rgb = buffer[index];
                        float alphaF = alpha[dataIndexFactor * i] / 255.0f;
                        scanLine[i] = qRgba(qRed(rgb) * alphaF, qGreen(rgb) * alphaF, qBlue(rgb) * alphaF, qAlpha(rgb) * alphaF);
                    }
                }
            else
                for(int i = 0; i < n; ++i)
                {
                    int index = (data[dataIndexFactor * i] - range.lower) * posToIndexFactor;
                    if(index < 0)
                        index = 0;
                    else if(index >= levelCount)
                        index = levelCount - 1;
                    if(alpha[dataIndexFactor * i] == 255)
                        scanLine[i] = buffer[index];
                    else
                    {
                        QRgb rgb = buffer[index];
                        float alphaF = alpha[dataIndexFactor * i] / 255.0f;
                        scanLine[i] = qRgba(qRed(rgb) * alphaF, qGreen(rgb) * alphaF, qBlue(rgb) * alphaF, qAlpha(rgb) * alphaF);
                    }
                }
        }
        else
        {
            //TODO log
        }

    }

    void TColorGradient::UpdateBuffer()
    {
        if(buffer.size() != levelCount)
            buffer.resize(levelCount);
        if(colorStops.size() > 1)
        {
            double indexToPosFactor = 1.0 / double(levelCount - 1);
            bool isUseAlpha = StopsUseAlpha();
            for(int  i = 0; i < levelCount; ++i)
            {
                double pos = i * indexToPosFactor;
                auto it = colorStops.lowerBound(pos);
                if(it == colorStops.end())
                {
                    if(isUseAlpha)
                    {
                        QColor col = (it - 1).value();
                        float alphaPremult = col.alpha() / 255.0f;
                        buffer[i] = qRgba(col.red() * alphaPremult, col.green() * alphaPremult, col.blue() * alphaPremult, col.alpha());
                    }
                    else
                        buffer[i] = (it - 1).value().rgba();
                }
                else if(it == colorStops.begin())
                {
                    if(isUseAlpha)
                    {
                        QColor col = it.value();
                        float alphaPremult = col.alpha() / 255.0f;
                        buffer[i] = qRgba(col.red() * alphaPremult, col.green() * alphaPremult, col.blue() * alphaPremult, col.alpha());
                    }
                    else
                        buffer[i] = it.value().rgba();
                }
                else
                {
                    auto high = it;
                    auto low = it - 1;
                    double t = (pos - low.key()) / (high.key() - low.key());
                    switch(interpolation)
                    {
                        case ciRGB:
                        {
                            if (isUseAlpha)
                            {
                                int alpha = (1 - t) * low.value().alpha() + t * high.value().alpha();
                                float alphaPremult = alpha / 255.0f;
                                buffer[i] = qRgba(
                                        ((1 - t) * low.value().red() + t * high.value().red()) * alphaPremult,
                                        ((1 - t) * low.value().green() + t * high.value().green()) * alphaPremult,
                                        ((1 - t) * low.value().blue() + t * high.value().blue()) * alphaPremult,
                                        alpha);
                            }
                            else
                            {
                                buffer[i] = qRgb(
                                        (1 - t) * low.value().red() + t * high.value().red(),
                                        (1 - t) * low.value().green() + t * high.value().green(),
                                        (1 - t) * low.value().blue() + t * high.value().blue());
                            }
                            break;
                        }
                        case ciHSV:
                        {
                            QColor lowHsv = low.value().toHsv();
                            QColor highHsv = high.value().toHsv();
                            double hue = 0;
                            double hueDiff = highHsv.hueF() - lowHsv.hueF();
                            if (hueDiff > 0.5)
                                hue = lowHsv.hueF() - t * (1.0 - hueDiff);
                            else if (hueDiff < -0.5)
                                hue = lowHsv.hueF() + t * (1.0 + hueDiff);
                            else
                                hue = lowHsv.hueF() + t * hueDiff;
                            if (hue < 0) hue += 1.0;
                            else if (hue >= 1.0) hue -= 1.0;
                            if (isUseAlpha)
                            {
                                const QRgb rgb = QColor::fromHsvF(hue,
                                                                  (1 - t) * lowHsv.saturationF() +
                                                                  t * highHsv.saturationF(),
                                                                  (1 - t) * lowHsv.valueF() +
                                                                  t * highHsv.valueF()).rgb();
                                const float alpha = (1 - t) * lowHsv.alphaF() + t * highHsv.alphaF();
                                buffer[i] = qRgba(qRed(rgb) * alpha, qGreen(rgb) * alpha, qBlue(rgb) * alpha,
                                                        255 * alpha);
                            }
                            else
                            {
                                buffer[i] = QColor::fromHsvF(hue,
                                                                   (1 - t) * lowHsv.saturationF() +
                                                                   t * highHsv.saturationF(),
                                                                   (1 - t) * lowHsv.valueF() +
                                                                   t * highHsv.valueF()).rgb();
                            }

                            break;
                        }
                    }
                }
            }
        }
        else
        {
            QRgb res = qRgb(0, 0, 0);
            if(colorStops.size() == 1)
            {
                QRgb rgb = colorStops.begin().value().rgb();
                float alpha = colorStops.begin().value().alphaF();
                QRgb res = qRgba(qRed(rgb) * alpha, qGreen(rgb) * alpha, qBlue(rgb) * alpha, 255 * alpha);
            }
            for(auto& r : buffer)
                r = res;
        }
        isBufferInvalidated = false;
    }

    bool TColorGradient::StopsUseAlpha()
    {
        for(auto it = colorStops.begin(); it != colorStops.end(); ++it)
            if(it.value().alpha() < 255)
                return true;
        return false;
    }
}









