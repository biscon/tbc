//
// Created by bison on 18-08-25.
//

#ifndef SANDBOX_WEATHER_H
#define SANDBOX_WEATHER_H

#include "data/WeatherData.h"

void InitWeather(WeatherData &weather, int numDrops, float mapWidth, float mapHeight);
void UpdateWeather(WeatherData &weather, float dt);
void DrawWeather(const WeatherData &weather, Color color);

#endif //SANDBOX_WEATHER_H
