//
// Created by user on 07.05.2020.
//

#ifndef NEO_PLOTQT_H
#define NEO_PLOTQT_H

#include <QWidget>
#include <QPainter>
#include <QMouseEvent>
//#include <QPrinter>
#include "Plot/Plot.h"
#include "Plot/Plottables.h"

namespace Plot {
    class TPixmap : public QPixmap{
    public:
        TPixmap() = default;
        TPixmap(int w, int h):QPixmap(w, h){}
    };

    class TRegion : public QRegion{
    public:
        TRegion():QRegion(){};
        TRegion(const QRegion &region):QRegion(region){};
    };
}
class TPaintBufferPixmap : public Plot::TPaintBuffer{
public:
    TPaintBufferPixmap(const Plot::TSize& size, double pixelRatio);

    virtual Plot::TUPtrPainter StartPaint() override;
    virtual void Draw(const Plot::TUPtrPainter& painter, int x, int y) override ;
    virtual void Clear(const TColor& color) override;
protected:
    Plot::TPixmap buffer;
    double devicePixelRatio;
    virtual void ReallocBuffer() override;
};

class TPainterQt : public QPainter, public Plot::TPainter{
public:
    TPainterQt();
    TPainterQt(QPaintDevice *device);

    void Save() override;
    void Restore() override;
    void SetClipRect(const Plot::TRect& value) override;
    void SetClipPolygon(const Plot::TVecPointF& points, bool isSub) override;

    Plot::TRegion ClipRegion() const override;
    bool IsActive() const override;

    bool Antialiasing() const override;
    void SetAntialiasing(bool value) override;

    void Offset(int x, int y) override;

    Plot::TPen Pen() const override;
    Plot::TBrush Brush() const override;

    void SetPen(const Plot::TPen& value) override;
    void SetBrush(const Plot::TBrush& value) override;
    void SetFont(const Plot::TFont& value) override;

    void DrawRect(const Plot::TRect& rect) override;
    void FillRect(const Plot::TRect& rect, const TColor& color) override;

    void DrawPixmap(int x, int y, const Plot::TPixmap& value) override;

    void DrawLine(const Plot::TPointF& b, const Plot::TPointF& e) override;
    void DrawLine(const Plot::TPoint& b, const Plot::TPoint& e) override;

    void DrawEllipse(const Plot::TPointF& center, double rx, double ry) override;

    void DrawText(const Plot::TPointF& p, const TString& value, Plot::TAlignText horz, Plot::TAlignText vert, double rotate) override;
    void DrawPolyline(const Plot::TPointF* points, int pointCount) override;
    void DrawPolygon(const Plot::TPointF* points, int pointCount) override;

    int WidthText(const TString& value) override;
    int HeightText(const TString& value) override;
protected:
    bool antialiasing = false;
};

inline QRect To(const Plot::TRect& rect)
{
    return QRect(rect.left(), rect.top(), rect.width(), rect.height());
}

inline QRectF To(const Plot::TRectF& rect)
{
    return QRectF(rect.left(), rect.top(), rect.width(), rect.height());
}

inline QPoint To(const Plot::TPoint& p)
{
    return QPoint(p.x(), p.y());
}

inline QPointF To(const Plot::TPointF& p)
{
    return QPointF(p.x(), p.y());
}

inline QSize To(const Plot::TSize& s)
{
    return QSize(s.width(), s.height());
}

inline QSizeF To(const Plot::TSizeF& s)
{
    return QSizeF(s.width(), s.height());
}

#include <QApplication>

template<typename TBasePlot>
class TPlotQt : public QWidget, public TBasePlot{
public:
    TPlotQt(QWidget* parent = nullptr, bool isDefault = true)
        :QWidget(parent), TBasePlot(isDefault)
    {
        setAttribute(Qt::WA_NoMousePropagation);
        setAttribute(Qt::WA_OpaquePaintEvent);
        setFocusPolicy(Qt::ClickFocus);
        setMouseTracking(true);

        TBasePlot::SetViewport(rect());
        if(isDefault)
            TBasePlot::Replot();
    }

    virtual void NativePaintNow() override
    {
        //перерисовывает виджет сразу вызывая событие paintEvent
        repaint();
    }
    virtual void NativePaintLater() override
    {
        //ставит в очередь на перерисовку виджета
        //уменьшается нагрузка т.к. для большого количества перерисовок обычно вызовется одна
        update();
    }

    virtual void NativeResize(const Plot::TSize& newSize) override
    {
        //устанавливает новые размеры для виджета путем вызова события resizeEvent
        resize(QSize(newSize.width(), newSize.height()));
    }

    virtual void NativeSetMinSize(const Plot::TSize& newSize)
    {
        //устанавливает минимальные размеры для виджета
        setMinimumSize(QSize(newSize.width(), newSize.height()));
    }

