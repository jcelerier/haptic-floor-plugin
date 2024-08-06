#include "HapticFloor.hpp"


namespace Example
{
void HapticFloor::loadLayout()
{
  //Reset the layout in case of an already loaded layout
  m_activenodes={};
  m_passivenodes={};

  try
  {
    //Parsing process to extract the data defining the haptic floor
    rapidjson::Document doc;
    doc.Parse(inputs.layout.value);
    if(doc.HasParseError())
      return;
    if(!doc.IsArray())
      return;

    for (auto& item : doc.GetArray()) {
      if (item.HasMember("coords") && item["coords"].IsArray() &&
         item["coords"].Size() == 2 &&
         item["coords"][0].IsInt() &&
         item["coords"][1].IsInt()) {

        int x = item["coords"][0].GetInt();
        int y = item["coords"][1].GetInt();
        bool isActive = false;
        int channel = 0;

        if (item.HasMember("type") && item["type"].IsString()) {
          std::string type = item["type"].GetString();
          isActive = (type == "active");

          if (isActive && item.HasMember("channel") && item["channel"].IsInt()) {
            channel = item["channel"].GetInt();
          }
        }

        if (isActive) {
          m_activenodes.emplace_back(node{.x = x,  .y= y, .channel = channel, .isActive = true});
        } else {
          m_passivenodes.emplace_back(node{.x = x,  .y= y});
        }
      } else {
        return;
      }
    }
    //Since the coordinates are cartesians we apply our transform to be able to draw an actual triangle mesh
    for (auto& n : m_activenodes){
      n.coordinateToMesh();
    }
    for (auto& n : m_passivenodes){
      n.coordinateToMesh();
    }
  }
  catch(...)
  {
  }
}
}
