# Documentation

Place markdown docs here. Put images in `docs/img/` and reference them with relative paths.

## Folder layout
- `docs/` – markdown files
- `docs/img/` – image assets
- `docs/templates/` – templates for capture and analysis

## Image usage
From a file in `docs/`:

```markdown
![Alt text](img/example.png)
```

From a file in `docs/guides/`:

```markdown
![Alt text](../img/example.png)
```

Linking to another doc:

```markdown
See the [Build Guide](build.md).
```

## Start here
- [Overview](overview.md)
- [Wireshark Capture Guide (Windows)](wireshark-windows.md)
- [Capture Log Template](templates/capture-log-template.md)

## Protocol References
- [UNI HUB AL v2 Protocol](protocols/uni-hub-alv2-protocol.md)
