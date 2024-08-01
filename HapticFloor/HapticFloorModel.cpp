#include "HapticFloor.hpp"


namespace Example
{
void HapticFloor::loadLayout()
{
  //reset the layout in case of an already loaded layout
  m_activenodes={};
  m_passivenodes={};

  try
  {
    //parsing process to extract the data defining the haptic floor
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
          m_activenodes.emplace_back(x, y, channel, true);
        } else {
          m_passivenodes.emplace_back(x, y);
        }
      } else {
        return;
      }
    }
  }
  catch(...)
  {
  }
}
}
