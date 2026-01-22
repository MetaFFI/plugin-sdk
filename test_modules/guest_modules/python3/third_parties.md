# Third-Party Library Tests (Python)

These tests are intended to be called by a host language directly (not by the Python unit tests).
They are optional and require the listed dependencies to be installed in the Python environment.

## beautifulsoup4
- Import `bs4`, parse a simple HTML string, and read elements.
- Example target:
  - `from bs4 import BeautifulSoup`
  - `soup = BeautifulSoup("<html><body><p>hi</p></body></html>", "html.parser")`
  - `return soup.body.p.text` should return `"hi"`.

## numpy
- Create a 2D array and perform a reduction.
- Example target:
  - `import numpy as np`
  - `arr = np.array([[1, 2], [3, 4]], dtype=np.int64)`
  - `return int(arr.sum())` should return `10`.

## pandas
- Create a DataFrame and select a column.
- Example target:
  - `import pandas as pd`
  - `df = pd.DataFrame({"a": [1, 2, 3], "b": [4, 5, 6]})`
  - `return int(df["b"].sum())` should return `15`.

## requests
- Parse a URL and return a component (no network call).
- Example target:
  - `from urllib.parse import urlparse`
  - `u = urlparse("https://example.com/path?x=1")`
  - `return u.scheme` should return `"https"`.
