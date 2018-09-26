#include <AnimationViewerGameState.hpp>

#include <AutoHGE.hpp>
#include <GameStateStack.hpp>
#include <hgefont.h>
#include <util.hpp>




extern float CREATURE_OPCODE_TICK_DT;
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




AnimationViewerGameState::AnimationViewerGameState(
	const CreatureDescriptor & creatureDescr,
	const Creature::OPCODE * opCodes,
	int nOpCodes)
: _creature(creatureDescr),
  _accDT(0),
  _mouseClickedX(0),
  _mouseClickedY(0)
{
	AutoHGE hge;
	
	_creature.setAlgorithm(opCodes, nOpCodes, CREATURE_OPCODE_TICK_DT);
	
	_rectContinue.x2 = (float) hge->System_GetState(HGE_SCREENWIDTH) - 15;
	_rectContinue.x1 = _rectContinue.x2 - 120;
	_rectContinue.y2 = (float) hge->System_GetState(HGE_SCREENHEIGHT) - 15;
	_rectContinue.y1 = _rectContinue.y2 - font->GetHeight() - 16;
	
	_rectDiscard = _rectContinue;
	_rectDiscard.x1 -= 130;
	_rectDiscard.x2 -= 130;
	
	_rectStore = _rectDiscard;
	_rectStore.x1 -= 130;
	_rectStore.x2 -= 130;
	
	_rectReplay = _rectStore;
	_rectReplay.x1 = 15;
	_rectReplay.x2 = _rectReplay.x1 + 70;
}




void AnimationViewerGameState::update(const GameStateUpdateEvent & evt)
{
	AutoHGE hge;
	
	// Update the physics system:
	_accDT += evt.dt;
	while (_accDT >= CREATURE_UPDATE_DT) {
		_creature.update(CREATURE_UPDATE_DT);
		_accDT -= CREATURE_UPDATE_DT;
	}
}




