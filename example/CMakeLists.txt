

set( tado_example_SRCS
	main.cpp
	)

add_executable( tado_example ${tado_example_SRCS} )
target_link_libraries(tado_example qtado Qt5::Gui Qt5::Core Qt5::Network)

