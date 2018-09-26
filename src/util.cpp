#include <util.hpp>

#include <hgeFont.h>

extern hgeFont * font;

#define CLAW_COLOR              0xFF00FFFF
#define CLAW_GRIP_COLOR         0xFFFF0000
#define CLAW_RADIUS             4
#define RIGIDBAR_COLOR          0xFF00FFFF
#define SPRING_RELAXED_COLOR       0x3FFFFF00
#define SPRING_CONTRACTED_COLOR    0x8FFFCF00
#define SPRING_STRETCH_COLOR       0x8FCFFF00





std::string toString(int i)
{
	char str[128];
	sprintf(str, "%d", i);
	return std::string(str);
}




std::string toString(unsigned int ui)
{
	char str[128];
	sprintf(str, "%u", ui);
	return std::string(str);
}




std::string toString(float f)
{
	char str[128];
	sprintf(str, "%g", f);
	return std::string(str);
}




std::string toString(double d)
{
	char str[128];
	sprintf(str, "%g", d);
	return std::string(str);
}




void renderCircle(HGE * hge, const Vector2D & center, float radius, DWORD color)
{
	renderCircle(hge, center.x, center.y, radius, color);
}




void renderCircle(HGE * hge, float centerX, float centerY, float radius, DWORD color)
{
	const int NUM_STEPS = 7;
	for (float angle = 0, nextAngle = 2 * M_PI / NUM_STEPS; nextAngle <= 2 * M_PI; angle += 2 * M_PI / NUM_STEPS, nextAngle += 2 * M_PI / NUM_STEPS)
		hge->Gfx_RenderLine(centerX + radius * cos(angle),
		                    centerY + radius * sin(angle),
		                    centerX + radius * cos(nextAngle),
		                    centerY + radius * sin(nextAngle),
		                    color);
}




void renderCreature(HGE * hge, const Creature * creature)
{
	int scrollX = (int) creature->getCurrentPositionCentroid().x;
	int dx = -scrollX + hge->System_GetState(HGE_SCREENWIDTH)/2;
	
	// The creature:
	{
		for (int i = 0; i < creature->getNumClaws(); i++) {
			DWORD color;
			if (creature->getClaw(i)->isGripping())
				color = CLAW_GRIP_COLOR;
			else
				color = CLAW_COLOR;
				
			renderCircle(hge, creature->getClaw(i)->getCurrentX() + dx, creature->getClaw(i)->getCurrentY(), CLAW_RADIUS, color);
		}
		
		for (int i = 0; i < creature->getNumBones(); i++)
			hge->Gfx_RenderLine(creature->getBone(i)->getP1()->getCurrentX() + dx,
			                    creature->getBone(i)->getP1()->getCurrentY(),
			                    creature->getBone(i)->getP2()->getCurrentX() + dx,
			                    creature->getBone(i)->getP2()->getCurrentY(),
			                    RIGIDBAR_COLOR);
		
		for (int i = 0; i < creature->getNumMuscles(); i++) {
			DWORD color;
			
			if (creature->getMuscle(i)->getLength() < creature->getInitMuscleLength(i))
				color = SPRING_CONTRACTED_COLOR;
			else
			if (creature->getMuscle(i)->getLength() > creature->getInitMuscleLength(i))
				color = SPRING_STRETCH_COLOR;
			else
				color = SPRING_RELAXED_COLOR;
			
			hge->Gfx_RenderLine(creature->getMuscle(i)->getP1()->getCurrentX() + dx,
			                    creature->getMuscle(i)->getP1()->getCurrentY(),
			                    creature->getMuscle(i)->getP2()->getCurrentX() + dx,
			                    creature->getMuscle(i)->getP2()->getCurrentY(),
			                    color);
		}
	}
	
	// The floor:
	{
		hgeQuad q;
		q.blend = BLEND_DEFAULT;
		q.tex = NULL;
		q.v[0].col = 0x5F7F7F7F;
		q.v[0].x = 0;
		q.v[0].y = creature->getYFloor();
		q.v[0].tx = q.v[0].ty = q.v[0].z = 0;
		
		q.v[1].col = 0x5F7F7F7F;
		q.v[1].x = (float) hge->System_GetState(HGE_SCREENWIDTH);
		q.v[1].y = creature->getYFloor();
		q.v[1].tx = q.v[1].ty = q.v[1].z = 0;
		
		q.v[2].col = 0x0F7F7F7F;
		q.v[2].x = (float) hge->System_GetState(HGE_SCREENWIDTH);
		q.v[2].y = (float) hge->System_GetState(HGE_SCREENHEIGHT);
		q.v[2].tx = q.v[2].ty = q.v[2].z = 0;
		
		q.v[3].col = 0x0F7F7F7F;
		q.v[3].x = 0;
		q.v[3].y = (float) hge->System_GetState(HGE_SCREENHEIGHT);
		q.v[3].tx = q.v[3].ty = q.v[3].z = 0;
		
		hge->Gfx_RenderQuad(&q);
		
		for (int x = scrollX - (scrollX % 50);
		     x < scrollX + hge->System_GetState(HGE_SCREENWIDTH);
		     x += 50)
		{
			hge->Gfx_RenderLine((float) (x - scrollX),
			                    (float) creature->getYFloor(),
			                    (float) (x - scrollX),
			                    (float) creature->getYFloor() + 5,
			                    0x7FFFFFFF);
		}
		
		font->SetColor(0xFFFFFFFF);
		
		for (int x = scrollX - (scrollX % 100);
		     x < scrollX + hge->System_GetState(HGE_SCREENWIDTH);
		     x += 100)
		{
			hge->Gfx_RenderLine((float) (x - scrollX),
			                    (float) creature->getYFloor(),
			                    (float) (x - scrollX),
			                    (float) creature->getYFloor() + 10,
			                    0x7FFFFFFF);
			
			font->printf((float) (x - scrollX),
			             (float) creature->getYFloor() + 10,
			             HGETEXT_CENTER,
			             "%d",
			             (int) (x - 300));
		}
	}
}




