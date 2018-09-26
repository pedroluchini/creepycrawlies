#include <CreatureEditorGameState.hpp>

#include <AutoHGE.hpp>
#include <CreatureTestGameState.hpp>
#include <GameStateStack.hpp>
#include <EvolveAnimationGameState.hpp>
#include <util.hpp>

extern hgeFont * font;
extern HTEXTURE texSprites;




#define TOOLBAR_W    150
#define TOOLBAR_H    280
#define CLAW_RADIUS    4
#define CLAW_COLOR   0xFF00FFFF
#define BONE_COLOR   0xFF00FFFF
#define MUSCLE_COLOR 0x8FFFFF00
#define BLINK_FREQ    16
#define HIT_MARGIN    13
#define INVALID_ID   0x7FFFFFFF

#define DRAW_BTN(rect, isPressed) \
{\
	DWORD colHighlight = 0x7FFFFFFF;\
	DWORD colShade     = 0x7F000000;\
	hge->Gfx_RenderLine(rect.x1, rect.y1, rect.x2, rect.y1, (isPressed ? colShade : colHighlight));\
	hge->Gfx_RenderLine(rect.x2, rect.y1, rect.x2, rect.y2, (isPressed ? colHighlight : colShade));\
	hge->Gfx_RenderLine(rect.x2, rect.y2, rect.x1, rect.y2, (isPressed ? colHighlight : colShade));\
	hge->Gfx_RenderLine(rect.x1, rect.y2, rect.x1, rect.y1, (isPressed ? colShade : colHighlight));\
}




template<typename T>
static T max(const T & x, const T & y)
{
	if (x > y)
		return x;
	else
		return y;
}




CreatureEditorGameState::CreatureEditorGameState()
: _selectedTool(TOOL_EDIT_CLAWS),
  _selectedToolMode(TOOLMODE_CREATE),
  _sprCreateToolMode(NULL),
  _sprSelectToolMode(NULL)
{
	AutoHGE hge;
	
	_rectToolBar.x1 = (float) hge->System_GetState(HGE_SCREENWIDTH) - TOOLBAR_W;
	_rectToolBar.x2 = (float) hge->System_GetState(HGE_SCREENWIDTH);
	_rectToolBar.y1 = 0;
	_rectToolBar.y2 = TOOLBAR_H;
	
	hge->Input_GetMousePos(&_prevMouseX, &_prevMouseY);
	_mouseClickedX = _mouseClickedY = 0;
	_clickedOnSelection = false;
	
	_sprCreateToolMode = new hgeSprite(texSprites,  0, 0, 22, 22);
	_sprSelectToolMode = new hgeSprite(texSprites, 22, 0, 22, 22);
	_sprCreateToolMode->SetHotSpot(11, 11);
	_sprSelectToolMode->SetHotSpot(11, 11);
	
	_rectEditClaws.x1 = (float) hge->System_GetState(HGE_SCREENWIDTH) - 140;
	_rectEditClaws.x2 = _rectEditClaws.x1 + 130;
	_rectEditClaws.y1 = 10;
	_rectEditClaws.y2 = _rectEditClaws.y1 + font->GetHeight() + 16;
	
	_rectEditBones = _rectEditClaws;
	_rectEditBones.y1 += 2*font->GetHeight() + 15;
	_rectEditBones.y2 += 2*font->GetHeight() + 15;
	
	_rectEditMuscles = _rectEditBones;
	_rectEditMuscles.y1 += 2*font->GetHeight() + 15;
	_rectEditMuscles.y2 += 2*font->GetHeight() + 15;
	
	_rectCreateMode = _rectEditMuscles;
	_rectCreateMode.x2 = _rectCreateMode.x1 + (_rectCreateMode.x2 - _rectCreateMode.x1)/2 - 9;
	_rectCreateMode.y1 += 2*font->GetHeight() + 25;
	_rectCreateMode.y2 += 2*font->GetHeight() + 25;
	
	_rectSelectMode = _rectEditMuscles;
	_rectSelectMode.x1 = _rectSelectMode.x1 + (_rectSelectMode.x2 - _rectSelectMode.x1)/2 + 9;
	_rectSelectMode.y1 += 2*font->GetHeight() + 25;
	_rectSelectMode.y2 += 2*font->GetHeight() + 25;
	
	_rectTestCreature.x1 = _rectEditMuscles.x1;
	_rectTestCreature.x2 = _rectEditMuscles.x2;
	_rectTestCreature.y1 = _rectSelectMode.y1 + 2*font->GetHeight() + 45;
	_rectTestCreature.y2 = _rectSelectMode.y2 + 2*font->GetHeight() + 45;
	
	_rectGenerateAnimation = _rectTestCreature;
	_rectGenerateAnimation.y1 += 2*font->GetHeight() + 15;
	_rectGenerateAnimation.y2 += 2*font->GetHeight() + 15;
	
}




CreatureEditorGameState::~CreatureEditorGameState()
{
	delete _sprCreateToolMode;
	delete _sprSelectToolMode;
}




bool CreatureEditorGameState::getClaw(int id, float * xPos, float * yPos) const
{
	if (_creatureClaws.count(id) > 0) {
		const Vector2D & claw = _creatureClaws.find(id)->second;
		
		*xPos = claw.x;
		*yPos = claw.y;
		
		return true;
	}
	else
		return false;
}




bool CreatureEditorGameState::getBone(int id, int * idClaw1, int * idClaw2) const
{
	if (_creatureBones.count(id) > 0) {
		const std::pair<int, int> & bone = _creatureBones.find(id)->second;
		
		*idClaw1 = bone.first;
		*idClaw2 = bone.second;
		
		return true;
	}
	else
		return false;
}




bool CreatureEditorGameState::getMuscle(int id, int * idClaw1, int * idClaw2) const
{
	if (_creatureMuscles.count(id) > 0) {
		const std::pair<int, int> & muscle = _creatureMuscles.find(id)->second;
		
		*idClaw1 = muscle.first;
		*idClaw2 = muscle.second;
		
		return true;
	}
	else
		return false;
}




