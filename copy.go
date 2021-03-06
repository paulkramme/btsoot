package main

import (
	"io"
	"os"
)

// CopyFile copies files and returnes error if it fails.
func CopyFile(Source string, Destination string) (err error) {
	fdSource, err := os.Open(Source)
	if err != nil {
		return
	}
	defer fdSource.Close()
	fdDestination, err := os.Create(Destination)
	if err != nil {
		return
	}
	defer fdDestination.Close()
	_, err = io.Copy(fdDestination, fdSource)
	if err != nil {
		return
	}
	fdDestination.Sync()
	return
}
