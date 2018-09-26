#ifndef CREATURE_HPP
#define CREATURE_HPP

#include <PhysicsSystem.hpp>
#include <vector>

struct CreatureDescriptor
{
	struct Claw {
		Claw(int _id, const Vector2D & _pos) : id(_id), pos(_pos) {}
		
		int id;
		Vector2D pos;
	};
	
	struct Bone {
		Bone(int _id, int _clawID1, int _clawID2) : id(_id), clawID1(_clawID1), clawID2(_clawID2) {}
		
		int id;
		int clawID1, clawID2;
	};
	
	struct Muscle {
		Muscle(int _id, int _clawID1, int _clawID2) : id(_id), clawID1(_clawID1), clawID2(_clawID2) {}
		
		int id;
		int clawID1, clawID2;
	};
	
	std::vector<Claw> claws;
	std::vector<Bone> bones;
	std::vector<Muscle> muscles;
};

class Creature
{
	public:
		
		enum OpCodeCommand
		{
			/** Do not change any states during this tick. */
			OPCMD_WAIT_TICK = 0,
			
			/** Change a muscle's state to "contracted" (set the spring's
			*   length to half the muscle's initial muscle length). The opcode 
			 *  parameter is the muscle ID. */
			OPCMD_CONTRACT_MUSCLE = 1,
			
			/** Change a muscle's state to "relaxed" (set the spring's length
			 *  to the muscle's initial length). The opcode parameter is the
			 *  muscle index. */
			OPCMD_RELAX_MUSCLE = 2,
			
			/** Change a muscle's state to "stretched" (set the spring's length
			 *  to twice the muscle's initial length). The opcode parameter is
			 *  the muscle index. */
			OPCMD_STRETCH_MUSCLE = 3,
			
			/** Change a claw's state to "gripping" (sticks to the floor). The 
			 *  opcode parameter is the claw index. */
			OPCMD_GRIP_CLAW = 4,
			
			/** Change a claw's state to "non-gripping" (does not stick to the 
			 *  floor). The opcode parameter is the claw index. */
			OPCMD_UNGRIP_CLAW = 5,
			
			/** @internal */
			OPCMD_MAX = 5
		};
		
		typedef unsigned short OPCODE;
		
		/** @throw std::runtime_error */
		Creature(const CreatureDescriptor & desc);
		
		inline int getNumClaws() const { return _system.getNumParticles(); }
		inline PhysicsSystem::Particle * getClaw(int i) { return _system.getParticle(i); }
		inline const PhysicsSystem::Particle * getClaw(int i) const { return _system.getParticle(i); }
		
		inline int getNumBones() const { return _system.getNumRigidBars(); }
		inline PhysicsSystem::RigidBar * getBone(int i) { return _system.getRigidBar(i); }
		inline const PhysicsSystem::RigidBar * getBone(int i) const { return _system.getRigidBar(i); }
		
		inline int getNumJoints() const { return _system.getNumJoints(); }
		inline PhysicsSystem::Joint * getJoint(int i) { return _system.getJoint(i); }
		inline const PhysicsSystem::Joint * getJoint(int i) const { return _system.getJoint(i); }
		
		inline int getNumMuscles() const { return _system.getNumSprings(); }
		inline PhysicsSystem::Spring * getMuscle(int i) { return _system.getSpring(i); }
		inline const PhysicsSystem::Spring * getMuscle(int i) const { return _system.getSpring(i); }
		
		inline float getInitMuscleLength(int iMuscle) const { return _initMuscleLengths.at(iMuscle); }
		
		inline float getYFloor() const { return _system.getYFloor(); }
		
		void update(float dt);
		
		void reset();
		
		Vector2D getCurrentPositionCentroid() const;
		
		/**
		 * Given a command and a parameter, returns the integer representation
		 * of the op-code.
		 * @warning If the parameter is invalid (e.g., a non-existing claw
		 *          index for OPCMD_GRIP_CLAW), the op-code returned will
		 *          contain the command OPCMD_WAIT_TICK regardless of the cmd
		 *          argument.
		 */
		OPCODE makeOpCode(OpCodeCommand cmd, int param = 0) const;
		
		/**
		 * Given an op-code, returns its corresponding command and parameter.
		 * If the op-code is invalid, the command OPCMD_WAIT_TICK will be
		 * returned instead.
		 * @param[in]  opCode The op-code.
		 * @param[out] cmd    Pointer to store the op-code command.
		 * @param[out] param  Pointer to store the op-code parameter, if any.
		 */
		void decodeOpCode(OPCODE opCode, OpCodeCommand * cmd, int * param) const;
		
		/**
		 * @return The upper bound for op-codes. Any value greater or equal
		 *         to this will be an invalid op-code.
		 */
		unsigned long getNumOfValidOpCodes() const;
		
		/**
		 * @param[in] opCodes An array of op-codes to control the creature.
		 * @param[in] size    The number of elements in the array.
		 * @param[in] dtTick  The time interval inbetween op-code execution.
		 */
		inline void setAlgorithm(const OPCODE * opCodes, int size, float dtTick)
		{
			_opCodes.clear();
			for (int i = 0; i < size; i++)
				_opCodes.push_back(opCodes[i]);
			
			_iNextOpCode = 0;
			_accOpCodeDT = 0;
			_dtTick = dtTick;
		}
		
	private:
		PhysicsSystem _system;
		
		std::vector<PhysicsSystem::Particle> _claws;
		std::vector<PhysicsSystem::RigidBar> _bones;
		std::vector<PhysicsSystem::Joint>    _joints;
		std::vector<PhysicsSystem::Spring>   _muscles;
		
		std::vector<Vector2D> _initClawPositions;
		std::vector<float>    _initMuscleLengths;
		
		std::vector<OPCODE> _opCodes;
		size_t _iNextOpCode;
		float _dtTick;
		float _accOpCodeDT;
		
		void _executeOpCode(OPCODE opCode);
};

#endif
