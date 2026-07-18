# Alatreon 몬스터 설계 문서

> 작성일: 2026-07-12
> 최종 수정: 2026-07-14 (전투 루프 골격 Blueprint 구현 및 테스트 완료)
> 담당: 몬스터 설계 (팀 프로젝트 내 몬스터 파트)

## 1. 프로젝트 전제

- 협동 소울라이크, 플레이어 인원 **1~4인**
- 플레이어/몬스터 공통으로 **GAS(Gameplay Ability System)를 사용하기로 확정** (현재 몬스터 쪽에는 아직 미적용 상태 — 지금 만드는 로직들(속도 조정, 공격 패턴 등)은 추후 GAS로 이전하는 걸 염두에 두고 설계함)
- 몬스터: **Alatreon** (보스형 단일 개체)
- **몬스터 사망 = 게임 클리어**

## 2. 몬스터 최상위 상태

몬스터는 아래 상태 중 하나에 있다 (Behavior Tree Selector + Blackboard `AIState` Enum으로 구현됨).

| 상태 | 설명 | 구현 상태 |
|---|---|---|
| **정찰 (Patrol)** | 기본 상태. 플레이어를 인지하기 전까지 유지 | 완료 |
| **경계 (Alert)** | 플레이어 인지 직후, 접근 + 포효 1회 실행 | 완료 |
| **전투 (Combat)** | 포효 종료 후 진입, 실제 공격 패턴 루프 | 루프 골격 Blueprint 구현+테스트 완료(패턴 내용은 TBD) |
| **전투 후 경계 (PostCombat)** | 전투 중 플레이어를 놓쳤을 때 잠시 경계 유지 | 미구현 |

### 상태 전이 조건

- **정찰 → 경계**: PawnSensing `On See Pawn`으로 플레이어 인지 시
- **경계 → 전투**: 경계 상태에서 플레이어 근처까지 접근 완료 후 포효 몽타주가 끝나는 순간
- **전투 → 정찰**: 몬스터가 **NavMeshBoundsVolume을 벗어났을 때**
- **사망**: 상태로 취급하지 않고, 사망 시 즉시 게임 클리어 트리거로 처리

```
[정찰] --(플레이어 인지)--> [경계: 접근+포효 1회] --(포효 완료)--> [전투]
                                                                      |
                                                    --(NavMeshBoundsVolume 이탈)--> [정찰]
                                                                      |
                                                                 (체력 0)
                                                                      v
                                                                 [게임 클리어]
```

## 3. 정찰(Patrol) 상태 — 구현 완료

- NavMesh 범위 내에서 랜덤 위치로 이동을 반복하는 로밍 동작
- 루프: `Wait(3.00s ± 1.00s)` → `BTTask_RandomTurn`(0/90/-90/180 중 랜덤, 애니메이션 기반 회전) → `BTTask_PickPatrolPoint`(회전 후의 forward 방향으로 800~2000 유닛 지점 선택) → `Move To`
- 회전은 애니메이션(Animation 013/014/015/016)의 루트모션을 그대로 사용하며, 걷기 전환 시 발이 안 맞는 문제는 브릿지 애니메이션으로 보완 중 (Blender 쪽 별도 세션에서 진행)
- 회전 후에는 `Character Movement`의 `Orient Rotation to Movement`/`Use Controller Rotation Yaw`를 꺼서 애니메이션이 만든 방향을 그대로 유지한 채 직진

## 3-1. 경계(Alert) 상태 — 구현 완료

플레이어를 감지한 직후, 실제 전투(공격 패턴)에 들어가기 전 접근 + 포효를 1회만 실행하는 상태.

