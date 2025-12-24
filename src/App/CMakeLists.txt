set(SRCS
    main.cpp
    Window.cpp
    Window.h

    Shaders/julia.fs
    Shaders/julia.vs

    resources.qrc
)

find_package(Qt5 COMPONENTS Widgets REQUIRED)

add_executable(julia_fractal ${SRCS})

target_link_libraries(julia_fractal
    PRIVATE
        Qt5::Widgets
        FGL::Base
)

add_custom_target(run
    COMMAND julia_fractal
)