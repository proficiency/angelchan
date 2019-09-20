#pragma once
class c_ui
{
public:
	void draw( )
	{
		ImGui::SetNextWindowPos( ImVec2( -3, 0 ) );
		ImGui::SetNextWindowSize( ImVec2( m_screensize[0] + 3, m_screensize[1] ) );

		ImGui::Begin( "##0", 0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings );
		{
			static char thread[512]		= {};
			static bool downloading		= false;
			const float item_width		= m_screensize[0] - 24.0f;

			if ( !downloading )
			{
				ImGui::Text( "thread" );
				ImGui::Separator( );

				ImGui::PushItemWidth( item_width );
				if ( ImGui::InputText( "", thread, sizeof( thread ), ImGuiInputTextFlags_EnterReturnsTrue ) && strlen( thread ) )
					g_parser.add_thread( thread );
			}

			const std::vector< c_download > queue = g_parser.queue( );

			if ( !queue.empty( ) )
			{
				ImGui::Text( ( "queue (" + std::to_string( queue.size( ) ) + ')' ).c_str( ) );

				ImGui::PushItemWidth( item_width );
				if ( ImGui::ListBoxHeader( "", queue.size( ), 13 ) )
				{
					for ( auto& entry : queue )
					{
						if ( entry.m_post.type_name( ) && c_thread::post_containsfile( entry.m_post ) )
							ImGui::Text( ( entry.m_post["tim"].dump( ) + entry.m_post["ext"].get< std::string >( ) ).c_str( ) );
					}
				}

				ImGui::ListBoxFooter( );
			}

			if ( !queue.empty( ) && !downloading && ImGui::Button( "download", ImVec2( item_width, 26.0f ) ) )
			{
				std::thread( &c_parser::download, std::move( &g_parser ) ).detach( );
				memset( thread, 0, sizeof( thread ) );

				downloading = true;
			}

			if ( queue.empty( ) )
				downloading = false;

			ImGui::End( );
		}
	}

	u32 m_screensize[2] = { 640, 480 };
};

inline c_ui g_ui;