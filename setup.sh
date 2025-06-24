#!/bin/bash

echo "Setting up DashChain Python environment..."

# Step 1: Create venv if it doesn't exist
if [ ! -d "venv" ]; then
python3 -m venv venv
echo "Created virtual environment."
else
echo "Virtual environment already exists."
fi

# Step 2: Activate venv
source venv/bin/activate

# Step 3: Install dependencies
pip install --upgrade pip
pip install -r python/requirements.txt

echo "Environment ready. To activate it later: source venv/bin/activate"