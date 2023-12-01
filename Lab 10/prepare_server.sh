#!/bin/bash

# Remove all files from previous run
rm -rf Backup 2>/dev/null
rm -rf Results 2>/dev/null
rm req_id.txt 2>/dev/null
rm -rf Analysis 2>/dev/null

# Create new files
mkdir Backup
mkdir Results
mkdir Analysis

# Create new files for Backup
touch req_id.txt
touch ./Backup/queue_backup.txt
touch ./Backup/map_backup.csv
touch ./Backup/counter.txt
touch ./Backup/abc.txt
touch ./Backup/output.txt

# Make the server

make clean

make