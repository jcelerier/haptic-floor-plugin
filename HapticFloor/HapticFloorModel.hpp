#pragma once

#include <halp/audio.hpp>
#include <halp/controls.hpp>
#include <halp/meta.hpp>
#include <rapidjson/document.h>
#include <rapidjson/filereadstream.h>
#include <halp/dynamic_port.hpp>
#include <halp/layout.hpp>
#include <avnd/concepts/painter.hpp>
#include <vector>
#include <cmath>

namespace Example
{

class HapticFloor
{
public:
  halp_meta(name, "Haptic floor")
  halp_meta(category, "Plugins")
  halp_meta(c_name, "hapticfloor")
  halp_meta(author, "Jean-Michaël Celerier, Hugo Genillier")
  halp_meta(uuid, "2fcacb29-7309-478a-9400-380ded08a64f")

  struct ins
  {
    //the first input is a json layout file depicting the haptic floor (same layout filed used for mmesh)
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
    //the actuators input are sliders [-1,1]
    halp::dynamic_port<halp::hslider_f32<"Node {}", halp::range{-1, 1, 0}>> in_i;
  } inputs;

  //the output is a list of float [-1,1]
  struct outs
  {
    halp::val_port<"Output", std::vector<float>> out;
  } outputs;

  struct node {
    halp_flag(relocatable);
    int x = 0;
    int y = 0;
    int channel = 0;
    bool isActive = false;
  };

  std::vector<node> m_activenodes;
  std::vector<node> m_passivenodes;

  struct processor_to_ui
  {
    halp_flag(relocatable);
    std::vector<node> send_activenodes;
    std::vector<node> send_passivenodes;
  };

  std::function<void(processor_to_ui)> send_message;

  struct custom_anim
  {
    using item_type = custom_anim;
    static constexpr float width() { return 200.; }
    static constexpr float height() { return 200.; }

    std::vector<node> rect_activenodes;
    std::vector<node> rect_passivenodes;

    void paint(avnd::painter auto ctx)
    {
      ctx.set_stroke_width(2.0);
      ctx.set_stroke_color({.r = 255, .g = 255, .b = 255, .a = 255});
      ctx.begin_path();
      for (const auto& n : rect_activenodes)
      {
        // Find neighboring nodes
        for (const auto& neighbor : rect_activenodes)
        {
          if ((neighbor.x == n.x + 1 && neighbor.y == n.y) ||
             (neighbor.x == n.x && neighbor.y == n.y + 1))
          {
            ctx.move_to(n.x * 20, n.y * 20);
            ctx.line_to(neighbor.x * 20, neighbor.y * 20);
          }
        }
        for (const auto& neighbor : rect_passivenodes)
        {
          if ((neighbor.x == n.x + 1 && neighbor.y == n.y) ||
             (neighbor.x == n.x && neighbor.y == n.y + 1))
          {
            ctx.move_to(n.x * 20, n.y * 20);
            ctx.line_to(neighbor.x * 20, neighbor.y * 20);
          }
        }
      }
      for (const auto& n : rect_passivenodes)
      {
        // Find neighboring nodes
        for (const auto& neighbor : rect_activenodes)
        {
          if ((neighbor.x == n.x + 1 && neighbor.y == n.y) ||
             (neighbor.x == n.x && neighbor.y == n.y + 1))
          {
            ctx.move_to(n.x * 20, n.y * 20);
            ctx.line_to(neighbor.x * 20, neighbor.y * 20);
          }
        }
        for (const auto& neighbor : rect_passivenodes)
        {
          if ((neighbor.x == n.x + 1 && neighbor.y == n.y) ||
             (neighbor.x == n.x && neighbor.y == n.y + 1))
          {
            ctx.move_to(n.x * 20, n.y * 20);
            ctx.line_to(neighbor.x * 20, neighbor.y * 20);
          }
        }
      }
      ctx.stroke();
      ctx.close_path();

      // Draw passive nodes
      for (const auto& n : rect_passivenodes)
      {
        ctx.set_fill_color({0, 0, 255, 255}); // Blue fill
        ctx.set_stroke_color({0, 0, 255, 255}); // Blue stroke
        ctx.draw_circle(n.x * 20, n.y * 20, 5);
        ctx.fill();
        ctx.stroke();
      }
      // Draw active nodes
      for (const auto& n : rect_activenodes)
      {
        ctx.set_fill_color({0, 255, 0, 255}); // Green fill
        ctx.set_stroke_color({0, 255, 0, 255}); // Green stroke
        ctx.draw_circle(n.x * 20, n.y * 20, 5);
        ctx.fill();
        ctx.stroke();
      }
      ctx.update();

    }
  };

  struct ui
  {
    halp_meta(name, "Main")
    halp_meta(layout, halp::layouts::hbox)
    halp_meta(background, halp::colors::mid)

    struct bus
    {
      static void process_message(ui& self, processor_to_ui msg)
      {
        self.haptic_floor.anim.rect_activenodes = msg.send_activenodes;
        self.haptic_floor.anim.rect_passivenodes = msg.send_passivenodes;
      }
    };


    struct {
      halp_meta(layout, halp::layouts::hbox)
      struct
      {
        halp_meta(layout, halp::layouts::grid)
        halp_meta(columns, 1)
        halp_meta(name, "inputs")
        decltype(&ins::layout) int_widget = &ins::layout;
      } layoutInput;
      struct
      {
        halp_meta(layout, halp::layouts::grid)
        halp_meta(columns, 1)
        halp_meta(name, "inputs")
        decltype(&ins::in_i) int_widget = &ins::in_i;
      } nodesInput;
    } input;

    struct
    {
      halp_meta(layout, halp::layouts::hbox)
      halp_meta(name, "haptic_floor")
      halp_meta(width, 300)
      halp_meta(height, 200)
      halp::custom_actions_item<custom_anim> anim{};
    } haptic_floor;
  };

  void operator()()
  {
    int numberOfChannels = m_activenodes.size();
    std::vector<float> vec(numberOfChannels, 0.0f);
    outputs.out.value = vec;
    int index = 0;
    for (const auto& val : inputs.in_i.ports) {
      if (index < numberOfChannels) {
        outputs.out.value[index] = val;
        ++index;
      } else {
        index = 0;
      }
    }
    send_message(processor_to_ui{.send_activenodes=m_activenodes,.send_passivenodes=m_passivenodes});
  }
  void loadLayout();
};

}