    Plot::TSize CalcSaveSize() const override;
    bool SavePdf(const TString& path, int newWidth, int newHeight) override;
    bool SavePng(const TString& path, int newWidth, int newHeight) override;
    bool SaveRastered(const TString& path, const TString& format, int newWidth, int newHeight);
    QPixmap ToPixmap(int newWidth, int newHeight);
protected:
    QPointF offsetPainter{0,0};
    virtual void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE
    {
        TBasePlot::PlotPaint();
    }
    virtual void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE
    {
        TBasePlot::SetViewport(rect());
        TBasePlot::Replot();
    }

    Plot::TMouseInfo MouseInfo(QMouseEvent *event)
    {
        Plot::TMouseInfoWheel info(event->pos() - offsetPainter, event->globalPos() - offsetPainter);
        info.isLeftButton = event->button() == Qt::LeftButton;
        return info;
    }

    Plot::TMouseInfo MouseInfo(QWheelEvent *event)
    {
        Plot::TMouseInfoWheel info(event->position() - offsetPainter, event->globalPosition() - offsetPainter);
        info.delta = event->angleDelta().y();
        return info;
    }

    template<typename T>
    Plot::TMouseInfo GetMouseInfo(T *event)
    {
        Plot::TMouseInfo info = MouseInfo(event);
        info.isCtrl = event->modifiers().testFlag(Qt::ControlModifier);
        info.isAlt = event->modifiers().testFlag(Qt::AltModifier);
        info.isShift = event->modifiers().testFlag(Qt::ShiftModifier);
        return info;
    }

    virtual void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE
    {
        Plot::TMouseInfo info = GetMouseInfo(event);
        TBasePlot::MousePress(info);
        event->accept();
    }

    virtual void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE
    {
        Plot::TMouseInfo info = GetMouseInfo(event);
        TBasePlot::MouseMove(info);
        event->accept();
    }

    virtual void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE
    {
        Plot::TMouseInfo info = GetMouseInfo(event);
        TBasePlot::MouseUp(info);
        event->accept();
    }

    virtual void wheelEvent(QWheelEvent *event) Q_DECL_OVERRIDE
    {
        Plot::TMouseInfo info = GetMouseInfo(event);
        TBasePlot::MouseWheel(info);
        if(info.IsMoving())
            event->accept();
        else
            qApp->sendEvent(parentWidget(), event);
    }

    virtual void mouseDoubleClickEvent(QMouseEvent *event) Q_DECL_OVERRIDE
    {
        Plot::TMouseInfo info = GetMouseInfo(event);
        TBasePlot::MouseDblClick(info);
        event->accept();
    }

    virtual Plot::TPtrPaintBuffer CreatePaintBuffer(const Plot::TSize &sizeBuffer) override
    {
        if(sizeBuffer.IsEmpty())
            return Plot::TPtrPaintBuffer(new TPaintBufferPixmap(TBasePlot::viewport.size(), 1.));//если размеры не указаны то создаем буфер по текущим размерам plot
        else
            return Plot::TPtrPaintBuffer(new TPaintBufferPixmap(sizeBuffer, 1.));//иначе создаем буфер по указаным размерам
    }
    virtual Plot::TUPtrPainter CreatePainter() override
    {
        return Plot::TUPtrPainter(new TPainterQt(this));//создаем painter который рисует по данному виджету
    }

    virtual void SetCursor(Plot::TCursorType value)
    {
        switch(value)
        {
            case Plot::ctHDrag: setCursor(Qt::SplitHCursor); break;
            case Plot::ctVDrag: setCursor(Qt::SplitVCursor); break;
            default: setCursor(Qt::ArrowCursor); break;
        }
    }
};
template <typename TBasePlot>
bool TPlotQt<TBasePlot>::SavePdf(const TString& path, int newWidth, int newHeight)
{
    bool res = false;
   /* QPrinter printer(QPrinter::ScreenResolution);
    printer.setOutputFileName(STR(path));
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setColorMode(QPrinter::Color);

    auto oldViewport = TBasePlot::viewport;
    TBasePlot::SetViewport(Plot::TRect(0, 0, newWidth, newHeight));

    QPageLayout pageLayout;
    pageLayout.setMode(QPageLayout::FullPageMode);
    pageLayout.setOrientation(QPageLayout::Portrait);
    pageLayout.setMargins({0,0,0,0});
    pageLayout.setPageSize(QPageSize(To(TBasePlot::viewport.size()), QPageSize::Point, QString(), QPageSize::ExactMatch));
    printer.setPageLayout(pageLayout);

    TPainterQt* painter = new TPainterQt();
    std::unique_ptr<Plot::TPainter> printPainter(painter);
    if(painter->begin(&printer))
    {
        painter->SetMode(Plot::TPainter::pmVectorized);
        painter->SetMode(Plot::TPainter::pmNoCaching);
        painter->SetMode(Plot::TPainter::pmNonCosmetic);
        painter->setWindow(To(TBasePlot::viewport));
        TBasePlot::Draw(printPainter);
        painter->end();
        res = true;
    }
    TBasePlot::SetViewport(oldViewport);*/
    return res;
}

