/*
*******************************************************************************

Copyright (C) 2019-2020 SugarBombEngine Developers

This file is part of SugarBombEngine

SugarBombEngine is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

SugarBombEngine is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with SugarBombEngine. If not, see <http://www.gnu.org/licenses/>.

*******************************************************************************
*/

/// @file

//*****************************************************************************

#include <cstdlib>

#include "ServerApp.hpp"
#include "SbGameFrameworkExternal.hpp"

//*****************************************************************************

sbe::IGameFramework *CreateGameFramework()
{
#ifndef SBE_GAMEFRAMEWORK_HARD_LINKED
	static sbe::SbGameFrameworkExternal SbGameFrameworkModule();
	return SbGameFrameworkModule.GetGameFramework();
#else
	return new sbe::SbGameFramework::SbGameFramework();
#endif
};

int main(int argc, const char **argv)
{
	sbe::IGameFramework *pGameFramework = CreateGameFramework();

	f3goaty::CGameServerApp App(pGameFramework, /*pSoundSystem, pRenderSystem, pInputSystem, pSystem,*/ argc, argv); // TODO: CServerApp
	App.Run();

	// never gets here
	return EXIT_SUCCESS;
};