void CreatureEditorGameState::update(const GameStateUpdateEvent & evt)
{
	AutoHGE hge;
	
	// Get current mouse position:
	float mouseX, mouseY;
	hge->Input_GetMousePos(&mouseX, &mouseY);
	
	float mouseDX = mouseX - _prevMouseX;
	float mouseDY = mouseY - _prevMouseY;
	
	
	// Drag recently-created claw:
	if (_selectedTool == TOOL_EDIT_CLAWS &&
	    _selectedToolMode == TOOLMODE_CREATE &&
	    hge->Input_GetKeyState(HGEK_LBUTTON) &&
	    !_rectToolBar.TestPoint(_mouseClickedX, _mouseClickedY) && 
	    !_selectedClaws.empty())
	{
		int idClaw = *_selectedClaws.begin();
		_creatureClaws[idClaw].x += mouseDX;
		_creatureClaws[idClaw].y += mouseDY;
	}
	
	// Drag selected claws:
	if (_selectedTool == TOOL_EDIT_CLAWS &&
	    _selectedToolMode == TOOLMODE_SELECT &&
	    hge->Input_GetKeyState(HGEK_LBUTTON) &&
	    !_rectToolBar.TestPoint(_mouseClickedX, _mouseClickedY) && 
	    !_selectedClaws.empty() &&
	    _clickedOnSelection)
	{
		for (std::set<int>::iterator itr = _selectedClaws.begin();
		     itr != _selectedClaws.end();
		     itr++)
		{
			_creatureClaws[*itr] += Vector2D(mouseDX, mouseDY);
			
			// Notify listeners:
			{
				CreatureEditorClawEvent evt(this, *itr);
				_subject.notify(&ICreatureEditorListener::onClawMoved, evt);
			}
		}
	}
	
	// Drag selected bones:
	if (_selectedTool == TOOL_EDIT_BONES &&
	    _selectedToolMode == TOOLMODE_SELECT &&
	    hge->Input_GetKeyState(HGEK_LBUTTON) &&
	    !_rectToolBar.TestPoint(_mouseClickedX, _mouseClickedY) && 
	    !_selectedBones.empty() &&
	    _clickedOnSelection)
	{
		std::set<int> idClaws;
		
		for (std::set<int>::iterator itr = _selectedBones.begin();
		     itr != _selectedBones.end();
		     itr++)
		{
			idClaws.insert(_creatureBones[*itr].first);
			idClaws.insert(_creatureBones[*itr].second);
		}
		
		for (std::set<int>::iterator itr = idClaws.begin();
		     itr != idClaws.end();
		     itr++)
		{	
			_creatureClaws[*itr] += Vector2D(mouseDX, mouseDY);
			
			// Notify listeners:
			{
				CreatureEditorClawEvent evt(this, *itr);
				_subject.notify(&ICreatureEditorListener::onClawMoved, evt);
			}
		}
	}
	
	// Drag selected muscles:
	if (_selectedTool == TOOL_EDIT_MUSCLES &&
	    _selectedToolMode == TOOLMODE_SELECT &&
	    hge->Input_GetKeyState(HGEK_LBUTTON) &&
	    !_selectedMuscles.empty() &&
	    _clickedOnSelection)
	{
		std::set<int> idClaws;
		
		for (std::set<int>::iterator itr = _selectedMuscles.begin();
		     itr != _selectedMuscles.end();
		     itr++)
		{
			idClaws.insert(_creatureMuscles[*itr].first);
			idClaws.insert(_creatureMuscles[*itr].second);
		}
		
		for (std::set<int>::iterator itr = idClaws.begin();
		     itr != idClaws.end();
		     itr++)
		{	
			_creatureClaws[*itr] += Vector2D(mouseDX, mouseDY);
			
			// Notify listeners:
			{
				CreatureEditorClawEvent evt(this, *itr);
				_subject.notify(&ICreatureEditorListener::onClawMoved, evt);
			}
		}
	}
	
	
	// Update mouse position for next frame:
	_prevMouseX = mouseX;
	_prevMouseY = mouseY;
}




