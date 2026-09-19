if(NOT DEFINED RO_CONTROL_INSTALL_ROOT)
    message(FATAL_ERROR "RO_CONTROL_INSTALL_ROOT must point at a staged install")
endif()

set(_required_files
    "bin/ro-control"
    "libexec/ro-control-helper"
    "share/applications/io.github.projectroasd.rocontrol.desktop"
    "share/metainfo/io.github.projectroasd.rocontrol.metainfo.xml"
    "share/polkit-1/actions/io.github.ProjectRoASD.rocontrol.policy"
    "share/man/man1/ro-control.1"
    "share/bash-completion/completions/ro-control"
    "share/zsh/site-functions/_ro-control"
    "share/fish/vendor_completions.d/ro-control.fish"
    "share/icons/hicolor/scalable/apps/ro-control.svg"
    "share/icons/hicolor/scalable/apps/io.github.projectroasd.rocontrol.svg"
    "share/icons/hicolor/256x256/apps/ro-control.png"
    "share/icons/hicolor/256x256/apps/io.github.projectroasd.rocontrol.png"
    "lib/systemd/system/ro-control.service"
    "lib/systemd/user/ro-control.service")

foreach(_file IN LISTS _required_files)
    if(NOT EXISTS "${RO_CONTROL_INSTALL_ROOT}/${_file}")
        message(FATAL_ERROR "Install tree is missing ${_file}")
    endif()
endforeach()

if(NOT IS_EXECUTABLE "${RO_CONTROL_INSTALL_ROOT}/bin/ro-control")
    message(FATAL_ERROR "Installed ro-control binary is not executable")
endif()
if(NOT IS_EXECUTABLE "${RO_CONTROL_INSTALL_ROOT}/libexec/ro-control-helper")
    message(FATAL_ERROR "Installed privileged helper is not executable")
endif()

find_program(_desktop_file_validate desktop-file-validate)
if(_desktop_file_validate)
    execute_process(
        COMMAND "${_desktop_file_validate}"
                "${RO_CONTROL_INSTALL_ROOT}/share/applications/io.github.projectroasd.rocontrol.desktop"
        RESULT_VARIABLE _desktop_result
        OUTPUT_VARIABLE _desktop_output
        ERROR_VARIABLE _desktop_error)
    if(NOT _desktop_result EQUAL 0)
        message(FATAL_ERROR "desktop-file validation failed:\n${_desktop_output}${_desktop_error}")
    endif()
endif()

find_program(_appstreamcli appstreamcli)
if(_appstreamcli)
    execute_process(
        COMMAND "${_appstreamcli}" validate --no-net
                "${RO_CONTROL_INSTALL_ROOT}/share/metainfo/io.github.projectroasd.rocontrol.metainfo.xml"
        RESULT_VARIABLE _appstream_result
        OUTPUT_VARIABLE _appstream_output
        ERROR_VARIABLE _appstream_error)
    if(NOT _appstream_result EQUAL 0)
        message(FATAL_ERROR "AppStream validation failed:\n${_appstream_output}${_appstream_error}")
    endif()
endif()

message(STATUS "Install tree verified: ${RO_CONTROL_INSTALL_ROOT}")
