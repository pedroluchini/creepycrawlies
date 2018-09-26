#include <Creature.hpp>

#include <cfloat>
#include <util.hpp>

#define M_PI                    3.14159265358979323846f
#define GRAVITY_Y            2700.0f
#define PARTICLE_DAMPENING      0.015f
#define SPRING_K              400.0f
#define MUSCLE_LENGTH_SCALE     0.7f
#define MIN_ANGLE              (M_PI/24)
#define MAX_ANGLE              (0.9f*M_PI - MIN_ANGLE)
#define MUSCLE_CONTRACT_SCALE   0.5f
#define MUSCLE_STRETCH_SCALE    2.0f

#define OPCODE_START_CONTRACT_MUSCLE  1
#define OPCODE_END_CONTRACT_MUSCLE   (OPCODE_START_CONTRACT_MUSCLE + (OPCODE) _muscles.size())
#define OPCODE_START_RELAX_MUSCLE    OPCODE_END_CONTRACT_MUSCLE
#define OPCODE_END_RELAX_MUSCLE      (OPCODE_START_RELAX_MUSCLE + (OPCODE) _muscles.size())
#define OPCODE_START_STRETCH_MUSCLE  OPCODE_END_RELAX_MUSCLE
#define OPCODE_END_STRETCH_MUSCLE    (OPCODE_START_STRETCH_MUSCLE + (OPCODE) _muscles.size())
#define OPCODE_START_GRIP_CLAW       OPCODE_END_STRETCH_MUSCLE
#define OPCODE_END_GRIP_CLAW         (OPCODE_START_GRIP_CLAW + (OPCODE) _claws.size())
#define OPCODE_START_UNGRIP_CLAW     OPCODE_END_GRIP_CLAW
#define OPCODE_END_UNGRIP_CLAW       (OPCODE_START_UNGRIP_CLAW + (OPCODE) _claws.size())
#define OPCODE_END                   OPCODE_END_UNGRIP_CLAW




static float estimateFloorY(const CreatureDescriptor & desc)
{
	float y = FLT_MIN;
	
	for (size_t i = 0; i < desc.claws.size(); i++)
		if (desc.claws.at(i).pos.y + 20 > y)
			y = desc.claws.at(i).pos.y + 20;
	
	return y;
}




template <typename T>
static void validateIDs(const std::vector<T> & v,
                        const std::string & typeDesc)
{
	for (size_t i = 0; i < v.size(); i++)
		for (size_t j = 0; j < v.size(); j++)
			if (i != j)
				if (v.at(i).id == v.at(j).id)
					throw std::runtime_error(
						"Invalid " + typeDesc + ": repeated ID #" + toString(v.at(i).id));
}




static int getClawIndexFromID(const CreatureDescriptor & desc,
                              int id)
{
	for (size_t i = 0; i < desc.claws.size(); i++)
		if (desc.claws.at(i).id == id)
			return static_cast<int>(i);
	
	return -1;
}




