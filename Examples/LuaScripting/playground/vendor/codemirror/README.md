# Vendored CodeMirror 5.65.18

Editor used by the Lua playground iframe (`../../editor.html`). Vendored so the
demo works offline and stays pinned; MIT-licensed, see `LICENSE`.

Files come from the `codemirror@5.65.18` npm package, minified with esbuild
(`npx esbuild <file> --minify --legal-comments=none`):

| file | source |
|------|--------|
| `codemirror.min.js`  | `lib/codemirror.js` |
| `codemirror.min.css` | `lib/codemirror.css` |
| `lua.min.js`         | `mode/lua/lua.js` |
| `addons.min.js`      | `addon/edit/matchbrackets.js` + `addon/edit/closebrackets.js` + `addon/comment/comment.js` + `addon/selection/active-line.js` (concatenated in that order) |
| `material-darker.min.css` | `theme/material-darker.css` |

To upgrade, repeat the steps above with the new version and bump this note.
