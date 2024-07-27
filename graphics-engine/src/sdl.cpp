#include "pch.h"
#include "window.h"
#include "primitive_drawer.h"
#include "graphics_headers.h"
#include "camera.h"
#include "keyboard.h"
#include "sdl.h"
#include "object.h"
#include "imgui.h"

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
	static void init() {
		if (instance().m_inited)
			return;

		// Init SDL
		SDL_Init(SDL_INIT_EVERYTHING);

		// Use OpenGL 3.3
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

		instance().m_keyboardState.init();

		instance().m_inited = true;
	}

	static void handleEvent(SDL_Event event) {

	}

	static void updateMouseState() {
		SDL_GetMouseState(&instance().m_mousePos.x, &instance().m_mousePos.y);
	}

	static void increaseTickCount() {
		++instance().m_tickCount;
	}

	static vec2i& mousePosition() {
		return instance().m_mousePos;
	}

	static vec2i& prevMousePosition() {
		return instance().m_prevMousePos;
	}

	static KeyboardState& keyboardState() {
		return instance().m_keyboardState;
	}

	static unsigned long getTicks() {
		return instance().m_tickCount;
	}

private:
	bool m_inited = false;

	vec2i m_mousePos;
	vec2i m_prevMousePos;

	KeyboardState	m_keyboardState{};

	unsigned long	m_tickCount = 0;

	static SDL& instance() {
		static SDL c_instance;
		return c_instance;
	}
};

struct Window::Impl {
	SDL_Window*		window;
	SDL_GLContext	glContext;
	bool			shouldQuit = false;
	bool			isOpen;

	Uint32			m_windowID;

	Uint8			m_mouseButtons = 0;
	Uint8			m_prevMouseButtons = 0;
	float			m_mouseScrollAmount = 0;

	unsigned		m_width;
	unsigned		m_height;
	bool			m_didResize = false;

	unsigned long   m_tickCount = 0;

	bool			m_isImGuiInited = false;

	Impl(unsigned width, unsigned height)
		: m_width(width)
		, m_height(height)
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
		}

		++m_tickCount;
		SDL::keyboardState().update();

		// Swap buffers
		SDL_GL_SwapWindow(window);

		// Clear buffer
		glClearColor(.2f, .2f, .2f, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void handleEvents() {
		// Update mouse button and scroll state
		m_prevMouseButtons = m_mouseButtons;
		SDL::prevMousePosition() = SDL::mousePosition();
		m_mouseScrollAmount = 0;

		// Handle window resize
		int newWidth, newHeight;
		SDL_GetWindowSize(window, &newWidth, &newHeight);
		m_didResize = (newWidth != m_width || newHeight != m_height);
		if (m_didResize) {
			m_width = newWidth;
			m_height = newHeight;
			
			debug::camera->setAspectRatio(m_width, m_height);
			glViewport(0, 0, m_width, m_height);
		}

		SDL::updateMouseState();
		SDL::increaseTickCount();

		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			// Pass event to ImGui
			if (m_isImGuiInited)
				ImGui_ImplSDL2_ProcessEvent(&event);

			SDL::handleEvent(event);

			switch (event.type)
			{
			case SDL_QUIT: {
				shouldQuit = true;
			} break;
			case SDL_MOUSEBUTTONDOWN: {
				if (m_windowID == event.button.windowID)
					m_mouseButtons |= (1u << event.button.button);
				SDL::keyboardState().setMod(SDL_GetModState());
			} break;
			case SDL_MOUSEBUTTONUP: {
				if (m_windowID == event.button.windowID)
					m_mouseButtons &= ~(1u << event.button.button);
				SDL::keyboardState().setMod(SDL_GetModState());
			} break;
			case SDL_MOUSEWHEEL:
				if (m_windowID == event.button.windowID)
					m_mouseScrollAmount = event.wheel.preciseY;
				break;
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
	SDL::init();

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

	Mesh::Shaders::matt.bindUniform("VP", debug::camera);
	Mesh::Shaders::wireframe.bindUniform("VP", debug::camera);

	Object::Shaders::matt.bindUniform("VP", debug::camera);
	Object::Shaders::normal.bindUniform("VP", debug::camera);
	Object::Shaders::textured.bindUniform("VP", debug::camera);
	Object::Shaders::wireframe.bindUniform("VP", debug::camera);

	impl->isOpen = true;
}

void graphics::Window::initImGui() {
	ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	ImGui::StyleColorsClassic();

	ImGui_ImplSDL2_InitForOpenGL(impl->window, impl->glContext);
	ImGui_ImplOpenGL3_Init("#version 330");

	impl->m_isImGuiInited = true;

	// Begin first imgui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame();
	ImGui::NewFrame();
	ImGui::DockSpaceOverViewport((const ImGuiViewport*)0, ImGuiDockNodeFlags_PassthruCentralNode);
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

vec2 graphics::Window::getMousePosition() const {
	return transformToNds(SDL::mousePosition(), impl->m_width, impl->m_height);
}

vec2 graphics::Window::getMouseMotion() const {
	return SDL::mousePosition() - SDL::prevMousePosition();
}

vec2 graphics::Window::getMouseMotionNds() const {
	return transformToNds(SDL::mousePosition(), impl->m_width, impl->m_height) 
		- transformToNds(SDL::prevMousePosition(), impl->m_width, impl->m_height);
}

bool graphics::Window::mouseMoved() const {
	return SDL::mousePosition() != SDL::prevMousePosition();
}

bool graphics::Window::mouseClicked(MouseButton button) const {
	unsigned char _button = (unsigned char)button;
	_button = (1u << _button);

	return ((impl->m_mouseButtons & _button) == _button) && !((impl->m_prevMouseButtons & _button) == _button);
}

bool Window::isMouseButtonDown(MouseButton button) const {
	unsigned char _button = (unsigned char)button;
	_button = (1u << _button);

	return (impl->m_mouseButtons & _button) == _button;
}

float graphics::Window::getMouseWheel() const {
	return impl->m_mouseScrollAmount;
}

Window::~Window() { }

bool graphics::isKeyDown(Key key) {
	return SDL::keyboardState().data[(uint8_t)key];
}

bool graphics::isKeyUp(Key key) {
	return !SDL::keyboardState().data[(uint8_t)key];
}

bool graphics::keyPressed(Key key) {
	return SDL::keyboardState().data[(uint8_t)key] && !SDL::keyboardState().prevData[(uint8_t)key];
}

bool graphics::keyReleased(Key key) {
	return !SDL::keyboardState().data[(uint8_t)key] && SDL::keyboardState().prevData[(uint8_t)key];
}

bool graphics::isModPressed(KeyMod mod) {
	return (SDL::keyboardState().mod & (uint8_t)mod);
}

unsigned long graphics::getTicks() {
	return SDL::getTicks();
}
