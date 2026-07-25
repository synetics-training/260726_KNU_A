# SLAS SW 테스트 실습 (4시간)

SLAS(Speed Limit Assist System)를 대상으로 테스트 케이스 설계, 리뷰, 테스트 수행, 결함 등록을 수행하는 팀 실습입니다.

---

## 실습 환경

### 사전 준비

| 도구 | 용도 | 비고 |
|---|---|---|
| GitHub 계정 | 레포지토리 접근, Issue/PR 작성 | 사전에 강사에게 계정 공유 |
| 웹 브라우저 | SLAS 검증 UI 실행, GitHub 사용 | Chrome 권장 |
| Excel / Google Sheets | TC 작성 | TC_template.xlsx 사용 |

> 이 실습은 코드 작성 없이 **테스트 설계 및 수행**에 집중합니다.

---

## 저장소 구조

```
├── src/                          # 소스 코드 (참조용)
│   ├── slas.h                    # SLAS 헤더 (구조체, 열거형 정의)
│   └── slas.c                    # SLAS 구현 코드
│
├── test/                         # 테스트 관련
│   └── TC_template.xlsx          # TC 작성 및 결과 기록 양식
│
├── docs/                         # 참조 문서
│   ├── SLAS_요구사항명세서.md      # 요구사항 명세서 (32개 SRS)
│   └── SLAS_과거오류DB.md         # 과거 오류 DB (오류 추정용)
│
├── tools/                        # 도구
│   └── SLAS_시뮬레이터.html         # SLAS 소프트웨어 검증 UI
│
└── .github/                      # GitHub 자동화
    ├── workflows/
    │   ├── codeql.yml            # CodeQL 보안 분석
    │   └── static-analysis.yml   # cppcheck + lizard CI
    ├── ISSUE_TEMPLATE/
    │   ├── defect-report.yml     # 결함 보고 템플릿
    │   ├── improvement.yml       # 개선 제안 템플릿
    │   └── config.yml            # Issue 템플릿 설정
    ├── pull_request_template.md  # PR 체크리스트 템플릿
    └── CODEOWNERS                # 리뷰어 자동 지정
```

---

## 실습 타임테이블

| 타임 | 주제 | 시간 |
|---|---|---|
| **1교시** | TC 설계 (EP, BV, 요구사항 분석) | 45분 |
| 쉬는 시간 | | 15분 |
| **2교시** | TC 리뷰 (PR + 동료 검토) | 45분 |
| 쉬는 시간 | | 15분 |
| **3교시** | 테스트 수행 + 결함 등록 | 45분 |
| 쉬는 시간 | | 15분 |
| **4교시** | 정적 분석 시연 + 결과 발표 | 45분 |

---

## 1교시. TC 설계

### 1-1. 브랜치 생성

```bash
git clone https://github.com/ORG/REPO.git
cd REPO
git checkout -b tc-design/이름
```

> main 브랜치는 보호되어 있어 직접 push할 수 없습니다.

### 1-2. 요구사항 분석 + TC 설계

[SLAS 요구사항 명세서](docs/SLAS_요구사항명세서.md)를 읽고 TC를 작성합니다.

**팀원별 담당 배분 (예시: 4인 팀)**

| 팀원 | 담당 영역 | 요구사항 | TC 파일명 |
|---|---|---|---|
| A | 제한속도 결정 + 표시 | SRS-001, 002, 019~024 (8건) | `test/TC_A_이름.xlsx` |
| B | 보조 표시 + 과속 경고 + 출처 | SRS-005~009, 029~032 (9건) | `test/TC_B_이름.xlsx` |
| C | 모드 + 안내 + 유효성 | SRS-003, 004, 010~016 (9건) | `test/TC_C_이름.xlsx` |
| D | 기능 제한 처리 | SRS-017, 018, 025~028 (6건) | `test/TC_D_이름.xlsx` |

> `test/TC_template.xlsx`를 복사하여 자신의 파일명으로 변경한 후 작성합니다.

**TC ID 네이밍:** `SLAS-TC-NNN` (순번)

**활용 기법:**

| 약어 | 기법 |
|---|---|
| RA | 요구사항 분석 (Requirement Analysis) |
| EP | 동등 분할 (Equivalence Partitioning) |
| BV | 경계값 분석 (Boundary Value Analysis) |
| EG | 오류 추정 (Error Guessing) — [과거 오류 DB](docs/SLAS_과거오류DB.md) 참고 (선택) |

### 1-3. TC 리뷰 요약 파일 작성

