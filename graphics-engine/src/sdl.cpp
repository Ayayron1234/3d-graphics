#include "pch.h"
#include "window.h"
#include "primitive_drawer.h"
#include "graphics_headers.h"
#include "camera.h"
#include "keyboard.h"
#include "mouse.h"
#include "sdl.h"
#include "object.h"
#include "imgui.h"
#include "viewport.h"

#pragma comment (lib, "Dwmapi")
#include <dwmapi.h>
#include "SDL2/SDL_syswm.h"

using namespace graphics;

struct KeyboardState {
	int			 arraySize = 0;
	const Uint8* data = nullptr;
	Uint8*		 prevData = nullptr;
	SDL_Keymod	 mod;

	void update() {
		memcpy(prevData, data, arraySize);
	}

	void init() {
		data = SDL_GetKeyboardState(&arraySize);
		prevData = new Uint8[arraySize];
	}

	void setMod(SDL_Keymod _mod) {
		mod = _mod;
	}
};

class SDL {
public:
	vec2i			mousePos;
	vec2i			prevMousePos;
	vec2i			mouseWindowPos;
	vec2i			prevMouseWindowPos;

	Uint8			mouseButtons = 0;
	Uint8			prevMouseButtons = 0;

	float			mouseScrollAmount = 0;

	KeyboardState	keyboardState{};

	static SDL& instance() {
		static SDL c_instance;
		return c_instance;
	}

	SDL() {
		// Init SDL
		SDL_Init(SDL_INIT_EVERYTHING);

		// Use OpenGL 3.3
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

		keyboardState.init();
	}

	void updateState() {
		// Update mouse button and scroll state
		prevMouseButtons = mouseButtons;
		prevMousePos = mousePos;
		prevMouseWindowPos = mouseWindowPos;
		mouseScrollAmount = 0;

		SDL_GetGlobalMouseState(&mousePos.x, &mousePos.y);
		SDL_GetMouseState(&mouseWindowPos.x, &mouseWindowPos.y);
	}

	void handleEvent(SDL_Event event) {
		switch (event.type)
		{
		case SDL_MOUSEBUTTONDOWN: {
			mouseButtons |= (1u << event.button.button);
		} break;
		case SDL_MOUSEBUTTONUP: {
			mouseButtons &= ~(1u << event.button.button);
		} break;
		case SDL_MOUSEWHEEL:
			mouseScrollAmount = event.wheel.preciseY;
			break;
		default:
			break;
		}
	}

	static void increaseTickCount() {
		++instance().m_tickCount;
	}

	static unsigned long getTicks() {
		return instance().m_tickCount;
	}

private:
	unsigned long	m_tickCount = 0;
};

struct Window::Impl {
	SDL_Window*		window;
	SDL_GLContext	glContext;
	bool			shouldQuit = false;
	bool			isOpen;

	Uint32			m_windowID;

	unsigned		m_width;
	unsigned		m_height;
	bool			m_didResize = false;

	unsigned long   m_tickCount = 0;

	bool			m_isImGuiInited = false;

	HWND			m_windowsWindowHandle;

	Viewport		m_viewport;

	Impl(unsigned width, unsigned height)
		: m_width(width)
		, m_height(height)
		, m_viewport(FrameBuffer::backbuffer())
	{ }

	void beginFrame() {
		PrimitiveDrawer::beginFrame();

		// Start the Dear ImGui frame
		if (m_isImGuiInited) {
			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplSDL2_NewFrame();
			ImGui::NewFrame();
			ImGui::DockSpaceOverViewport((const ImGuiViewport*)0, ImGuiDockNodeFlags_PassthruCentralNode);
		}
	}

