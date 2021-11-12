//
// Created by user on 05.04.2021.
//

#include "gtest/gtest.h"
#include "../Plot.h"
#include "MockPlot.h"

using namespace Plot;

namespace Plot {

    TColor TConsts::TransparentColor(){ return 0; }
    TColor TConsts::WhiteColor(){ return 0; }
    TColor TConsts::BlackColor(){ return 0; }
    TColor TConsts::BlueColor(){ return 0; }

    int TConsts::GetAlpha(const TColor &value){ return 0; }
    TColor TConsts::SetAlpha(const TColor &value, int alpha){ return 0; }

    TColor TConsts::RandColor(){ return 0; }

    TColor TConsts::CreateColor(int r, int g, int b, int a)
    {
        return 0;
    }
}


TEST(Creates, Init)
{
    TPlotMock plot;

    //для создания axisRects по умолчанию нужен только plot
    auto axisRect = plot.AddAxisRect();

    auto layGrid = plot.CreateLayout<TLayoutGrid>();
    auto layStack = plot.CreateLayout<TLayoutStack>(orHorz);

    auto leftAxis = axisRect->AddAxis(atLeft);
    auto bottomAxis = axisRect->AddAxis(atBottom);

    auto graph = plot.AddPlottable<TGraph>(leftAxis, bottomAxis);

}

TEST(Plot, Init)
{
    TPlotMock plot(false);
    plot.SetViewport(TRect(0, 0, 300, 400));
    plot.Replot();

    EXPECT_EQ(plot.CountLayers(), 5);
    EXPECT_EQ(plot.CountAxisRect(), 0);
    EXPECT_EQ(plot.CountPlottables(), 0);
    EXPECT_EQ(plot.SelectionTolerance(), 8);

    auto plotLay = plot.PlotLayout();
    EXPECT_EQ(plotLay != nullptr, true);
    EXPECT_EQ(plotLay->CountElements(), 2);//один пустой верхний и один mainLayout
    EXPECT_EQ(plotLay->OutRect(), TRect(0, 0, 300, 400));//размеры остались теже т.к. AutoSize не включен
    EXPECT_EQ(plotLay->MinSize(), TSize(0, 0));
    EXPECT_EQ(plotLay->MaxSize(), TSize(0, 0));

    auto mainLay = plot.MainLayout();
    EXPECT_EQ(mainLay != nullptr, true);
    EXPECT_EQ(mainLay->CountElements(), 0);             //нет вложенных элементов
    EXPECT_EQ(mainLay->OutRect(), TRect(0, 0, 0, 0));   //размеры нулевые т.к. это вложенный Layout и у него нет элементов
    EXPECT_EQ(mainLay->MinSize(), TSize(0, 0));
    EXPECT_EQ(mainLay->MaxSize(), TSize(0, 0));

    auto rect = plot.AddAxisRect();
    plot.Replot();

    EXPECT_EQ(plotLay->CountElements(), 2);//один пустой верхний и один mainLayout
    EXPECT_EQ(plotLay->OutRect(), TRect(0, 0, 300, 400));
    EXPECT_EQ(plotLay->MinSize(), TSize(40, 40));           //т.к. есть AxisRect у которого фиксированы размеры шкал
    EXPECT_EQ(plotLay->MaxSize(), TSize(INT_MAX, INT_MAX));

    EXPECT_EQ(mainLay->CountElements(), 1);                 //один axisRects
    EXPECT_EQ(mainLay->OutRect(), TRect(0, 0, 300, 400));
    EXPECT_EQ(mainLay->MinSize(), TSize(40, 40));           //т.к. есть AxisRect у которого фиксированы размеры шкал
    EXPECT_EQ(mainLay->MaxSize(), TSize(INT_MAX, INT_MAX));

    EXPECT_EQ(rect->CountElements(), 9);             //вложенные элементы под шкалы
    EXPECT_EQ(rect->OutRect(), TRect(0, 0, 300, 400));
    EXPECT_EQ(rect->MinSize(), TSize(40, 40));
    EXPECT_EQ(rect->MaxSize(), TSize(INT_MAX, INT_MAX));

    auto leftAxis = rect->Axis(atLeft);
    EXPECT_EQ(leftAxis->OutRect(), TRect(0, 0, 40, 360));
    EXPECT_EQ(leftAxis->MinSize(), TSize(40, 0));
    EXPECT_EQ(leftAxis->MaxSize(), TSize(40, INT_MAX));

    auto bottomAxis = rect->Axis(atBottom);
    EXPECT_EQ(bottomAxis->OutRect(), TRect(40, 360, 260, 40));
    EXPECT_EQ(bottomAxis->MinSize(), TSize(0, 40));
    EXPECT_EQ(bottomAxis->MaxSize(), TSize(INT_MAX, 40));
}

