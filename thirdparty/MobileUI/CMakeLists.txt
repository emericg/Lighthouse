project(MobileUI LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)

################################################################################

# Generic dependencies
set(CORE_COMPONENTS Core Gui Qml)
find_package(Qt6 6.7 REQUIRED COMPONENTS ${CORE_COMPONENTS})

set(CORE_LIBRARIES Qt::Core Qt::Gui Qt::Qml)

# Generic sources
set(CORE_SOURCES
    MobileUI.h
    MobileUI.cpp)

# OS specific sources & dependencies
if(ANDROID)
    set(PLATFORM_SOURCES MobileUI_android.cpp)
elseif(IOS)
    set(PLATFORM_SOURCES MobileUI_ios.mm)
    set(PLATFORM_LIBRARIES "-framework UIKit")
else()
    set(PLATFORM_SOURCES MobileUI_dummy.cpp)
endif()

################################################################################

add_library(MobileUI OBJECT ${CORE_SOURCES} ${PLATFORM_SOURCES})
add_library(MobileUI::MobileUI ALIAS MobileUI)

target_link_libraries(MobileUI PRIVATE ${CORE_LIBRARIES} ${PLATFORM_LIBRARIES})
target_include_directories(MobileUI PUBLIC ${CMAKE_CURRENT_LIST_DIR})