void renderFilledArc(HGE * hge,
                     float xCenter, float yCenter,
                     float radius,
                     float fromAngle, float toAngle,
                     DWORD color)
{
	static const int NUM_STEPS = 4;
	
	hgeTriple t;
	t.blend = BLEND_DEFAULT;
	t.tex = NULL;
	t.v[0].col = t.v[1].col = t.v[2].col = color;
	t.v[0].tx  = t.v[1].tx  = t.v[2].tx  = 0;
	t.v[0].ty  = t.v[1].ty  = t.v[2].ty  = 0;
	t.v[0].z   = t.v[1].z   = t.v[2].z   = 0;
	
	t.v[0].x = xCenter;
	t.v[0].y = yCenter;
	
	const float deltaAngle = toAngle - fromAngle;
	
	float angle = fromAngle;
	float nextAngle = angle + deltaAngle/NUM_STEPS;
	
	for (int i = 0; i < NUM_STEPS; i++) {
		t.v[1].x = xCenter + radius * cos(angle);
		t.v[1].y = yCenter + radius * sin(angle);
		
		t.v[2].x = xCenter + radius * cos(nextAngle);
		t.v[2].y = yCenter + radius * sin(nextAngle);
		
		hge->Gfx_RenderTriple(&t);
		
		angle     += deltaAngle/NUM_STEPS;
		nextAngle += deltaAngle/NUM_STEPS;
	}
}




void renderChamferRect(HGE * hge,
                       float x1, float x2,
                       float y1, float y2,
                       float chamferRadius,
                       DWORD color)
{
	hgeQuad q;
	q.blend = BLEND_DEFAULT;
	q.tex = NULL;
	q.v[0].col = q.v[1].col = q.v[2].col = q.v[3].col = color;
	q.v[0].tx  = q.v[1].tx  = q.v[2].tx  = q.v[3].tx  = 0;
	q.v[0].ty  = q.v[1].ty  = q.v[2].ty  = q.v[3].ty  = 0;
	q.v[0].z   = q.v[1].z   = q.v[2].z   = q.v[3].z   = 0;
	
	// Main fill:
	q.v[0].x = x1 + chamferRadius;
	q.v[1].x = x2 - chamferRadius;
	q.v[2].x = x2 - chamferRadius;
	q.v[3].x = x1 + chamferRadius;
	
	q.v[0].y = y1 + chamferRadius;
	q.v[1].y = y1 + chamferRadius;
	q.v[2].y = y2 - chamferRadius;
	q.v[3].y = y2 - chamferRadius;
	
	hge->Gfx_RenderQuad(&q);
	
	// Left edge:
	q.v[0].x = x1;
	q.v[1].x = x1 + chamferRadius;
	q.v[2].x = x1 + chamferRadius;
	q.v[3].x = x1;
	
	q.v[0].y = y1 + chamferRadius;
	q.v[1].y = y1 + chamferRadius;
	q.v[2].y = y2 - chamferRadius;
	q.v[3].y = y2 - chamferRadius;
	
	hge->Gfx_RenderQuad(&q);
	
	// Right edge:
	q.v[0].x = x2 - chamferRadius;
	q.v[1].x = x2;
	q.v[2].x = x2;
	q.v[3].x = x2 - chamferRadius;
	
	q.v[0].y = y1 + chamferRadius;
	q.v[1].y = y1 + chamferRadius;
	q.v[2].y = y2 - chamferRadius;
	q.v[3].y = y2 - chamferRadius;
	
	hge->Gfx_RenderQuad(&q);
	
	// Top edge:
	q.v[0].x = x1 + chamferRadius;
	q.v[1].x = x2 - chamferRadius;
	q.v[2].x = x2 - chamferRadius;
	q.v[3].x = x1 + chamferRadius;
	
	q.v[0].y = y1;
	q.v[1].y = y1;
	q.v[2].y = y1 + chamferRadius;
	q.v[3].y = y1 + chamferRadius;
	
	hge->Gfx_RenderQuad(&q);
	
	// Bottom edge:
	q.v[0].x = x1 + chamferRadius;
	q.v[1].x = x2 - chamferRadius;
	q.v[2].x = x2 - chamferRadius;
	q.v[3].x = x1 + chamferRadius;
	
	q.v[0].y = y2 - chamferRadius;
	q.v[1].y = y2 - chamferRadius;
	q.v[2].y = y2;
	q.v[3].y = y2;
	
	hge->Gfx_RenderQuad(&q);
	
	// Top-left corner:
	renderFilledArc(hge,
	                x1 + chamferRadius, y1 + chamferRadius,
	                chamferRadius, 
	                M_PI, 3*M_PI/2,
	                color);
	
	// Top-right corner:
	renderFilledArc(hge,
	                x2 - chamferRadius, y1 + chamferRadius,
	                chamferRadius, 
	                0, -M_PI/2,
	                color);
	
	// Bottom-right corner:
	renderFilledArc(hge,
	                x2 - chamferRadius, y2 - chamferRadius,
	                chamferRadius, 
	                0, M_PI/2,
	                color);
	
	// Bottom-left corner:
	renderFilledArc(hge,
	                x1 + chamferRadius, y2 - chamferRadius,
	                chamferRadius, 
	                M_PI/2, M_PI,
	                color);
}