TEST(Plot, InitArrayLay)
{
    TPlotMock plot(false);
    plot.SetViewport(TRect(0, 0, 300, 400));
    auto plotLay = plot.PlotLayout();

    auto rect = plot.AddAxisRect();
    auto grid = plot.CreateLayout<TLayoutGrid>();
    plot.MainLayout()->AddElement(grid);
    grid->SetMaxAddCols(1);//делаем вертикальное выравнивание(максимум один элемент в строке)

    auto sub1 = plot.CreateLayout<TAxisRect>();
    grid->AddElement(sub1);
    plot.AddAxisRect(sub1);
    sub1->SetIsVisible(false);

    plot.Replot();

    //т.к. второй axisRects не видим
    EXPECT_EQ(plotLay->OutRect(), TRect(0, 0, 300, 400));
    EXPECT_EQ(plotLay->MinSize(), TSize(40, 40));
    EXPECT_EQ(plotLay->MaxSize(), TSize(INT_MAX, INT_MAX));

    EXPECT_EQ(rect->OutRect(), TRect(0, 0, 300, 400));
    EXPECT_EQ(rect->MinSize(), TSize(40, 40));
    EXPECT_EQ(rect->MaxSize(), TSize(INT_MAX, INT_MAX));

    EXPECT_EQ(rect->Axis(atLeft)->OutRect(), TRect(0, 0, 40, 360));
    EXPECT_EQ(rect->Axis(atBottom)->OutRect(), TRect(40, 360, 260, 40));

    sub1->SetIsVisible(true);
    plot.Replot();
    //второй виден и минимум обновился
    EXPECT_EQ(plotLay->OutRect(), TRect(0, 0, 300, 400));
    EXPECT_EQ(plotLay->MinSize(), TSize(80, 40));
    EXPECT_EQ(plotLay->MaxSize(), TSize(INT_MAX, INT_MAX));

    EXPECT_EQ(rect->OutRect(), TRect(0, 0, 150, 400));
    EXPECT_EQ(rect->MinSize(), TSize(40, 40));
    EXPECT_EQ(rect->MaxSize(), TSize(INT_MAX, INT_MAX));

    EXPECT_EQ(rect->Axis(atLeft)->OutRect(), TRect(0, 0, 40, 360));
    EXPECT_EQ(rect->Axis(atBottom)->OutRect(), TRect(40, 360, 110, 40));

    EXPECT_EQ(sub1->OutRect(), TRect(150, 0, 150, 400));
    EXPECT_EQ(sub1->MinSize(), TSize(40, 40));
    EXPECT_EQ(sub1->MaxSize(), TSize(INT_MAX, INT_MAX));

    EXPECT_EQ(sub1->Axis(atLeft)->OutRect(), TRect(150, 0, 40, 360));
    EXPECT_EQ(sub1->Axis(atBottom)->OutRect(), TRect(190, 360, 110, 40));

    //добавляем еще один axisRects в правый столбец

    auto sub2 = plot.CreateLayout<TAxisRect>();
    grid->AddElement(sub2);
    plot.AddAxisRect(sub2);
    sub2->SetIsVisible(false);
    plot.Replot();

    //третий axisRects не видим, это значит размеры не должны были поменяться
    EXPECT_EQ(plotLay->OutRect(), TRect(0, 0, 300, 400));
    EXPECT_EQ(plotLay->MinSize(), TSize(80, 40));
    EXPECT_EQ(plotLay->MaxSize(), TSize(INT_MAX, INT_MAX));

    EXPECT_EQ(rect->OutRect(), TRect(0, 0, 150, 400));
    EXPECT_EQ(rect->MinSize(), TSize(40, 40));
    EXPECT_EQ(rect->MaxSize(), TSize(INT_MAX, INT_MAX));

    EXPECT_EQ(sub1->OutRect(), TRect(150, 0, 150, 400));
    EXPECT_EQ(sub1->MinSize(), TSize(40, 40));
    EXPECT_EQ(sub1->MaxSize(), TSize(INT_MAX, INT_MAX));

    //делаем видимым третий AxisRect
    sub2->SetIsVisible(true);
    plot.Replot();

    EXPECT_EQ(plotLay->OutRect(), TRect(0, 0, 300, 400));
    EXPECT_EQ(plotLay->MinSize(), TSize(80, 80));
    EXPECT_EQ(plotLay->MaxSize(), TSize(INT_MAX, INT_MAX));

    EXPECT_EQ(rect->OutRect(), TRect(0, 0, 150, 400));
    EXPECT_EQ(rect->MinSize(), TSize(40, 40));
    EXPECT_EQ(rect->MaxSize(), TSize(INT_MAX, INT_MAX));

    EXPECT_EQ(sub1->OutRect(), TRect(150, 0, 150, 200));
    EXPECT_EQ(sub1->MinSize(), TSize(40, 40));
    EXPECT_EQ(sub1->MaxSize(), TSize(INT_MAX, INT_MAX));

    EXPECT_EQ(sub2->OutRect(), TRect(150, 200, 150, 200));
    EXPECT_EQ(sub2->MinSize(), TSize(40, 40));
    EXPECT_EQ(sub2->MaxSize(), TSize(INT_MAX, INT_MAX));

    //уменьшаем размер по высоте близкой к минимальной
    plot.SetViewport(TRect(0, 0, 300, 100));
    plot.Replot();

    EXPECT_EQ(plotLay->OutRect(), TRect(0, 0, 300, 100));
    EXPECT_EQ(plotLay->MinSize(), TSize(80, 80));
    EXPECT_EQ(plotLay->MaxSize(), TSize(INT_MAX, INT_MAX));

    EXPECT_EQ(rect->OutRect(), TRect(0, 0, 150, 100));
    EXPECT_EQ(rect->MinSize(), TSize(40, 40));
    EXPECT_EQ(rect->MaxSize(), TSize(INT_MAX, INT_MAX));

    EXPECT_EQ(sub1->OutRect(), TRect(150, 0, 150, 50));
    EXPECT_EQ(sub1->MinSize(), TSize(40, 40));
    EXPECT_EQ(sub1->MaxSize(), TSize(INT_MAX, INT_MAX));

    EXPECT_EQ(sub2->OutRect(), TRect(150, 50, 150, 50));
    EXPECT_EQ(sub2->MinSize(), TSize(40, 40));
    EXPECT_EQ(sub2->MaxSize(), TSize(INT_MAX, INT_MAX));
}

TEST(Plot, Layers)
{
    TPlotMock plot;

    auto layGrid = plot.Layer(0);
    EXPECT_EQ(layGrid != nullptr, true);
    EXPECT_EQ(layGrid->Name(), "grid");
    EXPECT_EQ(layGrid->Mode(), TLayerMode::Logical);

    auto layBadIndex = plot.Layer(10);
    EXPECT_EQ(layBadIndex == nullptr, true);

    auto layOverlay = plot.Layer("overlay");
    EXPECT_EQ(layOverlay != nullptr, true);
    EXPECT_EQ(layOverlay->Name(), "overlay");
    EXPECT_EQ(layOverlay->Mode(), TLayerMode::Buffered);//данный слой буферный

    auto layBadName = plot.Layer("noname");
    EXPECT_EQ(layBadName == nullptr, true);

    auto layMain = plot.CurrentLayer();
    EXPECT_EQ(layMain != nullptr, true);
    EXPECT_EQ(layMain->Name(), "main");

    auto newLayer = plot.AddLayer("newLayer");
    EXPECT_EQ(newLayer != nullptr, true);
    EXPECT_EQ(newLayer->Name(), "newLayer");
}

