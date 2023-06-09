#include "SDLRenderWindow.h"

#include <SDL.h>

#include <GL/glew.h>

#include "GlobalVariables.h"
#include "Renderer.h"

CSDLRenderWindow::CSDLRenderWindow(int width, int height) 
	: CRenderWindow(width, height)
{
	ResetKeys();

	m_sdlKeyMap[SDL_SCANCODE_F1] = Key::F1;
	m_sdlKeyMap[SDL_SCANCODE_F2] = Key::F2;
	m_sdlKeyMap[SDL_SCANCODE_F3] = Key::F3;
	m_sdlKeyMap[SDL_SCANCODE_F4] = Key::F4;
	m_sdlKeyMap[SDL_SCANCODE_F5] = Key::F5;
	m_sdlKeyMap[SDL_SCANCODE_F6] = Key::F6;
	m_sdlKeyMap[SDL_SCANCODE_F7] = Key::F7;
}

void CSDLRenderWindow::Init()
{
	SDL_Window*		window;
	SDL_GLContext	context;

	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		return;
	}

	/* Turn on double buffering with a 24bit Z buffer.
	* You may need to change this to 16 or 32 for your system */
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

	window = SDL_CreateWindow("App", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, m_width, m_height, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
	if (window == nullptr)
	{
		return;
	}

	context = SDL_GL_CreateContext(window);
	SDL_GL_SetSwapInterval(0);

	gVars->pRenderer->Init();

	while (ProcessEvents())
	{
		gVars->pRenderer->Update();
		SDL_GL_SwapWindow(window);
	}

	gVars->pRenderer->Reset();

	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

void CSDLRenderWindow::Reshape(int width, int height)
{
	m_width = width;
	m_height = height;
	gVars->pRenderer->Reshape(m_width, m_height);
}

Vec2 CSDLRenderWindow::GetMousePos()
{
	int x, y;
	SDL_GetMouseState(&x, &y);
	y = m_height - y;

	return Vec2((float)x, (float)y);
}

bool CSDLRenderWindow::GetMouseButton(int button)
{
	int flag = 0;
	switch (button)
	{
	case 0:	flag = SDL_BUTTON_LEFT; break;
	case 1: flag = SDL_BUTTON_MIDDLE; break;
	case 2: flag = SDL_BUTTON_RIGHT; break;
	}

	return ((SDL_GetMouseState(nullptr, nullptr) & SDL_BUTTON(flag)) != 0);
}

bool CSDLRenderWindow::IsPressingKey(Key key)
{
	return m_pressedKeys[(size_t)key];
}

bool CSDLRenderWindow::JustPressedKey(Key key)
{
	return m_justPressedKeys[(size_t)key];
}

bool CSDLRenderWindow::ProcessEvents()
{
	for (size_t i = 0; i < (size_t)Key::Count; ++i)
	{
		m_justPressedKeys[i] = false;
	}

	SDL_Event event;

	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
		case SDL_QUIT:
			return false;

		case SDL_WINDOWEVENT:
			{
				switch (event.window.event)
				{
				case SDL_WINDOWEVENT_RESIZED:
					Reshape(event.window.data1, event.window.data2);
					break;
				}
			}
			break;

		case SDL_KEYDOWN:
			{
				if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE)
				{
					return false;
				}

				auto itSdlKey = m_sdlKeyMap.find(event.key.keysym.scancode);
				if (itSdlKey != m_sdlKeyMap.end())
				{
					m_justPressedKeys[(size_t)itSdlKey->second] = !m_pressedKeys[(size_t)itSdlKey->second];
					m_pressedKeys[(size_t)itSdlKey->second] = true;					
				}
			}
			break;

		case SDL_KEYUP:
			{
				auto itSdlKey = m_sdlKeyMap.find(event.key.keysym.scancode);
				if (itSdlKey != m_sdlKeyMap.end())
				{
					m_pressedKeys[(size_t)itSdlKey->second] = false;
				}
			}
			break;
		}
	}

	return true;
}

void CSDLRenderWindow::ResetKeys()
{
	for (size_t i = 0; i < (size_t)Key::Count; ++i)
	{
		m_pressedKeys[i] = false;
		m_justPressedKeys[i] = false;
	}
}