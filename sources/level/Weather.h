//
// Created by bison on 18-08-25.
//

#ifndef SANDBOX_WEATHER_H
#define SANDBOX_WEATHER_H

#include "data/WeatherData.h"
#include "data/GameData.h"

void InitWeather(GameData &data, int numDrops, float mapWidth, float mapHeight);
void UpdateWeather(GameData &data, bool outdoor, float dt);
void DrawWeather(const WeatherData &weather, Color color);
void SetWeatherType(GameData &data, WeatherType type, bool outdoor);
void StartWeatherSound(SoundData& soundData, WeatherData &weather, bool outdoor);
void StopWeatherSound(SoundData& soundData, WeatherData &weather);

#endif //SANDBOX_WEATHER_H
