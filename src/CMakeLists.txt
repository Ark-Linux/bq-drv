AUX_SOURCE_DIRECTORY(. src_files)

ADD_EXECUTABLE(bq25703a ${src_files})

find_package(Threads)

target_link_libraries(bq25703a ${CMAKE_THREAD_LIBS_INIT})

INSTALL(TARGETS bq25703a
	RUNTIME DESTINATION bin
)


