#ifndef UTIL_HPP
#define UTIL_HPP

#include <Creature.hpp>
#include <hge.h>
#include <string>
#include <Vector2D.hpp>

std::string toString(int i);
std::string toString(unsigned int ui);
std::string toString(float f);
std::string toString(double d);

void renderCircle(HGE * hge, const Vector2D & center, float radius, DWORD color);
void renderCircle(HGE * hge, float centerX, float centerY, float radius, DWORD color);

void renderFilledArc(HGE * hge,
                     float xCenter, float yCenter,
                     float radius,
                     float fromAngle, float toAngle,
                     DWORD color);

void renderChamferRect(HGE * hge,
                       float x1, float x2,
                       float y1, float y2,
                       float chamferRadius,
                       DWORD color);

void renderCreature(HGE * hge, const Creature * creature);

#endif
