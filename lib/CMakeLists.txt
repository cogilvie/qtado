set( qtado_HDRS
    include/qtado/tado.h
	)

set( qtado_SRCS
	tado.cpp
	)

add_library( qtado STATIC ${qtado_HDRS} ${qtado_SRCS} )
target_link_libraries(qtado Qt5::Core Qt5::Network)
target_compile_definitions(qtado PRIVATE BUILDING_TADO)

target_include_directories(qtado
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include/qtado
)
