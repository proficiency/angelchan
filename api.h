#pragma once

#define _4CH_CHN_URL "http://boards.4channel.org/"
#define _4CH_NRM_URL "http://boards.4chan.org/"
#define _4CH_API_URL "http://a.4cdn.org/"
#define _4CH_CDN_URL "http://i.4cdn.org/"

using namespace nlohmann;

// metadata for a download
class c_media
{
	// board media originated from
	std::string m_board;

	// string media originated from
	std::string m_thread;

	u8a m_data;
};

static u32 write_function( char* data, u32 size, u32 nmemb, u8a* writedata )
{
	const u32 len = size * nmemb;

	for ( u32 i = 0; i < len; ++i )
		writedata->push_back( data[i] );
	
	return len;
}

class c_networking
{
public:
	inline void setup( )
	{
		if ( curl_global_init( CURL_GLOBAL_DEFAULT ) == CURLE_FAILED_INIT )
		{
			printf( "$> failed to init curl\n" );
			return;
		}

		m_curl = curl_easy_init( );

		if ( !m_curl )
		{
			printf( "$> failed to init curl\n" );
			return;
		}

		curl_easy_setopt( m_curl, CURLOPT_WRITEFUNCTION, write_function );
		curl_easy_setopt( m_curl, CURLOPT_PROTOCOLS, CURLPROTO_HTTP | CURLPROTO_HTTPS );
		//curl_easy_setopt( m_curl, CURLOPT_POSTREDIR, CURL_REDIR_GET_ALL );
	}

	inline void finish( )
	{
		curl_easy_cleanup( m_curl );
	}

	inline int load_webpage( std::string_view url, u8a* data )
	{
		int response = 0;

		if ( !data->empty( ) )
			data->clear( );

		curl_slist* headers{};
		headers = curl_slist_append( headers, "User-Agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/74.0.3724.8 Safari/537.36" );
		curl_easy_setopt( m_curl, CURLOPT_WRITEDATA, data );
		curl_easy_setopt( m_curl, CURLOPT_URL, url.data( ) );
		curl_easy_setopt( m_curl, CURLOPT_HTTPHEADER, headers );
		curl_easy_perform( m_curl );
		curl_easy_getinfo( m_curl, CURLINFO_RESPONSE_CODE, &response );

		return response;
	}

	bool download_file( std::string_view location, std::string_view url )
	{
		u8a data{};

		// does the file already exist?
		{
			std::ifstream file( location.data( ) );
			if ( file.good( ) )
			{
				file.close( );
				return false;
			}
		}

		load_webpage( url, &data );
		if ( !data.empty( ) )
		{
			std::ofstream media( location.data( ), std::ios::out | std::ios::binary );
			media.write( ( char* ) data.data( ), data.size( ) );
			media.close( );

			return true;
		}

		return false;
	}

private:
	CURL*						m_curl;
	curl_slist*					m_headers;
};

inline c_networking g_networking;

class c_download
{
public:
	std::string m_path;
	std::string m_url;
	json		m_post;

	void download( )
	{
		if ( g_networking.download_file( m_path, m_url ) )
			printf( "%s(%ikb) %s\n", m_post["filename"].get< std::string >( ).c_str( ), m_post["fsize"].get< u32 >( ) / 1024, m_url.c_str( ) );

		else
			printf( "failed %s\n", m_url.c_str( ) );
	}
};

class c_thread
{
public:
	c_thread( std::string_view url ) : m_url( url ) { }

	inline static bool post_containsfile( const json& post )
	{
		if ( post.find( "filename" ) == post.end( ) || post.find( "fsize" ) == post.end( ) || post.find( "tim" ) == post.end( ) || post.find( "ext" ) == post.end( ) )
			return false;

		return true;
	}
	