void CreatureEditorGameState::processInputEvent(const GameStateInputEvent & evt)
{
	AutoHGE hge;
	int screenW = hge->System_GetState(HGE_SCREENWIDTH);
	int screenH = hge->System_GetState(HGE_SCREENHEIGHT);
	
	// Update mouse params:
	if (evt.type == INPUT_MBUTTONDOWN && evt.key == HGEK_LBUTTON)
	{
		_mouseClickedX = evt.x;
		_mouseClickedY = evt.y;
	}
	
	if (evt.type == INPUT_MBUTTONDOWN &&
	    evt.key == HGEK_LBUTTON &&
	    _selectedToolMode == TOOLMODE_CREATE)
	{
		_clickedOnSelection = false;
	}
	
	// Check for tool-change:
	{
		if (_rectEditClaws.TestPoint(evt.x, evt.y)) {
			if (evt.type == INPUT_MBUTTONDOWN && evt.key == HGEK_LBUTTON)
				_selectedTool = TOOL_EDIT_CLAWS;
			
			return;
		}
		
		if (_rectEditBones.TestPoint(evt.x, evt.y)) {
			if (evt.type == INPUT_MBUTTONDOWN && evt.key == HGEK_LBUTTON)
				_selectedTool = TOOL_EDIT_BONES;
			
			return;
		}
		
		if (_rectEditMuscles.TestPoint(evt.x, evt.y)) {
			if (evt.type == INPUT_MBUTTONDOWN && evt.key == HGEK_LBUTTON)
				_selectedTool = TOOL_EDIT_MUSCLES;
			
			return;
		}
	}
	
	// Check for tool-mode-change:
	{
		if (_rectCreateMode.TestPoint(evt.x, evt.y)) {
			if (evt.type == INPUT_MBUTTONDOWN && evt.key == HGEK_LBUTTON)
				_selectedToolMode = TOOLMODE_CREATE;
			
			return;
		}
		if (_rectSelectMode.TestPoint(evt.x, evt.y)) {
			if (evt.type == INPUT_MBUTTONDOWN && evt.key == HGEK_LBUTTON)
				_selectedToolMode = TOOLMODE_SELECT;
			
			return;
		}
	}
	
	if (_selectedTool == TOOL_EDIT_CLAWS &&
	    _selectedToolMode == TOOLMODE_SELECT &&
	    evt.type == INPUT_MBUTTONDOWN)
	{
		// If user clicks on a selected claw...
		if (_selectedClaws.count(_findHighlightedClaw(evt.x, evt.y)) != 0)
		{
			// De-select it if Alt is pressed:
			if (evt.flags & HGEINP_ALT)
				_selectedClaws.erase(_findHighlightedClaw(evt.x, evt.y));
			
			_clickedOnSelection = true;
			return;
		}
		else
		// Change selection if user clicks on an unselected claw:
		if (_findHighlightedClaw(evt.x, evt.y) != INVALID_ID)
		{
			if (!(evt.flags & (HGEINP_CTRL | HGEINP_SHIFT | HGEINP_ALT)))
				_selectedClaws.clear();
			
			if (!(evt.flags & HGEINP_ALT)) {
				_selectedClaws.insert(_findHighlightedClaw(evt.x, evt.y));
				_clickedOnSelection = true;
			}
			else
				_clickedOnSelection = false;
			
			return;
		}
		else
			_clickedOnSelection = false;
	}
	
	if (_selectedTool == TOOL_EDIT_BONES &&
	    _selectedToolMode == TOOLMODE_SELECT &&
	    evt.type == INPUT_MBUTTONDOWN)
	{
		// If user clicks on a selected bone...
		if (_selectedBones.count(_findHighlightedBone(evt.x, evt.y)) != 0)
		{
			// De-select it if Alt is pressed:
			if (evt.flags & HGEINP_ALT)
				_selectedBones.erase(_findHighlightedBone(evt.x, evt.y));
			
			_clickedOnSelection = true;
			return;
		}
		else
		// Change selection if user clicks on an unselected bone:
		if (_findHighlightedBone(evt.x, evt.y) != INVALID_ID)
		{
			if (!(evt.flags & (HGEINP_CTRL | HGEINP_SHIFT | HGEINP_ALT)))
				_selectedBones.clear();
			
			if (!(evt.flags & HGEINP_ALT)) {
				_selectedBones.insert(_findHighlightedBone(evt.x, evt.y));
				_clickedOnSelection = true;
			}
			else
				_clickedOnSelection = false;
			
			return;
		}
		else
			_clickedOnSelection = false;
	}
	
	if (_selectedTool == TOOL_EDIT_MUSCLES &&
	    _selectedToolMode == TOOLMODE_SELECT &&
	    evt.type == INPUT_MBUTTONDOWN)
	{
		// If user clicks on a selected muscle...
		if (_selectedMuscles.count(_findHighlightedMuscle(evt.x, evt.y)) != 0)
		{
			// De-select it if Alt is pressed:
			if (evt.flags & HGEINP_ALT)
				_selectedMuscles.erase(_findHighlightedMuscle(evt.x, evt.y));
			
			_clickedOnSelection = true;
			return;
		}
		else
		// Change selection if user clicks on an unselected muscle:
		if (_findHighlightedMuscle(evt.x, evt.y) != INVALID_ID)
		{
			if (!(evt.flags & (HGEINP_CTRL | HGEINP_SHIFT | HGEINP_ALT)))
				_selectedMuscles.clear();
			
			if (!(evt.flags & HGEINP_ALT)) {
				_selectedMuscles.insert(_findHighlightedMuscle(evt.x, evt.y));
				_clickedOnSelection = true;
			}
			else
				_clickedOnSelection = false;
			
			return;
		}
		else
			_clickedOnSelection = false;
	}
	
	// Select claws when user releases the mouse:
	if (_selectedTool == TOOL_EDIT_CLAWS &&
	    _selectedToolMode == TOOLMODE_SELECT &&
	    evt.type == INPUT_MBUTTONUP &&
	    !_rectToolBar.TestPoint(_mouseClickedX, _mouseClickedY) && 
	    !_clickedOnSelection)
	{
		// Reset the selection:
		if (!(evt.flags & (HGEINP_CTRL | HGEINP_SHIFT | HGEINP_ALT)))
			_selectedClaws.clear();
		
		hgeRect selectionRect(_mouseClickedX, _mouseClickedY, evt.x, evt.y);
		if (selectionRect.x1 > selectionRect.x2)
			std::swap(selectionRect.x1, selectionRect.x2);
		if (selectionRect.y1 > selectionRect.y2)
			std::swap(selectionRect.y1, selectionRect.y2);
		
		for (ClawItr itr = _creatureClaws.begin(); itr != _creatureClaws.end(); itr++)
			if (selectionRect.TestPoint(itr.position().x, itr.position().y)) {
				// If holding Alt, remove from selection:
				if (evt.flags & HGEINP_ALT)
					_selectedClaws.erase(itr.id());
				// Add to selection:
				else
					_selectedClaws.insert(itr.id());
			}
		
		return;
	}
	
	// Select bones when user releases the mouse:
	if (_selectedTool == TOOL_EDIT_BONES &&
	    _selectedToolMode == TOOLMODE_SELECT &&
	    evt.type == INPUT_MBUTTONUP &&
	    !_rectToolBar.TestPoint(_mouseClickedX, _mouseClickedY) && 
	    !_clickedOnSelection)
	{
		// Reset the selection:
		if (!(evt.flags & (HGEINP_CTRL | HGEINP_SHIFT | HGEINP_ALT)))
			_selectedBones.clear();
		
		hgeRect selectionRect(_mouseClickedX, _mouseClickedY, evt.x, evt.y);
		if (selectionRect.x1 > selectionRect.x2)
			std::swap(selectionRect.x1, selectionRect.x2);
		if (selectionRect.y1 > selectionRect.y2)
			std::swap(selectionRect.y1, selectionRect.y2);
		
		for (BoneItr itr = _creatureBones.begin(); itr != _creatureBones.end(); itr++)
		{
			const Vector2D & claw1 = _creatureClaws.find(itr.clawID1())->second;
			const Vector2D & claw2 = _creatureClaws.find(itr.clawID2())->second;
			
			if (selectionRect.TestPoint(claw1.x, claw1.y) &&
			    selectionRect.TestPoint(claw2.x, claw2.y))
			{
				// If holding Alt, remove from selection:
				if (evt.flags & HGEINP_ALT)
					_selectedBones.erase(itr.id());
				// Add to selection:
				else
					_selectedBones.insert(itr.id());
			}
		}
		
		return;
	}
	
	// Select muscles when user releases the mouse:
	if (_selectedTool == TOOL_EDIT_MUSCLES &&
	    _selectedToolMode == TOOLMODE_SELECT &&
	    evt.type == INPUT_MBUTTONUP &&
	    !_rectToolBar.TestPoint(_mouseClickedX, _mouseClickedY) && 
	    !_clickedOnSelection)
	{
		// Reset the selection:
		if (!(evt.flags & (HGEINP_CTRL | HGEINP_SHIFT | HGEINP_ALT)))
			_selectedMuscles.clear();
		
		hgeRect selectionRect(_mouseClickedX, _mouseClickedY, evt.x, evt.y);
		if (selectionRect.x1 > selectionRect.x2)
			std::swap(selectionRect.x1, selectionRect.x2);
		if (selectionRect.y1 > selectionRect.y2)
			std::swap(selectionRect.y1, selectionRect.y2);
		
		for (MuscleItr itr = _creatureMuscles.begin(); itr != _creatureMuscles.end(); itr++)
		{
			const Vector2D & claw1 = _creatureClaws.find(itr.clawID1())->second;
			const Vector2D & claw2 = _creatureClaws.find(itr.clawID2())->second;
			
			if (selectionRect.TestPoint(claw1.x, claw1.y) &&
			    selectionRect.TestPoint(claw2.x, claw2.y))
			{
				// If holding Alt, remove from selection:
				if (evt.flags & HGEINP_ALT)
					_selectedMuscles.erase(itr.id());
				// Add to selection:
				else
					_selectedMuscles.insert(itr.id());
			}
		}
		
		return;
	}
	
	// Create new claw:
	if (!_rectToolBar.TestPoint(evt.x, evt.y))
	{
		if (_selectedTool == TOOL_EDIT_CLAWS &&
		    _selectedToolMode == TOOLMODE_CREATE &&
		    (evt.type == INPUT_MBUTTONDOWN && evt.key == HGEK_LBUTTON))
		{
			int idClaw = _nextClawID();
			_creatureClaws[idClaw] = Vector2D(evt.x, evt.y);
			_selectedClaws.clear();
			_selectedClaws.insert(idClaw);
			
			// Notify listeners:
			{
				CreatureEditorClawEvent evt(this, idClaw);
				_subject.notify(&ICreatureEditorListener::onClawAdded, evt);
			}
			
			return;
		}
	}
	
	// Create a bone when user releases the mouse:
	if (_selectedTool == TOOL_EDIT_BONES &&
	    _selectedToolMode == TOOLMODE_CREATE &&
	    evt.type == INPUT_MBUTTONUP &&
	    !_rectToolBar.TestPoint(_mouseClickedX, _mouseClickedY) && 
	    !_clickedOnSelection)
	{
		int idClaw1 = _findHighlightedClaw(_mouseClickedX, _mouseClickedY);
		int idClaw2 = _findHighlightedClaw(evt.x, evt.y);
		
		if (idClaw1 != INVALID_ID && idClaw2 != INVALID_ID) {
			// Create the bone:
			int idNewBone = _nextBoneID();
			_creatureBones[idNewBone] = std::pair<int, int>(idClaw1, idClaw2);
			
			// And select it:
			_selectedBones.clear();
			_selectedBones.insert(idNewBone);
			
			// Notify listeners:
			{
				CreatureEditorBoneEvent evt(this, idNewBone);
				_subject.notify(&ICreatureEditorListener::onBoneAdded, evt);
			}
			
			return;
		}
	}
	
	// Create a muscle when user releases the mouse:
	if (_selectedTool == TOOL_EDIT_MUSCLES &&
	    _selectedToolMode == TOOLMODE_CREATE &&
	    evt.type == INPUT_MBUTTONUP &&
	    !_rectToolBar.TestPoint(_mouseClickedX, _mouseClickedY) && 
	    !_clickedOnSelection)
	{
		int idClaw1 = _findHighlightedClaw(_mouseClickedX, _mouseClickedY);
		int idClaw2 = _findHighlightedClaw(evt.x, evt.y);
		
		if (idClaw1 != INVALID_ID && idClaw2 != INVALID_ID) {
			// Create the muscle:
			int idNewMuscle = _nextMuscleID();
			_creatureMuscles[idNewMuscle] = std::pair<int, int>(idClaw1, idClaw2);
			
			// And select it:
			_selectedMuscles.clear();
			_selectedMuscles.insert(idNewMuscle);
			
			// Notify listeners:
			{
				CreatureEditorMuscleEvent evt(this, idNewMuscle);
				_subject.notify(&ICreatureEditorListener::onMuscleAdded, evt);
			}
			
			return;
		}
	}
	
	// Delete selected claws:
	if (_selectedTool == TOOL_EDIT_CLAWS &&
	    evt.type == INPUT_KEYDOWN &&
	    evt.key == HGEK_DELETE &&
	    !hge->Input_GetKeyState(HGEK_LBUTTON))
	{
		for (std::set<int>::iterator itrClawID = _selectedClaws.begin();
		     itrClawID != _selectedClaws.end();
		     itrClawID++)
		{
			_creatureClaws.erase(*itrClawID);
			
			// Erase bones connected to this claw:
			while (true) {
				bool keepGoing = false;
				for (BoneItr itrBone = _creatureBones.begin();
				     itrBone != _creatureBones.end();
				     itrBone++)
				{
					if (itrBone.clawID1() == *itrClawID || itrBone.clawID2() == *itrClawID) {
						_creatureBones.erase(itrBone);
						_selectedBones.erase(itrBone.id());
						keepGoing = true;
						
						// Notify listeners:
						{
							CreatureEditorBoneEvent evt(this, itrBone.id());
							_subject.notify(&ICreatureEditorListener::onBoneRemoved, evt);
						}
						
						break;
					}
				}
				if (!keepGoing)
					break;
			}
			
			// Erase muscles connected to this claw:
			while (true) {
				bool keepGoing = false;
				for (MuscleItr itrMuscle = _creatureMuscles.begin();
				     itrMuscle != _creatureMuscles.end();
				     itrMuscle++)
				{
					if (itrMuscle.clawID1() == *itrClawID || itrMuscle.clawID2() == *itrClawID) {
						_creatureMuscles.erase(itrMuscle);
						_selectedMuscles.erase(itrMuscle.id());
						keepGoing = true;
						
						// Notify listeners:
						{
							CreatureEditorMuscleEvent evt(this, itrMuscle.id());
							_subject.notify(&ICreatureEditorListener::onMuscleRemoved, evt);
						}
						
						break;
					}
				}
				if (!keepGoing)
					break;
			}
			
			// Notify listeners:
			{
				CreatureEditorClawEvent evt(this, *itrClawID);
				_subject.notify(&ICreatureEditorListener::onClawRemoved, evt);
			}
		}
		
		_selectedClaws.clear();
		
		return;
	}
	
	// Delete selected bones:
	if (_selectedTool == TOOL_EDIT_BONES &&
	    evt.type == INPUT_KEYDOWN &&
	    evt.key == HGEK_DELETE &&
	    !hge->Input_GetKeyState(HGEK_LBUTTON))
	{
		for (std::set<int>::iterator itrBoneID = _selectedBones.begin();
		     itrBoneID != _selectedBones.end();
		     itrBoneID++)
		{
			_creatureBones.erase(*itrBoneID);
			
			// Notify listeners:
			{
				CreatureEditorBoneEvent evt(this, *itrBoneID);
				_subject.notify(&ICreatureEditorListener::onBoneRemoved, evt);
			}
		}
		
		_selectedBones.clear();
		
		return;
	}
	
	// Delete selected muscles:
	if (_selectedTool == TOOL_EDIT_MUSCLES &&
	    evt.type == INPUT_KEYDOWN &&
	    evt.key == HGEK_DELETE &&
	    !hge->Input_GetKeyState(HGEK_LBUTTON))
	{
		for (std::set<int>::iterator itrMuscleID = _selectedMuscles.begin();
		     itrMuscleID != _selectedMuscles.end();
		     itrMuscleID++)
		{
			_creatureMuscles.erase(*itrMuscleID);
			
			// Notify listeners:
			{
				CreatureEditorMuscleEvent evt(this, *itrMuscleID);
				_subject.notify(&ICreatureEditorListener::onMuscleRemoved, evt);
			}
		}
		
		_selectedMuscles.clear();
		
		return;
	}
	
	// Test creature:
	if (_rectTestCreature.TestPoint(_mouseClickedX, _mouseClickedY) &&
	    _rectTestCreature.TestPoint(evt.x, evt.y) &&
	    evt.type == INPUT_MBUTTONUP &&
	    evt.key == HGEK_LBUTTON)
	{
		if (_creatureClaws.size() == 0 || _creatureMuscles.size() == 0)
			MessageBox(
				hge->System_GetState(HGE_HWND),
				"The creature must have claws and muscles!",
				"Error",
				MB_OK | MB_ICONERROR | MB_SYSTEMMODAL);
		else
			evt.source->pushState(new CreatureTestGameState(getCreatureDescr()));
		
		return;
	}
	
	// Generate animation:
	if (_rectGenerateAnimation.TestPoint(_mouseClickedX, _mouseClickedY) &&
	    _rectGenerateAnimation.TestPoint(evt.x, evt.y) &&
	    evt.type == INPUT_MBUTTONUP &&
	    evt.key == HGEK_LBUTTON)
	{
		if (_creatureClaws.size() == 0 || _creatureMuscles.size() == 0)
			MessageBox(
				hge->System_GetState(HGE_HWND),
				"The creature must have claws and muscles!",
				"Error",
				MB_OK | MB_ICONERROR | MB_SYSTEMMODAL);
		else
			evt.source->pushState(new EvolveAnimationGameState(getCreatureDescr()));
		
		return;
	}
	
}




