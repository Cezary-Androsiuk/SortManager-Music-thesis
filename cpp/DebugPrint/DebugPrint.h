#ifndef DEBUG_PRINT_H
#define DEBUG_PRINT_H

#include <QDebug>

#ifndef EC
// easier to set error code
#define EC(x) if(error_code != nullptr) *error_code = (x)
#endif

#define FUNCTION __PRETTY_FUNCTION__
// #define FUNCTION __FUNCTION__

#ifndef DB
// easier to trace execution path
#define DB qDebug() << qSetFieldWidth(30) << FUNCTION << qSetFieldWidth(0)
#endif


#ifndef WR
// easier to trace WARNING path
#define WR qWarning() << qSetPadChar('#') << qSetFieldWidth(30) \
<< QString(" ") + FUNCTION + " " << qSetPadChar(' ') << qSetFieldWidth(0)
#endif



#endif // DEBUG_PRINT_H