TEST(Layer, SetParams)
{
    TPlotMock plot;
    auto layer = plot.Layer(0);
    layer->SetIsVisible(false);
    EXPECT_EQ(layer->IsVisible(), false);

    layer->SetMode(TLayerMode::Buffered);
    EXPECT_EQ(layer->Mode(), TLayerMode::Buffered);
}
namespace Plot {
    bool operator==(const TRect &one, const TRect &two)
    {
        return one.left() == two.left() &&
               one.top() == two.top() &&
               one.bottom() == two.bottom() &&
               one.right() == two.right();
    }
}

TEST(LayoutElement, Init)
{
    TPlotMock plot;
    auto elem = plot.CreateLayout<TLayoutElement>();
    ASSERT_EQ(elem != nullptr, true);
    EXPECT_EQ(elem->HasParent(), false);
    EXPECT_EQ(elem->Layout() == nullptr, true);
    EXPECT_EQ(elem->MinSize().width(), 0);
    EXPECT_EQ(elem->MinSize().height(), 0);
    EXPECT_EQ(elem->MaxSize().width(), INT_MAX);
    EXPECT_EQ(elem->MaxSize().height(), INT_MAX);
    EXPECT_EQ(elem->OutRect(), TRect());
    EXPECT_EQ(elem->InnerRect(), TRect());
    EXPECT_EQ(elem->Margins(), TMargins());
    EXPECT_EQ(elem->IsFixedWidth(), false);
    EXPECT_EQ(elem->IsFixedHeight(), false);
    EXPECT_EQ(elem->MarginGroup(atLeft) == nullptr, true);
    EXPECT_EQ(elem->MarginGroup(atTop) == nullptr, true);
}

TEST(LayoutElement, SetParams)
{
    TPlotMock plot;
    auto elem = plot.CreateLayout<TLayoutElement>();
    //устанавливаем минимальные и максимальные размера
    elem->SetMinSize(TSize(10, 20));
    elem->SetMaxSize(TSize(100, 200));

    //в текущем варианте проверки на min max не делаеются,
    //должен проверять родительский Layout
    //ставим меньше минимального
    elem->SetOutRect(TRect(5, 6, 7, 8));
    elem->Update();
    EXPECT_EQ(elem->MinSize(), TSize(10, 20));
    EXPECT_EQ(elem->MaxSize(), TSize(100, 200));

    EXPECT_EQ(elem->OutRect(), TRect(5, 6, 7, 8));
    EXPECT_EQ(elem->InnerRect(), TRect(5, 6, 7, 8));

    //ставим больше максимального
    elem->SetOutRect(TRect(50, 60, 700, 800));
    elem->Update();
    EXPECT_EQ(elem->OutRect(), TRect(50, 60, 700, 800));
    EXPECT_EQ(elem->InnerRect(), TRect(50, 60, 700, 800));

    //ставим в пределах минимального и максимального
    elem->SetOutRect(TRect(20, 30, 80, 90));
    elem->SetMargins(TMargins(1, 2, 3, 4));
    elem->Update();
    EXPECT_EQ(elem->OutRect(), TRect(20, 30, 80, 90));
    //значение посчитано сразу с margins, т.к. он не нулевой
    EXPECT_EQ(elem->InnerRect(), TRect(21, 32, 76, 84));

    EXPECT_EQ(elem->IsAutoMinMax().width(), true);
    EXPECT_EQ(elem->IsAutoMinMax().height(), true);

    elem->SetFixedWidth(100);
    EXPECT_EQ(elem->MinSize(), TSize(100, 20));
    EXPECT_EQ(elem->MaxSize(), TSize(100, 200));
    EXPECT_EQ(elem->IsAutoMinMax().width(), false);
    EXPECT_EQ(elem->IsAutoMinMax().height(), true);

    elem->SetFixedHeight(300);
    EXPECT_EQ(elem->MinSize(), TSize(100, 300));
    EXPECT_EQ(elem->MaxSize(), TSize(100, 300));
    EXPECT_EQ(elem->IsAutoMinMax().width(), false);
    EXPECT_EQ(elem->IsAutoMinMax().height(), false);
}

TEST(LayoutStack, Init)
{
    TPlotMock plot;
    auto stack = plot.CreateLayout<TLayoutStack>(orVert);
    ASSERT_EQ(stack != nullptr, true);

    EXPECT_EQ(stack->HasParent(), false);
    EXPECT_EQ(stack->Layout() == nullptr, true);
    EXPECT_EQ(stack->CountElements(), 0);
    EXPECT_EQ(stack->Element(0) == nullptr, true);
    EXPECT_EQ(stack->IndexElement(nullptr), -1);
    EXPECT_EQ(stack->Take(-1) == nullptr, true);
    EXPECT_EQ(stack->Stretch(-1), 0.);
    EXPECT_EQ(stack->MinSize(), TSize(0, 0));
    EXPECT_EQ(stack->MaxSize(), TSize(INT_MAX, INT_MAX));
}

