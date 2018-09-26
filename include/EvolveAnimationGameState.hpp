#ifndef EVOLVEANIMATIONGAMESTATE_HPP
#define EVOLVEANIMATIONGAMESTATE_HPP

#include <Creature.hpp>
#include <ga/ga.h>
#include <hgerect.h>
#include <IGameState.hpp>

class EvolveAnimationGameState : public IGameState
{
	public:
		
		EvolveAnimationGameState(const CreatureDescriptor & creatureDescr);
		~EvolveAnimationGameState();
		
		void update(const GameStateUpdateEvent & evt);
		void render(const GameStateRenderEvent & evt) const;
		void processInputEvent(const GameStateInputEvent & evt);
		
	private:
		EvolveAnimationGameState(const EvolveAnimationGameState &);
		EvolveAnimationGameState & operator=(const EvolveAnimationGameState &);
		
		CreatureDescriptor _creatureDescr;
		Creature _creature;
		GAListGenome<Creature::OPCODE> * _genome;
		GASigmaTruncationScaling * _scale;
		GASteadyStateGA * _ga;
		
		float _accDT;
		hgeRect _rectViewResult;
		float _mouseClickedX, _mouseClickedY;
};

#endif
