# DOCS — how this project is tracked

Everything about the rewrite lives here. If it is not in `DOCS/`, it did not happen.

| File | What it is | Who edits it |
|---|---|---|
| [`FEATURES.md`](FEATURES.md) | Feature diff whatsie ↔ whatly ↔ proposal. **The scope contract.** Owner fills `Decision`; implementer updates `Status`. | owner + implementer |
| [`ROADMAP.md`](ROADMAP.md) | Milestones M0–M6 with feature rows and exit criteria. | implementer |
| [`PROGRESS.md`](PROGRESS.md) | Session-by-session log + milestone status table. Newest first. | implementer, every session |
| [`CODING_STANDARDS.md`](CODING_STANDARDS.md) | Binding Qt 6 / C++20 rules: layout, naming, ownership, Qt idioms, WebEngine/JS rules, tests, git. | rarely; via ADR |
| [`DECISIONS.md`](DECISIONS.md) | Architecture Decision Records (append-only) + open questions for the owner. | implementer proposes, owner decides |
| [`LESSONS.md`](LESSONS.md) | Mistakes in whatsie/whatly and the rule each became. | implementer |
| `reference/analysis-whatsie.md` | Full technical audit of the original (build, features, bugs, JS tricks). | frozen |
| `reference/analysis-whatly.md` | Full technical audit of the fork incl. adopt/drop recommendations. | frozen |
| `reference/github-issues-2026-08-27.md` | Every issue/PR of both repos, categorised, with recurring themes and requested features. | frozen (re-pull when needed) |

## Workflow

1. **Before coding a feature:** its row in `FEATURES.md` must say `KEEP` (or `LATER` moved to
   `KEEP`). Check `ROADMAP.md` for the milestone it belongs to.
2. **While coding:** follow `CODING_STANDARDS.md`; new architectural choices get an ADR.
3. **After coding:** update the row's `Status`, add a `PROGRESS.md` entry, run
   `scripts/dev-build.sh --tests`.
4. **Commit style:** conventional commits, no trailers (owner preference).

## Build & run (dev)

```sh
sudo snap install kde-qt6-core24-sdk kf6-core24   # once
scripts/dev-build.sh --tests                       # configure + build + ctest
scripts/dev-run.sh                                 # launch
QT_LOGGING_RULES="whatsie.*.debug=true" scripts/dev-run.sh
```

`scripts/dev-build.sh` targets the same Qt 6.11 that the snap and Flathub runtimes ship
(ADR-001). A system-Qt build works too when the distro has Qt ≥ 6.11:
`cmake -B build && cmake --build build && ctest --test-dir build`.
