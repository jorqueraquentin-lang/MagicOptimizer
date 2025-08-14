# AI Development Workflow

This project uses a hybrid workflow combining ChatGPT, Aider, Ollama (Qwen3-Coder:30b), Unreal Engine, and Git/GitHub.

## Overview
1. **Planning & Reviews (ChatGPT)**
   - Use ChatGPT for outcome-based planning, breaking work into small compilable steps, and reviewing diffs/PRs.

2. **Local Edits (Aider + Ollama)**
   - Aider makes multi-file edits and auto-commits.
   - Ollama runs `qwen3-coder:30b` locally for private codegen.

3. **Version Control (Git + GitHub)**
   - Git is the local source of truth.
   - GitHub is the remote backup and collaboration point (and what ChatGPT reads).

## Daily Loop
1. **Plan in ChatGPT**: Describe the outcome (not line-by-line code).
2. **Edit with Aider**:
   ```
   cd D:\Documents\UE_Plugins\MagicOptimizer
   aider --model ollama/qwen3-coder:30b --edit-format whole
   ```
   - Paste a single structured prompt that lists each file path followed by a fenced code block containing full desired content.
   - End with: `Commit message: <your message>`
   - Keep one outcome per prompt.
3. **Build & Test in Unreal/VS**
   - Open `HostProject.uproject`.
   - Accept rebuild prompts.
   - If errors appear, copy exact error text back into Aider and say "fix the build error only".
4. **Push to GitHub**
   ```
   git push
   ```
5. **Iterate**
   - Repeat steps 2-4 until the feature is complete.

## Branching (optional)