- `On See Pawn`: `AIState = Alert` + `LastKnownPlayerLocation = 감지된 플레이어 위치`로 Blackboard SET
- BT: `Move To(LastKnownPlayerLocation, Acceptable Radius=거대 몬스터 기준 큰 값)` → `BTTask_PlayRoarMontage`
- `BTTask_PlayRoarMontage`의 `On Completed`에서 `AIState = Combat`으로 전환 (여기서 경계 → 전투로 넘어감)
- Selector 우선순위: `Alert 진입(CombatStart)` > `Combat` > `Patrol`(데코레이터 없음, 기본 폴백)
- **이동 애니메이션**: 정찰 중엔 걷기, 플레이어 인지(Alert 진입) 순간부터는 달리기로 전환하고 속도도 올림. 전투 상태 내내 유지하다가 정찰로 복귀할 때 다시 걷기/원래 속도로.
  - AnimBP: `BS_Alatreon_Walk`/`BS_Alatreon_Run`을 `Blend Poses by Bool(Is Combat Moving)`으로 스위칭, `Is Combat Moving = (AIState == Combat 또는 Alert)`를 매 틱 Blackboard에서 읽어와 계산
  - 속도 SET/복원은 게임플레이 로직(`BP_Alatreon`)에서 `Character Movement → Max Walk Speed`를 직접 조정 (GAS 미적용 상태라 임시로 직접 SET, 추후 `MoveSpeed`/`MaxMoveSpeed` 어트리뷰트로 전환 예정)

## 4. 전투(Combat) 상태 — 골격 설계 완료, 실제 패턴은 TBD

### 기본 루프

전투 진입 시 Combat Idle(`Animation 003`)로 대기하며 아래 루프를 반복한다.

```
CombatLoop
 1. 타겟 선정: 네비게이션(전투 범위) 내에 있는 플레이어 중 랜덤 1명 → CurrentTarget
 2. 콤보 횟수 결정: Random(1~3) → ComboCount
 3. ComboCount 회 반복 (매 회마다):
     a. CurrentTarget 유효성 확인 (사망/사거리 이탈) → 무효면 남은 반복 중단, 1번으로
     b. 공격 패턴 선택 (SelectPattern, 아래 "패턴 선택" 참고) — 포지셔닝보다 먼저 결정
     c. 선택된 패턴이 요구하는 포지셔닝이 필요한지 확인 → 필요하면 포지셔닝 실행 (아래 "포지셔닝" 참고)
     d. 공격 몽타주 재생 (지금은 Roar로 임시 대체, 실제 패턴은 TBD)
 4. Combat Idle로 복귀 → 1번으로 루프
```

- 콤보(1~3회) 동안 타겟은 고정하되, **매 공격 시도마다 타겟의 현재 위치를 새로 스냅샷**한다 (한 번만 찍지 않음) — 콤보 도중 플레이어가 멀어지거나 몬스터 뒤로 이동하면 다음 공격 시도에서 그 위치 변화가 반영되도록.
- 매 공격 시도마다 **공격 패턴도 새로 뽑는다** (콤보 시작 때 정한 패턴을 그대로 반복하지 않음) — 같은 이유(위치 변화에 따라 다른 패턴이 더 적합할 수 있음).

### 포지셔닝(방향 정렬) 시스템

공격 패턴이 정해진 뒤, 그 패턴이 포지셔닝을 요구하면 아래 로직으로 회전/이동을 결정한다.

**애니메이션**:
- `Animation 003` = 전투 Idle
- `Animation 104` = 왼쪽 ~90도 회전
- `Animation 105` = 왼쪽 뒤돌기 (후면-좌)
- `Animation 106` = 정면 근처에서 뒤로 이동 (거리 기반, 각도 아님)
- `Animation 107` = 오른쪽 ~90도 회전
- `Animation 108` = 오른쪽 뒤돌기 (후면-우)
- 재생 방식은 정찰 턴(013~016)과 다름: **Force Root Lock을 켜고**, 애니메이션 재생 전에 타겟 위치를 스냅샷해서 그 방향으로 블루프린트가 직접 회전(RInterpTo 등)시킨다. 애니메이션 자체의 회전각과 실제 필요한 각도가 조금 어긋나도 부자연스럽지 않은 모션들이라 정찰 턴처럼 정확한 각도 분석이 필요 없음.

**로직**:
```
1. 몬스터→타겟 상대각 계산
2. 선택된 공격 패턴이 "회전 불필요"(뒤쪽에서도 맞는/뒤쪽 전용 패턴이고 조건 충족)면 → 회전 스킵
3. 아니면 각도에 따라:
   - 정면 근처(허용 범위 안) → 회전 없음
   - 왼쪽 ~90도 → 104
   - 왼쪽 후면 → 105
   - 오른쪽 후면 → 108
   - 오른쪽 ~90도 → 107
4. 회전이 필요 없는(정면 근처) 경우에도 거리 체크:
   - 너무 가까움 → 106 (뒤로 이동)
   - 적정 거리 → 아무것도 안 함
```

