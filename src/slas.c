/**
 * @file slas.c
 * @brief SLAS (Speed Limit Assist System) 판정 로직 구현
 *
 * 전방 카메라와 내비게이션으로부터 수신한 제한속도 정보를 기반으로
 * 운전자에게 속도 관련 경고 및 안내를 제공하는 시스템이다.
 *
 * 주요 처리 흐름:
 *   1. 입력값 유효성 검증 (validate)
 *   2. 제한속도 선택 정책에 따른 적용 제한속도 결정 (select_speed_limit)
 *   3. 과속 여부 판정 및 표시 색상/안내 결정 (evaluate)
 */

#include "slas.h"
#include <stdlib.h>

/**
 * @brief 테스트용 기본 입력값을 생성한다.
 *
 * WARNING 모드, 카메라 정상, 제한속도 80km/h의 일반 주행 조건을 반환한다.
 * 단위 테스트에서 기본 시나리오의 출발점으로 사용한다.
 */
SLASInput default_input(void)
{
    SLASInput in;
    in.mode = SLAS_MODE_WARNING;
    in.offsetKph = 0;
    in.setSpeedKph = 80;
    in.vehicleSpeedKph = 70;
    in.cameraStatus = CAM_NORMAL;
    in.cameraSpeedLimitKph = 80;
    in.navSpeedLimitKph = 80;
    in.signType = SIGN_SPEED_LIMIT;
    in.roadContext = ROAD_MAIN;
    in.multipleSigns = 0;
    in.exitRoadSpeedLimitKph = 0;
    in.cameraInitializing = 0;
    in.ignitionElapsedSec = 60;
    return in;
}

/**
 * @brief 제한속도 값이 유효 범위(30~130km/h)인지 확인한다.
 *
 * 0은 "제한속도 없음"을 의미하므로 유효한 값으로 취급한다.
 */
int is_valid_speed_limit(int value)
{
    return value == 0 || (value >= SPEED_LIMIT_MIN && value <= SPEED_LIMIT_MAX);
}

/**
 * @brief 입력 구조체의 각 필드가 유효 범위 내인지 검증한다.
 *
 * 차량 속도, Offset, 시동 경과 시간, 각종 제한속도 값을 순서대로 확인한다.
 * 하나라도 범위를 벗어나면 즉시 0(무효)을 반환한다.
 */
int validate(const SLASInput *in)
{
    if (in->vehicleSpeedKph < VEHICLE_SPEED_MIN ||
        in->vehicleSpeedKph > VEHICLE_SPEED_MAX)
        return 0;

    if (in->offsetKph < OFFSET_MIN || in->offsetKph > OFFSET_MAX)
        return 0;

    if (in->ignitionElapsedSec < 0)
        return 0;

    if (!is_valid_speed_limit(in->cameraSpeedLimitKph))
        return 0;
    if (!is_valid_speed_limit(in->navSpeedLimitKph))
        return 0;
    if (!is_valid_speed_limit(in->exitRoadSpeedLimitKph))
        return 0;
    if (!is_valid_speed_limit(in->setSpeedKph))
        return 0;

    return 1;
}

/**
 * @brief 제한속도 선택 정책에 따라 적용 제한속도를 결정한다.
 *
 * 카메라 상태와 표지판 정보를 기반으로 우선순위에 따라 제한속도를 선택한다.
 * 선택 우선순위:
 *   1. 카메라 BLOCKED → 제한속도 없음
 *   2. 복수 표지판 감지 → 출구도로 제한속도
 *   3. 카메라 정상 + 속도제한 표지판 → 카메라 인식 제한속도
 *   4. 카메라 신뢰도 낮음 → 내비게이션 제한속도로 대체
 *   5. 표지판 미인식 → 내비게이션 제한속도 사용
 */
SpeedSelection select_speed_limit(const SLASInput *in)
{
    SpeedSelection sel;
    sel.valid = 0;
    sel.appliedLimitKph = 0;
    sel.sourceUsed = SOURCE_NONE;

    /* 카메라 가려진 경우 제한속도를 결정할 수 없다 */
    if (in->cameraStatus == CAM_BLOCKED) {
        return sel;
    }

    /* 복수 표지판이 감지된 경우 출구도로 제한속도를 적용한다 */
    if (in->multipleSigns &&
        in->exitRoadSpeedLimitKph != 0 &&
        is_valid_speed_limit(in->exitRoadSpeedLimitKph)) {
        sel.valid = 1;
        sel.appliedLimitKph = in->exitRoadSpeedLimitKph;
        sel.sourceUsed = SOURCE_CAMERA;
        return sel;
    }

    /* 카메라가 정상이고 속도제한 표지판을 인식한 경우 */
    if (in->cameraStatus == CAM_NORMAL &&
        in->signType == SIGN_SPEED_LIMIT &&
        in->cameraSpeedLimitKph != 0 &&
        is_valid_speed_limit(in->cameraSpeedLimitKph)) {
        sel.valid = 1;
        sel.appliedLimitKph = in->cameraSpeedLimitKph;
        sel.sourceUsed = SOURCE_CAMERA;
        return sel;
    }

    /* 카메라 신뢰도가 낮은 경우 내비게이션 제한속도로 대체한다 */
    if (in->cameraStatus == CAM_LOW_CONFIDENCE &&
        in->navSpeedLimitKph != 0 &&
        is_valid_speed_limit(in->navSpeedLimitKph)) {
        sel.valid = 1;
        sel.appliedLimitKph = in->navSpeedLimitKph;
        sel.sourceUsed = SOURCE_CAMERA;
        return sel;
    }

    /* 표지판을 인식하지 못한 경우 내비게이션 제한속도를 사용한다 */
    if (in->signType == SIGN_UNKNOWN &&
        in->navSpeedLimitKph != 0 &&
        is_valid_speed_limit(in->navSpeedLimitKph)) {
        sel.valid = 1;
        sel.appliedLimitKph = in->navSpeedLimitKph;
        sel.sourceUsed = SOURCE_NAVIGATION;
        return sel;
    }

    return sel;
}

