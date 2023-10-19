# Image convert to PNG file system by FUSE

## build

```bash
g++ -o myfs.out myfs.cpp `pkg-config fuse --cflags --libs`
```
