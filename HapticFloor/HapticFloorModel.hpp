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
  //We define metadata
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
      //The json file is parsed with the loadLayout method
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
    //The actuators input are sliders [-1,1]
    halp::dynamic_port<halp::hslider_f32<"Actuator {}", halp::range{-1, 1, 0}>> in_i;
  } inputs;

  //The output is a list of float [-1,1]
  struct outs
  {
    halp::val_port<"Output", std::vector<float>> out;
  } outputs;

  //This structure defines our nodes
  struct node {
    halp_flag(relocatable);
    int x = 0;
    int y = 0;
    int channel = 0;
    bool isActive = false;

    //Since the coordinates given by the json file are not cartesian,
    //we can modify them to draw an actual mesh of triangle
    void coordinateToMesh()
    {
      if (y%2==0){
        x=1+2*x;
        y=2*y;
      }
      else{
        x=2*x;
        y=2*y;
      }
    }
  };

  std::vector<node> m_activenodes;
  std::vector<node> m_passivenodes;

  //The message containing the actives and passives nodes is send to the UI
  struct processor_to_ui
  {
    halp_flag(relocatable);
    std::vector<node> send_activenodes;
    std::vector<node> send_passivenodes;
  };

  std::function<void(processor_to_ui)> send_message;

  //This custom_anim defines the haptic floor's drawing
  struct custom_anim
  {
    using item_type = custom_anim;
    static constexpr float width() { return 300.; }
    static constexpr float height() { return 300.; }

    //These are from the message received
    std::vector<node> rect_activenodes;
    std::vector<node> rect_passivenodes;

    //This is the drawing process
    void paint(avnd::painter auto ctx)
    {
      if (rect_activenodes.empty() && rect_passivenodes.empty())
        return;

      //The are steps to center our drawing in the box
      float min_x = std::numeric_limits<float>::max();
      float max_x = std::numeric_limits<float>::min();
      float min_y = std::numeric_limits<float>::max();
      float max_y = std::numeric_limits<float>::min();

      auto update_bounds = [&](const node& n) {
        if (n.x < min_x) min_x = n.x;
        if (n.x > max_x) max_x = n.x;
        if (n.y < min_y) min_y = n.y;
        if (n.y > max_y) max_y = n.y;
      };

      for (const auto& n : rect_activenodes) update_bounds(n);
      for (const auto& n : rect_passivenodes) update_bounds(n);

      // Calculate center and scale factor
      float center_x = (min_x + max_x) / 2;
      float center_y = (min_y + max_y) / 2;

      float scale_x = width() / (max_x - min_x + sqrt(5));
      float scale_y = height() / (max_y - min_y + sqrt(5));
      float scale = std::min(scale_x, scale_y);

      //Once the coordinates centered, we can draw
      //Draw lines between neighboring nodes
      ctx.set_stroke_width(1.0);
      ctx.set_stroke_color({255, 255, 255, 255});
      ctx.begin_path();

      auto draw_lines = [&](const std::vector<node>& nodes1, const std::vector<node>& nodes2) {
        for (const auto& n : nodes1)
        {
          for (const auto& neighbor : nodes2)
          {
            //Neighbors are detected if both x and y distance is less than or equal to sqrt(5)
            //which is the maximum distance for nodes to be neighbors with our coordinate system
            if ((abs(neighbor.x - n.x) <= sqrt(5)) && (abs(neighbor.y - n.y) <= sqrt(5)))
            {
              ctx.move_to((n.x - center_x) * scale + width() / 2, (n.y - center_y) * scale + height() / 2);
              ctx.line_to((neighbor.x - center_x) * scale + width() / 2, (neighbor.y - center_y) * scale + height() / 2);
            }
          }
        }
      };

      //We make sure to draw every possible connections
      draw_lines(rect_activenodes, rect_activenodes);
      draw_lines(rect_activenodes, rect_passivenodes);
      draw_lines(rect_passivenodes, rect_passivenodes);

      ctx.stroke();
      ctx.close_path();

      //Draw passive nodes
      ctx.set_font("Comic Sans");
      ctx.set_font_size(8.0);
      ctx.begin_path();
      for (const auto& n : rect_passivenodes)
      {
        ctx.set_fill_color({255, 255, 255, 255});
        ctx.set_stroke_color({255, 255, 255, 255});
        ctx.draw_circle((n.x - center_x) * scale + width() / 2, (n.y - center_y) * scale + height() / 2, 10);
        ctx.fill();
        ctx.stroke();
      }
      ctx.close_path();

      //Draw active nodes
      ctx.begin_path();
      for (const auto& n : rect_activenodes)
      {
        ctx.set_fill_color({84, 105, 28, 255});
        ctx.set_stroke_color({255, 255, 255, 255});
        ctx.draw_circle((n.x - center_x) * scale + width() / 2, (n.y - center_y) * scale + height() / 2, 10);
        ctx.fill();
        ctx.stroke();
      }
      ctx.close_path();

      //Draw the channels on active nodes
      ctx.begin_path();
      for (size_t i = 0; i < rect_activenodes.size(); ++i)
      {
        const auto& n = rect_activenodes[i];
        ctx.set_fill_color({255, 255, 255, 255});
        ctx.set_stroke_color({255, 255, 255, 255});

        //Convert the channel to a string
        std::string channel_text = std::to_string(n.channel);

        //Adjustments to center the text in node's circles
        float text_offset_x = 3.0f;
        float text_offset_y = 3.0f;

        ctx.draw_text((n.x - center_x) * scale + width() / 2 - text_offset_x,
                      (n.y - center_y) * scale + height() / 2 + text_offset_y,
                      channel_text.c_str());
        ctx.fill();
        ctx.stroke();
      }
      ctx.close_path();

      ctx.update();
    }
  };

  //This defines the organizaton of our UI
  struct ui
  {
    halp_meta(name, "Main")
    halp_meta(layout, halp::layouts::hbox)
    halp_meta(background, halp::colors::mid)

    //We have a bus to process the message sent by the operator
    struct bus
    {
      static void process_message(ui& self, processor_to_ui msg)
      {
        self.haptic_floor.anim.rect_activenodes = msg.send_activenodes;
        self.haptic_floor.anim.rect_passivenodes = msg.send_passivenodes;
      }
    };

    //We have our inputs
    struct {
      halp_meta(layout, halp::layouts::hbox)
      //The json layout file input
      struct
      {
        halp_meta(layout, halp::layouts::grid)
        halp_meta(columns, 1)
        halp_meta(name, "inputs")
        decltype(&ins::layout) int_widget = &ins::layout;
      } layoutInput;
      //The generated actuators inputs
      struct
      {
        halp_meta(layout, halp::layouts::grid)
        halp_meta(columns, 1)
        halp_meta(name, "inputs")
        decltype(&ins::in_i) int_widget = &ins::in_i;
      } nodesInput;
    } input;

    //The drawing of the haptic floor
    struct
    {
      halp_meta(layout, halp::layouts::hbox)
      halp_meta(name, "haptic_floor")
      halp_meta(width, 300)
      halp_meta(height, 200)
      halp::custom_actions_item<custom_anim> anim{};
    } haptic_floor;
  };

  //The operator function simply compiles the inputs into a list of float
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
    //We send information to the UI to display the mesh
    send_message(processor_to_ui{.send_activenodes=m_activenodes,.send_passivenodes=m_passivenodes});
  }
  void loadLayout();
};

}
