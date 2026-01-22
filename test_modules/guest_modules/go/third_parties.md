# Third-Party Library Tests (Go)

These tests are intended to be called by a host language directly.
They are optional and require the listed dependencies to be available.

## goquery (HTML parsing)
- Import `github.com/PuerkitoBio/goquery` and parse a simple HTML string.
- Example target:
  - `doc, _ := goquery.NewDocumentFromReader(strings.NewReader("<p>hi</p>"))`
  - `return doc.Find("p").Text()` should return `"hi"`.

## uuid
- Import `github.com/google/uuid` and return a generated UUID string.
- Example target:
  - `return uuid.NewString()`

## x/text
- Import `golang.org/x/text/unicode/norm` and normalize a string.
- Example target:
  - `return norm.NFC.String("e\u0301")`
