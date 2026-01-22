# Third-Party Library Tests (Java)

These tests are intended to be called by a host language directly (not by Java unit tests).
They are optional and require the listed dependencies to be available on the classpath.

## log4j (2.x)
- Create a logger and emit a message.
- Example target:
  - `org.apache.logging.log4j.LogManager.getLogger("test").info("hello");`

## Jackson
- Serialize a simple object to JSON.
- Example target:
  - `new com.fasterxml.jackson.databind.ObjectMapper().writeValueAsString(Map.of("a", 1))`

## Apache Commons Lang
- Use `StringUtils` and return a result.
- Example target:
  - `org.apache.commons.lang3.StringUtils.reverse("abc")` returns `"cba"`.
