## v0.0.4
- Improve handling of fractional extents and advances to get better aligned glyphs
- Other miscellaneous fixes

## v0.0.3
- Fix issue with offsets not working in `ttr_draw_text_*` methods.

## v0.0.2
- Avoid allocating pixel buffer for each glyph.
- Improve rendering quality by using scaled size with harfbuzz.

## v0.0.1
- Initial release.
- Expose simple API for measuring and drawing shaped text.
- Example usage in main.cpp