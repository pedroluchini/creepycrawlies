#include <CreatureTestGameState.hpp>

#include <AutoHGE.hpp>
#include <GameStateStack.hpp>
#include <hgefont.h>
#include <util.hpp>

extern float CREATURE_UPDATE_DT;
extern hgeFont * font;




#define DRAW_BTN(rect, isPressed) \
{\
	DWORD colHighlight = 0x7FFFFFFF;\
	DWORD colShade     = 0x7F000000;\
	hge->Gfx_RenderLine(rect.x1, rect.y1, rect.x2, rect.y1, (isPressed ? colShade : colHighlight));\
	hge->Gfx_RenderLine(rect.x2, rect.y1, rect.x2, rect.y2, (isPressed ? colHighlight : colShade));\
	hge->Gfx_RenderLine(rect.x2, rect.y2, rect.x1, rect.y2, (isPressed ? colHighlight : colShade));\
	hge->Gfx_RenderLine(rect.x1, rect.y2, rect.x1, rect.y1, (isPressed ? colShade : colHighlight));\
}




#define N_KEYS 9
static int _stretchKeys [N_KEYS] = {HGEK_Q, HGEK_W, HGEK_E, HGEK_R, HGEK_T, HGEK_Y, HGEK_U, HGEK_I, HGEK_O};
static int _contractKeys[N_KEYS] = {HGEK_A, HGEK_S, HGEK_D, HGEK_F, HGEK_G, HGEK_H, HGEK_J, HGEK_K, HGEK_L};




CreatureTestGameState::CreatureTestGameState(const CreatureDescriptor & creature)
: _creature(creature),
  _mouseClickedX(0),
  _mouseClickedY(0)
{
	AutoHGE hge;
	
	_accDT = 0;
	
	_rectDone.x2 = (float) hge->System_GetState(HGE_SCREENWIDTH) - 15;
	_rectDone.x1 = _rectDone.x2 - 70;
	_rectDone.y2 = (float) hge->System_GetState(HGE_SCREENHEIGHT) - 15;
	_rectDone.y1 = _rectDone.y2 - font->GetHeight() - 16;
}




void CreatureTestGameState::update(const GameStateUpdateEvent & evt)
{
	AutoHGE hge;
	
	// Update state based on keyboard input:
	for (int i = 0; i < min(N_KEYS, _creature.getNumClaws()); i++)
		_creature.getClaw(i)->setGripping(hge->Input_GetKeyState(HGEK_1 + i));
	
	for (int i = 0; i < min(N_KEYS, _creature.getNumMuscles()); i++) {
		float initLength = _creature.getInitMuscleLength(i);
		
		// Stretched?
		if (hge->Input_GetKeyState(_stretchKeys[i]) && !hge->Input_GetKeyState(_contractKeys[i])) {
			_creature.getMuscle(i)->setLength(initLength * 2);
		}
		else
		// Contracted?
		if (!hge->Input_GetKeyState(_stretchKeys[i]) && hge->Input_GetKeyState(_contractKeys[i])) {
			_creature.getMuscle(i)->setLength(initLength / 2);
		}
		else {
			// Relaxed:
			_creature.getMuscle(i)->setLength(initLength);
		}
	}
	
	// Update the physics system:
	_accDT += evt.dt;
	while (_accDT >= CREATURE_UPDATE_DT) {
		_creature.update(CREATURE_UPDATE_DT);
		_accDT -= CREATURE_UPDATE_DT;
	}
}




void CreatureTestGameState::render(const GameStateRenderEvent & evt) const
{
	AutoHGE hge;
	
	float mouseX, mouseY;
	hge->Input_GetMousePos(&mouseX, &mouseY);
	
	
	renderCreature(hge, &_creature);
	
	// Tool bar:
	{
		hgeQuad q;
		q.blend = BLEND_DEFAULT;
		q.tex = NULL;
		q.v[0].col = q.v[1].col = q.v[2].col = q.v[3].col = 0x7F007F7F;
		q.v[0].tx  = q.v[1].tx  = q.v[2].tx  = q.v[3].tx  = 0;
		q.v[0].ty  = q.v[1].ty  = q.v[2].ty  = q.v[3].ty  = 0;
		q.v[0].z   = q.v[1].z   = q.v[2].z   = q.v[3].z   = 0;
		q.v[0].x = _rectDone.x1;
		q.v[1].x = _rectDone.x2;
		q.v[2].x = _rectDone.x2;
		q.v[3].x = _rectDone.x1;
		q.v[0].y = _rectDone.y1;
		q.v[1].y = _rectDone.y1;
		q.v[2].y = _rectDone.y2;
		q.v[3].y = _rectDone.y2;
		hge->Gfx_RenderQuad(&q);
		
		font->SetColor(0xAFFFFFFF);
		font->Render(
			(_rectDone.x1 + _rectDone.x2)/2,
			(_rectDone.y1 + _rectDone.y2)/2 - font->GetHeight()/2,
			HGETEXT_CENTER,
			"Done");
		
		DRAW_BTN(
			_rectDone,
			(hge->Input_GetKeyState(HGEK_LBUTTON) &&
			 _rectDone.TestPoint(_mouseClickedX, _mouseClickedY) &&
			 _rectDone.TestPoint(mouseX, mouseY)));
		
	}
	
	// Display controls
	{
		// Claws:
		std::string str = "Hold to grip with claws: ";
		for (int i = 0; i < min(N_KEYS, _creature.getNumClaws()); i++) {
			if (i != 0)
				str += ", ";
			str += hge->Input_GetKeyName(HGEK_1 + i);
		}
		
		str += "\n";
		
		// Muscles:
		str += "Hold to contract muscles: ";
		for (int i = 0; i < min(N_KEYS, _creature.getNumMuscles()); i++) {
			if (i != 0)
				str += ", ";
			str += hge->Input_GetKeyName(_contractKeys[i]);
		}
		str += "\n";
		str += "Hold to stretch muscles: ";
		for (int i = 0; i < min(N_KEYS, _creature.getNumMuscles()); i++) {
			if (i != 0)
				str += ", ";
			str += hge->Input_GetKeyName(_stretchKeys[i]);
		}
		
		font->SetColor(0xFFFFFFFF);
		font->Render(
			10,
			hge->System_GetState(HGE_SCREENHEIGHT) - 10 - 3 * font->GetHeight(),
			HGETEXT_LEFT,
			str.c_str());
	}
}




void CreatureTestGameState::processInputEvent(const GameStateInputEvent & evt)
{
	// Update mouse params:
	if (evt.type == INPUT_MBUTTONDOWN && evt.key == HGEK_LBUTTON)
	{
		_mouseClickedX = evt.x;
		_mouseClickedY = evt.y;
	}
	
	// Exit the game state if user clicks on "Done":
	if (evt.type == INPUT_MBUTTONUP &&
	    evt.key == HGEK_LBUTTON &&
	    _rectDone.TestPoint(evt.x, evt.y) &&
	    _rectDone.TestPoint(_mouseClickedX, _mouseClickedY))
	{
		evt.source->popState();
	}
	
}

