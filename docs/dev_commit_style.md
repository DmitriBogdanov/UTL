# Commit style

This file contains standard commit style for this project.

## Commit types

| Type       | Description                      |
| ---------- | -------------------------------- |
| `feat`     | New features                     |
| `test`     | New unit tests                   |
| `fix`      | Bugfixes                         |
| `refactor` | Code refactors                   |
| `docs`     | Documentation changes            |
| `build`    | Build script changes             |
| `chore`    | Typo fixes, file renames and etc |

## Commit message format

### Format

```
<type>(<scope>): <summary>.
// repeat for all changes according to commit types
// <scope> can be omitted for changes not related to a particular module
```

### Example

```
feat(mvl): Mostly implemented sparse and dense blocking operations.

chore(): Typo fixes in comments & documentation.

docs(mvl): Fixes some incorrect definitions and wonky SVG rendering.

docs(): Improved table with used tools & libraries.
```