Creature::Creature(const CreatureDescriptor & desc)
: _system(estimateFloorY(desc), GRAVITY_Y),
  _dtTick(0),
  _iNextOpCode(0),
  _accOpCodeDT(0)
{
	validateIDs(desc.bones, "bone");
	validateIDs(desc.claws, "claw");
	validateIDs(desc.muscles, "muscle");
	
	// Claws are easy to instantiate:
	for (size_t i = 0; i < desc.claws.size(); i++) {
		_claws.push_back(
			PhysicsSystem::Particle(
				desc.claws.at(i).pos,
				PARTICLE_DAMPENING));
		
		_initClawPositions.push_back(desc.claws.at(i).pos);
	}
	
	// Muscles need to be connected to two claws:
	for (size_t i = 0; i < desc.muscles.size(); i++)
	{
		int iClaw1 = getClawIndexFromID(desc, desc.muscles.at(i).clawID1);
		int iClaw2 = getClawIndexFromID(desc, desc.muscles.at(i).clawID2);
		
		if (iClaw1 < 0 || iClaw2 < 0)
			throw std::runtime_error(
				"Muscle #" +
				toString(desc.muscles.at(i).id) +
				" uses undefined claw IDs");
		
		float length = MUSCLE_LENGTH_SCALE *
			(_claws.at(iClaw1).getCurrentPosition() -
			 _claws.at(iClaw2).getCurrentPosition()).length();
		
		_muscles.push_back(
			PhysicsSystem::Spring(
				&_claws.at(iClaw1),
				&_claws.at(iClaw2),
				SPRING_K,
				length));
		
		_initMuscleLengths.push_back(length);
	}
	
	// Bones need to be connected to two claws:
	for (size_t i = 0; i < desc.bones.size(); i++)
	{
		int iClaw1 = getClawIndexFromID(desc, desc.bones.at(i).clawID1);
		int iClaw2 = getClawIndexFromID(desc, desc.bones.at(i).clawID2);
		
		if (iClaw1 < 0 || iClaw2 < 0)
			throw std::runtime_error(
				"Bone #" +
				toString(desc.bones.at(i).id) +
				" uses undefined claw IDs");
		
		_bones.push_back(
			PhysicsSystem::RigidBar(
				&_claws.at(iClaw1),
				&_claws.at(iClaw2)));
	}
	
	// Joints need to be created for every pair of bones that share a claw:
	for (size_t i = 0; i < _bones.size(); i++) {
		for (size_t j = i + 1; j < _bones.size(); j++) {
			PhysicsSystem::Particle * iP1 = _bones.at(i).getP1();
			PhysicsSystem::Particle * iP2 = _bones.at(i).getP2();
			PhysicsSystem::Particle * jP1 = _bones.at(j).getP1();
			PhysicsSystem::Particle * jP2 = _bones.at(j).getP2();
			
			PhysicsSystem::Particle * pivot = NULL;
			PhysicsSystem::Particle * barP1 = NULL;
			PhysicsSystem::Particle * barP2 = NULL;
			if (iP1 == jP1) {
				pivot = iP1;
				barP1 = iP2;
				barP2 = jP2;
			}
			else
			if (iP1 == jP2) {
				pivot = iP1;
				barP1 = iP2;
				barP2 = jP1;
			}
			else
			if (iP2 == jP2) {
				pivot = iP2;
				barP1 = iP1;
				barP2 = jP1;
			}
			else
			if (iP2 == jP1) {
				pivot = iP2;
				barP1 = iP1;
				barP2 = jP2;
			}
			
			if (!pivot)
				// These two bones don't share a pivot:
				continue;
			
			// OK, we've got a pivot.
			
			// Make sure the joint isn't degenerate:
			if (barP1 == barP2)
				continue;
			
			// Which way is the joint facing? The cross product of v1 % v2
			// needs to be pointing at Z positive (i.e., the angle from v1 to
			// v2 needs to be positive).
			Vector2D v1 = (barP1->getCurrentPosition() - pivot->getCurrentPosition());
			Vector2D v2 = (barP2->getCurrentPosition() - pivot->getCurrentPosition());
			if (v1.angle(v2) < 0) {
				// Swap 'em:
				std::swap(v1, v2);
				std::swap(barP1, barP2);
			}
			
			// If it's already violating a constraint, don't create the joint.
			if (fabs(v1.angle(v2)) > MAX_ANGLE)
				continue;
			
			// That's all we need:
			_joints.push_back(
				PhysicsSystem::Joint(pivot, barP1, barP2, MIN_ANGLE, MAX_ANGLE));
		}
	}
	
	// Done! Add everything into the PhysicsSystem:
	for (size_t i = 0; i < _claws.size(); i++)
		_system.addParticle(&_claws.at(i));
	
	for (size_t i = 0; i < _bones.size(); i++)
		_system.addRigidBar(&_bones.at(i));
	
	for (size_t i = 0; i < _joints.size(); i++)
		_system.addJoint(&_joints.at(i));
	
	for (size_t i = 0; i < _muscles.size(); i++)
		_system.addSpring(&_muscles.at(i));
}




Creature::OPCODE Creature::makeOpCode(OpCodeCommand cmd, int param) const
{
	if (cmd == OPCMD_CONTRACT_MUSCLE || cmd == OPCMD_RELAX_MUSCLE || cmd == OPCMD_STRETCH_MUSCLE)
		if (param < 0 || param > (int) _muscles.size())
			cmd = OPCMD_WAIT_TICK;
	
	if (cmd == OPCMD_GRIP_CLAW || cmd == OPCMD_UNGRIP_CLAW)
		if (param < 0 || param > (int) _claws.size())
			cmd = OPCMD_WAIT_TICK;
	
	// Pack 'em into 16 bits and return:
	if (cmd == OPCMD_WAIT_TICK)
		return 0;
	if (cmd == OPCMD_CONTRACT_MUSCLE)
		return OPCODE_START_CONTRACT_MUSCLE + (OPCODE) param;
	if (cmd == OPCMD_RELAX_MUSCLE)
		return OPCODE_START_RELAX_MUSCLE + (OPCODE) param;
	if (cmd == OPCMD_STRETCH_MUSCLE)
		return OPCODE_START_STRETCH_MUSCLE + (OPCODE) param;
	if (cmd == OPCMD_GRIP_CLAW)
		return OPCODE_START_GRIP_CLAW + (OPCODE) param;
	if (cmd == OPCMD_UNGRIP_CLAW)
		return OPCODE_START_UNGRIP_CLAW + (OPCODE) param;
	
	return 0;
}