TEST(LayoutStack, AddElements)
{
    TPlotMock plot;
    auto stack = plot.CreateLayout<TLayoutStack>(orVert);
    stack->SetOutRect(TRect(10, 20, 30, 40));
    auto elem1 = plot.CreateLayout<TLayoutElement>();
    stack->AddElement(elem1);

    EXPECT_EQ(stack->CountElements(), 1);
    EXPECT_EQ(stack->Stretch(0), 1);
    //если не был вызыван Update у stack то размеры не установлены
    EXPECT_EQ(elem1->OutRect(), TRect());

    stack->Update();
    //размеры элемента должны были обновится
    EXPECT_EQ(elem1->OutRect(), TRect(10, 20, 30, 40));

    //добавляем еще элемент
    auto elem2 = plot.CreateLayout<TLayoutElement>();
    stack->AddElement(elem2);
    stack->Update();

    EXPECT_EQ(stack->CountElements(), 2);
    EXPECT_EQ(stack->Stretch(1), 1);
    //проверка новых размеров
    EXPECT_EQ(elem1->OutRect(), TRect(10, 20, 30, 20));
    EXPECT_EQ(elem2->OutRect(), TRect(10, 40, 30, 20));
    //делаем первый элемент больше
    stack->SetStretch(0, 3);
    stack->Update();
    EXPECT_EQ(elem1->OutRect(), TRect(10, 20, 30, 30));
    EXPECT_EQ(elem2->OutRect(), TRect(10, 50, 30, 10));

    //устанавливаем максимальную высоту для второго элемента
    elem2->SetMaxSize(TSize(INT_MAX, 5));
    stack->Update();
    //первый элемент дополнительно растянется, т.к. он не фиксированный
    EXPECT_EQ(elem1->OutRect(), TRect(10, 20, 30, 35));
    EXPECT_EQ(elem2->OutRect(), TRect(10, 55, 30, 5));

    //установим максимальную высоту для первого элемента
    elem1->SetMaxSize(TSize(INT_MAX, 10));
    stack->Update();
    //элементы возьмут только максимальные размеры, остаток будет пустым
    EXPECT_EQ(elem1->OutRect(), TRect(10, 20, 30, 10));
    EXPECT_EQ(elem2->OutRect(), TRect(10, 30, 30, 5));
    EXPECT_EQ(stack->OutRect(), TRect(10, 20, 30, 40));

    //установим фиксированные размеры

    elem1->SetFixedHeight(15);
    elem2->SetFixedHeight(15);
    stack->Update();
    EXPECT_EQ(elem1->OutRect(), TRect(10, 20, 30, 15));
    EXPECT_EQ(elem2->OutRect(), TRect(10, 35, 30, 15));
    EXPECT_EQ(stack->OutRect(), TRect(10, 20, 30, 40));

    auto group = std::make_shared<TMarginGroup>(orVert);

    auto bigSize = plot.CreateLayout<TLayoutElement>();
    bigSize->SetOutRect(TRect(10, 20, 30, 40));
    bigSize->SetFixedHeight(10);
    bigSize->SetMarginGroup(atTop, group, bigSize);

    stack->SetMarginGroup(atTop, group, TPtrLayoutElement());
    stack->Update();
    //группа не меняет текущие внешние размеры
    EXPECT_EQ(stack->OutRect(), TRect(10, 20, 30, 40));
    //уменьшается внутренняя область: top и height
    EXPECT_EQ(stack->InnerRect(), TRect(10, 30, 30, 30));

    EXPECT_EQ(elem1->OutRect(), TRect(10, 30, 30, 15));
    EXPECT_EQ(elem2->OutRect(), TRect(10, 45, 30, 15));

}

