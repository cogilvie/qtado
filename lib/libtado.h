#ifndef LIBTADO_H
#define LIBTADO_H

#if defined TEST
 #define QTADO_EXPORT Q_DECL_EXPORT
#else
 #define QTADO_EXPORT Q_DECL_IMPORT
#endif

#endif // LIBTADO_H
