set(SdlTemplate_SOURCES
    Main.cpp
)

add_executable(SdlTemplate ${SdlTemplate_SOURCES})
target_link_libraries(SdlTemplate ${SdlTemplate_DEP_LIBS})

