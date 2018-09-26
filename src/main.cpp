
#include <AutoHGE.hpp>
#include <GameStateStack.hpp>
#include <hge.h>
#include <CreatureEditorGameState.hpp>
#include <vld.h>




float CREATURE_UPDATE_DT = 1.0f/500.0f;
float CREATURE_OPCODE_TICK_DT = 2.0f/256.0f;
hgeFont * font = NULL;
hgeFont * fontBig = NULL;
HTEXTURE texSprites = NULL;



static GameStateStack * g_gameStateStack = NULL;




bool frameFunction()
{
	AutoHGE hge;
	
	// Toggle full screen:
	if (hge->Input_GetKey() == HGEK_ENTER && hge->Input_GetKeyState(HGEK_ALT))
		hge->System_SetState(
			HGE_WINDOWED,
			!hge->System_GetState(HGE_WINDOWED));
	
	// Update the game stack:
	hgeInputEvent evt;
	while (hge->Input_GetEvent(&evt))
		g_gameStateStack->dispatchEvent(evt);
	
	g_gameStateStack->update(hge->Timer_GetDelta());
	
	return false;
}



bool renderFunction()
{
	AutoHGE hge;
	
	hge->Gfx_BeginScene();
	{
		hge->Gfx_Clear(0);
		
		g_gameStateStack->render();
	}
	hge->Gfx_EndScene();
	
	return false;
}




int WINAPI WinMain ( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow )
{
	// Set up HGE:
	HGE * hge = hgeCreate(HGE_VERSION);
	hge->System_SetState(HGE_SHOWSPLASH, false);
	
	hge->System_SetState(HGE_FRAMEFUNC,     frameFunction);
	hge->System_SetState(HGE_RENDERFUNC,    renderFunction);
	hge->System_SetState(HGE_TITLE,         "Creepy Crawlies - An experiment in procedural animation" );
	hge->System_SetState(HGE_SCREENWIDTH,   640);
	hge->System_SetState(HGE_SCREENHEIGHT,  480);
	hge->System_SetState(HGE_DONTSUSPEND,   true);
	hge->System_SetState(HGE_HIDEMOUSE,     false);
	hge->System_SetState(HGE_USESOUND,      false);
	hge->System_SetState(HGE_WINDOWED,      true);
	hge->System_SetState(HGE_SCREENBPP,     32);
	hge->System_SetState(HGE_FPS,           HGEFPS_VSYNC);
	
	if (!hge->System_Initiate())
		MessageBox(NULL, hge->System_GetErrorMessage(), "Error", MB_OK | MB_ICONERROR | MB_SYSTEMMODAL);
	
	// Load resources:
	font = new hgeFont("font2.fnt");
	fontBig = new hgeFont("font1.fnt");
	texSprites = hge->Texture_Load("sprites.png");
	
	// Set up the game state stack:
	g_gameStateStack = new GameStateStack();
	
	// Start with the main creature editor:
	g_gameStateStack->pushState(new CreatureEditorGameState());
	
	// Start the game loop:
	if (!hge->System_Start()) {
		MessageBox(NULL, hge->System_GetErrorMessage(), "Error", MB_OK | MB_ICONERROR | MB_SYSTEMMODAL);
	}
	
	// Done!
	delete g_gameStateStack;
	hge->Texture_Free(texSprites);
	delete font;
	delete fontBig;
	hge->System_Shutdown();
	hge->Release();
	
	return 0;
}
