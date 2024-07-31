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

      static std::function<void(HapticFloor&, std::string_view)>
      on_controller_interaction()
      {
        return [](HapticFloor& object, std::string_view value) {
          int count = object.m_activenodes.size();
          object.inputs.in_i.request_port_resize(count);
        };
      }

    } layout;
    halp::dynamic_port<halp::hslider_f32<"Node {}", halp::range{-1, 1, 0}>> in_i;
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

  void operator()()
  {
    int numberOfChannels=m_activenodes.size();
    std::vector<float> vec(numberOfChannels, 0.0f);
    outputs.out.value = vec;
    int index = 0;
    for (const auto& val : inputs.in_i.ports) {
      if (index < numberOfChannels) {
        outputs.out.value[index] = val;
        ++index;
      } else {
        index=0;
      }
    }
  }
  void loadLayout();
};
}