TEST(LayoutGrid, Init)
{
    TPlotMock plot;
    auto grid = plot.CreateLayout<TLayoutGrid>();
    ASSERT_EQ(grid != nullptr, true);

    EXPECT_EQ(grid->HasParent(), false);
    EXPECT_EQ(grid->Layout() == nullptr, true);
    EXPECT_EQ(grid->CountElements(), 0);
    EXPECT_EQ(grid->Element(0) == nullptr, true);
    EXPECT_EQ(grid->IndexElement(nullptr), -1);
    EXPECT_EQ(grid->Take(-1) == nullptr, true);
    EXPECT_EQ(grid->MinSize(), TSize(0, 0));
    EXPECT_EQ(grid->MaxSize(), TSize(INT_MAX, INT_MAX));

    EXPECT_EQ(grid->CountRow(), 0);
    EXPECT_EQ(grid->CountCol(), 0);
    EXPECT_EQ(grid->RowStretches().size(), 0);
    EXPECT_EQ(grid->ColStretches().size(), 0);
    EXPECT_EQ(grid->RowSpacing(), 0);
    EXPECT_EQ(grid->ColSpacing(), 0);

    EXPECT_EQ(grid->Element(0, 0) == nullptr, true);
    EXPECT_EQ(grid->HasElement(0, 0), false);
    //после пересчета т.к. по умолчанию включен AutoMinMax и нет вложенных элементов мин и макс нулевые
    auto mm = grid->CalcMinMaxSizes();
    EXPECT_EQ(TSize(mm.first.width().get(), mm.first.height().get()), TSize(0, 0));
    EXPECT_EQ(TSize(mm.second.width().get(), mm.second.height().get()), TSize(0, 0));

    EXPECT_EQ(grid->MinSize(), TSize(0, 0));
    EXPECT_EQ(grid->MaxSize(), TSize(0, 0));
}
/*
TEST(LayoutGrid, IsAutoMinMaxAutoSize)
{
    TPlotMock plot(false);
    plot.SetViewport(TRect(10, 20, 30, 40));
    auto grid = plot.MainLayout();
    grid->SetIsAutoMinMax({false, false});
    //добавляем элемент просто по умолчанию
    auto el1 = grid->AddElement(plot.CreateLayout());
    plot.Replot();

    ASSERT_EQ(el1 != nullptr, true);
    EXPECT_EQ(el1->OutRect(), TRect(10, 20, 30, 40));
    EXPECT_EQ(el1->InnerRect(), TRect(10, 20, 30, 40));
    EXPECT_EQ(el1->HasParent(), true);
    EXPECT_EQ(el1->MinSize(), TSize(0, 0));
    EXPECT_EQ(el1->MaxSize(), TSize(INT_MAX, INT_MAX));

    //ставим минимальные размеры меньше текущих
    el1->SetMinSize(TSize(5, 7));
    plot.Replot();

    //размеры остались те же самые
    EXPECT_EQ(grid->OutRect(), TRect(10, 20, 30, 40));
    EXPECT_EQ(grid->MinSize(), TSize(0, 0));
    EXPECT_EQ(grid->MaxSize(), TSize(INT_MAX, INT_MAX));

    EXPECT_EQ(el1->OutRect(), TRect(10, 20, 30, 40));
    EXPECT_EQ(el1->MinSize(), TSize(5, 7));
    EXPECT_EQ(el1->MaxSize(), TSize(INT_MAX, INT_MAX));

    //ставим максим размеры меньше текущих
    el1->SetMaxSize(TSize(6, 9));
    plot.Replot();

    //размеры остались те же самые т.к. подстраивание под размеры вложенных элементов отключено
    EXPECT_EQ(grid->OutRect(), TRect(10, 20, 30, 40));
    EXPECT_EQ(grid->MinSize(), TSize(0, 0));
    EXPECT_EQ(grid->MaxSize(), TSize(INT_MAX, INT_MAX));
    //размеры не остались те же самые т.к. есть максимально возможные
    EXPECT_EQ(el1->OutRect(), TRect(10, 20, 6, 9));
    EXPECT_EQ(el1->MinSize(), TSize(5, 7));
    EXPECT_EQ(el1->MaxSize(), TSize(6, 9));

    //включаем подстраивание минимальных и максимальных размеров
    grid->SetIsAutoMinMax({true, true});
    plot.Replot();

    //размеры остались теже мин макс обновился
    EXPECT_EQ(grid->OutRect(), TRect(10, 20, 30, 40));
    EXPECT_EQ(grid->MinSize(), TSize(5, 7));
    EXPECT_EQ(grid->MaxSize(), TSize(6, 9));
    //размеры не остались те же самые т.к. есть максимально возможные
    EXPECT_EQ(el1->OutRect(), TRect(10, 20, 6, 9));
    EXPECT_EQ(el1->MinSize(), TSize(5, 7));
    EXPECT_EQ(el1->MaxSize(), TSize(6, 9));

    //включаем корректировку своих размеров исходя из мин
    grid->SetIsAutoSize(true);
    plot.Replot();

    //размеры поменялись подстроившись под вложенные элменты
    EXPECT_EQ(grid->OutRect(), TRect(10, 20, 6, 9));
    EXPECT_EQ(grid->MinSize(), TSize(5, 7));
    EXPECT_EQ(grid->MaxSize(), TSize(6, 9));
    //размеры не остались те же самые т.к. есть максимально возможные
    EXPECT_EQ(el1->OutRect(), TRect(10, 20, 6, 9));
    EXPECT_EQ(el1->MinSize(), TSize(5, 7));
    EXPECT_EQ(el1->MaxSize(), TSize(6, 9));
}

TEST(LayoutGrid, MinMaxAutoStack)
{
    TPlotMock plot;
    auto grid = plot.CreateLayout<TLayoutGrid>();
    grid->SetOutRect(TRect(10, 20, 30, 40));
    //добавляем элемент просто по умолчанию
    auto st1 = plot.CreateLayout<TLayoutStack>(orVert);
    grid->AddElement(st1);
    grid->Update();

    //размеры все теже остались
    EXPECT_EQ(grid->OutRect(), TRect(10, 20, 30, 40));
    EXPECT_EQ(grid->MinSize(), TSize());
    EXPECT_EQ(grid->MaxSize(), TSize(INT_MAX, INT_MAX));

    EXPECT_EQ(st1->OutRect(), TRect(10, 20, 30, 40));
    EXPECT_EQ(st1->MinSize(), TSize());
    EXPECT_EQ(st1->MaxSize(), TSize(INT_MAX, INT_MAX));

    //st1->SetIsAutoMinMax(true);
    grid->Update();
    //размеры все теже остались
    EXPECT_EQ(grid->OutRect(), TRect(10, 20, 30, 40));
    EXPECT_EQ(grid->MinSize(), TSize());
    EXPECT_EQ(grid->MaxSize(), TSize(INT_MAX, INT_MAX));

    //поменялся максимум высоты т.к. нет элементов и orVert то обнулился максимум
    //а так как элемент вложенный то и мин макс применяется к размеру элмента
    EXPECT_EQ(st1->OutRect(), TRect(10, 20, 30, 0));
    EXPECT_EQ(st1->MinSize(), TSize());
    EXPECT_EQ(st1->MaxSize(), TSize(INT_MAX, 0));

    auto el1 = st1->AddElement(plot.CreateLayout());
    el1->SetMinSize(TSize(5, 8));
    el1->SetMaxSize(TSize(7, 9));
    grid->Update();

    //размеры все теже остались так как IsAutoMinMax IsAutoSize не включены
    EXPECT_EQ(grid->OutRect(), TRect(10, 20, 30, 40));
    EXPECT_EQ(grid->MinSize(), TSize());
    EXPECT_EQ(grid->MaxSize(), TSize(INT_MAX, INT_MAX));

    //элемент не является корневым поэтому к нему применяется минимальные размеры вложенного элемента st1->IsAutoMinMax == true
    EXPECT_EQ(st1->OutRect(), TRect(10, 20, 7, 9));
    EXPECT_EQ(st1->MinSize(), TSize(5, 8));
    EXPECT_EQ(st1->MaxSize(), TSize(7, 9));

    grid->SetIsAutoMinMax({true, true});
    grid->SetIsAutoSize(true);
    grid->Update();

    //размеры обновились и мин макс и сами размеры grid
    EXPECT_EQ(grid->OutRect(), TRect(10, 20, 7, 9));
    EXPECT_EQ(grid->MinSize(), TSize(5, 8));
    EXPECT_EQ(grid->MaxSize(), TSize(7, 9));

    //добавляем еще один элемент в стек без ограничений
    auto el2 = st1->AddElement(plot.CreateLayout());
    grid->Update();

    //обновился максимум изза нового элемента
    EXPECT_EQ(grid->OutRect(), TRect(10, 20, 7, 9));
    EXPECT_EQ(grid->MinSize(), TSize(5, 8));
    EXPECT_EQ(grid->MaxSize(), TSize(7, INT_MAX));

    EXPECT_EQ(st1->OutRect(), TRect(10, 20, 7, 9));
    EXPECT_EQ(st1->MinSize(), TSize(5, 8));
    EXPECT_EQ(st1->MaxSize(), TSize(7, INT_MAX));

}

TEST(LayoutGrid, AddElement)
{
    TPlotMock plot;
    auto grid = plot.CreateLayout<TLayoutGrid>();
    grid->SetOutRect(TRect(10, 20, 30, 40));
    auto elem1 = plot.CreateLayout<TLayoutElement>();
    grid->AddElement(0, 0, elem1);
    grid->Update();

    EXPECT_EQ(elem1->OutRect(), TRect(10, 20, 30, 40));

    auto elem2 = plot.CreateLayout<TLayoutElement>();
    grid->AddElement(0, 1, elem2);
    grid->Update();

    EXPECT_EQ(elem1->OutRect(), TRect(10, 20, 15, 40));
    EXPECT_EQ(elem2->OutRect(), TRect(25, 20, 15, 40));

    grid->AddElement(1, 0, plot.CreateLayout<TLayoutElement>());
    grid->AddElement(1, 1, plot.CreateLayout<TLayoutElement>());
    grid->Update();

    EXPECT_EQ(elem1->OutRect(), TRect(10, 20, 15, 20));
    EXPECT_EQ(elem2->OutRect(), TRect(25, 20, 15, 20));
    EXPECT_EQ(grid->Element(1, 0)->OutRect(), TRect(10, 40, 15, 20));
    EXPECT_EQ(grid->Element(1, 1)->OutRect(), TRect(25, 40, 15, 20));

}

TEST(LayoutGrid, AxisRectLayout)
{
    TPlotMock plot;
    plot.SetViewport(TRect(0, 0, 300, 300));

    auto grid = plot.MainLayout();
    grid->SetIsAutoSize(true);

    auto top01 = plot.CreateLayout<TLayoutStack>(orVert);
    auto left10 = plot.CreateLayout<TLayoutStack>(orHorz);
    auto right12 = plot.CreateLayout<TLayoutStack>(orHorz);
    auto bottom21 = plot.CreateLayout<TLayoutStack>(orVert);

    grid->AddElement(0, 1, top01);
    grid->AddElement(1, 0, left10);
    grid->AddElement(1, 2, right12);
    grid->AddElement(2, 1, bottom21);

    EXPECT_EQ(top01->Layout(), grid);
    EXPECT_EQ(left10->Layout(), grid);
    EXPECT_EQ(right12->Layout(), grid);
    EXPECT_EQ(bottom21->Layout(), grid);

    grid->Update();

    EXPECT_EQ(grid->CountElements(), 9);//пустые элементы тоже считаются
    EXPECT_EQ(grid->HasElement(0, 0), false);
    EXPECT_EQ(grid->HasElement(0, 1), true);
    EXPECT_EQ(grid->HasElement(0, 2), false);

    EXPECT_EQ(grid->HasElement(1, 0), true);
    EXPECT_EQ(grid->HasElement(1, 1), false);
    EXPECT_EQ(grid->HasElement(1, 2), true);

    EXPECT_EQ(grid->HasElement(2, 0), false);
    EXPECT_EQ(grid->HasElement(2, 1), true);
    EXPECT_EQ(grid->HasElement(2, 2), false);

    //проверяем размеры по умолчанию без ограничений
    EXPECT_EQ(grid->OutRect(), TRect(0, 0, 300, 300));
    EXPECT_EQ(top01->OutRect(), TRect(100, 0, 100, 100));
    EXPECT_EQ(left10->OutRect(), TRect(0, 100, 100, 100));
    EXPECT_EQ(right12->OutRect(), TRect(200, 100, 100, 100));
    EXPECT_EQ(bottom21->OutRect(), TRect(100, 200, 100, 100));

    grid->Update();

    EXPECT_EQ(grid->OutRect(), TRect(0, 0, 300, 300));
    EXPECT_EQ(top01->OutRect(), TRect(0, 0, 300, 0));
    EXPECT_EQ(left10->OutRect(), TRect(0, 0, 0, 300));
    EXPECT_EQ(right12->OutRect(), TRect(300, 0, 0, 300));
    EXPECT_EQ(bottom21->OutRect(), TRect(0, 300, 300, 0));

    //добавляем элемент в стек "слева"
    left10->AddElement(plot.CreateLayout());
    left10->Element(0)->SetFixedWidth(40);
    grid->Update();

    EXPECT_EQ(grid->OutRect(), TRect(0, 0, 300, 300));
    EXPECT_EQ(top01->OutRect(), TRect(40, 0, 260, 0));
    EXPECT_EQ(left10->OutRect(), TRect(0, 0, 40, 300));
    EXPECT_EQ(right12->OutRect(), TRect(300, 0, 0, 300));
    EXPECT_EQ(bottom21->OutRect(), TRect(40, 300, 260, 0));

    //добавляем элемент в стек "сверху"
    top01->AddElement(plot.CreateLayout<TLayoutElement>());
    top01->Element(0)->SetFixedHeight(40);
    grid->Update();

    EXPECT_EQ(grid->OutRect(), TRect(0, 0, 300, 300));
    EXPECT_EQ(top01->OutRect(), TRect(40, 0, 260, 40));
    EXPECT_EQ(left10->OutRect(), TRect(0, 40, 40, 260));
    EXPECT_EQ(right12->OutRect(), TRect(300, 40, 0, 260));
    EXPECT_EQ(bottom21->OutRect(), TRect(40, 300, 260, 0));

    //имитируем превышение размеров родителя
    top01->Element(0)->SetFixedHeight(400);
    grid->Update();
    //общие размеры layout изменились чтобы вместить минимальные размеры вложенных элементов
    EXPECT_EQ(grid->OutRect(), TRect(0, 0, 300, 400));
    EXPECT_EQ(top01->OutRect(), TRect(40, 0, 260, 400));
    EXPECT_EQ(left10->OutRect(), TRect(0, 400, 40, 0));
    EXPECT_EQ(right12->OutRect(), TRect(300, 400, 0, 0));
    EXPECT_EQ(bottom21->OutRect(), TRect(40, 400, 260, 0));

    auto group = std::make_shared<TMarginGroup>(orVert);

    //вернем обратно минразмер
    top01->Element(0)->SetFixedHeight(40);
    grid->SetMarginGroup(atTop, group, top01);
    auto bigSize = plot.CreateLayout<TLayoutElement>();
    bigSize->SetFixedHeight(150);
    bigSize->SetMarginGroup(atTop, group, bigSize);
    grid->Update();

    //проверяем что изменился InnerRect изза группы
    EXPECT_EQ(grid->OutRect(), TRect(0, 0, 300, 400));
    EXPECT_EQ(grid->InnerRect(), TRect(0, 110, 300, 290));

    EXPECT_EQ(top01->OutRect(), TRect(40, 110, 260, 40));
    EXPECT_EQ(left10->OutRect(), TRect(0, 150, 40, 250));
    EXPECT_EQ(right12->OutRect(), TRect(300, 150, 0, 250));
    EXPECT_EQ(bottom21->OutRect(), TRect(40, 400, 260, 0));
}

TEST(LayoutGrid, VisibleItem)
{
    TPlotMock plot(false);//создаем пустой plot
    plot.SetViewport(TRect(0, 0, 300, 300));

    auto grid = plot.MainLayout();
    auto el1 = plot.CreateLayout();
    auto el2 = plot.CreateLayout();

    grid->AddElement(0, 0, el1);
    grid->AddElement(0, 1, el2);
    grid->Update();

    EXPECT_EQ(el1->OutRect(), TRect(0, 0, 150, 300));
    EXPECT_EQ(el2->OutRect(), TRect(150, 0, 150, 300));

    el2->SetIsVisible(false);
    grid->Update();

    EXPECT_EQ(el1->OutRect(), TRect(0, 0, 300, 300));
    EXPECT_EQ(el2->OutRect(), TRect(150, 0, 150, 300));

    auto st1 = plot.CreateLayout<TLayoutStack>(orVert);
    grid->AddElement(0, 2, st1);
    el2->SetIsVisible(true);
    grid->Update();

    EXPECT_EQ(el1->OutRect(), TRect(0, 0, 100, 300));
    EXPECT_EQ(el2->OutRect(), TRect(100, 0, 100, 300));
    EXPECT_EQ(st1->OutRect(), TRect(200, 0, 100, 300));

    st1->SetIsVisible(false);
    grid->Update();

    EXPECT_EQ(el1->OutRect(), TRect(0, 0, 150, 300));
    EXPECT_EQ(el2->OutRect(), TRect(150, 0, 150, 300));
    EXPECT_EQ(st1->OutRect(), TRect(200, 0, 100, 300));
}
*/
TEST(AxisRect, Init)
{
    TPlotMock plot(false);

    auto rect = plot.AddAxisRect(false);//создаем пустой axisRects
    EXPECT_EQ(rect != nullptr, true);
    EXPECT_EQ(rect->Layer() != nullptr, true);//чтобы можно было выбирать пользователем
    EXPECT_EQ(rect->Grid() == nullptr, true);

    //проверяем что layout для axis созданы
    for(size_t i = atLeft; i < atCount; i++)
    {
        ASSERT_EQ(rect->AxisLay(TAxisType(i)) != nullptr, true);
        EXPECT_EQ(rect->AxisLay(TAxisType(i))->CountElements(), 0);
    }
    //проверяем что шкал нет
    for(size_t i = atLeft; i < atCount; i++)
        EXPECT_EQ(rect->CountAxis(TAxisType(i)), 0);
}

