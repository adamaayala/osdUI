add_executable(osdui WIN32)

target_sources(osdui PRIVATE
  main.cpp
  platform/ts_variables.cpp
  platform/wsh_script_host.cpp
)

target_link_libraries(osdui
  PRIVATE osdui-core
          Microsoft::WIL
          ole32.lib
          oleaut32.lib
)

# populated in later tasks
