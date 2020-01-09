#pragma once
#if !defined( imgex_hpp_HEADER_GUARD_7556619d_62b7_4f3b_b364_f02af36a3bbc )
#define imgex_hpp_HEADER_GUARD_7556619d_62b7_4f3b_b364_f02af36a3bbc 1

#if defined( _WIN32 )
#include <tchar.h>
#include <Windows.h>
#endif /* defined( _WIN32 ) */

#include "imgui.h"

#if defined( __cplusplus )
#include <cassert>
#else /* defined( __cplusplus ) */
#include <assert.h>
#endif /* defined( __cplusplus ) */

#if !defined( VERIFY_ASSERT )
#if defined(IM_ASSERT)
#define VERIFY_ASSERT( exp ) IM_ASSERT( exp )
#else /* defined(IM_ASSERT) */
#define VERIFY_ASSERT( exp ) assert( exp )
#endif /* defined(IM_ASSERT) */
#endif /* !defined( VERIFY_ASSERT ) */

#if !defined( VERIFY )
#if defined( NDEBUG )
#define VERIFY( exp ) do{ exp ; }while(0) 
#else /* defined( NDEBUG ) */
#define VERIFY( exp ) VERIFY_ASSERT( exp )
#endif /* defined( NDEBUG ) */
#endif /* !defined( VERIFY ) */

#if defined( __cplusplus )

namespace imgex {
  // composition of flags
  namespace implements {
    template<typename first_t>
    constexpr inline unsigned int
      composite_flags_0 (first_t first)
    {
      return static_cast<unsigned int>(first);
    }
    template<typename first_t, typename ... tail_t>
    constexpr inline unsigned int
      composite_flags_0 (first_t first, tail_t... tail)
    {
      return static_cast<unsigned int>(first) | composite_flags_0 (tail...);
    }
  } // end of namespace implements;

  template<typename require_t, typename ... tail_t>
  constexpr inline require_t
    composite_flags (tail_t ... tail)
  {
    return static_cast<require_t>(implements::composite_flags_0 (tail ...));
  }

  static inline constexpr const TCHAR* imm_associate_property_name ()
  {
    return TEXT ("IMM32-InputContext-3bd72cfe-c271-4071-a440-1677a5057572");
  }

  inline bool imm_associate_context_enable (HWND const hWnd)
  {
    VERIFY_ASSERT (NULL != hWnd);
    VERIFY_ASSERT (IsWindow (hWnd));
    if (hWnd) {
      HIMC const hImc = reinterpret_cast<HIMC>(GetProp (hWnd, imm_associate_property_name ()));
      if (hImc) {
        VERIFY (NULL == ImmAssociateContext (hWnd, hImc));
        VERIFY (SetProp (hWnd, imm_associate_property_name (), nullptr));
      }
      return true;
    }
    return false;
  }

  inline bool imm_associate_context_disable (HWND const hWnd)
  {
    VERIFY_ASSERT (NULL != hWnd);
    VERIFY_ASSERT (IsWindow (hWnd));
    if (hWnd) {
      HIMC hImc = reinterpret_cast<HIMC>(GetProp (hWnd, imm_associate_property_name ()));
      if (!hImc) {
        VERIFY_ASSERT (NULL == hImc || !"window property dose not have HIMC");
        VERIFY (hImc = ImmAssociateContext (hWnd, nullptr));
        VERIFY (SetProp (hWnd, imm_associate_property_name (), hImc));
      }
      return true;
    }
    return false;
  }

#if 0
  inline bool imm_associate_context (HWND const hWnd, bool const value)
  {
    VERIFY_ASSERT (NULL != hWnd);
    VERIFY_ASSERT (IsWindow (hWnd));
    if (hWnd && IsWindow (hWnd)) {
      if (value) {
        return imm_associate_context_enable (hWnd);
      } else {
        return imm_associate_context_disable (hWnd);
      }
    }
    return false;
  }
#endif

  inline bool imm_associate_context_cleanup (HWND const hWnd) {
    if (IsWindow (hWnd)) {
      HIMC const hImc = reinterpret_cast<HIMC>(RemoveProp (hWnd, imm_associate_property_name ()));
      if (hImc) {
        VERIFY (nullptr == ImmAssociateContext (hWnd, hImc));
      }
    }
    return true;
  }
}

#endif /* defined( __cplusplus ) */
#endif /* !defined( imgex_hpp_HEADER_GUARD_7556619d_62b7_4f3b_b364_f02af36a3bbc ) */
