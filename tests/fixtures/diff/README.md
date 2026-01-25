# Diff golden fixtures

Each subfolder contains:
- `left.txt`
- `right.txt`
- `expected_exact.txt`
- `expected_ignore_trailing.txt`
- `expected_ignore_all.txt`

The test `DiffGoldenFixtures.FixturesMatchExpectedOutput` compares a deterministic text serialization of the diff engine output.

To (re)generate expected outputs:

```bash
BENDIFF_UPDATE_GOLDENS=1 ctest --test-dir build --output-on-failure -R DiffGoldenFixtures
```