### 거리 기반 분기 (근접/돌진/원거리)

타겟과의 거리가 너무 멀 때(공격 사거리 밖)의 처리 — 3가지 선택지가 있고, **패턴 선택 풀 안에서 가중치로 자연스럽게 결정**된다 (별도 우선순위 분기 아님):

1. **다가가기(접근)**: 거리가 멀 때 뽑힌 패턴이 근접형이면, 접근 이동을 먼저 하고 **그 자리에서 바로 이어서 그 패턴의 공격을 실행** (접근이 별도 시도로 소모되지 않음 — 확정)
2. **돌진**: 거리가 멀 때 뽑힌 패턴이 돌진형이면 돌진 실행 (돌진 자체가 이동+공격을 겸함)
3. **원거리 공격** (Phase 2 전용): 거리가 멀 때 뽑힌 패턴이 원거리형이면 원거리 공격 실행
   - 일부러 플레이어의 현재 위치보다 살짝 늦은(지연된) 위치를 조준해서, 플레이어에게 피할 시간을 줌 (읽기 쉬운 텔레그래프)
   - 단, 너무 안 맞으면 재미없으므로 판정 범위(AoE)나 패턴 속도로 밸런스 조절 필요 (TBD 수치)

**가중치 규칙**:
- 거리가 멀수록 **돌진/원거리 패턴의 가중치를 높여서** 더 자주 선택되게 함
- 타겟이 몬스터 뒤쪽에 있을 때 **뒤쪽 대응 가능한 패턴의 가중치를 높여서** 더 자주 선택되게 함

### 패턴 후보 필터링 (뒤쪽 전용 패턴)

일부 공격 패턴은 "뒤쪽도 맞음" 또는 "뒤쪽만 맞음" 특성을 가진다.
- **뒤쪽 전용 패턴은 타겟이 실제로 몬스터 뒤쪽에 있을 때만 후보 풀에 포함** (기존 `MinRange`/`MaxRange` 필터와 같은 방식의 조건)
- 포함된 경우 위 가중치 규칙에 따라 더 자주 뽑히도록 조정

### Phase 전환 조건 (기존 유지)

- 체력이 절반(50%) 이하가 되면 Phase 2로 전환 (원거리 공격, 브레스 패턴은 Phase 2부터 등장)

### 테스트 방법 — 완료 (2026-07-14)

실제 공격 패턴(몽타주)이 없는 상태라, Roar를 임시 공격으로 대체해서 루프 구조(타겟 선정 → 콤보 → Roar 재생) 자체를 검증했고 **정상 동작 확인함**. 포지셔닝(104~108)/실제 공격 패턴은 아직 이 루프에 연결 안 됨 (다음 단계).

### 구현 세부사항 (Blueprint 구조)

**Blackboard 키 추가** (`BB_Alatreon`): `CurrentTarget`(Object, Base Class=`BP_PlayerCharacter`), `ComboCount`(Int)

**`BT_Alatreon`**: `Combat` 시퀀스 = `BTTask_CombatLoop` 하나만 (실제 반복 로직은 전부 Blueprint 커스텀 이벤트에서 처리)

**`BTTask_CombatLoop`** (롱-러닝 태스크):
- `Event Receive Execute AI`: Cast to BP_Alatreon → `StartCombatLoop` 호출 (여기서 Finish Execute 안 함, 계속 활성 유지)
- `Event Receive Tick AI`: `AIState != Combat`이면 `StopCombatLoop` 호출 → `Finish Execute`

