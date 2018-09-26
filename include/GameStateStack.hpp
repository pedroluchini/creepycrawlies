#ifndef GAMESTATESTACK_HPP
#define GAMESTATESTACK_HPP

#include <hge.h>
#include <ObserverBase.hpp>
#include <vector>

class IGameState;

/**
 * A game state stack keeps track of game states, updating and rendering them
 * as needed.
 */
class GameStateStack
{
	public:
		
		/**
		 * Packs information relative to a game-state-pushed event.
		 */
		class PushedEvent
		{
			public:
				PushedEvent(GameStateStack * a_source, IGameState * a_state)
				: source(a_source), state(a_state) {}
				
				/**
				 * The object originating the event.
				 */
				GameStateStack * source;
				
				/**
				 * The state that was pushed on the stack.
				 */
				IGameState * state;
		};
		
		/**
		 * Packs information relative to a game-state-popped event.
		 */
		class PoppedEvent
		{
			public:
				PoppedEvent(GameStateStack * a_source, IGameState * a_state)
				: source(a_source), state(a_state) {}
				
				/**
				 * The object originating the event.
				 */
				GameStateStack * source;
				
				/**
				 * The state that was popped on the stack.
				 */
				IGameState * state;
		};
		
		/**
		 * An object must implement this interface in order to be notified of
		 * changes to the game state stack.
		 */
		class IListener : public ObserverBase
		{
			public:
				virtual void gameStatePushed(const PushedEvent & evt) = 0;
				virtual void gameStatePopped(const PoppedEvent & evt) = 0;
		};
		
		/**
		 * Empty constructor.
		 */
		inline GameStateStack() {}
		
		/**
		 * Adds a listener that will be notified of changes to the stack.
		 */
		inline void addListener(IListener * l)
		{
			_subject.addObserver(l);
		}
		
		/**
		 * Removes a listener so that it will no longer be notified of changes
		 * to the stack.
		 */
		inline void removeListener(IListener * l)
		{
			_subject.removeObserver(l);
		}
		
		/**
		 * Adds a game state to the top of the stack. The GameStateStack takes
		 * ownership of the pointer.
		 */
		void pushState(IGameState * state);
		
		/**
		 * Removes the game state at the top of the stack. Deletes the corresponding
		 * IGameState object.
		 * 
		 * If the stack is empty, the method does nothing.
		 */
		void popState();
		
		/**
		 * Use this to access and modify the state at the top of the stack. 
		 * Returns NULL if the stack is empty.
		 */
		inline IGameState * getTop()
		{
			if (_stack.empty())
				return NULL;
			else
				return _stack.back();
		}
		
		/**
		 * Use this to access the state at the top of the stack. Returns NULL
		 * if the stack is empty.
		 */
		const IGameState * getTop() const
		{
			if (_stack.empty())
				return NULL;
			else
				return _stack.back();
		}
		
		/**
		 * Updates the state at the top of the stack.
		 */
		void update(float dt);
		
		/**
		 * Forwards the event to the state at the top of the stack.
		 */
		void dispatchEvent(const hgeInputEvent & evt);
		
		/**
		 * Renders all states in the stack, from bottom to top.
		 */
		void render() const;
		
		/**
		 * Deletes all IGameStates still present in the stack.
		 */
		virtual ~GameStateStack();
		
	private:
		GameStateStack(const GameStateStack &);
		GameStateStack & operator=(const GameStateStack &);
		
		std::vector<IGameState *> _stack;
		std::vector<IGameState *> _statesToDelete;
		Subject<IListener> _subject;
};

#endif
