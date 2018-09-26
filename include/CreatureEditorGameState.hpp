#ifndef CREATUREEDITORGAMESTATE_HPP
#define CREATUREEDITORGAMESTATE_HPP

#include <Creature.hpp>
#include <hgeFont.h>
#include <hgerect.h>
#include <hgeSprite.h>
#include <IGameState.hpp>
#include <map>
#include <ObserverBase.hpp>
#include <set>
#include <Vector2D.hpp>

class CreatureEditorGameState;



class CreatureEditorClawEvent
{
	public:
		inline CreatureEditorClawEvent(CreatureEditorGameState * a_src, int a_idClaw)
		{
			src = a_src;
			idClaw = a_idClaw;
		}
		
		CreatureEditorGameState * src;
		int idClaw;
};




class CreatureEditorBoneEvent
{
	public:
		inline CreatureEditorBoneEvent(CreatureEditorGameState * a_src, int a_idBone)
		{
			src = a_src;
			idBone = a_idBone;
		}
		
		CreatureEditorGameState * src;
		int idBone;
};




class CreatureEditorMuscleEvent
{
	public:
		inline CreatureEditorMuscleEvent(CreatureEditorGameState * a_src, int a_idMuscle)
		{
			src = a_src;
			idMuscle = a_idMuscle;
		}
		
		CreatureEditorGameState * src;
		int idMuscle;
};



class ICreatureEditorListener : public ObserverBase
{
	public:
		virtual void onClawAdded    (const CreatureEditorClawEvent & evt) = 0;
		virtual void onClawRemoved  (const CreatureEditorClawEvent & evt) = 0;
		virtual void onClawMoved    (const CreatureEditorClawEvent & evt) = 0;
		virtual void onBoneAdded    (const CreatureEditorBoneEvent & evt) = 0;
		virtual void onBoneRemoved  (const CreatureEditorBoneEvent & evt) = 0;
		virtual void onMuscleAdded  (const CreatureEditorMuscleEvent & evt) = 0;
		virtual void onMuscleRemoved(const CreatureEditorMuscleEvent & evt) = 0;
};




class CreatureEditorGameState : public IGameState
{
	public:
		
		CreatureEditorGameState();
		
		~CreatureEditorGameState();
		
		inline void addListener(ICreatureEditorListener * l) { _subject.addObserver(l); }
		inline void removeListener(ICreatureEditorListener * l) { _subject.removeObserver(l); }
		
		bool getClaw(int id, float * xPos, float * yPos) const;
		bool getBone(int id, int * idClaw1, int * idClaw2) const;
		bool getMuscle(int id, int * idClaw1, int * idClaw2) const;
		
		void update(const GameStateUpdateEvent & evt);
		void render(const GameStateRenderEvent & evt) const;
		void processInputEvent(const GameStateInputEvent & evt);
		
		CreatureDescriptor getCreatureDescr() const;
		
	private:
		enum Tool
		{
			TOOL_EDIT_CLAWS,
			TOOL_EDIT_BONES,
			TOOL_EDIT_MUSCLES
		};
		
		enum ToolMode
		{
			TOOLMODE_CREATE,
			TOOLMODE_SELECT
		};
		
		Subject<ICreatureEditorListener> _subject;
		
		hgeSprite * _sprCreateToolMode;
		hgeSprite * _sprSelectToolMode;
		
		hgeRect _rectToolBar;
		
		float _prevMouseX, _prevMouseY;
		float _mouseClickedX, _mouseClickedY;
		bool _clickedOnSelection;
		
		Tool      _selectedTool;
		ToolMode  _selectedToolMode;
		hgeRect   _rectEditClaws, _rectEditBones, _rectEditMuscles;
		hgeRect   _rectCreateMode, _rectSelectMode;
		hgeRect   _rectTestCreature, _rectGenerateAnimation;
		
		std::map<int, Vector2D> _creatureClaws;
		std::map< int, std::pair<int, int> > _creatureBones;
		std::map< int, std::pair<int, int> > _creatureMuscles;
		
		std::set<int> _selectedClaws;
		std::set<int> _selectedBones;
		std::set<int> _selectedMuscles;
		
		struct ClawItr : public std::map<int, Vector2D>::iterator
		{
			public:
				ClawItr(std::map<int, Vector2D>::iterator itr)
				: std::map<int, Vector2D>::iterator(itr) {}
				
				int id() const { return __super::operator->()->first; }
				Vector2D & position() { return __super::operator->()->second; }
		};
		
		struct ClawConstItr : public std::map<int, Vector2D>::const_iterator
		{
			public:
				ClawConstItr(std::map<int, Vector2D>::const_iterator itr)
				: std::map<int, Vector2D>::const_iterator(itr) {}
				
				int id() const { return __super::operator->()->first; }
				const Vector2D & position() const { return __super::operator->()->second; }
		};
		
		struct ClawConstRItr : public std::map<int, Vector2D>::const_reverse_iterator
		{
			public:
				ClawConstRItr(std::map<int, Vector2D>::const_reverse_iterator itr)
				: std::map<int, Vector2D>::const_reverse_iterator(itr) {}
				
				int id() const { return __super::operator->()->first; }
				const Vector2D & position() const { return __super::operator->()->second; }
		};
		
		struct BoneItr : public std::map< int, std::pair<int, int> >::iterator
		{
			public:
				BoneItr(std::map< int, std::pair<int, int> >::iterator itr)
				: std::map< int, std::pair<int, int> >::iterator(itr) {}
				
				int id() const { return __super::operator->()->first; }
				int clawID1() const { return __super::operator->()->second.first; }
				int clawID2() const { return __super::operator->()->second.second; }
		};
		
		struct BoneConstItr : public std::map< int, std::pair<int, int> >::const_iterator
		{
			public:
				BoneConstItr(std::map< int, std::pair<int, int> >::const_iterator itr)
				: std::map< int, std::pair<int, int> >::const_iterator(itr) {}
				
				int id() const { return __super::operator->()->first; }
				int clawID1() const { return __super::operator->()->second.first; }
				int clawID2() const { return __super::operator->()->second.second; }
		};
		
		struct BoneConstRItr : public std::map< int, std::pair<int, int> >::const_reverse_iterator
		{
			public:
				BoneConstRItr(std::map< int, std::pair<int, int> >::const_reverse_iterator itr)
				: std::map< int, std::pair<int, int> >::const_reverse_iterator(itr) {}
				
				int id() const { return __super::operator->()->first; }
				int clawID1() const { return __super::operator->()->second.first; }
				int clawID2() const { return __super::operator->()->second.second; }
		};
		
		typedef BoneItr MuscleItr;
		typedef BoneConstItr MuscleConstItr;
		typedef BoneConstRItr MuscleConstRItr;
		
		int _nextClawID() const;
		int _nextBoneID() const;
		int _nextMuscleID() const;
		int _findHighlightedClaw(float mouseX, float mouseY) const;
		int _findHighlightedBone(float mouseX, float mouseY) const;
		int _findHighlightedMuscle(float mouseX, float mouseY) const;
};

#endif