**`BP_Alatreon`의 커스텀 이벤트 4개** (`bCombatLoopActive` bool 변수 사용):
- `StartCombatLoop`: `bCombatLoopActive=true` → `CombatLoop` 호출
- `CombatLoop`: `bCombatLoopActive` 체크 → `Get All Actors Of Class(BP_PlayerCharacter)` → 랜덤 1명 선택(`Is Valid`로 검증) → `CurrentTarget`/`ComboCount(Random 1~3)` Blackboard SET → `ComboStep` 호출
- `ComboStep`: `bCombatLoopActive` 체크 → `CurrentTarget` `Is Valid` 확인(무효면 `CombatLoop`로 재시도) → 거리 체크(너무 멀면 `CombatLoop`로 재시도) → `ComboCount<=0`이면 Delay 후 `CombatLoop` → 아니면 `ComboCount` 감소 후 `Play Montage`(현재는 Roar) → `On Completed`/`On Interrupted` 둘 다 `ComboStep` 재귀 호출
- `StopCombatLoop`: `bCombatLoopActive=false`

### 디버깅 중 발견한 함정 (재발 방지용 기록)

오늘 이 루프를 실제로 동작시키기까지 아래 문제들을 순서대로 겪음 — 비슷한 구조를 다른 곳에 적용할 때 참고:

1. **Blackboard Enum 값 읽기**: `Get Value as Enum`은 강타입 Enum이 아니라 **순수 Byte를 반환**한다. `E_AIState`끼리 직접 비교하려면 `Byte to E_AIState` 변환 노드를 명시적으로 껴야 함 (Blueprint가 자동으로 안 해줌).
2. **`Literal enum` 드롭다운 오타성 실수**: `On See Pawn`에서 `AIState`를 `Alert`로 설정해야 하는데 실수로 `Combat`으로, `BTTask_PlayRoarMontage`의 `On Completed`도 엉뚱한 값으로 되어 있던 적이 있었음 — 상태 전이 관련 `Literal enum` 노드는 항상 드롭다운 값을 재확인할 것.
3. **`Play Montage`의 `On Completed`/`On Interrupted` 핀 오배선**: 완료 콜백을 엉뚱한 핀에 연결해서 상태 전이가 아예 안 일어나는 문제가 두 번 있었음 (`BTTask_PlayRoarMontage`, `ComboStep` 둘 다).
4. **`Is Valid` 매크로의 `Is Valid`/`Is Not Valid` 핀 반대로 연결**: "유효할 때" 쪽에 재시도(`CombatLoop`) 로직을 연결하는 실수를 두 번 함 — 유효할 때는 계속 진행, 무효할 때 재시도가 맞음. 이게 **무한 루프의 근본 원인**이었음 (유효한 타겟을 찾을 때마다 오히려 처음으로 돌아가서, 딜레이 없이 계속 재귀호출됨).
5. **Behavior Tree Selector 자식 순서**: Selector는 왼쪽(또는 낮은 인덱스)부터 평가해서 첫 통과를 선택한다. 조건 없는 `Patrol`이 `Combat`보다 먼저 있으면 `AIState`가 `Combat`이어도 `Patrol`이 항상 이김. **우선순위 높은 상태(조건 있는 것)를 항상 먼저, 조건 없는 기본값(Patrol)은 항상 맨 마지막**에 둘 것.
6. **`BeginPlay`에서 Blackboard Component 캐싱 금지**: `Event BeginPlay` 시점엔 AIController가 아직 폰을 possess 안 했을 수 있어서 `Get Controller`가 `None`을 반환 → Cast 실패 → 캐싱 변수가 영원히 `None`으로 남음. **`AIBlueprintHelperLibrary::Get Blackboard(Target=self)`를 필요할 때마다 새로 호출**하는 게 안전함 (또는 `Event Possessed`에서 캐싱).
7. **BT Task의 `Event Receive Execute AI`/`Event Receive Tick AI`는 완전히 분리된 독립 체인으로 작성**: 두 이벤트의 로직을 하나의 `Branch` 노드로 얽어서 쓰면 실행 시점이 꼬여서 의도치 않게 즉시 종료되거나 아예 안 도는 문제가 생김.
8. **재귀 호출 경로에는 항상 최소한의 Delay를 둘 것**: `ComboStep`→`CombatLoop`로 돌아가는 모든 경로(타겟 무효, 거리 초과, 콤보 소진)에 Delay가 없으면, 4번 같은 로직 버그와 겹쳤을 때 무한 루프 감지로 강제 종료됨. 근본 버그를 다 잡았어도 안전장치로 남겨두는 게 좋음.

### 전투 이탈

- NavMeshBoundsVolume을 벗어나면 전투를 강제 종료하고 정찰 상태로 복귀 (아직 미구현)

