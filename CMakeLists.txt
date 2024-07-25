cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

if(NOT TARGET score_lib_base)
  include(ScoreExternalAddon)
endif()

if(NOT TARGET score_plugin_avnd)
  return()
endif()

project(score_addon_hapticfloor LANGUAGES CXX)

avnd_score_plugin_init(
  BASE_TARGET score_addon_hapticfloor
)

avnd_score_plugin_add(
  BASE_TARGET score_addon_hapticfloor
  SOURCES
    HapticFloor/HapticFloor.hpp
    HapticFloor/HapticFloorModel.hpp
    HapticFloor/HapticFloorModel.cpp
    HapticFloor/HapticFloorUi.hpp
  TARGET hapticfloor
  MAIN_CLASS HapticFloor
  NAMESPACE Example
)

avnd_score_plugin_finalize(
  BASE_TARGET score_addon_hapticfloor
  PLUGIN_VERSION 1
  PLUGIN_UUID "22186a4e-9547-4365-bb73-8f1e270d73df"
)
