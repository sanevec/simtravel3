#!/bin/sh

if [ "$(uname)" = "Darwin" ]; then
    # macOS
    cp .vscode/tasks_mac.json .vscode/tasks.json
elif [ "$(expr substr $(uname -s) 1 5)" = "Linux" ]; then
    # GNU/Linux (incluyendo Ubuntu)
    cp .vscode/tasks_ubuntu.json .vscode/tasks.json
else
    echo "Sistema operativo desconocido"
fi
