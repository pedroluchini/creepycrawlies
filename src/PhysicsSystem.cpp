#include <PhysicsSystem.hpp>

#include <algorithm>




template <typename T>
T max(T x, T y)
{
	if (x < y)
		return y;
	else
		return x;
}




template <typename T>
T min(T x, T y)
{
	if (x > y)
		return y;
	else
		return x;
}




void PhysicsSystem::addParticle(Particle * p)
{
	std::vector<Particle *>::iterator itr = std::find(_particles.begin(), _particles.end(), p);
	if (itr == _particles.end()) {
		_particles.push_back(p);
		
		if (p->_currPos.y > _yFloor) {
			float dy = _yFloor - p->_currPos.y;
			p->_currPos.y += dy;
			p->_prevPos.y += dy;
		}
	}
}




void PhysicsSystem::addSpring(Spring * s)
{
	std::vector<Spring *>::iterator itr = std::find(_springs.begin(), _springs.end(), s);
	if (itr == _springs.end())
		_springs.push_back(s);
}




void PhysicsSystem::addRigidBar(RigidBar * rb)
{
	std::vector<RigidBar *>::iterator itr = std::find(_rigidBars.begin(), _rigidBars.end(), rb);
	if (itr == _rigidBars.end())
		_rigidBars.push_back(rb);
}




void PhysicsSystem::addJoint(Joint * j)
{
	std::vector<Joint *>::iterator itr = std::find(_joints.begin(), _joints.end(), j);
	if (itr == _joints.end())
		_joints.push_back(j);
}