## 5. 애니메이션 블루프린트 구조

- 단일 AnimBP 유지: `Speed → (BS_Alatreon_Walk / BS_Alatreon_Run, Blend Poses by Bool) → Slot 'DefaultSlot' → Output Pose`
- 정찰=걷기, 경계/전투=달리기로 스위칭 (3-1 참고)
- "모드" 구분은 AnimBP가 아니라 게임플레이 로직에서 어떤 몽타주를 재생시키는가로 이루어짐 — 공격/포효/턴 몽타주는 `Play Anim Montage`(또는 async `Play Montage` 노드) 호출만으로 기존 `DefaultSlot`을 통해 자동 재생됨
- 전투 포지셔닝(104~108)은 위에서 설명한 대로 Force Root Lock + 블루프린트 회전 방식이라 이 Slot 구조를 그대로 재사용

## 6. 공격 패턴 선택 시스템

패턴을 Blueprint 그래프에 하드코딩하지 않고 **DataTable로 데이터화**해서 관리한다.

### 패턴 데이터 (`FAlatreonPatternRow`) — 확장됨

| 필드 | 타입 | 설명 |
|---|---|---|
| `PatternID` | Name | 패턴 식별자 |
| `Category` | Enum (Ground / Aerial / Charge / Ranged) | 공격 유형 구분 |
| `Montage` | AnimMontage | 재생할 몽타주 (루트모션 내장) |
| `Weight` | Float | 기본 가중치 |
| `MinPhase` | Int (1/2) | 등장 가능한 최소 페이즈 (브레스/원거리는 2) |
| `MinRange` / `MaxRange` | Float | 이 거리 범위일 때만 후보에 포함 |
| `RequiresRearTarget` | Bool | true면 타겟이 뒤쪽에 있을 때만 후보에 포함 |
| `CanHitFromRear` | Bool | true면 회전 없이(뒤쪽에서도) 사용 가능 — 포지셔닝 스킵 조건 |
| `Cooldown` | Float | 재사용 대기시간 (선택) |

**컨텍스트 기반 가중치 보정** (기본 Weight에 곱/가산):
- 거리가 멀수록 `Category == Charge` / `Category == Ranged` 패턴 가중치 ↑
- 타겟이 뒤쪽에 있을 때 `RequiresRearTarget == true` 패턴 가중치 ↑

공중 서브패턴(3종 + Phase 2 브레스)은 같은 구조체로 별도 테이블(`AerialSubPatterns`)에서 관리 (기존 설계 유지).

### 선택 로직 (Combat Loop) — 4번 항목 갱신

```
CombatLoop
 1. CurrentState != Combat → 종료
 2. 체력 <= 50% && CurrentPhase == 1 → CurrentPhase = 2
 3. NavMeshBoundsVolume 이탈 확인 → 벗어났으면 CurrentState = Patrol, 종료
 4. 타겟 유효성 확인 (콤보 중이면 콤보 유지, 무효화 시 콤보 중단하고 새 타겟)
 5. SelectPattern() 호출
    - Phase/거리/RequiresRearTarget 조건 통과한 후보만
    - 컨텍스트 가중치 보정 적용 후 룰렛휠 방식으로 랜덤 선택
 6. 선택된 패턴이 포지셔닝 필요(CanHitFromRear==false 이고 타겟이 정면 밖) 하면 포지셔닝 먼저 실행
 7. Category로 분기: Ground/Aerial/Charge/Ranged 각각에 맞는 몽타주 재생
 8. On Completed → 콤보 카운트 확인 → 남았으면 5번부터 재시도(타겟 위치/패턴 재선정), 다 썼으면 Idle 복귀 후 CombatLoop 재호출
```

## 7. 사망 처리

- 몬스터 체력이 0이 되면 **게임 클리어** 트리거 실행

## 8. 미정 사항 (TBD)