void CreatureEditorGameState::render(const GameStateRenderEvent & evt) const
{
	AutoHGE hge;
	
	int screenW = hge->System_GetState(HGE_SCREENWIDTH);
	int screenH = hge->System_GetState(HGE_SCREENHEIGHT);
	
	float mouseX, mouseY;
	hge->Input_GetMousePos(&mouseX, &mouseY);
	
	bool isOnTop = (evt.source->getTop() == this);
	
	// Render the creature:
	{
		// Claws:
		{
			for (ClawConstItr itr = _creatureClaws.begin(); itr != _creatureClaws.end(); itr++)
				renderCircle(hge, itr.position(), CLAW_RADIUS, CLAW_COLOR);
			
			// Blink selected claws:
			if (isOnTop && _selectedTool == TOOL_EDIT_CLAWS) {
				for (std::set<int>::const_iterator itr = _selectedClaws.begin();
				     itr != _selectedClaws.end();
				     itr++)
				{
					unsigned char alpha =
						(unsigned char) (255 * (0.5 + sin(BLINK_FREQ * hge->Timer_GetTime())/2));
					
					renderCircle(
						hge,
						_creatureClaws.find(*itr)->second,
						2 * CLAW_RADIUS,
						SETA(CLAW_COLOR, alpha));
				}
			}
			
			// Highlight claw on mouse-over:
			bool highlightIt = false;
			if (_selectedTool == TOOL_EDIT_CLAWS && _selectedToolMode == TOOLMODE_SELECT)
				highlightIt = true;
			if (_selectedTool == TOOL_EDIT_BONES && _selectedToolMode == TOOLMODE_CREATE)
				highlightIt = true;
			if (_selectedTool == TOOL_EDIT_MUSCLES && _selectedToolMode == TOOLMODE_CREATE)
				highlightIt = true;
			
			if (isOnTop && highlightIt)
			{
				int idHighlightedClaw = _findHighlightedClaw(mouseX, mouseY);
				if (idHighlightedClaw != INVALID_ID) {
					Vector2D pos = _creatureClaws.find(idHighlightedClaw)->second;
					renderCircle(hge, pos, CLAW_RADIUS + 1, CLAW_COLOR);
					renderCircle(hge, pos, CLAW_RADIUS + 2, CLAW_COLOR);
				}
			}
		}
		
		// Bones:
		{
			for (BoneConstItr itr = _creatureBones.begin(); itr != _creatureBones.end(); itr++)
			{
				const Vector2D & claw1 = _creatureClaws.find(itr.clawID1())->second;
				const Vector2D & claw2 = _creatureClaws.find(itr.clawID2())->second;
				
				hge->Gfx_RenderLine(claw1.x, claw1.y, claw2.x, claw2.y, BONE_COLOR);
			}
			
			// Blink selected bones:
			if (isOnTop && _selectedTool == TOOL_EDIT_BONES) {
				for (std::set<int>::const_iterator itr = _selectedBones.begin();
				     itr != _selectedBones.end();
				     itr++)
				{
					unsigned char alpha =
						(unsigned char) (255 * (0.5 + sin(BLINK_FREQ * hge->Timer_GetTime())/2));
					
					const std::pair<int, int> & bone = _creatureBones.find(*itr)->second;
					
					const Vector2D & claw1 = _creatureClaws.find(bone.first)->second;
					const Vector2D & claw2 = _creatureClaws.find(bone.second)->second;
					
					Vector2D tangent = (claw2 - claw1);
					Vector2D normal = tangent.makeRotated(M_PI/2).unit() * CLAW_RADIUS;
					
					hge->Gfx_RenderLine(
						claw1.x + normal.x, claw1.y + normal.y, 
						claw2.x + normal.x, claw2.y + normal.y,
						SETA(BONE_COLOR, alpha));
					
					hge->Gfx_RenderLine(
						claw1.x - normal.x, claw1.y - normal.y, 
						claw2.x - normal.x, claw2.y - normal.y,
						SETA(BONE_COLOR, alpha));
				}
			}
			
			// Highlight bone on mouse-over:
			bool highlightIt = false;
			if (_selectedTool == TOOL_EDIT_BONES && _selectedToolMode == TOOLMODE_SELECT)
				highlightIt = true;
			
			if (isOnTop && highlightIt)
			{
				int idHighlightedBone = _findHighlightedBone(mouseX, mouseY);
				if (idHighlightedBone != INVALID_ID) {
					const std::pair<int, int> & bone = _creatureBones.find(idHighlightedBone)->second;
					
					const Vector2D & posClaw1 = _creatureClaws.find(bone.first)->second;
					const Vector2D & posClaw2 = _creatureClaws.find(bone.second)->second;
					
					#define DRAW_BONE(dx, dy) hge->Gfx_RenderLine(posClaw1.x + dx, posClaw1.y + dy, posClaw2.x + dx, posClaw2.y + dy, BONE_COLOR)
					
					DRAW_BONE(-1, -1);
					DRAW_BONE( 0, -1);
					DRAW_BONE(+1, -1);
					DRAW_BONE(+1,  0);
					DRAW_BONE(+1, +1);
					DRAW_BONE( 0, +1);
					DRAW_BONE(-1, +1);
					DRAW_BONE(-1,  0);
				}
			}
		}
		
		// Muscles:
		{
			for (MuscleConstItr itr = _creatureMuscles.begin(); itr != _creatureMuscles.end(); itr++)
			{
				const Vector2D & claw1 = _creatureClaws.find(itr.clawID1())->second;
				const Vector2D & claw2 = _creatureClaws.find(itr.clawID2())->second;
				
				hge->Gfx_RenderLine(claw1.x, claw1.y, claw2.x, claw2.y, MUSCLE_COLOR);
			}
			
			// Blink selected muscles:
			if (isOnTop && _selectedTool == TOOL_EDIT_MUSCLES) {
				for (std::set<int>::const_iterator itr = _selectedMuscles.begin();
				     itr != _selectedMuscles.end();
				     itr++)
				{
					unsigned char alpha =
						(unsigned char) (255 * (0.5 + sin(BLINK_FREQ * hge->Timer_GetTime())/2));
					
					const std::pair<int, int> & muscle = _creatureMuscles.find(*itr)->second;
					
					const Vector2D & claw1 = _creatureClaws.find(muscle.first)->second;
					const Vector2D & claw2 = _creatureClaws.find(muscle.second)->second;
					
					Vector2D tangent = (claw2 - claw1);
					Vector2D normal = tangent.makeRotated(M_PI/2).unit() * CLAW_RADIUS;
					
					hge->Gfx_RenderLine(
						claw1.x + normal.x, claw1.y + normal.y, 
						claw2.x + normal.x, claw2.y + normal.y,
						SETA(MUSCLE_COLOR, alpha));
					
					hge->Gfx_RenderLine(
						claw1.x - normal.x, claw1.y - normal.y, 
						claw2.x - normal.x, claw2.y - normal.y,
						SETA(MUSCLE_COLOR, alpha));
				}
			}
			
			// Highlight muscle on mouse-over:
			bool highlightIt = false;
			if (_selectedTool == TOOL_EDIT_MUSCLES && _selectedToolMode == TOOLMODE_SELECT)
				highlightIt = true;
			
			if (isOnTop && highlightIt)
			{
				int idHighlightedMuscle = _findHighlightedMuscle(mouseX, mouseY);
				if (idHighlightedMuscle != INVALID_ID) {
					const std::pair<int, int> & muscle = _creatureMuscles.find(idHighlightedMuscle)->second;
					
					const Vector2D & posClaw1 = _creatureClaws.find(muscle.first)->second;
					const Vector2D & posClaw2 = _creatureClaws.find(muscle.second)->second;
					
					#define DRAW_MUSCLE(dx, dy) hge->Gfx_RenderLine(posClaw1.x + dx, posClaw1.y + dy, posClaw2.x + dx, posClaw2.y + dy, MUSCLE_COLOR)
					
					DRAW_MUSCLE(-1, -1);
					DRAW_MUSCLE( 0, -1);
					DRAW_MUSCLE(+1, -1);
					DRAW_MUSCLE(+1,  0);
					DRAW_MUSCLE(+1, +1);
					DRAW_MUSCLE( 0, +1);
					DRAW_MUSCLE(-1, +1);
					DRAW_MUSCLE(-1,  0);
				}
			}
		}
	}
	
	// Render bone being created:
	if (_selectedTool == TOOL_EDIT_BONES &&
	    _selectedToolMode == TOOLMODE_CREATE &&
	    hge->Input_GetKeyState(HGEK_LBUTTON) &&
	    !_rectToolBar.TestPoint(_mouseClickedX, _mouseClickedY) && 
	    !_clickedOnSelection)
	{
		hge->Gfx_RenderLine(
			_mouseClickedX, _mouseClickedY,
			mouseX, mouseY,
			0x7F00FFFF);
	}
	
	// Render muscle being created:
	if (_selectedTool == TOOL_EDIT_MUSCLES &&
	    _selectedToolMode == TOOLMODE_CREATE &&
	    hge->Input_GetKeyState(HGEK_LBUTTON) &&
	    !_rectToolBar.TestPoint(_mouseClickedX, _mouseClickedY) && 
	    !_clickedOnSelection)
	{
		hge->Gfx_RenderLine(
			_mouseClickedX, _mouseClickedY,
			mouseX, mouseY,
			0x5FFFFF00);
	}
	
	// Render mouse-selection overlay:
	if (!_clickedOnSelection &&
	    !_rectToolBar.TestPoint(_mouseClickedX, _mouseClickedY))
	{
		if (_selectedToolMode == TOOLMODE_SELECT &&
		    hge->Input_GetKeyState(HGEK_LBUTTON))
		{
			hge->Gfx_RenderLine(_mouseClickedX, _mouseClickedY, mouseX,         _mouseClickedY);
			hge->Gfx_RenderLine(mouseX,         _mouseClickedY, mouseX,         mouseY);
			hge->Gfx_RenderLine(mouseX,         mouseY,         _mouseClickedX, mouseY);
			hge->Gfx_RenderLine(_mouseClickedX, mouseY,         _mouseClickedX, _mouseClickedY);
			
			hgeQuad q;
			memset(&q, 0, sizeof(hgeQuad));
			q.v[0].col = q.v[1].col = q.v[2].col = q.v[3].col = 0x1FFFFFFF;
			q.v[0].x = _mouseClickedX;
			q.v[1].x = mouseX;
			q.v[2].x = mouseX;
			q.v[3].x = _mouseClickedX;
			q.v[0].y = _mouseClickedY;
			q.v[1].y = _mouseClickedY;
			q.v[2].y = mouseY;
			q.v[3].y = mouseY;
			
			hge->Gfx_RenderQuad(&q);
		}
	}
	
	// Render control panel:
	{
		// Background:
		{
			hgeQuad q;
			q.blend = BLEND_DEFAULT;
			q.tex = NULL;
			q.v[0].col = 0x3F00FFFF;
			q.v[1].col = 0x3F00FFFF;
			q.v[2].col = 0x3F00FFFF;
			q.v[3].col = 0x3F00FFFF;
			q.v[0].tx = q.v[0].ty = 0;
			q.v[1].tx = q.v[1].ty = 0;
			q.v[2].tx = q.v[2].ty = 0;
			q.v[3].tx = q.v[3].ty = 0;
			q.v[0].x = _rectToolBar.x1;
			q.v[1].x = _rectToolBar.x2;
			q.v[2].x = _rectToolBar.x2;
			q.v[3].x = _rectToolBar.x1;
			q.v[0].y = _rectToolBar.y1;
			q.v[1].y = _rectToolBar.y1;
			q.v[2].y = _rectToolBar.y2;
			q.v[3].y = _rectToolBar.y2;
			q.v[0].z = q.v[1].z = q.v[2].z = q.v[3].z = 0;
			
			hge->Gfx_RenderQuad(&q);
		}
		
		// Tool buttons:
		{
			// Edit claws:
			if (_selectedTool == TOOL_EDIT_CLAWS)
				font->SetColor(0xFFFFFFFF);
			else
				font->SetColor(0xAFFFFFFF);
			font->Render(
				(_rectEditClaws.x1 + _rectEditClaws.x2)/2,
				(_rectEditClaws.y1 + _rectEditClaws.y2)/2 - font->GetHeight()/2,
				HGETEXT_CENTER,
				"Edit claws");
			DRAW_BTN(_rectEditClaws, (_selectedTool == TOOL_EDIT_CLAWS));
			
			// Edit bones:
			if (_selectedTool == TOOL_EDIT_BONES)
				font->SetColor(0xFFFFFFFF);
			else
				font->SetColor(0xAFFFFFFF);
			font->Render(
				(_rectEditBones.x1 + _rectEditBones.x2)/2,
				(_rectEditBones.y1 + _rectEditBones.y2)/2 - font->GetHeight()/2,
				HGETEXT_CENTER,
				"Edit bones");
			DRAW_BTN(_rectEditBones, (_selectedTool == TOOL_EDIT_BONES));
			
			// Edit muscles:
			if (_selectedTool == TOOL_EDIT_MUSCLES)
				font->SetColor(0xFFFFFFFF);
			else
				font->SetColor(0xAFFFFFFF);
			font->Render(
				(_rectEditMuscles.x1 + _rectEditMuscles.x2)/2,
				(_rectEditMuscles.y1 + _rectEditMuscles.y2)/2 - font->GetHeight()/2,
				HGETEXT_CENTER,
				"Edit muscles");
			DRAW_BTN(_rectEditMuscles, (_selectedTool == TOOL_EDIT_MUSCLES));
		}
		
		// Tool-mode buttons:
		{
			// Create tool-mode:
			if (_selectedToolMode == TOOLMODE_CREATE)
				_sprCreateToolMode->SetColor(0xFFFFFFFF);
			else
				_sprCreateToolMode->SetColor(0xAFFFFFFF);
			_sprCreateToolMode->Render(
				(_rectCreateMode.x1 + _rectCreateMode.x2)/2,
				(_rectCreateMode.y1 + _rectCreateMode.y2)/2);
			DRAW_BTN(_rectCreateMode, (_selectedToolMode == TOOLMODE_CREATE));
			
			// Select tool-mode:
			if (_selectedToolMode == TOOLMODE_SELECT)
				_sprSelectToolMode->SetColor(0xFFFFFFFF);
			else
				_sprSelectToolMode->SetColor(0xAFFFFFFF);
			_sprSelectToolMode->Render(
				(_rectSelectMode.x1 + _rectSelectMode.x2)/2,
				(_rectSelectMode.y1 + _rectSelectMode.y2)/2);
			DRAW_BTN(_rectSelectMode, (_selectedToolMode == TOOLMODE_SELECT));
		}
		
		// Control buttons:
		{
			font->SetColor(0xAFFFFFFF);
			
			font->Render(
				(_rectTestCreature.x1 + _rectTestCreature.x2)/2,
				(_rectTestCreature.y1 + _rectTestCreature.y2)/2 - font->GetHeight()/2,
				HGETEXT_CENTER,
				"Test creature");
			DRAW_BTN(
				_rectTestCreature,
				(isOnTop && 
				 _rectTestCreature.TestPoint(_mouseClickedX, _mouseClickedY) &&
				 _rectTestCreature.TestPoint(mouseX, mouseY) &&
				 hge->Input_GetKeyState(HGEK_LBUTTON)));
			
			font->Render(
				(_rectGenerateAnimation.x1 + _rectGenerateAnimation.x2)/2,
				(_rectGenerateAnimation.y1 + _rectGenerateAnimation.y2)/2 - font->GetHeight()/2,
				HGETEXT_CENTER,
				"Generate animation!");
			DRAW_BTN(
				_rectGenerateAnimation,
				(isOnTop && 
				 _rectGenerateAnimation.TestPoint(_mouseClickedX, _mouseClickedY) &&
				 _rectGenerateAnimation.TestPoint(mouseX, mouseY) &&
				 hge->Input_GetKeyState(HGEK_LBUTTON)));
		}
	}
}




