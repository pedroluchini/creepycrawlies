#ifndef PHYSICSSYSTEM_HPP
#define PHYSICSSYSTEM_HPP

#include <vector>
#include <Vector2D.hpp>




class PhysicsSystem
{
	public:
		
		class Particle
		{
			public:
				Particle(const Vector2D & position, float dampening, bool gripping = false) : _gripping(gripping), _dampening(dampening), _currPos(position), _prevPos(position), _tmpForce(0, 0), _isInContact(false) {}
				inline const Vector2D & getCurrentPosition() const { return _currPos; }
				inline float getCurrentX() const { return _currPos.x; }
				inline float getCurrentY() const { return _currPos.y; }
				inline bool isGripping() const { return _gripping; }
				inline void setGripping(bool gripping) { _gripping = gripping; }
				inline void resetPosition(const Vector2D & p)
				{
					_currPos = _prevPos = p;
					_tmpForce.x = _tmpForce.y = 0;
					_isInContact = false;
					_gripping = false;
				}
			private:
				friend class PhysicsSystem;
				Vector2D _currPos, _prevPos;
				Vector2D _tmpForce;
				bool _isInContact;
				Vector2D _contactPos;
				bool _gripping;
				float _dampening;
		};
		
		class Spring
		{
			public:
				Spring(Particle * p1, Particle * p2, float strength, float length) : _p1(p1), _p2(p2), _length(length), _strength(strength) {}
				Spring(Particle * p1, Particle * p2, float strength) : _p1(p1), _p2(p2), _length((p1->getCurrentPosition() - p2->getCurrentPosition()).length()), _strength(strength) {}
				inline float getLength() const { return _length; }
				inline float getStrength() const { return _strength; }
				inline void setLength(float length) { _length = length; }
				inline void setStrength(float strength) { _strength = strength; }
				inline Particle * getP1() { return _p1; }
				inline Particle * getP2() { return _p2; }
				inline const Particle * getP1() const { return _p1; }
				inline const Particle * getP2() const { return _p2; }
			private:
				friend class PhysicsSystem;
				Particle * _p1, * _p2;
				float _length, _strength;
		};
		
		class RigidBar
		{
			public:
				RigidBar(Particle * p1, Particle * p2, float length) : _p1(p1), _p2(p2), _length(length) {}
				RigidBar(Particle * p1, Particle * p2) : _p1(p1), _p2(p2), _length((p1->getCurrentPosition() - p2->getCurrentPosition()).length()) {}
				inline float getLength() const { return _length; }
				inline Particle * getP1() { return _p1; }
				inline Particle * getP2() { return _p2; }
				inline const Particle * getP1() const { return _p1; }
				inline const Particle * getP2() const { return _p2; }
			private:
				friend class PhysicsSystem;
				Particle * _p1, * _p2;
				float _length;
		};
		
		class Joint
		{
			public:
				Joint(Particle * pivot, Particle * p1, Particle * p2, float minAngle, float maxAngle) : _pivot(pivot), _p1(p1), _p2(p2), _minAngle(minAngle), _maxAngle(maxAngle) {}
				inline float getMinAngle() const { return _minAngle; }
				inline float getMaxAngle() const { return _maxAngle; }
				inline Particle * getPivot() { return _pivot; }
				inline Particle * getP1() { return _p1; }
				inline Particle * getP2() { return _p2; }
				inline const Particle * getP1() const { return _p1; }
				inline const Particle * getP2() const { return _p2; }
			private:
				friend class PhysicsSystem;
				Particle * _pivot, * _p1, * _p2;
				float _minAngle, _maxAngle;
		};
		
		PhysicsSystem(float yFloor, float yGravity) : _yFloor(yFloor), _yGravity(yGravity) {}
		
		void addParticle(Particle * p);
		void addSpring(Spring * s);
		void addRigidBar(RigidBar * rb);
		void addJoint(Joint * j);
		
		void update(float dt);
		
		inline float getYFloor() const { return _yFloor; }
		
		inline int getNumParticles() const { return (int) _particles.size(); }
		inline Particle * getParticle(int i) { return _particles.at(i); }
		inline const Particle * getParticle(int i) const { return _particles.at(i); }
		
		inline int getNumRigidBars() const { return (int) _rigidBars.size(); }
		inline RigidBar * getRigidBar(int i) { return _rigidBars.at(i); }
		inline const RigidBar * getRigidBar(int i) const { return _rigidBars.at(i); }
		
		inline int getNumSprings() const { return (int) _springs.size(); }
		inline Spring * getSpring(int i) { return _springs.at(i); }
		inline const Spring * getSpring(int i) const { return _springs.at(i); }
		
		inline int getNumJoints() const { return (int) _joints.size(); }
		inline Joint * getJoint(int i) { return _joints.at(i); }
		inline const Joint * getJoint(int i) const { return _joints.at(i); }
		
	private:
		float _yFloor;
		float _yGravity;
		std::vector<Particle *> _particles;
		std::vector<Spring *> _springs;
		std::vector<RigidBar *> _rigidBars;
		std::vector<Joint *> _joints;
};

#endif
