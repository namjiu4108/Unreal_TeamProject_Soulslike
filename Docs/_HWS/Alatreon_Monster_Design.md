# Alatreon 몬스터 설계 문서

> 작성일: 2026-07-12
> 담당: 몬스터 설계 (팀 프로젝트 내 몬스터 파트)

## 1. 프로젝트 전제

- 협동 소울라이크, 플레이어 인원 **1~4인**
- 플레이어/몬스터 공통으로 **GAS(Gameplay Ability System)** 사용 예정 (현재 몬스터 쪽에는 아직 미적용 상태)
- 몬스터: **Alatreon** (보스형 단일 개체)
- **몬스터 사망 = 게임 클리어**

## 2. 몬스터 최상위 상태

몬스터는 항상 아래 네 상태 중 하나에 있다. (Alert/PostCombat은 Patrol⇄Combat 사이의 세부 전이 단계 — 큰 틀에서는 여전히 "정찰"과 "전투" 두 축)

| 상태 | 설명 |
|---|---|
| **정찰 (Patrol)** | 기본 상태. 플레이어를 인지하기 전까지 유지 |
| **경계 (Alert)** | 시야범위에서 플레이어를 포착, 아직 확정 인식은 아님 |
| **전투 (Combat)** | 인식범위 진입 또는 시야범위 5초 이상 노출 시 진입 |
| **전투 후 경계 (PostCombat)** | 전투 이탈/플레이어 사망 후 10초간 마지막 위치 경계, 이후 Patrol 복귀 |

### 상태 전이 조건

- **정찰 → 경계**: 플레이어가 시야범위에 포착됐을 때
- **경계 → 전투**: 인식범위 진입(즉시) 또는 시야범위 내 5초 이상 연속 노출
- **경계 → 정찰**: 일정 시간 플레이어를 다시 못 봤을 때
- **전투 → 전투 후 경계**: 몬스터가 **NavMeshBoundsVolume을 벗어나거나** 플레이어가 전투 영역 이탈/사망했을 때
- **전투 후 경계 → 정찰**: 10초 경과
- **사망**: 상태로 취급하지 않고, 사망 시 즉시 게임 클리어 트리거로 처리

```
[정찰] --(시야범위 포착)--> [경계] --(인식범위 or 5초 노출)--> [전투]
   ^                           |                                  |
   |                      (재감지 실패)                    (영역 이탈/플레이어 이탈·사망)
   |                           v                                  v
   +------------------- [정찰]                          [전투 후 경계] --(10초)--> [정찰]

[전투] --(체력 0)--> [게임 클리어]
```

## 3. 정찰(Patrol) 상태

### 3.1 세부 동작

정찰 상태는 "가만히 있기 → (확률적으로 회전) → (확률적으로 걷기)"를 반복하는 루프로 구성한다. **둘러보기 연출은 제외**(스코프 아웃 확정, 재검토 안 함).

```
Sequence (Loop)
├─ PlayIdle        : Idle 애니메이션 재생, 2~5초 랜덤 대기
├─ MaybeTurn        : {0, 90, 180, -90, -180} 중 랜덤 선택, 0이면 스킵 / 아니면 제자리 회전
└─ MaybeWalk        : 확률적으로 전방 300~600유닛 이동, 아니면 스킵하고 PlayIdle로 복귀
```

### 3.2 감지 (정찰 → 경계 → 전투)

- **감지 컴포넌트: PawnSensing 유지** (AI Perception 대비 마이그레이션 이득 적음 — 기존 Roam/Chase/Roar 프로토타입 재사용, 신규 요구사항도 결국 커스텀 거리/타이머 로직이 필요해 어느 쪽이든 동일 비용)
- 감지 범위 2단계:
  - **시야범위** (바깥, 넓음): 플레이어를 인지했지만 확신 없음 → **Alert** 상태, `LastKnownPlayerLocation` 방향을 바라보며 경계 자세. 5초 이상 연속 노출 시 인식 확정
  - **인식범위** (안쪽, 좁음): 노출 시간과 무관하게 즉시 **Combat** 진입

```
OnSeePawn(Player) 호출마다:
  DistanceToPlayer = 계산
  IF DistanceToPlayer <= 인식범위:
      AIState = Combat (즉시)
  ELIF DistanceToPlayer <= 시야범위:
      AIState = Alert
      LastKnownPlayerLocation = Player 현재 위치
      SightTimer += DeltaTime
      IF SightTimer >= 5.0: AIState = Combat

일정 시간 OnSeePawn 미호출 시(Tick에서 마지막 감지 시각 체크):
  SightTimer = 0
  AIState == Alert 였다면 → Patrol 복귀
```

