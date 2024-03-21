#!/usr/bin/env sh

echo "Compiling go header and archive"

go mod tidy

export CC="gcc -w"
go build -buildmode=c-archive -o go-background.a background.go

echo "done"