void PhysicsSystem::update(float dt)
{
	// Reset all forces:
	for (size_t i = 0; i < _particles.size(); i++) {
		_particles.at(i)->_tmpForce.x = 0;
		_particles.at(i)->_tmpForce.y = 0;
	}
	
	// Accumulate gravity:
	for (size_t i = 0; i < _particles.size(); i++) {
		_particles.at(i)->_tmpForce.y += _yGravity;
	}
	
	// Accumulate spring forces:
	for (size_t i = 0; i < _springs.size(); i++) {
		Spring * spr = _springs.at(i);
		
		Vector2D vP1ToP2(spr->_p2->_currPos.x - spr->_p1->_currPos.x,
		                 spr->_p2->_currPos.y - spr->_p1->_currPos.y);
		float dist = vP1ToP2.length();
		vP1ToP2 /= dist;
		
		float attractionForce = (dist - spr->_length) * spr->_strength;
		
		spr->_p1->_tmpForce.x += vP1ToP2.x * attractionForce;
		spr->_p1->_tmpForce.y += vP1ToP2.y * attractionForce;
		
		spr->_p2->_tmpForce.x -= vP1ToP2.x * attractionForce;
		spr->_p2->_tmpForce.y -= vP1ToP2.y * attractionForce;
	}
	
	// Update particle positions:
	for (size_t i = 0; i < _particles.size(); i++) {
		Particle * part = _particles.at(i);
		
		Vector2D newPos((2 - part->_dampening)*part->_currPos.x - (1 - part->_dampening) * part->_prevPos.x + dt*dt * part->_tmpForce.x,
		                (2 - part->_dampening)*part->_currPos.y - (1 - part->_dampening) * part->_prevPos.y + dt*dt * part->_tmpForce.y);
		
		part->_prevPos = part->_currPos;
		part->_currPos = newPos;
		
		if (part->_currPos.y >= _yFloor - 1 && !part->_isInContact) {
			part->_isInContact = true;
			part->_contactPos = part->_currPos;
			part->_contactPos.y = _yFloor;
		}
		else
		if (part->_currPos.y >= _yFloor - 1 && part->_isInContact && !part->_gripping) {
			part->_contactPos = part->_currPos;
			part->_contactPos.y = _yFloor;
		}
		else
		if (part->_currPos.y < _yFloor - 1 && part->_isInContact && !part->_gripping) {
			part->_isInContact = false;
		}
	}
	
	// Enforce constraints:
	int maxConstraint = 60;
	while (maxConstraint > 0) {
		maxConstraint--;
		
		float maxDeviation = 0;
		
		// Rigid bars:
		for (size_t i = 0; i < _rigidBars.size(); i++) {
			RigidBar * rb = _rigidBars.at(i);
			
			Vector2D vP1ToP2(rb->_p2->_currPos.x - rb->_p1->_currPos.x,
			                 rb->_p2->_currPos.y - rb->_p1->_currPos.y);
			float dist = vP1ToP2.length();
			vP1ToP2 /= dist;
			
			float deviationCausingAttraction = (dist - rb->_length);
			
			rb->_p1->_currPos.x += 0.5f * deviationCausingAttraction * vP1ToP2.x;
			rb->_p1->_currPos.y += 0.5f * deviationCausingAttraction * vP1ToP2.y;
			
			rb->_p2->_currPos.x -= 0.5f * deviationCausingAttraction * vP1ToP2.x;
			rb->_p2->_currPos.y -= 0.5f * deviationCausingAttraction * vP1ToP2.y;
			
			if (fabs(deviationCausingAttraction) > maxDeviation)
				maxDeviation = fabs(deviationCausingAttraction);
		}
		
		// Joint angles:
		for (size_t i = 0; i < _joints.size(); i++) {
			Joint * joint = _joints.at(i);
			
			Vector2D v1(joint->_p1->_currPos.x - joint->_pivot->_currPos.x,
			            joint->_p1->_currPos.y - joint->_pivot->_currPos.y);
			Vector2D v2(joint->_p2->_currPos.x - joint->_pivot->_currPos.x,
			            joint->_p2->_currPos.y - joint->_pivot->_currPos.y);
			
			float angle = v1.angle(v2);
			
			if (angle < joint->_minAngle) {
				float deltaAngle = (joint->_minAngle - angle);
				v1.rotate(-deltaAngle/2);
				v2.rotate(+deltaAngle/2);
			}
			
			if (angle > joint->_maxAngle) {
				float deltaAngle = (joint->_maxAngle - angle);
				v1.rotate(-deltaAngle/2);
				v2.rotate(+deltaAngle/2);
			}
			
			Vector2D newPos1(joint->_pivot->_currPos.x + v1.x,
			                 joint->_pivot->_currPos.y + v1.y);
			Vector2D newPos2(joint->_pivot->_currPos.x + v2.x,
			                 joint->_pivot->_currPos.y + v2.y);
			
			{
				Vector2D dirOfPivotDispl((newPos1.x - joint->_p1->_currPos.x) + (newPos2.x - joint->_p2->_currPos.x),
				                         (newPos1.y - joint->_p1->_currPos.y) + (newPos2.y - joint->_p2->_currPos.y));
				dirOfPivotDispl /= 3;
				
				newPos1 -= dirOfPivotDispl;
				newPos2 -= dirOfPivotDispl;
				joint->_pivot->_currPos -= dirOfPivotDispl;
			}
			
			float deviation;
			{
				float dx1 = newPos1.x - joint->_p1->_currPos.x;
				float dy1 = newPos1.y - joint->_p1->_currPos.y;
				float dx2 = newPos2.x - joint->_p2->_currPos.x;
				float dy2 = newPos2.y - joint->_p2->_currPos.y;
				
				deviation = max(sqrt(dx1 * dx1 + dy1 * dy1),
			                    sqrt(dx2 * dx2 + dy2 * dy2));
			}
			
			if (deviation > maxDeviation)
				maxDeviation = deviation;
			
			joint->_p1->_currPos = newPos1;
			joint->_p2->_currPos = newPos2;
		}
		
		// Keep above floor:
		for (size_t i = 0; i < _particles.size(); i++) {
			if (_particles.at(i)->_currPos.y > _yFloor) {
				float deviation = _particles.at(i)->_currPos.y - _yFloor;
				_particles.at(i)->_currPos.y = _yFloor;
				
				if (deviation > maxDeviation)
					maxDeviation = deviation;
			}
		}
		
		// Maintain grip:
		for (size_t i = 0; i < _particles.size(); i++) {
			Particle * part = _particles.at(i);
			if (part->_gripping && part->_isInContact) {
				float deviation;
				{
					float dx = part->_currPos.x - part->_contactPos.x;
					float dy = part->_currPos.y - part->_contactPos.y;
					deviation = sqrt(dx * dx + dy * dy);
				}
				
				part->_currPos = part->_contactPos;
				part->_prevPos = part->_contactPos;
				
				if (deviation > maxDeviation)
					maxDeviation = deviation;
			}
		}
		
		// All constraints within reasonable threshold?
		if (maxDeviation <= 1)
			break;
	}
}



