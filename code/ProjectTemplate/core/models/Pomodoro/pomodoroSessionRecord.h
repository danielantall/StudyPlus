#ifndef SESSIONRECORD_H
#define SESSIONRECORD_H

#include "intervalRecord.h"
#include <QDateTime>
#include <vector>
#include <string>

struct SessionRecord {
    int sessionId;
    std::string taskId;
    QDateTime startTime;
    QDateTime endTime;
    std::vector<IntervalRecord> intervals;  // All intervals in this session


    int getTotalFocusMinutes() const {
        int total = 0;
        for (const auto& interval : intervals) {
            if (interval.type == IntervalType::FOCUS) {
                total += interval.durationMinutes;
            }
        }
        return total;
    }

    int getIntervalCount() const {
        return intervals.size();
    }
};

#endif // SESSIONRECORD_H
