cmake_minimum_required(VERSION 3.16)
project(AiotClient LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC_MOC_EXECUTABLE /usr/lib/x86_64-linux-gnu/qt5/bin/moc)
set(CMAKE_AUTOUIC_UIC_EXECUTABLE /usr/lib/x86_64-linux-gnu/qt5/bin/uic)

find_package(Qt5 REQUIRED COMPONENTS
  Core Gui Widgets Network Multimedia MultimediaWidgets Sql Svg DataVisualization)

add_executable(AiotClient
  main.cpp
  mainwidget.cpp mainwidget.h mainwidget.ui
  sockclient.cpp  sockclient.h
  mjpegclient.cpp mjpegclient.h
  tab1data.cpp    tab1data.h    tab1data.ui
  tab3dgraph.cpp  tab3dgraph.h
  Images.qrc
  media.qrc
)

target_link_libraries(AiotClient PRIVATE
  Qt5::Core Qt5::Gui Qt5::Widgets Qt5::Network
  Qt5::Multimedia Qt5::MultimediaWidgets
  Qt5::Sql Qt5::Svg Qt5::DataVisualization
)