void AnimationViewerGameState::render(const GameStateRenderEvent & evt) const
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
		
		// "Continue" button:
		q.v[0].x = _rectContinue.x1;
		q.v[1].x = _rectContinue.x2;
		q.v[2].x = _rectContinue.x2;
		q.v[3].x = _rectContinue.x1;
		q.v[0].y = _rectContinue.y1;
		q.v[1].y = _rectContinue.y1;
		q.v[2].y = _rectContinue.y2;
		q.v[3].y = _rectContinue.y2;
		hge->Gfx_RenderQuad(&q);
		
		font->SetColor(0xAFFFFFFF);
		font->Render(
			(_rectContinue.x1 + _rectContinue.x2)/2,
			(_rectContinue.y1 + _rectContinue.y2)/2 - font->GetHeight()/2,
			HGETEXT_CENTER,
			"Resume generation");
		
		DRAW_BTN(
			_rectContinue,
			(hge->Input_GetKeyState(HGEK_LBUTTON) &&
			 _rectContinue.TestPoint(_mouseClickedX, _mouseClickedY) &&
			 _rectContinue.TestPoint(mouseX, mouseY)));
		
		// "Discard" button:
		q.v[0].x = _rectDiscard.x1;
		q.v[1].x = _rectDiscard.x2;
		q.v[2].x = _rectDiscard.x2;
		q.v[3].x = _rectDiscard.x1;
		q.v[0].y = _rectDiscard.y1;
		q.v[1].y = _rectDiscard.y1;
		q.v[2].y = _rectDiscard.y2;
		q.v[3].y = _rectDiscard.y2;
		hge->Gfx_RenderQuad(&q);
		
		font->SetColor(0xAFFFFFFF);
		font->Render(
			(_rectDiscard.x1 + _rectDiscard.x2)/2,
			(_rectDiscard.y1 + _rectDiscard.y2)/2 - font->GetHeight()/2,
			HGETEXT_CENTER,
			"Discard this animation");
		
		DRAW_BTN(
			_rectDiscard,
			(hge->Input_GetKeyState(HGEK_LBUTTON) &&
			 _rectDiscard.TestPoint(_mouseClickedX, _mouseClickedY) &&
			 _rectDiscard.TestPoint(mouseX, mouseY)));
		
		/*
		// "Store" button:
		q.v[0].x = _rectStore.x1;
		q.v[1].x = _rectStore.x2;
		q.v[2].x = _rectStore.x2;
		q.v[3].x = _rectStore.x1;
		q.v[0].y = _rectStore.y1;
		q.v[1].y = _rectStore.y1;
		q.v[2].y = _rectStore.y2;
		q.v[3].y = _rectStore.y2;
		hge->Gfx_RenderQuad(&q);
		
		font->SetColor(0xAFFFFFFF);
		font->Render(
			(_rectStore.x1 + _rectStore.x2)/2,
			(_rectStore.y1 + _rectStore.y2)/2 - font->GetHeight()/2,
			HGETEXT_CENTER,
			"Store this animation");
		
		DRAW_BTN(
			_rectStore,
			(hge->Input_GetKeyState(HGEK_LBUTTON) &&
			 _rectStore.TestPoint(_mouseClickedX, _mouseClickedY) &&
			 _rectStore.TestPoint(mouseX, mouseY)));
		*/
		
		// "Replay" button:
		q.v[0].x = _rectReplay.x1;
		q.v[1].x = _rectReplay.x2;
		q.v[2].x = _rectReplay.x2;
		q.v[3].x = _rectReplay.x1;
		q.v[0].y = _rectReplay.y1;
		q.v[1].y = _rectReplay.y1;
		q.v[2].y = _rectReplay.y2;
		q.v[3].y = _rectReplay.y2;
		hge->Gfx_RenderQuad(&q);
		
		font->SetColor(0xAFFFFFFF);
		font->Render(
			(_rectReplay.x1 + _rectReplay.x2)/2,
			(_rectReplay.y1 + _rectReplay.y2)/2 - font->GetHeight()/2,
			HGETEXT_CENTER,
			"Replay");
		
		DRAW_BTN(
			_rectReplay,
			(hge->Input_GetKeyState(HGEK_LBUTTON) &&
			 _rectReplay.TestPoint(_mouseClickedX, _mouseClickedY) &&
			 _rectReplay.TestPoint(mouseX, mouseY)));
	}
}




void AnimationViewerGameState::processInputEvent(const GameStateInputEvent & evt)
{
	// Update mouse params:
	if (evt.type == INPUT_MBUTTONDOWN && evt.key == HGEK_LBUTTON)
	{
		_mouseClickedX = evt.x;
		_mouseClickedY = evt.y;
	}
	
	// Exit the game state if user clicks on "Continue":
	if (evt.type == INPUT_MBUTTONUP &&
	    evt.key == HGEK_LBUTTON &&
	    _rectContinue.TestPoint(evt.x, evt.y) &&
	    _rectContinue.TestPoint(_mouseClickedX, _mouseClickedY))
	{
		evt.source->popState();
		return;
	}
	
	// Return to editor if user clicks on "Discard":
	if (evt.type == INPUT_MBUTTONUP &&
	    evt.key == HGEK_LBUTTON &&
	    _rectDiscard.TestPoint(evt.x, evt.y) &&
	    _rectDiscard.TestPoint(_mouseClickedX, _mouseClickedY))
	{
		evt.source->popState();
		evt.source->popState();
		return;
	}
	
	// Reset the creature if user clicks on "Replay":
	if (evt.type == INPUT_MBUTTONUP &&
	    evt.key == HGEK_LBUTTON &&
	    _rectReplay.TestPoint(evt.x, evt.y) &&
	    _rectReplay.TestPoint(_mouseClickedX, _mouseClickedY))
	{
		_creature.reset();
		return;
	}
}


