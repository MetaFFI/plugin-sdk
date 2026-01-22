# Third-Party Library Tests (C)

These tests are intended to be called by a host language directly.
They are optional and require the listed dependencies to be available.

## OpenSSL
- Compute a SHA256 hash.
- Example target:
  - `SHA256("hello", ...)` and return hex string.

## libcurl
- Parse a URL (no network).
- Example target:
  - Use `curl_url` to extract scheme/host.

## cJSON
- Build a JSON object and stringify.
- Example target:
  - `cJSON_CreateObject`, add fields, `cJSON_PrintUnformatted`.