CreatureDescriptor CreatureEditorGameState::getCreatureDescr() const
{
	CreatureDescriptor descr;
	
	for (ClawConstItr itr = _creatureClaws.begin(); itr != _creatureClaws.end(); itr++)
		descr.claws.push_back(
			CreatureDescriptor::Claw(
				itr.id(),
				itr.position()));
	
	for (BoneConstItr itr = _creatureBones.begin(); itr != _creatureBones.end(); itr++)
		descr.bones.push_back(
			CreatureDescriptor::Bone(
				itr.id(),
				itr.clawID1(),
				itr.clawID2()));
	
	for (MuscleConstItr itr = _creatureMuscles.begin(); itr != _creatureMuscles.end(); itr++)
		descr.muscles.push_back(
			CreatureDescriptor::Muscle(
				itr.id(),
				itr.clawID1(),
				itr.clawID2()));
	
	return descr;
}




int CreatureEditorGameState::_nextClawID() const
{
	if (_creatureClaws.empty())
		return 1;
	
	for (int id = 1; true; id++) {
		bool next = false;
		for (ClawConstItr itr = _creatureClaws.begin(); itr != _creatureClaws.end(); itr++)
			if (itr.id() == id) {
				next = true;
				break;
			}
		
		if (!next)
			return id;
	}
}




