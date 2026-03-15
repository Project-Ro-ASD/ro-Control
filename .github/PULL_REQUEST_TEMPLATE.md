## Summary

Describe what this PR changes and why.

## Type of Change

- [ ] Bug fix
- [ ] New feature
- [ ] Refactor
- [ ] Documentation update
- [ ] Build/CI change

## Related Issues

Closes #

## Testing

List how this change was tested:
- [ ] Unit tests
- [ ] Integration tests
- [ ] Manual test

Commands run:

```bash
cmake -B build -GNinja -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

## Checklist

- [ ] I read `CONTRIBUTING.md`
- [ ] I updated documentation if needed
- [ ] I added/updated tests where relevant
- [ ] I verified formatting and linting locally
