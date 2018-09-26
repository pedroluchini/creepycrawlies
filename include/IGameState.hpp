#ifndef IGAMESTATE_HPP
#define IGAMESTATE_HPP

#include <hge.h>

class GameStateStack;

/**
 * Packs information used by a game state during its update event.
 */
class GameStateUpdateEvent
{
	public:
		GameStateUpdateEvent(GameStateStack * a_source, float a_dt)
		: source(a_source), dt(a_dt) {}
		
		/**
		 * The time elapsed since the last update event.
		 */
		float dt;
		
		/**
		 * The object originating this event.
		 */
		GameStateStack * source;
};

/**
 * Packs information used by a game state during its render event.
 */
class GameStateRenderEvent
{
	public:
		GameStateRenderEvent(const GameStateStack * a_source)
		: source(a_source) {}
		
		/**
		 * The object originating this event.
		 */
		const GameStateStack * source;
};

/**
 * Packs information used by a game state during the input event.
 */
class GameStateInputEvent : public hgeInputEvent
{
	public:
		GameStateInputEvent(GameStateStack * a_source, const hgeInputEvent & evt)
		: source(a_source), hgeInputEvent(evt) {}
		
		/**
		 * The object originating this event.
		 */
		GameStateStack * source;
};

/**
 * A game state must implement this interface in order to be added to a game
 * state stack.
 */
class IGameState
{
	public:
		/**
		 * Update the game state.
		 */
		virtual void update(const GameStateUpdateEvent & evt) = 0;
		
		/**
		 * Display the game state. The game state cannot be changed during this
		 * event.
		 */
		virtual void render(const GameStateRenderEvent & evt) const = 0;
		
		/**
		 * Receive an input event.
		 */
		virtual void processInputEvent(const GameStateInputEvent & evt) = 0;
		
		virtual ~IGameState() {}
};

#endif
