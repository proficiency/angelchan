#include "context.h"

int main( )
{
#ifdef _WIN32
	FreeConsole( );
#endif

	g_networking.setup( );

	if ( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER ) != 0 )
	{
		printf( "Error: %s\n", SDL_GetError( ) );
		return -1;
	}

	// Decide GL+GLSL versions
#if __APPLE__
	// GL 3.2 Core + GLSL 150
	const char* glsl_version = "#version 150";
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG ); // Always required on Mac
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 3 );
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 2 );
#else
	// GL 3.0 + GLSL 130
	const char* glsl_version = "#version 130";
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_FLAGS, 0 );
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 3 );
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 0 );
#endif

	// Create window with graphics context
	SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
	SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 24 );
	SDL_GL_SetAttribute( SDL_GL_STENCIL_SIZE, 8 );
	SDL_WindowFlags window_flags = ( SDL_WindowFlags ) ( SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI );
	SDL_Window* window = SDL_CreateWindow( "angelchan", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, g_ui.m_screensize[0], g_ui.m_screensize[1], window_flags );
	SDL_GLContext gl_context = SDL_GL_CreateContext( window );
	SDL_GL_MakeCurrent( window, gl_context );
	SDL_GL_SetSwapInterval( 1 ); // Enable vsync

	// Initialize OpenGL loader
#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
	bool err = gl3wInit( ) != 0;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
	bool err = glewInit( ) != GLEW_OK;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
	bool err = gladLoadGL( ) == 0;
#else
	bool err = false; // If you use IMGUI_IMPL_OPENGL_LOADER_CUSTOM, your loader is likely to requires some form of initialization.
#endif
	if ( err )
	{
		fprintf( stderr, "Failed to initialize OpenGL loader!\n" );
		return 1;
	}

	IMGUI_CHECKVERSION( );
	ImGui::CreateContext( );

	ImGuiIO& io = ImGui::GetIO( );
	{
		ImGuiStyle* style = &ImGui::GetStyle( );

		io.Fonts->AddFontFromFileTTF( "c:\\Windows\\Fonts\\Verdana.ttf", 16.0f, 0, io.Fonts->GetGlyphRangesDefault( ) );
		io.Fonts->AddFontFromFileTTF( "c:\\Windows\\Fonts\\Arial.ttf", 16.0f, 0, io.Fonts->GetGlyphRangesDefault( ) );
		io.Fonts->AddFontFromFileTTF( "c:\\Windows\\Fonts\\Tahoma.ttf", 16.0f, 0, io.Fonts->GetGlyphRangesDefault( ) );
		io.Fonts->AddFontFromFileTTF( "c:\\Windows\\Fonts\\comic.ttf", 24.0f, 0, io.Fonts->GetGlyphRangesDefault( ) );

		style->WindowBorderSize = 0.0f;
		style->WindowPadding = ImVec2( 15, 15 );
		style->WindowRounding = 0.0f;
		style->FramePadding = ImVec2( 5, 5 );
		style->FrameRounding = 5.0f;
		style->ItemSpacing = ImVec2( 3, 8 );
		style->ItemInnerSpacing = ImVec2( 8, 6 );
		style->IndentSpacing = 25.0f;
		style->ScrollbarSize = 15.0f;
		style->ScrollbarRounding = 9.0f;
		style->GrabMinSize = 5.0f;
		style->GrabRounding = 3.0f;

		ImGui::SetMouseCursor( ImGuiMouseCursor_Arrow );

		ImGui::StyleColorsDark( );
	}

	// Setup Platform/Renderer bindings
	ImGui_ImplSDL2_InitForOpenGL( window, gl_context );
	ImGui_ImplOpenGL3_Init( glsl_version );

	ImVec4 clear_color = ImVec4( 0.45f, 0.55f, 0.60f, 1.00f );

	// Main loop
	bool done = false;
	while ( !done )
	{
		// Poll and handle events (inputs, window resize, etc.)
		// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
		// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
		// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
		// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
		SDL_Event event;
		while ( SDL_PollEvent( &event ) )
		{
			ImGui_ImplSDL2_ProcessEvent( &event );
			if ( event.type == SDL_QUIT )
				done = true;
			if ( event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID( window ) )
				done = true;

			switch ( event.type )
			{
				case SDL_QUIT:
				{
					done = true;
					break;
				}

				case SDL_WINDOWEVENT:
				{
					switch ( event.window.event )
					{
						case SDL_WINDOWEVENT_CLOSE:
						{
							if ( event.window.windowID == SDL_GetWindowID( window ) )
								done = true;

							break;
						}

						/*case SDL_WINDOWEVENT_SIZE_CHANGED:
						{
							if ( event.window.windowID == SDL_GetWindowID( window ) )
							{
								g_ui.m_screensize[0] = event.window.data1;
								g_ui.m_screensize[1] = event.window.data2;
							}

							break;
						}*/
					}

					break;
				}
			}
		}

		// Start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame( );
		ImGui_ImplSDL2_NewFrame( window );
		ImGui::NewFrame( );

		g_ui.draw( );

		// Rendering
		ImGui::Render( );
		glViewport( 0, 0, ( int ) io.DisplaySize.x, ( int ) io.DisplaySize.y );
		glClearColor( clear_color.x, clear_color.y, clear_color.z, clear_color.w );
		glClear( GL_COLOR_BUFFER_BIT );
		ImGui_ImplOpenGL3_RenderDrawData( ImGui::GetDrawData( ) );
		SDL_GL_SwapWindow( window );
	}

	// Cleanup
	ImGui_ImplOpenGL3_Shutdown( );
	ImGui_ImplSDL2_Shutdown( );
	ImGui::DestroyContext( );

	SDL_GL_DeleteContext( gl_context );
	SDL_DestroyWindow( window );
	SDL_Quit( );

	return 0;
}