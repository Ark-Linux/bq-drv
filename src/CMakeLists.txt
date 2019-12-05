AUX_SOURCE_DIRECTORY(. src_files)

ADD_EXECUTABLE(bq25703-drv ${src_files})

find_package(Threads)

target_link_libraries(bq25703-drv ${CMAKE_THREAD_LIBS_INIT})

INSTALL(TARGETS bq25703-drv
	RUNTIME DESTINATION bin
)


