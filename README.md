# Data Monitoring PoC

JSON 파일을 영속 저장소로 사용하는 **C++ 데이터 모니터링 PoC**입니다.

회원 데이터의 CRUD 동작과 앱 재시작 후 데이터 유지(Persistence)를 검증하며,  
**빌드 모드 하나로** 테스트와 실제 실행을 전환할 수 있도록 설계되어 있습니다.

---

## 프로젝트 구조

```
DataMonitor-JH/
├── DataMonitor-JH.sln              # Visual Studio 솔루션 (단일 프로젝트)
│
├── DataMonitor-JH/                 # 소스 프로젝트
│   ├── Member.h                    # 데이터 모델 + JSON 직렬화
│   ├── MemberRepo.h                # CRUD 저장소 인터페이스
│   ├── MemberRepo.cpp              # CRUD 저장소 구현 (Write-Through)
│   ├── main.cpp                    # Release 전용: 콘솔 CRUD UI
│   ├── test_crud.cpp               # Debug 전용: gmock/gtest 테스트 (16개)
│   └── packages.config             # NuGet: gmock 1.11.0, nlohmann.json 3.11.3
│
└── packages/
    ├── gmock.1.11.0/               # Google Mock / Google Test
    └── nlohmann.json.3.11.3/       # JSON 파싱 라이브러리 (헤더 온리)
```

---

## 빌드 모드

솔루션 구성만 바꾸면 **테스트 모드**와 **실행 모드**가 전환됩니다.

| 구성 | 컴파일 대상 | 동작 |
|------|------------|------|
| **Debug \| x64** | `test_crud.cpp` + `MemberRepo.cpp` | 빌드 완료 후 16개 gtest 자동 실행 |
| **Release \| x64** | `main.cpp` + `MemberRepo.cpp` | CRUD 콘솔 UI 실행 파일 생성 |

### Debug | x64 — gmock/gtest 자동 테스트

**Visual Studio IDE**
1. 솔루션 구성을 `Debug` / `x64` 로 설정
2. `Ctrl + Shift + B`
3. 출력 창(Output Window)에서 테스트 결과 확인

**CLI**
```powershell
MSBuild.exe DataMonitor-JH.sln /p:Configuration=Debug /p:Platform=x64
```

**출력 예시**
```
========== gmock/gtest 자동 실행 ==========
[==========] Running 16 tests from 1 test suite.
[ RUN      ] MemberRepoTest.Create_SetsIdAndFields
[       OK ] MemberRepoTest.Create_SetsIdAndFields (3 ms)
[ RUN      ] MemberRepoTest.Create_AutoIncrementsId
[       OK ] MemberRepoTest.Create_AutoIncrementsId (1 ms)
...
[  PASSED  ] 16 tests.
```

---

### Release | x64 — CRUD 콘솔 앱 실행

**Visual Studio IDE**
1. 솔루션 구성을 `Release` / `x64` 로 설정
2. `Ctrl + Shift + B`

**CLI**
```powershell
MSBuild.exe DataMonitor-JH.sln /p:Configuration=Release /p:Platform=x64
```

**실행**
```powershell
.\x64\Release\DataMonitor-JH.exe
```

데이터 파일 `members.json`은 실행 파일과 같은 폴더에 자동 생성됩니다.

---

## 앱 사용법 (Release 모드)

실행하면 다음 메뉴가 반복 표시됩니다.

```
===== Member CRUD =====
 1. List all
 2. Find by ID
 3. Search by name
 4. Create
 5. Update
 6. Delete
 0. Exit
Select:
```

| 메뉴 | 동작 | 설명 |
|------|------|------|
| `1` | List all | 전체 회원 목록 출력 |
| `2` | Find by ID | ID로 단건 조회 |
| `3` | Search by name | 이름 키워드 부분 검색 |
| `4` | Create | 새 회원 등록 (name / email / phone / age) |
| `5` | Update | ID 지정 후 필드 수정 (빈 입력 시 기존값 유지) |
| `6` | Delete | ID 지정 후 삭제 |
| `0` | Exit | 앱 종료 |

