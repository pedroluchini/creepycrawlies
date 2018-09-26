#ifndef OBSERVERBASE_HPP
#define OBSERVERBASE_HPP

#include <algorithm>
#include <vector>

class ObserverBase;
class SubjectBase;
template <typename TObserver> class Subject;




class ObserverBase
{
	public:
		virtual ~ObserverBase();
	private:
		friend class SubjectBase;
		friend class Subject;
		std::vector<SubjectBase *> _subjects;
		void _addSubject(SubjectBase *);
		void _removeSubject(SubjectBase *);
};




class SubjectBase
{
	private:
		friend class ObserverBase;
		virtual void _removeObserver(ObserverBase *) = 0;
};




template <typename TObserver>
class Subject : public SubjectBase
{
	public:
		template <typename T0>
		void notify(void (TObserver::*method) (T0), T0 arg0);
		
		template <typename T0>
		void notify(void (TObserver::*method) (const T0 &), const T0 & arg0);
		
		typedef TObserver TObserver;
		virtual ~Subject();
		void addObserver(TObserver *);
		void removeObserver(TObserver *);
		inline std::vector<TObserver *> getObservers() const;
	private:
		friend class ObserverBase;
		std::vector<TObserver *> _observers;
		void _removeObserver(ObserverBase *);
};




template <typename TObserver>
void Subject<TObserver>::_removeObserver(ObserverBase * o)
{
	std::vector<TObserver *>::iterator itr = std::find(_observers.begin(), _observers.end(), dynamic_cast<TObserver *>(o));
	if (itr != _observers.end())
		_observers.erase(itr);
}




template <typename TObserver>
void Subject<TObserver>::addObserver(TObserver * o)
{
	std::vector<TObserver *>::iterator itr = std::find(_observers.begin(), _observers.end(), o);
	if (itr == _observers.end()) {
		_observers.push_back(o);
		o->_addSubject(this);
	}
}




template <typename TObserver>
void Subject<TObserver>::removeObserver(TObserver * o)
{
	std::vector<TObserver *>::iterator itr = std::find(_observers.begin(), _observers.end(), o);
	if (itr != _observers.end()) {
		_observers.erase(itr);
		o->_removeSubject(this);
	}
}




template <typename TObserver>
std::vector<TObserver *> Subject<TObserver>::getObservers() const
{
	return _observers;
}




template <typename TObserver>
Subject<TObserver>::~Subject()
{
	for (size_t i = 0; i < _observers.size(); i++) {
		_observers.at(i)->_removeSubject(this);
	}
}




template <typename TObserver>
template <typename T0>
void Subject<TObserver>::notify(void (TObserver::*method) (T0), T0 arg0)
{
	if (_observers.empty())
		return;
	
	std::vector<TObserver *> observers = _observers;
	for (size_t i = 0; i < observers.size(); i++)
		(observers.at(i)->*method)(arg0);
}




template <typename TObserver>
template <typename T0>
void Subject<TObserver>::notify(void (TObserver::*method) (const T0 &), const T0 & arg0)
{
	if (_observers.empty())
		return;
	
	std::vector<TObserver *> observers = _observers;
	for (size_t i = 0; i < observers.size(); i++)
		(observers.at(i)->*method)(arg0);
}

#endif