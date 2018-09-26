#ifndef CREATURETESTGAMESTATE_HPP
#define CREATURETESTGAMESTATE_HPP

#include <Creature.hpp>
#include <hgerect.h>
#include <IGameState.hpp>

class CreatureTestGameState : public IGameState
{
	public:
		
		CreatureTestGameState(const CreatureDescriptor & creature);
		
		void update(const GameStateUpdateEvent & evt);
		void render(const GameStateRenderEvent & evt) const;
		void processInputEvent(const GameStateInputEvent & evt);
		
	private:
		Creature _creature;
		float _accDT;
		hgeRect _rectDone;
		float _mouseClickedX, _mouseClickedY;
};

#endif
