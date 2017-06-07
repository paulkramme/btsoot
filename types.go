package main

import "os"
import "time"

type Duration struct {
	time.Duration
}

type File struct {
	Block
	Path     string
	Finfo    os.FileInfo
	Checksum string
}

type Block struct {
	Blockname   string
	Source      string
	Destination string
	Interval    Duration
}

type Configuration struct {
	LogFileLocation string
	DBFileLocation  string
}

// This function is needed to convert intervals (4m3s) to understandable formats
func (d *Duration) UnmarshalText(text []byte) error {
	var err error
	d.Duration, err = time.ParseDuration(string(text))
	return err
}