#pragma once

#ifdef PHANTOMDRIVE_STATIC_DEFINE
#   define PHANTOMDRIVE_EXPORT
#else
#   ifdef PhantomDrive_EXPORTS
#       define PHANTOMDRIVE_EXPORT __declspec(dllexport)
#   else
#       define PHANTOMDRIVE_EXPORT __declspec(dllimport)
#   endif
#endif

#define PHANTOMDRIVE_NO_EXPORT