TC 엑셀과 함께 `tc_review_이름.md` 파일을 작성합니다.

```markdown
# TC 리뷰 요약 — 홍길동

- 담당 요구사항: SRS-001 ~ SRS-008
- 작성 TC 수: 12건
- 적용 기법: EP 6건, BV 4건, EG 2건
- 특이사항: SRS-005 경계값에서 부등호 방향 확인 필요
```

> 이 파일은 GitHub PR에서 인라인 리뷰를 위한 것입니다.

### 1-4. 커밋 + PR 생성

```bash
git add test/TC_*.xlsx tc_review_이름.md
git commit -m "test: TC 설계 (SRS-001~008)"
git push origin tc-design/이름
```

GitHub에서 **Compare & pull request** 클릭 → PR 템플릿의 **TC 리뷰** 체크리스트를 작성합니다.

> 리뷰어 1명을 반드시 지정하세요.

---

## 2교시. TC 리뷰

### 2-1. 리뷰 수행

1. **엑셀 파일**: 다운로드하여 내용 검토 → PR 체크리스트에 결과 기록
2. **tc_review_이름.md**: GitHub 웹에서 인라인 코멘트로 리뷰 (질문, 수정 요청 등)

### 2-2. PR 체크리스트 기준

| 항목 | 확인 내용 |
|---|---|
| 요구사항 커버리지 | 담당 요구사항에 대한 TC가 빠짐없이 존재하는가? |
| 기법 적용 여부 | EP, BV가 적절히 적용되었는가? |
| 요구사항 정확도 | 기대 결과가 요구사항 명세서와 정확히 일치하는가? |
| 오류 추정 (선택) | 과거 오류 DB를 참고한 TC가 포함되어 있는가? |

### 2-3. 리뷰 반영 + 머지

1. 리뷰어의 코멘트를 확인하고 수정 반영
2. 모든 코멘트가 **Resolved** 상태여야 머지 가능
3. 리뷰어 Approve 후 **Merge**

---

## 3교시. 테스트 수행

### 3-1. 검증 UI로 TC 실행

`tools/SLAS_시뮬레이터.html`을 브라우저에서 열고 TC를 실행합니다.

1. 시나리오 프리셋 버튼으로 기본 환경 설정
2. TC에 명시된 입력값으로 필드 변경
3. **"현재 Step 실행"** 클릭
4. 결과를 TC의 **Actual Result**에 기록
5. **Fail**인 경우 → **GitHub Issue로 결함 등록**

### 3-2. 결함 등록

Issues 탭 → New issue → **"결함 보고 (Defect Report)"** 선택

**필수 항목:**

| 항목 | 내용 |
|---|---|
| 발견 수단 | 소프트웨어 테스트 (기능 테스트) |
| 스크린샷 | **필수** — 검증 UI 결과 화면 캡처 |
| 재현 절차 | 입력값과 실행 순서 |
| 기대 결과 | 요구사항에 따른 기대 동작 |
| 실제 결과 | 검증 UI에서 관찰된 동작 |

| 심각도 | 기준 |
|---|---|
| Critical | 과속 경고 미표시 또는 잘못된 제한속도 표시 |
| Major | SLAS 보조 표시 색상 오류, 출처 오류 |
| Minor | 설정속도 변경 안내 오류, 표시 형식 오류 |
| Info | 코딩 표준 위반, 개선 권고 (정적 분석) |

---

## 4교시. 정적 분석 + 결과 발표

### 4-1. 정적 분석 결과 확인

GitHub에 등록된 코드에 대해 CI가 자동으로 수행한 정적 분석 결과를 확인합니다.

> 개발자 로컬 → **GitHub CI** → Jenkins — 3단계 정적 분석 중 GitHub CI 단계에 해당합니다.

- **Security 탭** → Code scanning alerts 확인
- **PR의 Checks 탭**에서 cppcheck / lizard 결과 확인
- 위반 사항을 **결함 보고** 양식으로 이슈 등록 (발견 수단: cppcheck/lizard 선택)

### 4-2. 팀별 결과 발표

- 발견한 결함 건수와 내용 발표
- 요구사항 커버리지 달성률 보고
- 사용한 테스트 기법과 효과 공유

---

## Branch 보호 규칙

| 설정 | 값 |
|---|---|
| Require a pull request before merging | ON |
| Require approvals | 1명 |
| Require conversation resolution before merging | ON |

---

## 참고 자료

- [SLAS 요구사항 명세서](docs/SLAS_요구사항명세서.md)
- [과거 오류 DB](docs/SLAS_과거오류DB.md)

