add_executable(osdui WIN32)

target_sources(osdui PRIVATE
  main.cpp
)

target_link_libraries(osdui
  PRIVATE osdui-core
          Microsoft::WIL
)

# populated in later tasks
