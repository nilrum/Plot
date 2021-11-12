//
// Created by user on 05.02.2021.
//

#ifndef ATLAS_MOCKPLOT_H
#define ATLAS_MOCKPLOT_H


namespace Plot {

    class TRegion{};

    class TPainterMock : public TPainter{
    public:
        void Save() override {};
        void Restore() override {};
        void SetClipRect(const TRect& value) override {};
        void SetClipPolygon(const TVecPointF& points, bool isSub) override {};
        TRegion ClipRegion() const override { return TRegion(); };
        bool IsActive() const  override { return false; };

        bool Antialiasing() const override { return false; };
        void SetAntialiasing(bool value) override {};
        void Offset(int x, int y) override{};

        TPen Pen() const override { return TPen(); };
        TBrush Brush() const override { return TBrush(); };

        void SetPen(const TPen& value) override {};
        void SetBrush(const TBrush& value) override {};
        void SetFont(const TFont& value) override {};
        void DrawRect(const Plot::TRect& rect) override {};
        void FillRect(const TRect& rect, const TColor& color) override {};

        void DrawPixmap(int x, int y, const TPixmap& value) override {};
        void DrawLine(const TPointF& b, const TPointF& e) override {};
        void DrawLine(const TPoint& b, const TPoint& e) override {};
        void DrawEllipse(const TPointF& center, double rx, double ry){};
        void DrawText(const TPointF& p, const TString& value, TAlignText horz, TAlignText vert, double rotate) override {};
        void DrawPolyline(const TPointF* points, int pointCount) override {};
        void DrawPolygon(const TPointF* points, int pointCount)  override {};

        int WidthText(const TString& value) override { return 0; };
        int HeightText(const TString& value) override { return 0; };
    };

    class TPaintBufferMock : public TPaintBuffer{
    public:
        TPaintBufferMock():TPaintBuffer(TSize()){};
        TUPtrPainter StartPaint() override{ return std::make_unique<TPainterMock>(); };
        void Draw(const TUPtrPainter& painter, int x, int y)override{};
        void Clear(const TColor& color) override{};
    protected:
        void ReallocBuffer() override { invalidate = true; }
    };

    class TPlotMock : public TPlot{
    public:
        TPlotMock(bool isDefault = true): TPlot(isDefault){}
        void NativePaintNow() override{}
        void NativePaintLater() override{}

        void NativeResize(const Plot::TSize& newSize) override
        {
            SetViewport(TRect(plotLayout->OutRect().topLeft(), newSize));
            Replot();
        }
        void NativeSetMinSize(const Plot::TSize& newSize) override{}

    protected:
        Plot::TPtrPaintBuffer CreatePaintBuffer(const Plot::TSize &sizeBuffer) override
        {
            return Plot::TPtrPaintBuffer(new TPaintBufferMock());
        }
        Plot::TUPtrPainter CreatePainter() override
        {
            return Plot::TUPtrPainter();
        }
        void SetCursor(Plot::TCursorType value) override{}
    };
/*
    class TDepthPlotMock : public TDepthPlot{
    public:
        void NativePaintNow() override{}
        void NativePaintLater() override{}

        void NativeResize(const Plot::TSize& newSize) override
        {
            SetViewport(TRect(plotLayout->OutRect().topLeft(), newSize));
            Replot();
        }
        void NativeSetMinSize(const Plot::TSize& newSize) override{}

    protected:
        Plot::TPtrPaintBuffer CreatePaintBuffer(const Plot::TSize &sizeBuffer) override
        {
            return Plot::TPtrPaintBuffer(new TPaintBufferMock());
        }
        Plot::TUPtrPainter CreatePainter() override
        {
            return Plot::TUPtrPainter();
        }
        void SetCursor(Plot::TCursorType value) override{}
    };*/
}

#endif //ATLAS_MOCKPLOT_H
