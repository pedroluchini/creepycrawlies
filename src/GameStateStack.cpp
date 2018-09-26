#include <GameStateStack.hpp>

#include <AutoHGE.hpp>
#include <IGameState.hpp>




void GameStateStack::pushState(IGameState * state)
{
	_stack.push_back(state);
	
	// Notify listeners:
	_subject.notify(&IListener::gameStatePushed, PushedEvent(this, state));
}




void GameStateStack::popState()
{
	if (_stack.empty())
		return;
	
	IGameState * state = _stack.back();
	_stack.pop_back();
	
	// Notify listeners:
	_subject.notify(&IListener::gameStatePopped, PoppedEvent(this, state));
	
	// Delete the object later:
	_statesToDelete.push_back(state);
}




void GameStateStack::update(float dt)
{
	for (size_t i = 0; i < _statesToDelete.size(); i++)
		delete _statesToDelete.at(i);
	_statesToDelete.clear();
	
	if (_stack.empty())
		return;
	
	_stack.back()->update(
		GameStateUpdateEvent(this, dt));
}




void GameStateStack::dispatchEvent(const hgeInputEvent & evt)
{
	if (_stack.empty())
		return;
	
	_stack.back()->processInputEvent(GameStateInputEvent(this, evt));
}




void GameStateStack::render() const
{
	if (_stack.empty())
		return;
	
	// Render the top-most state:
	GameStateRenderEvent evt(this);
	_stack.back()->render(evt);
}




GameStateStack::~GameStateStack()
{
	// Delete all states still held by this stack:
	for (size_t i = 0; i < _stack.size(); i++)
		delete _stack.at(i);
	for (size_t i = 0; i < _statesToDelete.size(); i++)
		delete _statesToDelete.at(i);
}



