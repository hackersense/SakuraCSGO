#pragma once

#include "../includes.hpp"

struct Count
{
    std::string text;
    float delay;
    int firstLayer;
    int secondLayer;
    bool isTake;
};

class Log : public singleton<Log>
{
public:
    void showLog(int count, Count layer) {
        const auto& text = layer.text;
        const auto w = render::get().text_width(fonts[NAME], text.c_str());
        const auto expiry = m_globals()->m_realtime < layer.delay;
        const auto y = 45 + (42 * (count - 1));
        const auto h = 12;
        const auto logW = (w < 150) ? 150 : (w + 15);
        const auto speed = 3;
        const auto layerSize = 15;

        layer.firstLayer = expiry ? min(layer.firstLayer + logW * 0.025, logW + layerSize) : max(layer.firstLayer - speed, 0);
        layer.secondLayer = expiry ? min(layer.secondLayer + logW * 0.015, logW) : max(layer.secondLayer - 2 * speed, 0);

        render::get().rect_filled(layer.firstLayer - layer.firstLayer, y, layer.firstLayer, h + 20, layer.isTake ? Color::Red : Color::Green);
        render::get().rect_filled(layer.secondLayer - layer.secondLayer, y, layer.secondLayer, h + 20, Color(16, 0, 0));

        render::get().text(fonts[NAME], layer.secondLayer - logW + 5, y + 3 + 6, Color(255, 255, 255), HFONT_CENTERED_NONE, text.c_str());

        logs[count] = layer;
        if (layer.firstLayer == 0) { //  && (count + 1) < logs.size()
            logs.erase(logs.begin() + count); //  + 1
        }
    }

    void onDraw() {
        for (int x = 0; x < logs.size(); x++) {
            showLog(x, logs[x]);
        }
    }

    void onPush(std::string message, float delay, bool isTake)
    {
        logs.push_back({ message, delay, 0, 0, isTake });
    }
private:
    std::vector<Count> logs;
};