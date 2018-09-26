#ifndef AUTOHGE_HPP
#define AUTOHGE_HPP

#include <hge.h>




class AutoHGE
{
	public:
		AutoHGE() { _hge = hgeCreate(HGE_VERSION); }
		~AutoHGE() { _hge->Release(); }
		
		inline operator HGE *() { return _hge; }
		inline HGE * operator->() { return _hge; }
		
	private:
		HGE * _hge;
};

#endif



