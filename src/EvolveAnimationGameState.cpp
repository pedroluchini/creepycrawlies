#include <EvolveAnimationGameState.hpp>

#include <AnimationViewerGameState.hpp>
#include <AutoHGE.hpp>
#include <cfloat>
#include <GameStateStack.hpp>
#include <hgefont.h>
#include <util.hpp>

extern float CREATURE_UPDATE_DT;
extern float CREATURE_OPCODE_TICK_DT;
extern hgeFont * font;
extern hgeFont * fontBig;




#define DRAW_BTN(rect, isPressed) \
{\
	DWORD colHighlight = 0x7FFFFFFF;\
	DWORD colShade     = 0x7F000000;\
	hge->Gfx_RenderLine(rect.x1, rect.y1, rect.x2, rect.y1, (isPressed ? colShade : colHighlight));\
	hge->Gfx_RenderLine(rect.x2, rect.y1, rect.x2, rect.y2, (isPressed ? colHighlight : colShade));\
	hge->Gfx_RenderLine(rect.x2, rect.y2, rect.x1, rect.y2, (isPressed ? colHighlight : colShade));\
	hge->Gfx_RenderLine(rect.x1, rect.y2, rect.x1, rect.y1, (isPressed ? colShade : colHighlight));\
}




static Creature::OPCODE makeRandomOpCode(Creature * creature)
{
	return (Creature::OPCODE) GARandomInt(0, (int) (1.5 * creature->getNumOfValidOpCodes()));
}




static void initializer(GAGenome & gaGenome)
{
	GAListGenome<Creature::OPCODE> & gaListGenome = (GAListGenome<Creature::OPCODE> &) gaGenome;
	Creature * creature = (Creature *) gaGenome.userData();
	
	// Make sure the list is empty:
	while (gaListGenome.head())
		gaListGenome.destroy();
	
	// Insert a random number of random op-codes:
	int n = GARandomInt(50, 200);
	for (int i = 0; i < n; i++)
		gaListGenome.insert(
			makeRandomOpCode(creature),
			GAListBASE::HEAD);
}




static float objective(GAGenome & c)
{
	GAListGenome<Creature::OPCODE> & genome = (GAListGenome<Creature::OPCODE> &)c;
	Creature * creature = (Creature *) genome.userData();
	
	if (genome.size() == 0)
		return 0;
	
	// Reset the simulation:
	creature->reset();
	
	// Get the initial centroid of the creature:
	Vector2D initialCentroid = creature->getCurrentPositionCentroid();
	
	// Extract the op-codes from the genome:
	Creature::OPCODE * opCodes = new Creature::OPCODE[genome.size()];
	opCodes[0] = *genome.head();
	for(int i = 1; i < genome.size(); i++)
		opCodes[i] = *genome.next();
	
	creature->setAlgorithm(opCodes, genome.size(), CREATURE_OPCODE_TICK_DT);
	
	delete opCodes;
	
	// Evaluate: Run it for 7 seconds
	float time = 0;
	while (time < 7) {
		creature->update(CREATURE_UPDATE_DT);
		time += CREATURE_UPDATE_DT;
	}
	
	// Get the final centroid of the creature:
	Vector2D finalCentroid = creature->getCurrentPositionCentroid();
	
	// Done! objective is the total X-displacement divided by the time:
	float speed = (finalCentroid.x - initialCentroid.x)/time;
	
	if (speed != speed || _isnan(speed) || !_finite(speed))
		// Invalid float!
		return -10000;
	else
		return speed;
}




static int constructiveMutator(GAGenome & c, float pmut)
{
	GAListGenome<Creature::OPCODE> & genome = (GAListGenome<Creature::OPCODE> &)c;
	Creature * creature = (Creature *) genome.userData();
	
	genome.head();
	
	int nMutations = 0;
	for (int i = 0; i <= genome.size(); i++) {
		if (GAFlipCoin(pmut)) {
			genome.insert(makeRandomOpCode(creature));
			nMutations++;
		}
		genome.next();
	}
	
	return nMutations;
}




static int flipMutator(GAGenome & c, float pmut)
{
	GAListGenome<Creature::OPCODE> & genome = (GAListGenome<Creature::OPCODE> &)c;
	Creature * creature = (Creature *) genome.userData();
	
	genome.head();
	
	int nMutations = 0;
	for (int i = 0; i < genome.size(); i++) {
		if (GAFlipCoin(pmut)) {
			*(genome.current()) = makeRandomOpCode(creature);
			nMutations++;
		}
		genome.next();
	}
	
	return nMutations;
}




static int mutator(GAGenome & c, float pmut)
{
	GAListGenome<Creature::OPCODE> & genome = (GAListGenome<Creature::OPCODE> &)c;
	
	// Choose randomly which mutator to apply:
	if (GAFlipCoin(0.25f))
		return GAListGenome<Creature::OPCODE>::SwapMutator(c, pmut);
	else
	if (GAFlipCoin(1.0f/3.0f))
		return GAListGenome<Creature::OPCODE>::DestructiveMutator(c, pmut);
	else
	if (GAFlipCoin(0.5f))
		return constructiveMutator(c, pmut);
	else
		return flipMutator(c, pmut);
}




