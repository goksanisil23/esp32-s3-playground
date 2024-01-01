# NOTE: Ensure the esp environment is sourced, e.g.
#   source $HOME/esp/esp-idf/export.sh
cmake_minimum_required(VERSION 3.5)

set(ESP_WHO_DIR ../../esp-who/)

set(EXTRA_COMPONENT_DIRS 
    ${ESP_WHO_DIR}/components
    $ENV{IDF_PATH}/examples/common_components/protocol_examples_common
)
add_compile_options(-fdiagnostics-color=always)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(camera_imu)
