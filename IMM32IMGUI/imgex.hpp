#pragma once
#if !defined( imgex_hpp_HEADER_GUARD_7556619d_62b7_4f3b_b364_f02af36a3bbc )
#define imgex_hpp_HEADER_GUARD_7556619d_62b7_4f3b_b364_f02af36a3bbc 1

#if defined( _WIN32 )
# include <tchar.h>
# include <Windows.h>
#endif /* defined( _WIN32 ) */

#include "imgui.h"

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

  static inline constexpr const TCHAR* imm_associate_property_name()
  {
    return TEXT("IMM32-InputContext-3bd72cfe-c271-4071-a440-1677a5057572");
  }
  inline bool imm_associate_context( HWND const hWnd , bool const value )
  {
    if( IsWindow( hWnd ) ){
      HIMC const hImc = reinterpret_cast<HIMC>(GetProp( hWnd, imm_associate_property_name() ));
      bool const associate_status = (( nullptr == hImc ) ? true : false);
      if( !( associate_status == value )){
        if( value ){
          if( hImc ){
            ImmAssociateContext( hWnd , hImc );
            VERIFY( SetProp( hWnd , imm_associate_property_name() , nullptr ) );
          }
        }else{
          HIMC const storeContext = ImmAssociateContext( hWnd , nullptr );
          if( storeContext ){
            VERIFY( SetProp( hWnd , imm_associate_property_name() , storeContext ) );
          }
        }
      }
    }
    return TRUE;
  }
  inline bool imm_associate_context_cleanup( HWND const hWnd ){
    if( IsWindow( hWnd )){
      HIMC const hImc = reinterpret_cast<HIMC>( RemoveProp( hWnd , imm_associate_property_name() ) );
      if( hImc ){
        VERIFY( nullptr == ImmAssociateContext( hWnd, hImc ) );
      }
    }
    return true;
  }
}

#endif /* defined( __cplusplus ) */
#endif /* !defined( imgex_hpp_HEADER_GUARD_7556619d_62b7_4f3b_b364_f02af36a3bbc ) */
