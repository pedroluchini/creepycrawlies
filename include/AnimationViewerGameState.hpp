#ifndef ANIMATIONVIEWERGAMESTATE_HPP
#define ANIMATIONVIEWERGAMESTATE_HPP

#include <Creature.hpp>
#include <hgerect.h>
#include <IGameState.hpp>

class AnimationViewerGameState : public IGameState
{
	public:
		
		AnimationViewerGameState(const CreatureDescriptor & creatureDescr,
		                         const Creature::OPCODE * opCodes,
		                         int nOpcodes);
		
		void update(const GameStateUpdateEvent & evt);
		void render(const GameStateRenderEvent & evt) const;
		void processInputEvent(const GameStateInputEvent & evt);
		
	private:
		Creature _creature;
		float _accDT;
		
		float _mouseClickedX, _mouseClickedY;
		hgeRect _rectContinue;
		hgeRect _rectStore;
		hgeRect _rectDiscard;
		hgeRect _rectReplay;
};

#endif
