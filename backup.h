/*
 * Copyright (C) Paul Kramme 2017
 * 
 * Part of BTSOOT
 * Single folder redundancy offsite-backup utility
 * 
 * Licensed under MIT License
 */

#pragma once

#define _XOPEN_SOURCE 500
#ifndef XXHASH_C_2097394837
#define XXHASH_C_2097394837
#include<stdio.h>
#include<ftw.h>
#include<stdint.h>
#include<inttypes.h>
#include<unistd.h>
#include<time.h>
#include<pthread.h>

#include"sqlite3.h"
#include"btsoot.h"

#define XXH_STATIC_LINKING_ONLY
#include"xxhash.h"
#include"db.h"

#define FILEBUFFER 100000

int backup(job_t *job_import);

#endif