- [ ] Phase 1/2 각 지상 공격 패턴의 구체적인 종류와 이름 (실제 몽타주 없음, Roar로 임시 테스트 중)
- [ ] 공중 3패턴의 구체적인 내용 (돌진, 원거리 공격 등)
- [ ] 브레스 패턴의 세부 정의
- [ ] 패턴별 정확한 가중치 수치 및 컨텍스트 보정값 (거리/후면 가중치를 얼마나 올릴지)
- [ ] 같은 패턴이 연달아 뽑히는 것을 막을지(쿨다운/재선택 방지) 여부
- [ ] 정면/측면/후면 판정 각도 임계값, 106(뒤로 이동) 발동 거리 임계값 — 실제 수치는 테스트하며 조정
- [ ] 원거리 공격의 "지연된 조준 위치" 오프셋 양, 판정 범위/속도 밸런스
- [ ] GAS 적용 범위 (GAS 사용 자체는 확정 — 몬스터의 공격/상태 전이/속도 조정 등을 GameplayAbility/GameplayEffect/GameplayTag로 언제·어디까지 이전할지가 TBD)
- [ ] 멀티플레이(1~4인) 대응 — 다수 플레이어 중 타겟팅(어그로) 로직 (지금은 순수 랜덤 선택으로 가정)
- [ ] 정찰로 복귀할 때 체력/전투 진행도 리셋 여부
- [ ] 전투 중 NavMeshBoundsVolume 이탈이 실제로 발생 가능한 상황인지
- [ ] PostCombat(전투 후 경계) 상태 미구현 — 10초 경계 후 정찰 복귀 로직 필요

## 9. 현재 구현 진행 상황 (2026-07-14 기준)

- **정찰(Patrol)**: 완료 — `BB_Alatreon`/`BT_Alatreon` 기반, `Wait → BTTask_RandomTurn → BTTask_PickPatrolPoint → Move To` 루프
- **경계(Alert)**: 완료 — `On See Pawn`에서 `AIState=Alert` 설정, `Move To(LastKnownPlayerLocation) → BTTask_PlayRoarMontage`, 완료 시 `AIState=Combat`. 실제 플레이 테스트로 정찰→경계(접근+포효)→전투 전이 확인함.
- **전투(Combat)**: **루프 골격 Blueprint 구현 완료 + 실제 테스트로 정상 동작 확인함** — 타겟 랜덤 선정, 콤보 1~3회 반복(매회 위치 재확인), Roar를 임시 공격으로 대체해서 검증. 포지셔닝(104~108)과 실제 공격 패턴(DataTable)은 아직 이 루프에 안 붙어있음 — 다음 단계.
- **이동 애니메이션 전환**: 정찰=걷기/경계·전투=달리기 스위칭 완료 (`Blend Poses by Bool`), 속도 SET도 완료
- **Mirror Data Table 방식**: 스켈레톤 리깅 특성상 근본적으로 안 맞아서 포기, 브릿지 애니메이션 방식으로 대체 (Blender 세션에서 진행 중)
- 스켈레톤/애니메이션: `AlatreonMaster_UE_bones.blend` 기준 본 186개, 애니메이션 613개(중복 제거 완료)

## 10. 다음에 할 일 (우선순위 순)

1. **포지셔닝(104~106~108) 로직을 `ComboStep`에 연결** — 지금은 타겟 방향/거리 계산 없이 바로 Roar만 재생함. 상대각 계산 → 104/105/106/107/108 선택 → Force Root Lock+블루프린트 회전 순서로 붙이기
2. **실제 공격 패턴 제작 시작** — `FAlatreonPatternRow` DataTable 실제로 만들고, Roar 자리에 진짜 패턴 몽타주 연결
3. **전투 이탈(NavMeshBoundsVolume 이탈 → 정찰 복귀)** 로직 아직 미구현 — 지금은 전투에 들어가면 나올 방법이 없음
4. **PostCombat(전투 후 경계) 상태** 미구현 — 타겟을 완전히 놓쳤을 때(사망/범위 이탈 후 새 타겟도 없음) 처리
5. 8절 TBD 항목들(가중치 수치, 각도/거리 임계값 등)은 실제 패턴/포지셔닝 붙이면서 테스트로 조정

## 11. 참고

- 스켈레톤/애니메이션 파이프라인(리타겟팅, FBX 추출, 루트 모션/브릿지 애니메이션 등)은 별도 Blender 세션에서 진행됨 — 상세 내용은 `blender/blendertool/HANDOFF_animation_pipeline.md` 참고