	// returns each post within thread
	std::vector< c_download > parse( )
	{
		auto resolve_board = []( std::string url ) -> std::string
		{
			std::string board;

			for ( u32 i = 0; i < url.length( ); ++i )
			{
				if ( url[i] != '/' || !std::isalpha( url[i - 1] ) )
					continue;

				const std::string substring = url.substr( i + 1 );

				u32 pos = substring.find_first_of( '/' );
				if ( pos != std::string::npos )
					board = substring.substr( 0, pos );

				break;
			}

			if ( board.empty( ) )
			{
				printf( "%s not supported\n", url.c_str( ) );
				return {};
			}

			return board;
		};

		u8a webpage;

		// no ssl support
		if ( fnv32::hash( m_url.substr( 0, 5 ) ) == fnvc( "https" ) )
			m_url.erase( m_url.begin( ) + 4 );

		// valid board/ site?
		const std::string board = resolve_board( m_url );
		if ( board.empty( ) || m_url.find( "/thread/" ) == std::string::npos )
			return {};

		// api query url
		{
			if ( m_url.find( _4CH_NRM_URL ) != std::string::npos )
				m_url.replace( m_url.begin( ), m_url.begin( ) + strlen_ct( _4CH_NRM_URL ), _4CH_API_URL );

			if ( m_url.find( _4CH_CHN_URL ) != std::string::npos )
				m_url.replace( m_url.begin( ), m_url.begin( ) + strlen_ct( _4CH_CHN_URL ), _4CH_API_URL );
		}

		// get json response
		g_networking.load_webpage( m_url + ".json", &webpage );

		// did we fail?
		if ( webpage.empty( ) )
		{
			printf( "$> failed to open %s\n", m_url.c_str( ) );
			return {};
		}

		const json			thread			= json::parse( std::string( ( char* ) webpage.data( ), webpage.size( ) ) )["posts"];
		const json			op				= thread.front( );
		std::string			directory_name	= op["semantic_url"].get< std::string >( ) + '_' + std::to_string( op["no"].get< u32 >( ) );

		std::experimental::filesystem::create_directory( directory_name );

		directory_name.push_back( '/' );

		std::ofstream info( directory_name + "info.txt" );

		info << "url: " << m_url << std::endl;
		info << "board: " << board << std::endl;
		info << "time: " << op["now"].get< std::string >( ) << std::endl;
		info << "poster: " << op["name"].get< std::string >( ) << std::endl;

		if ( op.find( "sub" ) != op.end( ) )
			info << "title: " << op["sub"].get< std::string >( ) << std::endl;

		info << "reply count: " << op["replies"].get< u32 >( ) << std::endl;
		info << "image count: " << op["images"].get< u32 >( ) << std::endl;
		info << "last reply time:" << thread.back( )["now"].get< std::string >( ) << std::endl;

		info.close( );

		for ( auto& post : thread )
		{
			// we may wanna use this later in the chain so we can save each post's comment, but for now, we're only downloading files, so we'll filter
			// away any post that doesn't contain a file
			if ( !post_containsfile( post ) )
				continue;

			const std::string filename = post["tim"].dump( ) + post["ext"].get< std::string >( );
			m_downloads.push_back( { directory_name + filename, std::string( _4CH_CDN_URL ) + board + '/' + filename, post } );
		}

		return m_downloads;
	}

private:
	std::string					m_url;
	std::vector< c_download >	m_downloads;
};

class c_parser
{
public:
	inline void add_thread( std::string_view url )
	{
		m_threads.push_back( c_thread( url ) );

		const std::vector< c_download > posts =  m_threads.back( ).parse( );

		for ( auto& post : posts )
			m_queue.push_back( post );
	}
	
	inline void download( )
	{
		for ( u32 i = 0; i < m_queue.size( ); ++i )
		{
			auto post = m_queue[i];

			post.download( );
			m_queue.erase( m_queue.begin( ) + i-- );
		}
	}

	inline std::vector< c_download > queue( )
	{
		return m_queue;
	}

private:
	std::vector< c_thread >		m_threads;
	std::vector< c_download >	m_queue;
};
inline c_parser g_parser;