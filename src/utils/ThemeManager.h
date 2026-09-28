#pragma once

#include <QColor>
#include <QString>
#include <QFont>

namespace OpenDaw {

struct Theme {
    // Classic Player visual language: deep blue-black surfaces, teal primary
    // action color, warm yellow secondary state, and cool light text.
    QColor background         {9, 16, 24};
    QColor surface            {21, 31, 40};
    QColor surfaceLight       {32, 44, 53};
    QColor border             {51, 65, 76};
    QColor text               {237, 244, 247};
    QColor textDim            {158, 171, 181};
    QColor accent             {19, 184, 173};
    QColor accentLight        {58, 218, 204};
    QColor trackBackground    {11, 20, 29};
    QColor clipBody           {19, 130, 122, 180};
    QColor clipBodySelected   {19, 184, 173, 210};
    QColor waveform           {164, 238, 226};
    QColor playhead           {255, 216, 74};
    QColor gridLine           {20, 31, 40};
    QColor gridLineMajor      {51, 65, 76};
    QColor meterGreen         {19, 184, 173};
    QColor meterYellow        {255, 216, 74};
    QColor meterRed           {244, 67, 54};
    QColor meterGlow          {0, 255, 230, 60};
    QColor muteButton         {255, 152, 0};
    QColor soloButton         {255, 235, 59};
    QColor recordArm          {244, 67, 54};
    QColor transportPlay      {76, 200, 100};
    QColor transportStop      {200, 200, 200};
    QColor transportRecord    {244, 67, 54};

    QColor midiClipBody           {36, 87, 127, 190};
    QColor midiClipBodySelected   {48, 134, 173, 215};
    QColor midiNotePreview        {112, 205, 235};
    QColor pianoRollBackground    {8, 14, 21};
    QColor pianoRollNote          {19, 184, 173};
    QColor pianoRollNoteSelected  {255, 216, 74};
    QColor pianoRollBlackKey      {7, 13, 19};
    QColor pianoRollWhiteKey      {42, 55, 64};
    QColor pianoRollGrid          {25, 39, 49};
    QColor pianoKeyWhite          {220, 229, 231};
    QColor pianoKeyBlack          {19, 29, 36};
    QColor pianoKeyBorder         {101, 120, 128};
    QColor pianoRollVelocityBar   {58, 218, 204};
};

class ThemeManager {
public:
    static ThemeManager& instance();

    const Theme& current() const { return theme_; }
    void setCurrent(const Theme& t) { theme_ = t; }

private:
    ThemeManager() = default;
    Theme theme_;
};

} // namespace OpenDaw