	void endFrame() {
		PrimitiveDrawer::endFrame();

		// Render ImGui draw data
		if (m_isImGuiInited) {
			ImGui::Render();
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
			ImGui::EndFrame();
		}

		++m_tickCount;
		SDL::instance().keyboardState.update();
		SDL::instance().keyboardState.setMod(SDL_GetModState());

		// Swap buffers
		SDL_GL_SwapWindow(window);

		// Clear buffer
		glClearColor(.2f, .2f, .2f, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void handleEvents() {
		// Handle window resize
		int newWidth, newHeight;
		SDL_GetWindowSize(window, &newWidth, &newHeight);
		m_didResize = (newWidth != m_width || newHeight != m_height);
		if (m_didResize) {
			m_width = newWidth;
			m_height = newHeight;
			
			debug::camera->setAspectRatio(m_width, m_height);
			glViewport(0, 0, m_width, m_height);
			m_viewport.resize(m_width, m_height);
		}

		SDL::instance().updateState();
		SDL::increaseTickCount();

		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			// Pass event to ImGui
			if (m_isImGuiInited)
				ImGui_ImplSDL2_ProcessEvent(&event);

			SDL::instance().handleEvent(event);

			switch (event.type)
			{
			case SDL_QUIT: {
				shouldQuit = true;
			} break;
			case SDL_KEYDOWN: {
			}
			default:
				break;
			}
		}
	}
};

Window::Window(const std::string& title, unsigned width, unsigned height)
	: m_title(title)
	, impl(std::make_unique<Impl>(width, height))
{ }

void Window::open() {
	// Init SDL and openGL
	SDL::instance();

	// Create window
	impl->window = SDL_CreateWindow(m_title.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
									impl->m_width, impl->m_height, SDL_WINDOW_RESIZABLE | SDL_RENDERER_ACCELERATED);

	impl->m_windowID = SDL_GetWindowID(impl->window);

	// Create opengl context
	impl->glContext = SDL_GL_CreateContext(impl->window);
	if (!impl->glContext) {
		std::cerr << "OpenGL context could not be created! SDL Error: " << SDL_GetError() << std::endl;
		return;
	}
	SDL_GL_MakeCurrent(impl->window, impl->glContext);

	// Initialize GLEW after creating OpenGL context
	glewExperimental = GL_TRUE;
	GLenum glewError = glewInit();
	if (glewError != GLEW_OK) {
		std::cerr << "Error initializing GLEW! " << glewGetErrorString(glewError) << std::endl;
		return;
	}
	// Remove error caused by glewExperimental
	glGetError();

	// Use Vsync
	if (SDL_GL_SetSwapInterval(0) < 0) {
		std::cerr << "Warning: Unable to set VSync! SDL Error: " << SDL_GetError() << std::endl;
		return;
	}

	glViewport(0, 0, impl->m_width, impl->m_height);
	PrimitiveDrawer::beginFrame();

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	glEnable(GL_DEPTH_TEST); 
	glDepthFunc(GL_LESS);

	glEnable(GL_CULL_FACE);

	SDL_GL_SetSwapInterval(-1);

	debug::camera->setAspectRatio(impl->m_width, impl->m_height);
	debug::cout.rdbuf(std::cout.rdbuf());

	//Mesh::Shaders::matt.bindUniform("VP", debug::camera);
	//Mesh::Shaders::wireframe.bindUniform("VP", debug::camera);

	//Object::Shaders::matt.bindUniform("VP", debug::camera);
	//Object::Shaders::normal.bindUniform("VP", debug::camera);
	//Object::Shaders::textured.bindUniform("VP", debug::camera);
	//Object::Shaders::wireframe.bindUniform("VP", debug::camera);

	// Get window handle
	SDL_SysWMinfo wmInfo;
	SDL_VERSION(&wmInfo.version);
	SDL_GetWindowWMInfo(impl->window, &wmInfo);
	impl->m_windowsWindowHandle = wmInfo.info.win.window;

	impl->isOpen = true;
	impl->m_viewport.resize(impl->m_width, impl->m_height);
}

void graphics::Window::initImGui() {
	ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	//io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	ImGui::StyleColorsClassic();

	ImGui_ImplSDL2_InitForOpenGL(impl->window, impl->glContext);
	ImGui_ImplOpenGL3_Init("#version 330");

	impl->m_isImGuiInited = true;

	// Begin first imgui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame();
	ImGui::NewFrame();
	ImGui::DockSpaceOverViewport((const ImGuiViewport*)0, ImGuiDockNodeFlags_PassthruCentralNode | ImGuiDockNodeFlags_AutoHideTabBar);
}

void Window::close() {
	SDL_DestroyWindow(impl->window);
	impl->isOpen = false;
}

bool Window::isOpen() {
	return impl->isOpen;
}

void Window::render() {
	impl->endFrame();
	
	impl->handleEvents();
	if (impl->shouldQuit) {
		close();
		return;
	}

	impl->beginFrame();
}

vec2 Window::getSize() const {
	return vec2(impl->m_width, impl->m_height);
}

bool Window::resized() const {
	return impl->m_didResize;
}

vec2 transformToNds(vec2i v, unsigned width, unsigned height) {
	return vec2(2.f * v.x / (float)width - 1.f, 1.f - 2.f * v.y / (float)height);
}

//vec2 graphics::Window::getMousePosition() const {
//	return transformToNds(SDL::mousePosition(), impl->m_width, impl->m_height);
//}

vec2i graphics::Mouse::globalPosition() {
	return SDL::instance().mousePos;
}

vec2i graphics::Mouse::windowPosition() {
	return SDL::instance().mouseWindowPos;
}

bool graphics::Mouse::isButtonDown(Button button) {
	unsigned char _button = (unsigned char)button;
	_button = (1u << _button);
	
	return (SDL::instance().mouseButtons & _button) == _button;
}

bool graphics::Mouse::clicked(Button button) {
	unsigned char _button = (unsigned char)button;
	_button = (1u << _button);
	
	return ((SDL::instance().mouseButtons & _button) == _button) && !((SDL::instance().prevMouseButtons & _button) == _button);
}

bool graphics::Mouse::released(Button button) {
	unsigned char _button = (unsigned char)button;
	_button = (1u << _button);

	return (!((SDL::instance().mouseButtons & _button) == _button)) && ((SDL::instance().prevMouseButtons & _button) == _button);
}

vec2i graphics::Mouse::motion() {
	return SDL::instance().mousePos - SDL::instance().prevMousePos;
}

bool graphics::Mouse::didMove() {
	return SDL::instance().mousePos != SDL::instance().prevMousePos;
}

float graphics::Mouse::wheel() {
	return SDL::instance().mouseScrollAmount;
}

void graphics::Mouse::setPosition(const vec2i& position) {
	SDL_WarpMouseGlobal(position.x, position.y);
}

//
//vec2 graphics::Window::getMouseMotion() const {
//	return SDL::mousePosition() - SDL::prevMousePosition();
//}
//
//vec2 graphics::Window::getMouseMotionNds() const {
//	return transformToNds(SDL::mousePosition(), impl->m_width, impl->m_height) 
//		- transformToNds(SDL::prevMousePosition(), impl->m_width, impl->m_height);
//}
//
//bool graphics::Window::mouseMoved() const {
//	return SDL::mousePosition() != SDL::prevMousePosition();
//}
//
//bool graphics::Window::mouseClicked(MouseButton button) const {
//	unsigned char _button = (unsigned char)button;
//	_button = (1u << _button);
//
//	return ((impl->m_mouseButtons & _button) == _button) && !((impl->m_prevMouseButtons & _button) == _button);
//}
//
//bool Window::isMouseButtonDown(MouseButton button) const {
//	unsigned char _button = (unsigned char)button;
//	_button = (1u << _button);
//
//	return (impl->m_mouseButtons & _button) == _button;
//}
//
//float graphics::Window::getMouseWheel() const {
//	return impl->m_mouseScrollAmount;
//}
//
//void graphics::Window::setBackfaceCulling(bool isEnabled) const {
//	if (isEnabled)
//		glEnable(GL_CULL_FACE);
//	else
//		glDisable(GL_CULL_FACE);
//}

Viewport& graphics::Window::viewport() {
	return impl->m_viewport;
}

void graphics::Window::setBorder(bool isEnabled) const {
	SDL_SetWindowBordered(impl->window, SDL_bool(isEnabled));
}

void graphics::Window::setTittlebarTheme(TittlebarTheme theme) {
	BOOL USE_DARK_MODE = (bool)theme;
	BOOL SET_IMMERSIVE_DARK_MODE_SUCCESS = SUCCEEDED(DwmSetWindowAttribute(
		impl->m_windowsWindowHandle, DWMWINDOWATTRIBUTE::DWMWA_USE_IMMERSIVE_DARK_MODE,
		&USE_DARK_MODE, sizeof(USE_DARK_MODE)));

	ShowWindow(impl->m_windowsWindowHandle, SW_MINIMIZE);
	ShowWindow(impl->m_windowsWindowHandle, SW_RESTORE);
}

Window::~Window() { }

bool graphics::isKeyDown(Key key) {
	return SDL::instance().keyboardState.data[(uint8_t)key];
}

bool graphics::isKeyUp(Key key) {
	return !SDL::instance().keyboardState.data[(uint8_t)key];
}

bool graphics::keyPressed(Key key) {
	return SDL::instance().keyboardState.data[(uint8_t)key] && !SDL::instance().keyboardState.prevData[(uint8_t)key];
}

bool graphics::keyReleased(Key key) {
	return !SDL::instance().keyboardState.data[(uint8_t)key] && SDL::instance().keyboardState.prevData[(uint8_t)key];
}

bool graphics::isModPressed(KeyMod mod) {
	return (SDL::instance().keyboardState.mod & (uint8_t)mod);
}

uint8_t graphics::getMods() {
	return SDL::instance().keyboardState.mod;
}

unsigned long graphics::getTicks() {
	return SDL::getTicks();
}
