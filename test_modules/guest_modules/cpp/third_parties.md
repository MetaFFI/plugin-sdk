# Third-Party Library Tests (C++)

These tests are intended to be called by a host language directly.
They are optional and require the listed dependencies to be available.

## fmt
- Format a string using `fmt::format`.
- Example target:
  - `return fmt::format("hello {}", 42);`

## nlohmann/json
- Serialize a simple object to JSON.
- Example target:
  - `nlohmann::json j = {{"a", 1}};`
  - `return j.dump();`

## Eigen
- Create a vector and compute a dot product.
- Example target:
  - `Eigen::Vector3d a(1,2,3), b(4,5,6);`
  - `return a.dot(b);`
