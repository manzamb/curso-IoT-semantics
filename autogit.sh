#!/bin/bash
echo "Autoguardamdo compias en Github"
git add .
git commit -m "$1"
git push origin master
git status
