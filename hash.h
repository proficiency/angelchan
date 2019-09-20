#pragma once

// https://en.cppreference.com/w/cpp/language/consteval soon..
#define fnvr( str ) ( fnv32::hash( str ) )
#define fnvc( str ) ( [&]( ){ constexpr u32 out = fnv32::hash( str ); return out; }( ) )

template< typename T = const char* >
inline constexpr u32 strlen_ct( T str )
{
	u32 out = 1;

	for ( ; str[out] != '\0'; ++out );

	return out;
}

namespace fnv32
{
	constexpr u32 fnv32_prime = 0x1000193;
	constexpr u32 fnv32_basis = 0x811c9dc5;

	template< typename t >
	inline constexpr u32 hash( const t& data, u32 len )
	{
		u32 out = fnv32_basis;
		for ( u32 i = 0; i < len; ++i )
			out = ( out ^ data[i] ) * fnv32_prime;

		return out;
	}

	inline constexpr u32 hash( const char* str )
	{
		const u32 len = strlen_ct( str );

		u32 out = fnv32_basis;
		for ( u32 i = 0; i < len; ++i )
			out = ( out ^ str[i] ) * fnv32_prime;

		return out;
	}

	template< typename T = const char* >
	inline u32 hash( T s )
	{
		if constexpr ( std::is_pointer_v< T > )
		{
			auto string = std::basic_string_view< std::remove_const_t< std::remove_pointer_t< T > > >( s );
			return hash( string.data( ), string.length( ) );
		}

		return hash( s.data( ), s.length( ) );
	}
}