TEST(AxisRect, AddAxis)
{
    TPlotMock plot(false);
    plot.SetViewport(TRect(0, 0, 300, 300));

    EXPECT_EQ(plot.Viewport().left(), 0);
    EXPECT_EQ(plot.Viewport().top(), 0);
    EXPECT_EQ(plot.Viewport().width(), 300);
    EXPECT_EQ(plot.Viewport().height(), 300);

    auto rect = plot.AddAxisRect(false);//создаем пустой axisRects
    plot.Replot();

    EXPECT_EQ(rect->OutRect().left(), 0);
    EXPECT_EQ(rect->OutRect().top(), 0);
    EXPECT_EQ(rect->OutRect().width(), 0);
    EXPECT_EQ(rect->OutRect().height(), 0);

    //добавляем шкалу слева
    auto axisLeft = rect->AddAxis(atLeft);
    plot.Replot();

    ASSERT_EQ(axisLeft != nullptr, true);
    EXPECT_EQ(axisLeft->TypeAxis(), atLeft);
    EXPECT_EQ(axisLeft->ScaleType(), stLinear);
    EXPECT_EQ(axisLeft->Label(), TString());
    EXPECT_EQ(axisLeft->Lower(), 0);
    EXPECT_EQ(axisLeft->Upper(), 5);
    EXPECT_EQ(axisLeft->Step(), 1);
    EXPECT_EQ(axisLeft->IsRangeReversed(), false);
    EXPECT_EQ(axisLeft->IsTicks(), true);
    EXPECT_EQ(axisLeft->IsSubTicks(), true);
    EXPECT_EQ(axisLeft->IsTickLabels(), true);
    EXPECT_EQ(axisLeft->ColorAxis(), TConsts::BlackColor());
    EXPECT_EQ(axisLeft->Orientation(), orVert);
    EXPECT_EQ(axisLeft->PixelOrientation(), -1);
    EXPECT_EQ(axisLeft->IsFixedPix(), false);
    EXPECT_EQ(axisLeft->IsUseBuffer(), true);
    EXPECT_EQ(axisLeft->IsSelected(), false);
    EXPECT_EQ(axisLeft->IsSelectable(), true);
    EXPECT_EQ(axisLeft->IsDraggable(), true);
    EXPECT_EQ(axisLeft->IsScalable(), true);
    EXPECT_EQ(axisLeft->AxisRect() != nullptr, true);

    EXPECT_EQ(axisLeft->OutRect().left(), 0);
    EXPECT_EQ(axisLeft->OutRect().top(), 0);
    EXPECT_EQ(axisLeft->OutRect().width(), 40);
    EXPECT_EQ(axisLeft->OutRect().height(), 300);

    EXPECT_EQ(rect->AxisLay(atLeft)->CountElements(), 1);
    EXPECT_EQ(rect->CountAxis(atLeft), 1);

    //добавляем шкалу снизу
    auto axisBottom = rect->AddAxis(atBottom);
    plot.Replot();

    //проверяем что размеры сместились для левой шкалы
    EXPECT_EQ(axisLeft->OutRect().left(), 0);
    EXPECT_EQ(axisLeft->OutRect().top(), 0);
    EXPECT_EQ(axisLeft->OutRect().width(), 40);
    EXPECT_EQ(axisLeft->OutRect().height(), 260);

    //проверяем размеры верхней шкалы
    EXPECT_EQ(axisBottom->OutRect().left(), 40);
    EXPECT_EQ(axisBottom->OutRect().top(), 260);
    EXPECT_EQ(axisBottom->OutRect().width(), 260);
    EXPECT_EQ(axisBottom->OutRect().height(), 40);
}
/*
TEST(DepthAxisRect, Init)
{
    TDepthPlotMock plot;
    plot.SetParentSize(TSize(300, 500));

    auto rect = plot.AddDepthAxisRect();
    plot.Replot();

    ASSERT_EQ(rect != nullptr, true);
    EXPECT_EQ(rect->MarginGroup(atTop) != nullptr, true);
    EXPECT_EQ(rect->IsHorzResizable(), true);               //можно пользователю менять ширину
    EXPECT_EQ(rect->IsVertResizable(), false);              //менять пользователю высоту нельзя
    EXPECT_EQ(rect->DepthAxis() != nullptr, true);          //шкала глубины одна общая на всех
    EXPECT_EQ(rect->CountAxis(atLeft), 0);                  //левой шкалы нет
    EXPECT_EQ(rect->CountAxis(atTop), 1);                   //есть верхняя для отрисовки сетки
    EXPECT_EQ(rect->Axis(atTop)->IsVisible(), true);       //верхняя шкала не видима
    EXPECT_EQ(rect->AxisLay(atLeft)->CountElements(), 0);   //в левом стеке layout'ов нет элементов
    EXPECT_EQ(rect->AxisLay(atTop)->CountElements(), 1);    //в верхнем есть
    EXPECT_EQ(rect->AxisLay(atTop)->Element(0)->OutRect().width(), 240);
    EXPECT_EQ(rect->AxisLay(atTop)->Element(0)->OutRect().height(), 0);

    auto first = rect->AddAxis(atTop);
    plot.Replot();
    EXPECT_EQ(first->OutRect().top(), 0);
    EXPECT_EQ(first->OutRect().left(), 60);
    EXPECT_EQ(first->OutRect().width(), 240);
    EXPECT_EQ(first->OutRect().height(), 40);

    auto second = rect->AddAxis(atTop);
    plot.Replot();
    EXPECT_EQ(second->OutRect().top(), 40);
    EXPECT_EQ(second->OutRect().left(), 60);
    EXPECT_EQ(second->OutRect().width(), 240);
    EXPECT_EQ(second->OutRect().height(), 40);
    auto last = 200;
    for(auto i = 2; i < last; i++)
        rect->AddAxis(atTop);

    const auto &ax = rect->AddAxis(atTop);
    plot.Replot();
    ASSERT_EQ(ax != nullptr, true);
    EXPECT_EQ(ax->OutRect().top(), last * 40) << "Error index: " << last;
    EXPECT_EQ(ax->OutRect().left(), 60);
    EXPECT_EQ(ax->OutRect().width(), 240);
    EXPECT_EQ(ax->OutRect().height(), 40);

}
*/
TEST(Plot, AddLegend)
{
    TPlotMock plot(false);
    plot.NativeResize(TSize(300, 500));
    auto rect = plot.AddAxisRect();
    auto legend = plot.AddLegend();
    plot.Replot();
    EXPECT_EQ(legend != nullptr, true);

    EXPECT_EQ(rect->OutRect(), TRect(0, 0, 300, 500));
    EXPECT_EQ(legend->OutRect(), TRect(0, 500, 300, 0));

    legend->AddItem<TLegendItemPlottable>(plot.AddPlottable<TGraph>(plot.BottomAxis(), plot.LeftAxis()));
    plot.Replot();
    EXPECT_EQ(rect->OutRect(), TRect(0, 0, 300, 480));
    EXPECT_EQ(legend->OutRect(), TRect(0, 480, 300, 20));

    legend->AddItem<TLegendItemPlottable>(plot.AddPlottable<TGraph>(plot.BottomAxis(), plot.LeftAxis()));
    plot.Replot();
    EXPECT_EQ(rect->OutRect(), TRect(0, 0, 300, 480));
    EXPECT_EQ(legend->OutRect(), TRect(0, 480, 300, 20));
}