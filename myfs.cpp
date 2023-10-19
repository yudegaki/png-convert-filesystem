#define FUSE_USE_VERSION 30

#include <fuse.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <fstream>
#include <iostream>
#include <sys/stat.h>

using namespace std;

static const char *mountpoint = "/mnt/fuse-convert-to-png";
static const char *base_directory = "/root";

static int myfs_getattr(const char *path, struct stat *stbuf) {

    string real_path = string(base_directory) + path;

    struct stat file_stat;
    if (lstat(real_path.c_str(), &file_stat) == -1) {
        return -ENOENT;
    }

    memcpy(stbuf, &file_stat, sizeof(struct stat));

    return 0;
}

static int myfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {

    string real_path = string(base_directory) + path;

    DIR *dir = opendir(real_path.c_str());
    if (!dir) {
        return -ENOENT;
    }

    struct dirent *entry;
    
    filler(buf, ".", NULL, 0);
    filler(buf, "..", NULL, 0);

    while ((entry = readdir(dir)) != NULL) {
        filler(buf, entry->d_name, NULL, 0);
    }

    closedir(dir);
    return 0;
}

static int myfs_open(const char *path, struct fuse_file_info *fi) {

    string real_path = string(base_directory) + path;

    struct stat stbuf;
    if (lstat(real_path.c_str(), &stbuf) == -1) {
        return -ENOENT;
    }

    int fd = open(real_path.c_str(), fi->flags);
    if (fd == -1) {
        return -errno;
    }

    fi->fh = fd;

    return 0;
}

static int myfs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {

    string real_path = string(base_directory) + path;

    ifstream file(real_path, ios::in | ios::binary);
    if (!file.is_open()) {
        return -ENOENT; 
    }

    file.seekg(offset);
    file.read(buf, size);

    size_t bytes_read = file.gcount();
    
    file.close();

    return bytes_read;
}

static int myfs_rename(const char *oldpath, const char *newpath) {

    string old_real_path = string(base_directory) + oldpath;
    string new_real_path = string(base_directory) + newpath;

    if (rename(old_real_path.c_str(), new_real_path.c_str()) == -1) {
        return -errno; 
    }

    return 0;
}

static int myfs_create(const char *path, mode_t mode, struct fuse_file_info *fi) {

    string real_path = string(base_directory) + path;

    // ref: https://www.ibm.com/docs/ja/zos/2.3.0?topic=functions-open-open-file
    // int fd = open(real_path.c_str(), fi->flags, mode);
    int fd = open(real_path.c_str(), O_CREAT | O_WRONLY | O_TRUNC, mode);

    if (fd == -1) {
        return -errno;
    }

    fi->fh = fd;

    // TODO: jpeg -> png 変換
    // TODO: heic -> png 変換

    return 0;
}

static struct fuse_operations myfs_op = {
    .getattr    = myfs_getattr,
    .rename     = myfs_rename,
    .open       = myfs_open,
    .read       = myfs_read,
    .readdir    = myfs_readdir,
    .create     = myfs_create,
};

int main(int argc, char *argv[]) {
    return fuse_main(argc, argv, &myfs_op, NULL);
}
