#ifndef INTERVALRECORD_H
#define INTERVALRECORD_H

#include <QDateTime>

/*
 * The type of study interval
*/
enum class IntervalType {
    FOCUS,
    SHORT_BREAK,
    LONG_BREAK
};

 /*
  * Interval Metadata
    */
struct IntervalRecord {
    int intervalId;
    IntervalType type;
    int durationMinutes;
    QDateTime startTime;
    QDateTime endTime;
};

#endif // INTERVALRECORD_H
