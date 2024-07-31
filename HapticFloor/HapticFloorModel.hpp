#pragma once

#include <halp/audio.hpp>
#include <halp/controls.hpp>
#include <halp/meta.hpp>
#include <rapidjson/document.h>
#include <rapidjson/filereadstream.h>
#include <halp/dynamic_port.hpp>

namespace Example
{

class HapticFloor
{
public:
  halp_meta(name, "Haptic floor")
  halp_meta(category, "Plugins")
  halp_meta(c_name, "hapticfloor")
  halp_meta(author, "Hugo Genillier")
  halp_meta(uuid, "2fcacb29-7309-478a-9400-380ded08a64f")

  struct ins
  {
    struct : halp::lineedit<"Layout file", "">
    {
      halp_meta(language, "json")
      void update(HapticFloor& self) { self.loadLayout(); }
    } layout;
    halp::hslider_f32<"Height of node {}", halp::range{-1, 1, 0}> height;

  } inputs;

  struct outs
  {
    halp::val_port<"Output", std::vector<float>> out;
  } outputs;

  //haptic floor's nodes coordinate system
  struct node {
    int x;
    int y;
    int channel;
    bool isActive;

    node(int x, int y, int channel = 0, bool isActive = false)
        : x(x), y(y), channel(channel), isActive(isActive) {}
  };

  std::vector<node> m_activenodes;
  std::vector<node> m_passivenodes;

  // Defined in the .cpp
  void operator()();
  void loadLayout();
};
}
