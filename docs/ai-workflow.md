# AI Development Workflow

This project uses a hybrid workflow combining ChatGPT, Aider, Ollama (Qwen3-Coder:30b), and GitHub.

## Overview
1. **Planning & Reviews (ChatGPT)**
   - Use ChatGPT (via the web) for high-level planning, brainstorming, and reviewing code or design.
   - ChatGPT can also review code from the GitHub repo if connected.

2. **Local Edits (Aider + Ollama)**
   - Aider is used locally to make code changes with AI assistance.
   - Ollama runs the `qwen3-coder:30b` model locally for fast, private code generation.
   - Aider interacts with your local Git repo to track changes.

3. **Version Control (Git + GitHub)**
   - **Git** is the source of truth locally.
   - **GitHub** is the remote repository for backup, collaboration, and ChatGPT access.
   - Work is committed locally and pushed to GitHub when ready.

## Typical Workflow
1. **Plan** feature or fix in ChatGPT.
2. **Edit** locally using Aider + Qwen3 for AI-assisted coding.
3. **Test** changes locally in Visual Studio Code or your dev environment.
4. **Commit** with `git commit -m "message"`.
5. **Push** to GitHub with `git push`.
6. **Review** in ChatGPT (optional) for feedback or improvements.

## Benefits
- Local AI coding without sending proprietary code to the cloud.
- Continuous Git version history for rollback and review.
- ChatGPT integration for strategic thinking and code review.
