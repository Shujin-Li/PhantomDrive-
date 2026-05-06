#pragma once

#ifdef PHANTOMDRIVE_STATIC_DEFINE
#   define PHANTOMDRIVE_EXPORT
#else
#   ifdef PhantomDrive_EXPORTS
#       define PHANTOMDRIVE_EXPORT __attribute__((visibility("default")))
#   else
#       define PHANTOMDRIVE_EXPORT __attribute__((visibility("default")))
#   endif
#endif

#define PHANTOMDRIVE_NO_EXPORT
