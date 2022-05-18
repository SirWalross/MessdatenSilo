#!/bin/bash

git add .
git commit -m "New data" && git push --force
echo ""
python3 main.py
rm data/data