### 3.3 Blackboard 키

| 키 | 타입 | 용도 |
|---|---|---|
| `AIState` | Enum (Patrol/Alert/Combat/PostCombat) | 현재 최상위 상태 |
| `MoveTarget` | Vector | 정찰 이동 목표 지점 |
| `LastKnownPlayerLocation` | Vector | 마지막으로 확인된 플레이어 위치 |
| `DistanceToPlayer` | Float | 매 틱 갱신 |
| `SightTimer` | Float | 시야범위 내 누적 노출 시간 |
| `PostCombatTimer` | Float | 전투 이탈/사망 후 경계 카운트다운 (10초) |

### 3.4 Behavior Tree 최상위 구조

```
Root
└─ Selector
    ├─ Combat       (AIState == Combat)        ← 별도 설계 (4장)
    ├─ PostCombat    (AIState == PostCombat)
    │    └─ Sequence
    │         ├─ FaceLocation(LastKnownPlayerLocation) + 경계 자세, 10초 유지
    │         └─ AIState = Patrol (10초 경과 후)
    ├─ Alert         (AIState == Alert)
    │    └─ FaceLocation(LastKnownPlayerLocation) + 경계 자세 (루프)
    └─ Patrol (기본값, 3.1 참고)
```

## 4. 전투(Combat) 상태 — 2페이즈 구조

### Phase 전환 조건

- **체력이 절반(50%) 이하가 되면 Phase 2로 전환**

### Phase 1

| 구분 | 내용 |
|---|---|
| 지상 | 육탄전(근접 공격 패턴) |
| 공중 | 비행 패턴 3종 수행 후 착지 |

### Phase 2

| 구분 | 내용 |
|---|---|
| 지상 | 육탄전 + **브레스 패턴 추가** |
| 공중 | 기존 비행 패턴 3종 + **브레스 패턴 추가** |

### 공격 패턴 선택 방식

- 지상 패턴(육탄전, Phase 2부터 브레스 포함)과 **공중 모드는 같은 풀(pool) 안에서 가중치 기반 랜덤으로 선택**
- 공중 모드는 다른 패턴들 대비 **가중치를 낮게 설정**해서 등장 빈도를 낮춤 (자주 뜨지 않도록)
- 공중 모드가 선택되면: 비행 패턴 3종(Phase 2는 여기에 브레스 패턴 포함) 중 진행 → 착지

### 전투 이탈

- NavMeshBoundsVolume을 벗어나면 전투를 강제 종료하고 정찰 상태로 복귀

## 5. 애니메이션 블루프린트 구조

**결정: AnimBP는 정찰/전투용으로 나누지 않고 하나로 유지한다.**

- 전투 중 각 공격 패턴(몽타주)이 자체 루트모션으로 이동까지 처리하며, 몬스터는 몽타주 재생 외에 별도의 전투 전용 이동(서큘 스트레이프 등)을 하지 않기로 함
- 따라서 정찰/전투 간에 서로 다른 로코모션 블렌드스페이스가 필요 없고, 지금의 단일 AnimBP(`Speed → BS_Alatreon_Walk → Slot 'DefaultSlot' → Output Pose`) 구조로 충분함
- "모드" 구분은 AnimBP가 아니라 **게임플레이 로직에서 어떤 몽타주를 재생시키는가**로만 이루어짐 — 공격 몽타주는 `Play Anim Montage` 호출만으로 기존 `DefaultSlot`을 통해 자동 재생됨
- 추후 상반신/하반신을 동시에 다르게 블렌드해야 하는 요구(예: 이동 중 포효)가 생기면 그때 Slot을 추가하는 정도로 대응

## 6. 공격 패턴 선택 시스템

패턴을 Blueprint 그래프에 하드코딩하지 않고 **DataTable로 데이터화**해서 관리한다. 패턴 추가/가중치 조정 시 그래프를 건드릴 필요가 없고, 추후 GAS로 전환할 때도 "패턴 하나 = Ability 하나" 구조로 자연스럽게 이어질 수 있다.

### 패턴 데이터 (`FAlatreonPatternRow`)

| 필드 | 타입 | 설명 |
|---|---|---|
| `PatternID` | Name | 패턴 식별자 |
| `Category` | Enum (Ground / Aerial) | 지상/공중 구분 |
| `Montage` | AnimMontage | 재생할 몽타주 (루트모션 내장) |
| `Weight` | Float | 가중치 (공중은 낮게) |
| `MinPhase` | Int (1/2) | 등장 가능한 최소 페이즈 (브레스 계열은 2) |
| `MinRange` / `MaxRange` | Float | 이 거리 범위일 때만 후보에 포함 |
| `Cooldown` | Float | 재사용 대기시간 (선택) |

