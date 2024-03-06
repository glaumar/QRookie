# build rclone as static library
go -C rclone build --buildmode=c-archive -o librclone.a github.com/rclone/rclone/librclone
