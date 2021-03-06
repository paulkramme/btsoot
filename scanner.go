package main

import (
	"encoding/hex"
	"io"
	"log"
	"os"
	"path/filepath"
	"runtime"
	"time"

	"golang.org/x/crypto/blake2b"
)

const (
	// StopCode signals a thread to stop.
	StopCode = 1000
	// ConfirmCode confirms the execution of a signal.
	ConfirmCode = 1001
	// ErrorCode denies the save execution of a signal due to an error.
	ErrorCode = 1002
)

func blake2bsum(filePath string) (result string, err error) {
	file, err := os.Open(filePath)
	if err != nil {
		return
	}
	defer file.Close()

	buf := make([]byte, 128)

	hash, err := blake2b.New512(nil)
	if err != nil {
		panic(err)
	}
	_, err = io.CopyBuffer(hash, file, buf)
	if err != nil {
		return
	}
	result = hex.EncodeToString(hash.Sum(nil))
	return
}

func worker(in chan File, out chan File, comm chan bool) {
	for {
		FileToProcess, ok := <-in
		if ok != true {
			comm <- true
			return
		}
		if FileToProcess.Directory == true {
			out <- FileToProcess
			continue
		}
		hash, err := blake2bsum(FileToProcess.Path)
		if err != nil {
			log.Println(err)
			continue
		}
		FileToProcess.Checksum = hash
		out <- FileToProcess
	}
}

// ScanFilesBlake2b takes a folder and a maximum thread number, scans the directory, and returnes File type with blake2b checksums.
func ScanFilesBlake2b(location string, MaxWorkerThreads int) (files []File) {
	WFin := make(chan File)
	WFout := make(chan File)

	CheckIfDone := make(chan bool)
	WorkerMap := make(map[int]chan bool)
	if MaxWorkerThreads == 0 {
		MaxWorkerThreads = runtime.NumCPU()
	}
	for i := MaxWorkerThreads; i > 0; i-- {
		WorkerMap[i] = make(chan bool)
		go worker(WFin, WFout, WorkerMap[i])
	}
	var walkcallback = func(path string, fileinfo os.FileInfo, inputerror error) (err error) {
		if inputerror != nil {
			log.Println(inputerror)
			return
		}
		var f File
		f.Path = filepath.ToSlash(filepath.Clean(path))
		f.Size = fileinfo.Size()
		f.Directory = fileinfo.IsDir()
		WFin <- f
		return
	}
	go func() {
		err := filepath.Walk(location, walkcallback)
		close(WFin)
		if err != nil {
			panic(err)
		}
		CheckIfDone <- true
	}()

Resultloop:
	for {
		file := <-WFout
		relpath, err := filepath.Rel(location, file.Path)
		if err != nil {
			log.Println(err)
		}
		file.Path = relpath
		files = append(files, file)
		select {
		case <-CheckIfDone:
			for len(WorkerMap) != 0 {
				for key, value := range WorkerMap {
					select {
					case <-value:
						delete(WorkerMap, key)
					default:
						file := <-WFout
						relpath, err := filepath.Rel(location, file.Path)
						if err != nil {
							log.Println(err)
						}
						file.Path = relpath
						files = append(files, file)
					}
				}
			}
			break Resultloop
		default:
		}
	}
	return
}

// ScanFilesTimestamp takes a folder, scans the directory, and returnes File type with last changed dates as checksums.
func ScanFilesTimestamp(location string) (files []File) {
	var walkcallback = func(path string, fileinfo os.FileInfo, inputerror error) (err error) {
		if inputerror != nil {
			log.Println(inputerror)
			return
		}
		var f File
		f.Path, err = filepath.Rel(location, filepath.ToSlash(filepath.Clean(path)))
		if err != nil {
			log.Println(err)
		}
		f.Size = fileinfo.Size()
		f.Directory = fileinfo.IsDir()
		if !f.Directory {
			f.Checksum = fileinfo.ModTime().Format(time.RFC3339)
		}
		files = append(files, f)
		return
	}
	err := filepath.Walk(location, walkcallback)
	if err != nil {
		panic(err)
	}
	return
}