공중 서브패턴(3종 + Phase 2 브레스)은 같은 구조체로 별도 테이블(`AerialSubPatterns`)에서 관리한다.

### 선택 로직 (Combat Loop)

```
EnterCombat
 → CurrentPhase = 1, CurrentState = Combat
 → CombatLoop() 호출

CombatLoop (재귀 호출되는 커스텀 이벤트)
 1. CurrentState != Combat → 종료
 2. 체력 <= 50% && CurrentPhase == 1 → CurrentPhase = 2
 3. NavMeshBoundsVolume 이탈 확인 → 벗어났으면 CurrentState = Patrol, Roam() 호출 후 종료
 4. SelectPattern() 호출
    - Phase 조건(MinPhase <= CurrentPhase) 통과
    - 거리 조건(MinRange~MaxRange) 통과
    - 쿨다운 안 걸림
    → 통과한 후보들 중 가중치 누적합(룰렛휠) 방식으로 랜덤 선택
 5. 선택된 Category로 분기
    - Ground → Play Anim Montage(패턴.Montage)
    - Aerial → 이륙 몽타주 → AerialSubPatterns에서 가중치 랜덤 1개(Phase 2면 브레스 포함 풀) → 착지 몽타주
 6. On Completed → CombatLoop() 재호출 (루프)
```

기존 Roam/Chase/Roar와 동일한 스타일(Custom Event + 상태 값 체크)로 이어지는 구조라, 프로토타입 위에 그대로 얹을 수 있다.

## 7. 사망 처리

- 몬스터 체력이 0이 되면 **게임 클리어** 트리거 실행

## 8. 미정 사항 (TBD)

다음 항목들은 아직 결정되지 않았으며, 추후 설계/논의가 필요하다.

- [ ] Phase 1/2 각 지상 공격 패턴의 구체적인 종류와 이름
- [ ] 공중 3패턴의 구체적인 내용 (돌진, 원거리 공격 등)
- [ ] 브레스 패턴의 세부 정의 (지상용/공중용이 다른 패턴인지, 종류가 하나인지 여러 개인지)
- [ ] 패턴별 정확한 가중치 수치 (공중 모드를 얼마나 낮게 줄지)
- [ ] 같은 패턴이 연달아 뽑히는 것을 막을지(쿨다운/재선택 방지) 여부
- [ ] GAS 적용 범위 — 몬스터의 공격/상태 전이를 GameplayAbility/GameplayTag로 관리할지 여부 (패턴 데이터 구조는 이 전환을 염두에 두고 설계함)
- [ ] 멀티플레이(1~4인) 대응 — 다수 플레이어 중 타겟팅(어그로) 로직
- [ ] 정찰로 복귀할 때 체력/전투 진행도 리셋 여부
- [ ] 전투 중 NavMeshBoundsVolume 이탈이 실제로 발생 가능한 상황인지 (예: 플레이어가 몬스터를 볼륨 밖으로 유인하는 경우 등)

## 9. 현재 구현 진행 상황

- `BP_Alatreon`에 PawnSensing 기반 감지 → Roam/Chase/Roar 흐름 프로토타입 구현 중
  - 정찰(Roam): NavMesh 내 랜덤 지점 이동 반복
  - 감지 시 추격(Chase) → 사거리 진입 시 포효(Roar) 몽타주 재생 → **이후 전투(Combat) 상태로 연결 예정** (설계 논의 중)
- 스켈레톤/애니메이션: `AlatreonMaster_UE_bones.blend` 기준 본 186개, 애니메이션 613개(중복 제거 완료), 언리얼 임포트 및 루트 모션 정상 동작 확인 완료
- AnimBP: 정찰/전투 통합 단일 AnimBP로 결정 (`Speed → BS_Alatreon_Walk → Slot 'DefaultSlot' → Output Pose`), 전투 패턴은 이 Slot을 통해 몽타주로 재생
- **2026-07-13**: 정찰(Patrol) 상태 상세 설계 확정 (3장) — 감지 컴포넌트는 PawnSensing 유지로 결정, 신규 요구사항(2단계 감지범위, 10초 전투 후 경계)은 커스텀 타이머 로직으로 구현. 오늘부터 정찰 상태 실제 구현 착수.

## 10. 참고

- 스켈레톤/애니메이션 파이프라인(리타겟팅, FBX 추출, 루트 모션 버그 수정 등)은 별도 Blender 세션에서 진행됨 — 상세 내용 필요 시 세션 기록 참고
