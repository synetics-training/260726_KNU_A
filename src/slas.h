/**
 * @file slas.h
 * @brief SLAS (Speed Limit Assist System) 소프트웨어 모듈 헤더
 *
 * 전방 카메라 및 내비게이션 정보로부터 제한속도 정보를 수신하고,
 * 운전자에게 제한속도 정보와 속도 상태를 표시하는 운전자 보조 기능.
 *
 * @version 1.0
 */

#ifndef SLAS_H
#define SLAS_H

#include <stdio.h>
#include <string.h>

/* ── 상수 정의 ── */

/** SLAS 운용 모드 */
#define SLAS_MODE_OFF       0
#define SLAS_MODE_WARNING   1
#define SLAS_MODE_ASSIST    2

/** 전방 카메라 상태 */
#define CAM_NORMAL          0
#define CAM_BLOCKED         1
#define CAM_LOW_CONFIDENCE  2

/** 표지판 타입 */
#define SIGN_SPEED_LIMIT    0
#define SIGN_MIN_SPEED      1
#define SIGN_UNKNOWN        2

/** 도로 컨텍스트 */
#define ROAD_MAIN           0
#define ROAD_EXIT           1
#define ROAD_TUNNEL         2
#define ROAD_NEW            3

/** SLAS 보조 표시 색상 */
#define COLOR_OFF           0
#define COLOR_GRAY          1
#define COLOR_GREEN         2
#define COLOR_AMBER         3
#define COLOR_RED           4

/** 설정속도 변경 안내 */
#define GUIDE_NONE          0
#define GUIDE_UP            1
#define GUIDE_DOWN          2

/** 시스템 상태 */
#define STATE_NORMAL        0
#define STATE_OFF           1
#define STATE_LIMITED       2
#define STATE_INITIALIZING  3
#define STATE_INVALID       4

/** 적용 제한속도 출처 */
#define SOURCE_NONE         0
#define SOURCE_CAMERA       1
#define SOURCE_NAVIGATION   2

/** 기능 제한 메시지 */
#define MSG_NONE            0
#define MSG_INITIALIZING    1
#define MSG_CAMERA_OBSCURED 2
#define MSG_INVALID_INPUT   3

/* 초기화 판정 기준 (초) */
#define INIT_TIME_THRESHOLD 15

/* 제한속도 유효 범위 */
#define SPEED_LIMIT_MIN     30
#define SPEED_LIMIT_MAX     130

/* 차량 속도 유효 범위 */
#define VEHICLE_SPEED_MIN   0
#define VEHICLE_SPEED_MAX   250

/* Offset 유효 범위 */
#define OFFSET_MIN          (-10)
#define OFFSET_MAX          10

/* ── 구조체 정의 ── */

/**
 * @brief SLAS 입력 구조체
 */
typedef struct {
    int mode;                   /**< SLAS 운용 모드 */
    int offsetKph;              /**< 속도 보정값 Offset (km/h) */
    int setSpeedKph;            /**< 현재 ACC/MSLA 설정속도 (km/h) */
    int vehicleSpeedKph;        /**< 차량 속도 (km/h) */
    int cameraStatus;           /**< 전방 카메라 상태 */
    int cameraSpeedLimitKph;    /**< 카메라 인식 제한속도 (km/h) */
    int navSpeedLimitKph;       /**< 내비게이션 제한속도 (km/h) */
    int signType;               /**< 표지판 타입 */
    int roadContext;            /**< 도로 컨텍스트 */
    int multipleSigns;          /**< 복수 표지판 감지 여부 (0/1) */
    int exitRoadSpeedLimitKph;  /**< 출구도로 제한속도 (km/h) */
    int cameraInitializing;     /**< 카메라 초기화 중 여부 (0/1) */
    int ignitionElapsedSec;     /**< 시동 후 경과 시간 (초) */
} SLASInput;

/**
 * @brief 제한속도 선택 결과 구조체
 */
typedef struct {
    int valid;              /**< 유효한 제한속도 선택 여부 */
    int appliedLimitKph;    /**< 적용 제한속도 (km/h) */
    int sourceUsed;         /**< 적용 제한속도 출처 */
} SpeedSelection;

/**
 * @brief SLAS 판정 결과 구조체
 */
typedef struct {
    int systemState;            /**< 시스템 상태 */
    int baseLimitKph;           /**< 기본 제한속도 정보 (0이면 "---") */
    int assistDisplayColor;     /**< SLAS 보조 표시 색상 */
    int appliedLimitKph;        /**< 적용 제한속도 (km/h) */
    int overspeedWarning;       /**< 과속 경고 (0/1) */
    int setSpeedChangeGuide;    /**< 설정속도 변경 안내 */
    int limitationMessage;      /**< 기능 제한 메시지 */
    int sourceUsed;             /**< 적용 제한속도 출처 */
} SLASResult;

/* ── 함수 선언 ── */

/**
 * @brief 기본 입력값을 반환한다.
 * @return 기본 조건이 설정된 SLASInput 구조체
 */
SLASInput default_input(void);

/**
 * @brief 제한속도 값이 유효 범위인지 확인한다.
 * @param value 제한속도 값
 * @return 유효하면 1, 아니면 0
 */
int is_valid_speed_limit(int value);

/**
 * @brief 입력값 유효성을 검증한다.
 * @param in 입력 구조체 포인터
 * @return 유효하면 1, 아니면 0
 */
int validate(const SLASInput *in);

/**
 * 제한속도 선택 정책에 따라 적용 제한속도를 결정한다.
 * @param in 입력 구조체 포인터
 * @return 제한속도 선택 결과
 */
SpeedSelection select_speed_limit(const SLASInput *in);

/**
 * @brief SLAS 판정을 수행한다.
 * @param in 입력 구조체 포인터
 * @param previousDisplayedLimit 이전 표시 제한속도 (없으면 0)
 * @return 판정 결과 구조체
 */
SLASResult evaluate(const SLASInput *in, int previousDisplayedLimit);

#endif /* SLAS_H */
