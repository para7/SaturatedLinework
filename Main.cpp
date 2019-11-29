
#include <Siv3D.hpp>
#include "SaturatedLinework.hpp"

struct
{
    double value;
    double max;
    double min = 0;
} lineCount, offsetRange, minThick, maxThick;

void Main()
{
    HSV lineColor = ColorF(0.11), frameColor = ColorF(0.2), windowColor = ColorF(0.9);

    Window::Resize(1280, 720);

    const Font font(90, Typeface::Default, FontStyle::BoldItalic);

    const Vec2 innerShapeCenter(800, 400);
    const auto innerShape = Ellipse(innerShapeCenter, 200, 100);
    // const auto innerShape = RectF(Arg::center = innerShapeCenter, 200, 100);
    SaturatedLinework linework(innerShape);

    // linework.SetSeed(0);

    lineCount.value = static_cast<int>(linework.getLineCount());
    offsetRange.value = linework.getOffsetRange();
    minThick.value = linework.getMinThickness();
    maxThick.value = linework.getMaxThickness();

    const uint64 seed = linework.getSeed();
    Print(U"seed:", seed);

    lineCount.max = 300;
    offsetRange.max = 400;
    minThick.max = 50;
    maxThick.max = 120;

    // Slider GUIのサイズ
    constexpr int32 label = 130;
    constexpr int32 slider = 250;

    // GUIのベース座標
    constexpr int32 x = 20;
    constexpr int32 y = 40;

    const Font colorFont(20);

    // デフォルトだと画面全体に描画されるので、画面より少し小さめの領域に描画するように再設定する
    const Rect virtualWindow = Scene::Rect().scaled(0.92);
    linework.setOuterRect(virtualWindow);

    while (System::Update())
    {
        virtualWindow.draw(windowColor);

        //集中線を描画
        linework.draw(lineColor);

        virtualWindow.drawFrame(0, 100, frameColor);

        font(U"集中線").drawAt(linework.getTargetShape().center, Palette::Black);

        SimpleGUI::Slider(U"Num{:.0f}"_fmt(lineCount.value), lineCount.value, lineCount.min, lineCount.max, Vec2(x, y), label, slider);
        SimpleGUI::Slider(U"PosRand{:.0f}"_fmt(offsetRange.value), offsetRange.value, offsetRange.min, offsetRange.max, Vec2(x, y + 40 * 1), label, slider);

        if (SimpleGUI::Slider(U"MinThick{:.0f}"_fmt(minThick.value), minThick.value, minThick.min, minThick.max, Vec2(x, y + 40 * 2), label, slider))
        {
            maxThick.value = Max(maxThick.value, minThick.value);
        }

        if (SimpleGUI::Slider(U"MaxThick{:.0f}"_fmt(maxThick.value), maxThick.value, maxThick.min, maxThick.max, Vec2(x, y + 40 * 3), label, slider))
        {
            minThick.value = Min(maxThick.value, minThick.value);
        }

        linework.setLineCount(static_cast<int>(lineCount.value));
        linework.setOffsetRange(offsetRange.value);
        linework.setThickness(minThick.value, maxThick.value);

        if (SimpleGUI::Button(U"SeedReset", Vec2(x + 160, y + 40 * 4)))
        {
            ClearPrint();
            linework.setSeed(RandomUint64());
            Print(U"seed:",linework.getSeed());
        }

        if (SimpleGUI::Button(U"RandomSet", Vec2(x, y + 40 * 4)))
        {
            lineCount.value = Random(lineCount.min, lineCount.max);
            offsetRange.value = Random(offsetRange.min, offsetRange.max);
            minThick.value = Random(minThick.min, minThick.max);
            maxThick.value = Random(minThick.value, maxThick.max);

            linework.setLineCount(static_cast<size_t>(lineCount.value))
                .setThickness(static_cast<double>(minThick.value), static_cast<double>(maxThick.value))
                .setOffsetRange(static_cast<double>(offsetRange.value));
        }

        if (SimpleGUI::Button(U"Generate", Vec2(x, y + 40 * 5)))
        {
            linework.generate();
        }

        const int width = 160;
        Rect(x, y + (int)(40 * 6.35), width * 3, 27 * 1).draw(Palette::White).drawFrame(0, 1, Palette::Black);

        colorFont(U"LineColor").draw(x + width * 0, y + 40 * 6.35, Palette::Black);
        colorFont(U"frameColor").draw(x + width * 1, y + 40 * 6.35, Palette::Black);
        colorFont(U"windowColor").draw(x + width * 2, y + 40 * 6.35, Palette::Black);

        SimpleGUI::ColorPicker(lineColor, Vec2(x + 170 * 0, y + 40 * 7));
        SimpleGUI::ColorPicker(frameColor, Vec2(x + width * 1, y + 40 * 7));
        SimpleGUI::ColorPicker(windowColor, Vec2(x + width * 2, y + 40 * 7));
    }
}