void Creature::decodeOpCode(OPCODE opCode, OpCodeCommand * cmd, int * param) const
{
	if (opCode >= OPCODE_START_CONTRACT_MUSCLE && opCode < OPCODE_END_CONTRACT_MUSCLE)
	{
		*cmd = OPCMD_CONTRACT_MUSCLE;
		*param = opCode - OPCODE_START_CONTRACT_MUSCLE;
	}
	else
	if (opCode >= OPCODE_START_RELAX_MUSCLE && opCode < OPCODE_END_RELAX_MUSCLE)
	{
		*cmd = OPCMD_RELAX_MUSCLE;
		*param = opCode - OPCODE_START_RELAX_MUSCLE;
	}
	else
	if (opCode >= OPCODE_START_STRETCH_MUSCLE && opCode < OPCODE_END_STRETCH_MUSCLE)
	{
		*cmd = OPCMD_STRETCH_MUSCLE;
		*param = opCode - OPCODE_START_STRETCH_MUSCLE;
	}
	else
	if (opCode >= OPCODE_START_GRIP_CLAW && opCode < OPCODE_END_GRIP_CLAW)
	{
		*cmd = OPCMD_GRIP_CLAW;
		*param = opCode - OPCODE_START_GRIP_CLAW;
	}
	else
	if (opCode >= OPCODE_START_UNGRIP_CLAW && opCode < OPCODE_END_UNGRIP_CLAW)
	{
		*cmd = OPCMD_UNGRIP_CLAW;
		*param = opCode - OPCODE_START_UNGRIP_CLAW;
	}
	else
	{
		*cmd = OPCMD_WAIT_TICK;
	}
}




void Creature::update(float dt)
{
	// Is it time to execute an op-code?
	if (_opCodes.size() > 0)
	{
		_accOpCodeDT += dt;
		while (_accOpCodeDT >= _dtTick) {
			_accOpCodeDT -= _dtTick;
			
			_executeOpCode(_opCodes.at(_iNextOpCode));
			
			_iNextOpCode++;
			if (_iNextOpCode >= _opCodes.size())
				_iNextOpCode = 0;
		}
	}
	
	// Update the physics system:
	_system.update(dt);
}




void Creature::_executeOpCode(OPCODE opCode)
{
	OpCodeCommand cmd;
	int param;
	decodeOpCode(opCode, &cmd, &param);
	
	if (cmd == OPCMD_CONTRACT_MUSCLE)
		_muscles.at(param).setLength(_initMuscleLengths.at(param) * MUSCLE_CONTRACT_SCALE);
	else
	if (cmd == OPCMD_RELAX_MUSCLE)
		_muscles.at(param).setLength(_initMuscleLengths.at(param));
	else
	if (cmd == OPCMD_STRETCH_MUSCLE)
		_muscles.at(param).setLength(_initMuscleLengths.at(param) * MUSCLE_STRETCH_SCALE);
	else
	if (cmd == OPCMD_GRIP_CLAW)
		_claws.at(param).setGripping(true);
	else
	if (cmd == OPCMD_UNGRIP_CLAW)
		_claws.at(param).setGripping(false);
	
}




unsigned long Creature::getNumOfValidOpCodes() const
{
	return OPCODE_END;
}




void Creature::reset()
{
	for (size_t i = 0; i < _claws.size(); i++) {
		_claws.at(i).resetPosition(_initClawPositions.at(i));
		_claws.at(i).setGripping(false);
	}
	
	for (size_t i = 0; i < _muscles.size(); i++) {
		_muscles.at(i).setLength(_initMuscleLengths.at(i));
	}
	
	_iNextOpCode = 0;
	_accOpCodeDT = 0;
}




Vector2D Creature::getCurrentPositionCentroid() const
{
	Vector2D v(0, 0);
	
	for (size_t i = 0; i < _claws.size(); i++)
		v += _claws.at(i).getCurrentPosition();
	
	v.x /= _claws.size();
	v.y /= _claws.size();
	
	return v;
}


