cmake_minimum_required(VERSION 3.16)
project(InvoiceExtractor LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Network Sql)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Network Sql)

set(SOURCES
    src/main.cpp
    src/core/TextNormalizer.cpp
    src/core/FieldExtractor.cpp
    src/core/TableExtractor.cpp
    src/core/InvoiceCalculator.cpp
    src/core/OcrClient.cpp
    src/core/Database.cpp
    src/core/HttpServer.cpp
    src/ui/MainWindow.cpp
    src/ui/Sidebar.cpp
    src/ui/StatCard.cpp
    src/ui/DashboardPage.cpp
    src/ui/NewInvoicePage.cpp
    src/ui/DropArea.cpp
    src/ui/QrCodeWidget.cpp
)

set(HEADERS
    src/core/InvoiceModel.h
    src/core/TextNormalizer.h
    src/core/FieldExtractor.h
    src/core/TableExtractor.h
    src/core/InvoiceCalculator.h
    src/core/OcrClient.h
    src/core/Database.h
    src/core/HttpServer.h
    src/ui/MainWindow.h
    src/ui/Sidebar.h
    src/ui/StatCard.h
    src/ui/DashboardPage.h
    src/ui/NewInvoicePage.h
    src/ui/DropArea.h
    src/ui/QrCodeWidget.h
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(InvoiceExtractor ${SOURCES} ${HEADERS} resources.qrc)
else()
    add_executable(InvoiceExtractor ${SOURCES} ${HEADERS} resources.qrc)
endif()

target_link_libraries(InvoiceExtractor PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::Sql
)