int CreatureEditorGameState::_nextBoneID() const
{
	if (_creatureBones.empty())
		return 1;
	
	for (int id = 1; true; id++) {
		bool next = false;
		for (BoneConstItr itr = _creatureBones.begin(); itr != _creatureBones.end(); itr++)
			if (itr.id() == id) {
				next = true;
				break;
			}
		
		if (!next)
			return id;
	}
}




int CreatureEditorGameState::_nextMuscleID() const
{
	if (_creatureMuscles.empty())
		return 1;
	
	for (int id = 1; true; id++) {
		bool next = false;
		for (MuscleConstItr itr = _creatureMuscles.begin(); itr != _creatureMuscles.end(); itr++)
			if (itr.id() == id) {
				next = true;
				break;
			}
		
		if (!next)
			return id;
	}
}




int CreatureEditorGameState::_findHighlightedClaw(float mouseX, float mouseY) const
{
	for (ClawConstRItr itr = _creatureClaws.rbegin(); itr != _creatureClaws.rend(); itr++)
		if (fabs(itr.position().x - mouseX) < HIT_MARGIN &&
		    fabs(itr.position().y - mouseY) < HIT_MARGIN)
			return itr.id();
		
	return INVALID_ID;
}




int CreatureEditorGameState::_findHighlightedBone(float mouseX, float mouseY) const
{
	Vector2D mouse(mouseX, mouseY);
	
	for (BoneConstRItr itr = _creatureBones.rbegin(); itr != _creatureBones.rend(); itr++)
	{
		const Vector2D & claw1 = _creatureClaws.find(itr.clawID1())->second;
		const Vector2D & claw2 = _creatureClaws.find(itr.clawID2())->second;
		
		Vector2D tangent = (claw1 - claw2);
		Vector2D normal = tangent.makeRotated(M_PI/2);
		
		float distPerpendicular = (mouse - claw1).project(normal).length();
		
		float dist1 = (mouse - claw1).project(tangent).length();
		float dist2 = (mouse - claw2).project(tangent).length();
		
		if (distPerpendicular > HIT_MARGIN)
			continue;
		
		if (max(dist1, dist2) > tangent.length())
			continue;
		
		return itr.id();
	}
		
	return INVALID_ID;
}




int CreatureEditorGameState::_findHighlightedMuscle(float mouseX, float mouseY) const
{
	Vector2D mouse(mouseX, mouseY);
	
	for (MuscleConstRItr itr = _creatureMuscles.rbegin(); itr != _creatureMuscles.rend(); itr++)
	{
		const Vector2D & claw1 = _creatureClaws.find(itr.clawID1())->second;
		const Vector2D & claw2 = _creatureClaws.find(itr.clawID2())->second;
		
		Vector2D tangent = (claw1 - claw2);
		Vector2D normal = tangent.makeRotated(M_PI/2);
		
		float distPerpendicular = (mouse - claw1).project(normal).length();
		
		float dist1 = (mouse - claw1).project(tangent).length();
		float dist2 = (mouse - claw2).project(tangent).length();
		
		if (distPerpendicular > HIT_MARGIN)
			continue;
		
		if (max(dist1, dist2) > tangent.length())
			continue;
		
		return itr.id();
	}
		
	return INVALID_ID;
}

