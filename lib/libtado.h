#ifndef LIBTADO_H
#define LIBTADO_H

#if defined BUILDING_TADO
 #define QTADO_EXPORT Q_DECL_EXPORT
#else
 #define QTADO_EXPORT Q_DECL_IMPORT
#endif

#endif // LIBTADO_H
