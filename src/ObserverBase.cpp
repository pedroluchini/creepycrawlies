#include <ObserverBase.hpp>




void ObserverBase::_addSubject(SubjectBase * o)
{
	std::vector<SubjectBase *>::iterator itr = std::find(_subjects.begin(), _subjects.end(), o);
	if (itr == _subjects.end())
		_subjects.push_back(o);
}




void ObserverBase::_removeSubject(SubjectBase * o)
{
	std::vector<SubjectBase *>::iterator itr = std::find(_subjects.begin(), _subjects.end(), o);
	if (itr != _subjects.end())
		_subjects.erase(itr);
}




ObserverBase::~ObserverBase()
{
	for (size_t i = 0; i < _subjects.size(); i++) {
		_subjects.at(i)->_removeObserver(this);
	}
}
