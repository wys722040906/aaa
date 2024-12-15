#ifndef TYPES_H
#define TYPES_H

#include <QString>
#include <QtGlobal>  // for qint64

struct PassengerRequest {
    int currentFloor;    // 当前楼层
    int targetFloor;     // 目标楼层
    int passengerCount;  // 乘客数量
    qint64 timestamp;   // 请求时间戳
};

#endif // TYPES_H 