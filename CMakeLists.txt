cmake_minimum_required(VERSION 3.20)
project(Plot)

set(CMAKE_CXX_STANDARD 17)

include(C:/CMakeProjects/Fetch.cmake)

#Определяем зависимости проекта

#Загрузка зависимостей без add_subdirectory
#Набор каталогов для указания target_include..
Populate(Base github.com/nilrum/base .)
Populate(sigslot github.com/palacaze/sigslot include)

set(Plot_SRC
        Primitives.cpp
        BaseClasses.cpp
        Plot.cpp
        Plottables.cpp
        PlotItems.cpp
        )

if(${QT_GUI})
    set(Plot_SRC ${Plot_SRC} Qt/PlotQt.cpp)
    set(Plot_DEPS_INCL ${Plot_DEPS_INCL} ${Qt5Widgets_INCLUDE_DIRS})
endif()

#Объявление библиотеки
if(NOT TARGET Plot)
    add_library(Plot STATIC ${Plot_SRC})
    target_include_directories(Plot PUBLIC ${FETCH_DIR} ${Plot_DEPS_INCL} )
endif()

if(${BUILD_TEST})
    message("Build plot test enabled")
    #Загрузка googletest и вызов add_subdirectory
    if(MSVC)
        set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    endif()
    if(NOT TARGET gtest)
        MakeAvailable(googletest github.com/google/googletest googletest/include)
    endif()

    #Объявим цель тестирования
    add_executable(PlotTest
            tests/PlotTest.cpp
            )

    target_include_directories(PlotTest
            PUBLIC ${FETCH_DIR}
            PUBLIC ${Plot_DEPS_INCL}
            )
    target_link_libraries(PlotTest PUBLIC Plot gtest gtest_main gmock)
    enable_testing()
    add_test(NAME PlotTest COMMAND PlotTest)
endif()

#Включаем зависимости для корневого проекта
DepsAvailable()