template<typename TBasePlot>
bool TPlotQt<TBasePlot>::SavePng(const TString &path, int newWidth, int newHeight)
{
    return SaveRastered(path, "PNG", newWidth, newHeight);
}

template<typename TBasePlot>
bool TPlotQt<TBasePlot>::SaveRastered(const TString &path, const TString &format, int newWidth, int newHeight)
{
    if(newWidth == 0) newWidth = width();
    if(newHeight == 0) newHeight = height();
    QImage buffer = ToPixmap(newWidth, newHeight).toImage();
    int dotsPerMeter = 96;
    int quality = -1;
    int resolution = 96;
    //QCP::ResolutionUnit resolutionUnit=QCP::ruDotsPerInch

    buffer.setDotsPerMeterX(dotsPerMeter);
    buffer.setDotsPerMeterY(dotsPerMeter);

    if(buffer.isNull() == false)
        return buffer.save(STR(path), format.c_str(), quality);
    return false;
}

template<typename TBasePlot>
QPixmap TPlotQt<TBasePlot>::ToPixmap(int newWidth, int newHeight)
{
    if(newWidth == 0) newWidth = width();
    if(newHeight == 0)  newHeight = height();
    QPixmap res(newWidth, newHeight);

    TPainterQt* painter = new TPainterQt();
    std::unique_ptr<Plot::TPainter> ptrPainter(painter);
    painter->begin(&res);
    if(painter->isActive())
    {
        auto oldViewport = TBasePlot::viewport;
        TBasePlot::SetViewport(Plot::TRect(0, 0, newWidth, newHeight));
        painter->SetMode(Plot::TPainter::pmNoCaching);
        painter->fillRect(To(TBasePlot::viewport), Qt::white);
        TBasePlot::Draw(ptrPainter);
        TBasePlot::SetViewport(oldViewport);
        painter->end();
    }
    return res;
}

template<typename TBasePlot>
Plot::TSize TPlotQt<TBasePlot>::CalcSaveSize() const
{
    return Plot::TSize(width(), height());
}


using TSimplePlotQt = TPlotQt<Plot::TPlot>;

namespace Plot{

    using TVecRgb = std::vector<QRgb>;

    class TColorGradient{
    public:
        TColorGradient() = default;
        TColorGradient(TGradientPreset preset);

        inline int LevelCount() const { return levelCount; }
        inline TColorInterpolation Interpolation() const { return interpolation; }
        inline bool IsPeriodic() const { return isPeriodic; }
        inline bool IsBufferInvalidated() const { return isBufferInvalidated; }

        void SetColorStop(double pos, const QColor& color);
        void SetInterpolation(TColorInterpolation value);

        void Colorize(const double* data, const TRange& range, QRgb* scanLine, int n, int dataIndexFactor = 1, bool logarithmic = false);
        void Colorize(const double* data, const unsigned char* alpha, const TRange& range, QRgb* scanLine, int n, int dataIndexFactor = 1, bool logarithmic = false);

        void LoadPreset(TGradientPreset preset);
        void ClearColorStops();

    protected:
        int levelCount = 350;
        TColorInterpolation interpolation = ciRGB;
        bool isPeriodic = false;
        QMap<double, QColor> colorStops;

        TVecRgb buffer = TVecRgb(levelCount, qRgb(0, 0, 0));
        bool isBufferInvalidated = true;
        void UpdateBuffer();
        bool StopsUseAlpha();
    };


    class TColorMap : public TColorMapBase {
    public:
        TColorMap(const TPtrAxis& key, const TPtrAxis& val);

        void SetPreset(TGradientPreset value) override;

    protected:

        TColorGradient gradient{preset};
        QImage mapImage;
        QImage underMapImage;

        void UpdateMapImage();
        virtual void Draw(const TUPtrPainter& painter) override;

        inline int OversamplingFactor(double value){ return (isInterpolate) ? 1 : int(1.0 + 100./ value); }
    };

    class TOptimizedColorMap : public TColorMap{
    public:
        TOptimizedColorMap(const TPtrAxis& key, const TPtrAxis& val):TColorMap(key, val){}
        void SetHeaderHeight(size_t value) override;
        void SetPreset(TGradientPreset value) override;
    protected:
        TRange bufferRange;
        QImage headerImage;
        sigslot::scoped_connection headerConnect;
        void UpdateOptimizedImage();
        virtual void Draw(const TUPtrPainter& painter) override;
        bool IsNeedUpdateBuffer();

        void DrawHeaderImage();
    };
}

#endif //NEO_PLOTQT_H
