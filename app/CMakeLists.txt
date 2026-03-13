add_executable(osdui WIN32)

target_sources(osdui PRIVATE
  main.cpp
  resources/osdui.rc
  platform/ts_variables.cpp
  platform/wsh_script_host.cpp
  dialogs/dialog_presenter.cpp
  dialogs/dialog_user_input.cpp
  dialogs/dialog_user_info.cpp
  dialogs/dialog_preflight.cpp
  dialogs/dialog_app_tree.cpp
  dialogs/dialog_ts_var.cpp
  dialogs/dialog_vars.cpp
  dialogs/dialog_error_info.cpp
  dialogs/dialog_save_items.cpp
)

target_include_directories(osdui PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/resources
)

target_link_libraries(osdui PRIVATE
  osdui-core
  WIL::WIL
  ole32.lib
  oleaut32.lib
  wbemuuid.lib
  comctl32.lib
)