EvolveAnimationGameState::EvolveAnimationGameState(const CreatureDescriptor & creatureDescr)
: _creatureDescr(creatureDescr),
  _creature(creatureDescr),
  _mouseClickedX(0),
  _mouseClickedY(0),
  _genome(NULL),
  _scale(NULL),
  _ga(NULL)
{
	AutoHGE hge;
	
	_accDT = 0;
	
	_rectViewResult.x1 = (float) hge->System_GetState(HGE_SCREENWIDTH)/2 - 60;
	_rectViewResult.x2 = _rectViewResult.x1 + 120;
	_rectViewResult.y2 = (float) hge->System_GetState(HGE_SCREENHEIGHT) - 70;
	_rectViewResult.y1 = _rectViewResult.y2 - font->GetHeight() - 16;
	
	// Initialize the GA:
	_genome = new GAListGenome<Creature::OPCODE>(objective, &_creature);
	_genome->initializer(initializer);
	_genome->mutator(mutator);
	
	_scale = new GASigmaTruncationScaling();
	
	_ga = new GASteadyStateGA(*_genome);
	_ga->scaling(*_scale);
	_ga->crossover(GAListGenome<Creature::OPCODE>::OnePointCrossover);
	_ga->userData(&_creature);
	
	_ga->nReplacement((int) (0.75 * 40));
	_ga->nConvergence(20);
	_ga->set(gaNpopulationSize, 40);  // population size
	_ga->set(gaNpCrossover, 0.9);     // probability of crossover
	_ga->set(gaNpMutation, 0.02);     // probability of mutation
//	_ga->set(gaNnGenerations, 150);   // number of generations
	_ga->set(gaNscoreFrequency, 1);   // how often to record scores
	_ga->set(gaNflushFrequency, 10);  // how often to dump scores to file
	_ga->set(gaNselectScores,         // which scores should we track?
	       GAStatistics::Maximum|GAStatistics::Minimum|GAStatistics::Mean);
//	_ga->set(gaNscoreFilename, "bog.dat");
	
	// Get ready to start!
	_ga->initialize();
}




EvolveAnimationGameState::~EvolveAnimationGameState()
{
	delete _ga;
	delete _scale;
	delete _genome;
}




void EvolveAnimationGameState::update(const GameStateUpdateEvent & evt)
{
	// Evolve one generation:
	_ga->step();
}




void EvolveAnimationGameState::render(const GameStateRenderEvent & evt) const
{
	AutoHGE hge;
	
	float mouseX, mouseY;
	hge->Input_GetMousePos(&mouseX, &mouseY);
	
	// GA statistics:
	{
		float x = (float) hge->System_GetState(HGE_SCREENWIDTH)/2;
		float y = 120;
		
		font->SetColor(0xFFFFFFFF);
		fontBig->SetColor(0xFFFFFFFF);
		
		font->Render(x, y, HGETEXT_CENTER, "Current generation:");
		
		y += font->GetHeight() + 15;
		
		fontBig->printf(x, y, HGETEXT_CENTER, "%d", _ga->generation());
		
		y += fontBig->GetHeight() + 15;
		
		y += 30;
		
		font->Render(x, y, HGETEXT_CENTER, "Max. speed achieved:");
		
		y += font->GetHeight() + 15;
		
		fontBig->printf(x, y, HGETEXT_CENTER, "%.2f px/s", _ga->statistics().maxEver());
	}
	
	// Tool bar:
	{
		hgeQuad q;
		q.blend = BLEND_DEFAULT;
		q.tex = NULL;
		q.v[0].col = q.v[1].col = q.v[2].col = q.v[3].col = 0x7F007F7F;
		q.v[0].tx  = q.v[1].tx  = q.v[2].tx  = q.v[3].tx  = 0;
		q.v[0].ty  = q.v[1].ty  = q.v[2].ty  = q.v[3].ty  = 0;
		q.v[0].z   = q.v[1].z   = q.v[2].z   = q.v[3].z   = 0;
		q.v[0].x = _rectViewResult.x1;
		q.v[1].x = _rectViewResult.x2;
		q.v[2].x = _rectViewResult.x2;
		q.v[3].x = _rectViewResult.x1;
		q.v[0].y = _rectViewResult.y1;
		q.v[1].y = _rectViewResult.y1;
		q.v[2].y = _rectViewResult.y2;
		q.v[3].y = _rectViewResult.y2;
		hge->Gfx_RenderQuad(&q);
		
		font->SetColor(0xAFFFFFFF);
		font->Render(
			(_rectViewResult.x1 + _rectViewResult.x2)/2,
			(_rectViewResult.y1 + _rectViewResult.y2)/2 - font->GetHeight()/2,
			HGETEXT_CENTER,
			"Pause and view result");
		
		DRAW_BTN(
			_rectViewResult,
			(hge->Input_GetKeyState(HGEK_LBUTTON) &&
			 _rectViewResult.TestPoint(_mouseClickedX, _mouseClickedY) &&
			 _rectViewResult.TestPoint(mouseX, mouseY)));
		
	}
	
}




void EvolveAnimationGameState::processInputEvent(const GameStateInputEvent & evt)
{
	// Update mouse params:
	if (evt.type == INPUT_MBUTTONDOWN && evt.key == HGEK_LBUTTON)
	{
		_mouseClickedX = evt.x;
		_mouseClickedY = evt.y;
	}
	
	// Go into animation-view mode if user clicks on "View result":
	if (evt.type == INPUT_MBUTTONUP &&
	    evt.key == HGEK_LBUTTON &&
	    _rectViewResult.TestPoint(evt.x, evt.y) &&
	    _rectViewResult.TestPoint(_mouseClickedX, _mouseClickedY))
	{
		// Extract the op-codes from the genome:
		GAListGenome<Creature::OPCODE> genome;
		genome = _ga->statistics().bestIndividual();
		Creature::OPCODE * opCodes = new Creature::OPCODE[genome.size()];
		opCodes[0] = *genome.head();
		for(int i = 1; i < genome.size(); i++)
			opCodes[i] = *genome.next();
		
		evt.source->pushState(
			new AnimationViewerGameState(
				_creatureDescr,
				opCodes,
				genome.size()));
	}
	
}

