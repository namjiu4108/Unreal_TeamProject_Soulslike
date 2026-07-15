# CLAUDE.md

Behavioral guidelines to reduce common LLM coding mistakes. Merge with project-specific instructions as needed.

**Tradeoff:** These guidelines bias toward caution over speed. For trivial tasks, use judgment.

## 1. Think Before Coding

**Don't assume. Don't hide confusion. Surface tradeoffs.**

Before implementing:
- State your assumptions explicitly. If uncertain, ask.
- If multiple interpretations exist, present them - don't pick silently.
- If a simpler approach exists, say so. Push back when warranted.
- If something is unclear, stop. Name what's confusing. Ask.

## 2. Simplicity First

**Minimum code that solves the problem. Nothing speculative.**

- No features beyond what was asked.
- No abstractions for single-use code.
- No "flexibility" or "configurability" that wasn't requested.
- No error handling for impossible scenarios.
- If you write 200 lines and it could be 50, rewrite it.

Ask yourself: "Would a senior engineer say this is overcomplicated?" If yes, simplify.

## 3. Surgical Changes

**Touch only what you must. Clean up only your own mess.**

When editing existing code:
- Don't "improve" adjacent code, comments, or formatting.
- Don't refactor things that aren't broken.
- Match existing style, even if you'd do it differently.
- If you notice unrelated dead code, mention it - don't delete it.

When your changes create orphans:
- Remove imports/variables/functions that YOUR changes made unused.
- Don't remove pre-existing dead code unless asked.

The test: Every changed line should trace directly to the user's request.

## 4. Goal-Driven Execution

**Define success criteria. Loop until verified.**

Transform tasks into verifiable goals:
- "Add validation" → "Write tests for invalid inputs, then make them pass"
- "Fix the bug" → "Write a test that reproduces it, then make it pass"
- "Refactor X" → "Ensure tests pass before and after"

For multi-step tasks, state a brief plan:

```
1. [Step] → verify: [check]
2. [Step] → verify: [check]
3. [Step] → verify: [check]
```

Strong success criteria let you loop independently. Weak criteria ("make it work") require constant clarification.

---

**These guidelines are working if:** fewer unnecessary changes in diffs, fewer rewrites due to overcomplication, and clarifying questions come before implementation rather than after mistakes.

## 5. Session Progress Tracking

**Every work session starts with a dated goal tracker and ends with it updated.**

- At the start of a session, create (or open, if it already exists) `Docs/Progress/YYYY-MM-DD.md` for today's date.
- List that session's goals as a markdown checklist (`- [ ] ...`) before starting work.
- Mark each item `- [x]` the moment it's actually verified done — not when merely attempted.
- If a goal isn't finished by the end of the session, leave it unchecked and add a one-line note on exactly where it stopped (what's done, what's blocking, what's next).
- At the start of the NEXT session, check the most recent tracker file for unfinished items before starting new work — carry them over into the new day's file rather than losing them.
- Don't let this tracker replace the real design docs (e.g. `Docs/Design/Alatreon_Monster_Design.md`) — it's a lightweight daily log of "what happened when," not a spec.

## Project
- Engine: UE 5.7
- Language: C++ + Blueprint
- Source: `TeamProject_SL/Source/TeamProject_SL/`

## Code Navigation
- 심볼/클래스 위치 찾을 때: `grep "이름" tags` 먼저 확인
- 파일 추가/삭제 후 태그 갱신:
  `ctags -R --c++-kinds=+pf --fields=+iaSn --extras=+q --languages=C++ --exclude=Intermediate --exclude=Saved --exclude=Binaries --exclude=.git TeamProject_SL/Source/`
- **tree-sitter (별도 규칙, ctags 대체가 아니라 보완용)**: 태그 조회로 안 잡히는 구조적 질의(함수 본문 안에서 특정 패턴 찾기, 클래스 상속 관계 트리, 특정 매크로 사용처의 문맥 등)가 필요할 때 tree-sitter의 AST 기반 쿼리를 사용. ctags는 "이 심볼이 어디 있는지"에 강하고, tree-sitter는 "이 코드 구조가 어떻게 생겼는지"에 강함 — 둘을 상호 보완적으로 쓸 것. (현재 이 머신에 tree-sitter CLI가 설치되어 있는지 미확인 — 사용 전 `tree-sitter --version`으로 먼저 확인하고, 없으면 설치 여부를 사용자에게 확인할 것.)

## Coding Rules
- 복잡한 로직 → C++
- 입력 처리, VFX 연결 → Blueprint
- UPROPERTY / UFUNCTION 매크로 필수
- 헤더엔 전방선언 우선, 꼭 필요할 때만 #include