/**
 * @brief SLAS 판정을 수행한다.
 *
 * 입력값 검증 → 시스템 상태 판정 → 제한속도 선택 → 과속 판정 →
 * 표시 색상 결정 → 설정속도 변경 안내 순서로 처리한다.
 *
 * 과속 판정 기준:
 *   - 경고 임계값 = 적용 제한속도 + Offset
 *   - 차량 속도 > 임계값 → RED (과속 경고)
 *   - 차량 속도가 임계값의 90% 이상 → AMBER (주의)
 *   - 그 외 → GREEN (정상)
 */
SLASResult evaluate(const SLASInput *in, int previousDisplayedLimit)
{
    SLASResult result;
    result.systemState = STATE_NORMAL;
    result.baseLimitKph = 0;
    result.assistDisplayColor = COLOR_GRAY;
    result.appliedLimitKph = 0;
    result.overspeedWarning = 0;
    result.setSpeedChangeGuide = GUIDE_NONE;
    result.limitationMessage = MSG_NONE;
    result.sourceUsed = SOURCE_NONE;

    /* 과속 판정에 사용할 경고 임계값 */
    int threshold;

    /* 입력값이 유효하지 않으면 INVALID 상태로 반환 */
    if (!validate(in)) {
        result.systemState = STATE_INVALID;
        result.limitationMessage = MSG_INVALID_INPUT;
        return result;
    }

    /* 카메라 초기화 중이거나 시동 후 일정 시간이 지나지 않은 경우 */
    if (in->cameraInitializing || in->ignitionElapsedSec < INIT_TIME_THRESHOLD) {
        result.systemState = STATE_INITIALIZING;
        result.limitationMessage = MSG_INITIALIZING;
        return result;
    }

    /* 카메라가 가려진 경우 기능 제한 */
    if (in->cameraStatus == CAM_BLOCKED) {
        result.systemState = STATE_LIMITED;
        result.limitationMessage = MSG_CAMERA_OBSCURED;
        return result;
    }

    /* 제한속도 선택 정책에 따라 적용 제한속도를 결정 */
    SpeedSelection selection = select_speed_limit(in);

    if (selection.valid) {
        result.appliedLimitKph = selection.appliedLimitKph;
        result.baseLimitKph = selection.appliedLimitKph;
        result.sourceUsed = selection.sourceUsed;
    }

    /* OFF 모드이면 표시만 끄고 반환 */
    if (in->mode == SLAS_MODE_OFF) {
        result.systemState = STATE_OFF;
        result.assistDisplayColor = COLOR_OFF;
        return result;
    }

    /* 유효한 제한속도가 없으면 판정 불가 */
    if (!selection.valid) {
        return result;
    }

    /* 경고 임계값 계산 및 과속 여부 판정 */
    threshold = selection.appliedLimitKph + in->offsetKph;
    result.overspeedWarning = (in->vehicleSpeedKph > threshold) ? 1 : 0;

    /* 과속 상태에 따른 SLAS 보조 표시 색상 결정 */
    if (result.overspeedWarning) {
        result.assistDisplayColor = COLOR_RED;
    } else if (in->vehicleSpeedKph > threshold * 0.9) {
        result.assistDisplayColor = COLOR_AMBER;
    } else {
        result.assistDisplayColor = COLOR_GREEN;
    }

    /* ASSIST 모드에서 설정속도와 적용 제한속도가 다르면 변경 안내 */
    if (in->mode == SLAS_MODE_ASSIST &&
        selection.appliedLimitKph >= 30 &&
        in->setSpeedKph != selection.appliedLimitKph) {
        if (in->setSpeedKph < selection.appliedLimitKph) {
            result.setSpeedChangeGuide = GUIDE_UP;
        } else {
            result.setSpeedChangeGuide = GUIDE_DOWN;
        }
    }

    return result;
}

/**
 * @brief 제한속도를 표시용 문자열로 변환한다.
 *
 * 양수이면 숫자 문자열, 0이면 "---"를 반환한다.
 * 반환된 포인터는 정적 버퍼를 가리키므로 다음 호출 시 덮어써진다.
 */
const char* get_limit_display_string(int baseLimitKph)
{
    static char buf[16];

    if (baseLimitKph > 0) {
        snprintf(buf, sizeof(buf), "%d", baseLimitKph);
    } else if (baseLimitKph == 0) {
        snprintf(buf, sizeof(buf), "---");
    }

    return buf;
}

/**
 * @brief 입력 구조체에서 지정된 필드의 값을 문자열로 반환한다.
 *
 * "mode" 또는 "speed" 필드명을 받아 해당 값을 문자열로 변환한다.
 * 반환된 포인터는 정적 버퍼를 가리키므로 다음 호출 시 덮어써진다.
 */
const char* get_input_field_str(const SLASInput *in, const char *field)
{
    static char result[64];

    if (strcmp(field, "mode") == 0) {
        snprintf(result, sizeof(result), "%d", in->mode);
    } else if (strcmp(field, "speed") == 0) {
        snprintf(result, sizeof(result), "%d", in->vehicleSpeedKph);
    } else {
        snprintf(result, sizeof(result), "unknown");
    }

    return result;
}