**출력 형식**
```
[ID] 이름 | 이메일 | 전화번호 | age:나이 | 생성시각(ISO 8601)
[1] Hong GilDong | hong@test.com | 010-1234-5678 | age:30 | 2026-06-12T10:22:33
```

---

## 데이터 영속화 동작

- **Write-Through 방식**: Create / Update / Delete 즉시 `members.json`에 저장
- **자동 로드**: 앱 시작 시 같은 경로의 JSON 파일을 자동으로 읽어들임
- **ID 연속성**: 재시작 후 신규 ID는 기존 최대 ID + 1 로 자동 채번

```
1회 실행: Create(id=1), Create(id=2) → 종료 → members.json 저장
2회 실행: 자동 로드 → id=1, id=2 확인 → Create(id=3)
```

---

## 테스트 항목 (Debug 빌드 시 자동 실행)

총 **16개** 테스트 케이스 (`test_crud.cpp`)

### CRUD 기본 테스트 (10개)

| 테스트명 | 검증 내용 |
|----------|----------|
| `Create_SetsIdAndFields` | 생성 시 ID · 필드 · 타임스탬프 정상 저장 |
| `Create_AutoIncrementsId` | 연속 생성 시 ID 자동 증가 (1 → 2 → 3) |
| `FindAll_ReturnsAllMembers` | 전체 조회 건수 일치 |
| `FindById_ReturnsCorrectMember` | ID로 정확한 회원 반환 |
| `FindById_ReturnsNulloptWhenNotFound` | 없는 ID 조회 → `std::nullopt` |
| `FindByName_ReturnsPartialMatch` | 이름 부분 일치 검색 |
| `Update_ModifiesExistingMember` | 수정 필드 반영, 나머지 필드 유지 |
| `Update_ReturnsFalseForNonExistent` | 없는 ID 수정 → `false` |
| `Remove_DeletesMember` | 삭제 후 조회 불가 확인 |
| `Remove_ReturnsFalseForNonExistent` | 없는 ID 삭제 → `false` |

### 영속화 테스트 (4개)

| 테스트명 | 검증 내용 |
|----------|----------|
| `Persistence_DataSurvivesRestart` | 재시작 후 데이터 그대로 유지 |
| `Persistence_IdContinuesAfterRestart` | 재시작 후 ID 채번 연속성 |
| `Persistence_UpdateSurvivesRestart` | 수정 내용 재시작 후 유지 |
| `Persistence_DeleteSurvivesRestart` | 삭제 내용 재시작 후 유지 |

### gmock 테스트 (2개)

| 테스트명 | 검증 내용 |
|----------|----------|
| `GMock_ObserverReceivesExpectedCalls` | `EXPECT_CALL`로 onCreated · onUpdated · onDeleted 각 1회 호출 검증 |
| `GMock_NoDeleteCallWhenMemberNotFound` | 삭제 실패 시 onDeleted 미호출 (`Times(0)`) 검증 |

---

## 아키텍처

```
[ Debug 모드 ]                      [ Release 모드 ]
test_crud.cpp                        main.cpp (CLI UI)
  (gtest main)                           │
      │                                  ▼
      └──────────┬──────────── MemberRepository
                 │               create / findAll / findById
                 │               findByName / update / remove
                 │
                 ├─ 메모리 캐시: vector<Member>
                 └─ 파일 I/O:   members.json  ←──→  nlohmann/json
                                (Write-Through 영속화)

Member (struct)
  ├─ id         : int     — 자동 증가
  ├─ name       : string
  ├─ email      : string
  ├─ phone      : string
  ├─ age        : int
  └─ created_at : string  — ISO 8601
```

---

## 의존 라이브러리

| 라이브러리 | 버전 | 용도 | 적용 모드 |
|-----------|------|------|----------|
| [nlohmann/json](https://github.com/nlohmann/json) | 3.11.3 | JSON 직렬화 · 역직렬화 (헤더 온리) | Debug + Release |
| [Google Mock/Test](https://github.com/google/googletest) | 1.11.0 | 단위 테스트 및 Mock 검증 | Debug only |
