#pragma once
#if !defined( imgex_hpp_HEADER_GUARD_7556619d_62b7_4f3b_b364_f02af36a3bbc )
#define imgex_hpp_HEADER_GUARD_7556619d_62b7_4f3b_b364_f02af36a3bbc 1

#if defined( __cplusplus )
# include <cassert>
#else /* defined( __cplusplus ) */
# include <assert.h>
#endif /* defined( __cplusplus ) */

#if !defined( VERIFY )
# if defined( NDEBUG )
#  define VERFIFY( exp ) do{ exp ; }while(0) 
# else /* defined( NDEBUG ) */
#  if !defined( VERIFY_ASSERT )
#   define VERIFY_ASSERT( exp ) assert( exp )
#  endif /* !defined( VERIFY_ASSERT ) */
#  define VERIFY( exp ) VERIFY_ASSERT( exp )
# endif /* defined( NDEBUG ) */
#endif /* !defined( VERIFY ) */

#if defined( __cplusplus )

namespace imgex{
  // composition of flags
  namespace implements{
    template<typename first_t>
    constexpr inline unsigned int
    composite_flags_0( first_t first )
    {
      return static_cast<unsigned int>( first );
    }
    template<typename first_t, typename ... tail_t>
    constexpr inline unsigned int
    composite_flags_0( first_t first, tail_t... tail)
    {
      return static_cast<unsigned int>( first ) | composite_flags_0( tail... );
    }
  }
  template<typename require_t,typename ... tail_t>
  constexpr inline require_t
  composite_flags( tail_t ... tail )
  {
    return static_cast<require_t>( implements::composite_flags_0( tail ... ) );
  }
  
}

#endif /* defined( __cplusplus ) */
#endif /* !defined( imgex_hpp_HEADER_GUARD_7556619d_62b7_4f3b_b364_f02af36a3bbc ) */
